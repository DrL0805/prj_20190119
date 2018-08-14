#include "platform_common.h"
#include "platform_debugcof.h"
/* FreeRTOS includes */
#include "rtos.h"
#include "mid_interface.h"
#include "mid_common.h"
#include "flash_task.h"

static QueueHandle_t flash_task_msg_queue;				/**< Queue handler for flash task */

void * FlashQueueProcess(flash_task_msg_t*	msg);

uint16 FlashTask_EventSet(flash_task_msg_t* msg)
{
	switch(msg->id)
	{
		case EXTFLASH_ID:
		if(Mid_ExtFlash_EventSetPre(&msg->flash.extflashEvent) == 0x00)//分配二值信号量
		{
			if(pdTRUE == xQueueSendToBack(flash_task_msg_queue, msg, 1))//事件入队列，队列执行完回调释放信号量
			{
				msg->flash.extflashEvent.para.result = Mid_ExtFlash_WaitComplete(&msg->flash.extflashEvent);//等待二值信号量释放
				return	msg->flash.extflashEvent.para.result;
			}
			else
			{
				Mid_ExtFlash_EventSetFail(&msg->flash.extflashEvent);
				return 0xff;
			}
		}
		break;

		case FRONT_ID:
		if(Mid_Front_EventSetPre(&msg->flash.frontEvent) == 0x00)
		{
			if(pdTRUE == xQueueSendToBack(flash_task_msg_queue, msg, 1))
			{
				msg->flash.frontEvent.para.result = Mid_Front_WaitComplete(&msg->flash.frontEvent);
				return msg->flash.frontEvent.para.result;
			}
			else
			{
				Mid_Front_EventSetFail(&msg->flash.frontEvent);
				return 0xff;
			}
		}
		break;
	}
	return 0xff;
}

void FlashTask(void *pvParameters)
{
	flash_task_msg_t	msg;

	for(;;)
	{
		if(xQueueReceive(flash_task_msg_queue, &msg, portMAX_DELAY))
		{
			switch(msg.id)
			{				
				case EXTFLASH_ID:
				Mid_ExtFlash_EventProcess(&msg.flash.extflashEvent);
				break;

				case FRONT_ID:
				Mid_Front_EventProcess(&msg.flash.frontEvent);
				break;

				default:
				break;
			}
		}
	}
}

void FlashTask_Create(void)
{
    //8M extflash
    Mid_ExtFlash_Init();
    Mid_Front_Init();

    flash_task_msg_queue = xQueueCreate(FLASH_TASK_QUEUE_SIZE, sizeof(flash_task_msg_t));
    if(flash_task_msg_queue == NULL)
    {
        Err_Info((0,"Create Flash Queue fail\n"));
        while(1);
    }

    xTaskCreate(FlashTask, "FlashTask", 256, NULL, TASK_PRIORITY_MIDDLE, NULL);
}

