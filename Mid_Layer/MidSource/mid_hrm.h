#ifndef MID_HRM_H
#define MID_HRM_H

#include "platform_common.h"

typedef enum
{
    HRM_FACTORYTEST         =0,
    HRM_FACTORYTEST_START,
    HRM_FACTORYTEST_STOP,
    HRM_ENABLE,
    HRM_DISABLE,
    HRM_START,
    HRM_STOP,
    HRM_DATA_READY,
    HRM_TOUCH,
    HRM_CALCULATE,
    HRM_ACCELREAD,
}HRM_INTERFACE_T;

typedef struct 
{
	uint16			id;
}hrm_event_s;

//**********************************************************************
//�������ܣ� ����ģ���ʼ��
//���������     
//ReadyIsrCb ��������ָ��
//TouchIsrCb���������ݳ���   
//���ز����� ��
//**********************************************************************
extern void Mid_Hrm_Init(func *ReadyIsrCb,func *TouchIsrCb);

//**********************************************************************
//�������ܣ�����״̬��ȡ
//�����������   
//���ز�����
// 0x00   :  �����ɹ�
// 0xFF   :  ����ʧ��
//**********************************************************************
extern uint16 Mid_Hrm_TouchStatusRead(uint8 *ui8istouch);

//**********************************************************************
//�������ܣ�����ֵ��ȡ
//�����������   
//���ز���������ֵ
//**********************************************************************
extern uint16  Mid_Hrm_Read(uint8 *hrmval);

//**********************************************************************
//�������ܣ�����ģ��©��������ݻ�ȡ
//�����������   
//���ز�����
// 0x00   :  �����ɹ�
// 0xFF   :  ����ʧ��
//**********************************************************************
extern uint16 Mid_Hrm_FactoryTestRead(uint16 ui16lightleak[3]);

//**********************************************************************
// ��������:    ����ģ���¼�����
// ���������    ��
// ���ز�����    0x00: success
//              0xff: fail
//**********************************************************************
extern uint16 Mid_Hrm_EventProcess(hrm_event_s *msg);

void    Mid_Hrm_ReadyCbInit(void (*hrmreadycb)(uint8 hrmval));

#endif
