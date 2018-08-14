#ifndef	DRV_KEY_H
#define	DRV_KEY_H

#include "platform_common.h"

//按键编号枚举定义
typedef enum
{
	KEY_S0	= 0,
	KEY_S1,
	KEY_S2,
	KEY_MAX,
} key_num_t;

typedef enum 
{
	KEY_PRESS 	= 0,
	KEY_RELEASE = 1,
}key_state_t;

typedef enum 
{
    KEY_SHORT_EVENT,     //短按一次事件
    KEY_HOLDSHORT_EVENT, //短按3s事件
    KEY_HOLDLONG_EVENT,  //长按6s事件 
}key_event;

//按键事件回调函数
//keynum:key类型值，event_type:短按，3s短按及6s长按事件
typedef void (*KeyCase)(key_num_t keynum,key_event event_type);


//**********************************************************************
// 函数功能:	注册的按键进行硬件初始化
// 输入参数：	
// keyinfo ： 	注册的按键信，按键个数，按键IO
// 返回参数：		
// 0x00    :    设置成功
// 0xff    :    设置失败
//**********************************************************************
extern uint8 Drv_Key_Init(void);

//**********************************************************************
// 函数功能:	设置按下功能回调函数
// 输入参数：   Key_Cb: 回调函数
// 返回参数:
// 0x00    :    设置成功
// 0xff    :    设置失败
//**********************************************************************
extern uint8 Drv_Key_SetCallBack(KeyCase Key_Cb);

//**********************************************************************
// 函数功能:  获取按键IO口电平状态
// 输入参数： keycode:key码
// 返回参数： 
// IO状态  : 
//**********************************************************************
extern uint8 Drv_Key_GetPinValue(uint8 *state,uint16 keycode);

//**********************************************************************
// 函数功能:  获取按键总个数，需在init key之后才能call
// 输入参数： keycode:key码
// 返回参数： 按键个数
//**********************************************************************
extern uint8 Drv_Key_GetKeyTotal(void);

//**********************************************************************
// 函数功能: 设置按键事件的时间，短按默认为3秒，长按默认为6秒
// 输入参数：u16holdtime: 设置的按键时间，每个计数为31.25ms
// 			 keycode:	   按键类型
//           keyhold:    按键事件:一次，短按3s，长按6s
// 返回参数：
//**********************************************************************
extern uint8 Drv_Key_SetHoldTime(key_event keyhold, uint16 u16holdtime, uint16 keycode);

#endif	//

