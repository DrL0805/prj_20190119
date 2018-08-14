/**********************************************************************
**
**模块说明: 按键KEY驱动接口
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.18  修改流程  ZSL  
**
**********************************************************************/
#define KEY_MODULE

#include "string.h"
#include "io_config.h"
#include "sm_gpio.h"
#include "sm_timer.h"

#include "drv_key.h"

// key status
#define KEY_STANDBY     0X0000
#define KEY_DELAY       0x0001
#define KEY_HOLE        0x0002

// hold key time
#define HOLD_3S         32*3
#define HOLD_6S         32*6

/*******************variable define*******************/
typedef struct 
{
	uint8  keyPin;
    uint16 time_holdshort;
    uint16 time_holdlong;
}bspkey_info_s;

static bspkey_info_s Key_info[KEY_MAX];
static KeyCase Key_CallBack = NULL;

//key status flag
//0x0000: idle 	0x0001: keydelay     0x0002:keyhold
static uint16    keyDelayFlag = KEY_STANDBY;
static key_num_t validKeyNum  = KEY_MAX;   //key码
static uint16    holdKeyCnt   = 0;
static uint8     KeyTotal   = 0;      //根据配置，记录实际按键个数

//**********************************************************************
// 函数功能: 设置key callback
// 输入参数：u32PinNum : pin number
// 返回参数：无
//**********************************************************************
void Drv_Key_Isr(uint32 u32PinNum)
{
//	 SEGGER_RTT_printf(0,"u32PinNum=%d\n",u32PinNum);
    if(keyDelayFlag == KEY_STANDBY)
    {
//        SEGGER_RTT_printf(0,"u32PinNum=%d\n",u32PinNum);
#if(KEY_S0_PIN != IO_UNKNOW)
        if(u32PinNum == KEY_S0_PIN)
            validKeyNum = KEY_S0;
#endif
#if(KEY_S1_PIN != IO_UNKNOW)
        if(u32PinNum == KEY_S1_PIN)
            validKeyNum = KEY_S1;
#endif
#if(KEY_S2_PIN != IO_UNKNOW)
        if(u32PinNum == KEY_S2_PIN)
            validKeyNum = KEY_S2;
#endif
        if(validKeyNum != KEY_MAX)
        {
            keyDelayFlag 	= KEY_DELAY;
            SMDrv_CTimer_Start(KEY_CTIMER_MODULE);
        }
    }
}

//**********************************************************************
// 函数功能:	按键定时器中断处理函数
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
void Drv_KeyTimer_Isr(void)
{
//	SEGGER_RTT_printf(0,"Drv_KeyTimer_Isr \n");
    //异常按键值
    if(validKeyNum >= KeyTotal)
        return;
    if(keyDelayFlag == KEY_HOLE)
    {
        if(SMDrv_GPIO_InBitRead(Key_info[validKeyNum].keyPin) == KEY_PRESS)
        {
            holdKeyCnt++;
            if(holdKeyCnt == Key_info[validKeyNum].time_holdshort)	//short keyhold function
            {
                if(Key_CallBack != NULL)
                {
                    (Key_CallBack)(validKeyNum,KEY_HOLDSHORT_EVENT);
                }
            }
            if(holdKeyCnt == Key_info[validKeyNum].time_holdlong)	//long keyhold function
            {
                SMDrv_CTimer_Stop(KEY_CTIMER_MODULE);
                SMDrv_CTimer_Clear(KEY_CTIMER_MODULE);
                keyDelayFlag 	= KEY_STANDBY;
                if(Key_CallBack != NULL)
                {
                    (Key_CallBack)(validKeyNum,KEY_HOLDLONG_EVENT);
                }
            }
        }
        else
        {
            // short key fuction
            SMDrv_CTimer_Stop(KEY_CTIMER_MODULE);
            SMDrv_CTimer_Clear(KEY_CTIMER_MODULE);
            keyDelayFlag 	= KEY_STANDBY;

            if(holdKeyCnt < Key_info[validKeyNum].time_holdshort)
            {
                if(Key_CallBack != NULL)
                {
                    (Key_CallBack)(validKeyNum,KEY_SHORT_EVENT);
                }
            }		
        }
    }

    if(keyDelayFlag == KEY_DELAY)
    {
        holdKeyCnt 		= 1;
        keyDelayFlag 	= KEY_HOLE;
    }
}

