#include "platform_common.h"
#include "app_protocal.h"

#include "rtos.h"
#include "mid_interface.h"
#include "app_variable.h"
#include "app_task.h"
#include "app_key.h"
#include "app_rtc.h"
#include "app_uart.h"
#include "app_display.h"
#include "app_remind_manage.h"
#include "app_packdata_manage.h"
#include "app_sleepdata_manage.h"
#include "app_win_common.h"
#include "app_win_process.h"
#include "app_systerm.h"
#include "app_cachedata_manage.h"
#include "app_scenedata_manage.h"
#include "app_action_recongnition.h"


/*******************functioin declear*******************/
void AppReadBleBroadcastName(void);


/*******************macro define*******************/
#define  WATCHDOG_TIMER_MAX            30//15秒看门狗复位

/*******************variable define*******************/
static QueueHandle_t        app_task_msg_queue;               /**< Queue handler for ble task */
uint8   wdtTime;


//************************************ key *******************************************//
//void AppKeyIsr(mid_key_msg key_msg)
//{
//   app_task_msg_t msg;
//    portBASE_TYPE xHigherPriorityTaskWoken;
//    
//    msg.id   = APP_MSG_KEY_ISR;
//    msg.para = key_msg;

//    xHigherPriorityTaskWoken  = pdFALSE;
//    if (errQUEUE_FULL == xQueueSendToFrontFromISR( app_task_msg_queue, (void *) &msg, &xHigherPriorityTaskWoken ))
//    {
//        // ("errQUEUE_FULL");
//    }
//    if( pdTRUE ==  xHigherPriorityTaskWoken )
//    {
//        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
//    }
//}

