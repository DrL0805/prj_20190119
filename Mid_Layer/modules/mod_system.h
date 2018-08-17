#ifndef	__MOD_SYSTEM_H
#define __MOD_SYSTEM_H

#include "main.h"

#define MOD_SYS_RTT_DEBUG	3
#if (1 == MOD_SYS_RTT_DEBUG)	// ����ȼ�
#define MOD_SYS_RTT_LOG(...)
#define MOD_SYS_RTT_WARN(...)
#define MOD_SYS_RTT_ERR		SEGGER_RTT_printf
#elif (2 == MOD_SYS_RTT_DEBUG)	// ����ȼ�
#define MOD_SYS_RTT_LOG(...)
#define MOD_SYS_RTT_WARN	SEGGER_RTT_printf
#define MOD_SYS_RTT_ERR		SEGGER_RTT_printf
#elif (3 == MOD_SYS_RTT_DEBUG)	// ���Եȼ�
#define MOD_SYS_RTT_LOG		SEGGER_RTT_printf
#define MOD_SYS_RTT_WARN	SEGGER_RTT_printf
#define MOD_SYS_RTT_ERR		SEGGER_RTT_printf
#else							// ���Թر�
#define MOD_SYS_RTT_LOG(...)
#define MOD_SYS_RTT_WARN(...)
#define MOD_SYS_RTT_ERR(...)
#endif



extern void Mod_Sys_Init(void);
extern void Mod_Sys_PwrOn(void);


#endif



