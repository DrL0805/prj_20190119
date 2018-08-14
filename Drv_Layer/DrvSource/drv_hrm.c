/**********************************************************************
**
**模块说明: 按键KEY驱动接口
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.18  修改流程  ZSL  
**
**********************************************************************/
#define HEART_RATE

//#include "string.h"
#include "io_config.h"

#if (HR_EN_PIN != IO_UNKNOW)
#include "sm_swi2c.h"
#include "sm_gpio.h"

// pah8011
#include "pah_driver.h"
#include "pah_verify.h"
//ALG file
#include "drv_accelerate.h"
// pah
#include "pah_driver_8011_reg.h"
#include "pah_verify_8011_option.h"
#include "pah_8011_internal.h"
#include "pah8011es_user.h"

#include "drv_hrm.h"

typedef enum
{
    bsp_status_idle                 = 0x0000,
    bsp_status_enabled              = 0x0001,
    bsp_status_disabled             = 0x0002,
    bsp_status_started              = 0x0004,
    bsp_status_stopped              = 0x0008,
    bsp_status_factorytest_started  = 0x0010,
    bsp_status_factorytest_stopped  = 0x0020,
} bsp_status_e;


/*******************variable define*******************/
static  bsp_status_e _hrmbspstate;

//heart rate adc data reference
uint8     *fifo_data;
uint32    fifo_data_num_per_ch;
uint32    fifo_ch_num;
uint8     ppg_mode_flag;
bool        is_touched ;

/*******************function define*******************/
void (*BspHrmCalculateCompleteCb)(uint8 HrmValue );

//记录mid层事件回调
hrm_event_cb Hrm_Event_CB;

void hrm_isr(uint32 u32PinNum)
{
    if(u32PinNum == HR_INT1_PIN)
    {
        Hrm_Event_CB(HR_DATA_READY);
    }
    else if(u32PinNum == HR_INT2_PIN)
    {
        Hrm_Event_CB(HR_TOUCH);
    }
    else
    {

    }
}

//**********************************************************************
//函数功能：心率模块硬件使能（I2C总线初始化、GPIO口配置）     
//输入参数：无   
//返回参数：
// 0x00   :  操作成功
// 0x01   :  操作失败
//**********************************************************************
uint8 Drv_Hrm_Open(hrm_event_cb hrm_cb)
{
	bool ret1;
	static uint8 initflag = 1;
 
	//power on
    SMDrv_GPIO_BitClear(HR_EN_PIN);
    SMDrv_GPIO_Open(HR_EN_PIN, NULL,NULL);

    //open iic for hrm
    SMDrv_SWI2C_Open(HR_IIC_MODULE,IIC_SPEED_HIGHEST); //MODULE 0

    //INT1 for data ready
    SMDrv_GPIO_Open(HR_INT1_PIN, NULL,hrm_isr);
	//INT2 for touch interrupt
	SMDrv_GPIO_Open(HR_INT2_PIN, NULL,hrm_isr);
    //set gpio isr prio
    SMDrv_GPIO_SetIrqPrio(2);

    //set mid event callback
    if(hrm_cb != NULL)
        Hrm_Event_CB = hrm_cb;
    
    _hrmbspstate = bsp_status_enabled;
	
	if(initflag)
	{
		ret1 = pah_init();
		if (!ret1)
		{
			return Ret_Fail;
		}
        initflag = 0;
	}
		
    return Ret_OK;
}

//**********************************************************************
//函数功能：心率模块硬件关闭（I2C总线关闭、GPIO关闭），电源启动会造成系统电压
//          下跌，硬件关闭暂不关闭电源，但需要关闭心率算法，才能降低功耗    
//输入参数：无   
//返回参数：
// 0x00   :  操作成功
// 0xFF   :  操作失败
//**********************************************************************
uint8 Drv_Hrm_Close(void)
{
    //power off
	//am_hal_gpio_out_bit_set(HR_EN_PIN);
    SMDrv_SWI2C_Close(HR_IIC_MODULE);

	//INT1
    SMDrv_GPIO_Close(HR_INT1_PIN);    
	//INT2    
    SMDrv_GPIO_Close(HR_INT2_PIN);
    
    _hrmbspstate = bsp_status_disabled;
    return Ret_OK;
}

