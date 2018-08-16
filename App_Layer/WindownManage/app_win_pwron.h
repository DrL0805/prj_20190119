#ifndef APP_WIN_PWRON_H
#define APP_WIN_PWRON_H

#include "app_win_common.h"
#include "app_win_process.h"

eAppWinHandle App_PwronWin_Init(void);
eAppWinHandle App_PwronWin_Cb(eAppWinHandle WinHandle, App_Win_Msg_T message);


#endif

