/**********************************************************************
**
**ģ��˵��: mid��RTC�ӿ�
**����汾���޸���־(ʱ�䣬����),�޸���:
**   V1.0   2018.4.24  �޸�����  ZSL  
**
**********************************************************************/
#include "sm_timer.h"
#include "mid_rtc.h"

#include "mod_time.h"

// 2000��-2099������������һ�������
//��7λ����ʾÿ���һ���Ӧ������
//��8λ��1λ���꣬0λƽ��
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

//ʱ����������������գ�ʱ�����Լ����ڼ�
static rtc_time_s  rtcTime;
//�������
static uint8  HalfSecCnt = 0;
//RTC �ص�����

static Mid_RTC_Param_t		Mid_RTC;

//**********************************************************************
// ��������: ���������գ����㵱ǰ���������ڼ�
//    ���������գ�ʹ�ò��������������ڼ������ڵ���Ч��ΧΪ2000/1/1��2099/12/30
// ���������   
//    LocalYear��ʵ����� - 2000��LocalYear ����Ч��ΧΪ0-99��
//    LocalMonth:�·�           ��LocalMonth����Ч��ΧΪ1-12��
//    LocalDay:��               ��LocalDay  ����Ч��ΧΪ1-31��
// ���ز�����    
// Intervaldays: ���ڼ�
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
// ��������: ÿ��ִ��һ�ε�ǰ���������ɶ�Ӧ��ʱ���룬�������Լ�����
//          �����Ч��ΧΪ��0-99��
// ���������   
//        ��
// ���ز�����    
//        ��
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

//RTC�жϻص�����
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
// ��������: RTC��ʼ��
//       RTCӲ����ʼ����ע���жϻص�����������ʼ��ʱ��ṹ�����
// ��������� ��
// ���ز�������
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
// ��������: ����Rtc timer
// �����������
// ���ز�������
//**********************************************************************
void Mid_Rtc_Start(void)
{
	SMDrv_CTimer_Start(RTC_CTIMER_MODULE);
}

//**********************************************************************
// ��������: ֹͣRtc timer
// �����������  
// ���ز�������  
//**********************************************************************
void Mid_Rtc_Stop(void)
{
    SMDrv_CTimer_Stop(RTC_CTIMER_MODULE);
}

//**********************************************************************
// ��������: ��ȡ��ǰRTCʱ��, �ѵ�ǰ��ʱ�䱣�浽ʱ��ṹ����
// ��������������ʱ��ṹ����������浱ǰʱ��
// ���ز�������
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
// ��������: ����RTCʱ��,   ����ϵͳʱ��
// ��������������ʱ��ṹ�������ϵͳʱ�����
// ���ز�������
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
// ��������: ������ݺ��·ݣ���ѯ�·ݶ�Ӧ������
// ���������   
//        year_1�����
//        month_1:�·�
// ���ز�����    
//        0xff:��ѯʧ��
//        ����ֵ�������·ݵ�����
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
// *Description :��ʱ��ת��ΪUTCʱ�䣬������ʱ��
// *
// *Input: ��ǰʱ����Ϣ,����Ҫʱ����Ϣ
// *Output:	0x00:	ת���ɹ�
// 			0xff:	ת��ʧ��
// /*****************************************************************************
uint16 TimeTransformUtc(rtc_time_s *timeTemp, uint32 *utcTemp)
{
	uint32	CurdayCnt;
	uint32	utcTemp2;
	utcTemp2		= Table_YearFirstDayUTC[timeTemp->year];
	
	CurdayCnt		= Table_Monthdyas[timeTemp->month];

	// �����Ҵ���2�¶��һ��
	if((timeTemp->month > 2) && (Table_YearFirstDay[timeTemp->year] & 0x08))
		CurdayCnt++;
	CurdayCnt		+= timeTemp->day - 1;

    // ��ǰʱ����UTCʱ��
    utcTemp2			= utcTemp2 + (uint32)CurdayCnt * 86400 
						+ ((uint32)timeTemp->hour*60 + (uint32)timeTemp->min)*60 + timeTemp->sec;

	*utcTemp		= utcTemp2;
	return 0;
}

