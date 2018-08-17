#ifndef MID_COUNTDOWN_H
#define MID_COUNTDOWN_H

#include "platform_common.h"

#define COUNTDOWN_PERIOD_MS	(1000)	// 倒计时中断，单位ms

typedef struct 
{
    uint32_t 	hour;
    uint32_t 	min;
    uint32_t 	sec;
}Mid_CountDown_Format_t;

typedef struct
{
	bool		InitedFlg;		// 初始化完成标志
	bool		RuningFlg;		// 正在运行标志
	
	uint32_t	TotalSec;			// 设置总时间
	uint32_t	RemainSec;			// 剩余时间
}Mid_CountDown_Param_t;

extern void Mid_Countdown_Reset(void);
extern void Mid_Countdown_Init(void);
extern void Mid_Countdown_Start(void);
extern void Mid_Countdown_Stop(void);
extern uint32_t Mid_Countdown_RemainRead(void);
extern void Mid_Countdown_TimeWrite(uint32_t Sec);

#endif



