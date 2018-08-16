
#include "mod_algorithm.h"
#include "mid_scheduler.h"


#include "rtos.h"

static QueueHandle_t 	sAlgo_QueueHandle;				// 队列句柄
#define		ALGO_TASK_QUEUE_LENGTH			3			// 
#define 	ALGO_TASK_QUEUE_WAIT_TICK		100			// 队列阻塞时间
#define		ALGO_TASK_QUEUE_SIZE				sizeof(Mod_Algo_TaskMsg_T)

static void Mid_Algo_AccelHandler(Mod_Algo_TaskMsg_T* Msg)
{
	int16_t	tData[3];
	uint32_t tInterval;
	MID_ACCEL_PARA_T	tMidAccel;
	
	// 读取数据
	Mid_Accel_ParamGet(&tMidAccel);
	
	// 计步算法
	Mid_SportScene_Algorithm(tMidAccel.LatestData, tMidAccel.SamplePeriod);
}

static void Mod_Algo_TaskProcess(void *pvParameters)
{
	Mod_Algo_TaskMsg_T	Msg;

	// 创建消息队列
	sAlgo_QueueHandle = xQueueCreate(ALGO_TASK_QUEUE_LENGTH, ALGO_TASK_QUEUE_SIZE);
	if(sAlgo_QueueHandle == NULL)
	{
		MOD_ALGO_RTT_ERR(0,"Algo_Queue Create Err \r\n");
	}
	
	MOD_ALGO_RTT_LOG(0,"Mod_Algo_TaskCreate Suc \r\n");

	while(1)
	{
		if(xQueueReceive(sAlgo_QueueHandle, &Msg, portMAX_DELAY))
		{
			switch (Msg.Id)
            {
            	case eAlgoTaskMsgAccel:
					Mid_Algo_AccelHandler(&Msg);
            		break;
            	case eAlgoTaskMsgGyro:
            		break;
				case eAlgoTaskMsgMagnetism:
					break;
				case eAlgoTaskMsgGPS:
					break;
            	default:
            		break;
            }
		}
	}
}

void Mod_Algo_TaskEventSet(Mod_Algo_TaskMsg_T* Msg, uint8_t FromISR)
{
	 portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	
	if(FromISR)
	{
		if(pdPASS != xQueueSendToBackFromISR(sAlgo_QueueHandle, Msg, &xHigherPriorityTaskWoken))
		{
			MOD_ALGO_RTT_ERR(0,"Mod_Algo_TaskEventSet Err \r\n");
		}
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
	else
	{
		if(pdPASS != xQueueSendToBack(sAlgo_QueueHandle, Msg, ALGO_TASK_QUEUE_WAIT_TICK))
		{
			MOD_ALGO_RTT_ERR(0,"Mod_Algo_TaskEventSet Err \r\n");
		}
	}
}

void Mod_Algo_TaskCreate(void)
{
    if(pdPASS != xTaskCreate(Mod_Algo_TaskProcess, "AlgoTask", TASK_STACKDEPTH_MOD_ALGO, NULL, TASK_PRIORITY_MOD_ALGO, NULL))
	{
		MOD_ALGO_RTT_ERR(0,"Mod_Algo_TaskCreate Err \r\n");
	}
}