// ******************************************************************************
// *Funtion name:UtcTransformTime
// *Description :��UTCʱ��ת��Ϊ
// *
// *Input: ��ǰʱ����Ϣ������ʱ��
// *Output:����0ʱ����UTCʱ��
// /*****************************************************************************
uint16 UtcTransformTime(uint32 utcTemp, rtc_time_s *timeTemp)
{
	uint16 i;
	uint16 dayCntTemp;
	uint16 flagTemp;

	uint32  utcTempNew;
	
	flagTemp	= 0;

	// ��2001�꿪ʼ�ж�
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

	// ��������
	dayCntTemp	= (utcTemp - Table_YearFirstDayUTC[timeTemp->year]) / 86400;

	// �ж��Ƿ����2��28��
	if(dayCntTemp >= 59)
	{
		// �ж��Ƿ�����
		if(Table_YearFirstDay[timeTemp->year] & 0x08)
		{
			// ��ȥ���¶������һ��
			dayCntTemp--;
			flagTemp	= 1;
		}

	}

	// ��1�¿�ʼ�����·�
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

	// ����ʣ�����
	utcTemp				= utcTemp % 86400;
	timeTemp->hour		= utcTemp / 3600;

	#if 0	//��time to UTC��Ӧ������ʱ��
	if (rtcTime.zone & 0x8000)
	{

		timeTemp->hour    -= ((rtcTime.zone & 0x7fff) >> 8);
	}
	else
	{
		timeTemp->hour    += ((rtcTime.zone & 0x7fff) >> 8);
	}
	#endif

	// ��ǰСʱʣ�����
	utcTemp				= utcTemp % 3600;
	timeTemp->min		= utcTemp / 60;
	timeTemp->sec		= utcTemp % 60;
	
	return 0;
}

// ******************************************************************************
// *Funtion name:TimeZoneTransform
// *Description :��������curtime��Ϣ��ת��ΪĿ��ʱ��ʱ��
// *
// *Input:	curTime����ǰʱ�䣬��Ҫ������ȷ�ĵ�ǰʱ����ʱ��
// *		aimTime��������Ŀ��ʱ��������ʱ����Ϣ����Ҫ���룬
// *Output:	0x00:	ת���ɹ�
// *		0xff:	ת��ʧ��
// /*****************************************************************************
uint16 TimeZoneTransform(rtc_time_s *curTime, rtc_time_s *aimTime)
{
	uint32	utcTemp, utcTemp2;
	int8		curZoneIntTemp, aimZoneIntTemp, zoneIntTemp;
	uint8		curZoneFractionTemp, aimZoneFractionTemp, zoneFractionTemp;
	uint8		curZoneLargeThanaimZone;
	// ��ʱ��ת��ΪUTC
	if(TimeTransformUtc(curTime, &utcTemp) == 0xff)
		return 0xff;
	// ��ʱ����Χ�ɣ�-12--+12��ת��Ϊ��0--24��
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
// *Description :����0ʱ����UTCʱ��
// *
// *Input: data
// *Output:None
// /*****************************************************************************
uint32 Read_GMT_Utc(void)
{
	rtc_time_s		timeTemp;
	uint32		utcTemp;
	// Ŀ��ʱ��Ϊ��ʱ��
	timeTemp.zone	= 0;

	if(TimeZoneTransform(&rtcTime, &timeTemp) == 0xff)//��ǰʱ��ת����Ŀ��ʱ��ʱ��
		return 0xff;

	TimeTransformUtc(&timeTemp, &utcTemp);//��ǰʱ��ת����UTCʱ��
	return utcTemp;
}

// ******************************************************************************
// *Funtion name:Read_Cur_Utc
// *Description :���ص�ǰʱ���UTCʱ��
// *
// *Input: none
// *Output: 0 : ת��ʧ��
// 			������UTCʱ��
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
// *Description :�ж���һ�����Ƿ����ƶ���ʱ�����
// *
// *Input:	startTime:	ʱ��ο�ʼʱ�̣�������ʱ�䣬��λΪ��
// 			endTime:	ʱ��ν���ʱ�̣�������ʱ�䣬��λΪ��
// 			inputTime:	�����ʱ�̣��жϸ�ʱ���Ƿ���ʱ����ڣ���λΪ��
// *Output: 0 : ���ڸ�ʱ�����
// 			1 : �ڸ�ʱ�����
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
// ��������: ���������գ����㵱ǰ������һ���еĵڼ���
//    ���������գ�ʹ�ò�������ڵ���Ч��ΧΪ2000/1/1��2099/12/30
// ���������   
//    LocalYear��ʵ����� - 2000��LocalYear ����Ч��ΧΪ0-99��
//    LocalMonth:�·�           ��LocalMonth����Ч��ΧΪ1-12��
//    LocalDay:��               ��LocalDay  ����Ч��ΧΪ1-31��
// ���ز�����    
// Intervaldays: һ���еĵڼ���
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

