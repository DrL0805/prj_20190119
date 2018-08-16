
#include "mod_flash.h"

#include "rtos.h"

static QueueHandle_t 	sFlash_QueueHandle;				// 队列句柄
#define		FLASH_TASK_QUEUE_LENGTH			3			// 
#define 	FLASH_TASK_QUEUE_WAIT_TICK		100			// 队列阻塞时间
#define		FLASH_TASK_QUEUE_SIZE				sizeof(Mod_Flash_TaskMsg_T)

// ***********************************************************************
//	以下是任务调度代码
// ***********************************************************************
static void Mod_Flash_TaskProcess(void *pvParameters)
{
	Mod_Flash_TaskMsg_T	Msg;

	// 创建消息队列
	sFlash_QueueHandle = xQueueCreate(FLASH_TASK_QUEUE_LENGTH, FLASH_TASK_QUEUE_SIZE);
	if(sFlash_QueueHandle == NULL)
	{
		MOD_FLASH_RTT_ERR(0,"Flash_Queue Create Err \r\n");
	}
	
	MOD_FLASH_RTT_LOG(0,"Mod_Flash_TaskCreate Suc \r\n");

	while(1)
	{
		if(xQueueReceive(sFlash_QueueHandle, &Msg, portMAX_DELAY))
		{

		}
	}
}

void Mod_Flash_TaskEventSet(Mod_Flash_TaskMsg_T* Msg, uint8_t FromISR)
{
	 portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	
	if(FromISR)
	{
		if(pdPASS != xQueueSendToBackFromISR(sFlash_QueueHandle, Msg, &xHigherPriorityTaskWoken))
		{
			MOD_FLASH_RTT_ERR(0,"Mod_Flash_TaskEventSet Err \r\n");
		}
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
	else
	{
		if(pdPASS != xQueueSendToBack(sFlash_QueueHandle, Msg, FLASH_TASK_QUEUE_WAIT_TICK))
		{
			MOD_FLASH_RTT_ERR(0,"Mod_Flash_TaskEventSet Err \r\n");
		}
	}
}

void Mod_Flash_TaskCreate(void)
{
    if(pdPASS != xTaskCreate(Mod_Flash_TaskProcess, "FlashTask", TASK_STACKDEPTH_MOD_FLASH, NULL, TASK_PRIORITY_MOD_FLASH, NULL))
	{
		MOD_FLASH_RTT_ERR(0,"Mod_Flash_TaskCreate Err \r\n");
	}
}










