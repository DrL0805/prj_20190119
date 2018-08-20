
#include "app_lcd.h"

#include "rtos.h"

static QueueHandle_t 	sLcd_QueueHandle;				// 队列句柄
#define		LCD_TASK_QUEUE_LENGTH			3			// 
#define 	LCD_TASK_QUEUE_WAIT_TICK		100			// 队列阻塞时间
#define		LCD_TASK_QUEUE_SIZE				sizeof(App_Lcd_TaskMsg_T)

App_Lcd_Param_t	App_Lcd;

//**********************************************************************
// 函数功能: 背光处理函数
// 输入参数：
// 返回参数：
// 调用：每秒调用一次
void App_Lcd_BacklightProess(void)
{
	App_Lcd.BacklightCnt++;
}

void App_Lcd_BacklightSet(uint32_t Cnt)
{
	App_Lcd.BacklightCnt = Cnt;
}

uint32_t App_Lcd_BacklightGet(void)
{
	return App_Lcd.BacklightCnt;
}

static void App_Lcd_TaskProcess(void *pvParameters)
{
	App_Lcd_TaskMsg_T	Msg;

	// 创建消息队列
	sLcd_QueueHandle = xQueueCreate(LCD_TASK_QUEUE_LENGTH, LCD_TASK_QUEUE_SIZE);
	if(sLcd_QueueHandle == NULL)
	{
		APP_LCD_RTT_ERR(0,"Lcd_Queue Create Err \r\n");
	}
	
	APP_LCD_RTT_LOG(0,"App_Lcd_TaskCreate Suc \r\n");

	while(1)
	{
		#if 1
		if(xQueueReceive(sLcd_QueueHandle, &Msg, portMAX_DELAY))
		{
			switch (Msg.Id)
            {
            	case eAppLcdEventOuter:
					APP_LCD_RTT_LOG(0,"Lcd Msg eAppLcdEventOuter \r\n");
            		break;
            	case eAppLcdEventInside:
					APP_LCD_RTT_LOG(0,"Lcd Msg eAppLcdEventOuter \r\n");
            		break;
            	default:
					APP_LCD_RTT_LOG(0,"Lcd Msg Err \r\n");
            		break;
            }
		}
		#endif
		
		#if 0
		Mid_Lcd_Test();
		vTaskDelay(pdMS_TO_TICKS(1000));
		#endif
	}
}

void App_Lcd_TaskEventSet(App_Lcd_TaskMsg_T* Msg, uint8_t FromISR)
{
	 portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	
	if(FromISR)
	{
		if(pdPASS != xQueueSendToBackFromISR(sLcd_QueueHandle, Msg, &xHigherPriorityTaskWoken))
		{
			APP_LCD_RTT_ERR(0,"App_Lcd_TaskEventSet Err \r\n");
		}
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
	else
	{
		if(pdPASS != xQueueSendToBack(sLcd_QueueHandle, Msg, LCD_TASK_QUEUE_WAIT_TICK))
		{
			APP_LCD_RTT_ERR(0,"App_Lcd_TaskEventSet Err \r\n");
		}
	}
}

void App_Lcd_TaskCreate(void)
{
    if(pdPASS != xTaskCreate(App_Lcd_TaskProcess, "LcdTask", TASK_STACKDEPTH_APP_LCD, NULL, TASK_PRIORITY_APP_LCD, NULL))
	{
		APP_LCD_RTT_ERR(0,"App_Lcd_TaskCreate Err \r\n");
	}
}










