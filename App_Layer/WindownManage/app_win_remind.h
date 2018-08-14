#ifndef APP_WIN_REMIND_H
#define APP_WIN_REMIND_H




uint16 App_Remind_Win_Init(void);
uint16 App_Remind_Win_Bak(uint16 sysHandle,menuMessage message);
uint16 App_Remind_Win_CallBack(uint16 sysHandle,uint16 tagMenu,menuMessage message);

uint16 App_Remind_Win_MissCallDelete(void);
uint16 App_Remind_Win_MissCallAdd(void);
uint16 App_Remind_Win_MissCallRead(void);
//**********************************************************************
// 函数功能：  来接来电个数清零
// 输入参数：  无	
// 返回参数：  无
void App_Remind_Win_MissCallClear(void);

#endif

