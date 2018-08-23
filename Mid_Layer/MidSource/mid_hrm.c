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
//#include "multimodule_task.h"
#include "main.h"

#define SystermTickPeriod           1000/APP_1SEC_TICK      //TICK时钟周期

static  uint8     mHeartRate      = 0xff;
static uint16     hrmAccelReadId;
static uint8      hrmEventCnt     = 0;
static uint8      hrmAccelRange   = 0;

//**********************************************************************
//函数功能：心率计算完成处理     
//输入参数：无   
//返回参数：无
// 调用：心率计算完成调用，向上层发送心率获取成功事件
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
	
	/* 向上层发送心率获取成功事件 */
	MID_HRM_RTT_LOG(0,"HrmCalculateCompleteProcess %d \r\n", mHeartRate);
}

// 调用：Drv_Hrm_Open函数调用
static void HrmProcess_Isr(hrm_event event)
{
	Mod_Algo_TaskMsg_T 	tAlgoMsg;
	
    if(event == HR_TOUCH)
    {
//		MID_HRM_RTT_LOG(0,"HR_TOUCH \r\n");	// 调用触摸函数，暂时没做处理		
    }
    else if(event == HR_DATA_READY)
    { 
//		MID_HRM_RTT_LOG(0,"HR_DATA_READY \r\n");
	
		// 通知算法模块
		tAlgoMsg.Id = eAlgoTaskMsgHrm;
		Mod_Algo_TaskEventSet(&tAlgoMsg, 1);		
    }
}


//**********************************************************************
//函数功能： 存储心率算法重力数据
//输入参数：     
//fifodata： 重力数据指针,三轴数据各一
//返回参数：     
// 0x00   :  操作成功
// 0xFF   :  操作失败  
// 调用：25Hz频率调用，传入Gsensor数据
void HrmAccelMenSet(int16 *fifodata)
{
    uint16     hrmAccelRate;

    //检测重力计量程并实现更新
//    Mid_Accel_SettingRead(&hrmAccelRate, &hrmAccelRange);
//    Drv_Hrm_SetAccelRange(hrmAccelRange);//获取当前的重力计配置更新给心率模块 
	Drv_Hrm_SetAccelRange(0);// 2g, 这版本算法好像没有设置Gsensor范围的接口
    Drv_Hrm_SetAccelMen(fifodata,3);
}

//**********************************************************************
//函数功能：心率算法启动
//输入参数：无   
//返回参数：
// 0x00   :  操作成功
// 0xFF   :  操作失败
uint16 HrmStart(void)   
{ 
	Drv_Hrm_Open(HrmProcess_Isr);
	Drv_Hrm_Start();

	return 0;  
}

//**********************************************************************
//函数功能：心率算法关闭
//输入参数：无   
//返回参数：
// 0x00   :  操作成功
// 0xFF   :  操作失败
uint16 HrmStop(void)    
{
	Drv_Hrm_Stop();
	Drv_Hrm_Close();

	return 0;
}

//**********************************************************************
// 函数功能: 心率模块爆光时间获取   
// 输入参数：    
// 返回参数：
// 调用：HrmCalculate() 心率计算调用
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
uint8 HrmCalculate(void)   
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
void Mid_Hrm_Init(void)
{
    Drv_Hrm_Open(HrmProcess_Isr);	//硬件初始化并配置心率模块
    Drv_Hrm_Close();				//硬件低功耗配置

    Drv_Hrm_SetCalCompleteCb(HrmCalculateCompleteProcess);
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
//函数功能：心率模块工厂测试
//输入参数：无   
//返回参数：
// 0x00   :  操作成功
// 0xFF   :  操作失败
//**********************************************************************
uint8 Mid_Hrm_FactoryTest(void)
{
    uint8 u8Ret;
    u8Ret = Drv_Hrm_Open(HrmProcess_Isr);
    vTaskDelay(10);
    u8Ret = Drv_Hrm_CheckHw();
    Drv_Hrm_Close();
    return u8Ret;
}

