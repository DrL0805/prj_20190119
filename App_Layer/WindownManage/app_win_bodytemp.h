#ifndef APP_WIN_BODYTEMP_H
#define APP_WIN_BODYTEMP_H

#include "app_win_common.h"
#include "app_win_process.h"


// 计步窗口参数定义
typedef struct
{
	uint8_t Id;
}App_BodyTempWin_Param_T;

eAppWinHandle App_BodyTempWin_Init(void);
eAppWinHandle App_BodyTempWin_Cb(eAppWinHandle WinHandle, App_Win_Msg_T message);

#endif

