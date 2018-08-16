
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
			/* ��ʱ��ģʽ�¶̰�����ʱ��ģʽ */
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
			/* ����״̬�³���12�����ִ�ģʽ*/
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
		case 1:		// �»�
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
		case 2:		// ��
			break;
		case 3:		// �һ�
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
// �������ܣ�  ���ڲ˵�������
// ���������  WinHandle	��ǰ���ھ��
// 				message		�������
// ���ز�����  �ɹ������Ĵ��ھ��
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
// �������ܣ�  ���ڳ�ʼ��
// ���������  
// ���ز�����  �ɹ������Ĵ��ھ��
eAppWinHandle App_ABCWin_Init(void)
{
	APP_WIN_RTT_LOG(0,"App_ABCWin_Init \r\n");
	
	AppWinParam.CurrSubWinHandle = eAppSubWinHandle0;
	
	return eABCWinHandle;
}

//**********************************************************************
// �������ܣ�  ���ڻص�����
// ���������  WinHandle	��ǰ���ھ��
// 			   message 		�������
// ���ز�����  �µĴ��ھ��
eAppWinHandle App_ABCWin_Cb(eAppWinHandle WinHandle, App_Win_Msg_T message)
{
	APP_WIN_RTT_LOG(0,"App_ABCWin_Cb \r\n");
	
	#if 1
	if(eABCWinHandle != WinHandle)
		return eErrWinHandle;
	#endif

	// ��Ѱ���ڲ˵������ţ����ڽ�����ȷ�Ļص�����
	uint32_t TmpWinMenuIndex;
	for(TmpWinMenuIndex = 0;TmpWinMenuIndex < AppABCWinMenuNum;TmpWinMenuIndex++)
	{
		if(AppABCWinMenu[TmpWinMenuIndex].MenuTag == message.MenuTag)
			break;
	}	
	
	// �˵�������Ч��������Ӧ�Ĵ�����
	if((TmpWinMenuIndex < AppABCWinMenuNum) && (NULL != AppABCWinMenu[TmpWinMenuIndex].callback))
	{
		return AppABCWinMenu[TmpWinMenuIndex].callback(WinHandle, message);
	}

	return WinHandle;
}












