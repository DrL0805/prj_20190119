#ifndef APP_WIN_PROCESS_H
#define APP_WIN_PROCESS_H

#include "app_win_common.h"
#include "main.h"

#define APP_WIN_LOCK_TIMEOUT	(10)
#define APP_WIN_RECOVER_TIMEOUT	(300)	// 5min


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
	
	eAppWinHandle		LastWinHanle;		// 上次一级句柄，用于锁屏状态的窗口恢复
	eAppSubWinHandle	LastSubWinHandle;	// 上次二级句柄
	
	uint32_t 			IdleWinCnt;			// 空闲窗口计时，用于锁屏状态
}App_Win_Param_T;

extern eAppWinHandle App_Window_Process(App_Win_Msg_T message);
extern eAppWinHandle App_Window_Init(eAppWinHandle eAppWinNew);
extern void App_Window_LockWinCnt(void);


void App_Win_TaskEventSet(App_Win_Msg_T* msg);
void App_Win_TaskCreate(void);

/* 全局变量声明 */
extern App_Win_Param_T	AppWinParam;

#endif

