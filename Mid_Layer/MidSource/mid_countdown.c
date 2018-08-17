/**********************************************************************
**
**ģ��˵��: mid��KEY�ӿ�
**����汾���޸���־(ʱ�䣬����),�޸���:
**   V1.0   2018.4.24  �޸�����  ZSL  
**
**********************************************************************/
#include "sm_timer.h"
#include "mid_countdown.h"

static Mid_CountDown_Format_t  countdownTime;

static Mid_CountDown_Param_t	MidCountDown;

//**********************************************************************
// ��������: 
// �����������
// ���ز�������
//**********************************************************************
void Mid_Countdown_Isr(void)
{
	if(0 == --MidCountDown.RemainSec)
	{
		Mid_Countdown_Stop();
	}
	
//	SEGGER_RTT_printf(0,"MidCountDown.RemainSec %d \n", Mid_Countdown_RemainRead());
}

//**********************************************************************
// ��������: ����ʱ��λ
// ����������� 
// ���ز������� 
//**********************************************************************
void Mid_Countdown_Reset(void)
{
	MidCountDown.RemainSec = MidCountDown.TotalSec;

    SMDrv_CTimer_Clear(COUNTDOWN_CTIMER_MODULE);
}

//**********************************************************************
// ��������: ����ʱ��ʼ��,Ӳ����ʼ������ע���жϻص�����
// �����������
// ���ز�������
//**********************************************************************
void Mid_Countdown_Init(void)
{
	MidCountDown.InitedFlg = true;

    SMDrv_CTimer_Open(COUNTDOWN_CTIMER_MODULE, COUNTDOWN_PERIOD_MS, Mid_Countdown_Isr);
    Mid_Countdown_Reset();
}

//**********************************************************************
// ��������: ����ʱ��ʼ
// �����������  
// ���ز�������
//**********************************************************************
void Mid_Countdown_Start(void)
{
	MidCountDown.RuningFlg = true;
	
    SMDrv_CTimer_Start(COUNTDOWN_CTIMER_MODULE);
}

//**********************************************************************
// ��������: ����ʱֹͣ
// �����������
// ���ز�������
//**********************************************************************
void Mid_Countdown_Stop(void)
{
	MidCountDown.RuningFlg = false;
	
    SMDrv_CTimer_Stop(COUNTDOWN_CTIMER_MODULE);
}

//**********************************************************************
// ��������: ��ȡ����ʱʣ��ʱ��
// ���������timeTemp��������������浹��ʱʣ��ʱ��
// ���ز������� 
//**********************************************************************
 uint32_t Mid_Countdown_RemainRead(void)
{
	return MidCountDown.RemainSec;
}

//**********************************************************************
// ��������: ����/д�뵹��ʱʣ��ʱ��
// ���������timeTemp�����뵹��ʱʣ��ʱ�����
//        
// ���ز�������  
//**********************************************************************
void Mid_Countdown_TimeWrite(uint32_t Sec)
{
	MidCountDown.TotalSec = Sec;
	MidCountDown.RemainSec = Sec;
}

