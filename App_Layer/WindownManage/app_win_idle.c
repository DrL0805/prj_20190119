
#include "app_win_idle.h"

//**********************************************************************
// �������ܣ�  ���д��ڳ�ʼ��
// ���������  
// ���ز�����  �ɹ������Ĵ��ھ��
eAppWinHandle App_IdleWin_Init(void)
{
	APP_WIN_RTT_LOG(0,"App_IdleWin_Init \r\n");
	
	AppWinParam.CurrSubWinHandle = eAppSubWinHandle0;
	
	return eIdleWinHandle;	
}

//**********************************************************************
// �������ܣ�  ���ڻص�����
// ���������  eAppWinHandle	��ǰ���ھ��
// 			   App_Win_Msg_T 	�������
// ���ز�����  �µĴ��ھ��
eAppWinHandle App_IdleWin_Cb(eAppWinHandle WinHandle, App_Win_Msg_T message)
{
	APP_WIN_RTT_LOG(0,"App_IdleWin_Cb %d \r\n",WinHandle);

	return WinHandle;
}
