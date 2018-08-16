#ifndef	__MOD_PDU_H
#define __MOD_PDU_H

#include "platform_common.h"

#define MOD_PDU_RTT_DEBUG	3
#if (1 == MOD_PDU_RTT_DEBUG)	// ����ȼ�
#define MOD_PDU_RTT_LOG(...)
#define MOD_PDU_RTT_WARN(...)
#define MOD_PDU_RTT_ERR		SEGGER_RTT_printf
#elif (2 == MOD_PDU_RTT_DEBUG)	// ����ȼ�
#define MOD_PDU_RTT_LOG(...)
#define MOD_PDU_RTT_WARN	SEGGER_RTT_printf
#define MOD_PDU_RTT_ERR		SEGGER_RTT_printf
#elif (3 == MOD_PDU_RTT_DEBUG)	// ���Եȼ�
#define MOD_PDU_RTT_LOG		SEGGER_RTT_printf
#define MOD_PDU_RTT_WARN	SEGGER_RTT_printf
#define MOD_PDU_RTT_ERR		SEGGER_RTT_printf
#else							// ���Թر�
#define MOD_PDU_RTT_LOG(...)
#define MOD_PDU_RTT_WARN(...)
#define MOD_PDU_RTT_ERR(...)
#endif

typedef struct
{
	uint8_t Flg;
}Mod_PDU_Param_T;

/* �м������������ݽṹ�� */
typedef struct
{
	uint8_t Id;
}Mod_PDU_TaskMsg_T;

void Mod_PDU_TaskEventSet(Mod_PDU_TaskMsg_T* Msg, uint8_t FromISR);
void Mod_PDU_TaskCreate(void);
#endif



