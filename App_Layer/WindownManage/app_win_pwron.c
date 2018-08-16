
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
// �������ܣ�  ���ڲ˵�������
// ���������  WinHandle	��ǰ���ھ��
// 				message		�������
// ���ز�����  �ɹ������Ĵ��ھ��
static eAppWinHandle App_Win_PwrOnMenuHandler(eAppWinHandle WinHandle,App_Win_Msg_T message)
{
	APP_WIN_RTT_LOG(0,"App_Win_PwrOnMenuHandler \r\n");
	
	eAppWinHandle TmpWinHandle = WinHandle;	
	
	/* ����������ɣ�����ʱ��ģʽ */
	TmpWinHandle = eTimeWinHandle;
	
	return TmpWinHandle;
}

//**********************************************************************
// �������ܣ�  ���ڲ˵�������
// ���������  WinHandle	��ǰ���ھ��
// 				message		�������
// ���ز�����  �ɹ������Ĵ��ھ��
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
// �������ܣ�  ���ڳ�ʼ��
// ���������  
// ���ز�����  �ɹ������Ĵ��ھ��
eAppWinHandle App_PwronWin_Init(void)
{
	APP_WIN_RTT_LOG(0,"App_PwronWin_Init \r\n");
	
	AppWinParam.CurrSubWinHandle = eAppSubWinHandle0;
	
	return ePwronWinHandle;
}

//**********************************************************************
// �������ܣ�  ���ڻص�����
// ���������  WinHandle	��ǰ���ھ��
// 			   message 		�������
// ���ز�����  �µĴ��ھ��
eAppWinHandle App_PwronWin_Cb(eAppWinHandle WinHandle, App_Win_Msg_T message)
{
	APP_WIN_RTT_LOG(0,"App_PwronWin_Cb \r\n");
	
	#if 1
	if(ePwronWinHandle != WinHandle)
		return eErrWinHandle;
	#endif
	
	// ��Ѱ���ڲ˵������ţ����ڽ�����ȷ�Ļص�����
	uint32_t TmpWinMenuIndex;
	for(TmpWinMenuIndex = 0;TmpWinMenuIndex < AppPwronWinMenuNum;TmpWinMenuIndex++)
	{
		if(AppPwronWinMenu[TmpWinMenuIndex].MenuTag == message.MenuTag)
			break;
	}	
	
	// �˵�������Ч��������Ӧ�Ĵ�����������������
	if((TmpWinMenuIndex < AppPwronWinMenuNum) && (NULL != AppPwronWinMenu[TmpWinMenuIndex].callback))
	{
		return AppPwronWinMenu[TmpWinMenuIndex].callback(WinHandle, message);
	}

	return WinHandle;
}




