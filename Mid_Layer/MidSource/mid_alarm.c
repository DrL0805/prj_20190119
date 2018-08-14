#include "platform_common.h"
#include "mid_rtc.h"
#include "mid_alarm.h"

//闹钟结构体数组，保存5次闹钟信息
static alarm_clock_t dailyAlarm[ALARM_CLOCK_GROUP];	//record alarm clock information


static uint8  ringSwtich;
static uint8	delayCnt;
static uint8	accTimer;


static uint8 curRingAlarmId = 0;
//**********************************************************************
// 函数功能:	闹钟贪睡检查。每隔10分钟提醒一次，共提醒五次。
// 输入参数：	
//      无
// 返回参数：
//	    0xff 闹钟响铃；0x00 闹钟不响铃
uint8 Mid_AlarmClock_DelayRing(void)
{
	if(ringSwtich)
	{
		accTimer++;
		if(accTimer == 10)
		{
			delayCnt++;
			if(delayCnt > 4)//如果闹钟有效，闹钟第一次响应没有计数
			{
				ringSwtich = 0;
			}
			else
			{
				accTimer = 0;
				return 0xff;
			}
		}
	}
	return 0x00;
}

//**********************************************************************
// 函数功能:	闹钟贪睡功能打开
// 输入参数：	
//      无
// 返回参数：
//	    无
 void Mid_AlarmClock_DelayRing_Open(void)
{
	#if 0 //使用到闹钟贪睡开关判断时开启
	if (dailyAlarm[curRingAlarmId].delayswitch)
	{
		ringSwtich 	        = 1;
		accTimer			= 0;
		delayCnt			= 0;
	}
	#endif

	ringSwtich 	        = 1;
	accTimer			= 0;
	delayCnt			= 0;

}

//**********************************************************************
// 函数功能:	闹钟贪睡功能关闭
// 输入参数：	
//      无
// 返回参数：
//	    无
 void Mid_AlarmClock_DelayRing_Close(void)
{
	ringSwtich = 0;
}


/****check alarm clock status,call it every one minute******/
//input:current time rtc
//return:alarm status: 
// 0xff:alarm ring, 0x00:no ring
//**********************************************************************
// 函数功能:检测闹钟状态，RTC每分钟调用一次
// 输入参数：	
//      curtime：当前的RTC time
// 返回参数：
//	     0xff 闹钟响铃；0x00 闹钟不响铃
 uint8 Mid_AlarmClock_Check(rtc_time_s *curtime)
{
	uint8 i;
	uint8 ret = 0;

	for(i = 0; i < ALARM_CLOCK_GROUP; i++)
	{
		if(dailyAlarm[i].alarmswitch)			//alarm is open
		{
			if (dailyAlarm[i].reptswitch & 0x80)//repeat
			{
				if((dailyAlarm[i].reptswitch & (0x01 << curtime->week)) && dailyAlarm[i].hour == curtime->hour
					&& dailyAlarm[i].min == curtime->min) 
				{	
					Mid_AlarmClock_DelayRing_Open();	
                    CurRingAlarmIdGet(i);					
					return 0xff;	
				}
			}
			else//single
			{
                if(dailyAlarm[i].hour == curtime->hour && dailyAlarm[i].min == curtime->min)				
			    {
					Mid_AlarmClock_DelayRing_Open();
					CurRingAlarmIdGet(i);
					dailyAlarm[i].alarmswitch = 0;//single ring	
                    return 0xff;					
				}						
			}
		}
	}
	ret |= Mid_AlarmClock_DelayRing();
	return ret;
}

void CurRingAlarmIdGet(uint8_t alarmid)
{
    curRingAlarmId = alarmid;
}

