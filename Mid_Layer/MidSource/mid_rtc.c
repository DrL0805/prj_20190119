/**********************************************************************
**
**模块说明: mid层RTC接口
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.24  修改流程  ZSL  
**
**********************************************************************/
#include "sm_timer.h"
#include "mid_rtc.h"

#include "mod_time.h"

// 2000年-2099年的闰年标记与第一天的星期
//低7位：表示每年第一天对应的星期
//第8位：1位闰年，0位平年
static const uint8 Table_YearFirstDay[] =
{
    0x0e,0x01,0x02,0x03,0x0c,0x06,0x00,0x01,0x0a,0x04,
    0x05,0x06,0x08,0x02,0x03,0x04,0x0d,0x00,0x01,0x02,
    0x0b,0x05,0x06,0x00,0x09,0x03,0x04,0x05,0x0e,0x01,
    0x02,0x03,0x0c,0x06,0x00,0x01,0x0a,0x04,0x05,0x06,
    0x08,0x02,0x03,0x04,0x0d,0x00,0x01,0x02,0x0b,0x05,
    0x06,0x00,0x09,0x03,0x04,0x05,0x0e,0x01,0x02,0x03,
    0x0c,0x06,0x00,0x01,0x0a,0x04,0x05,0x06,0x08,0x02,
    0x03,0x04,0x0d,0x00,0x01,0x02,0x0b,0x05,0x06,0x00,
    0x09,0x03,0x04,0x05,0x0e,0x01,0x02,0x03,0x0c,0x06,
    0x00,0x01,0x0a,0x04,0x05,0x06,0x08,0x02,0x03,0x04,
};

static const uint16 Table_Monthdyas[] =
{
    //31,28,31,30,31,30,31,31,30,31,30,31
    0,0,31,59,90,120,151,181,212,243,273,304,334,365
};

static const uint32 Table_YearFirstDayUTC[101] =
{
    946684800,978307200,1009843200,1041379200,1072915200,1104537600,1136073600,1167609600,1199145600,1230768000,
    1262304000,1293840000,1325376000,1356998400,1388534400,1420070400,1451606400,1483228800,1514764800,1546300800,
    1577836800,1609459200,1640995200,1672531200,1704067200,1735689600,1767225600,1798761600,1830297600,1861920000,
    1893456000,1924992000,1956528000,1988150400,2019686400,2051222400,2082758400,2114380800,2145916800,2177452800,
    2208988800,2240611200,2272147200,2303683200,2335219200,2366841600,2398377600,2429913600,2461449600,2493072000,
    2524608000,2556144000,2587680000,2619302400,2650838400,2682374400,2713910400,2745532800,2777068800,2808604800,
    2840140800,2871763200,2903299200,2934835200,2966371200,2997993600,3029529600,3061065600,3092601600,3124224000,
    3155760000,3187296000,3218832000,3250454400,3281990400,3313526400,3345062400,3376684800,3408220800,3439756800,
    3471292800,3502915200,3534451200,3565987200,3597523200,3629145600,3660681600,3692217600,3723753600,3755376000,
    3786912000,3818448000,3849984000,3881606400,3913142400,3944678400,3976214400,4007836800,4039372800,4070908800,
    4102444800,
};

//时间变量，保存年月日，时分秒以及星期几
static rtc_time_s  rtcTime;
//半秒计数
static uint8  HalfSecCnt = 0;
//RTC 回调函数

static Mid_RTC_Param_t		Mid_RTC;

//**********************************************************************
// 函数功能: 根据年月日，计算当前日期是星期几
//    输入年月日，使用查表法，计算出是星期几。日期的有效范围为2000/1/1到2099/12/30
// 输入参数：   
//    LocalYear：实际年份 - 2000（LocalYear 的有效范围为0-99）
//    LocalMonth:月份           （LocalMonth的有效范围为1-12）
//    LocalDay:日               （LocalDay  的有效范围为1-31）
// 返回参数：    
// Intervaldays: 星期几
//**********************************************************************
uint16 AutoWeek(uint16 LocalYear,uint8 LocalMonth,uint8 LocalDay)
{
    uint16 Intervaldays ;
    LocalYear = Table_YearFirstDay[(uint8)LocalYear % 100];
    Intervaldays = Table_Monthdyas[LocalMonth];
    if((LocalYear & 0x08) && (LocalMonth > 2))
    {
        Intervaldays++;
    }
    Intervaldays += LocalDay + (LocalYear & 0x07) - 1;
    Intervaldays = Intervaldays % 7;                    
    return Intervaldays;                        //0:Sunday
}

