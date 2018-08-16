
#include "app_win_pwron.h"

static eAppWinHandle App_Win_KeyMenuHandler(eAppWinHandle WinHandle,App_Win_Msg_T message);
static eAppWinHandle App_Win_PwrOnMenuHandler(eAppWinHandle WinHandle,App_Win_Msg_T message);

#define AppPwronWinMenuNum (sizeof(AppPwronWinMenu)/sizeof(AppPwronWinMenu[0]))	
App_Win_Menu_T	AppPwronWinMenu[] = 
{
	{eWinMenukey, App_Win_KeyMenuHandler},
	{eWinMenuPwrOn, App_Win_PwrOnMenuHandler},
};

//**********************************************************************
// 函数功能：  窗口菜单处理函数
// 输入参数：  WinHandle	当前窗口句柄
// 				message		传入参数
// 返回参数：  成功创建的窗口句柄
static eAppWinHandle App_Win_PwrOnMenuHandler(eAppWinHandle WinHandle,App_Win_Msg_T message)
{
	APP_WIN_RTT_LOG(0,"App_Win_PwrOnMenuHandler \r\n");
	
	eAppWinHandle TmpWinHandle = WinHandle;	
	
	/* 开机动画完成，进入时间模式 */
	TmpWinHandle = eTimeWinHandle;
	
	return TmpWinHandle;
}

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
			break;
		case MID_KEY0_HOLDSHORT:
			break;
		case MID_KEY0_HOLDLONG:
			break;
		default: break;
	}
	
	return WinHandle;
}

//**********************************************************************
// 函数功能：  窗口初始化
// 输入参数：  
// 返回参数：  成功创建的窗口句柄
eAppWinHandle App_PwronWin_Init(void)
{
	APP_WIN_RTT_LOG(0,"App_PwronWin_Init \r\n");
	
	AppWinParam.CurrSubWinHandle = eAppSubWinHandle0;
	
	return ePwronWinHandle;
}

//**********************************************************************
// 函数功能：  窗口回调函数
// 输入参数：  WinHandle	当前窗口句柄
// 			   message 		传入参数
// 返回参数：  新的窗口句柄
eAppWinHandle App_PwronWin_Cb(eAppWinHandle WinHandle, App_Win_Msg_T message)
{
	APP_WIN_RTT_LOG(0,"App_PwronWin_Cb \r\n");
	
	#if 1
	if(ePwronWinHandle != WinHandle)
		return eErrWinHandle;
	#endif
	
	// 搜寻窗口菜单索引号，用于进入正确的回调函数
	uint32_t TmpWinMenuIndex;
	for(TmpWinMenuIndex = 0;TmpWinMenuIndex < AppPwronWinMenuNum;TmpWinMenuIndex++)
	{
		if(AppPwronWinMenu[TmpWinMenuIndex].MenuTag == message.MenuTag)
			break;
	}	
	
	// 菜单命令有效，则进入对应的处理函数，否则不做处理
	if((TmpWinMenuIndex < AppPwronWinMenuNum) && (NULL != AppPwronWinMenu[TmpWinMenuIndex].callback))
	{
		return AppPwronWinMenu[TmpWinMenuIndex].callback(WinHandle, message);
	}

	return WinHandle;
}