uint8_t Mid_AlarmClock_CurRingAlarmIdReturn(void)
{
    return curRingAlarmId;
}
//in case phone setting
//**********************************************************************
// 函数功能:某个闹钟配置信息写入
// 输入参数：	
//      configinfo：闹钟的配置信息
//      group ：闹钟ID（0-4）
// 返回参数：
//	    无
 void Mid_AlarmClock_Write(alarm_clock_t *configinfo, uint8 group)
{
	if(group >= ALARM_CLOCK_GROUP)
	{
		group = 0;
	}

	dailyAlarm[group].reptswitch	= configinfo->reptswitch;
	dailyAlarm[group].hour			= configinfo->hour;
	dailyAlarm[group].min			= configinfo->min;
	dailyAlarm[group].delayswitch	= configinfo->delayswitch;
	dailyAlarm[group].alarmswitch	= configinfo->alarmswitch;

	if ((curRingAlarmId == group) && (dailyAlarm[group].alarmswitch == 0))
	{
		Mid_AlarmClock_DelayRing_Close();//设置当前在响的闹钟关闭时，贪睡响关闭
	}	
}

//in case phone reading
//**********************************************************************
// 函数功能:某个闹钟配置信息读取
// 输入参数：	
//      configinfo：闹钟的配置信息（传入变量，保存信息）
//      group ：闹钟ID（0-4）
// 返回参数：
//	    无
 void Mid_AlarmClock_Read(alarm_clock_t *configinfo, uint8 group)
{
	if(group >= ALARM_CLOCK_GROUP)
	{
		group = 0;
	}

	configinfo->reptswitch		= dailyAlarm[group].reptswitch;
	configinfo->hour            = dailyAlarm[group].hour;
	configinfo->min				= dailyAlarm[group].min;
	configinfo->delayswitch		= dailyAlarm[group].delayswitch;
	configinfo->alarmswitch		= dailyAlarm[group].alarmswitch;
}

//set a alarm groupn status,state =0, the alarm group function is disable, no alarm ring
//**********************************************************************
// 函数功能:设置一个闹钟的开关状态
// 输入参数：	
//      group ：闹钟ID（0-4）
//      state ：闹钟状态（0关闭，1打开）
// 返回参数：
//	    无
 void Mid_AlarmClock_Switch_Set(uint8 group, uint8 state)
{
	if(group >= ALARM_CLOCK_GROUP)
	{
		group = 0;
	}
	dailyAlarm[group].alarmswitch = state;
}

//read a alarm groupn status,state =0, the alarm group function is disable, no alarm ring
//**********************************************************************
// 函数功能:读一个闹钟的开关状态
// 输入参数：	
//      group ：闹钟ID（0-4）
// 返回参数：
//     闹钟状态（0关闭，1打开）
 uint8 Mid_AlarmClock_Switch_Read(uint8 group)
{
	if(group >= ALARM_CLOCK_GROUP)
	{
		return 0x00;
	}
	return dailyAlarm[group].alarmswitch;
}


//read a alarm group delay ring function
//**********************************************************************
// 函数功能:读某个闹钟的贪睡状态
// 输入参数：	
//      group ：闹钟ID（0-4）
// 返回参数：
//     闹钟贪睡状态（ 1：贪睡开启  0：贪睡关闭  ）
 uint8 Mid_AlarmClock_DelayRing_Read(uint8 group)
{
	if(group >= ALARM_CLOCK_GROUP)
	{
		return 0x00;
	}
	return dailyAlarm[group].delayswitch;
}

//set a alarm group delay ring function
//**********************************************************************
// 函数功能:设置某个闹钟的贪睡的状态
// 输入参数：	
//      group ：闹钟ID（0-4）
//      state ：设置的状态
// 返回参数：
//      无
 void Mid_AlarmClock_DelayRing_Set(uint8 group, uint8 state)
{
	if(group >= ALARM_CLOCK_GROUP)
	{
		group = 0;
	}
	dailyAlarm[group].delayswitch = state;
}