//**********************************************************************
// 函数功能: 每秒执行一次当前函数，生成对应的时分秒，年月日以及星期
//          年份有效范围为：0-99年
// 输入参数：   
//        无
// 返回参数：    
//        无
//**********************************************************************
eMidRTCMsg RtcSecPeriodProcess(void)
{
	eMidRTCMsg tRetVal = eMidRTCMsgSec;
	rtcTime.sec ++;
	if(rtcTime.sec > 59)   //every min
	{
		tRetVal = eMidRTCMsgMin;
		
		rtcTime.min ++;
		rtcTime.sec = 0;
		if(rtcTime.min > 59)   //every Hour
		{
			tRetVal = eMidRTCMsgHour;
			
			rtcTime.min = 0;
			rtcTime.hour ++;
			if(rtcTime.hour > 23)  //every Day
			{
				tRetVal = eMidRTCMsgDay;
				
				rtcTime.hour = 0;
				rtcTime.day ++;
				if(rtcTime.day > MonthDay(rtcTime.year, rtcTime.month))  //every Month
				{
					rtcTime.day = 1;
					rtcTime.month ++;
					if(rtcTime.month > 12)    //every Year
					{
						rtcTime.month = 1;
						rtcTime.year ++;
						if(rtcTime.year > 99)
						{
							rtcTime.year = 0;
						}
                    }
                }
                rtcTime.week = AutoWeek(rtcTime.year, rtcTime.month, rtcTime.day);
            }
        }
    }
	
	return tRetVal;
}

//RTC中断回调函数
void Mid_Rtc_Isr(void)
{
	Mod_Time_TaskMsg_T TimeMsg;
	
	TimeMsg.Id = eTimeTaskMsgRTC;
	TimeMsg.Param.RTC.Msg = eMidRTCMsgHalfSec;
	
	if(Mid_RTC.HalfSecCnt++ > 0)
	{
		Mid_RTC.HalfSecCnt = 0;
		
		TimeMsg.Param.RTC.Msg = RtcSecPeriodProcess();
	}

	Mod_Time_TaskEventSet(&TimeMsg, 1);	
}

//**********************************************************************
// 函数功能: RTC初始化
//       RTC硬件初始化，注册中断回调函数，并初始化时间结构体变量
// 输入参数： 无
// 返回参数：无
//**********************************************************************
void Mid_Rtc_Init(void)
{
	static uint8 hasInit = 0;

	if(hasInit)
		return;

    //open a ctimer for RTC
    SMDrv_CTimer_Open(RTC_CTIMER_MODULE,127,Mid_Rtc_Isr);

	rtcTime.year  	= 18;
	rtcTime.month	= 1;
	rtcTime.day		= 1;
	rtcTime.hour    = 0;
	rtcTime.min		= 0;
	rtcTime.sec		= 0;
	rtcTime.zone    = 0x0800;
	rtcTime.week 	= AutoWeek(rtcTime.year, rtcTime.month, rtcTime.day);;
	hasInit			= 1;
}

//**********************************************************************
// 函数功能: 启动Rtc timer
// 输入参数：无
// 返回参数：无
//**********************************************************************
void Mid_Rtc_Start(void)
{
	SMDrv_CTimer_Start(RTC_CTIMER_MODULE);
}

//**********************************************************************
// 函数功能: 停止Rtc timer
// 输入参数：无  
// 返回参数：无  
//**********************************************************************
void Mid_Rtc_Stop(void)
{
    SMDrv_CTimer_Stop(RTC_CTIMER_MODULE);
}

