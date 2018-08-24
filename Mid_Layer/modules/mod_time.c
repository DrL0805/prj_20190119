
#include "mod_time.h"
#include "rtos.h"



static QueueHandle_t 	sTime_QueueHandle;				// 队列句柄
#define		TIME_TASK_QUEUE_LENGTH			3			// 
#define 	TIME_TASK_QUEUE_WAIT_TICK		100			// 队列阻塞时间
#define		TIME_TASK_QUEUE_SIZE				sizeof(Mod_Time_TaskMsg_T)

// RTC每秒调用事件处理
static inline void Mod_Time_RTCSecHandler(void)
{
	rtc_time_s tRTCTime;
	Mid_Rtc_TimeRead(&tRTCTime);
//	MOD_TIME_RTT_LOG(0,"20%d/%d/%d %02d:%02d:%02d \r\n",tRTCTime.year, tRTCTime.month, tRTCTime.day, tRTCTime.hour, tRTCTime.min, tRTCTime.sec);

	// 每秒检测一次锁屏事件
	App_Window_LockWinCnt();

	// 心率log信息存储
	HrmLogStorageProcess();
	
	//有与手机强关联状态
	if (phoneState.state != PHONE_STATE_NORMAL)
	{
		phoneState.timeCnt++;

		if (phoneState.timeCnt > phoneState.timeMax)
		{
			switch(phoneState.state)
			{
				case PHONE_STATE_PHOTO:	// 拍照
					//						message.state   = EXIT_TAKE_PHOTO_MODE;
					//						message.op      = TAKE_PHOTO_REMIND;
					//						WinDowHandle    = App_Window_Process(WinDowHandle,TAG_REMIND,message);
					break;
				case PHONE_STATE_AUTHOR:
					MOD_TIME_RTT_LOG(0,"PHONE_STATE_AUTHOR TimeOut \r\n");		// 授权超时
					break;
				case PHONE_STATE_PAIRE:
					// message.state = EXIT_PAIRE_STATE;
					// message.op    = PAIRE_REMIND;
					// WinDowHandle  = App_Window_Process(WinDowHandle,TAG_REMIND,message);
					break;
				case PHONE_STATE_HRM:
					//关闭实时心率场景（启动怀关闭需要成对出现）
					//						msg.id                                  = HEARTRATE_ID;     
					//						msg.module.hrmEvent.id                  = HRM_STOP;
					//						MultiModuleTask_EventSet(msg); 
					break;
				default: break;
			}
			phoneState.state    = PHONE_STATE_NORMAL,
			phoneState.timeCnt  = 0;
		} 
	}	
}

// RTC每分钟调用事件处理
static inline void Mod_Time_RTCMinHandler(void)
{
	rtc_time_s tRTCTime;
	uint32_t tToTalStep;
	
	Mid_Rtc_TimeRead(&tRTCTime);
	Mid_SportScene_StepRead(&tToTalStep);
	
	/* 静息心率算法 */
	HrmLogPeriodProcess();
	HrmLogRestingJudge(tToTalStep);
	
	// 每分钟检测一次闹钟
	if(Mid_AlarmClock_Check(&tRTCTime))
	{
		// 拍照状态不触发闹钟
		if (phoneState.state != PHONE_STATE_PHOTO)
		{
			// 向APP发送闹钟事件
			App_Protocal_AlarmRing(CurRingAlarmIdGet());

			// 向APP层发送闹钟事件
			Mid_Motor_ParamSet(eMidMotorShake2Hz, 5);
			Mid_Motor_ShakeStart();
		}
	}	
}

// RTC每小时调用事件处理
static inline void Mod_Time_RTCHourHandler(void)
{
	// 每小时向APP获取天气信息
    if(bleState == BLE_CONNECT)
    {
       App_Protocal_GetWeatherProcess();
    }
}

// RTC每天调用事件处理
static inline void Mod_Time_RTCDayHandler(void)
{
	// 每天同步RTC时间
    if (bleState == BLE_CONNECT)
    {
        App_Protocal_AdjustTimeprocess();
    }	
}

//**********************************************************************
// 函数功能: 按键调度任务处理函数
// 输入参数：
// 返回参数：
static void Mod_Time_RTCHandler(Mod_Time_TaskMsg_T*	Msg)
{
	switch (Msg->Param.RTC.Msg)
	{
		case eMidRTCMsgHalfSec:
			break;
		case eMidRTCMsgSec:
			Mod_Time_RTCSecHandler();
			break;
		case eMidRTCMsgMin:
			Mod_Time_RTCSecHandler();
			Mod_Time_RTCMinHandler();
			break;
		case eMidRTCMsgHour:
			Mod_Time_RTCSecHandler();
			Mod_Time_RTCMinHandler();			
			Mod_Time_RTCHourHandler();
			break;
		case eMidRTCMsgDay:
			Mod_Time_RTCSecHandler();
			Mod_Time_RTCMinHandler();	
			Mod_Time_RTCHourHandler();		
			Mod_Time_RTCDayHandler();
			break;
		default:
			break;
	}
}

// ***********************************************************************
//	以下是任务调度代码
// ***********************************************************************
static void Mod_Time_TaskProcess(void *pvParameters)
{
	Mod_Time_TaskMsg_T	Msg;

	// 创建消息队列
	sTime_QueueHandle = xQueueCreate(TIME_TASK_QUEUE_LENGTH, TIME_TASK_QUEUE_SIZE);
	if(sTime_QueueHandle == NULL)
	{
		MOD_TIME_RTT_ERR(0,"Time_Queue Create Err \r\n");
	}
	
	MOD_TIME_RTT_LOG(0,"Mod_Time_TaskCreate Suc \r\n");

	while(1)
	{
		if(xQueueReceive(sTime_QueueHandle, &Msg, portMAX_DELAY))
		{
			switch(Msg.Id)
			{
				case eTimeTaskMsgRTC:
					Mod_Time_RTCHandler(&Msg);
					break;
				default:
					MOD_TIME_RTT_WARN(0,"Time Msg Err %d \r\n", Msg.Id);
					break;
			}
		}
	}
}

void Mod_Time_TaskEventSet(Mod_Time_TaskMsg_T* Msg, uint8_t FromISR)
{
	 portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	
	if(FromISR)
	{
		if(pdPASS != xQueueSendToBackFromISR(sTime_QueueHandle, Msg, &xHigherPriorityTaskWoken))
		{
			MOD_TIME_RTT_ERR(0,"Mod_Time_TaskEventSet Err \r\n");
		}
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
	else
	{
		if(pdPASS != xQueueSendToBack(sTime_QueueHandle, Msg, TIME_TASK_QUEUE_WAIT_TICK))
		{
			MOD_TIME_RTT_ERR(0,"Mod_Time_TaskEventSet Err \r\n");
		}
	}
}

void Mod_Time_TaskCreate(void)
{
    if(pdPASS != xTaskCreate(Mod_Time_TaskProcess, "TimeTask", TASK_STACKDEPTH_MOD_TIME, NULL, TASK_PRIORITY_MOD_TIME, NULL))
	{
		MOD_TIME_RTT_ERR(0,"Mod_Time_TaskCreate Err \r\n");
	}
}










