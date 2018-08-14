#ifndef			MID_ALARM_H
#define			MID_ALARM_H



#define ALARM_CLOCK_GROUP 		(5)//group number of alarm clock

typedef struct 
{
	uint8 reptswitch;			//bit0~bit6: sun~sat,bit7:overall switch
	uint8 hour;
	uint8 min;
	uint8 delayswitch;	    //1: delay alarm enable, 0: delay alarm disable
	uint8 alarmswitch;	    //1: alarm open,         0: alarm close
}alarm_clock_t;



uint8 Mid_AlarmClock_Check(rtc_time_s *curtime);
void  Mid_AlarmClock_Write(alarm_clock_t *configinfo, uint8 group);
void  Mid_AlarmClock_Read(alarm_clock_t *configinfo, uint8 group);
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
uint8 Mid_AlarmClock_ValidNunRead(void);
void CurRingAlarmIdGet(uint8_t alarmid);
uint8_t Mid_AlarmClock_CurRingAlarmIdReturn(void);

#endif			//	ALARM_APP_H

