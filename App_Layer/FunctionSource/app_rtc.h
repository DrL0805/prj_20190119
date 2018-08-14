#ifndef     APP_RTC_H
#define     APP_RTC_H





void App_RtcSec_Process(void);

void App_RtcHour_Process(void);

void App_RtcHalfSec_Process(void);

void App_RtcMin_Process(void);

void App_RtcDay_Process(void);



uint16 App_Rtc_ExchangeTimeforCount(uint8 Hour, uint8 Min, uint8 Sec);

uint16_t App_NotDisturdTimeCheck(void);

void App_Rtc_Movt(void);

#endif      //APP_RTC_H
