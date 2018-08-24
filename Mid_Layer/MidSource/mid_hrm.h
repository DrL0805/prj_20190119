#ifndef MID_HRM_H
#define MID_HRM_H

#include "platform_common.h"
#include "drv_hrm.h"

#define MID_HRM_RTT_DEBUG	3
#if (1 == MID_HRM_RTT_DEBUG)	// 错误等级
#define MID_HRM_RTT_LOG(...)
#define MID_HRM_RTT_WARN(...)
#define MID_HRM_RTT_ERR		SEGGER_RTT_printf
#elif (2 == MID_HRM_RTT_DEBUG)	// 警告等级
#define MID_HRM_RTT_LOG(...)
#define MID_HRM_RTT_WARN	SEGGER_RTT_printf
#define MID_HRM_RTT_ERR		SEGGER_RTT_printf
#elif (3 == MID_HRM_RTT_DEBUG)	// 调试等级
#define MID_HRM_RTT_LOG		SEGGER_RTT_printf
#define MID_HRM_RTT_WARN	SEGGER_RTT_printf
#define MID_HRM_RTT_ERR		SEGGER_RTT_printf
#else							// 调试关闭
#define MID_HRM_RTT_LOG(...)
#define MID_HRM_RTT_WARN(...)
#define MID_HRM_RTT_ERR(...)
#endif

//typedef enum 
//{
//    MID_HRM_OFF_TOUCH  = 0,
//    MID_HRM_ON_TOUCH  = 1,
//}hrm_touch_t;

typedef struct
{
	bool		InitFlg;	
}Mid_Hrm_Param_t;

void HrmAccelMenSet(int16 *fifodata);
uint16 HrmStart(void);
uint16 HrmStop(void);
uint8 HrmCalculate(void);
void Mid_Hrm_Init(void);
uint16 Mid_Hrm_TouchStatusRead(uint8 *ui8istouch);
uint16  Mid_Hrm_Read(uint8 *hrmval);
uint16 Mid_Hrm_FactoryTestRead(uint16 ui16lightleak[3]);
uint8 Mid_Hrm_FactoryTest(void);

#endif
