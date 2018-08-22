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
	eMidAlarmGroupNum,	// ����������������ڴ�������Ч 
}eMidAlarmGroup;

typedef struct 
{
	uint8 reptswitch;			// �����ظ����� bit0~bit6: sun~sat,bit7:�ظ�����
	uint8 hour;
	uint8 min;
	uint8 delayswitch;	    // ����̰˯���� 1: delay alarm enable, 0: delay alarm disable
	uint8 alarmswitch;	    // ���ӿ��� 1: alarm open,         0: alarm close
}alarm_clock_t;


typedef struct
{
	uint8_t 			RingSwitch;		// ����̰˯���ܿ��أ�1 ��ǰ���ڴ���̰˯ģʽ
	uint8_t 			RingCnt;		// ̰˯���������5��
	uint8_t 			accTimer;		// ̰˯ʱ���ۻ���ÿ10����һ��
	eMidAlarmGroup 		curRingAlarmId;	// ��ǰ���Ӿ����ͨ����ȡ���ֵ���ж��ĸ��������ˣ�
	
	alarm_clock_t	Clock[eMidAlarmGroupNum];	// ��������
}Mid_Alarm_Param_t;


uint8 Mid_AlarmClock_Check(rtc_time_s *curtime);
void  Mid_AlarmClock_Write(alarm_clock_t *configinfo, eMidAlarmGroup group);
void  Mid_AlarmClock_Read(alarm_clock_t *configinfo, eMidAlarmGroup group);
void CurRingAlarmIdSet(eMidAlarmGroup alarmid);
eMidAlarmGroup CurRingAlarmIdGet(void);
uint8 Mid_AlarmClock_ValidNunRead(void);

// ���º���û�õ�
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

