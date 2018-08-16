/**********************************************************************
**
**模块说明: mid层加速计接口
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.25  修改流程  ZSL  
**
**********************************************************************/
/* FreeRTOS includes */
#include "rtos.h"
#include "mid_accelerate.h"
#include "drv_mtimer.h"
#include "mid_scheduler.h"

// DEBUG开关 **************************************************************
#define MID_ACCEL_RTT_DEBUG_ON 1
#if MID_ACCEL_RTT_DEBUG_ON
#define MID_ACCEL_RTT_printf	SEGGER_RTT_printf
#else
#define MID_ACCEL_RTT_printf(...)
#endif

// 内部变量 **************************************************************
static MID_ACCEL_PARA_T		MID_ACCEL;

//**********************************************************************
// 函数功能：	采样定时器超时回调函数
// 输入参数：	
// 返回参数：	
static void Mid_Accel_IsrCb(void)
{
	Mid_Schd_TaskMsg_T Msg;
	Msg.Id = eSchdTaskMsgAccel;
	
	Mid_Schd_TaskEventSet(&Msg, 1);	
}

//**********************************************************************
// 函数功能：	获取驱动层采样率参数
// 输入参数：	Rate 中间层采样率枚举参数
// 返回参数：	驱动层采用率枚举参数
static bsp_accelerate_osr Mid_Accel_DrvRateGet(eMidAccelSampleRate Rate)
{
	bsp_accelerate_osr RetVal;
	
	switch(Rate)
	{
		case eMidAccelSampleRate1HZ:
			RetVal = ACCEL_SAMPLERATE_1_56HZ;
			break;
		case eMidAccelSampleRate2HZ:
			RetVal = ACCEL_SAMPLERATE_3_12HZ;
			break;
		case eMidAccelSampleRate25HZ:
			RetVal = ACCEL_SAMPLERATE_25HZ;
			break;
		case eMidAccelSampleRate50HZ:
			RetVal = ACCEL_SAMPLERATE_50HZ;
			break;
		case eMidAccelSampleRate100HZ:
			RetVal = ACCEL_SAMPLERATE_100HZ;
			break;
		case eMidAccelSampleRate200HZ:
			RetVal = ACCEL_SAMPLERATE_200HZ;
			break;
		default:
			RetVal = ACCEL_SAMPLERATE_NONE;
			break;
	}
	
	return RetVal;
}

//**********************************************************************
// 函数功能：	获取驱动层采样参数
// 输入参数：	Rate 中间层采样范围枚举参数
// 返回参数：	驱动层采用范围枚举参数
static bsp_accelerate_scalerange Mid_Accel_DrvRangeGet(eMidAccelSampleRange Range)
{
	bsp_accelerate_scalerange RetVal;
	
	switch(Range)
	{
		case eMidAccelSampleRange2G:
			RetVal = ACCEL_SCALERANGE_2G;
			break;
		case eMidAccelSampleRange4G:
			RetVal = ACCEL_SCALERANGE_4G;
			break;
		case eMidAccelSampleRange8G:
			RetVal = ACCEL_SCALERANGE_8G;
			break;
		case eMidAccelSampleRange16G:
			RetVal = ACCEL_SCALERANGE_16G;
			break;
		default:
			RetVal = ACCEL_SCALERANGE_2G;
			break;
	}
	
	return RetVal;
}

//**********************************************************************
// 函数功能：	初始化硬件传感器，并设置可定时读取的回调函数
// 输入参数：	IsrCb: 传感器达到可设置的读取时间回调函数
// 返回参数：	无
void Mid_Accel_Init(void)
{
	// 硬件初始并休眠
	Drv_Accel_GoSleep();
	
	// 默认参数初始化
	MID_ACCEL.SampleRate = eMidAccelSampleRate1HZ;
	MID_ACCEL.SampleRange = eMidAccelSampleRange2G;
	MID_ACCEL.SamplePeriod = 1000 / MID_ACCEL.SampleRate;
	MID_ACCEL.InitedFlg = true;
	
	// 创建采样定时器
	Drv_MTimer_Create(&MID_ACCEL.MTiemrId, MID_ACCEL.SamplePeriod, Mid_Accel_IsrCb);
}

//**********************************************************************
// 函数功能：	参数设置
// 输入参数：	Rate 采样率
//				Range 采样周期
// 返回参数：	无
void Mid_Accel_ParamSet(eMidAccelSampleRate Rate, eMidAccelSampleRange Range)
{
	MID_ACCEL.SampleRate = Rate;
	MID_ACCEL.SampleRange = Range;
	MID_ACCEL.SamplePeriod = 1000 / MID_ACCEL.SampleRate;		
}

//**********************************************************************
// 函数功能：	开始采样
// 输入参数：	
// 返回参数：	
void Mid_Accel_StartSample(void)
{
	// 硬件配置
	Mid_Schd_M2MutexTake();
	Drv_Accel_Set(Mid_Accel_DrvRateGet(MID_ACCEL.SampleRate), Mid_Accel_DrvRangeGet(MID_ACCEL.SampleRange));
	Mid_Schd_M2MutexGive();
	
	// 更新采样定时器参数并启动
	Drv_MTimer_Stop(MID_ACCEL.MTiemrId);	
	Drv_MTimer_Start(MID_ACCEL.MTiemrId, MID_ACCEL.SamplePeriod);
	
	MID_ACCEL.SamplingFlg = true;
}

//**********************************************************************
// 函数功能：	停止采样
// 输入参数：	
// 返回参数：	
void Mid_Accel_StopSample(void)
{
	// 停止硬件采样
	Mid_Schd_M2MutexTake();
	Drv_Accel_GoSleep();
	Mid_Schd_M2MutexGive();
	
	// 停止采样定时器
	Drv_MTimer_Stop(MID_ACCEL.MTiemrId);	
	
	MID_ACCEL.SamplingFlg = false;
}

//**********************************************************************
// 函数功能：	读取一次硬件数据并更新，等待有需要的外设来获取
// 输入参数：	
// 返回参数：
void Mid_Accel_DataUpdate(void)
{
	Mid_Schd_M2MutexTake();
	Drv_Accel_Read(MID_ACCEL.LatestData);
	Mid_Schd_M2MutexGive();
}

//**********************************************************************
// 函数功能：	获取所有配置参数
// 输入参数：	无
// 返回参数：	无
void Mid_Accel_ParamGet(MID_ACCEL_PARA_T* MID_ACCEL_PARA)
{
//	MID_ACCEL_PARA = &MID_ACCEL;
	memcpy(MID_ACCEL_PARA, &MID_ACCEL, sizeof(MID_ACCEL_PARA_T));
}

//**********************************************************************
// 函数功能：	读取当前加速度的最新值
// 输入参数：	无
// 返回参数：	无
void Mid_Accel_DataRead(int16 data[3])
{
	data[0]		= MID_ACCEL.LatestData[0];
	data[1]		= MID_ACCEL.LatestData[1];
	data[2]		= MID_ACCEL.LatestData[2];
}

//**********************************************************************
// 函数功能：	传感器自检,调用该函数时，确保该资源未使用
// 输入参数：	无
// 返回参数：	0x00:成功
// 				0xff:失败
uint16 Mid_Accel_SelfTest(void)
{
	return Drv_Accel_SelfTest();
}