//**********************************************************************
// 函数功能: 读取当前RTC时间, 把当前的时间保存到时间结构体中
// 输入参数：输入的时间结构体变量，保存当前时间
// 返回参数：无
//**********************************************************************
void Mid_Rtc_TimeRead(rtc_time_s *timeTemp)
{
	timeTemp->year  = rtcTime.year;
	timeTemp->month = rtcTime.month;
	timeTemp->day   = rtcTime.day;
	timeTemp->hour  = rtcTime.hour;
	timeTemp->min   = rtcTime.min;
	timeTemp->sec   = rtcTime.sec;
	timeTemp->zone  = rtcTime.zone;
	timeTemp->week  = rtcTime.week;
}

//**********************************************************************
// 函数功能: 设置RTC时间,   设置系统时间
// 输入参数：输入的时间结构体变量，系统时间参数
// 返回参数：无
//**********************************************************************
void Mid_Rtc_TimeWrite(rtc_time_s *timeTemp)
{
    if( (timeTemp->month > 12) || (timeTemp->day > 31) || (timeTemp->hour > 24) || (timeTemp->min > 60) || (timeTemp->sec > 60) )
    {
        rtcTime.year	= 18;
        rtcTime.month   = 1;
        rtcTime.day     = 1;
        rtcTime.hour	= 0;
        rtcTime.min     = 0; 
        rtcTime.sec     = 0;
        rtcTime.zone    = 0;
    }
    else
    {
        rtcTime.year	= timeTemp->year;
        rtcTime.month   = timeTemp->month;
        rtcTime.day     = timeTemp->day;
        rtcTime.hour	= timeTemp->hour;
        rtcTime.min     = timeTemp->min; 
        rtcTime.sec     = timeTemp->sec;
        rtcTime.zone    = timeTemp->zone;
    }
	
	rtcTime.week	= AutoWeek(rtcTime.year, rtcTime.month, rtcTime.day);
}

//**********************************************************************
// 函数功能: 根据年份和月份，查询月份对应的天数
// 输入参数：   
//        year_1：年份
//        month_1:月份
// 返回参数：    
//        0xff:查询失败
//        其他值：返回月份的天数
//**********************************************************************
uint8 MonthDay(uint16 year_1, uint8 month_1)
{
	uint8 maxday;
	if((year_1%4) == 0)
	{
		if(year_1 % 100)
			maxday = 29;
		else
		{
			if(((year_1+2000)%400 == 0))
			{
				maxday = 29;
			}
			else
			{
				maxday = 28;
			}
		}
	}
	else
	{
		maxday = 28;
	}
	switch(month_1)
	{
		case 1:
			return(31);
		case 2:
			return(maxday);
		case 3:
			return(31);
		case 4:
			return (30);
		case 5:
			return(31);
		case 6:
			return(30);
		case 7:
			return(31);
		case 8:
			return (31);
		case 9:
			return(30);
		case 10:
			return(31);
		case 11:
			return(30);
		case 12:
			return (31);
		default:
			break;
	}
	return 0xff;
}

// ******************************************************************************
// *Funtion name:TimeTransformUtc
// *Description :将时间转换为UTC时间，不考虑时区
// *
// *Input: 当前时间信息,不需要时区信息
// *Output:	0x00:	转换成功
// 			0xff:	转换失败
// /*****************************************************************************
uint16 TimeTransformUtc(rtc_time_s *timeTemp, uint32 *utcTemp)
{
	uint32	CurdayCnt;
	uint32	utcTemp2;
	utcTemp2		= Table_YearFirstDayUTC[timeTemp->year];
	
	CurdayCnt		= Table_Monthdyas[timeTemp->month];

	// 闰年且大于2月多加一天
	if((timeTemp->month > 2) && (Table_YearFirstDay[timeTemp->year] & 0x08))
		CurdayCnt++;
	CurdayCnt		+= timeTemp->day - 1;

    // 当前时区的UTC时间
    utcTemp2			= utcTemp2 + (uint32)CurdayCnt * 86400 
						+ ((uint32)timeTemp->hour*60 + (uint32)timeTemp->min)*60 + timeTemp->sec;

	*utcTemp		= utcTemp2;
	return 0;
}

