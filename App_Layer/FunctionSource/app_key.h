#ifndef         APP_KEY_H
#define         APP_KEY_H



void App_KeyProcess(uint16 keyMsg);
void App_SwitchWinInit(void (*TimerCb)(TimerHandle_t xTimer));

void App_SwitchWinStart(void);

void App_SwitchWinStop(void);
void App_SwitchWinProcess(void);

#endif          //APP_KEY_H

