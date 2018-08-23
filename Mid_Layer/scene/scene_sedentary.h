#ifndef SCENE_SEDENTARY_H
#define SCENE_SEDENTARY_H

#include "platform_common.h"
#include "algorithm_sedentary.h"

#define SCENE_SEDENTARY_RTT_DEBUG	1
#if (1 == SCENE_SEDENTARY_RTT_DEBUG)	// 错误等级
#define SCENE_SEDENTARY_RTT_LOG(...)
#define SCENE_SEDENTARY_RTT_WARN(...)
#define SCENE_SEDENTARY_RTT_ERR		SEGGER_RTT_printf
#elif (2 == SCENE_SEDENTARY_RTT_DEBUG)	// 警告等级
#define SCENE_SEDENTARY_RTT_LOG(...)
#define SCENE_SEDENTARY_RTT_WARN	SEGGER_RTT_printf
#define SCENE_SEDENTARY_RTT_ERR		SEGGER_RTT_printf
#elif (3 == SCENE_SEDENTARY_RTT_DEBUG)	// 调试等级
#define SCENE_SEDENTARY_RTT_LOG		SEGGER_RTT_printf
#define SCENE_SEDENTARY_RTT_WARN	SEGGER_RTT_printf
#define SCENE_SEDENTARY_RTT_ERR		SEGGER_RTT_printf
#else							// 调试关闭
#define SCENE_SEDENTARY_RTT_LOG(...)
#define SCENE_SEDENTARY_RTT_WARN(...)
#define SCENE_SEDENTARY_RTT_ERR(...)
#endif

#define SCENE_SEDENTARY_PERIOD_MS	(1000)	// 睡眠算法调用周期

typedef struct
{
	bool		EnableFlg;		
	uint8_t		SampleId;
	uint32_t 	IntervalMs;		
}Scene_Sedentary_Param_t;

extern void Scene_Sedentary_algorithm(int16 *xyzData, uint32_t Interval);
extern uint16 Scene_Sedentary_Init(void);	
extern void Scene_Sedentary_Start(void);
extern void Mid_GestureScene_Stop(void);

#endif

