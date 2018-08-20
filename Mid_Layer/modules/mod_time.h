#ifndef	__MOD_TIME_H
#define __MOD_TIME_H

#include "platform_common.h"

#include "mid_rtc.h"
#include "mid_alarm.h"
#include "mid_stopwatch.h"
#include "mid_countdown.h"

#include "main.h"

#define MOD_TIME_RTT_DEBUG	3
#if (1 == MOD_TIME_RTT_DEBUG)	// ����ȼ�
#define MOD_TIME_RTT_LOG(...)
#define MOD_TIME_RTT_WARN(...)
#define MOD_TIME_RTT_ERR		SEGGER_RTT_printf
#elif (2 == MOD_TIME_RTT_DEBUG)	// ����ȼ�
#define MOD_TIME_RTT_LOG(...)
#define MOD_TIME_RTT_WARN	SEGGER_RTT_printf
#define MOD_TIME_RTT_ERR		SEGGER_RTT_printf
#elif (3 == MOD_TIME_RTT_DEBUG)	// ���Եȼ�
#define MOD_TIME_RTT_LOG		SEGGER_RTT_printf
#define MOD_TIME_RTT_WARN	SEGGER_RTT_printf
#define MOD_TIME_RTT_ERR		SEGGER_RTT_printf
#else							// ���Թر�
#define MOD_TIME_RTT_LOG(...)
#define MOD_TIME_RTT_WARN(...)
#define MOD_TIME_RTT_ERR(...)
#endif

typedef struct
{
	uint8_t Flg;
}Mod_Time_Param_T;


typedef struct
{
	eMidRTCMsg	Msg;
}Mod_Time_RTCParam_T;

/* �м�����������Ϣ���� */
typedef enum
{
	eTimeTaskMsgRTC,
	eTimeTaskMsgMax,
}eTimeTaskMsgId;

/* �м������������ݽṹ�� */
typedef struct
{
	eTimeTaskMsgId Id;
	union
	{
		Mod_Time_RTCParam_T	RTC;
	}Param;
}Mod_Time_TaskMsg_T;

void Mod_Time_TaskEventSet(Mod_Time_TaskMsg_T* Msg, uint8_t FromISR);
void Mod_Time_TaskCreate(void);
#endif



