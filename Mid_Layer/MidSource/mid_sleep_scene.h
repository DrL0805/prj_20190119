#ifndef MID_SLEEP_SCENE_H
#define MID_SLEEP_SCENE_H

#include "platform_common.h"
#include "algorithm_sleep.h"

#define SLEEP_SCENE_PERIOD_MS	(1000)	// 睡眠算法调用周期

typedef struct
{
	bool		EnableFlg;		
	uint8_t		SampleId;
	uint32_t 	IntervalMs;		
}Mid_SleepScene_Param_t;

extern void Mid_SleepScene_algorithm(int16 *xyzData, uint32_t Interval);
extern uint16 Mid_SleepScene_Init(void);
extern uint16 Mid_SleepScene_GetRecord(sleep_data *sleepDataTemp, uint8 *validnum);
extern uint16 Mid_SleepScene_CurSleepGet(sleep_ui_info *sleepInfoTemp);
extern void Mid_SleepScene_Start(void);
extern void Mid_SleepScene_Stop(void);
#endif


