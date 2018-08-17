#ifndef MID_STOPWATCH_H
#define MID_STOPWATCH_H

#include "platform_common.h"

#define STOPWATCH_PERIOD_MS	(100)	// 秒表中断，单位ms
#define STOPWATCH_MAX_STORE		(10)	// 最大存储多少值

typedef struct 
{
	uint8   hour;
	uint8   min;
	uint8   sec;
	uint32	ms;
}Mid_StopWatch_Format_t;	// 秒表格式

typedef struct
{
	bool		InitedFlg;		// 初始化完成标志
	bool		RuningFlg;		// 正在运行标志
	
	uint32_t	TotalMs;							// 累积运行了多少ms
	uint32_t	MeasureCnt;							// 测量次数累加
	uint32_t	MeasurePoint[STOPWATCH_MAX_STORE];	// 测量时间点，单位ms，相对于开始计数出
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
