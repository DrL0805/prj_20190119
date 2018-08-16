#ifndef	_DRV_TOUCH_H
#define	_DRV_TOUCH_H

#include "platform_common.h"

#if 1

#define DRV_TOUCH_RTT_DEBUG	3
#if (1 == DRV_TOUCH_RTT_DEBUG)	// ����ȼ�
#define DRV_TOUCH_RTT_LOG(...)
#define DRV_TOUCH_RTT_WARN(...)
#define DRV_TOUCH_RTT_ERR		SEGGER_RTT_printf
#elif (2 == DRV_TOUCH_RTT_DEBUG)	// ����ȼ�
#define DRV_TOUCH_RTT_LOG(...)
#define DRV_TOUCH_RTT_WARN	SEGGER_RTT_printf
#define DRV_TOUCH_RTT_ERR		SEGGER_RTT_printf
#elif (3 == DRV_TOUCH_RTT_DEBUG)	// ���Եȼ�
#define DRV_TOUCH_RTT_LOG		SEGGER_RTT_printf
#define DRV_TOUCH_RTT_WARN	SEGGER_RTT_printf
#define DRV_TOUCH_RTT_ERR		SEGGER_RTT_printf
#else							// ���Թر�
#define DRV_TOUCH_RTT_LOG(...)
#define DRV_TOUCH_RTT_WARN(...)
#define DRV_TOUCH_RTT_ERR(...)
#endif

typedef struct
{
	uint8_t	 Scale;
	uint16_t XResolution;
	uint16_t YResolution;
	uint8_t PointData[14];	// ����������Ϣ
}Drv_IT7259_Param_t;

extern void Drv_IT7259_Init(void);

#endif

#endif