//************************************ bat *******************************************//
 void AppBatChargeStatesIsr(uint8 chargeState)
 {
	app_task_msg_t msg;
    portBASE_TYPE xHigherPriorityTaskWoken  = pdFALSE;

    if (chargeState == MID_BAT_OFF_CHARGE)
    {
        msg.para = BAT_DISCHARGE_STATE;
    }else if (chargeState == MID_BAT_IN_CHARGING)
    {
        msg.para = BAT_CHARGING_STATE;
    }
    else if (chargeState == MID_BAT_LOW_LEVEL)
    {
        msg.para = BAT_LOW_VOLTAGE_STATE;
    }
    else
    {
        msg.para = BAT_FULL_STATE;
    }
    msg.id                                  = APP_MSG_BAT_CHARGE_EVENT;
    
    if (errQUEUE_FULL == xQueueSendToFrontFromISR( app_task_msg_queue, (void *) &msg, &xHigherPriorityTaskWoken ))
    {
        // ("errQUEUE_FULL");
    }
    if( pdTRUE ==  xHigherPriorityTaskWoken )
    {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
 }

//************************************ rtc *******************************************//
void AppRtcIsr(MidRtc_Msg rtc_msg)
{
    uint8_t    wdtCnt  = 0;
    app_task_msg_t msg;
    portBASE_TYPE xHigherPriorityTaskWoken;
        
    switch(rtc_msg)
    {
        case RTC_DAY_MSG:     //RTC一天时间计时完成消息
        msg.id = APP_MSG_RTC_DAY;
        break;

        case RTC_MIN_MSG:     //RTC每分钟计时完成消息
        msg.id = APP_MSG_RTC_MIN;
        break;

        case RTC_SEC_MSG:     //RTC每秒计时完成消息
        msg.id = APP_MSG_RTC_SEC_ISR;
        break;

        case RTC_HALFSEC_MSG: //相比RtcSecCbInit延时半秒，计时完成消息
        //监测500内窗口切换无执行(实则监测app_task)
        if (taskWatchdog.lastCnt == taskWatchdog.curCnt)
        {
           wdtCnt ++ ;
        }
        else
        {
           taskWatchdog.lastCnt = taskWatchdog.curCnt;
        }
        if (wdtCnt == 0)
        {              
           wdtTime     = 0;            
        }
        else
        {       
            wdtTime ++;
        }

        if (wdtTime <= WATCHDOG_TIMER_MAX)  //连续15秒检测到有任务看门狗出现超时[空任务看门狗不计数]，认为任务出错，不喂狗
        {
           App_WdtRestart();
        }  
        //半秒事件设置
        msg.id = APP_MSG_RTC_HALF_SEC_ISR;
        break;

        default:
        return;
    }
    
    xHigherPriorityTaskWoken  = pdFALSE;
    if (errQUEUE_FULL == xQueueSendToFrontFromISR( app_task_msg_queue, (void *)&msg, &xHigherPriorityTaskWoken ))
    {
        // ("errQUEUE_FULL");
    }
    if( pdTRUE ==  xHigherPriorityTaskWoken )
    {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

//************************************ countdown *******************************************//
void AppCoundownIsr(MidCountDown_Msg count_msg)
{
    switch(count_msg)
    {
    case COUNTDOWN_DONE_MSG:     //倒计时完成消息
        
        break;
    case COUNTDOWN_SEC_MSG:      //倒计时一秒计时完成消息
        
        break;
    case COUNTDOWN_HALFSEC_MSG:  //相比COUNTDOWN_SEC_MSG延时半秒，倒计时完成消息
        
        break;
    default:
        return ;
    }
}

//************************************ stopwatch *******************************************//
void AppStopWatchIsr(MidStopWatch_Msg stopwatch_msg)
{
    switch(stopwatch_msg)
    {
    case STOPWATCH_32Hz_MSG:     //倒计时完成消息
        
        break;
    case STOPWATCH_SEC_MSG:      //倒计时一秒计时完成消息
        
        break;
    case STOPWATCH_HALFSEC_MSG:  //相比COUNTDOWN_SEC_MSG延时半秒，倒计时完成消息
        
        break;
    default:
        return ;
    }
}

//************************************ uart *******************************************//
static uint8 uartDataBuf[32];
static uint8 uartDataLength;
static void App_Uart_ReceiveAnalysisCb(uint8* dataBuf, uint8 length)
{
    uint8 i;
    app_task_msg_t msg;

    msg.id                                  = APP_MSG_UART_RECEIVE;
    for(i = 0; i < length; i++)
    {
        uartDataBuf[i] = dataBuf[i];
    }
    uartDataLength = length;
    xQueueSendToBack(app_task_msg_queue, &msg, 1);

}

//************************************ BLE *******************************************//
void App_BleDisConnCb(void)
{
    app_task_msg_t msg;

    msg.id        = APP_MSG_BLE_DISCONNCET;
    xQueueSendToBack(app_task_msg_queue, &msg, 1);
}

void App_BleOTA_Cb(uint16 u16status)
{
    app_task_msg_t msg;

    msg.id        = APP_MSG_BLE_OTA;
    msg.para      = u16status;
    xQueueSendToBack(app_task_msg_queue, &msg, 1);
}

//************************************ hrm *******************************************//
void App_SportStateCb(uint16 status)
{
    app_task_msg_t msg;

    msg.id        = APP_SPORT_EVENT;
    msg.para      = status;
    xQueueSendToBack(app_task_msg_queue, &msg, 1);
}

//************************************ sport stae *******************************************//
void AppHrmReadyCbProcess(uint8 hrmval)
{
    app_task_msg_t msg;

    msg.id        = APP_HRM_PROCESS;
    msg.para      = hrmval;
    xQueueSendToBack(app_task_msg_queue, &msg, 1);
}

//************************************ gesture *******************************************//
void AppGestureIsrCbProcess(uint8 gesturetype)
{
    app_task_msg_t     msg;

    switch(gesturetype)
    {
        case GESTURE_FORWARD:
        msg.para    = GESTURE_ACTION_FORWARD;
        break;

        case GESTURE_BACKWARD:
        msg.para    = GESTURE_ACTION_BACKWARD;
        break;

        case GESTURE_LEFT:
        msg.para    = GESTURE_ACTION_LEFT;
        break;

        case GESTURE_RIGHT:
        msg.para    = GESTURE_ACTION_RIGHT;
        break;

        case GESTURE_HAND_LIFT:
        msg.para    = GESTURE_ACTION_HAND_LIFT;
        break;

        case GESTURE_HAND_DOWN:
        msg.para    = GESTURE_ACTION_HAND_DOWN;
        break;
    }
    if ((WinDowHandle == WIN_IDLE_HANDLE || WinDowHandle == WIN_RUN_HANDLE) && systermConfig.gestureSwitch == SWITCH_ON)
    {
        msg.id                                  = APP_GESTURE_PROCESS;

        portBASE_TYPE xHigherPriorityTaskWoken  = pdFALSE;
        if (errQUEUE_FULL == xQueueSendToFrontFromISR(app_task_msg_queue, (void *) &msg, &xHigherPriorityTaskWoken ))
        {
            // ("errQUEUE_FULL");
        }
        if( pdTRUE ==  xHigherPriorityTaskWoken )
        {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    } 
}

//************************************ screen:滚屏事件设置 *******************************************//
void App_PicRoundProcess(TimerHandle_t xTimer)
{
    app_task_msg_t msg;

    msg.id        = APP_DISP_ROUND_PROCESS;
    xQueueSendToBack(app_task_msg_queue, &msg, 1);
}

//************************************ windown：窗口切换事件设置 *******************************************//
void App_SwitchWinProcessCb(TimerHandle_t xTimer)
{
    app_task_msg_t msg;

    msg.id        = APP_SWITCH_WIN_PROCESS;
    xQueueSendToBack(app_task_msg_queue, &msg, 1);
}

//************************************ 分包数据上传事件 *******************************************//
void App_PackData_DataReUploadCb(TimerHandle_t xTimer)
{
    app_task_msg_t msg;

    msg.id        = APP_PACK_DATA_UPLOAD_PROCESS;
    xQueueSendToBack(app_task_msg_queue, &msg, 1);
}

//************************************ 睡眠数据上传事件 *******************************************//
void App_SleepData_DataReUploadCb(TimerHandle_t xTimer)
{
    app_task_msg_t msg;

    msg.id        = APP_SLEEP_DATA_UPLOAD_PROCESS;
    xQueueSendToBack(app_task_msg_queue, &msg, 1);
}

//************************************ 场景数据上传事件 *******************************************//
void App_SceneData_DataReUploadCb(TimerHandle_t xTimer)
{
    app_task_msg_t msg;

    msg.id        = APP_SCENE_DATA_UPLOAD_PROCESS;
    xQueueSendToBack(app_task_msg_queue, &msg, 1);
}

//************************************ app tash事件设置 *******************************************//
void App_Task_EventSet(app_task_msg_t msg)
{
    xQueueSendToBack(app_task_msg_queue, &msg, 1);
}


void AppTask(void *pvParameters)
{
    app_task_msg_t msg;
    menuMessage         message;
	rtc_time_s         appRtcTemp;
    reset_type         powerOnType;


    resetStatus     = 0;
    // 其他任务优先执行，
    portYIELD();

    //获取reset flag，如果是上电复位，将蓝牙power off
    powerOnType =  App_ResetFlagCheck();
    if(powerOnType == POWER_OFF)
    {
        App_Protocal_BleStateSet(BLE_POWEROFF);
        bleState = BLE_POWEROFF;
    }

    app_task_msg_queue  = xQueueCreate(APP_APP_TASK_QUEUE_SIZE, sizeof(app_task_msg_t));
	
    //window set
    WinDowHandle            = WIN_INVALID_HANDLE;
    backstageTimeCounterSec = 0;
    backstageTimeCounterMin = 0;

    //movt
    movtState.state  = MOVT_STATE_STOP;
    phoneState.state = PHONE_STATE_NORMAL;
    // key
    //设置短按时间 3s
//    Mid_Key_HoldShortTimeSet(32*3,KEY_S0);
//    Mid_Key_HoldShortTimeSet(32*3,KEY_S1);
//    Mid_Key_HoldShortTimeSet(32*3,KEY_S2);
//    //设置长按时间 6s
//    Mid_Key_HoldLongTimeSet(32*12,KEY_S0);
//    Mid_Key_HoldLongTimeSet(32*12,KEY_S1);
//    Mid_Key_HoldLongTimeSet(32*12,KEY_S2);   
//    //设置消息处理callback
//    Mid_Key_SetCallBack(AppKeyIsr);

    //rtc
    Mid_Rtc_Init();
    Mid_Rtc_SetCallBack(AppRtcIsr);
 
    // 初始化时间2018年1月1日 00:00:00  东八区
    appRtcTemp.year       = 18;
    appRtcTemp.month      = 1;
    appRtcTemp.day        = 1; 
    appRtcTemp.hour       = 0;
    appRtcTemp.min        = 0;
    appRtcTemp.sec        = 0;
    appRtcTemp.zone       = 0x0800;
    Mid_Rtc_TimeWrite(&appRtcTemp);
    TimeZoneTransform(&appRtcTemp,&WorldRtc.rtc);

    //coundown
    Mid_Countdown_Init();
    Mid_Countdown_SetCallBack(AppCoundownIsr);

    // stopwatch
    Mid_StopWatch_Init();
    Mid_StopWatch_SetCallBack(AppStopWatchIsr);

    //app uart
    Mid_Uart_ReceiveCbInit(DebugUartModule,App_Uart_ReceiveAnalysisCb);
    //bat
    Mid_Bat_Init();
    Mid_Bat_SetCallBack(AppBatChargeStatesIsr);

    //step scene
    stepState = STEP_IDLE_STATE;
    Mid_SportScene_Init();
    Mid_SportScene_SportStateCbSet(App_SportStateCb);
 
    //gesture
    Mid_GestureScene_Init(AppGestureIsrCbProcess,App_Action_Recongnition);

    //climb scene
    Mid_ClimbScene_Init();

     //hrm
    Mid_Hrm_ReadyCbInit(AppHrmReadyCbProcess);
    Mid_HeartRateScene_Init();

    //usual scene
    Mid_UsualScene_Init();
    Mid_UsualScene_Enable(USUAL_SCENE_SLEEP);
    Mid_UsualScene_Enable(USUAL_SCENE_SEDENTARY);
    Mid_UsualScene_Enable(USUAL_SCENE_STANDSTILL);

    //sport scene
    sceneSwitch = SCENE_NULL;
    Mid_Scene_Init();

    //watchdog 
    App_WdtInit();

    //protocal
    App_Protocal_Init(); // 初始化协议分析模块

    //remind manage 
    App_RemindManage_SaveInit();  

    //pack data 
    App_PackData_DataSaveInit(App_PackData_DataReUploadCb);

    //sleep data
    App_SleepData_SaveInit(App_SleepData_DataReUploadCb);

    //scene data
    App_SceneData_DataSaveInit(App_SceneData_DataReUploadCb);

    //显示功能模块初始化
    App_DispInit(App_PicRoundProcess);

    //显示窗口切换功能模块初始化
    App_SwitchWinInit(App_SwitchWinProcessCb);

     //配置信息恢复
    //App_CacheDataRecoverAll(); 

    //数据分包恢复 //暂不作分包恢复
    // App_PackData_DataSavePowerdownRecover();

    //处理几种复位: WDT异常复位重启，OTA 复位重启以及掉电复位
    //step -1: recover data info
    if(powerOnType == WDT_RESET)
    {
        App_WdtCacheRecover();   //配置信息恢复，恢复wdt复位前的保存的数据
    }
    else if((powerOnType == OTA_RESET) || (powerOnType == POWER_DOWN))
    {
        App_CacheDataRecoverAll();    //配置信息恢复
    }
    else
        ;  //按键关机复位系统使用默认值，不作恢复数据动作

    //step -2:reset reset flag
    App_ResetFlagClear();
    
    if((powerOnType == WDT_RESET) || (powerOnType == OTA_RESET))
    {
        //初始化完成
        vTaskDelay(100);
        //step -3: goto time mode
        WinDowHandle = App_Window_Create(WIN_TIME_HANDLE);
        //step -4:power on system, and start display timer
        App_SystermPowerOn();
        App_SwitchWinStart();

        resetStatus = 1;
    }
    else
    {
        //初始化完成
        vTaskDelay(200); 
        WinDowHandle = App_Window_Create(WIN_STORE_HANDLE);
    }

    for(;;)
    {
        if(xQueueReceive(app_task_msg_queue, &msg, portMAX_DELAY))
        {
            switch(msg.id)
            {
            case APP_MSG_BLE_DISCONNCET:  //蓝牙断开
                App_Protocal_BleDisConn();         
                break;
            case APP_MSG_BLE_OTA:         //OTA
                App_Protocal_OTAStatus(msg.para);
                break;

            // KEY 
            case APP_MSG_KEY_ISR:
            App_KeyProcess(msg.para);
            break;

            // RTC
            case APP_MSG_RTC_SEC_ISR:
            App_RtcSec_Process();            
            break;

            case APP_MSG_RTC_HALF_SEC_ISR:
            App_RtcHalfSec_Process();
            break;

            //alarm one mine process
            case APP_MSG_RTC_MIN:
            App_RtcMin_Process();   
            break;

            case APP_MSG_RTC_HOUR:
			App_RtcHour_Process(); 
            break;

            case APP_MSG_RTC_DAY:
            App_RtcDay_Process();
            break;

            case APP_MSG_RTC_MONTH:
            break;

            case APP_MSG_RTC_YEAR:
            break;

            case APP_MSG_BAT_CHARGE_EVENT:                
            message.state   = msg.para;
			message.op      = BAT_REMIND;
            WinDowHandle    = App_Window_Process(WinDowHandle,TAG_REMIND,message);
            break;

            case APP_MSG_UART_RECEIVE:
			App_Uart_Analysis(uartDataBuf, uartDataLength);
            break;

            case APP_SPORT_EVENT:
            if (msg.para == STEP_COMPLETE)
            {
                message.op      = SPORT_COMPLETE_REMIND;
                WinDowHandle    = App_Window_Process(WinDowHandle,TAG_REMIND,message);
            }
            else
            {

            }
            break;

            case APP_HRM_PROCESS:
            if (WinDowHandle == WIN_HEART_HANDLE)
            {
                message.val         = msg.para;
                WinDowHandle        = App_Window_Process(WinDowHandle,TAG_HEART,message); 
            }
            #if 0 //暂不开放app心率场景
            if (phoneState.state == PHONE_STATE_HRM && msg.para != 0)
            {
                App_Protocal_HrmRet((uint8)msg.para);
            }
            #endif           
            break;

            case APP_GESTURE_PROCESS:
            message.op         = msg.para;
            WinDowHandle        = App_Window_Process(WinDowHandle,TAG_GESTURE,message);
            break;

            case APP_DISP_ROUND_PROCESS:
            App_EventRoundProcess();
            break;

            case APP_SWITCH_WIN_PROCESS:
            App_SwitchWinProcess();
            break;

            case APP_PACK_DATA_UPLOAD_PROCESS:
            App_PackData_DataUploadProcess();
            break;

            case APP_SLEEP_DATA_UPLOAD_PROCESS:
            App_SleepData_DataUploadProcess();
            break;

            case APP_SCENE_DATA_UPLOAD_PROCESS:
            App_SceneData_DataUploadProcess();
            break;

            case APP_SCENE_DATA_SAVE:
            App_SceneData_SaveEnd(msg.para);
            break;

            case APP_SCENE_DATA_SAVE_CHECK:
            App_SceneData_StorageSpaceCheck(msg.para);
            break;

            case APP_CACEH_DATA_SAVE:
            App_CacheDataSaveType(msg.para);
            break;

            default:
                break;
            }
        }
    }
}

void AppTask_Create(void)
{
    xTaskCreate(AppTask, "AppTask", 512, NULL, TASK_PRIORITY_LOW, NULL);
}




