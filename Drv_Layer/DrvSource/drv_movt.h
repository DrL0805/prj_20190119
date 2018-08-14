#ifndef			MOVT_H
#define			MOVT_H

#define MOVT_MODULE
#include "io_config.h"
#include "platform_common.h"

// movt direction set
#define	MOVT_DIR_AUTO      0
#define	MOVT_DIR_SETTING   1

#define	MOVT_DIR_FORWARD   0
#define	MOVT_DIR_REVERSE   1


#ifndef		MOVT_NUM_TYPEDEF
#define		MOVT_NUM_TYPEDEF

#if(((MOVT_M_A_PIN != IO_UNKNOW) && (MOVT_M_B_PIN != IO_UNKNOW)) || \
    ((MOVT_3_A_PIN != IO_UNKNOW) && (MOVT_3_B_PIN != IO_UNKNOW)) || \
    ((MOVT_6_A_PIN != IO_UNKNOW) && (MOVT_6_B_PIN != IO_UNKNOW)) || \
    ((MOVT_9_A_PIN != IO_UNKNOW) && (MOVT_9_B_PIN != IO_UNKNOW)))
#define SUPPORT_MOVT        1
#else
#define SUPPORT_MOVT        0
#endif


// movt num
typedef enum
{
#if((MOVT_M_A_PIN != IO_UNKNOW) && (MOVT_M_B_PIN != IO_UNKNOW))
    MOVT_M_CLOCK = 0,
#endif
#if((MOVT_3_A_PIN != IO_UNKNOW) && (MOVT_3_B_PIN != IO_UNKNOW))
    MOVT_3_CLOCK,
#endif
#if((MOVT_6_A_PIN != IO_UNKNOW) && (MOVT_6_B_PIN != IO_UNKNOW))
    MOVT_6_CLOCK,
#endif
#if((MOVT_9_A_PIN != IO_UNKNOW) && (MOVT_9_B_PIN != IO_UNKNOW))
    MOVT_9_CLOCK,
#endif
    MOVT_MAX_NUM,
} movt_num_t;
#endif

#if(((MOVT_3_A_PIN != IO_UNKNOW) && (MOVT_3_B_PIN != IO_UNKNOW)) || \
    ((MOVT_6_A_PIN != IO_UNKNOW) && (MOVT_6_B_PIN != IO_UNKNOW)) || \
    ((MOVT_9_A_PIN != IO_UNKNOW) && (MOVT_9_B_PIN != IO_UNKNOW)))
#define SUB_MOVT   //֧��С����
#endif

#define	MOVT_M_ERROR    0X01
#define	MOVT_3_ERROR    0X02
#define	MOVT_6_ERROR    0X04
#define	MOVT_9_ERROR    0X08