//for watch device(key) setting
//**********************************************************************
// 函数功能:某个闹钟的Hour十位加一
// 输入参数：	
//      group ：闹钟ID（0-4）
// 返回参数：
//     无
 void Mid_AlarmClock_HighHour_Add(uint8 group)
{
	dailyAlarm[group].hour += 10;
	if(dailyAlarm[group].hour >= 30)
	{
		dailyAlarm[group].hour %= 10;
 	}
}

//for watch device(key) setting
//**********************************************************************
// 函数功能:某个闹钟的Hour十位减一
// 输入参数：	
//      group ：闹钟ID（0-4）
// 返回参数：
//     无
 void Mid_AlarmClock_HighHour_Less(uint8 group)
{
	if(dailyAlarm[group].hour < 10)
	{
		dailyAlarm[group].hour += 20;
 	}
	else
	{
		dailyAlarm[group].hour -= 10;
	}
}

//**********************************************************************
// 函数功能:闹钟的Hour十位有效范围检测并更正，超过23自动减为20
// 输入参数：	
//      group ：闹钟ID（0-4）
// 返回参数：
//     无
 void Mid_AlarmClock_HighHour_Check(uint8 group)
{
	if(dailyAlarm[group].hour > 23)
	{
		dailyAlarm[group].hour = 20;
 	}
}

//**********************************************************************
// 函数功能:某个闹钟的Hour个位加一
// 输入参数：	
//      group ：闹钟ID（0-4）
// 返回参数：
//     无
 void Mid_AlarmClock_LowHour_Add(uint8 group)
{
	if(dailyAlarm[group].hour == 23)
	{
		dailyAlarm[group].hour = 20;
 	}
	else if(dailyAlarm[group].hour == 19)
	{
		dailyAlarm[group].hour = 10;
	}
	else if(dailyAlarm[group].hour == 9)
	{
		dailyAlarm[group].hour = 0;
	}
	else 
	{
		dailyAlarm[group].hour++;
	}
}

//**********************************************************************
// 函数功能:某个闹钟的Hour个位减一
// 输入参数：	
//      group ：闹钟ID（0-4）
// 返回参数：
//     无
 void Mid_AlarmClock_LowHour_Less(uint8 group)
{
	if(dailyAlarm[group].hour == 0)
	{
		dailyAlarm[group].hour = 9;
	}
	else if(dailyAlarm[group].hour == 10)
	{
		dailyAlarm[group].hour = 19;
	}
	else if(dailyAlarm[group].hour == 20)
	{
		dailyAlarm[group].hour = 23;
	}
	else
	{
		dailyAlarm[group].hour--;
	}
}

//**********************************************************************
// 函数功能:某个闹钟的Min十位加一
// 输入参数：	
//      group ：闹钟ID（0-4）
// 返回参数：
//     无
 void Mid_AlarmClock_HighMin_Add(uint8 group)
{
	dailyAlarm[group].min += 10;
	if(dailyAlarm[group].min >= 60)
	{
		dailyAlarm[group].min %= 10;
 	}
}

//**********************************************************************
// 函数功能:某个闹钟的Min十位减一
// 输入参数：	
//      group ：闹钟ID（0-4）
// 返回参数：
//     无
 void Mid_AlarmClock_HighMin_Less(uint8 group)
{
	if(dailyAlarm[group].min < 10)
	{
		dailyAlarm[group].min += 50;
 	}
	else
	{
		dailyAlarm[group].min -= 10;
	}
}

//**********************************************************************
// 函数功能:某个闹钟的Min个位加一
// 输入参数：	
//      group ：闹钟ID（0-4）
// 返回参数：
//     无
 void Mid_AlarmClock_LowMin_Add(uint8 group)
{
	uint8 temp;

	temp = dailyAlarm[group].min / 10;
	dailyAlarm[group].min = dailyAlarm[group].min % 10 + 1;
	if(dailyAlarm[group].min > 9)
	{
		dailyAlarm[group].min = 0;
	}
	dailyAlarm[group].min = temp * 10 + dailyAlarm[group].min;
}

