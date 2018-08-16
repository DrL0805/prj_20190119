/**********************************************************************
**
**模块说明: mid层陀螺仪接口
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.25  修改流程  ZSL  
**
**********************************************************************/
#include "rtos.h"
#include "mid_gyroscope.h"
#include "drv_mtimer.h"
#include "mid_scheduler.h"

// DEBUG开关 **************************************************************
#define MID_GYRO_RTT_DEBUG_ON	1
#if MID_GYRO_RTT_DEBUG_ON
#define MID_GYRO_RTT_printf	SEGGER_RTT_printf
#else
#define MID_GYRO_RTT_printf(...)
#endif

// 内部变量 **************************************************************
MID_GYRO_PARA_T	MID_GYRO;

//**********************************************************************
// 函数功能：	采样定时器超时回调函数
// 输入参数：	
// 返回参数：	
static void Mid_Accel_IsrCb(void)
{
	Mid_Schd_TaskMsg_T Msg;
	Msg.Id = eSchdTaskMsgGyro;
	
	Mid_Schd_TaskEventSet(&Msg, 1);	
}

//**********************************************************************
// 函数功能：	根据当前采样率，返回采样周期
// 输入参数：	SampleRate 采样率
// 返回参数：	采样周期，单位ms
//**********************************************************************
static uint16_t Mid_Gyro_GetSamplePeriod(bsp_gyroscope_osr SampleRate)
{
	uint16_t TmpSamplePeriod;	
	
	switch (SampleRate)
	{
		case GYO_SAMPLERATE_NONE   :
			TmpSamplePeriod = 0;
			break;
		case GYO_SAMPLERATE_25HZ :	
			TmpSamplePeriod = 40;
			break;
		case GYO_SAMPLERATE_50HZ :	
			TmpSamplePeriod = 20;
			break;
		case GYO_SAMPLERATE_100HZ :	
			TmpSamplePeriod = 10;
			break;
		case GYO_SAMPLERATE_200HZ :	
		case GYO_SAMPLERATE_400HZ :	
		case GYO_SAMPLERATE_800HZ   :	
		case GYO_SAMPLERATE_1600HZ  :	
		case GYO_SAMPLERATE_3200HZ  :	
			TmpSamplePeriod = 5;
			break;
		default:
			TmpSamplePeriod = 0;
			break;
	}
	
	return TmpSamplePeriod;
}

//**********************************************************************
// 函数功能：	获取驱动层采样率参数
// 输入参数：	Rate 中间层采样率枚举参数
// 返回参数：	驱动层采用率枚举参数
static bsp_gyroscope_osr Mid_Gyro_DrvRateGet(eMidGyroSampleRate Rate)
{
	bsp_gyroscope_osr RetVal;
	
	switch(Rate)
	{
		case eMidGyroSampleRate1HZ:
		case eMidGyroSampleRate2HZ:
		case eMidGyroSampleRate25HZ:
			RetVal = GYO_SAMPLERATE_25HZ;
			break;
		case eMidGyroSampleRate50HZ:
			RetVal = GYO_SAMPLERATE_50HZ;
			break;
		case eMidGyroSampleRate100HZ:
			RetVal = GYO_SAMPLERATE_100HZ;
			break;
		case eMidGyroSampleRate200HZ:
			RetVal = GYO_SAMPLERATE_200HZ;
			break;
		default:
			RetVal = GYO_SAMPLERATE_25HZ;
			break;
	}
	
	return RetVal;	
}

//**********************************************************************
// 函数功能：	获取驱动层采样参数
// 输入参数：	Rate 中间层采样范围枚举参数
// 返回参数：	驱动层采用范围枚举参数
static bsp_gyroscope_scalerange Mid_Gyro_DrvRangeGet(eMidGyroSampleRange Range)
{
	bsp_gyroscope_scalerange RetVal;
	
	switch(Range)
	{
		case eMidGyroSampleRange125S:
			RetVal = GYRO_SCALE_RANGE_125_DEG_SEC;
			break;
		case eMidGyroSampleRange250S:
			RetVal = GYRO_SCALE_RANGE_250_DEG_SEC;
			break;
		case eMidGyroSampleRange500S:
			RetVal = GYRO_SCALE_RANGE_500_DEG_SEC;
			break;
		case eMidGyroSampleRange1000S:
			RetVal = GYRO_SCALE_RANGE_1000_DEG_SEC;
			break;
		case eMidGyroSampleRange2000S:
			RetVal = GYRO_SCALE_RANGE_2000_DEG_SEC;
			break;
		default:
			RetVal = GYRO_SCALE_RANGE_500_DEG_SEC;
			break;
	}
	
	return RetVal;
}

//**********************************************************************
// 函数功能：	初始化硬件传感器，并设置可定时读取的回调函数
// 输入参数：	IsrCb: 传感器达到可设置的读取时间回调函数
// 返回参数：	无
//**********************************************************************
void Mid_Gyro_Init(void)
{
	// 硬件初始并休眠
	Drv_Gyro_GoSleep();
	
	// 默认参数初始化
	MID_GYRO.SampleRate = eMidGyroSampleRate25HZ;
	MID_GYRO.SampleRange = eMidGyroSampleRange500S;
	MID_GYRO.SamplePeriod = 1000 / MID_GYRO.SampleRate;	
	MID_GYRO.InitedFlg = true;

	// 创建采样定时器
//	Drv_MTimer_Create(&MID_GYRO.MTiemrId, MID_GYRO.SamplePeriod, Mid_Accel_IsrCb);
}