// ******************************************************************************
// *Funtion name:UtcTransformTime
// *Description :将UTC时间转换为
// *
// *Input: 当前时间信息，包含时区
// *Output:返回0时区的UTC时间
// /*****************************************************************************
uint16 UtcTransformTime(uint32 utcTemp, rtc_time_s *timeTemp)
{
	uint16 i;
	uint16 dayCntTemp;
	uint16 flagTemp;

	uint32  utcTempNew;
	
	flagTemp	= 0;

	// 从2001年开始判断
	for(i = 1; i <= 100; i++)
	{
		if(utcTemp < Table_YearFirstDayUTC[i])
		{
			timeTemp->year = i - 1;
			break;
		}
	}
	if(i == 101)
		return 0xff;

	// 整数天数
	dayCntTemp	= (utcTemp - Table_YearFirstDayUTC[timeTemp->year]) / 86400;

	// 判断是否大于2月28日
	if(dayCntTemp >= 59)
	{
		// 判断是否闰年
		if(Table_YearFirstDay[timeTemp->year] & 0x08)
		{
			// 减去闰月多出来的一天
			dayCntTemp--;
			flagTemp	= 1;
		}

	}

	// 从1月开始查找月份
	for(i = 2; i <= 13; i++)
	{
		if(dayCntTemp < Table_Monthdyas[i])
		{
			timeTemp->month		= i - 1;
			timeTemp->day		= dayCntTemp - Table_Monthdyas[i - 1] + 1;

			if((flagTemp) && (dayCntTemp == 58))
			{
				timeTemp->day	= 29;
			}
			break;
		}
	}	

	// 当天剩余的秒
	utcTemp				= utcTemp % 86400;
	timeTemp->hour		= utcTemp / 3600;

	#if 0	//与time to UTC对应，不加时区
	if (rtcTime.zone & 0x8000)
	{

		timeTemp->hour    -= ((rtcTime.zone & 0x7fff) >> 8);
	}
	else
	{
		timeTemp->hour    += ((rtcTime.zone & 0x7fff) >> 8);
	}
	#endif

	// 当前小时剩余的秒
	utcTemp				= utcTemp % 3600;
	timeTemp->min		= utcTemp / 60;
	timeTemp->sec		= utcTemp % 60;
	
	return 0;
}

// ******************************************************************************
// *Funtion name:TimeZoneTransform
// *Description :根据输入curtime信息，转换为目标时区时间
// *
// *Input:	curTime：当前时间，需要输入正确的当前时间与时区
// *		aimTime：需输入目标时区，其他时间信息不需要输入，
// *Output:	0x00:	转换成功
// *		0xff:	转换失败
// /*****************************************************************************
uint16 TimeZoneTransform(rtc_time_s *curTime, rtc_time_s *aimTime)
{
	uint32	utcTemp, utcTemp2;
	int8		curZoneIntTemp, aimZoneIntTemp, zoneIntTemp;
	uint8		curZoneFractionTemp, aimZoneFractionTemp, zoneFractionTemp;
	uint8		curZoneLargeThanaimZone;
	// 将时间转换为UTC
	if(TimeTransformUtc(curTime, &utcTemp) == 0xff)
		return 0xff;
	// 将时区范围由（-12--+12）转换为（0--24）
	curZoneIntTemp		= (int8)(curTime->zone >> 8) + 12;
	aimZoneIntTemp		= (int8)(aimTime->zone >> 8) + 12;

	curZoneFractionTemp	= (uint8)(curTime->zone);
	aimZoneFractionTemp	= (uint8)(aimTime->zone);

	if(curZoneIntTemp >= aimZoneIntTemp)
	{
		if(curZoneIntTemp == aimZoneIntTemp)
		{
			if(curZoneFractionTemp >= aimZoneFractionTemp)
			{
				curZoneLargeThanaimZone		= 1;
			}
			else
			{
				curZoneLargeThanaimZone		= 0;
			}
		}
		else
		{
			curZoneLargeThanaimZone			= 1;
		}
	}
	else
	{
		curZoneLargeThanaimZone				= 0;
	}

	if(curZoneLargeThanaimZone)
	{
		zoneIntTemp		= curZoneIntTemp - aimZoneIntTemp;
		if(curZoneFractionTemp >= aimZoneFractionTemp)
		{
			zoneFractionTemp	= curZoneFractionTemp - aimZoneFractionTemp;
		}
		else
		{
			zoneFractionTemp	= 100 + curZoneFractionTemp - aimZoneFractionTemp;
			zoneIntTemp--;
		}
	}
	else
	{
		zoneIntTemp		= aimZoneIntTemp - curZoneIntTemp;
		if(aimZoneFractionTemp >= curZoneFractionTemp)
		{
			zoneFractionTemp	= aimZoneFractionTemp - curZoneFractionTemp;
		}
		else
		{
			zoneFractionTemp	= 100 + aimZoneFractionTemp - curZoneFractionTemp;
			zoneIntTemp--;
		}
	}

	utcTemp2	= (uint32)zoneIntTemp * 3600 + (uint32)zoneFractionTemp * 3600 / 100;

	if(curZoneLargeThanaimZone)
	{
		utcTemp	= utcTemp - utcTemp2;
	}
	else
	{
		utcTemp	= utcTemp + utcTemp2;
	}

	return UtcTransformTime(utcTemp, aimTime);
}

