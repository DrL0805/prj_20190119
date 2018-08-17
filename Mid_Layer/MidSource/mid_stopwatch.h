#ifndef MID_STOPWATCH_H
#define MID_STOPWATCH_H

#include "platform_common.h"

#define STOPWATCH_PERIOD_MS	(100)	// ����жϣ���λms
#define STOPWATCH_MAX_STORE		(10)	// ���洢����ֵ

typedef struct 
{
	uint8   hour;
	uint8   min;
	uint8   sec;
	uint32	ms;
}Mid_StopWatch_Format_t;	// ����ʽ

typedef struct
{
	bool		InitedFlg;		// ��ʼ����ɱ�־
	bool		RuningFlg;		// �������б�־
	
	uint32_t	TotalMs;							// �ۻ������˶���ms
	uint32_t	MeasureCnt;							// ���������ۼ�
	uint32_t	MeasurePoint[STOPWATCH_MAX_STORE];	// ����ʱ��㣬��λms������ڿ�ʼ������
}Mid_StopWatch_Param_t;

extern void Mid_StopWatch_Reset(void);
extern void Mid_StopWatch_Init(void);
extern void Mid_StopWatch_Start(void);
extern void Mid_StopWatch_Stop(void);
extern uint32_t Mid_StopWatch_MeasurePoint(void);
extern uint32_t Mid_StopWatch_TotalMsGet(uint8_t Methon);
extern void Mid_StopWatch_ParamGet(Mid_StopWatch_Param_t* Mid_StopWatch_Param);
extern void Mid_StopWatch_FormatSwitch(uint32_t Ms, Mid_StopWatch_Format_t* Mid_StopWatch_Format);

#endif			//	STOPWATCH_APP_H
