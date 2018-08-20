
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
	
//	eAppWinHandle TmpWinHandle = WinHandle;
	
	switch(message.val)
	{
		case MID_KEY0_SHORT:
			/* 锁屏状态下5min内，短按恢复之前窗口。超过5min回到时间窗口 */
			if(AppWinParam.LockWinCnt < APP_WIN_RECOVER_TIMEOUT)
			{
				AppWinParam.CurrWinHanle = AppWinParam.LastWinHanle;
				AppWinParam.WinRecoverFlg = true;
			}
			else
			{
				AppWinParam.CurrWinHanle = eTimeWinHandle;
			}
			break;
		case MID_KEY0_HOLDSHORT:
			break;
		case MID_KEY0_HOLDLONG:
			/* 长按12秒进入仓储模式*/
			AppWinParam.CurrWinHanle = eStoreWinHandle;
			break;
		default: break;
	}
	
	return AppWinParam.CurrWinHanle;	
}

//**********************************************************************
// 函数功能：  空闲窗口初始化
// 输入参数：  
// 返回参数：  成功创建的窗口句柄
eAppWinHandle App_LockWin_Init(void)
{
	APP_WIN_RTT_LOG(0,"App_LockWin_Init \r\n");
	
	// 进入锁屏窗口
	AppWinParam.CurrWinHanle = eLockWinHandle;
	AppWinParam.CurrSubWinHandle = eAppSubWinHandle0;
	
	return AppWinParam.CurrWinHanle;	
}

//**********************************************************************
// 函数功能：  窗口回调函数
// 输入参数：  eAppWinHandle	当前窗口句柄
// 			   App_Win_Msg_T 	传入参数
// 返回参数：  新的窗口句柄
eAppWinHandle App_LockWin_Cb(eAppWinHandle WinHandle, App_Win_Msg_T message)
{
	APP_WIN_RTT_LOG(0,"App_LockWin_Cb \r\n");
	
	uint32_t TmpWinMenuIndex;
	
	#if 1
	if(eLockWinHandle != WinHandle)
		return eErrWinHandle;
	#endif
	
	// 搜寻窗口菜单索引号，用于进入正确的回调函数
	for(TmpWinMenuIndex = 0;TmpWinMenuIndex < AppLockWinMenuNum;TmpWinMenuIndex++)
	{
		if(AppLockWinMenu[TmpWinMenuIndex].MenuTag == message.MenuTag)
			break;
	}
	
	// 菜单命令有效，则进入对应的处理函数
	if((TmpWinMenuIndex < AppLockWinMenuNum) && (NULL != AppLockWinMenu[TmpWinMenuIndex].callback))
	{
		return AppLockWinMenu[TmpWinMenuIndex].callback(WinHandle, message);
	}

	return WinHandle;
}
