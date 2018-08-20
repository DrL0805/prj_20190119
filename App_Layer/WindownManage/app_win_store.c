
#include "app_win_store.h"

static eAppWinHandle App_Win_KeyMenuHandler(eAppWinHandle WinHandle,App_Win_Msg_T message);

#define AppStoreWinMenuNum (sizeof(AppStoreWinMenu)/sizeof(AppStoreWinMenu[0]))		// 窗口有效菜单数量
App_Win_Menu_T	AppStoreWinMenu[] = 
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
	
	switch(message.val)
	{
		case MID_KEY0_SHORT:
			/* 仓储模式下短按显示电量信息 */
			break;
		case MID_KEY0_HOLDSHORT:
			/* 仓储模式下长按3秒进入开机模式 */
			AppWinParam.CurrWinHanle = ePwronWinHandle;
			break;
		case MID_KEY0_HOLDLONG:
			break;
		default: break;
	}
	
	return AppWinParam.CurrWinHanle;
}

//**********************************************************************
// 函数功能：  窗口初始化
// 输入参数：  
// 返回参数：  成功创建的窗口句柄
eAppWinHandle App_StoreWin_Init(void)
{
	APP_WIN_RTT_LOG(0,"App_StoreWin_Init \r\n");
	
	AppWinParam.CurrWinHanle = eStoreWinHandle;
	AppWinParam.CurrSubWinHandle = eAppSubWinHandle0;
	
	return AppWinParam.CurrWinHanle;
}

//**********************************************************************
// 函数功能：  窗口回调函数
// 输入参数：  WinHandle	当前窗口句柄
// 			   message 		传入参数
// 返回参数：  新的窗口句柄
eAppWinHandle App_StoreWin_Cb(eAppWinHandle WinHandle, App_Win_Msg_T message)
{
	APP_WIN_RTT_LOG(0,"App_StoreWin_Cb \r\n");
	
	uint32_t TmpWinMenuIndex;
	
	#if 1
	if(eStoreWinHandle != WinHandle)
		return eErrWinHandle;
	#endif
	
	// 搜寻窗口菜单索引号，用于进入正确的回调函数
	for(TmpWinMenuIndex = 0;TmpWinMenuIndex < AppStoreWinMenuNum;TmpWinMenuIndex++)
	{
		if(AppStoreWinMenu[TmpWinMenuIndex].MenuTag == message.MenuTag)
			break;
	}
	
	// 菜单命令有效，则进入对应的处理函数
	if((TmpWinMenuIndex < AppStoreWinMenuNum) && (NULL != AppStoreWinMenu[TmpWinMenuIndex].callback))
	{
		return AppStoreWinMenu[TmpWinMenuIndex].callback(WinHandle, message);
	}

	return WinHandle;
}