//**********************************************************************
// 函数功能:	注册的按键进行硬件初始化
// 输入参数：	
// keyinfo ： 	注册的按键信，按键个数，按键IO
// 返回参数：		
// 0x00    :    设置成功
//**********************************************************************
uint8 Drv_Key_Init(void)
{
	uint8 u8keys;
    
    memset(Key_info,0x00,sizeof(Key_info));
    //step 1:open keys with interrupt,as hw design
#if(KEY_S0_PIN != IO_UNKNOW)
    SMDrv_GPIO_Open(KEY_S0_PIN,NULL,Drv_Key_Isr);
    Key_info[KEY_S0].keyPin = KEY_S0_PIN;
    KeyTotal++;
#endif
#if(KEY_S1_PIN != IO_UNKNOW)
    SMDrv_GPIO_Open(KEY_S1_PIN,NULL,Drv_Key_Isr);
    Key_info[KEY_S1].keyPin = KEY_S1_PIN;
    KeyTotal++;
#endif
#if(KEY_S2_PIN != IO_UNKNOW)
    SMDrv_GPIO_Open(KEY_S2_PIN,NULL,Drv_Key_Isr);
    Key_info[KEY_S2].keyPin = KEY_S2_PIN;
    KeyTotal++;
#endif

    //没有key
    if(KeyTotal == 0)
    {
        return Ret_NoDevice;
    }

    //step 2:open a timer for KEY
    SMDrv_CTimer_SetIsrPrio(1);
    //set key timer 定时时间: 31.25ms
    SMDrv_CTimer_Open(KEY_CTIMER_MODULE,7,Drv_KeyTimer_Isr);

    //step 3: init param
	keyDelayFlag			= KEY_STANDBY;
	validKeyNum				= KEY_MAX;
	holdKeyCnt				= 0;
    //init Key_info
    Key_CallBack   = NULL;
	for(u8keys = 0; u8keys < KeyTotal; u8keys++)
	{
        Key_info[u8keys].time_holdshort = HOLD_3S;
        Key_info[u8keys].time_holdlong  = HOLD_6S;
	}
	return Ret_OK;
}

//**********************************************************************
// 函数功能:	设置按下功能回调函数
// 输入参数：   Key_Cb: 回调函数
// 返回参数:
// 0x00    :    设置成功
// 0x02    :    参数非法
//**********************************************************************
uint8 Drv_Key_SetCallBack(KeyCase Key_Cb)
{
    if(Key_Cb == NULL)
    {
        return Ret_InvalidParam;
    }
    Key_CallBack = Key_Cb;
	return Ret_OK;
}

//**********************************************************************
// 函数功能:  获取按键IO口电平状态
// 输入参数： keycode:key码
// 返回参数：
//      0x00:  设置成功
//      0x02:  参数非法
// IO状态  : 
//**********************************************************************
uint8 Drv_Key_GetPinValue(uint8 *state,uint16 keycode)
{
	if(keycode >= KeyTotal)
		return Ret_InvalidParam;
 
    if(KeyTotal == 0)
        return Ret_NoDevice;

	*state	= SMDrv_GPIO_InBitRead(Key_info[keycode].keyPin);
	return Ret_OK;
}

//**********************************************************************
// 函数功能:  获取按键总个数，需在init key之后才能call
// 输入参数： keycode:key码
// 返回参数： 按键个数
//**********************************************************************
uint8 Drv_Key_GetKeyTotal(void)
{
	return KeyTotal;
}

//**********************************************************************
// 函数功能: 设置按键事件的时间，短按默认为3秒，长按默认为6秒
// 输入参数：u16holdtime: 设置的按键时间，每个计数为31.25ms
// 			 keycode:	   按键类型
//           keyhold:    按键事件:一次，短按3s，长按6s
// 返回参数：
// 0x00    :    设置成功
// 0x02    :    参数非法
//**********************************************************************
uint8 Drv_Key_SetHoldTime(key_event keyhold, uint16 u16holdtime, uint16 keycode)
{
    if((keycode >= KeyTotal) || (keyhold == KEY_SHORT_EVENT))
        return Ret_InvalidParam;

    if(KeyTotal == 0)
        return Ret_NoDevice;

    //设置长按6s按键的时间
    if(keyhold == KEY_HOLDLONG_EVENT)
    {
        //如果设置长按时间小于默认的短按时间，参数出错
        if(Key_info[keycode].time_holdshort >= u16holdtime)
            return Ret_InvalidParam;
        Key_info[keycode].time_holdlong = u16holdtime;
    }
    //设置短按3s按键的时间
    if(keyhold == KEY_HOLDSHORT_EVENT)
    {
        //如果设置短按时间小于默认的长按时间，参数出错
        if(Key_info[keycode].time_holdlong <= u16holdtime)
            return Ret_InvalidParam;
        Key_info[keycode].time_holdshort = u16holdtime;
    }

    return Ret_OK;
}

