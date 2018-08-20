
#include "app_win_idle.h"

static eAppWinHandle App_Win_KeyMenuHandler(eAppWinHandle WinHandle,App_Win_Msg_T message);


#define AppLockWinMenuNum (sizeof(AppLockWinMenu)/sizeof(AppLockWinMenu[0]))	
App_Win_Menu_T	AppLockWinMenu[] = 
{
	{eWinMenukey, App_Win_KeyMenuHandler},
};

//**********************************************************************
// 函数功能：  窗口菜单处理函数
// 输入参数：  WinHandle	当前窗口句柄
// 				message		传入参数
// 返回参数：  成功创建的窗口句柄
static eAppWinHandle App_Win_KeyMenuHandler(eAppWinHandle WinHandle,App_Win_Msg_T message)
{
	APP_WIN_RTT_LOG(0,"App_Win_KeyMenuHandler \r\n");
	
	eAppWinHandle TmpWinHandle = WinHandle;
	
	switch(message.val)
	{
		case MID_KEY0_SHORT:
			
			break;
		case MID_KEY0_HOLDSHORT:
			break;
		case MID_KEY0_HOLDLONG:
			/* 时间模式下长按12秒进入仓储模式*/
			TmpWinHandle = eStoreWinHandle;
			break;
		default: break;
	}
	
	return TmpWinHandle;	
}

//**********************************************************************
// 函数功能：  空闲窗口初始化
// 输入参数：  
// 返回参数：  成功创建的窗口句柄
eAppWinHandle App_LockWin_Init(void)
{
	APP_WIN_RTT_LOG(0,"App_LockWin_Init \r\n");
	
	AppWinParam.CurrSubWinHandle = eAppSubWinHandle0;
	
	return eLockWinHandle;	
}

//**********************************************************************
// 函数功能：  窗口回调函数
// 输入参数：  eAppWinHandle	当前窗口句柄
// 			   App_Win_Msg_T 	传入参数
// 返回参数：  新的窗口句柄
eAppWinHandle App_LockWin_Cb(eAppWinHandle WinHandle, App_Win_Msg_T message)
{
	APP_WIN_RTT_LOG(0,"App_LockWin_Cb %d \r\n",WinHandle);

	return WinHandle;
}