typedef enum 
{
    MOVT_SET_MC_FORWARD         = 0x0001,       //position 2clock msg
    MOVT_SET_MC_REVERSE,
    MOVT_SET_MC_FORWARDING,
    MOVT_SET_MC_REVERSEING,
    MOVT_SET_MC_SET_CUR,
    MOVT_SET_MC_SET_AIM,
    MOVT_SET_MC_SET_CUR_AIM,
    MOVT_SET_MC_SET_CUR_FORWARD,
    MOVT_SET_MC_SET_AIM_FORWARD,
    MOVT_SET_MC_SET_CUR_AIM_FORWARD,
    MOVT_SET_MC_SET_CUR_REVERSE,
    MOVT_SET_MC_SET_AIM_REVERSE,
    MOVT_SET_MC_SET_CUR_AIM_REVERSE,
    MOVT_SET_MC_STOP,
    MOVT_SET_MC_RECOVER,
    MOVT_SET_MC_RECOVER_FORWARD,
    MOVT_SET_MC_RECOVER_REVERSE,
    MOVT_SET_MC_READ_CUR,
    MOVT_SET_MC_READ_AIM,
    MOVT_SET_MC_FORWARD_FINISH,
    MOVT_SET_MC_REVERSE_FINISH,

    
    MOVT_SET_3C_FORWARD     = 0x0301,       //position 10clock msg
    MOVT_SET_3C_REVERSE,
    MOVT_SET_3C_FORWARDING,
    MOVT_SET_3C_REVERSEING,
    MOVT_SET_3C_SET_CUR,
    MOVT_SET_3C_SET_AIM,
    MOVT_SET_3C_SET_CUR_AIM,
    MOVT_SET_3C_SET_CUR_FORWARD,
    MOVT_SET_3C_SET_AIM_FORWARD,
    MOVT_SET_3C_SET_CUR_AIM_FORWARD,
    MOVT_SET_3C_SET_CUR_REVERSE,
    MOVT_SET_3C_SET_AIM_REVERSE,
    MOVT_SET_3C_SET_CUR_AIM_REVERSE,
    MOVT_SET_3C_STOP,
    MOVT_SET_3C_RECOVER,
    MOVT_SET_3C_RECOVER_FORWARD,
    MOVT_SET_3C_RECOVER_REVERSE,
    MOVT_SET_3C_READ_CUR,
    MOVT_SET_3C_READ_AIM,
    MOVT_SET_3C_FORWARD_FINISH,
    MOVT_SET_3C_REVERSE_FINISH,

    MOVT_SET_6C_FORWARD     = 0x0601,       //position 10clock msg
    MOVT_SET_6C_REVERSE,
    MOVT_SET_6C_FORWARDING,
    MOVT_SET_6C_REVERSEING,
    MOVT_SET_6C_SET_CUR,
    MOVT_SET_6C_SET_AIM,
    MOVT_SET_6C_SET_CUR_AIM,
    MOVT_SET_6C_SET_CUR_FORWARD,
    MOVT_SET_6C_SET_AIM_FORWARD,
    MOVT_SET_6C_SET_CUR_AIM_FORWARD,
    MOVT_SET_6C_SET_CUR_REVERSE,
    MOVT_SET_6C_SET_AIM_REVERSE,
    MOVT_SET_6C_SET_CUR_AIM_REVERSE,
    MOVT_SET_6C_STOP,
    MOVT_SET_6C_RECOVER,
    MOVT_SET_6C_RECOVER_FORWARD,
    MOVT_SET_6C_RECOVER_REVERSE,
    MOVT_SET_6C_READ_CUR,
    MOVT_SET_6C_READ_AIM,
    MOVT_SET_6C_FORWARD_FINISH,
    MOVT_SET_6C_REVERSE_FINISH,

    MOVT_SET_9C_FORWARD     = 0x0901,       //position 10clock msg
    MOVT_SET_9C_REVERSE,
    MOVT_SET_9C_FORWARDING,
    MOVT_SET_9C_REVERSEING,
    MOVT_SET_9C_SET_CUR,
    MOVT_SET_9C_SET_AIM,
    MOVT_SET_9C_SET_CUR_AIM,
    MOVT_SET_9C_SET_CUR_FORWARD,
    MOVT_SET_9C_SET_AIM_FORWARD,
    MOVT_SET_9C_SET_CUR_AIM_FORWARD,
    MOVT_SET_9C_SET_CUR_REVERSE,
    MOVT_SET_9C_SET_AIM_REVERSE,
    MOVT_SET_9C_SET_CUR_AIM_REVERSE,
    MOVT_SET_9C_STOP,
    MOVT_SET_9C_RECOVER,
    MOVT_SET_9C_RECOVER_FORWARD,
    MOVT_SET_9C_RECOVER_REVERSE,
    MOVT_SET_9C_READ_CUR,
    MOVT_SET_9C_READ_AIM,
    MOVT_SET_9C_FORWARD_FINISH,
    MOVT_SET_9C_REVERSE_FINISH,
}movt_set_action;

// variable
typedef struct
{
    uint16 currentPosition;
    uint16 aimPosition;
    uint16 movtStep;
    uint16 movtForwardMax;
    uint16 movtReverseMax;
    uint16 movtRangeMax;
    uint16 tinyTimeCnt;
    uint8  direction;
    uint8  tempDirection;
    uint8  directionSet;
    uint8  state;
    uint8  repeat;
    uint8  stop;
    uint8  *clockForward;
    uint8  *clockReverse;
    void   (*timerStart)(uint16 cnt);
    void   (*timerStop)(void);
} movt_att_s;

//extern movt_att_s	movtConfig[MOVT_MAX_NUM];

//**********************************************************************
// ��������:  ��ʼ������
// ��������� ��
// ���ز����� ��
//**********************************************************************
extern void Drv_Movt_Init(void);

//**********************************************************************
// ��������:  ����FrowardFinish callback
// ��������� movt_id:����ID
//      fwd_finish_cb:callback
// ���ز����� ��
//**********************************************************************
extern uint8 Drv_Movt_SetFrowardFinishCb(movt_num_t movt_id, comm_cb fwd_finish_cb);

