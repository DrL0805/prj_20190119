/**********************************************************************
**
**ģ��˵��: mid��stop watch�ӿ�
**����汾���޸���־(ʱ�䣬����),�޸���:
**   V1.0   2018.4.24  �޸�����  ZSL  
**
**********************************************************************/
#include "sm_timer.h"
#include "mid_stopwatch.h"

Mid_StopWatch_Param_t	MidStopWatch;

//**********************************************************************
// ��������: �жϺ��� 
// �����������
// ���ز�������
void Mid_StopWatch_Isr(void)
{
	MidStopWatch.TotalMs += STOPWATCH_PERIOD_MS;
}

//**********************************************************************
// ��������: ���λ����
//       ��������ȫ����λΪ0�������Timer����Ӳ����λ
// �����������
// ���ز�������
//**********************************************************************
void Mid_StopWatch_Reset(void)
{
	MidStopWatch.MeasureCnt = 0;
	MidStopWatch.TotalMs = 0;
	MidStopWatch.RuningFlg = false;
	
    SMDrv_CTimer_Clear(STOPWATCH_CTIMER_MODULE);
}

//**********************************************************************
// ��������: ����ܳ�ʼ��������
//       �����Timer����Ӳ�����г�ʼ������ע���жϻص�����������������λ
// �����������
// ���ز�������
//**********************************************************************
void Mid_StopWatch_Init(void)
{
    SMDrv_CTimer_Open(STOPWATCH_CTIMER_MODULE,STOPWATCH_PERIOD_MS,Mid_StopWatch_Isr);
	
	Mid_StopWatch_Reset();
	
	MidStopWatch.InitedFlg = true;
};

//**********************************************************************
// ��������:  ���������
// �����������
// ���ز�������
//**********************************************************************
void Mid_StopWatch_Start(void)
{
	MidStopWatch.RuningFlg = true;
	
    SMDrv_CTimer_Start(STOPWATCH_CTIMER_MODULE);
}

//**********************************************************************
// ��������:  ��ͣ�����
// �����������
// ���ز�������
//**********************************************************************
void Mid_StopWatch_Stop(void)
{
	MidStopWatch.RuningFlg = false;
	
    SMDrv_CTimer_Stop(STOPWATCH_CTIMER_MODULE);
}

//**********************************************************************
// ��������: �����������㣬�ѵ�ǰ���ʱ�䱣������
//           �����ʱ���ȡ������������
// ���������
// ���ز�����  0 �ɹ�����0 ʧ��
//**********************************************************************
uint32_t Mid_StopWatch_MeasurePoint(void)
{
	uint32 cntTemp;
	
	if(MidStopWatch.MeasureCnt >= STOPWATCH_MAX_STORE)
		return 0xFF;	
	
	// ��ȡ��ǰtickֵ����תΪms��ʱ��ʹ�õ���32768Hz
	cntTemp = (uint32_t)SMDrv_CTimer_ReadCount(STOPWATCH_CTIMER_MODULE) * 0.0305;	// (1000 / 32768)
	
	// ���浱ǰ��������ʱ��ֵ
	MidStopWatch.MeasurePoint[MidStopWatch.MeasureCnt] = MidStopWatch.TotalMs + cntTemp;
	MidStopWatch.MeasureCnt++;
	
	return 0x00;
}

//**********************************************************************
// ��������: ��ȡ��ǰ������ֵ
// ���������Methon ��ȡ���� 0 ����������1 ������
// ���ز�����    
uint32_t Mid_StopWatch_TotalMsGet(uint8_t Methon)
{	
	if(Methon)
	{
		uint32 cntTemp;
		
		// ��ȡ��ǰtickֵ����תΪms��ʱ��ʹ�õ���32768Hz
		cntTemp = (uint32_t)SMDrv_CTimer_ReadCount(STOPWATCH_CTIMER_MODULE) * 0.0305;	// (1000 / 32768)
		
		return (MidStopWatch.TotalMs + cntTemp);
	}
	else
	{
		return MidStopWatch.TotalMs;
	}
}

//**********************************************************************
// ��������: ��ȡ����ڲ�����
// ���������
// ���ز�����
void Mid_StopWatch_ParamGet(Mid_StopWatch_Param_t* Mid_StopWatch_Param)
{
	memcpy(Mid_StopWatch_Param, &MidStopWatch, sizeof(Mid_StopWatch_Param_t));
}

//**********************************************************************
// ��������: msֵתΪ h:m:s:msģʽ
// ���������   Ms ��ת����Msֵ
//				Mid_StopWatch_Format ����ת�����
// ���ز�����
//**********************************************************************
void Mid_StopWatch_FormatSwitch(uint32_t Ms, Mid_StopWatch_Format_t* Mid_StopWatch_Format)
{
	uint32_t tMs = Ms;
	
	Mid_StopWatch_Format->hour = tMs / 3600000;
	
	tMs %= 3600000;
	Mid_StopWatch_Format->min = tMs / 60000;
	
	tMs %= 60000;
	Mid_StopWatch_Format->sec = tMs / 1000;	
	
	tMs %= 1000;
	Mid_StopWatch_Format->ms = tMs;	
}












