/**********************************************************************
**
**模块说明: mid层心率接口
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.25  修改流程  ZSL  
**
**********************************************************************/
/* FreeRTOS includes */
#include "rtos.h"

#include "drv_hrm.h"
#include "mid_hrm.h"
#include "multimodule_task.h"

#define SystermTickPeriod           1000/APP_1SEC_TICK      //TICK时钟周期

static  uint8     mHeartRate      = 0xff;
static uint16     hrmAccelReadId;
static uint8      hrmEventCnt     = 0;
static uint8      hrmAccelRange   = 0;

func *HrmTouchIsrProcess;
func *HrmDataReadyIsrProcess;
void (*HrmReadyProcess)(uint8 hrmval);


//**********************************************************************
//函数功能：心率计算完成处理     
//输入参数：无   
//返回参数：无
void HrmCalculateCompleteProcess(uint8 hrmval)
{
    mHeartRate     = hrmval;

    if (mHeartRate >240)
    {
        mHeartRate = 240;
    }
    else if (mHeartRate < 30)
    {
        mHeartRate = 30;
    }
    else
        ;
    HrmReadyProcess(mHeartRate);
}


static void HrmProcess_Isr(hrm_event event)
{
    if(event == HR_TOUCH)
    {
        HrmTouchIsrProcess();
    }
    else if(event == HR_DATA_READY)
    {
        HrmDataReadyIsrProcess();
    }
}


//**********************************************************************
//函数功能：心率模块使能
//输入参数：无   
//返回参数：
// 0x00   :  操作成功
// 0xFF   :  操作失败
static uint16 HrmEnable(void)  
{
   return Drv_Hrm_Open(HrmProcess_Isr);
}

//**********************************************************************
//函数功能：心率模块关闭
//输入参数：无   
//返回参数：
// 0x00   :  操作成功
// 0xFF   :  操作失败
static uint16 HrmDisable(void) 
{
    return Drv_Hrm_Close();
}

//**********************************************************************
//函数功能： 存储心率算法重力数据
//输入参数：     
//fifodata： 重力数据指针,三轴数据各一
//返回参数：     
// 0x00   :  操作成功
// 0xFF   :  操作失败  
static void HrmAccelMenSet(int16 *fifodata)
{
    uint16     hrmAccelRate;

    //检测重力计量程并实现更新
    Mid_Accel_SettingRead(&hrmAccelRate, &hrmAccelRange);
    Drv_Hrm_SetAccelRange(hrmAccelRange);//获取当前的重力计配置更新给心率模块 
    Drv_Hrm_SetAccelMen(fifodata,3);
}

//**********************************************************************
//函数功能：心率算法启动
//输入参数：无   
//返回参数：
// 0x00   :  操作成功
// 0xFF   :  操作失败
static uint16 HrmStart(void)   
{ 
    multimodule_task_msg_t  msg;

    if (hrmEventCnt < 255)
    {
        if (hrmEventCnt == 0)//第一个应用开启需要分配AccelReadId并启动心率模块
        {
            Drv_Hrm_Open(HrmProcess_Isr);
            Drv_Hrm_Start();
            
            msg.id                            = ACCEL_ID;
            msg.module.accelEvent.id          = ACCEL_READ_SET;
            msg.module.accelEvent.readId      = &hrmAccelReadId;
            msg.module.accelEvent.rate        = ACCEL_25HZ;
            msg.module.accelEvent.scaleRange  = ACCEL_2G;
            msg.module.accelEvent.Cb          = HrmAccelMenSet;
            MultiModuleTask_EventSet(msg);         
        }
        hrmEventCnt ++;//每启动一次，心率事件加1
        return 0;  
    }
    return 0xff; 
}

