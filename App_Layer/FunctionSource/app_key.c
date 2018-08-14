#include "platform_common.h"

#include "rtos.h"
#include "mid_interface.h"
#include "app_task.h"
#include "app_variable.h"
#include "app_protocal.h"
#include "app_key.h"
#include "app_systerm.h"
#include "app_win_common.h"
#include "app_win_process.h"
#include "app_packdata_manage.h"
#include "app_scenedata_manage.h"


#define  BAT_CHECK_PERIOD_STORE      14400//(3600 * WIN_SWITCH_PERIOD) //仓储下，1小时检测一次电量
#define  BAT_CHECK_TIME_INCHARGING   60   //电池电量检测时间间隔
#define  BAT_CHARGE_STATE_PERIOD     6//每3秒检查一次充电状态


#define  SwitchWinTime500mS       2
#define  SwitchWinTime1S          4              

TimerHandle_t switchWinTime;

static uint16 switchWinTimeCnt;
static uint16 fluckWinTimeCnt;
static uint8 fluckWinTime2HZCnt;



//**********************************************************************
// 函数功能：    窗口切换事件理
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
void App_SwitchWinProcess(void)
{
     menuMessage        message; 
     uint8          batLevel;
	 uint8 			stateTemp;
     multimodule_task_msg_t  msg;

     switchWinTimeCnt ++;
     fluckWinTimeCnt ++;

     if (switchWinTimeCnt >= 65535)
     {
         switchWinTimeCnt = 0;
         fluckWinTimeCnt = 0;
     }

    if (WinDowHandle == WIN_INVALID_HANDLE)
    {
        /* 窗口未创建前的响应 */
    }
    else
    {    
        //图标闪烁
        if (WinDowHandle == WIN_TIME_HANDLE)
        {    
            batLevel   = Mid_Bat_LevelRead();
            if (bleState == BLE_BROADCAST || batLevel == MID_BAT_LEVER_0)
            {
                message.val = fluckWinTimeCnt;              //2HZ闪烁
                WinDowHandle = App_Window_Process(WinDowHandle,TAG_ICON_FLICK,message); 
            }       
        }
        else if (WinDowHandle == WIN_HEART_HANDLE || WinDowHandle == WIN_STORE_HANDLE || WinDowHandle == WIN_RUN_HANDLE ||
                 WinDowHandle == WIN_CLIMB_HANDLE || WinDowHandle == WIN_SWING_HANDLE)
        {
           if (fluckWinTimeCnt % SwitchWinTime500mS == 0)   //1HZ闪烁
           {
                fluckWinTime2HZCnt ++;
                message.val = fluckWinTime2HZCnt % 2;
                WinDowHandle = App_Window_Process(WinDowHandle,TAG_ICON_FLICK,message); 
           }       
        }
     
        //每秒检索窗口切换
        if ((WinDowHandle != WIN_IDLE_HANDLE) && (switchWinTimeCnt % SwitchWinTime500mS)==0)
        {
            WinDowHandle        = App_Window_Process(WinDowHandle,TAG_WIN_CHANGE,message);
        }

        //BAT_CHARGE_STATE_PERIOD秒检测一次充放电状态
        if (switchWinTimeCnt % BAT_CHARGE_STATE_PERIOD == 0)
        {
            msg.id                     = BAT_ID;
            msg.module.batEvent.id     = CHARGE_CHECK;
            MultiModuleTask_EventSet(msg);
        }

        //电池检测——仓储状态下
        if ((WinDowHandle == WIN_STORE_HANDLE)&& (switchWinTimeCnt % BAT_CHECK_TIME_INCHARGING == 0))
        {   
            //充电状态下每
            Mid_Bat_ChargeStateRead(&stateTemp);
            if (stateTemp == MID_BAT_IN_CHARGING)
            {
                 msg.id                     = BAT_ID;
                msg.module.batEvent.id     = BAT_CHECK;
                MultiModuleTask_EventSet(msg);
            }
            else
            {
                if(switchWinTimeCnt % BAT_CHECK_PERIOD_STORE == 0)
                {  
                     msg.id                     = BAT_ID;
                    msg.module.batEvent.id     = BAT_CHECK;
                    MultiModuleTask_EventSet(msg);
                }  
            }
        }
    }
    
    //看门狗监测任务
    taskWatchdog.curCnt += 1;
}

