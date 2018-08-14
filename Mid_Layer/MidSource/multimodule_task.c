#include "platform_common.h"
#include "mid_interface.h"
#include "mid_common.h"
#include "multimodule_task.h"
#include "drv_mtimer.h"



/*******************macro define*******************/
#define 	SINGOLE_PORT_FRE 					32
#define		MULTIMODULE_TASK_QUEUE_SIZE			32


/*******************variable define*******************/
//multimodule queue
static QueueHandle_t		multimodule_task_msg_queue;				/**< Queue handler for multiModule task */

//单口驱动系统定时器
TimerHandle_t PortPeriodTimer[3];



/*******************function define*******************/

//**********************************************************************
// 函数功能:    
// 输入参数：    
// 返回参数： 
void Uart0AppIsrCb(uint16 isrHandle)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    multimodule_task_msg_t msg;

    msg.id                                  = UARTAPP_ID;
    msg.module.uartappEvent.id              = isrHandle;
    msg.module.uartappEvent.uartmodule      = 0;

    if (errQUEUE_FULL == xQueueSendToBackFromISR( multimodule_task_msg_queue, (void *) &msg, &xHigherPriorityTaskWoken ))
    {
                // ("errQUEUE_FULL");
    }
    if( pdTRUE ==  xHigherPriorityTaskWoken )
    {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

//**********************************************************************
// 函数功能:    
// 输入参数：    
// 返回参数： 
void Uart1AppIsrCb(uint16 isrHandle)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    multimodule_task_msg_t msg;

    msg.id                                  = UARTAPP_ID;
    msg.module.uartappEvent.id              = isrHandle;
    msg.module.uartappEvent.uartmodule      = 1;

    if (errQUEUE_FULL == xQueueSendToBackFromISR( multimodule_task_msg_queue, (void *) &msg, &xHigherPriorityTaskWoken ))
    {
                // ("errQUEUE_FULL");
    }
    if( pdTRUE ==  xHigherPriorityTaskWoken )
    {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

//**********************************************************************
// 函数功能:    
// 输入参数：    
// 返回参数： 
//**********************************************************************
void SinglePortFinish(uint16 portType)
{
	xTimerStop(PortPeriodTimer[portType], 0);
}

//**********************************************************************
// 函数功能: Green LED周期性单口驱动 
// 输入参数：    
// 返回参数：  
void GreenLedPeriodProcess(TimerHandle_t xTimer)
{
    Mid_SinglePort_PeriodProcessGreenLed();
}

//**********************************************************************
// 函数功能: Red LED周期性单口驱动 
// 输入参数：    
// 返回参数：  
void RedLedPeriodProcess(TimerHandle_t xTimer)
{
    Mid_SinglePort_PeriodProcessRedLed();
}

//**********************************************************************
// 函数功能: 马达周期性单口驱动 
// 输入参数：    
// 返回参数：  
void MotoPeriodProcess(TimerHandle_t xTimer)
{
    Mid_SinglePort_PeriodProcessMoto();
}

//**********************************************************************
// 函数功能:    背光周期性单口驱动
// 输入参数：    
// 返回参数：  
void BackLightPeriodProcess(TimerHandle_t xTimer)
{
    Mid_SinglePort_PeriodProcessBackLight();
}
//**********************************************************************
// 函数功能: 重力传感器定时中断回调处理  
// 输入参数：    
// 返回参数：   
void AccelReadDataIsrProcess(void)
{	
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    multimodule_task_msg_t msg;

    msg.id									= ACCEL_ID;
	msg.module.accelEvent.id				= ACCEL_READ_PROCESS;

    if (errQUEUE_FULL == xQueueSendToBackFromISR( multimodule_task_msg_queue, (void *) &msg, &xHigherPriorityTaskWoken ))
    {
                // ("errQUEUE_FULL");
    }
    if( pdTRUE ==  xHigherPriorityTaskWoken )
    {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

//**********************************************************************
// 函数功能: 陀磥仪传感器定时中断回调处理    
// 输入参数：    
// 返回参数：   
void GyroReadDataIsrProcess(void)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    multimodule_task_msg_t msg;

    msg.id									 = GYRO_ID;
	  msg.module.accelEvent.id = GYRO_READ_PROCESS;

    if (errQUEUE_FULL == xQueueSendToBackFromISR( multimodule_task_msg_queue, (void *) &msg, &xHigherPriorityTaskWoken ))
    {
                // ("errQUEUE_FULL");
    }
    if( pdTRUE ==  xHigherPriorityTaskWoken )
    {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

//**********************************************************************
// 函数功能: 地磁传感器定时中断回调处理    
// 输入参数：    
// 返回参数：   
void MagReadDataIsrProcess(void)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    multimodule_task_msg_t msg;

    msg.id									= MAG_ID;
	msg.module.accelEvent.id				= MAG_READ_PROCESS;

    if (errQUEUE_FULL == xQueueSendToBackFromISR( multimodule_task_msg_queue, (void *) &msg, &xHigherPriorityTaskWoken ))
    {
                // ("errQUEUE_FULL");
    }
    if( pdTRUE ==  xHigherPriorityTaskWoken )
    {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

//**********************************************************************
// 函数功能:  压力传感器功能转换周期性处理  
// 输入参数：    
// 返回参数：   
void PressruePeriodProcess(TimerHandle_t xTimer)
{
	multimodule_task_msg_t msg;
	
	msg.id									= PRESSURE_ID;
	msg.module.pressureEvent.id				= PRESSURE_DATA_PROCESS;
	xQueueSendToBack(multimodule_task_msg_queue, &msg, 1);
}


//**********************************************************************
// 函数功能:  心率传感器数据准备好中断回调处理  
// 输入参数：    
// 返回参数：   
void HrmDataReadyIsrCb(void)
{
   portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
   multimodule_task_msg_t msg;

   msg.id                              = HEARTRATE_ID;
   msg.module.hrmEvent.id              = HRM_CALCULATE;

   if (errQUEUE_FULL == xQueueSendToBackFromISR( multimodule_task_msg_queue, (void *) &msg, &xHigherPriorityTaskWoken ))
   {
               // ("errQUEUE_FULL");
   }
   if( pdTRUE ==  xHigherPriorityTaskWoken )
   {
       portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
   }
}

//**********************************************************************
// 函数功能:  心率传感器触摸中断回调处理——暂不用到
// 输入参数：    
// 返回参数：   
void HrmTouchIsrCb(void)
{
#if 0
   portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
   multimodule_task_msg_t msg;

   msg.id                              	= HEARTRATE_ID;
   msg.module.hrmEvent.id              	= HRM_TOUCH;

   if (errQUEUE_FULL == xQueueSendToFrontFromISR( multimodule_task_msg_queue, (void *) &msg, &xHigherPriorityTaskWoken ))
   {
               // ("errQUEUE_FULL");
   }
   if( pdTRUE ==  xHigherPriorityTaskWoken )
   {
       portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
   }
#endif
}

//**********************************************************************
// 函数功能:    
// 输入参数：    
// 返回参数： 
uint16 MultiModuleTask_EventSet(multimodule_task_msg_t msg)
{
	xQueueSendToBack(multimodule_task_msg_queue, &msg, 1);
	return 0;
}

//**********************************************************************
// 任务功能:    
// 输入参数：    
// 返回参数：
void MultiModuleTask(void *pvParameters)
{
	static multimodule_task_msg_t msg;
  movt_task_msg_t movtMsg;

	//key
	Mid_Key_Init();
	
	//single port
	Mid_SinglePort_Init();
	Mid_SinglePort_SetCallBack(SinglePortFinish);
	PortPeriodTimer[RED_LED]      = xTimerCreate("R_T", APP_1SEC_TICK / SINGOLE_PORT_FRE, pdTRUE, 0, RedLedPeriodProcess);	
	PortPeriodTimer[GREEN_LED]    = xTimerCreate("G_T", APP_1SEC_TICK / SINGOLE_PORT_FRE, pdTRUE, 0, GreenLedPeriodProcess);	
	PortPeriodTimer[MOTO]         = xTimerCreate("Moto_T", APP_1SEC_TICK / SINGOLE_PORT_FRE, pdTRUE, 0, MotoPeriodProcess);	

	//	multitimer init
    Drv_MultiTimer_Init();
	
	//heartrate
	Mid_Hrm_Init(HrmDataReadyIsrCb,HrmTouchIsrCb);

	//pressure
	Mid_Pressure_Init(PressruePeriodProcess);
	
	//weather
	Mid_WeatherScene_Init();
	
	// accel init
	Mid_Accel_Init(AccelReadDataIsrProcess);

	// gyo init
    Mid_Gyro_Init(GyroReadDataIsrProcess);

	//mag init
	Mid_Magnetism_Init(MagReadDataIsrProcess);

	// compass Init
	Mid_Compass_Init();
	//oled
	Mid_Screen_Enable();
    Mid_Screen_SoftInit();

   // uart without flow Init
	Mid_Uart_Init(0,Uart0AppIsrCb);
	Mid_Uart_Init(1,Uart1AppIsrCb);
	
	multimodule_task_msg_queue = xQueueCreate(MULTIMODULE_TASK_QUEUE_SIZE, sizeof(multimodule_task_msg_t));

	vTaskDelay(2);
    
	for(;;)
	{
		if(xQueueReceive(multimodule_task_msg_queue, &msg, portMAX_DELAY))
		{
			switch(msg.id)
			{
				//单口
				case SINGLE_PORT_ID:	
				Mid_SinglePort_SetPara(&(msg.module.singlePortPara));
				xTimerStart(PortPeriodTimer[msg.module.singlePortPara.portType], 1);	
				break;

				case STOPWATCH_ID:
				//null
				break;

				case BAT_ID:
				Mid_Bat_EventProcess(&msg.module.batEvent);
				break;

				//重力传感器 
				case ACCEL_ID:
				Mid_Accel_EventProcess(&msg.module.accelEvent);
				break;
				//地磁传感器
                case MAG_ID:
				Mid_Magnetism_EventProcess(&msg.module.magEvent);
                break;
				
				//陀螺仪传感器 
                case GYRO_ID:
				Mid_Gyro_EventProcess(&msg.module.gyroEvent);
                break;

				//压力传感器
				case PRESSURE_ID:
				Mid_Pressure_EventProcess(&msg.module.pressureEvent);
                break;                            
						
				case RTC_ID:
				break;

				//心率传感器
				case HEARTRATE_ID:
				Mid_Hrm_EventProcess(&msg.module.hrmEvent);
				break;

				//指南针
				case COMPASS_ID:
				Mid_Compass_EventProcess(&msg.module.compassEvent);
				break;
				//心率存储
				case HEARTRATE_LOG_ID:
				Mid_HeartRateScene_EventProcess(&msg.module.hrmLogEvent);
				break;

				//运动 
				case SPORT_ID:
				Mid_SportScene_EventProcess(&msg.module.sportEvent);
				break;

				//天气
				case WEATHER_ID:
				Mid_WeatherScene_EventProcess(&msg.module.weatherEvent);
				break;

				case UARTAPP_ID:
        Mid_Uart_EnvetProcess(&msg.module.uartEvent);
        break;

        case CLIMB_ID:
        Mid_ClimbScene_EventProcess(&msg.module.climbEvent);
        break;
        
        case GESTURE_ID:
        Mid_GestureScene_EventProcess(&msg.module.gestureEvent);
        break;

        case SLEEP_ID:
        Mid_SleepScene_EventProcess(&msg.module.sleepEvent);
        break;

        case USUAL_ID:
        Mid_UsualScene_EventProcess(&msg.module.usualEvent);
        break;

        case SCENE_ID:
        Mid_Scene_EventProcess(&msg.module.sceneEvent);
        break;

        default:break;

			}
		}	
	}	
}


void MultiModuleTask_Create(void)
{
	xTaskCreate(MultiModuleTask, "MultiModuleTask", 1024, NULL, TASK_PRIORITY_MIDDLE, NULL);
}










