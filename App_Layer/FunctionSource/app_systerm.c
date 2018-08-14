#include "platform_common.h"

#include "rtos.h"
#include "mid_interface.h"
#include "app_task.h"
#include "app_variable.h"
#include "app_protocal.h"
#include "app_key.h"
#include "app_win_common.h"
#include "app_win_process.h"
#include "app_systerm.h"
#include "app_cachedata_manage.h"



//**********************************************************************
// 函数功能：    打开系统的模块功能
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
void App_SystermRecover(void)
{
    movt_task_msg_t  movtMsg;
	
	phoneState.timeCnt  = 0;
	phoneState.timeMax  = 15;
    phoneState.state    = PHONE_STATE_NORMAL,
    
    systermConfig.stepCountSwitch = SWITCH_ON;
    systermConfig.heartrateSwitch = SWITCH_ON;
    systermConfig.gestureSwitch   = SWITCH_ON;

#if 0
    //充电恢复追针
	movtState.timeCnt = 0;
    movtState.state = MOVT_STATE_NORMAL;
    movtMsg.id      = MOVT_MSG_MC_RECOVER;
    MovtTask_EventSet(movtMsg);
#endif

    #if(AIR_PRESSURE_SUPPORT == 1)
    systermConfig.weatherSwitch   = SWITCH_OFF;
    #endif

    bleState = BLE_BROADCAST;
    App_Protocal_BleStateSet(BLE_BROADCAST);
}

//**********************************************************************
// 函数功能：    挂系统的模块功能
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
void App_SystermSuspend(void)
{
    multimodule_task_msg_t  msg;
    movt_task_msg_t  movtMsg;

    systermConfig.stepCountSwitch = SWITCH_OFF;
    systermConfig.heartrateSwitch = SWITCH_OFF;
    systermConfig.gestureSwitch   = SWITCH_OFF;

#if 0
    //充电停针
	movtState.timeCnt  = 0;
    movtState.state    = MOVT_STATE_STOP;
    movtMsg.id   	   = MOVT_MSG_MC_STOP;
    MovtTask_EventSet(movtMsg);
#endif

    #if(AIR_PRESSURE_SUPPORT == 1)
    systermConfig.weatherSwitch   = SWITCH_OFF;
    #endif

    bleState = BLE_SLEEP;
    App_Protocal_BleStateSet(BLE_SLEEP);

    //心率监测关闭
    msg.id                        = HEARTRATE_LOG_ID;
    msg.module.hrmLogEvent.id     = HRM_LOG_CLOSE;
    MultiModuleTask_EventSet(msg);  
}

//**********************************************************************
// 函数功能：    打开系统的模块功能
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
void App_OtaSystermRecover(void)
{
    multimodule_task_msg_t  msg;
    
    //走针开启
	movtState.timeCnt  = 0;
    movtState.state    = MOVT_STATE_NORMAL;
	
	phoneState.timeMax  = 15;
	phoneState.timeCnt  = 0;
    phoneState.state    = PHONE_STATE_NORMAL;

    systermConfig.stepCountSwitch = SWITCH_ON;
    systermConfig.heartrateSwitch = SWITCH_ON;
    systermConfig.gestureSwitch   = SWITCH_ON;

    #if(AIR_PRESSURE_SUPPORT == 1)
    systermConfig.weatherSwitch   = SWITCH_OFF;
    #endif 

    //电量计启动
    msg.id                        = BAT_ID;
    msg.module.batEvent.id        = BAT_WAKEUP;
    MultiModuleTask_EventSet(msg);

    //日常开启
    msg.id                       = USUAL_ID;
    msg.module.usualEvent.id     = USUAL_SCENCE_OPEN;
    MultiModuleTask_EventSet(msg);

     //睡眠开启
    msg.id                       = SLEEP_ID;
    msg.module.sleepEvent.id     = SLEEP_SCENE_START;
    MultiModuleTask_EventSet(msg);

     //手势开启
    msg.id                        = GESTURE_ID;
    msg.module.gestureEvent.id    = GESTURE_SCENCE_OPEN;
    MultiModuleTask_EventSet(msg); 

     //运动开启
    msg.id                       = SPORT_ID;
    msg.module.sportEvent.id     = SPORT_STEP_START;
    MultiModuleTask_EventSet(msg);
    stepState = STEP_PROCESS_STATE;   

    //心率监测开启
    msg.id                        = HEARTRATE_LOG_ID;
    msg.module.hrmLogEvent.id     = HRM_LOG_OPEN;
    MultiModuleTask_EventSet(msg);  

    #if(AIR_PRESSURE_SUPPORT == 1)
    //天气预报开启
    if (systermConfig.weatherSwitch == SWITCH_ON)
    {
        msg.id                        = WEATHER_ID;
        msg.module.weatherEvent.id = WEATHER_FORECAST_OPEN;
        MultiModuleTask_EventSet(msg);   
    }
    #endif

    //开机启动一次电量检测
    msg.id                     = BAT_ID;
    msg.module.batEvent.id     = BAT_CHECK;
    MultiModuleTask_EventSet(msg);

    //rtc计时开始
    Mid_Rtc_Start();
}

