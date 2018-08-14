/**********************************************************************
**
**模块说明: mid层KEY接口
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.24  修改流程  ZSL  
**
**********************************************************************/
#include "mid_key.h"
#include "mid_scheduler.h"
#include "rtos.h"

#define MID_KEY_RTT_DEBUG_ON	1
#if MID_KEY_RTT_DEBUG_ON
#define MID_KEY_RTT_printf	SEGGER_RTT_printf
#else
#define MID_KEY_RTT_printf(...)
#endif

//mid layer处理key消息isr
void KeyAppCase_Isr(key_num_t keynum,key_event event_type)
{
    eMidKeyVal key_msg = MID_KEY_NONE;

    //键码超过按键总个数或app key 回调无效，无需处理
	if(keynum >= Drv_Key_GetKeyTotal())
        return;

	switch(event_type)
	{
		case KEY_SHORT_EVENT:        //短按一次事件
			switch (keynum)
			{
				case KEY_S0:
					key_msg = MID_KEY0_SHORT;
					break;
				case KEY_S1:
					key_msg = MID_KEY1_SHORT;
					break;
				case KEY_S2:
					key_msg = MID_KEY2_SHORT;
					break;
				default:
					break;
			}
			break;
		case KEY_HOLDSHORT_EVENT:    //短按3s事件
			switch (keynum)
			{
				case KEY_S0:
					key_msg = MID_KEY0_HOLDSHORT;
					break;
				case KEY_S1:
					key_msg = MID_KEY1_HOLDSHORT;
					break;
				case KEY_S2:
					key_msg = MID_KEY2_HOLDSHORT;
					break;
				default:
					break;
			}
			break;
		case KEY_HOLDLONG_EVENT:     //长按6s事件
			switch (keynum)
			{
				case KEY_S0:
					key_msg = MID_KEY0_HOLDLONG;
					break;
				case KEY_S1:
					key_msg = MID_KEY1_HOLDLONG;
					break;
				case KEY_S2:
					key_msg = MID_KEY2_HOLDLONG;
					break;
				default:
					break;
			}
			break;
		default:
			return;
	}
	
//	MID_KEY_RTT_printf(0,"Key_Isr %d \n",keynum);
	
	 // 发送按键消息
	Mid_Schd_TaskMsg_T Msg;
	Msg.Id = eSchdTaskMsgKey;
	Msg.Param.Key.Val = key_msg;
	Mid_Schd_TaskEventSet(&Msg, 1);
}

//**********************************************************************
// 函数功能: 初始化Key
// 输入参数：	
// 返回参数：
//**********************************************************************
void Mid_Key_Init(void)
{
    // 硬件初始化
    Drv_Key_Init();
	
	// 设置短按时长
	Drv_Key_SetHoldTime(KEY_HOLDSHORT_EVENT, 32*3, KEY_S0);
	
	// 设置长按时长
	Drv_Key_SetHoldTime(KEY_HOLDLONG_EVENT, 32*12, KEY_S0);
	
    // 按键回调函数
    Drv_Key_SetCallBack(KeyAppCase_Isr);
}

//**********************************************************************
// 函数功能:	读取按键电平状态
// 输入参数：	status:	 返回电平的指针
// 				keyType: 按键类型
// 返回参数：	0x00:操作成功
// 				0xff:操作失败
//**********************************************************************
uint8 Mid_Key_ReadStatus(uint8 *status, uint16 keyType)
{
	// 此函数未测试过 by DrL
    return Drv_Key_GetPinValue(status, keyType);
}