// ******************************************************************************
// *Funtion name:Read_GMT_Utc
// *Description :返回0时区的UTC时间
// *
// *Input: data
// *Output:None
// /*****************************************************************************
uint32 Read_GMT_Utc(void)
{
	rtc_time_s		timeTemp;
	uint32		utcTemp;
	// 目的时区为零时区
	timeTemp.zone	= 0;

	if(TimeZoneTransform(&rtcTime, &timeTemp) == 0xff)//当前时间转换成目标时区时间
		return 0xff;

	TimeTransformUtc(&timeTemp, &utcTemp);//当前时间转换成UTC时间
	return utcTemp;
}

// ******************************************************************************
// *Funtion name:Read_Cur_Utc
// *Description :返回当前时间的UTC时间
// *
// *Input: none
// *Output: 0 : 转换失败
// 			其他：UTC时间
// /*****************************************************************************
uint32 Mid_Rtc_ReadCurUtc(void)
{
	uint32 utcTemp;
	if(TimeTransformUtc(&rtcTime, &utcTemp) == 0x00)
		return utcTemp;
	else
		return 0;
}

// ******************************************************************************
// *Funtion name:TimePeriodJudge
// *Description :判断在一天内是否在制定的时间段内
// *
// *Input:	startTime:	时间段开始时刻，包含该时间，单位为秒
// 			endTime:	时间段结束时刻，包含该时间，单位为秒
// 			inputTime:	输入的时刻，判断该时刻是否在时间段内，单位为秒
// *Output: 0 : 不在该时间段内
// 			1 : 在该时间段内
// /*****************************************************************************
uint16 TimePeriodJudge(uint32 startTime, uint32 endTime, uint32 inputTime)
{
	if(startTime <= endTime)
	{
		if((inputTime >= startTime) && (inputTime <= endTime))
			return 1;
		else
			return 0;
	}
	else
	{
		if((inputTime <= endTime) || (inputTime >= startTime))
			return 1;
		else
			return 0;
	}
}

//**********************************************************************
// 函数功能: 根据年月日，计算当前日期是一年中的第几天
//    输入年月日，使用查表法，日期的有效范围为2000/1/1到2099/12/30
// 输入参数：   
//    LocalYear：实际年份 - 2000（LocalYear 的有效范围为0-99）
//    LocalMonth:月份           （LocalMonth的有效范围为1-12）
//    LocalDay:日               （LocalDay  的有效范围为1-31）
// 返回参数：    
// Intervaldays: 一年中的第几天
//**********************************************************************
uint16 Mid_Rtc_AutoDay(uint8 LocalYear,uint8 LocalMonth,uint8 LocalDay)
{
    uint8 yearflag;
    uint16 daytemp = 0;
    uint8  i;

    yearflag    = Table_YearFirstDay[LocalYear % 100];

    for (i = 1; i <= LocalMonth; i++)
    {
        if (i % 2 == 0)
        {
            if (i == 2)
            {
                if (yearflag & 0x08)
                {
                    daytemp += 29;
                }
                else
                {
                    daytemp += 28;
                }
            }
            else
            {
                daytemp += 30;
            }
            
        }
        else
        {
            daytemp += 31;
        }
    } 
    daytemp += LocalDay; 
    return daytemp;  
} 

