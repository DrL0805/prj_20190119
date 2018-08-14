#ifndef MID_SUNSET_ALGORITHM_H
#define MID_SUNSET_ALGORITHM_H



uint16 Mid_SunRiseSet_TimeCalculate(rtc_time_s rtctime,int16 latitude,int16 longitude);
uint16 Mid_SunRiseSet_SunRiseSetTimeGet(uint16 *sunrisetime, uint16 *sunsettime);


#endif

