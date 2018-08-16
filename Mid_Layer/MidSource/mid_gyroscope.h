#ifndef	MID_GYROSCOPE_H
#define	MID_GYROSCOPE_H

#include "platform_common.h"
#include "drv_gyroscope.h"

// ������Χ
typedef enum
{
	eMidGyroSampleRange125S,
	eMidGyroSampleRange250S,
	eMidGyroSampleRange500S,
	eMidGyroSampleRange1000S,
	eMidGyroSampleRange2000S,
}eMidGyroSampleRange;

// ������
typedef enum
{
	eMidGyroSampleRate1HZ   = 1,		// 1000ms
	eMidGyroSampleRate2HZ   = 2,		// 500ms
	eMidGyroSampleRate25HZ  = 25,		// 40ms
	eMidGyroSampleRate50HZ  = 50,		// 20ms
	eMidGyroSampleRate100HZ = 100,		// 10ms
	eMidGyroSampleRate200HZ = 200,		// 5ms
}eMidGyroSampleRate;

typedef struct
{
	uint32_t 					MTiemrId;		
	
	bool						InitedFlg;		// �ѳ�ʼ����־
	bool						SamplingFlg;	// ���ڲ�����־	
	
	int16_t 					LatestData[3];		// �������²ɵ�������
	eMidGyroSampleRate			SampleRate;			// Ӳ��������
	eMidGyroSampleRange			SampleRange;		// Ӳ��������Χ
	uint32_t    				SamplePeriod;		// ������ʱ���ڣ���λms
}MID_GYRO_PARA_T;

extern void Mid_Gyro_Init(void);
extern void Mid_Gyro_ParamSet(eMidGyroSampleRate Rate, eMidGyroSampleRange Range);
extern void Mid_Gyro_StartSample(void);
extern void Mid_Gyro_StopSample(void);
extern void Mid_Gyro_DataUpdate(void);
extern void Mid_Gyro_ParamGet(MID_GYRO_PARA_T* MID_GYRO_PARA);
extern void Mid_Gyro_DataRead(int16 data[3]);
extern uint16 Mid_Gyro_SelfTest(void);


#endif		// GYROSCOPE_APP_H
