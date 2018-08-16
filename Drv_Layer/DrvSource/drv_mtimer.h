/**********************************************************/
//  				   created by huaangrunguang
// V1.0   2017-7-12    modified by yudongyou 
/*********************************************************/
#ifndef MULTITIMER_H
#define MULTITIMER_H

#include "platform_common.h"

#define DRV_MTIMER_RTT_DEBUG	0
#if (1 == DRV_MTIMER_RTT_DEBUG)	// 错误等级
#define DRV_MTIMER_RTT_LOG(...)
#define DRV_MTIMER_RTT_WARN(...)
#define DRV_MTIMER_RTT_ERR		SEGGER_RTT_printf
#elif (2 == DRV_MTIMER_RTT_DEBUG)	// 警告等级
#define DRV_MTIMER_RTT_LOG(...)
#define DRV_MTIMER_RTT_WARN	SEGGER_RTT_printf
#define DRV_MTIMER_RTT_ERR		SEGGER_RTT_printf
#elif (3 == DRV_MTIMER_RTT_DEBUG)	// 调试等级
#define DRV_MTIMER_RTT_LOG		SEGGER_RTT_printf
#define DRV_MTIMER_RTT_WARN	SEGGER_RTT_printf
#define DRV_MTIMER_RTT_ERR		SEGGER_RTT_printf
#else							// 调试关闭
#define DRV_MTIMER_RTT_LOG(...)
#define DRV_MTIMER_RTT_WARN(...)
#define DRV_MTIMER_RTT_ERR(...)
#endif

#define MULTITIMER_NUM_MAX       8  // 事件数量,根据需求而定。
#define MULTITIMER_MIN_PERIOD	 5	// 最小周期，单位ms

typedef enum
{
	eMTimerStateUninited,				// 未初始化
	eMTimerStateInited,					// 已初始化
	eMTimerStateRunning,				// 运行
	eMTimerStateSuspend					// 暂停
}eMTimerState;

typedef struct
{
	eMTimerState	State;
	uint32_t	Period;			// 定期周期
	uint32_t	AimTick;		// 目标tick数
	uint32_t    CurrTick;		// 已运行的tick数
	comm_cb		*IsrCb;			// 回调函数
}MTimer_Param_t;

typedef struct
{
	uint32_t	MinPeriod;	// 所有定时器的最小周期
	uint32_t	CreateCnt;		// 已创建的定时器数量
	uint32_t 	RunningCnt;	// 正在运行定时器数量
	MTimer_Param_t	MTimer[MULTITIMER_NUM_MAX];
}Drv_MTimer_Param_t;


extern void Drv_MTimer_Init(void);
extern uint32_t Drv_MTimer_Create(uint32_t *Id, uint32_t Period, void (*IsrCb)(void));
extern uint32_t Drv_MTimer_Start(uint32_t Id, uint32_t Period);
extern uint32_t Drv_MTimer_Stop(uint32_t Id);

#if DRV_MTIMER_RTT_DEBUG
extern void Drv_MTimer_Test(void);
#endif

#endif



