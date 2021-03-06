/**********************************************************************
**
**模块说明: 
**   
**
**********************************************************************/

#include "mid_scheduler.h"


#if 0	// 测试引入定时器
#include "mod_time.h"
#include "mid_front.h"
#include "app_lcd.h"
#endif

static QueueHandle_t 	sSchd_QueueHandle;				// 队列句柄
#define		SCHD_TASK_QUEUE_LENGTH			3			// 
#define 	SCHD_TASK_QUEUE_WAIT_TICK		100			// 队列阻塞时间
#define		SCHD_TASK_QUEUE_SIZE			sizeof(Mid_Schd_TaskMsg_T)

// 外部变量
SemaphoreHandle_t	SPI_I2C_M0_SemaphoreHandle;
SemaphoreHandle_t	SPI_I2C_M2_SemaphoreHandle;		// 加速度和角速度

// 获取外设资源
void Mid_Schd_M2MutexTake(void)
{
	xSemaphoreTake(SPI_I2C_M2_SemaphoreHandle, portMAX_DELAY);
}

// 释放外设资源
void Mid_Schd_M2MutexGive(void)
{
	xSemaphoreGive(SPI_I2C_M2_SemaphoreHandle);
}

//**********************************************************************
// 函数功能: 按键调度任务处理函数
// 输入参数：
// 返回参数：
//static font_para_t	FontPram; 
static uint8_t tDotBuf[140];
static uint32_t tTmp;
uint8_t stFlashWriteBuf[2177], stFlashReadBuf[2177];
static void KeyTest(void)
{
	#if 1	// 气压环温	
	MID_SCHD_RTT_LOG(0,"Drv_AirPress_SelfTest %d \r\n", Drv_AirPress_SelfTest());
	#endif
	
	#if 0	// 心率测试
	uint8_t tTouchFlg;
//	MID_SCHD_RTT_LOG(0,"Drv_Hrm_CheckHw %d \r\n", Mid_Hrm_FactoryTest());
	HrmStart();
	vTaskDelay(pdMS_TO_TICKS(50));
	Mid_Hrm_TouchStatusRead(&tTouchFlg);
	MID_SCHD_RTT_LOG(0,"Mid_Hrm_TouchStatusRead %d \r\n", tTouchFlg);
	#endif
	
	#if 0	// 查找手机指令测试
	App_Protocal_FinePhone();
	#endif
	
	#if 0	// 读取身高体重信息
	bodyInfo_s tbodyInfo;
	Mid_SportScene_BodyInfoRead(&tbodyInfo);
	MID_SCHD_RTT_LOG(0,"age %d, bodyHeight %d, bodyWeight %d, sex %d \r\n", tbodyInfo.age,tbodyInfo.bodyHeight,tbodyInfo.bodyWeight,tbodyInfo.sex);
	#endif
	
	#if 0	// 闹钟测试，读取闹钟信息
	alarm_clock_t	tAlarm;
	
	MID_SCHD_RTT_LOG(0,"tAlarm:reptswitch, hour, min, delayswitch, alarmswitch \r\n");
	for(uint32_t i = 0;i < eMidAlarmGroupNum;i++)
	{
		Mid_AlarmClock_Read(&tAlarm, i);
		MID_SCHD_RTT_LOG(0,"tAlarm %02X, %02X, %02X, %02X, %02X\r\n", tAlarm.reptswitch, tAlarm.hour, tAlarm.min, tAlarm.delayswitch, tAlarm.alarmswitch);
	}
	#endif
	
	#if 0	// 获取RTC时间测试
	// 手表主动向APP获取时间接口好像不能用，待和app确认
	App_Protocal_AdjustTimeprocess();
	#endif
	
	#if 0	// 天气场景测试
	Mid_Weahter_Param_t	tWeahter;
	
	App_Protocal_GetWeatherProcess();
	 vTaskDelay(200);
	Mid_WeatherScene_TendencyGet(&tWeahter);
	MID_SCHD_RTT_LOG(0,"tWeahter %04X, %02X, %02X, %02X, \r\n", tWeahter.Status, tWeahter.CurTemperature, tWeahter.MaxTemperature, tWeahter.MinTemperature);
	#endif
	
	#if 0	// NandFlash 测试
	MID_SCHD_RTT_LOG(0,"Mid_NandFlash_SelfTest %02X \r\n", Mid_NandFlash_SelfTest());
	#endif
	
	#if 1	// LCD测试
	App_Lcd_TaskMsg_T	LcdMsg;
	
	LcdMsg.Id = eAppLcdEventOuter;
	App_Lcd_TaskEventSet(&LcdMsg, 0);
	#endif
	
	#if 1	// 字库测试
	// 点阵读取测试
//	font_para_t FontPram;
//	FontPram.code.codeGB = 0xC4BF;		// "目"
//	FontPram.dataAddr = tDotBuf;
//	FontPram.sizeKind = GB_SIZE_16X16;
//	Mid_Font_ReadGB(&FontPram);
//	
//	FontPram.code.codeASCII = 0x41;		// "A"
//	FontPram.dataAddr = tDotBuf;
//	FontPram.sizeKind = ASCII_SIZE_8X16;
//	Mid_Font_ReadASCII(&FontPram);
	
	// 自检函数	
	MID_SCHD_RTT_LOG(0,"Drv_Font_SelfTest %d \r\n", Mid_Font_SelfTest());	
	#endif
	
	#if 0	// 倒计时测试
	Mid_Countdown_TimeWrite(17);
	Mid_Countdown_Start();
	#endif
	
	#if 0	// 秒表测试
	Mid_StopWatch_Param_t	tStopWatchParam;
	Mid_StopWatch_Format_t	sStopWatchFormat;
	uint32_t	tStopWatchTotalMs = 0;
	
	Mid_StopWatch_ParamGet(&tStopWatchParam);
	
	if(!tStopWatchParam.RuningFlg)
	{
		Mid_StopWatch_Start();
		MID_SCHD_RTT_LOG(0,"Mid_StopWatch_Start \r\n");
	}
	else
	{
		Mid_StopWatch_MeasurePoint();	// 测量点
		Mid_StopWatch_ParamGet(&tStopWatchParam);	// 获取新参数
		tStopWatchTotalMs = Mid_StopWatch_TotalMsGet(true);	// 带补偿方式获取当前总时间
		
		Mid_StopWatch_FormatSwitch(tStopWatchTotalMs, &sStopWatchFormat);	// 时间格式转换
		
		MID_SCHD_RTT_LOG(0,"tStopWatchTatolMs %d, %d:%d:%d:%d \r\n", 
			tStopWatchTotalMs, sStopWatchFormat.hour, sStopWatchFormat.min, sStopWatchFormat.sec, sStopWatchFormat.ms);
		
		for(uint32_t i = 0; i < tStopWatchParam.MeasureCnt; i++)
		{
			MID_SCHD_RTT_LOG(0,"%d ", tStopWatchParam.MeasurePoint[i]);
		}MID_SCHD_RTT_LOG(0,"\r\n");
	}	
	#endif
	
	#if 1	// 
//	Mod_Time_TaskMsg_T TimeMsg;
//	
//	Mod_Time_TaskEventSet(&TimeMsg, 0);
	
//	Mid_Rtc_Start();
	#endif
	
	#if 0	// 马达任务测试
	Mid_Motor_ParamSet(eMidMotorShake4Hz, 5);
	Mid_Motor_ShakeStart();
	#endif
	
	#if 0	// 地磁任务测试
	Mid_Mag_ParamSet(eMidMagSampleRate25HZ, eMidMagSampleRange12GS);
	Mid_Mag_StartSample();
	#endif
	
	#if 0	// 加速度计任务测试
//	MID_SCHD_RTT_LOG(0,"Mid_Accel_SelfTest %02X \r\n", Mid_Accel_SelfTest());
//	Mid_Accel_ParamSet(eMidAccelSampleRate25HZ, eMidAccelSampleRange2G);
//	Mid_Accel_StartSample();
	#endif 
	
	#if 0	// 角速度计任务测试
//	MID_SCHD_RTT_LOG(0,"Mid_Gyro_SelfTest %02X \r\n", Mid_Gyro_SelfTest());
	Mid_Gyro_ParamSet(eMidGyroSampleRate25HZ, eMidGyroSampleRange1000S);
	Mid_Gyro_StartSample();
	#endif
	
	#if 0	// 窗口任务测试
	App_Win_Msg_T Msg;
	Msg.MenuTag = eWinMenukey;
	
	App_Win_TaskEventSet(&Msg);
	#endif
	
	#if 0	// 串口任务测试
	uint8_t TmpUartBuf[10] = {1,2,3,4,5,6,7,8,9,0};
	
	Mid_Uart_Send(UART_MODULE_0,TmpUartBuf,6);
	#endif	
}

