#include "app_win_time.h"

static eAppWinHandle App_Win_KeyMenuHandler(eAppWinHandle WinHandle,App_Win_Msg_T message);
static eAppWinHandle App_Win_SlideMenuHandler(eAppWinHandle WinHandle,App_Win_Msg_T message);
static eAppWinHandle App_Win_ClickMenuHandler(eAppWinHandle WinHandle,App_Win_Msg_T message);
static eAppWinHandle App_Win_LockMenuHandler(eAppWinHandle WinHandle,App_Win_Msg_T message);


#define AppTimeWinMenuNum (sizeof(AppTimeWinMenu)/sizeof(AppTimeWinMenu[0]))	
App_Win_Menu_T	AppTimeWinMenu[] = 
{
	{eWinMenukey, App_Win_KeyMenuHandler},
	{eWinMenuSlide, App_Win_SlideMenuHandler},
	{eWinMenuClick, App_Win_ClickMenuHandler},
	{eWinMenuLock, App_Win_LockMenuHandler},
};

//**********************************************************************
// �������ܣ�  ���ڲ˵�������
// ���������  WinHandle	��ǰ���ھ��
// 				message		�������
// ���ز�����  �ɹ������Ĵ��ھ��
static eAppWinHandle App_Win_KeyMenuHandler(eAppWinHandle WinHandle,App_Win_Msg_T message)
{
	APP_WIN_RTT_LOG(0,"App_Win_KeyMenuHandler \r\n");
	
	eAppWinHandle TmpWinHandle = WinHandle;
	
	switch(message.val)
	{
		case MID_KEY0_SHORT:
			/* ʱ��ģʽ�¶̰����볡��ģʽ */
			break;
		case MID_KEY0_HOLDSHORT:
			break;
		case MID_KEY0_HOLDLONG:
			/* ʱ��ģʽ�³���12�����ִ�ģʽ*/
			TmpWinHandle = eStoreWinHandle;
			break;
		default: break;
	}
	
	return TmpWinHandle;	
}

//**********************************************************************
// �������ܣ�  ���ڲ˵�������
// ���������  WinHandle	��ǰ���ھ��
// 				message		�������
// ���ز�����  �ɹ������Ĵ��ھ��
static eAppWinHandle App_Win_SlideMenuHandler(eAppWinHandle WinHandle,App_Win_Msg_T message)
{
	APP_WIN_RTT_LOG(0,"App_Win_SlideMenuHandler \r\n");
	
	eAppWinHandle TmpWinHandle = WinHandle;
	
	switch(message.val)
	{
		case 0:		// �ϻ�
			TmpWinHandle = eSportWinHandle;
			break;
		case 1:		// �»�
			TmpWinHandle = eHistoryWinHandle;
			break;
		case 2:		// ��
			break;
		case 3:		// �һ�
			break;
		default: break;
	}
	
	return TmpWinHandle;	
}

//**********************************************************************
// �������ܣ�  ���ڲ˵�������
// ���������  WinHandle	��ǰ���ھ��
// 				message		�������
// ���ز�����  �ɹ������Ĵ��ھ��
static eAppWinHandle App_Win_ClickMenuHandler(eAppWinHandle WinHandle,App_Win_Msg_T message)
{
	APP_WIN_RTT_LOG(0,"App_Win_ClickMenuHandler \r\n");
	
	eAppWinHandle TmpWinHandle = WinHandle;
	
	return TmpWinHandle;	
}

//**********************************************************************
// �������ܣ�  ���ڲ˵�������
// ���������  WinHandle	��ǰ���ھ��
// 				message		�������
// ���ز�����  �ɹ������Ĵ��ھ��
static eAppWinHandle App_Win_LockMenuHandler(eAppWinHandle WinHandle,App_Win_Msg_T message)
{
	APP_WIN_RTT_LOG(0,"App_Win_LockMenuHandler \r\n");
	
	eAppWinHandle TmpWinHandle = WinHandle;
	
	// ����֮ǰ���ھ��
	AppWinParam.LastWinHanle = AppWinParam.CurrWinHanle;
	AppWinParam.LastSubWinHandle = AppWinParam.CurrSubWinHandle;
	
	// ������������
	AppWinParam.IdleWinCnt = 0;
	TmpWinHandle = eLockWinHandle;
	
	return TmpWinHandle;	
}

//**********************************************************************
// �������ܣ�  ���ڳ�ʼ��
// ���������  
// ���ز�����  �ɹ������Ĵ��ھ��
eAppWinHandle App_TimeWin_Init(void)
{
	APP_WIN_RTT_LOG(0,"App_TimeWin_Init \r\n");
	
	AppWinParam.CurrSubWinHandle = eAppSubWinHandle0;
	
	return eTimeWinHandle;
}

//**********************************************************************
// �������ܣ�  ���ڻص�����
// ���������  WinHandle	��ǰ���ھ��
// 			   message 		�������
// ���ز�����  �µĴ��ھ��
eAppWinHandle App_TimeWin_Cb(eAppWinHandle WinHandle, App_Win_Msg_T message)
{
	APP_WIN_RTT_LOG(0,"App_TimeWin_Cb \r\n");
	
	#if 1
	if(eTimeWinHandle != WinHandle)
		return eErrWinHandle;
	#endif

	// ��Ѱ���ڲ˵������ţ����ڽ�����ȷ�Ļص�����
	uint32_t TmpWinMenuIndex;
	for(TmpWinMenuIndex = 0;TmpWinMenuIndex < AppTimeWinMenuNum;TmpWinMenuIndex++)
	{
		if(AppTimeWinMenu[TmpWinMenuIndex].MenuTag == message.MenuTag)
			break;
	}
	
	// �˵�������Ч��������Ӧ�Ĵ�����
	if((TmpWinMenuIndex < AppTimeWinMenuNum) && (NULL != AppTimeWinMenu[TmpWinMenuIndex].callback))
	{
		return AppTimeWinMenu[TmpWinMenuIndex].callback(WinHandle, message);
	}

	return WinHandle;
}