//**********************************************************************
// 函数功能：    挂系统的模块功能
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
void App_OtaSystermSuspend(void)
{
    multimodule_task_msg_t  msg;

    //ＯＴＡ不停针
    // movtState    = MOVT_STATE_STOP;

    //OTA，rtc计时不停止
    // Mid_Rtc_Stop();

    systermConfig.stepCountSwitch = SWITCH_OFF;
    systermConfig.heartrateSwitch = SWITCH_OFF;
    systermConfig.gestureSwitch   = SWITCH_OFF;

    #if(AIR_PRESSURE_SUPPORT == 1)
    systermConfig.weatherSwitch   = SWITCH_OFF;  
    #endif  

     //运动关闭
    msg.id                       = SPORT_ID;
    msg.module.sportEvent.id     = SPORT_STEP_STOP;
    MultiModuleTask_EventSet(msg);

    //日常关闭
    msg.id                       = USUAL_ID;
    msg.module.usualEvent.id     = USUAL_SCENCE_CLOSE;
    MultiModuleTask_EventSet(msg);

     //睡眠关闭
    msg.id                       = SLEEP_ID;
    msg.module.sleepEvent.id     = SLEEP_SCENE_STOP;
    MultiModuleTask_EventSet(msg);

    //心率监测关闭
    msg.id                        = HEARTRATE_LOG_ID;
    msg.module.hrmLogEvent.id     = HRM_LOG_CLOSE;
    MultiModuleTask_EventSet(msg);  

    //天气预报关闭
    #if(AIR_PRESSURE_SUPPORT == 1)
    if (systermConfig.weatherSwitch == SWITCH_ON)
    {
        msg.id                        = WEATHER_ID;
        msg.module.weatherEvent.id = WEATHER_FORECAST_CLOSE;
        MultiModuleTask_EventSet(msg); 
    }
    #endif
     
    //手势关闭
    msg.id                        = GESTURE_ID;
    msg.module.gestureEvent.id    = GESTURE_SCENCE_CLOSE;
    MultiModuleTask_EventSet(msg);       
}

//**********************************************************************
// 函数功能：    系统开机
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
void App_SystermPowerOn(void)
{
    multimodule_task_msg_t  msg;

  //走针开启
	movtState.timeCnt  = 0;
    movtState.state    = MOVT_STATE_NORMAL;
	
	phoneState.timeMax  = 15;
	phoneState.timeCnt  = 0;
    phoneState.state  = PHONE_STATE_NORMAL;


    systermConfig.stepCountSwitch = SWITCH_ON;
    systermConfig.heartrateSwitch = SWITCH_ON;
    systermConfig.gestureSwitch   = SWITCH_ON;
    systermConfig.longSitRemindSwitch = SWITCH_ON;

    #if(AIR_PRESSURE_SUPPORT == 1)
    systermConfig.weatherSwitch   = SWITCH_ON;
    #endif

    //rtc计时开始
    Mid_Rtc_Start();
    //看门狗开启
    App_WdtRestart();

    //电量计启动
    msg.id                        = BAT_ID;
    msg.module.batEvent.id        = BAT_WAKEUP;
    MultiModuleTask_EventSet(msg);

    //日常开启
    msg.id                       = USUAL_ID;
    msg.module.usualEvent.id     = USUAL_SCENCE_OPEN;
    MultiModuleTask_EventSet(msg);

     //睡眠开启
    msg.id                       = SLEEP_ID;
    msg.module.sleepEvent.id     = SLEEP_SCENE_START;
    MultiModuleTask_EventSet(msg);

     //手势开启
    msg.id                        = GESTURE_ID;
    msg.module.gestureEvent.id    = GESTURE_SCENCE_OPEN;
    MultiModuleTask_EventSet(msg); 

     //运动开启
    msg.id                       = SPORT_ID;
    msg.module.sportEvent.id     = SPORT_STEP_START;
    MultiModuleTask_EventSet(msg);
    stepState = STEP_PROCESS_STATE;   

    //心率监测开启
    msg.id                        = HEARTRATE_LOG_ID;
    msg.module.hrmLogEvent.id     = HRM_LOG_OPEN;
    MultiModuleTask_EventSet(msg);  

    #if(AIR_PRESSURE_SUPPORT == 1)
    //天气预报开启
    if (systermConfig.weatherSwitch == SWITCH_ON)
    {
        msg.id                        = WEATHER_ID;
        msg.module.weatherEvent.id = WEATHER_FORECAST_OPEN;
        MultiModuleTask_EventSet(msg);   
    }
    #endif

    //开机启动一次电量检测
    msg.id                     = BAT_ID;
    msg.module.batEvent.id     = BAT_CHECK;
    MultiModuleTask_EventSet(msg);
}