static void Mid_Schd_KeyHandler(Mid_Schd_TaskMsg_T* Msg)
{
	MID_SCHD_RTT_LOG(0,"Schd Msg Key %d %d \r\n", Msg->Id, Msg->Param.Key.Val);
	KeyTest();

	App_Win_Msg_T WinMsg;
	
	switch(phoneState.state)
	{
		case PHONE_STATE_NORMAL:
			// 向上层发送按键消息
			WinMsg.MenuTag = eWinMenukey;	
			WinMsg.val = Msg->Param.Key.Val;
			
			App_Win_TaskEventSet(&WinMsg);			
			break;		
		case PHONE_STATE_PHOTO:
			App_Protocal_TakePhoto();
			break;
		case PHONE_STATE_AUTHOR:
			phoneState.state = PHONE_STATE_NORMAL;
			App_Protocal_AuthorPass();			
			break;
		case PHONE_STATE_PAIRE:
			break;
		case PHONE_STATE_HRM:
			break;
		default: break;
	}
}

static void Mid_Schd_AccelHandler(Mid_Schd_TaskMsg_T* Msg)
{
	// 读取并更新一次传感器数据，通知其他外设需要自取
	Mid_Accel_DataUpdate();	
	
	// 通知算法模块
	Mod_Algo_TaskMsg_T 	tAlgoMsg;
	
	tAlgoMsg.Id = eAlgoTaskMsgAccel;
	Mod_Algo_TaskEventSet(&tAlgoMsg, 0);

//	int16_t	tData[3];
//	Mid_Accel_DataRead(tData);
//	MID_SCHD_RTT_LOG(0,"Accel: %d, %d, %d \r\n",tData[0],tData[1],tData[2]);
}

