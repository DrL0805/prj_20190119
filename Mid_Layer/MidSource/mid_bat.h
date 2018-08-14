#ifndef BAT_APP_H
#define	BAT_APP_H

#include "platform_common.h"

typedef enum
{
    MID_BAT_OFF_CHARGE 		= 0x00,
    MID_BAT_IN_CHARGING,
    MID_BAT_FULL_CHARGE,
    MID_BAT_LOW_LEVEL,
}MidBat_Msg;

typedef enum 
{
	BOTTUN_CELL = 0,
	CHARGING_CELL,
}mid_bat_cell_t;

typedef enum
{
    MID_BAT_LEVER_0,
	MID_BAT_LEVER_1,
	MID_BAT_LEVER_2,
	MID_BAT_LEVER_3,
	MID_BAT_LEVER_4,
	MID_BAT_LEVER_5,
}mid_bat_level_t;		// 电池电量等级

// 事件定义
typedef enum
{
	BAT_WAKEUP  = 0,
	BAT_SLEEP,
	BAT_CHECK,
	CHARGE_CHECK,
}BAT_INTERFACE_E;

typedef struct 
{
	uint8 id;
}bat_event_s;

typedef void (*midbat_cb)(uint8 bat_msg);


void Mid_Bat_Init(void);
void Mid_Bat_SetCallBack(midbat_cb bat_msg);
uint16 Mid_Bat_SelfTest(void);
void Mid_Bat_VolRead(uint16 *dataTemp);
void Mid_Bat_ChargeStateCheck(void);
void Mid_Bat_ChargeStateRead(uint8 *dataTemp);

//**********************************************************************
// 函数功能:  把ADC采集的电池电压采集值转化成电池电量百分比 
// 输入参数： dataTemp：传入变量，保存电池电量百分比
// 返回参数： 电池类型
//**********************************************************************
uint8 Mid_Bat_SocRead(uint8 *dataTemp);

//**********************************************************************
// 函数功能:  获取电量百分比并作等级评估
// 输入参数： 无
// 返回参数： 电量等级
//**********************************************************************
uint8 Mid_Bat_LevelRead(void);

//**********************************************************************
// 函数功能:  启动一次电池检测
// 输入参数： 无 
// 返回参数： 无
//**********************************************************************
void Mid_Bat_BatCheck(void);

//**********************************************************************
// 函数功能:  电池充电、电量处理事件  
// msg     :    事件信息
// 返回参数： 无
//**********************************************************************
uint16 Mid_Bat_EventProcess(bat_event_s* msg);

//**********************************************************************
// 函数功能:  电量计唤醒
// 输入参数： 无 
// 返回参数： 无
//**********************************************************************
void Mid_Bat_WakeUp(void);

//**********************************************************************
// 函数功能:  电量计休眠
// 输入参数： 无 
// 返回参数： 无
//**********************************************************************
void Mid_Bat_WakeSleep(void);

#endif			//BAT_APP_H