//**********************************************************************
//函数功能：设置callback函数
//输入参数：事件回调函数
//返回参数：
// 0x00   :  操作成功
// 0x02   :  参数非法
//**********************************************************************
uint8 Drv_Hrm_SetCallBack(hrm_event_cb hrm_cb)
{
    if(hrm_cb == NULL)
        return Ret_InvalidParam;
    Hrm_Event_CB = hrm_cb;
    return Ret_OK;
}

//**********************************************************************
//函数功能： 启动心率测量：心率模块配置、状态设置    
//输入参数： 无    
//返回参数：
// 0x00   :  操作成功
// 0x01   :  操作失败
//**********************************************************************
uint8 Drv_Hrm_Start(void)
{
    if(_hrmbspstate == bsp_status_disabled) //8011在BSPdisable状态下，不能启动测量。
        return Ret_Fail;    
    
//    bool ret1 = pah_init();
//    if (!ret1)
//    {
//        return 0xFF;
//    }

#if defined(PPG_MODE_ONLY)
    start_healthcare_ppg();
#else
    start_healthcare_touch_only();//
#endif
    _state.status = main_status_start_healthcare;
    _hrmbspstate = bsp_status_started;

    return Ret_OK;
}

//**********************************************************************
//函数功能： 关闭心率测量：关闭心率算法、设置空闲状态  
//输入参数： 无    
//返回参数：
// 0x00   :  操作成功
// 0xFF   :  操作失败
//**********************************************************************
uint8 Drv_Hrm_Stop(void)
{    
    hr_algorithm_close();
    stop_healthcare();
    _state.status = main_status_idle;
    _hrmbspstate = bsp_status_stopped;
    return 0x00;
}

//**********************************************************************
//函数功能： 工厂测试 
//输入参数： 无    
//返回参数：
// 0x00   :  操作成功
// 0xFF   :  操作失败
//**********************************************************************
uint8 Drv_Hrm_FactoryTest(uint16 ui16lightleak[3])
{
    pah_verify_adc_data_s adc_data;
    pah_ret  ret = pah_err_unknown;

    _hrmbspstate = bsp_status_factorytest_started;

    ret = pah_verify_init();
    if (PAH_FAILED(ret))
        return 0xff;

    ret = pah_verify_light_leak_read_adc_data(&adc_data);
    
    if (PAH_FAILED(ret))
        return 0xff;

    _hrmbspstate = bsp_status_factorytest_stopped;

    ui16lightleak[0] = adc_data.ch_adc_data[0].value;
	ui16lightleak[1] = adc_data.ch_adc_data[2].value;
	ui16lightleak[2] = adc_data.ch_adc_data[3].value;

    if (ui16lightleak[0] > PAH_LIGHT_LEAK_CH_T_MAX)
    {
       return 0xff;
    }
    if (ui16lightleak[1] > PAH_LIGHT_LEAK_CH_B_MAX)
    {
       return 0xff;
    }
    if (ui16lightleak[2] > PAH_LIGHT_LEAK_CH_C_MAX)
    {
       return 0xff;
    }
    return 0x00;
}

//**********************************************************************
//函数功能： 读取心率模块有效触摸状态
//输入参数： 
//ui8istouch： 触摸状态指针    
//返回参数：
// 0x00   :  操作成功
// 0xFF   :  操作失败
//**********************************************************************
uint8 Drv_Hrm_ReadTouchStatus(uint8 *ui8istouch)
{
    if (SMDrv_GPIO_InBitRead(HR_INT2_PIN))//pah_is_touched()
    {
        *ui8istouch = ON_TOUCH;
    }
    else
    {
        *ui8istouch = OFF_TOUCH;
    }
    
    return 0x00;
}

