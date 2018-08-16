
#include "app_win_idle.h"

//**********************************************************************
// 函数功能：  空闲窗口初始化
// 输入参数：  
// 返回参数：  成功创建的窗口句柄
eAppWinHandle App_IdleWin_Init(void)
{
	APP_WIN_RTT_LOG(0,"App_IdleWin_Init \r\n");
	
	AppWinParam.CurrSubWinHandle = eAppSubWinHandle0;
	
	return eIdleWinHandle;	
}

//**********************************************************************
// 函数功能：  窗口回调函数
// 输入参数：  eAppWinHandle	当前窗口句柄
// 			   App_Win_Msg_T 	传入参数
// 返回参数：  新的窗口句柄
eAppWinHandle App_IdleWin_Cb(eAppWinHandle WinHandle, App_Win_Msg_T message)
{
	APP_WIN_RTT_LOG(0,"App_IdleWin_Cb %d \r\n",WinHandle);

	return WinHandle;
}