//**********************************************************************
// 函数功能：    窗口切换定时器初始化
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
void App_SwitchWinInit(void (*TimerCb)(TimerHandle_t xTimer))
{
    switchWinTimeCnt = 0;
    fluckWinTimeCnt  = 0;
    fluckWinTime2HZCnt = 0;
    switchWinTime = xTimerCreate("switch_T", APP_1SEC_TICK / WIN_SWITCH_PERIOD, pdTRUE, 0, TimerCb);
}

//**********************************************************************
// 函数功能：    窗口切换定时器启动
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
void App_SwitchWinStart(void)
{
    if (xTimerIsTimerActive(switchWinTime) == pdFALSE)
    {
        xTimerReset(switchWinTime, 1);
        xTimerStart(switchWinTime, 1);
    }
}

//**********************************************************************
// 函数功能：    窗口切换定时器停止
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
void App_SwitchWinStop(void)
{
    if (xTimerIsTimerActive(switchWinTime) != pdFALSE)
    {
        xTimerStop(switchWinTime, 1);
    }
}

//**********************************************************************
// 函数功能：    按键处理
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
void App_KeyProcess(uint16 keyMsg)
{
    uint8           stateTemp;
    menuMessage     message;
	uint8 			socTemp;


    switchWinTimeCnt  = 0;
    fluckWinTimeCnt   = 0;
    fluckWinTime2HZCnt = 0;

    if (phoneState.state == PHONE_STATE_AUTHOR)
    {
        App_Protocal_AuthorPass();
        MultiModuleTask_EventSet(OP_MOTO);
        MultiModuleTask_EventSet(NULL_GREEN_LED);
        phoneState.state = PHONE_STATE_NORMAL;
		phoneState.timeCnt  = 0;		
    }
    else
    {
        App_SwitchWinStop();
        switch(keyMsg)
        {
            case MID_KEY0_SHORT:
             if (WinDowHandle == WIN_INVALID_HANDLE)
            {

            }  
            else
            {
                 message.val         = PRESS_S0;
                WinDowHandle        = App_Window_Process(WinDowHandle,TAG_KEY,message);
            }
            break;

            case MID_KEY1_SHORT:
            #if(PACK_DATA ==1)
            App_PackData_DataSaveFakeData();
            App_PackData_DataSaveFakeDataHeart();
            App_PackData_DataSavePrintf(STEPDATA_CLASSIFY);
            App_PackData_DataSavePrintf(SLEEPDATA_CLASSIFY);
            App_PackData_DataSavePrintf(HEARTDATA_CLASSIFY);
            #endif

            #if(SCENE_DATA == 1)
            // App_SceneData_DataSaveFakeData();
            App_SceneData_DataSavePrintf(RUN_SCENE_DATA_CLASSIFY + BASE_SCENE_DATA_CLASSIFY);
            #endif

            break;

            case MID_KEY2_SHORT:
            break;

            case MID_KEY0_HOLDSHORT:
            if (WinDowHandle == WIN_STORE_HANDLE)
            {
                //低电无法开机
                 Mid_Bat_SocRead(&socTemp);
                 if (socTemp < 5)
                 {
                     return ;
                 }
                
                 //充电状态下无法开机
                 Mid_Bat_ChargeStateRead(&stateTemp);
                 if (stateTemp == MID_BAT_IN_CHARGING)
                 {
                    return ;
                 }

                App_Protocal_BleStateSet(BLE_POWERON);
                bleState = BLE_BROADCAST;
                //等待协议栈开启
                vTaskDelay(50); 
                
				message.val         = HOLD_SHORT_S0;
				WinDowHandle        = App_Window_Process(WinDowHandle,TAG_KEY,message);
            }else if (WinDowHandle == WIN_RUN_HANDLE || WinDowHandle == WIN_CLIMB_HANDLE || WinDowHandle == WIN_SWING_HANDLE )
            {
                message.val         = HOLD_SHORT_S0;
                WinDowHandle        = App_Window_Process(WinDowHandle,TAG_KEY,message);
            }
            else
                ;
                 
            break;

            case MID_KEY1_HOLDSHORT:
            break;

            case MID_KEY2_HOLDSHORT:
            break;

            case MID_KEY0_HOLDLONG:
            if (WinDowHandle == WIN_INVALID_HANDLE)
            {
                /* 窗口未创建前的响应 */
            }
            else
            {
                message.val         = HOLD_LONG_S0;
                WinDowHandle        = App_Window_Process(WinDowHandle,TAG_KEY,message);  
            } 
            break;

            case MID_KEY1_HOLDLONG:
            
            break;

            case MID_KEY2_HOLDLONG:
            
            break;
        }
        App_SwitchWinStart();
    } 
}