//**********************************************************************
//函数功能： 心率计算
//输入参数： 
//ui64timestamp： 曝光时间调整量   
//返回参数：
// 0x00   :  操作成功
// 0xFF   :  操作失败
//**********************************************************************
uint8 Drv_Hrm_Calculate(uint64_t ui64timestamp)
{
	pah_ret ret = pah_err_unknown;
    uint8 hrm_temp;

    ret = pah_task();
    if (ret == pah_success)
    {
        if(pah_is_ppg_mode())
        {
            ppg_mode_flag   =0 ;
            #ifdef PPG_MODE_ONLY
            ppg_mode_flag   = 1 ;
            #endif
            is_touched      = pah_is_touched();
            if (is_touched || ppg_mode_flag)
            {
                fifo_data               = pah_get_fifo_data();
                fifo_data_num_per_ch    = pah_fifo_data_num_per_ch();
                fifo_ch_num             = pah_get_fifo_ch_num();    
                hrm_temp                = report_fifo_data(ui64timestamp, fifo_data, fifo_data_num_per_ch, fifo_ch_num, is_touched);//调整采样率，计算心率
                if (hrm_temp != 0)
                {
                    BspHrmCalculateCompleteCb(hrm_temp);//心率计算完成且有效回调
                }               
            }
            else
            {
                hr_algorithm_close();
                start_healthcare_touch_only();
            }
        }
        else if (pah_touch_mode == pah_query_mode())
        {
            if (pah_is_touched())
            {
                start_healthcare_ppg_touch(ui64timestamp);
            }                    
        }
    }
    else
    {
        return 0xFF;            
    }
    return 0x00;
}

//**********************************************************************
//函数功能： 心率计算完成回调函数初始化
//输入参数： 
//cb       : 回调函数
//返回参数： 
// 0x00   :  操作成功
// 0xFF   :  操作失败 
//**********************************************************************
 uint8 Drv_Hrm_SetCalCompleteCb(void (*cb)(uint8 hrmval))
{
    BspHrmCalculateCompleteCb = cb; 
    return 0x00;
}

                                                 
//*******************************accelermeter*************************************//
//动态测量需要加入重力数据
#define SAMPLE_NUM_PER_CH   80
#define AXIS_SIZE           3

static int16 mems_data[SAMPLE_NUM_PER_CH * 3];

static uint16 memscnt = 0;

//**********************************************************************
//函数功能： 存储心率算法重力数据
//输入参数：     
//fifodata： 重力数据指针
//fifo_size：重力数据长度   
//返回参数：     
// 0x00   :  操作成功
// 0xFF   :  操作失败 
//**********************************************************************
uint8 Drv_Hrm_SetAccelMen(int16 *fifodata, uint16 fifo_size)
{   
    uint16 i;

    if((memscnt + fifo_size) >= (SAMPLE_NUM_PER_CH * 3))
        return 0xff;
    
    for(i = 0; i < fifo_size; i++)
    {
        mems_data[memscnt++] = fifodata[i];
    }	
	return 0x00;
}

//**********************************************************************
//函数功能： 设置重力计量程
//输入参数： 量程值：0:2G, 1:4G, 2:8G, 3:16G
//返回参数： 无
//**********************************************************************
uint8 Drv_Hrm_SetAccelRange(uint8 newrange)
{
    pah8011_gSensor_Range_Set(newrange);

    return 0;
}

//**********************************************************************
//函数功能： 获取心率算法重力数据
//输入参数：     
//fifodata： 重力数据指针
//fifo_size：重力数据长度（每轴的数据长度）   
//返回参数： 无
//**********************************************************************
void accelerometer_get_fifo(int16 **fifo, uint32 *fifo_size)
{
    *fifo = mems_data;
    *fifo_size = memscnt / AXIS_SIZE;
    memscnt = 0;
}

//**********************************************************************
//函数功能： 启动重力计，数据缓存清零
//输入参数： 无
//返回参数： 无
//**********************************************************************
void accelerometer_start(void)
{
    // TODO
    // accel start
    memscnt = 0;    
}

//**********************************************************************
//函数功能： 关闭重力计，数据缓存清零
//输入参数： 无
//返回参数： 无
//**********************************************************************
void accelerometer_stop(void)
{
    // TODO
    //accel stop
    memscnt = 0;
}
#endif

