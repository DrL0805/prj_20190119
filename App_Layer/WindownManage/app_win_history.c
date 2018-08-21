
#include "app_win_history.h"


static eAppWinHandle App_Win_KeyMenuHandler(eAppWinHandle WinHandle,App_Win_Msg_T message);
static eAppWinHandle App_Win_SlideMenuHandler(eAppWinHandle WinHandle,App_Win_Msg_T message);
static eAppWinHandle App_Win_ClickMenuHandler(eAppWinHandle WinHandle,App_Win_Msg_T message);
static eAppWinHandle App_Win_LockMenuHandler(eAppWinHandle WinHandle,App_Win_Msg_T message);


#define AppHistoryWinMenuNum (sizeof(AppHistoryWinMenu)/sizeof(AppHistoryWinMenu[0]))	
App_Win_Menu_T	AppHistoryWinMenu[] = 
{
	{eWinMenukey, App_Win_KeyMenuHandler},
	{eWinMenuSlide, App_Win_SlideMenuHandler},
	{eWinMenuClick, App_Win_ClickMenuHandler},
	{eWinMenuLock, App_Win_LockMenuHandler},
};

//**********************************************************************
// 函数功能：  窗口菜单处理函数
// 输入参数：  WinHandle	当前窗口句柄
// 				message		传入参数
// 返回参数：  成功创建的窗口句柄
static eAppWinHandle App_Win_KeyMenuHandler(eAppWinHandle WinHandle,App_Win_Msg_T message)
{
	APP_WIN_RTT_LOG(0,"App_Win_KeyMenuHandler \r\n");

	switch(message.val)
	{
		case MID_KEY0_SHORT:
			/* 非时间模式下短按进入时间模式 */
			AppWinParam.CurrWinHanle = eTimeWinHandle;
			break;
		case MID_KEY0_HOLDSHORT:
			break;
		case MID_KEY0_HOLDLONG:
			/* 时间模式下长按12秒进入仓储模式*/
			AppWinParam.CurrWinHanle = eStoreWinHandle;
			break;
		default: break;
	}
	
	return AppWinParam.CurrWinHanle;	
}

//**********************************************************************
// 函数功能：  窗口菜单处理函数
// 输入参数：  WinHandle	当前窗口句柄
// 				message		传入参数
// 返回参数：  成功创建的窗口句柄
static eAppWinHandle App_Win_SlideMenuHandler(eAppWinHandle WinHandle,App_Win_Msg_T message)
{
	APP_WIN_RTT_LOG(0,"App_Win_SlideMenuHandler \r\n");

	switch(message.val)
	{
		case 0:		// 上滑
			AppWinParam.CurrWinHanle = eTimeWinHandle;
			break;
		case 1:		// 下滑
			AppWinParam.CurrWinHanle = eABCWinHandle;
			break;
		case 2:		// 左滑
			break;
		case 3:		// 右滑
			break;
		default: break;
	}
	
	return AppWinParam.CurrWinHanle;
}

//**********************************************************************
// 函数功能：  窗口菜单处理函数
// 输入参数：  WinHandle	当前窗口句柄
// 				message		传入参数
// 返回参数：  成功创建的窗口句柄
static eAppWinHandle App_Win_ClickMenuHandler(eAppWinHandle WinHandle,App_Win_Msg_T message)
{
	APP_WIN_RTT_LOG(0,"App_Win_ClickMenuHandler \r\n");
	
	return AppWinParam.CurrWinHanle;	
}

//**********************************************************************
// 函数功能：  窗口菜单处理函数
// 输入参数：  WinHandle	当前窗口句柄
// 				message		传入参数
// 返回参数：  成功创建的窗口句柄
static eAppWinHandle App_Win_LockMenuHandler(eAppWinHandle WinHandle,App_Win_Msg_T message)
{
	APP_WIN_RTT_LOG(0,"App_Win_LockMenuHandler \r\n");

	// 进入锁屏前保存之前窗口类型
	AppWinParam.LastWinHanle = AppWinParam.CurrWinHanle;
	AppWinParam.LastSubWinHandle = AppWinParam.CurrSubWinHandle;	
	
	// 进入锁屏窗口
	AppWinParam.CurrWinHanle = eLockWinHandle;
	AppWinParam.LockWinCnt = 0;
	
	return AppWinParam.CurrWinHanle;	
}

//**********************************************************************
// 函数功能：  窗口初始化
// 输入参数：  
// 返回参数：  成功创建的窗口句柄
eAppWinHandle App_HistoryWin_Init(void)
{
	APP_WIN_RTT_LOG(0,"App_HistoryWin_Init \r\n");
	
	if(AppWinParam.WinRecoverFlg)
	{
		AppWinParam.WinRecoverFlg = false;
		AppWinParam.CurrWinHanle = eHistoryWinHandle;
		AppWinParam.CurrSubWinHandle = AppWinParam.LastSubWinHandle;
	}
	else
	{
		AppWinParam.CurrWinHanle = eHistoryWinHandle;
		AppWinParam.CurrSubWinHandle = eAppSubWinHandle0;
	}
	
	return AppWinParam.CurrWinHanle;
}

//**********************************************************************
// 函数功能：  窗口回调函数
// 输入参数：  WinHandle	当前窗口句柄
// 			   message 		传入参数
// 返回参数：  新的窗口句柄
eAppWinHandle App_HistoryWin_Cb(eAppWinHandle WinHandle, App_Win_Msg_T message)
{
	APP_WIN_RTT_LOG(0,"App_HistoryWin_Cb \r\n");
	
	#if 1
	if(eHistoryWinHandle != WinHandle)
		return eErrWinHandle;
	#endif

	// 搜寻窗口菜单索引号，用于进入正确的回调函数
	uint32_t TmpWinMenuIndex;
	for(TmpWinMenuIndex = 0;TmpWinMenuIndex < AppHistoryWinMenuNum;TmpWinMenuIndex++)
	{
		if(AppHistoryWinMenu[TmpWinMenuIndex].MenuTag == message.MenuTag)
			break;
	}
	
	// 菜单命令有效，则进入对应的处理函数
	if((TmpWinMenuIndex < AppHistoryWinMenuNum) && (NULL != AppHistoryWinMenu[TmpWinMenuIndex].callback))
	{
		return AppHistoryWinMenu[TmpWinMenuIndex].callback(WinHandle, message);
	}

	return WinHandle;
}