//**********************************************************************
// 函数功能：    系统关机
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
void App_SystermPowerOff(void)
{
    multimodule_task_msg_t  msg;
	
    //看门狗关闭
    App_WdtDisable();

    //切换窗口关闭
    App_SwitchWinStop();

    systermConfig.stepCountSwitch = SWITCH_OFF;
    systermConfig.heartrateSwitch = SWITCH_OFF;
    systermConfig.gestureSwitch   = SWITCH_OFF;

    #if(AIR_PRESSURE_SUPPORT == 1)
    systermConfig.weatherSwitch   = SWITCH_OFF;
    #endif
	
	movtState.timeCnt  = 0;
    movtState.state    = MOVT_STATE_STOP;
	
	phoneState.timeCnt  = 0;
    phoneState.state  = PHONE_STATE_NORMAL;

    //蓝牙关闭
    bleState = BLE_POWEROFF;
    App_Protocal_BleStateSet(BLE_POWEROFF);

     //运动关闭
    msg.id                       = SPORT_ID;
    msg.module.sportEvent.id     = SPORT_STEP_STOP;
    MultiModuleTask_EventSet(msg);

    //日常关闭
    msg.id                       = USUAL_ID;
    msg.module.usualEvent.id     = USUAL_SCENCE_CLOSE;
    MultiModuleTask_EventSet(msg);

     //睡眠关闭
    msg.id                       = SLEEP_ID;
    msg.module.sleepEvent.id     = SLEEP_SCENE_STOP;
    MultiModuleTask_EventSet(msg);

    //心率监测关闭
    msg.id                        = HEARTRATE_LOG_ID;
    msg.module.hrmLogEvent.id     = HRM_LOG_CLOSE;
    MultiModuleTask_EventSet(msg);  

    //天气预报关闭
    #if(AIR_PRESSURE_SUPPORT == 1)
    if (systermConfig.weatherSwitch == SWITCH_ON)
    {
        msg.id                        = WEATHER_ID;
        msg.module.weatherEvent.id = WEATHER_FORECAST_CLOSE;
        MultiModuleTask_EventSet(msg); 
    }
    #endif
     
    //手势关闭
    msg.id                        = GESTURE_ID;
    msg.module.gestureEvent.id    = GESTURE_SCENCE_CLOSE;
    MultiModuleTask_EventSet(msg);           

    //rtc计时停止
    Mid_Rtc_Stop();
}

//**********************************************************************
// 函数功能：    系统复位
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
void App_Systerm_Reset(void)
{
    //fix: 机芯开机后自动与APP连接，出现Disconnect 19
    App_Protocal_BleStateSet(BLE_SLEEP);
    bleState = BLE_SLEEP;
    //等待蓝牙连接断开，并停止广播
    vTaskDelay(50);//delay 值小，则无法达到断开连接的效果
    //fix 2018.5.31

    //request: 长按按键关机，进入store模式时，不需保存数据，并置power off 复位标志
    //App_CacheDataSaveAll();
    App_WriteResetFlag(POWER_OFF);
    //request: 2018.6.20
    
    vTaskDelay(50);

    // 关闭总中断并复位
    Mid_SystermReset();
}

//**********************************************************************
// 函数功能：    提醒反馈关闭
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
void App_Systerm_RemindCancel(void)
{
    MultiModuleTask_EventSet(NULL_GREEN_LED);
    MultiModuleTask_EventSet(NULL_RED_LED);
    MultiModuleTask_EventSet(NULL_MOTO);
}

