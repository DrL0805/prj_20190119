
#include "app_win_abc.h"

static eAppWinHandle App_Win_KeyMenuHandler(eAppWinHandle WinHandle,App_Win_Msg_T message);
static eAppWinHandle App_Win_SlideMenuHandler(eAppWinHandle WinHandle,App_Win_Msg_T message);
static eAppWinHandle App_Win_ClickMenuHandler(eAppWinHandle WinHandle,App_Win_Msg_T message);

#define AppABCWinMenuNum (sizeof(AppABCWinMenu)/sizeof(AppABCWinMenu[0]))	
App_Win_Menu_T	AppABCWinMenu[] = 
{
	{eWinMenukey, App_Win_KeyMenuHandler},
	{eWinMenuSlide, App_Win_SlideMenuHandler},
	{eWinMenuClick, App_Win_ClickMenuHandler},
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
			/* 非时间模式下短按进入时间模式 */
			switch (AppWinParam.CurrSubWinHandle)
            {
            	case eAppSubWinHandle0:
            	case eAppSubWinHandle1:
				case eAppSubWinHandle2:
            		TmpWinHandle = eTimeWinHandle;
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
            		TmpWinHandle = eStoreWinHandle;
					break;
            	default:
            		break;
            }			
			break;
		default: break;
	}
	
	return TmpWinHandle;	
}

//**********************************************************************
// 函数功能：  窗口菜单处理函数
// 输入参数：  WinHandle	当前窗口句柄
// 				message		传入参数
// 返回参数：  成功创建的窗口句柄
static eAppWinHandle App_Win_SlideMenuHandler(eAppWinHandle WinHandle,App_Win_Msg_T message)
{
	APP_WIN_RTT_LOG(0,"App_Win_SlideMenuHandler \r\n");
	
	eAppWinHandle TmpWinHandle = WinHandle;
	
	switch(message.val)
	{
		case 0:		// 上滑
			switch (AppWinParam.CurrSubWinHandle)
            {
            	case eAppSubWinHandle0:
					TmpWinHandle = eHistoryWinHandle;
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
					TmpWinHandle = eSleepWinHandle;
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
	
	return TmpWinHandle;	
}

//**********************************************************************
// 函数功能：  窗口菜单处理函数
// 输入参数：  WinHandle	当前窗口句柄
// 				message		传入参数
// 返回参数：  成功创建的窗口句柄
static eAppWinHandle App_Win_ClickMenuHandler(eAppWinHandle WinHandle,App_Win_Msg_T message)
{
	APP_WIN_RTT_LOG(0,"App_Win_ClickMenuHandler \r\n");
	
	eAppWinHandle TmpWinHandle = WinHandle;
	
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
	
	return TmpWinHandle;	
}

//**********************************************************************
// 函数功能：  窗口初始化
// 输入参数：  
// 返回参数：  成功创建的窗口句柄
eAppWinHandle App_ABCWin_Init(void)
{
	APP_WIN_RTT_LOG(0,"App_ABCWin_Init \r\n");
	
	AppWinParam.CurrSubWinHandle = eAppSubWinHandle0;
	
	return eABCWinHandle;
}

//**********************************************************************
// 函数功能：  窗口回调函数
// 输入参数：  WinHandle	当前窗口句柄
// 			   message 		传入参数
// 返回参数：  新的窗口句柄
eAppWinHandle App_ABCWin_Cb(eAppWinHandle WinHandle, App_Win_Msg_T message)
{
	APP_WIN_RTT_LOG(0,"App_ABCWin_Cb \r\n");
	
	#if 1
	if(eABCWinHandle != WinHandle)
		return eErrWinHandle;
	#endif

	// 搜寻窗口菜单索引号，用于进入正确的回调函数
	uint32_t TmpWinMenuIndex;
	for(TmpWinMenuIndex = 0;TmpWinMenuIndex < AppABCWinMenuNum;TmpWinMenuIndex++)
	{
		if(AppABCWinMenu[TmpWinMenuIndex].MenuTag == message.MenuTag)
			break;
	}	
	
	// 菜单命令有效，则进入对应的处理函数
	if((TmpWinMenuIndex < AppABCWinMenuNum) && (NULL != AppABCWinMenu[TmpWinMenuIndex].callback))
	{
		return AppABCWinMenu[TmpWinMenuIndex].callback(WinHandle, message);
	}

	return WinHandle;
}












