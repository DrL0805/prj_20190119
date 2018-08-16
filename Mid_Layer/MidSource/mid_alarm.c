#include "platform_common.h"
#include "mid_rtc.h"
#include "mid_alarm.h"


static Mid_Alarm_Param_t	Mid_Alarm;


//**********************************************************************
// 函数功能:	闹钟贪睡检查。每隔10分钟提醒一次，共提醒五次。
// 输入参数：	
//      无
// 返回参数：
//	    0xff 闹钟响铃；0x00 闹钟不响铃
// 调用：Mid_AlarmClock_Check
uint8 Mid_AlarmClock_DelayRing(void)
{
	if(Mid_Alarm.RingSwitch)
	{
		Mid_Alarm.accTimer++;	
		if(Mid_Alarm.accTimer == 10)
		{
			Mid_Alarm.RingCnt++;
			if(Mid_Alarm.RingCnt > 4)//如果闹钟有效，闹钟第一次响应没有计数
			{
				Mid_Alarm.RingSwitch = 0;
			}
			else
			{
				Mid_Alarm.accTimer = 0;
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
// 调用:Mid_AlarmClock_Check
 void Mid_AlarmClock_DelayRing_Open(void)
{
	Mid_Alarm.RingSwitch 	        = 1;
	Mid_Alarm.accTimer			= 0;
	Mid_Alarm.RingCnt			= 0;

}

//**********************************************************************
// 函数功能:	闹钟贪睡功能关闭
// 输入参数：	
//      无
// 返回参数：
//	    无
 void Mid_AlarmClock_DelayRing_Close(void)
{
	Mid_Alarm.RingSwitch = 0;
}

//**********************************************************************
// 函数功能:检测闹钟状态，RTC每分钟调用一次
// 输入参数：	
//      curtime：当前的RTC time
// 返回参数： 0xff 闹钟响铃；0x00 闹钟不响铃
// 调用:外部调用，每分钟调用一次，检测是否有闹钟产生
uint8 Mid_AlarmClock_Check(rtc_time_s *curtime)
{
	uint8 i;
	uint8 ret = 0;
	
	for(i = 0; i < eMidAlarmGroupNum; i++)
	{
		// 判断当前闹钟是否有效
		if(Mid_Alarm.Clock[i].alarmswitch)			//alarm is open
		{
			// 判断当前闹钟是重复闹钟还是单次闹钟
			if (Mid_Alarm.Clock[i].reptswitch & 0x80)	//repeat
			{
				// 若为重复闹钟，根据当前星期几判断闹钟是响应
				if((Mid_Alarm.Clock[i].reptswitch & (0x01 << curtime->week)) && Mid_Alarm.Clock[i].hour == curtime->hour
					&& Mid_Alarm.Clock[i].min == curtime->min) 
				{	
					Mid_AlarmClock_DelayRing_Open();	
                    CurRingAlarmIdSet(i);					
					return 0xff;	
				}
			}
			else//single
			{
				// 若为单次闹钟，无论星期几都响。
                if(Mid_Alarm.Clock[i].hour == curtime->hour && Mid_Alarm.Clock[i].min == curtime->min)				
			    {
					Mid_AlarmClock_DelayRing_Open();
					CurRingAlarmIdSet(i);
					Mid_Alarm.Clock[i].alarmswitch = 0;	// 只响一次后关闭闹钟
                    return 0xff;					
				}						
			}
		}
	}
	
	// 判断当前是否处于贪睡状态，且贪睡时间到
	ret |= Mid_AlarmClock_DelayRing();
	
	return ret;
}

// 设置当前闹钟ID
// 调用 Mid_AlarmClock_Check
void CurRingAlarmIdSet(eMidAlarmGroup alarmid)
{
    Mid_Alarm.curRingAlarmId = alarmid;
}

// 获取当前闹钟ID
// 调用：外部
eMidAlarmGroup CurRingAlarmIdGet(void)
{
    return Mid_Alarm.curRingAlarmId;
}

//in case phone setting
//**********************************************************************
// 函数功能:某个闹钟配置信息写入
// 输入参数：	
//      configinfo：闹钟的配置信息
//      group ：闹钟ID（0-4）
// 返回参数：无
//	  调用：外部
 void Mid_AlarmClock_Write(alarm_clock_t *configinfo, eMidAlarmGroup group)
{
	if(group >= eMidAlarmGroupNum)
	{
		group = 0;
	}

	Mid_Alarm.Clock[group].reptswitch	= configinfo->reptswitch;
	Mid_Alarm.Clock[group].hour			= configinfo->hour;
	Mid_Alarm.Clock[group].min			= configinfo->min;
	Mid_Alarm.Clock[group].delayswitch	= configinfo->delayswitch;
	Mid_Alarm.Clock[group].alarmswitch	= configinfo->alarmswitch;

	if ((Mid_Alarm.curRingAlarmId == group) && (Mid_Alarm.Clock[group].alarmswitch == 0))
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
// 返回参数：无
//	   调用：外部
 void Mid_AlarmClock_Read(alarm_clock_t *configinfo, eMidAlarmGroup group)
{
	if(group >= eMidAlarmGroupNum)
	{
		group = 0;
	}

	configinfo->reptswitch		= Mid_Alarm.Clock[group].reptswitch;
	configinfo->hour            = Mid_Alarm.Clock[group].hour;
	configinfo->min				= Mid_Alarm.Clock[group].min;
	configinfo->delayswitch		= Mid_Alarm.Clock[group].delayswitch;
	configinfo->alarmswitch		= Mid_Alarm.Clock[group].alarmswitch;
}

//set a alarm groupn status,state =0, the alarm group function is disable, no alarm ring
//**********************************************************************
// 函数功能:设置一个闹钟的开关状态
// 输入参数：	
//      group ：闹钟ID（0-4）
//      state ：闹钟状态（0关闭，1打开）
// 返回参数：无
//	  调用：  无
 void Mid_AlarmClock_Switch_Set(uint8 group, uint8 state)
{
	#if 0
	if(group >= eMidAlarmGroupNum)
	{
		group = 0;
	}
	Mid_Alarm.Clock[group].alarmswitch = state;
	#endif
}

//read a alarm groupn status,state =0, the alarm group function is disable, no alarm ring
//**********************************************************************
// 函数功能:读一个闹钟的开关状态
// 输入参数：
//      group ：闹钟ID（0-4）
// 返回参数：
//     闹钟状态（0关闭，1打开）
// 调用 ：无
 uint8 Mid_AlarmClock_Switch_Read(uint8 group)
{
	#if 0
	if(group >= eMidAlarmGroupNum)
	{
		return 0x00;
	}
	return Mid_Alarm.Clock[group].alarmswitch;
	#endif
}


//read a alarm group delay ring function
//**********************************************************************
// 函数功能:读某个闹钟的贪睡状态
// 输入参数：	
//      group ：闹钟ID（0-4）
// 返回参数：
//     闹钟贪睡状态（ 1：贪睡开启  0：贪睡关闭  ）
// 调用：无
 uint8 Mid_AlarmClock_DelayRing_Read(uint8 group)
{
	#if 0
	if(group >= eMidAlarmGroupNum)
	{
		return 0x00;
	}
	return Mid_Alarm.Clock[group].delayswitch;
	#endif
}

//set a alarm group delay ring function
//**********************************************************************
// 函数功能:设置某个闹钟的贪睡的状态
// 输入参数：	
//      group ：闹钟ID（0-4）
//      state ：设置的状态
// 返回参数：无
//   调用：   无

 void Mid_AlarmClock_DelayRing_Set(uint8 group, uint8 state)
{
	#if 0
	if(group >= eMidAlarmGroupNum)
	{
		group = 0;
	}
	Mid_Alarm.Clock[group].delayswitch = state;
	#endif
}



//for watch device(key) setting
//**********************************************************************
// 函数功能:某个闹钟的Hour十位加一		
// 输入参数：	
//      group ：闹钟ID（0-4）
// 返回参数： 无
// 调用：无
 void Mid_AlarmClock_HighHour_Add(uint8 group)
{
	#if 0
	Mid_Alarm.Clock[group].hour += 10;
	if(Mid_Alarm.Clock[group].hour >= 30)
	{
		Mid_Alarm.Clock[group].hour %= 10;
 	}
	#endif
}

//for watch device(key) setting
//**********************************************************************
// 函数功能:某个闹钟的Hour十位减一
// 输入参数：	
//      group ：闹钟ID（0-4）
// 返回参数：无
//  调用：   无
 void Mid_AlarmClock_HighHour_Less(uint8 group)
{
	#if 0
	if(Mid_Alarm.Clock[group].hour < 10)
	{
		Mid_Alarm.Clock[group].hour += 20;
 	}
	else
	{
		Mid_Alarm.Clock[group].hour -= 10;
	}
	#endif
}

//**********************************************************************
// 函数功能:闹钟的Hour十位有效范围检测并更正，超过23自动减为20
// 输入参数：	
//      group ：闹钟ID（0-4）
// 返回参数：无
//    调用： 无
 void Mid_AlarmClock_HighHour_Check(uint8 group)
{
	#if 0
	if(Mid_Alarm.Clock[group].hour > 23)
	{
		Mid_Alarm.Clock[group].hour = 20;
 	}
	#endif
}

//**********************************************************************
// 函数功能:某个闹钟的Hour个位加一
// 输入参数：	
//      group ：闹钟ID（0-4）
// 返回参数：无
//    调用： 无
 void Mid_AlarmClock_LowHour_Add(uint8 group)
{
	#if 0
	if(Mid_Alarm.Clock[group].hour == 23)
	{
		Mid_Alarm.Clock[group].hour = 20;
 	}
	else if(Mid_Alarm.Clock[group].hour == 19)
	{
		Mid_Alarm.Clock[group].hour = 10;
	}
	else if(Mid_Alarm.Clock[group].hour == 9)
	{
		Mid_Alarm.Clock[group].hour = 0;
	}
	else 
	{
		Mid_Alarm.Clock[group].hour++;
	}
	#endif
}

//**********************************************************************
// 函数功能:某个闹钟的Hour个位减一
// 输入参数：	
//      group ：闹钟ID（0-4）
// 返回参数：无
//    调用： 无
 void Mid_AlarmClock_LowHour_Less(uint8 group)
{
	#if 0
	if(Mid_Alarm.Clock[group].hour == 0)
	{
		Mid_Alarm.Clock[group].hour = 9;
	}
	else if(Mid_Alarm.Clock[group].hour == 10)
	{
		Mid_Alarm.Clock[group].hour = 19;
	}
	else if(Mid_Alarm.Clock[group].hour == 20)
	{
		Mid_Alarm.Clock[group].hour = 23;
	}
	else
	{
		Mid_Alarm.Clock[group].hour--;
	}
	#endif
}

//**********************************************************************
// 函数功能:某个闹钟的Min十位加一
// 输入参数：	
//      group ：闹钟ID（0-4）
// 返回参数：无
//    调用： 无
 void Mid_AlarmClock_HighMin_Add(uint8 group)
{
	#if 0
	Mid_Alarm.Clock[group].min += 10;
	if(Mid_Alarm.Clock[group].min >= 60)
	{
		Mid_Alarm.Clock[group].min %= 10;
 	}
	#endif
}

//**********************************************************************
// 函数功能:某个闹钟的Min十位减一
// 输入参数：	
//      group ：闹钟ID（0-4）
// 返回参数：无
//    调用： 无
 void Mid_AlarmClock_HighMin_Less(uint8 group)
{
	#if 0
	if(Mid_Alarm.Clock[group].min < 10)
	{
		Mid_Alarm.Clock[group].min += 50;
 	}
	else
	{
		Mid_Alarm.Clock[group].min -= 10;
	}
	#endif
}

//**********************************************************************
// 函数功能:某个闹钟的Min个位加一
// 输入参数：	
//      group ：闹钟ID（0-4）
// 返回参数：无
//    调用： 无
 void Mid_AlarmClock_LowMin_Add(uint8 group)
{
	#if 0
	uint8 temp;

	temp = Mid_Alarm.Clock[group].min / 10;
	Mid_Alarm.Clock[group].min = Mid_Alarm.Clock[group].min % 10 + 1;
	if(Mid_Alarm.Clock[group].min > 9)
	{
		Mid_Alarm.Clock[group].min = 0;
	}
	Mid_Alarm.Clock[group].min = temp * 10 + Mid_Alarm.Clock[group].min;
	#endif
}

//**********************************************************************
// 函数功能:某个闹钟的Min个位减一
// 输入参数：	
//      group ：闹钟ID（0-4）
// 返回参数：无
//    调用： 无
 void Mid_AlarmClock_LowMin_Less(uint8 group)
{
	#if 0
	uint8 temp;

	temp = Mid_Alarm.Clock[group].min / 10;
	Mid_Alarm.Clock[group].min = Mid_Alarm.Clock[group].min % 10;
	if(Mid_Alarm.Clock[group].min == 0)
	{
		Mid_Alarm.Clock[group].min = 9;
	}
	else 
	{
		Mid_Alarm.Clock[group].min--;
	}
	Mid_Alarm.Clock[group].min = temp * 10 + Mid_Alarm.Clock[group].min;
	#endif
}


//**********************************************************************
// 函数功能:设置某个闹钟的重复功能
// 输入参数：	
//      group ：闹钟ID（0-4）
//      reptMainSwitch ：设置模式（0：单次， 1：重复）
// 返回参数：无
//     调用： 无
 void Mid_AlarmClock_ReptMainSwtich_Set(uint8 group, uint8 reptMainSwitch)
{
	#if 0
	if(reptMainSwitch)
	{
		Mid_Alarm.Clock[group].reptswitch |= 0x80;
	}
	else
	{
		Mid_Alarm.Clock[group].reptswitch &= 0x7F;
	}
	#endif
}

//**********************************************************************
// 函数功能:读某个闹钟的重复开关状态
// 输入参数：	
//      group ：闹钟ID（0-4）
// 返回参数：
//     闹钟重复开关状态（ 0：单次， 1：重复）
// 调用：无
 uint8 Mid_AlarmClock_ReptMainSwtich_Read(uint8 group)
{
	#if 0
	return Mid_Alarm.Clock[group].reptswitch & 0x80;
	#endif
	return 0;
}

//retDay: sun~sat
//**********************************************************************
// 函数功能:设置重复闹钟某天的开关状态
// 输入参数：	
//      group ：闹钟ID（0-4）
//      reptDay ：星期
//      reptDaySwitch ：闹钟重复开关状态（ 0：单次， 1：重复）
// 返回参数：无
//    调用： 无
 void Mid_AlarmClock_ReptDaySwtich_Set(uint8 group, uint8 reptDay, uint8 reptDaySwitch)
{
	#if 0
	if (reptDay > 6)
	{
		reptDay = 0;
	}

	if (reptDaySwitch)//open (day)repeat alarm function
	{
		Mid_Alarm.Clock[group].reptswitch |= (0x01 << reptDay);
	}
	else
	{
		Mid_Alarm.Clock[group].reptswitch &= ~(0x01 << reptDay);
	}
	#endif
}

//**********************************************************************
// 函数功能:读重复闹钟一周某天的开关状态
// 输入参数：	
//      group ：闹钟ID（0-4）
//      reptDay ：星期      
// 返回参数：
//     reptDaySwitch ：闹钟重复开关状态（ 0：单次， 1：重复）
// 调用：无
 uint8 Mid_AlarmClock_ReptDaySwtich_Read(uint8 group, uint8 reptDay)
{
	#if 0
	uint8 switchtemp = 0;

	if (reptDay > 6)
	{
		return 0x00;
	}
	
	switchtemp	= Mid_Alarm.Clock[group].reptswitch & (0x01 << reptDay);

	return switchtemp;
	#endif
	
	return 0;
}

//**********************************************************************
// 函数功能:读取闹钟的有效个数
// 输入参数：无    
// 返回参数：闹钟的有效个数
// 调用：外部
uint8 Mid_AlarmClock_ValidNunRead(void)
{
	uint8 numCnt;
	uint8 validAlarmNun = 0;

	for (numCnt = 0; numCnt < eMidAlarmGroupNum; numCnt ++)
	{
		//全部为0，闹钟被设置为无效
		if (!(Mid_Alarm.Clock[numCnt].reptswitch == 0 && Mid_Alarm.Clock[numCnt].alarmswitch == 0  && 
			 Mid_Alarm.Clock[numCnt].hour == 0 && Mid_Alarm.Clock[numCnt].min == 0))
		{
				validAlarmNun ++;
		}
	}	
	return validAlarmNun;
}
