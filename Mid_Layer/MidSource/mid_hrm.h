#ifndef MID_HRM_H
#define MID_HRM_H

#include "platform_common.h"

typedef enum
{
    HRM_FACTORYTEST         =0,
    HRM_FACTORYTEST_START,
    HRM_FACTORYTEST_STOP,
    HRM_ENABLE,
    HRM_DISABLE,
    HRM_START,
    HRM_STOP,
    HRM_DATA_READY,
    HRM_TOUCH,
    HRM_CALCULATE,
    HRM_ACCELREAD,
}HRM_INTERFACE_T;

typedef struct 
{
	uint16			id;
}hrm_event_s;

//**********************************************************************
//函数功能： 心率模块初始化
//输入参数：     
//ReadyIsrCb 重力数据指针
//TouchIsrCb：重力数据长度   
//返回参数： 无
//**********************************************************************
extern void Mid_Hrm_Init(func *ReadyIsrCb,func *TouchIsrCb);

//**********************************************************************
//函数功能：触摸状态获取
//输入参数：无   
//返回参数：
// 0x00   :  操作成功
// 0xFF   :  操作失败
//**********************************************************************
extern uint16 Mid_Hrm_TouchStatusRead(uint8 *ui8istouch);

//**********************************************************************
//函数功能：心率值获取
//输入参数：无   
//返回参数：心率值
//**********************************************************************
extern uint16  Mid_Hrm_Read(uint8 *hrmval);

//**********************************************************************
//函数功能：心率模块漏光测试数据获取
//输入参数：无   
//返回参数：
// 0x00   :  操作成功
// 0xFF   :  操作失败
//**********************************************************************
extern uint16 Mid_Hrm_FactoryTestRead(uint16 ui16lightleak[3]);

//**********************************************************************
// 函数功能:    心率模块事件处理
// 输入参数：    无
// 返回参数：    0x00: success
//              0xff: fail
//**********************************************************************
extern uint16 Mid_Hrm_EventProcess(hrm_event_s *msg);

void    Mid_Hrm_ReadyCbInit(void (*hrmreadycb)(uint8 hrmval));

#endif