static void Mid_Schd_GyroHandler(Mid_Schd_TaskMsg_T* Msg)
{
	int16_t	tData[3];
	
	// 读取并更新一次传感器数据，通知其他外设需要自取
	Mid_Gyro_DataUpdate();	
	
	Mid_Gyro_DataRead(tData);
	MID_SCHD_RTT_LOG(0,"Gyro: %d, %d, %d \r\n",tData[0],tData[1],tData[2]);
}

static void Mid_Schd_MagHandler(Mid_Schd_TaskMsg_T* Msg)
{
//	int16_t	tData[3];
//	
//	// 读取并更新一次传感器数据，通知其他外设需要自取
//	Mid_Mag_DataUpdate();	
//	
//	Mid_Mag_DataRead(tData);
//	MID_SCHD_RTT_LOG(0,"Gyro: %d, %d, %d \r\n",tData[0],tData[1],tData[2]);
}

static void Mid_Schd_UartHandler(Mid_Schd_TaskMsg_T* Msg)
{
    uint16 u16ReadLen = 0, u16LenWritten;
    uint8 u8Buffer[128];

	SMDrv_UART_ReadBytes(BASE_UART_MODULE,u8Buffer,32,&u16ReadLen);
//	SMDrv_UART_WriteBytes(BASE_UART_MODULE,u8Buffer,u16ReadLen,&u16LenWritten);
//	for(uint32_t i = 0;i < u16ReadLen;i++)
//	{
//		MID_SCHD_RTT_LOG(0,"%02X ", u8Buffer[i]);
//	}MID_SCHD_RTT_LOG(0,"\r\n");
	
	// 串口模拟按键、触摸等，调试windows切换逻辑
	App_Win_Msg_T WinMsg;
	WinMsg.MenuTag = u8Buffer[0];	// 模拟菜单类型
	WinMsg.val = u8Buffer[1];		// 模拟菜单值
	App_Win_TaskEventSet(&WinMsg);
}