//**********************************************************************
// 函数功能:某个闹钟的Min个位减一
// 输入参数：	
//      group ：闹钟ID（0-4）
// 返回参数：
//     无
 void Mid_AlarmClock_LowMin_Less(uint8 group)
{
	uint8 temp;

	temp = dailyAlarm[group].min / 10;
	dailyAlarm[group].min = dailyAlarm[group].min % 10;
	if(dailyAlarm[group].min == 0)
	{
		dailyAlarm[group].min = 9;
	}
	else 
	{
		dailyAlarm[group].min--;
	}
	dailyAlarm[group].min = temp * 10 + dailyAlarm[group].min;
}


//**********************************************************************
// 函数功能:设置某个闹钟的重复功能
// 输入参数：	
//      group ：闹钟ID（0-4）
//      reptMainSwitch ：设置模式（0：单次， 1：重复）
// 返回参数：
//      无
 void Mid_AlarmClock_ReptMainSwtich_Set(uint8 group, uint8 reptMainSwitch)
{
	if(reptMainSwitch)
	{
		dailyAlarm[group].reptswitch |= 0x80;
	}
	else
	{
		dailyAlarm[group].reptswitch &= 0x7F;
	}
}

//**********************************************************************
// 函数功能:读某个闹钟的重复开关状态
// 输入参数：	
//      group ：闹钟ID（0-4）
// 返回参数：
//     闹钟重复开关状态（ 0：单次， 1：重复）
 uint8 Mid_AlarmClock_ReptMainSwtich_Read(uint8 group)
{
	return dailyAlarm[group].reptswitch & 0x80;
}

//retDay: sun~sat
//**********************************************************************
// 函数功能:设置重复闹钟某天的开关状态
// 输入参数：	
//      group ：闹钟ID（0-4）
//      reptDay ：星期
//      reptDaySwitch ：闹钟重复开关状态（ 0：单次， 1：重复）
// 返回参数：
//     无
 void Mid_AlarmClock_ReptDaySwtich_Set(uint8 group, uint8 reptDay, uint8 reptDaySwitch)
{
	
	if (reptDay > 6)
	{
		reptDay = 0;
	}

	if (reptDaySwitch)//open (day)repeat alarm function
	{
		dailyAlarm[group].reptswitch |= (0x01 << reptDay);
	}
	else
	{
		dailyAlarm[group].reptswitch &= ~(0x01 << reptDay);
	}
}

//**********************************************************************
// 函数功能:读重复闹钟一周某天的开关状态
// 输入参数：	
//      group ：闹钟ID（0-4）
//      reptDay ：星期      
// 返回参数：
//     reptDaySwitch ：闹钟重复开关状态（ 0：单次， 1：重复）
 uint8 Mid_AlarmClock_ReptDaySwtich_Read(uint8 group, uint8 reptDay)
{
	uint8 switchtemp = 0;

	if (reptDay > 6)
	{
		return 0x00;
	}
	
	switchtemp	= dailyAlarm[group].reptswitch & (0x01 << reptDay);

	return switchtemp;
}

//**********************************************************************
// 函数功能:读取闹钟的有效个数
// 输入参数：无    
// 返回参数：闹钟的有效个数
uint8 Mid_AlarmClock_ValidNunRead(void)
{
	uint8 numCnt;
	uint8 validAlarmNun = 0;

	for (numCnt = 0; numCnt < ALARM_CLOCK_GROUP; numCnt ++)
	{
		//全部为0，闹钟被设置为无效
		if (!(dailyAlarm[numCnt].reptswitch == 0 && dailyAlarm[numCnt].alarmswitch == 0  && 
			 dailyAlarm[numCnt].hour == 0 && dailyAlarm[numCnt].min == 0))
		{
				validAlarmNun ++;
		}
	}	
	return validAlarmNun;
}
