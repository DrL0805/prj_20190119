#ifndef	__MOD_SYSTEM_H
#define __MOD_SYSTEM_H

#include "main.h"

#define MOD_SYS_RTT_DEBUG	3
#if (1 == MOD_SYS_RTT_DEBUG)	// 错误等级
#define MOD_SYS_RTT_LOG(...)
#define MOD_SYS_RTT_WARN(...)
#define MOD_SYS_RTT_ERR		SEGGER_RTT_printf
#elif (2 == MOD_SYS_RTT_DEBUG)	// 警告等级
#define MOD_SYS_RTT_LOG(...)
#define MOD_SYS_RTT_WARN	SEGGER_RTT_printf
#define MOD_SYS_RTT_ERR		SEGGER_RTT_printf
#elif (3 == MOD_SYS_RTT_DEBUG)	// 调试等级
#define MOD_SYS_RTT_LOG		SEGGER_RTT_printf
#define MOD_SYS_RTT_WARN	SEGGER_RTT_printf
#define MOD_SYS_RTT_ERR		SEGGER_RTT_printf
#else							// 调试关闭
#define MOD_SYS_RTT_LOG(...)
#define MOD_SYS_RTT_WARN(...)
#define MOD_SYS_RTT_ERR(...)
#endif



extern void Mod_Sys_Init(void);
extern void Mod_Sys_PwrOn(void);


#endif