void Mid_Schd_ParamInit(void)
{
	// 外设访问互斥信号量创建
	SPI_I2C_M0_SemaphoreHandle   = xSemaphoreCreateMutex();	
	SPI_I2C_M2_SemaphoreHandle   = xSemaphoreCreateMutex();
}

//**********************************************************************
// 函数功能: 中间层调度任务处理函数
// 输入参数：	
// 返回参数：
static void Mid_Schd_TaskProcess(void *pvParameters)
{
	Mid_Schd_TaskMsg_T	Msg;
	
	sSchd_QueueHandle = xQueueCreate(SCHD_TASK_QUEUE_LENGTH, SCHD_TASK_QUEUE_SIZE);
	if(sSchd_QueueHandle == NULL)
	{
		MID_SCHD_RTT_ERR(0,"Schd Queue Create Err \r\n");
	}
	
	MID_SCHD_RTT_LOG(0,"Mid_Schd_TaskCreate Suc \r\n");
	
	while(1)
	{
		if(xQueueReceive(sSchd_QueueHandle, &Msg, portMAX_DELAY))
		{
			
			switch(Msg.Id)
			{
				case eSchdTaskMsgKey:
					Mid_Schd_KeyHandler(&Msg);
					break;
				case eSchdTaskMsgAccel:
					Mid_Schd_AccelHandler(&Msg);
					break;
				case eSchdTaskMsgGyro:
					Mid_Schd_GyroHandler(&Msg);
					break;
				case eSchdTaskMsgMagnetism:
					Mid_Schd_MagHandler(&Msg);
					break;
				case eSchdTaskMsgUart:
					Mid_Schd_UartHandler(&Msg);
					break;
				default:
					MID_SCHD_RTT_WARN(0,"Schd Msg Err %d \r\n", Msg.Id);
					break;
			}
		}
	}
}

//**********************************************************************
// 函数功能:	
// 输入参数：
// 返回参数：
void Mid_Schd_TaskEventSet(Mid_Schd_TaskMsg_T* Msg, uint8_t FromISR)
{
	 portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	
	/* 若中断函数中调用，需判断是否需要进行任务调度 */
	if(FromISR)
	{
		if(pdPASS != xQueueSendToBackFromISR(sSchd_QueueHandle, Msg, &xHigherPriorityTaskWoken))
		{
			MID_SCHD_RTT_ERR(0,"Mid_Schd_TaskEventSet Err \r\n");
		}
		
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
	else
	{
		if(pdPASS != xQueueSendToBack(sSchd_QueueHandle, Msg, SCHD_TASK_QUEUE_WAIT_TICK))
		{
			MID_SCHD_RTT_ERR(0,"Mid_Schd_TaskEventSet Err \r\n");
		}		
	}	
}

void Mid_Schd_TaskCreate(void)
{
    xTaskCreate(Mid_Schd_TaskProcess, "SchdTask", TASK_STACKDEPTH_MID_SCHD, NULL, TASK_PRIORITY_MID_SCHD, NULL);
}



