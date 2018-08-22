#ifndef			MID_ALARM_H
#define			MID_ALARM_H

#include "platform_common.h"
#include "mid_rtc.h"

typedef enum
{
	eMidAlarmGroup0,
	eMidAlarmGroup1,
	eMidAlarmGroup2,
	eMidAlarmGroup3,
	eMidAlarmGroup4,
	eMidAlarmGroupNum,	// 最大闹钟组数，大于此闹钟无效 
}eMidAlarmGroup;

typedef struct 
{
	uint8 reptswitch;			// 闹钟重复开关 bit0~bit6: sun~sat,bit7:重复开关
	uint8 hour;
	uint8 min;
	uint8 delayswitch;	    // 闹钟贪睡开关 1: delay alarm enable, 0: delay alarm disable
	uint8 alarmswitch;	    // 闹钟开关 1: alarm open,         0: alarm close
}alarm_clock_t;


typedef struct
{
	uint8_t 			RingSwitch;		// 闹钟贪睡功能开关，1 当前正在处于贪睡模式
	uint8_t 			RingCnt;		// 贪睡次数，最多5次
	uint8_t 			accTimer;		// 贪睡时间累积，每10分钟一次
	eMidAlarmGroup 		curRingAlarmId;	// 当前闹钟句柄（通过获取这个值来判断哪个闹钟响了）
	
	alarm_clock_t	Clock[eMidAlarmGroupNum];	// 闹钟组数
}Mid_Alarm_Param_t;


uint8 Mid_AlarmClock_Check(rtc_time_s *curtime);
void  Mid_AlarmClock_Write(alarm_clock_t *configinfo, eMidAlarmGroup group);
void  Mid_AlarmClock_Read(alarm_clock_t *configinfo, eMidAlarmGroup group);
void CurRingAlarmIdSet(eMidAlarmGroup alarmid);
eMidAlarmGroup CurRingAlarmIdGet(void);
uint8 Mid_AlarmClock_ValidNunRead(void);

// 以下函数没用到
void  Mid_AlarmClock_Switch_Set(uint8 group, uint8 state);
void  Mid_AlarmClock_DelayRing_Set(uint8 group, uint8 state);
uint8 Mid_AlarmClock_Switch_Read(uint8 group);
uint8 Mid_AlarmClock_DelayRing_Read(uint8 group);
void  Mid_AlarmClock_HighHour_Add(uint8 group);
void  Mid_AlarmClock_HighHour_Less(uint8 group);
void  Mid_AlarmClock_LowHour_Add(uint8 group);
void  Mid_AlarmClock_LowHour_Less(uint8 group);
void  Mid_AlarmClock_HighHour_Check(uint8 group);
void  Mid_AlarmClock_HighMin_Add(uint8 group);
void  Mid_AlarmClock_HighMin_Less(uint8 group);
void  Mid_AlarmClock_LowMin_Add(uint8 group);
void  Mid_AlarmClock_LowMin_Less(uint8 group);
void  Mid_AlarmClock_ReptMainSwtich_Set(uint8 group, uint8 reptMainSwitch);
void  Mid_AlarmClock_ReptDaySwtich_Set(uint8 group, uint8 reptDay, uint8 reptDaySwitch);
uint8 Mid_AlarmClock_ReptMainSwtich_Read(uint8 group); 
uint8 Mid_AlarmClock_ReptDaySwtich_Read(uint8 group, uint8 reptDay);
void  Mid_AlarmClock_DelayRing_Close(void);

#endif			//	ALARM_APP_H

