#ifndef	MID_KEY_H
#define MID_KEY_H

#include "platform_common.h"
#include "drv_key.h"

typedef enum
{
    MID_KEY_NONE,       //无效的消息
    MID_KEY0_SHORT,     //短按消息
    MID_KEY1_SHORT,
    MID_KEY2_SHORT,
    MID_KEY0_HOLDSHORT, //短按3s消息
    MID_KEY1_HOLDSHORT,
    MID_KEY2_HOLDSHORT,
    MID_KEY0_HOLDLONG,  //长按6s消息
    MID_KEY1_HOLDLONG,
    MID_KEY2_HOLDLONG,
}eMidKeyVal;

extern void Mid_Key_Init(void);
extern uint8 Mid_Key_ReadStatus(uint8 *status, uint16 keyType);

#endif	// KEYAPP_H
	
