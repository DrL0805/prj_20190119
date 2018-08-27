
#include "app_lcd.h"

#include "rtos.h"

static QueueHandle_t 	sLcd_QueueHandle;				// 队列句柄
#define		LCD_TASK_QUEUE_LENGTH			3			// 
#define 	LCD_TASK_QUEUE_WAIT_TICK		100			// 队列阻塞时间
#define		LCD_TASK_QUEUE_SIZE				sizeof(App_Lcd_TaskMsg_T)

static TimerHandle_t 	sLcd_TimerHandle;	

App_Lcd_Param_t	App_Lcd;

static void sLcd_TimerCallBack(TimerHandle_t xTimer)
{
	App_Lcd_TaskMsg_T LcdMsg;
	
	LcdMsg.Id = eAppLcdEventInside;
	App_Lcd_TaskEventSet(&LcdMsg, 1);
}

void App_Lcd_TimerStart(uint32_t TimerMs)
{
	// 更新定时器参数并启动
//	xTimerStart(sLcd_TimerHandle,0);
	xTimerChangePeriod(sLcd_TimerHandle, pdMS_TO_TICKS(TimerMs), 3);
}

void App_Lcd_TimerStop(void)
{
	xTimerStop(sLcd_TimerHandle,3);
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
	
	// 创建定时器
    sLcd_TimerHandle=xTimerCreate((const char*		)"sLcd_Timer",
									    (TickType_t			)(200/portTICK_PERIOD_MS),
							            (UBaseType_t		)pdFALSE,
							            (void*				)1,
							            (TimerCallbackFunction_t)sLcd_TimerCallBack); //周期定时器，周期1s(1000个时钟节拍)，周期模式	
	
	APP_LCD_RTT_LOG(0,"App_Lcd_TaskCreate Suc \r\n");

	while(1)
	{
		#if 1
		if(xQueueReceive(sLcd_QueueHandle, &Msg, portMAX_DELAY))
		{
			switch (Msg.Id)
            {
            	case eAppLcdEventOuter:		// 外部事件，如key、触摸等
//					APP_LCD_RTT_LOG(0,"eAppLcdEventOuter \r\n");
					App_Lcd_TimerStart(1000);
					Mid_Lcd_Test();
            		break;
            	case eAppLcdEventInside:	// 内事件，如LCD定时器等
					Mid_Lcd_Test();
					APP_LCD_RTT_LOG(0,"eAppLcdEventInside \r\n");
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










