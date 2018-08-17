#ifndef MID_COUNTDOWN_H
#define MID_COUNTDOWN_H

#include "platform_common.h"

#define COUNTDOWN_PERIOD_MS	(1000)	// ����ʱ�жϣ���λms

typedef struct 
{
    uint32_t 	hour;
    uint32_t 	min;
    uint32_t 	sec;
}Mid_CountDown_Format_t;

typedef struct
{
	bool		InitedFlg;		// ��ʼ����ɱ�־
	bool		RuningFlg;		// �������б�־
	
	uint32_t	TotalSec;			// ������ʱ��
	uint32_t	RemainSec;			// ʣ��ʱ��
}Mid_CountDown_Param_t;

extern void Mid_Countdown_Reset(void);
extern void Mid_Countdown_Init(void);
extern void Mid_Countdown_Start(void);
extern void Mid_Countdown_Stop(void);
extern uint32_t Mid_Countdown_RemainRead(void);
extern void Mid_Countdown_TimeWrite(uint32_t Sec);

#endif



