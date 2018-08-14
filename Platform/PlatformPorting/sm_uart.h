#ifndef __SM_UART_H
#define __SM_UART_H

#include "platform_common.h"

//����ʹ��UART��ģ������
typedef enum
{
    BASE_UART_MODULE,   //����ͨѶ�ã�����ԣ���о�о�
    GPS_UART_MODULE,    //GPS
    BLE_UART_MODULE,    //ʹ��UARTͨѶ��BLE����
}uart_module;

//����driver��ͨ�õ�uart�ж�����
#define UART_EVENT_NONE       0XFFFFFFFF               //���ж�
#define UART_EVENT_TX         AM_HAL_UART_INT_TX       //�����ж�
#define UART_EVENT_RX         AM_HAL_UART_INT_RX       //�����ж�
#define UART_EVENT_RX_TIMEOUT AM_HAL_UART_INT_RX_TMOUT //���ճ�ʱ�ж�

//����driver��ͨ�õĴ���uart�ж�ʱFIFO����
#define UART_FIFO_NONE   0XFFFFFFFF               //������FIFO
//tx fifo depth
#define UART_TX_FIFO_1_8 AM_HAL_UART_TX_FIFO_1_8  //���ݵ�1/8 FIFOʱ�����������ж�
#define UART_TX_FIFO_1_4 AM_HAL_UART_TX_FIFO_1_4  //���ݵ�1/4 FIFOʱ�����������ж�
#define UART_TX_FIFO_1_2 AM_HAL_UART_TX_FIFO_1_2  //���ݵ�1/2 FIFOʱ�����������ж�
#define UART_TX_FIFO_3_4 AM_HAL_UART_TX_FIFO_3_4  //���ݵ�3/4 FIFOʱ�����������ж�
#define UART_TX_FIFO_7_8 AM_HAL_UART_TX_FIFO_7_8  //���ݵ�7/8 FIFOʱ�����������ж�
//rx fifo depth
#define UART_RX_FIFO_1_8 AM_HAL_UART_RX_FIFO_1_8  //���ݵ�1/8 FIFOʱ�����������ж�
#define UART_RX_FIFO_1_4 AM_HAL_UART_RX_FIFO_1_4  //���ݵ�1/4 FIFOʱ�����������ж�
#define UART_RX_FIFO_1_2 AM_HAL_UART_RX_FIFO_1_2  //���ݵ�1/2 FIFOʱ�����������ж�
#define UART_RX_FIFO_3_4 AM_HAL_UART_RX_FIFO_3_4  //���ݵ�3/4 FIFOʱ�����������ж�
#define UART_RX_FIFO_7_8 AM_HAL_UART_RX_FIFO_7_8  //���ݵ�7/8 FIFOʱ�����������ж�

typedef struct
{
    uint32 u32event_type;  //ֵ�ο�UART_EVENT_XXX
    uint32 u32fifo_type;   //ֵ�ο�UART_TX_FIFO_XXX��UART_RX_FIFO_XXX
}uart_openinfo;


typedef void (*uart_cb)(uint32 uart_event);

//**********************************************************************
// ��������: ��ʼ��UART
// ���������	
// ���ز�����
//**********************************************************************
extern void SMDrv_UART_Init(void);

//**********************************************************************
// ��������: ����driver module ID��Ӳ����Ӧ��UART
// ���������	
//    modul: driver module ID,ֵ�ο�uart_module
//    ptype_info:Ҫ���õ�uart�ж����ͣ�FIFO����, �������жϣ�������ΪNULL
//    ut_callback:�ϲ�ע����жϻص�����
// ���ز�����Ret_InvalidParam��Ret_OK
//
// ����: ʹ��uart���շ����жϣ�����fifo������Ϊ1/8ʱ���������ж�
//      uart_openinfo type_info;
//      type_info.u32event_type = UART_EVENT_RX | UART_EVENT_TX;
//      type_info.u32fifo_type =  UART_RX_FIFO_1_8;
//      SMDrv_UART_Open(GPS_UART_MODULE,&type_info,ut_callback);
//**********************************************************************
extern ret_type SMDrv_UART_Open(uart_module modul,uart_openinfo *ptype_info,uart_cb ut_callback);

//**********************************************************************
// ��������: �ر�driver module IDӲ����Ӧ��UART,��ʵ�ֵ͹���
// ���������	
//    modul: driver module ID,ֵ�ο�uart_module
// ���ز�����Ret_InvalidParam��Ret_OK
//**********************************************************************
extern ret_type SMDrv_UART_Close(uart_module modul);

//**********************************************************************
// ��������:  ����GPIO�ж����ȼ�,������uart�ж�
// �������:
//     modul: driver module ID,ֵ�ο�uart_module
//     prio:�ж����ȼ���bit0~2λֵ��Ч
// ���ز����� ��
//**********************************************************************
extern ret_type SMDrv_UART_SetIsrPrio(uart_module modul,uint32 prio);

//**********************************************************************
// ��������: ʹ��/ʧ��UARTĳ�����͵��ж�
// �������:
//     modul: driver module ID,ֵ�ο�uart_module
//     irq_type:�ж�����
//     benable: ʹ��/ʧ��UARTĳ�����ж�  =1--ʹ�ܣ� =0--ʧ��
// ���ز����� ��
//**********************************************************************
extern ret_type SMDrv_UART_EnableIrq(uart_module modul,uint32 irq_type,uint8 benable);

//**********************************************************************
// ��������: ��driver module ID��Ӧ��UARTд��n���ֽ�
// ���������	
//    modul: driver module ID
//    pData: Ҫ����������
//    len: Ҫ���������ݵĳ���
// ���ز�����
//    pu16LenWritten: ʵ�ʷ��͵ĳ���
//**********************************************************************
extern ret_type SMDrv_UART_WriteBytes(uart_module modul,uint8 *pData,uint16 len,uint16 *pu16LenWritten);

//**********************************************************************
// ��������: ��driver module ID��Ӧ��UARTдn���ֽ�
// ���������	
//    modul: driver module ID
//    pBuffer: Ҫ��ȡ�����ݻ���buffer
//    len: Ҫ��ȡ�����ݵĳ���
// ���ز�����
//    pu16LenWritten: ʵ�ʶ�ȡ�����ݳ���
//**********************************************************************
extern ret_type SMDrv_UART_ReadBytes(uart_module modul,uint8 *pBuffer,uint16 len,uint16 *pu16ReadLen);

#endif