//**********************************************************************
//函数功能：心率算法关闭
//输入参数：无   
//返回参数：
// 0x00   :  操作成功
// 0xFF   :  操作失败
static uint16 HrmStop(void)    
{
    multimodule_task_msg_t  msg;

    if (hrmEventCnt > 0)
    {
        hrmEventCnt --;//每关闭一次，心率事件减1

        if (hrmEventCnt == 0)//所有应用关闭，则关闭心率模块，同时删除AccelReadId
        {
            Drv_Hrm_Stop();
            Drv_Hrm_Close();

            msg.id                            = ACCEL_ID;
            msg.module.accelEvent.id          = ACCEL_READ_DELETE;
            msg.module.accelEvent.readId      = &hrmAccelReadId;
            MultiModuleTask_EventSet(msg);
        }
        return 0;
    }   
    return 0xff;
}

//**********************************************************************
// 函数功能: 心率模块爆光时间获取   
// 输入参数：    
// 返回参数：
static uint64 get_tick_count(void)
{
    return ( SystermTickPeriod * xTaskGetTickCount());
}

//**********************************************************************
//函数功能：心率计算
//输入参数：无
//ui64timestamp： 曝光时间
//返回参数：
// 0x00   :  操作成功
// 0xFF   :  操作失败
static uint8 HrmCalculate(void)   
{
    return Drv_Hrm_Calculate(get_tick_count());
}

//**********************************************************************
//函数功能： 心率模块初始化
//输入参数：     
//ReadyIsrCb 重力数据指针
//TouchIsrCb：重力数据长度   
//返回参数： 无
//**********************************************************************
void Mid_Hrm_Init(func *ReadyIsrCb,func *TouchIsrCb)
{
    hrmAccelReadId  = 0xffff;
    hrmEventCnt     = 0;

    Drv_Hrm_Open(HrmProcess_Isr);//硬件初始化并配置心率模块
    Drv_Hrm_Close();//硬件低功耗配置
	
	HrmDataReadyIsrProcess     = ReadyIsrCb;
    HrmTouchIsrProcess         = TouchIsrCb;
    Drv_Hrm_SetCalCompleteCb(HrmCalculateCompleteProcess);
}

//**********************************************************************
//函数功能： 心率模块初始化
//输入参数：     
//ReadyIsrCb 重力数据指针
//TouchIsrCb：重力数据长度   
//返回参数： 无
//**********************************************************************
void Mid_Hrm_ReadyCbInit(void (*hrmreadycb)(uint8 hrmval))
{
    HrmReadyProcess     = hrmreadycb;
}

//**********************************************************************
//函数功能：触摸状态获取
//输入参数：无   
//返回参数：
// 0x00   :  操作成功
// 0xFF   :  操作失败
//**********************************************************************
uint16 Mid_Hrm_TouchStatusRead(uint8 *ui8istouch)
{    
    return Drv_Hrm_ReadTouchStatus(ui8istouch);
}

//**********************************************************************
//函数功能：心率值获取
//输入参数：无   
//返回参数：心率值
//**********************************************************************
uint16  Mid_Hrm_Read(uint8 *hrmval)
{
    *hrmval = mHeartRate;
    return 0;
}

//**********************************************************************
//函数功能：心率模块漏光测试数据获取
//输入参数：无   
//返回参数：
// 0x00   :  操作成功
// 0xFF   :  操作失败
//**********************************************************************
uint16 Mid_Hrm_FactoryTestRead(uint16 ui16lightleak[3])              
{
    uint16 ret;

    ret = Drv_Hrm_FactoryTest(ui16lightleak);

    return ret;
}

//**********************************************************************
// 函数功能:    心率模块事件处理
// 输入参数：    无
// 返回参数：    0x00: success
//              0xff: fail
//**********************************************************************
uint16 Mid_Hrm_EventProcess(hrm_event_s *msg)
{
     switch(msg->id)
     {
        case HRM_ENABLE:
         HrmEnable();
         break;

         case HRM_DISABLE:
         HrmDisable();
         break;

         case HRM_STOP:
         HrmStop();
         break;

         case HRM_START:
         HrmStart();
         break;

         case HRM_DATA_READY:   
         HrmCalculate();      
         break;
         
         case HRM_TOUCH:         
         break;

         case HRM_CALCULATE:
         HrmCalculate();  
         break;

         default:
         break;
    }
    return 0x00;
}

