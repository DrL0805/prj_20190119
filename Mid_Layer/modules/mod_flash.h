#ifndef	__MOD_FLASH_H
#define __MOD_FLASH_H

#include "platform_common.h"

#define MOD_FLASH_RTT_DEBUG	3
#if (1 == MOD_FLASH_RTT_DEBUG)	// 错误等级
#define MOD_FLASH_RTT_LOG(...)
#define MOD_FLASH_RTT_WARN(...)
#define MOD_FLASH_RTT_ERR		SEGGER_RTT_printf
#elif (2 == MOD_FLASH_RTT_DEBUG)	// 警告等级
#define MOD_FLASH_RTT_LOG(...)
#define MOD_FLASH_RTT_WARN	SEGGER_RTT_printf
#define MOD_FLASH_RTT_ERR		SEGGER_RTT_printf
#elif (3 == MOD_FLASH_RTT_DEBUG)	// 调试等级
#define MOD_FLASH_RTT_LOG		SEGGER_RTT_printf
#define MOD_FLASH_RTT_WARN	SEGGER_RTT_printf
#define MOD_FLASH_RTT_ERR		SEGGER_RTT_printf
#else							// 调试关闭
#define MOD_FLASH_RTT_LOG(...)
#define MOD_FLASH_RTT_WARN(...)
#define MOD_FLASH_RTT_ERR(...)
#endif

typedef struct
{
	uint8_t Flg;
}Mod_Flash_Param_T;

/* 中间层任务调度数据结构体 */
typedef struct
{
	uint8_t Id;
}Mod_Flash_TaskMsg_T;

void Mod_Flash_TaskEventSet(Mod_Flash_TaskMsg_T* Msg, uint8_t FromISR);
void Mod_Flash_TaskCreate(void);
#endif



