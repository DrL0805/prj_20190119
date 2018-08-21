
#include "app_win_press.h"

static eAppWinHandle App_Win_KeyMenuHandler(eAppWinHandle WinHandle,App_Win_Msg_T message);
static eAppWinHandle App_Win_SlideMenuHandler(eAppWinHandle WinHandle,App_Win_Msg_T message);
static eAppWinHandle App_Win_ClickMenuHandler(eAppWinHandle WinHandle,App_Win_Msg_T message);
static eAppWinHandle App_Win_LockMenuHandler(eAppWinHandle WinHandle,App_Win_Msg_T message);

#define AppPressWinMenuNum (sizeof(AppPressWinMenu)/sizeof(AppPressWinMenu[0]))	
App_Win_Menu_T	AppPressWinMenu[] = 
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
			switch (AppWinParam.CurrSubWinHandle)
            {
            	case eAppSubWinHandle0:
            	case eAppSubWinHandle1:
				case eAppSubWinHandle2:
            		AppWinParam.CurrWinHanle = eTimeWinHandle;
					break;
            	default:
            		break;
            }
			break;
		case MID_KEY0_HOLDSHORT:
			switch (AppWinParam.CurrSubWinHandle)
            {
            	case eAppSubWinHandle0:
            	case eAppSubWinHandle1:
				case eAppSubWinHandle2:	
            		break;
            	default:
            		break;
            }			
			break;
		case MID_KEY0_HOLDLONG:
			/* 开机状态下长按12秒进入仓储模式*/
			switch (AppWinParam.CurrSubWinHandle)
            {
            	case eAppSubWinHandle0:
            	case eAppSubWinHandle1:
				case eAppSubWinHandle2:
            		AppWinParam.CurrWinHanle = eStoreWinHandle;
					break;
            	default:
            		break;
            }			
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
			switch (AppWinParam.CurrSubWinHandle)
            {
            	case eAppSubWinHandle0:
					AppWinParam.CurrWinHanle = eBodyTempWinHandle;
					break;
            	case eAppSubWinHandle1:
					AppWinParam.CurrSubWinHandle = eAppSubWinHandle2;
					break;
				case eAppSubWinHandle2:
            		AppWinParam.CurrSubWinHandle = eAppSubWinHandle1;
					break;
            	default:
            		break;
            }			
			break;
		case 1:		// 下滑
			switch (AppWinParam.CurrSubWinHandle)
            {
            	case eAppSubWinHandle0:
					AppWinParam.CurrWinHanle = eHeartWinHandle;
					break;
            	case eAppSubWinHandle1:
					AppWinParam.CurrSubWinHandle = eAppSubWinHandle2;
					break;
				case eAppSubWinHandle2:
            		AppWinParam.CurrSubWinHandle = eAppSubWinHandle1;
					break;
            	default:
            		break;
            }
			break;
		case 2:		// 左滑
			break;
		case 3:		// 右滑
			switch (AppWinParam.CurrSubWinHandle)
            {
            	case eAppSubWinHandle0: 
					break;
            	case eAppSubWinHandle1:
				case eAppSubWinHandle2:
					AppWinParam.CurrSubWinHandle = eAppSubWinHandle0;
					break;
            	default: break;
            }			
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

	switch (AppWinParam.CurrSubWinHandle)
	{
		case eAppSubWinHandle0: 
			AppWinParam.CurrSubWinHandle = eAppSubWinHandle1;
			break;
		case eAppSubWinHandle1:
		case eAppSubWinHandle2:
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
eAppWinHandle App_PressWin_Init(void)
{
	APP_WIN_RTT_LOG(0,"App_PressWin_Init \r\n");
	
	if(AppWinParam.WinRecoverFlg)
	{
		AppWinParam.WinRecoverFlg = false;
		AppWinParam.CurrWinHanle = ePressWinHandle;
		AppWinParam.CurrSubWinHandle = AppWinParam.LastSubWinHandle;
	}
	else
	{
		AppWinParam.CurrWinHanle = ePressWinHandle;
		AppWinParam.CurrSubWinHandle = eAppSubWinHandle0;
	}
	
	return AppWinParam.CurrWinHanle;
}

//**********************************************************************
// 函数功能：  窗口回调函数
// 输入参数：  WinHandle	当前窗口句柄
// 			   message 		传入参数
// 返回参数：  新的窗口句柄
eAppWinHandle App_PressWin_Cb(eAppWinHandle WinHandle, App_Win_Msg_T message)
{
	APP_WIN_RTT_LOG(0,"App_PressWin_Cb \r\n");
	
	#if 1
	if(ePressWinHandle != WinHandle)
		return eErrWinHandle;
	#endif

	// 搜寻窗口菜单索引号，用于进入正确的回调函数
	uint32_t TmpWinMenuIndex;
	for(TmpWinMenuIndex = 0;TmpWinMenuIndex < AppPressWinMenuNum;TmpWinMenuIndex++)
	{
		if(AppPressWinMenu[TmpWinMenuIndex].MenuTag == message.MenuTag)
			break;
	}	
	
	// 菜单命令有效，则进入对应的处理函数
	if((TmpWinMenuIndex < AppPressWinMenuNum) && (NULL != AppPressWinMenu[TmpWinMenuIndex].callback))
	{
		return AppPressWinMenu[TmpWinMenuIndex].callback(WinHandle, message);
	}

	return WinHandle;
}