//**********************************************************************
// 函数功能：	参数设置
// 输入参数：	Rate 采样率
//				Range 采样周期
// 返回参数：	无
void Mid_Gyro_ParamSet(eMidGyroSampleRate Rate, eMidGyroSampleRange Range)
{
	MID_GYRO.SampleRate = Rate;
	MID_GYRO.SampleRange = Range;
	MID_GYRO.SamplePeriod = 1000 / MID_GYRO.SampleRate;		
}

//**********************************************************************
// 函数功能：	开始采样
// 输入参数：	采样周期和采样范围
// 返回参数：	
void Mid_Gyro_StartSample(void)
{
	// 硬件配置
	Drv_Gyro_Set(Mid_Gyro_DrvRateGet(MID_GYRO.SampleRate), Mid_Gyro_DrvRangeGet(MID_GYRO.SampleRange));
	
	// 更新采样定时器参数并启动
//	Drv_MTimer_Stop(MID_GYRO.MTiemrId);	
//	Drv_MTimer_Start(MID_GYRO.MTiemrId, MID_GYRO.SamplePeriod);
	
	MID_GYRO.SamplingFlg = true;	
}

//**********************************************************************
// 函数功能：	停止采样
// 输入参数：	
// 返回参数：	
void Mid_Gyro_StopSample(void)
{
	// 停止硬件采样
	Drv_Gyro_GoSleep();
	
	// 停止采样定时器
//	Drv_MTimer_Stop(MID_GYRO.MTiemrId);	
	
	MID_GYRO.SamplingFlg = false;
}

//**********************************************************************
// 函数功能：	读取一次硬件数据并更新，等待有需要的外设来获取
// 输入参数：	
// 返回参数：
void Mid_Gyro_DataUpdate(void)
{
	Drv_Gyro_Read(MID_GYRO.LatestData);
}

//**********************************************************************
// 函数功能：	获取所有配置参数
// 输入参数：	无
// 返回参数：	无
void Mid_Gyro_ParamGet(MID_GYRO_PARA_T* MID_GYRO_PARA)
{
	memcpy(MID_GYRO_PARA, &MID_GYRO, sizeof(MID_GYRO_PARA_T));
}

//**********************************************************************
// 函数功能：	读取当前加速度的最新值
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
void Mid_Gyro_DataRead(int16 data[3])
{
	data[0]		= MID_GYRO.LatestData[0];
	data[1]		= MID_GYRO.LatestData[1];
	data[2]		= MID_GYRO.LatestData[2];
}

//**********************************************************************
// 函数功能：	传感器自检,调用该函数时，确保该资源未使用
// 输入参数：	无
// 返回参数：	0x00:成功
// 				0xff:失败
//**********************************************************************
uint16 Mid_Gyro_SelfTest(void)
{
	return Drv_Gyro_SelfTest();
}

// ***********************************************************************
//	以下是任务调度代码
// ***********************************************************************
// 采样定时器回调函数
#if 0
static void Gyro_TimerCallback(TimerHandle_t xTimer)
{
	 // 发送任务信号量，通知Task可读取新的数据
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	vTaskNotifyGiveFromISR(Gyro_TaskHandle, &xHigherPriorityTaskWoken);

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);	
}

static void GyroTask_Process(void *pvParameters)
{	
	// 创建采样定时器
    Gyro_TimerHandle=xTimerCreate(
					(const char*		)"Gyro_TimerHandle",
					(TickType_t			)(Gyro_TimerPeriod),
					(UBaseType_t		)pdTRUE,
					(void*				)Gyro_TimerID,
					(TimerCallbackFunction_t)Gyro_TimerCallback); //周期定时器，周期1s(1000个时钟节拍)，周期模式	
	if(Gyro_TimerHandle == NULL)
	{
		MID_GYRO_RTT_printf(0,"Gyro_Timer Create Err \r\n");
	}	
					
	while(1)
	{
		// 等待任务信号量
		ulTaskNotifyTake(pdFALSE, portMAX_DELAY);			
		
		// 读取最新数据，等待处理
		Drv_Gyro_Read(MID_GYRO.LatestData);

		MID_GYRO_RTT_printf(0,"Gyro: %d, %d, %d \r\n",MID_GYRO.LatestData[0],MID_GYRO.LatestData[1],MID_GYRO.LatestData[2]);
		
	}
}

void GyroTask_Create(void)
{
    if(pdPASS != xTaskCreate(GyroTask_Process, "GyroTask", TASK_STACKDEPTH_GYRO, NULL, TASK_PRIORITY_GYRO, &Gyro_TaskHandle))
	{
		MID_GYRO_RTT_printf(0,"GyroTask_Create Err \r\n");
	}
}
#endif




