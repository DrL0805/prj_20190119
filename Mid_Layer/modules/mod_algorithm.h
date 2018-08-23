#ifndef	__MOD_ALGORITHM_H
#define __MOD_ALGORITHM_H

#include "platform_common.h"
#include "mid_sport_scene.h"



#define MOD_ALGO_RTT_DEBUG	3
#if (1 == MOD_ALGO_RTT_DEBUG)	// ����ȼ�
#define MOD_ALGO_RTT_LOG(...)
#define MOD_ALGO_RTT_WARN(...)
#define MOD_ALGO_RTT_ERR		SEGGER_RTT_printf
#elif (2 == MOD_ALGO_RTT_DEBUG)	// ����ȼ�
#define MOD_ALGO_RTT_LOG(...)
#define MOD_ALGO_RTT_WARN	SEGGER_RTT_printf
#define MOD_ALGO_RTT_ERR		SEGGER_RTT_printf
#elif (3 == MOD_ALGO_RTT_DEBUG)	// ���Եȼ�
#define MOD_ALGO_RTT_LOG		SEGGER_RTT_printf
#define MOD_ALGO_RTT_WARN	SEGGER_RTT_printf
#define MOD_ALGO_RTT_ERR		SEGGER_RTT_printf
#else							// ���Թر�
#define MOD_ALGO_RTT_LOG(...)
#define MOD_ALGO_RTT_WARN(...)
#define MOD_ALGO_RTT_ERR(...)
#endif


typedef enum
{
	eAlgoTaskMsgAccel,
	eAlgoTaskMsgGyro,
	eAlgoTaskMsgMagnetism,
	eAlgoTaskMsgGPS,
	eAlgoTaskMsgHrm,
	eAlgoTaskMsgMax,
}eAlgoTaskMsgId;

typedef struct
{
	uint8_t Flg;
}Mod_Algo_Param_T;

/* �м������������ݽṹ�� */
typedef struct
{
	eAlgoTaskMsgId Id;
}Mod_Algo_TaskMsg_T;

void Mod_Algo_TaskEventSet(Mod_Algo_TaskMsg_T* Msg, uint8_t FromISR);
void Mod_Algo_TaskCreate(void);
#endif



