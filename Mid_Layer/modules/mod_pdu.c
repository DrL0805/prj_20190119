
#include "mod_pdu.h"

#include "rtos.h"

static QueueHandle_t 	sPDU_QueueHandle;				// 队列句柄
#define		PDU_TASK_QUEUE_LENGTH			3			// 
#define 	PDU_TASK_QUEUE_WAIT_TICK		100			// 队列阻塞时间
#define		PDU_TASK_QUEUE_SIZE				sizeof(Mod_PDU_TaskMsg_T)

// ***********************************************************************
//	以下是任务调度代码
// ***********************************************************************
static void Mod_PDU_TaskProcess(void *pvParameters)
{
	Mod_PDU_TaskMsg_T	Msg;

	// 创建消息队列
	sPDU_QueueHandle = xQueueCreate(PDU_TASK_QUEUE_LENGTH, PDU_TASK_QUEUE_SIZE);
	if(sPDU_QueueHandle == NULL)
	{
		MOD_PDU_RTT_ERR(0,"PDU_Queue Create Err \r\n");
	}
	
	MOD_PDU_RTT_LOG(0,"Mod_PDU_TaskCreate Suc \r\n");

	while(1)
	{
		if(xQueueReceive(sPDU_QueueHandle, &Msg, portMAX_DELAY))
		{

		}
	}
}

void Mod_PDU_TaskEventSet(Mod_PDU_TaskMsg_T* Msg, uint8_t FromISR)
{
	 portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	
	if(FromISR)
	{
		if(pdPASS != xQueueSendToBackFromISR(sPDU_QueueHandle, Msg, &xHigherPriorityTaskWoken))
		{
			MOD_PDU_RTT_ERR(0,"Mod_PDU_TaskEventSet Err \r\n");
		}
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
	else
	{
		if(pdPASS != xQueueSendToBack(sPDU_QueueHandle, Msg, PDU_TASK_QUEUE_WAIT_TICK))
		{
			MOD_PDU_RTT_ERR(0,"Mod_PDU_TaskEventSet Err \r\n");
		}
	}
}

void Mod_PDU_TaskCreate(void)
{
    if(pdPASS != xTaskCreate(Mod_PDU_TaskProcess, "PDUTask", TASK_STACKDEPTH_MOD_PDU, NULL, TASK_PRIORITY_MOD_PDU, NULL))
	{
		MOD_PDU_RTT_ERR(0,"Mod_PDU_TaskCreate Err \r\n");
	}
}










