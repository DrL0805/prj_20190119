#ifndef	_DRV_TOUCH_H
#define	_DRV_TOUCH_H

#include "platform_common.h"

#if 1

#define DRV_TOUCH_RTT_DEBUG	3
#if (1 == DRV_TOUCH_RTT_DEBUG)	// 错误等级
#define DRV_TOUCH_RTT_LOG(...)
#define DRV_TOUCH_RTT_WARN(...)
#define DRV_TOUCH_RTT_ERR		SEGGER_RTT_printf
#elif (2 == DRV_TOUCH_RTT_DEBUG)	// 警告等级
#define DRV_TOUCH_RTT_LOG(...)
#define DRV_TOUCH_RTT_WARN	SEGGER_RTT_printf
#define DRV_TOUCH_RTT_ERR		SEGGER_RTT_printf
#elif (3 == DRV_TOUCH_RTT_DEBUG)	// 调试等级
#define DRV_TOUCH_RTT_LOG		SEGGER_RTT_printf
#define DRV_TOUCH_RTT_WARN	SEGGER_RTT_printf
#define DRV_TOUCH_RTT_ERR		SEGGER_RTT_printf
#else							// 调试关闭
#define DRV_TOUCH_RTT_LOG(...)
#define DRV_TOUCH_RTT_WARN(...)
#define DRV_TOUCH_RTT_ERR(...)
#endif

typedef struct
{
	uint8_t	 Scale;
	uint16_t XResolution;
	uint16_t YResolution;
	uint8_t PointData[14];	// 触摸点阵信息
}Drv_IT7259_Param_t;

extern void Drv_IT7259_Init(void);

#endif

#endif
