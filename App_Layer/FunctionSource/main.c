#include "platform_common.h"
#include "platform_debugcof.h"

#include "system_task.h"

#include "mid_scheduler.h"
#include "drv_touch.h"
#include "module_test.h"
#include "SEGGER_RTT.h"
#include "SEGGER_RTT_Conf.h"

#include "am_util_delay.h"

static void Bsp_Init(void)
{
	Mid_Schd_ParamInit();	// 共享互斥初始化放在前面
	
	Drv_MTimer_Init();		// 放在初始化前面

	Mid_Key_Init();
	Mid_Motor_Init();
//	Mid_Magnetism_Init();
	Mid_Accel_Init();
	Mid_Gyro_Init();
	
//	Drv_RGBLcd_Init();
//	
//	// 开启RTC时间
//	Mid_Rtc_Init();
////	Mid_Rtc_Start();
//	
//	// 测试闹钟
//	alarm_clock_t tAlarm;
//	tAlarm.alarmswitch = 1;
//	tAlarm.reptswitch = 0;
//	tAlarm.hour = 0;
//	tAlarm.min = 2;
//	Mid_AlarmClock_Write(&tAlarm, 3);
//	
//	Mid_StopWatch_Init();	// 秒表
//	Mid_Countdown_Init();	// 倒计时

//	Mid_Font_Init();
	
	Drv_IT7259_Init();
	
//	Mid_NandFlash_Init();
}

static void Task_Init(void)
{
//	UartTask_Create();
	
	// 中间层任务 
	Mid_Schd_TaskCreate();
//	Mid_GPS_TaskCreate();
//	Mid_MIC_TaskCreate();
//	Mid_Ble_TaskCreate();
//	
//	// 模块化任务
//	Mod_Pwr_TaskCreate();
//	Mod_Algo_TaskCreate();
//	Mod_PDU_TaskCreate();
//	Mod_Flash_TaskCreate();
//	Mod_Time_TaskCreate();
//	
//	// 应用层任务
//	App_Win_TaskCreate();
//	App_Lcd_TaskCreate();	
}

static void start_task(void *pvParameters)
{
	/* 初始化过程中禁止中断和任务调度 */ 
	taskENTER_CRITICAL();  	
	
	Bsp_Init();
	
	Task_Init();
	
	taskEXIT_CRITICAL();            //退出临界区
	
	vTaskDelete(NULL); 				//删除开始任务
}

#if 1
int main(void)
{
	/* 系统初始化 */ 
	System_Init();
	SEGGER_RTT_Init();

	
	/* 
		创建开始任务，让外设在OS系统内初始化
		因为有些外设初始化过程中用到OS相关函数
		注：开始任务优先级必须最高
	*/
//    xTaskCreate(start_task, "start_task", 1024, NULL, TASK_PRIORITY_START, NULL); 
	Bsp_Init();
	Task_Init();
	
	SEGGER_RTT_printf(0,"Sys Init Suc \n");
   
	vTaskStartScheduler();
    while(1)
	{
		SEGGER_RTT_printf(0,"Sys Init Fai \n");
	}
}
#endif

#if 0
void App_Protocal_Init(void);

int main(void)     //this case: for module test
{
	/* Use the NVIC offset register to locate the stack. */
    System_Init();
    SEGGER_RTT_Init();

	Mid_Key_Init();
	
	Mid_Schd_TaskCreate();
	
	
	
    //App_Protocal_Init();
    //Mid_Ble_Init();
	 SEGGER_RTT_printf(0,"sys init ok \n");
    xTaskCreate(Module_Test_Main, "test", 512, NULL, TASK_PRIORITY_MIDDLE_LOW, NULL);
    vTaskStartScheduler();
    while(1);
}

#endif


#if 0
int main(void)   //this case: for app
{
	/* Use the NVIC offset register to locate the stack. */
    System_Init();
#if(PLATFORM_DEBUG_ENABLE == 1)
    SEGGER_RTT_Init();
#endif

#ifndef AM_FREERTOS_USE_STIMER_FOR_TICK
    //stimer×÷?aosê±?ó?D??￡?os2éó?tickless?￡ê?￡??úidle task?D1üàísleep?°wakeup
    SystemTask_Create();
#endif
#if(SUPPORT_MOVT == 1)  //support movt
    MovtTask_CreateTask();
#endif
	FlashTask_Create();
    MultiModuleTask_Create();
    AppTask_Create(); 
  
    Mid_Ble_Init();

    vTaskStartScheduler();
    
    while(1);
}

#endif

