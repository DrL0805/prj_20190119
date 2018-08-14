#include "platform_common.h"
#include "mid_rtc.h"
#include "algorithm_usual.h"
#include "mid_sunset_scene.h"




/*******************macro define*******************/
//none



/*******************variable define*******************/
//日落日出时间
static uint16    	sunRiseTime;
static uint16 	sunSetTime;



/*******************function define*******************/

//**********************************************************************
// 函数功能:	计算日出日落时间，单位为分
// 输入参数：	
// rtctime : 	当前地区日期信息
// latitude:    本地纬度，单位：分
// longitude：  本地经度，单位：分
// 返回参数：	
// 0x00	   : 	操作成功
// 0xff    : 	操作失败
uint16 Mid_SunRiseSet_TimeCalculate(rtc_time_s rtctime,int16 latitude,int16 longitude)
{
	float	 zonetemp;
	uint16 zonetemp1;
	uint16 zonetemp2;
	float 	 latitudetemp;// 纬度
	float 	 longitudetemp;//经度
	uint16 daytemp;
	float 	 x1,x2,x;
	float 	 errangle;
	float 	 basetemp1,basetemp2;

	zonetemp1  	= (rtctime.zone & 0x7fff) >> 8;
	zonetemp2  	= (uint8)(rtctime.zone & 0x00ff);
	zonetemp 	= (float)zonetemp1 + (float)zonetemp2 / 100.0f; 
	if (rtctime.zone & 0x8000)
	{
		zonetemp *= -1.0f;
	}

	daytemp 	= Mid_Rtc_AutoDay(rtctime.year,rtctime.month, rtctime.day);

	latitudetemp 	= (float)latitude / 60.0f;
	longitudetemp 	= (float)longitude / 60.0f;


	x1 			= (float)PI * latitudetemp   / 180.0f;
	
	x1 			= Algorithm_sin(x1) / Algorithm_cos(x1);

	x2 			= 2 * PI * (daytemp + 9) / 365;
	x2 			= 10547 * PI / 81000 * Algorithm_cos(x2);
	x2 			= Algorithm_sin(x2) / Algorithm_cos(x2);

	x 			= x1 * x2;

	basetemp1 	= 180 + zonetemp * 15.0f - longitudetemp;
	basetemp2 	= Algorithm_arccos(x) * 180.0f / (float)PI;

	x1  		= ((basetemp1 - basetemp2) / 15) * 60 ;//单位为分
	x2 			= ((basetemp1 + basetemp2) / 15) * 60;//单位为分

	//误差修正
	errangle 	= (float)PI / 182.5f;

	x1 			= x1 + 0.4f *  Algorithm_arccos(errangle * daytemp);
	x2 			= x2 - (0.3f *  Algorithm_arccos(errangle * daytemp)) + 0.16f;//0.16 = 10 / 60

	sunRiseTime = (uint16)x1;
	sunSetTime 	= (uint16)x2;
	
	return 0;
}

//**********************************************************************
// 函数功能:	获取日落日出时间，24小时制，单位为分
// 输入参数：	
// cb      : 	回调函数 
// 返回参数：	
// 0x00	   : 	操作成功
// 0xff    : 	操作失败
uint16 Mid_SunRiseSet_SunRiseSetTimeGet(uint16 *sunrisetime, uint16 *sunsettime)
{
	*sunrisetime    = sunRiseTime;
	*sunsettime 	= sunSetTime;
	return 0;
}
