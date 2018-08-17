#ifndef	MID_ACCELERATE_H
#define	MID_ACCELERATE_H

#include "platform_common.h"
#include "drv_accelerate.h"

#define MID_ACCEL_ID_MAX	(5)

// 采样范围
typedef enum
{
	eMidAccelSampleRange2G,
	eMidAccelSampleRange4G,
	eMidAccelSampleRange8G,
	eMidAccelSampleRange16G,
}eMidAccelSampleRange;

// 采样率
typedef enum
{
	eMidAccelSampleRate1HZ   = 1,		// 1000ms
	eMidAccelSampleRate2HZ   = 2,		// 500ms
	eMidAccelSampleRate25HZ  = 25,		// 40ms
	eMidAccelSampleRate50HZ  = 50,		// 20ms
	eMidAccelSampleRate100HZ = 100,		// 10ms
	eMidAccelSampleRate200HZ = 200,		// 5ms
}eMidAccelSampleRate;

typedef struct
{
	bool					EnableFlg;	
	eMidAccelSampleRate 	Rate;		// 采样率
	eMidAccelSampleRange	Range;		// 采样范围
}MID_ACCEL_ID_T;

typedef struct
{
	bool						InitedFlg;		// 已初始化标志
	bool						SamplingFlg;	// 正在采样标志	
	
	uint32_t 					MTiemrId;
	MID_ACCEL_ID_T				ID[MID_ACCEL_ID_MAX];

	int16_t 					LatestData[3];		// 保存最新采到的数据
	eMidAccelSampleRate			SampleRate;			// 硬件采样率
	eMidAccelSampleRange		SampleRange;		// 硬件采样范围
	uint32_t    				SamplePeriod;		// 采样定时周期，单位ms
}MID_ACCEL_PARA_T;

extern void Mid_Accel_Init(void);
//extern void Mid_Accel_ParamSet(eMidAccelSampleRate Rate, eMidAccelSampleRange Range);
extern uint16_t Mid_Accel_StartSample(uint8_t* Id, eMidAccelSampleRate Rate, eMidAccelSampleRange Range);
extern uint16_t Mid_Accel_StopSample(uint8_t Id);
extern void Mid_Accel_DataUpdate(void);
extern void Mid_Accel_ParamGet(MID_ACCEL_PARA_T* MID_ACCEL_PARA);
extern void Mid_Accel_DataRead(int16 data[3]);
extern uint16 Mid_Accel_SelfTest(void);

#endif		// ACCELERATE_APP_H
