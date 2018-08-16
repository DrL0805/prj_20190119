
#include "mod_power.h"

#include "rtos.h"

static QueueHandle_t 	sPwr_QueueHandle;				// 队列句柄
#define		PWR_TASK_QUEUE_LENGTH			3			// 
#define 	PWR_TASK_QUEUE_WAIT_TICK		100			// 队列阻塞时间
#define		PWR_TASK_QUEUE_SIZE				sizeof(Mod_Pwr_TaskMsg_T)

// ***********************************************************************
//	以下是任务调度代码
// ***********************************************************************
static void Mod_Pwr_TaskProcess(void *pvParameters)
{
	Mod_Pwr_TaskMsg_T	Msg;

	// 创建消息队列
	sPwr_QueueHandle = xQueueCreate(PWR_TASK_QUEUE_LENGTH, PWR_TASK_QUEUE_SIZE);
	if(sPwr_QueueHandle == NULL)
	{
		MOD_PWR_RTT_ERR(0,"Pwr_Queue Create Err \r\n");
	}
	
	MOD_PWR_RTT_LOG(0,"Mod_Pwr_TaskCreate Suc \r\n");

	while(1)
	{
		if(xQueueReceive(sPwr_QueueHandle, &Msg, portMAX_DELAY))
		{

		}
	}
}

void Mod_Pwr_TaskEventSet(Mod_Pwr_TaskMsg_T* Msg, uint8_t FromISR)
{
	 portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	
	if(FromISR)
	{
		if(pdPASS != xQueueSendToBackFromISR(sPwr_QueueHandle, Msg, &xHigherPriorityTaskWoken))
		{
			MOD_PWR_RTT_ERR(0,"Mod_Pwr_TaskEventSet Err \r\n");
		}
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
	else
	{
		if(pdPASS != xQueueSendToBack(sPwr_QueueHandle, Msg, PWR_TASK_QUEUE_WAIT_TICK))
		{
			MOD_PWR_RTT_ERR(0,"Mod_Pwr_TaskEventSet Err \r\n");
		}
	}
}

void Mod_Pwr_TaskCreate(void)
{
    if(pdPASS != xTaskCreate(Mod_Pwr_TaskProcess, "PwrTask", TASK_STACKDEPTH_MOD_PWR, NULL, TASK_PRIORITY_MOD_PWR, NULL))
	{
		MOD_PWR_RTT_ERR(0,"Mod_Pwr_TaskCreate Err \r\n");
	}
}










