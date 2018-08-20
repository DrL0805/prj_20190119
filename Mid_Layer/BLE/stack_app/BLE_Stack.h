#ifndef __BLE_STACK__H
#define __BLE_STACK__H

#include "platform_common.h"
#include "mid_common.h"

#define BT_ADV_OFF   0X00    //�������㲥�������ر�״̬
#define BT_ADV_ON    0X01    //�������㲥����������״̬
#define BT_CONNECTED 0X02    //����������
#define BT_POWERON   0X03    //�����ϵ�
#define BT_POWEROFF  0X04    //��������

typedef enum 
{
    BLE_OTA_UNKOWN =0,  //��Ч״̬
    BLE_OTA_START,      //OTA������ʼ
    BLE_OTA_RECV,       //OTA������������
    BLE_OTA_RESET,      //OTA������λ
    BLE_OTA_ERROR       //OTA����ʧ��
}Ble_Ota_Status;

//�����ֻ�����
#define BLE_PHONE_ANDROID    0 //Android�ֻ�
#define BLE_PHONE_IOS        1 //IOS�ֻ�

#define BLE_MODE_NORMAL  0   //����ͨѶģʽ
#define BLE_MODE_OTA     1   //OTAģʽ

//**********************************************************************
// ���BLEЭ��ջ������MCU�ڲ���������Ϊ 1: MCU��BLEͨѶ�Ժ������õķ�ʽ
// ����Ϊ0:MCU��BLEͨѶ��Э�鶨��
//**********************************************************************
#define BLE_STACK_RUN_INMCU    1

#if(BLE_STACK_RUN_INMCU == 1)
//Ble_Interact_Type: MCU��BLE���ݽ�������,��Ӧ·��:0x12
typedef enum 
{
	BLE_STATUS_SET,             //����/�ر� �㲥
	BLE_STATUS_GET,             //��ȡ����״̬
	BLE_POWERONOFF,             //����ģ���ϵ�/����
	BLE_AVD_NAME_SET,           //���ù㲥��
    BLE_LINKINTV_SET,           //�������Ӽ��
    BLE_CLEAR_PAIRNG_INFO,      //��������Ϣ��IOS���ʱ�������Ϣ
	BLE_UNKNOW,                 //δ֪����
}Ble_Interact_Type;

typedef enum 
{
	BLE_CONN_STATUS_ECHO = 0,   //��������״̬�����ı�
	BLE_PAIRING_PASSKEY_ECHO,   //�����������pass key
	BLE_PAIRING_AUTH_STATUS,    //����������Խ��
	BLE_ANCS_DISCOVERING,       //ANCS������
	BLE_OTA_ECHO,               //��֪MCU��OTA��
}Ble_Echo_Type;

#endif

typedef void (*ble_recv)(const uint8 *buf,uint16 len);
typedef void (*ble_echo)(Ble_Echo_Type echo,uint32 u32Status,uint8 u8Option);

#if(BLE_STACK_RUN_INMCU == 1)

//**********************************************************************
// ��������: MCU��BLE֮�����ݽ���
// ���������type:��������
//    param: ���ݲ�������������������״̬�����Ӽ�����㲥����
//    option:��ѡ��������㲥������
// ���ز�����
//**********************************************************************
extern ret_type BLE_Stack_Interact(Ble_Interact_Type type,uint8 *param,uint8 option);

#endif

//**********************************************************************
// ��������: ��������Echo����callback
// ���������pfun:callback
// ���ز�����ret_type
//**********************************************************************
extern ret_type BLE_Stack_SetEcho_CallBack(ble_echo pfun);

//**********************************************************************
// ��������: ���ble�����Ƿ����
// �����������
// ���ز�����1:���У��ɷ��ͣ�  0: ����æ
//**********************************************************************
extern uint8 BLE_Stack_CheckSendStatus(void);

//**********************************************************************
// ��������: ����MAC��ַ
// ���������mac_buf
// ���ز�����Ret_OK:������  
//    Ret_InvalidParam: �����Ƿ�
//**********************************************************************
extern ret_type BLE_Stack_SetMac(uint8 *mac_buf);

//**********************************************************************
// ��������: ��BLE��������/����
// ���������buf/len:�������ݵ�buffer�ͳ���
// ���ز�����Ret_OK:������  
//    Ret_InvalidParam: �����Ƿ�
//**********************************************************************
extern ret_type BLE_Stack_Send(uint8 *buf,uint8 len);

//**********************************************************************
// ��������: ע���ϲ�Ӧ�õ����ݼ�����ͨ�����ջص�������ota����ͨ���ص�����
// ���������recv_data_handler:���ݽ���callback��
// ���ز�����Ret_OK:������  
//    Ret_InvalidParam: �����Ƿ���data��ota callback��ͬʱ��Ч
//**********************************************************************
extern ret_type BLE_Stack_SetRecvCallBack(ble_recv recv_data_handler);

//**********************************************************************
// ��������: ��ȡbleģʽ
// �������: u8Mode 
// ���ز����� BLE_MODE_OTA       OTAģʽ
//            BLE_MODE_NORMAL   ����ͨѶģʽ
//**********************************************************************
extern uint8 BLE_Stack_GetMode(void);

//**********************************************************************
// ��������: ����bleģʽ
// �������: u8Mode = BLE_MODE_OTA OTAģʽ
//           =  BLE_MODE_NORMAL    ����ͨѶģʽ
// ���ز�������
//**********************************************************************
extern void BLE_Stack_SetMode(uint8 u8Mode);

//**********************************************************************
// ��������: ��ʼ��������BLEЭ��ջ��Ϣ�¼���������
// ���������
// ���ز�������
//**********************************************************************
extern void BLE_Stack_Init(void);

#endif

