#ifndef APP_WIN_PROCESS_H
#define APP_WIN_PROCESS_H

#include "app_win_common.h"
#include "main.h"


typedef 	eAppWinHandle WinInitFunc(void);
typedef 	eAppWinHandle CallBackFunc(eAppWinHandle sysHandle, App_Win_Msg_T message);

typedef struct
{
	uint16 			HandleIndex;
	WinInitFunc 		*WinInit;
	CallBackFunc 		*CallBack;
}APP_WIN_T;

typedef struct
{
	eAppWinHandle		CurrWinHanle;		// 当前一级句柄
	eAppSubWinHandle	CurrSubWinHandle;	// 当前二级句柄
}App_Win_Param_T;

eAppWinHandle App_Window_Process(App_Win_Msg_T message);
eAppWinHandle App_Window_Init(eAppWinHandle eAppWinNew);


void App_Win_TaskEventSet(App_Win_Msg_T* msg);
void App_Win_TaskCreate(void);

/* 全局变量声明 */
extern App_Win_Param_T	AppWinParam;

#endif

