#ifndef APP_WIN_HISTORY_H
#define APP_WIN_HISTORY_H

#include "app_win_common.h"
#include "app_win_process.h"

eAppWinHandle App_HistoryWin_Init(void);
eAppWinHandle App_HistoryWin_Cb(eAppWinHandle WinHandle, App_Win_Msg_T message);

#endif




