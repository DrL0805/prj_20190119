#ifndef APP_WIN_TIME_H
#define APP_WIN_TIME_H

#include "app_win_common.h"
#include "app_win_process.h"

eAppWinHandle App_TimeWin_Init(void);
eAppWinHandle App_TimeWin_Cb(eAppWinHandle WinHandle, App_Win_Msg_T message);

#endif

