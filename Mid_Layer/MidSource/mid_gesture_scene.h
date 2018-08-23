#ifndef MID_GESTURE_SCENE_H
#define MID_GESTURE_SCENE_H

#include "platform_common.h"
#include "algorithm_gesture.h"

#define MID_GESTURE_RTT_DEBUG	1
#if (1 == MID_GESTURE_RTT_DEBUG)	// ����ȼ�
#define MID_GESTURE_RTT_LOG(...)
#define MID_GESTURE_RTT_WARN(...)
#define MID_GESTURE_RTT_ERR		SEGGER_RTT_printf
#elif (2 == MID_GESTURE_RTT_DEBUG)	// ����ȼ�
#define MID_GESTURE_RTT_LOG(...)
#define MID_GESTURE_RTT_WARN	SEGGER_RTT_printf
#define MID_GESTURE_RTT_ERR		SEGGER_RTT_printf
#elif (3 == MID_GESTURE_RTT_DEBUG)	// ���Եȼ�
#define MID_GESTURE_RTT_LOG		SEGGER_RTT_printf
#define MID_GESTURE_RTT_WARN	SEGGER_RTT_printf
#define MID_GESTURE_RTT_ERR		SEGGER_RTT_printf
#else							// ���Թر�
#define MID_GESTURE_RTT_LOG(...)
#define MID_GESTURE_RTT_WARN(...)
#define MID_GESTURE_RTT_ERR(...)
#endif

#define GESTURE_SCENE_PERIOD_MS	(1000)	// �����㷨��������

typedef struct
{
	bool		EnableFlg;		
	uint8_t		SampleId;
	uint32_t 	IntervalMs;		
}Mid_GestureScene_Param_t;

extern void Mid_GestureScene_algorithm(int16 *xyzData, uint32_t Interval);
extern uint16 Mid_GestureScene_Init(void);	
extern void Mid_GestureScene_Start(void);
extern void Mid_GestureScene_Stop(void);

#endif

