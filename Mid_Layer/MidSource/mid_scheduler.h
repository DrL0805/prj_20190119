#ifndef	_MID_SCHEDULER_H
#define	_MID_SCHEDULER_H

#include "platform_common.h"
#include "rtos.h"

#include "drv_mtimer.h"
#include "mid_key.h"
#include "mid_motor.h"
//#include "mid_magnetism.h"
#include "mid_accelerate.h"
#include "mid_gyroscope.h"
//#include "mid_uart.h"
//#include "mid_rtc.h"
//#include "mid_stopwatch.h"
//#include "mid_nandflash.h"

//#include "App_win_process.h"

#define MID_SCHD_RTT_DEBUG	3
#if (1 == MID_SCHD_RTT_DEBUG)	// 错误等级
#define MID_SCHD_RTT_LOG(...)
#define MID_SCHD_RTT_WARN(...)
#define MID_SCHD_RTT_ERR		SEGGER_RTT_printf
#elif (2 == MID_SCHD_RTT_DEBUG)	// 警告等级
#define MID_SCHD_RTT_LOG(...)
#define MID_SCHD_RTT_WARN	SEGGER_RTT_printf
#define MID_SCHD_RTT_ERR		SEGGER_RTT_printf
#elif (3 == MID_SCHD_RTT_DEBUG)	// 调试等级
#define MID_SCHD_RTT_LOG		SEGGER_RTT_printf
#define MID_SCHD_RTT_WARN	SEGGER_RTT_printf
#define MID_SCHD_RTT_ERR		SEGGER_RTT_printf
#else							// 调试关闭
#define MID_SCHD_RTT_LOG(...)
#define MID_SCHD_RTT_WARN(...)
#define MID_SCHD_RTT_ERR(...)
#endif

/* Key参数信息 */
typedef struct
{
	eMidKeyVal	Val;
}Mid_Schd_KeyParam_T;
	

/* 中间层任务调度消息类型 */
typedef enum
{
	eSchdTaskMsgKey,
	eSchdTaskMsgAccel,
	eSchdTaskMsgGyro,
	eSchdTaskMsgMagnetism,
	eSchdTaskMsgMax,
}eSchdTaskMsgId;

/* 中间层任务调度数据结构体 */
typedef struct 
{
	eSchdTaskMsgId	Id;
	union
	{
		Mid_Schd_KeyParam_T	Key;
	}Param;
}Mid_Schd_TaskMsg_T;


extern void Mid_Schd_M2MutexTake(void);
extern void Mid_Schd_M2MutexGive(void);

extern void Mid_Schd_ParamInit(void);
extern void Mid_Schd_TaskEventSet(Mid_Schd_TaskMsg_T* Msg, uint8_t FromISR);
extern void Mid_Schd_TaskCreate(void);

extern SemaphoreHandle_t	SPI_I2C_M0_SemaphoreHandle;
extern SemaphoreHandle_t	SPI_I2C_M2_SemaphoreHandle;

#endif
