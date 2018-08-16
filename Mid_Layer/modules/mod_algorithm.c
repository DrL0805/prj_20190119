
#include "mod_algorithm.h"

#include "rtos.h"

static QueueHandle_t 	sAlgo_QueueHandle;				// 队列句柄
#define		ALGO_TASK_QUEUE_LENGTH			3			// 
#define 	ALGO_TASK_QUEUE_WAIT_TICK		100			// 队列阻塞时间
#define		ALGO_TASK_QUEUE_SIZE				sizeof(Mod_Algo_TaskMsg_T)

// ***********************************************************************
//	以下是任务调度代码
// ***********************************************************************
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










