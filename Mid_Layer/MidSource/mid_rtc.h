#ifndef  MID_RTC_H
#define  MID_RTC_H

#include "platform_common.h"

typedef enum
{
    eMidRTCWeekSUN     = 0,
    eMidRTCWeekMON,
    eMidRTCWeekTUE,
    eMidRTCWeekWED,
    eMidRTCWeekTHU,
    eMidRTCWeekFRI,
    eMidRTCWeekSAT,
}eMidRTCWeek;

typedef enum
{
    eMidRTCMonthJAN     = 1,
    eMidRTCMonthFEB,
    eMidRTCMonthMAR,
    eMidRTCMonthAPR,
    eMidRTCMonthMAY,
    eMidRTCMonthJUN,
    eMidRTCMonthJUL,
    eMidRTCMonthAUG,
    eMidRTCMonthSEP,
    eMidRTCMonthOCT,
    eMidRTCMonthNOV,
    eMidRTCMonthDEC,
} eMidRTCMonth;

typedef struct
{
	uint8_t HalfSecCnt;
}Mid_RTC_Param_t;

typedef struct 
{
    uint8  			year;
    eMidRTCMonth  	month;
    uint8  			day;
    uint8  			hour;
    uint8  			min;
    uint8  			sec;
    eMidRTCWeek  	week;
    uint16 			zone;
}rtc_time_s;

typedef enum
{
    eMidRTCMsgDay,     //RTC一天时间计时完成消息
	eMidRTCMsgHour,
    eMidRTCMsgMin,     //RTC每分钟计时完成消息
    eMidRTCMsgSec,     //RTC每秒计时完成消息
    eMidRTCMsgHalfSec, //相比RtcSecCbInit延时半秒，计时完成消息
}eMidRTCMsg;

//**********************************************************************
// 函数功能: RTC初始化
//       RTC硬件初始化，注册中断回调函数，并初始化时间结构体变量
// 输入参数： 无
// 返回参数：无
//**********************************************************************
extern void Mid_Rtc_Init(void);
extern eMidRTCMsg RtcSecPeriodProcess(void);

//**********************************************************************
// 函数功能: 启动Rtc timer
// 输入参数：无
// 返回参数：无
//**********************************************************************
extern void Mid_Rtc_Start(void);

//**********************************************************************
// 函数功能: 停止Rtc timer
// 输入参数：无  
// 返回参数：无  
//**********************************************************************
extern void Mid_Rtc_Stop(void);

//**********************************************************************
// 函数功能: 读取当前RTC时间, 把当前的时间保存到时间结构体中
// 输入参数：输入的时间结构体变量，保存当前时间
// 返回参数：无
//**********************************************************************
extern void Mid_Rtc_TimeRead(rtc_time_s *timeTemp);

//**********************************************************************
// 函数功能: 设置RTC时间,   设置系统时间
// 输入参数：输入的时间结构体变量，系统时间参数
// 返回参数：无
//**********************************************************************
extern void Mid_Rtc_TimeWrite(rtc_time_s *timeTemp);

//**********************************************************************
// 函数功能: 根据年份和月份，查询月份对应的天数
// 输入参数：   
//        year_1：年份
//        month_1:月份
// 返回参数：    
//        0xff:查询失败
//        其他值：返回月份的天数
//**********************************************************************
extern uint8 MonthDay(uint16 year_1, uint8 month_1);

// ******************************************************************************
// *Funtion name:TimeTransformUtc
// *Description :将时间转换为UTC时间，不考虑时区
// *
// *Input: 当前时间信息,不需要时区信息
// *Output:	0x00:	转换成功
// 			0xff:	转换失败
// /*****************************************************************************
extern uint16 TimeTransformUtc(rtc_time_s *timeTemp, uint32 *utcTemp);

// ******************************************************************************
// *Funtion name:UtcTransformTime
// *Description :将UTC时间转换为
// *
// *Input: 当前时间信息，包含时区
// *Output:返回0时区的UTC时间
// /*****************************************************************************
extern uint16 UtcTransformTime(uint32 utcTemp, rtc_time_s *timeTemp);

// ******************************************************************************
// *Funtion name:TimeZoneTransform
// *Description :根据输入curtime信息，转换为目标时区时间
// *
// *Input:	curTime：当前时间，需要输入正确的当前时间与时区
// *		aimTime：需输入目标时区，其他时间信息不需要输入，
// *Output:	0x00:	转换成功
// *		0xff:	转换失败
// /*****************************************************************************
extern uint16 TimeZoneTransform(rtc_time_s *curTime, rtc_time_s *aimTime);

// ******************************************************************************
// *Funtion name:Read_GMT_Utc
// *Description :返回0时区的UTC时间
// *
// *Input: data
// *Output:None
// /*****************************************************************************
extern uint32 Read_GMT_Utc(void);

// ******************************************************************************
// *Funtion name:Read_Cur_Utc
// *Description :返回当前时间的UTC时间
// *
// *Input: none
// *Output: 0 : 转换失败
// 			其他：UTC时间
// /*****************************************************************************
extern uint32 Mid_Rtc_ReadCurUtc(void);

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
extern uint16 TimePeriodJudge(uint32 startTime, uint32 endTime, uint32 inputTime);

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
extern uint16 Mid_Rtc_AutoDay(uint8 LocalYear,uint8 LocalMonth,uint8 LocalDay);
#endif          //  RTC_APP_H

