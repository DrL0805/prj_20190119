
#include "mod_time.h"
#include "rtos.h"



static QueueHandle_t 	sTime_QueueHandle;				// 队列句柄
#define		TIME_TASK_QUEUE_LENGTH			3			// 
#define 	TIME_TASK_QUEUE_WAIT_TICK		100			// 队列阻塞时间
#define		TIME_TASK_QUEUE_SIZE				sizeof(Mod_Time_TaskMsg_T)

//**********************************************************************
// 函数功能: 按键调度任务处理函数
// 输入参数：
// 返回参数：
static void Mod_Time_RTCHandler(Mod_Time_TaskMsg_T*	Msg)
{
	rtc_time_s tRTCTime;
	
	switch (Msg->Param.RTC.Msg)
	{
		case eMidRTCMsgHalfSec:
			break;
		case eMidRTCMsgSec:
			Mid_Rtc_TimeRead(&tRTCTime);
			MOD_TIME_RTT_LOG(0,"RTC %02d:%02d:%02d \r\n",tRTCTime.hour, tRTCTime.min, tRTCTime.sec);

			// 每秒检测一次锁屏事件
			App_Window_LockWinCnt();
			break;
		case eMidRTCMsgMin:
			Mid_Rtc_TimeRead(&tRTCTime);
			MOD_TIME_RTT_LOG(0,"eMidRTCMsgMin %02d:%02d:%02d \r\n",tRTCTime.hour, tRTCTime.min, tRTCTime.sec);	
			
			// 每分钟检测一次闹钟
//			if(Mid_AlarmClock_Check(&tRTCTime))
			{
				// 打印当前闹钟ID
//				MOD_TIME_RTT_LOG(0,"AlarmClock Id %d \r\n",CurRingAlarmIdGet());

				// 向APP层发送闹钟事件
				
			}
		
			break;
		case eMidRTCMsgDay:
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