//**********************************************************************
// ��������:  ����ReverseFinish callback
// ��������� movt_id:����ID
//      rvs_finish_cb:callback
// ���ز����� ��
//**********************************************************************
extern uint8 Drv_Movt_SetReverseFinishCb(movt_num_t movt_id, comm_cb rvs_finish_cb);

//**********************************************************************
// ��������:  �Ƚϱ��뵱ǰλ�ú�Ŀ��λ��
// ��������� movtCompare:����ID
// ���ز����� ��
//**********************************************************************
extern void Drv_Movt_ClockCompare(movt_att_s *movtCompare);

//**********************************************************************
// ��������:  �ж���ת��ת
// ��������� movtCompare
// ���ز����� ��
//**********************************************************************
extern uint16 Drv_Movt_RunDiretion(movt_att_s *movtCompare);

//**********************************************************************
// ��������:	���ó����伫��
// ���������	��
// ���ز�����	0x00:�����ɹ�
// 				0xff:����ʧ��
//**********************************************************************
extern uint16 Drv_Movt_PolarSet(movt_num_t clock, uint8 polar);

//**********************************************************************
// ��������:	��ȡ�����伫��
// ���������	clock��ָ��ָ��
// 				polar�����Է���ָ��
// ���ز�����	0x00:�����ɹ�
// 				0xff:����ʧ��
//**********************************************************************
extern uint16 Drv_Movt_PolarRead(movt_num_t clock, uint8 *polar);

//**********************************************************************
// ��������:	������ת���ο�ȣ��м����Ϊÿ1������30.05uS������ÿ������Ϊ61uS
// ���������	clock��ָ��ָ��
// 				polar����������ָ�룬����Ϊ24���ֽ�
// ���ز�����	0x00:�����ɹ�
// 				0xff:����ʧ��
//**********************************************************************
extern uint16 Drv_Movt_ForwardWaveformSet(movt_num_t clock, uint8 *waveform);

//**********************************************************************
// ��������: ���÷�ת���ο�ȣ��м����Ϊÿ1������30.05uS������ÿ������Ϊ61uS
// ���������clock��ָ��ָ��
// 			 polar����������ָ�룬����Ϊ24���ֽ�
// ���ز�����0x00:�����ɹ�
// 			 0xff:����ʧ��
//**********************************************************************
extern uint16 Drv_Movt_ReverseWaveformSet(movt_num_t clock, uint8 *waveform);



//**********************************************************************
// ��������: �����Բ�
// �����������
// ���ز�����	
//  0x00: �����ɹ�
// 	0x01: ����̲���ʧ��
// 	0x02: 3��λ���̲���ʧ��
// 	0x04: 6��λ���̲���ʧ��
// 	0x08: 9��λ���̲���ʧ��
//**********************************************************************
extern uint8 Drv_Movt_SelfTest(void);



extern void Drv_Movt_MClockForwardFinish(void);
extern void Drv_Movt_3ClockForwardFinish(void);
extern void Drv_Movt_6ClockForwardFinish(void);
extern void Drv_Movt_9ClockForwardFinish(void);

extern void Drv_Movt_MClockReverseFinish(void);
extern void Drv_Movt_3ClockReverseFinish(void);
extern void Drv_Movt_6ClockReverseFinish(void);
extern void Drv_Movt_9ClockReverseFinish(void);

uint16 Drv_Movt_ReadPositionInfo(movt_num_t movtId,uint16 actionId);
void Drv_Movt_Forward(movt_num_t movtId);
void Drv_Movt_Reverse(movt_num_t movtId);
void Drv_Movt_Forwarding(movt_num_t movtId);
void Drv_Movt_Reversing(movt_num_t movtId);
void Drv_Movt_SetCurAction(movt_num_t movtId,uint16 actionId,uint16 curval);
void Drv_Movt_SetAimAction(movt_num_t movtId,uint16 actionId,uint16 aimval);
void Drv_Movt_SetCurAimAction(movt_num_t movtId,uint16 actionId,uint16 curval,uint16 aimval);
void Drv_Movt_SetStopAction(movt_num_t movtId);
void Drv_Movt_SetRecoverAction(movt_num_t movtId,uint16 actionId);

#endif		//MOVT_H
