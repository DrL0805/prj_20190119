#ifndef     APP_RTC_H
#define     APP_RTC_H


uint16 App_Rtc_ExchangeTimeforCount(uint8 Hour, uint8 Min, uint8 Sec);

uint16_t App_NotDisturdTimeCheck(void);

void App_Rtc_Movt(void);

#endif      //APP_RTC_H
