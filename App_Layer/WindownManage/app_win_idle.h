#ifndef APP_WIN_IDLE_H
#define APP_WIN_IDLE_H

#include "app_win_common.h"
#include "app_win_process.h"

eAppWinHandle App_IdleWin_Init(void);
eAppWinHandle App_IdleWin_Cb(eAppWinHandle WinHandle, App_Win_Msg_T message);

#endif

