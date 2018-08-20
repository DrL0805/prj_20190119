
#include "app_win_idle.h"

static eAppWinHandle App_Win_KeyMenuHandler(eAppWinHandle WinHandle,App_Win_Msg_T message);


#define AppLockWinMenuNum (sizeof(AppLockWinMenu)/sizeof(AppLockWinMenu[0]))	
App_Win_Menu_T	AppLockWinMenu[] = 
{
	{eWinMenukey, App_Win_KeyMenuHandler},
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
// �������ܣ�  ���д��ڳ�ʼ��
// ���������  
// ���ز�����  �ɹ������Ĵ��ھ��
eAppWinHandle App_LockWin_Init(void)
{
	APP_WIN_RTT_LOG(0,"App_LockWin_Init \r\n");
	
	AppWinParam.CurrSubWinHandle = eAppSubWinHandle0;
	
	return eLockWinHandle;	
}

//**********************************************************************
// �������ܣ�  ���ڻص�����
// ���������  eAppWinHandle	��ǰ���ھ��
// 			   App_Win_Msg_T 	�������
// ���ز�����  �µĴ��ھ��
eAppWinHandle App_LockWin_Cb(eAppWinHandle WinHandle, App_Win_Msg_T message)
{
	APP_WIN_RTT_LOG(0,"App_LockWin_Cb %d \r\n",WinHandle);

	return WinHandle;
}
