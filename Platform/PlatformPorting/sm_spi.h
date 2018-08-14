#ifndef __SM_SPI_H
#define __SM_SPI_H

#include "platform_common.h"

//����ʹ��spi��ģ������
typedef enum
{
    BLE_SPI_MODULE,    //ʹ��SPIͨѶ��BLE����    
    FONT_SPI_MODULE,   //�ֿ�
    FLASH_SPI_MODULE,  //SPI FLASH
    MAX_SPI_MODULE,    
}spi_module;

#if 0
typedef enum
{
    SPI_OPEN_INIT,  //�Գ�ʼ��spi��ʽopen����ϵͳ������ʱ��
    SPI_OPEN_AWAKE, //�Ի���spi��ʽopen����Ҫ��spi����sleepģʽ���ٴλ���
}spi_open_cmd;

typedef enum
{
    SPI_CLOSE_DEINIT,  //�Թر�spi��ʽclose����SPI_OPEN_INIT ƥ��
    SPI_CLOSE_SLEEP,   //�����߷�ʽclose����SPI_OPEN_AWAKEƥ��
}spi_open_cmd;
#endif

//**********************************************************************
// ��������: ��ʼ��SPI
// ���������	
// ���ز�����
//**********************************************************************
extern void SMDrv_SPI_Init(void);

//**********************************************************************
// ��������: ����driver module ID��Ӳ����Ӧ��SPI
// ���������	
//    modul: driver module ID
// ���ز�����Ret_InvalidParam��Ret_OK
//**********************************************************************
extern ret_type SMDrv_SPI_Open(spi_module modul);

//**********************************************************************
// ��������: �ر�driver module IDӲ����Ӧ��SPI
// ���������	
//    modul: driver module ID
// ���ز�����Ret_InvalidParam��Ret_OK
//**********************************************************************
extern ret_type SMDrv_SPI_Close(spi_module modul);

//**********************************************************************
// ��������: ����driver module ID��Ӳ����Ӧ��SPI
// ���������	
//    modul: driver module ID,ֵ�ο�spi_module
// ���ز�����Ret_InvalidParam��Ret_OK
//**********************************************************************
extern ret_type SMDrv_SPI_Wake(spi_module modul);

//**********************************************************************
// ��������: ��driver module IDӲ����Ӧ��SPI����sleepģʽ
// �������:
//    modul: driver module ID,ֵ�ο�spi_module
// ���ز�����Ret_InvalidParam��Ret_OK
//**********************************************************************
extern ret_type SMDrv_SPI_Sleep(spi_module modul);

//**********************************************************************
// ��������: ����driver module��ȡspi module ID
// ���������	
//    modul: driver module ID,ֵ�ο�spi_module
// ���ز�����spi Module ID
//**********************************************************************
extern uint32 SMDrv_SPI_GetModuleId(spi_module modul);

//**********************************************************************
// ��������: ��ȡSPI��Դ��������driverģ��ʹ��ͬһ��SPIʱ��Ҫ��������
// ���������	
//    modul: driver module ID
//    u32timeout:�ȴ���ʱʱ��
// ���ز�����
//**********************************************************************
extern ret_type SMDrv_SPI_LockMutex(spi_module modul,uint32 u32timeout);

//**********************************************************************
// ��������: �ͷ�SPI��Դ
// ���������	
//    modul: driver module ID
// ���ز�����
//**********************************************************************
extern ret_type SMDrv_SPI_UnLockMutex(spi_module modul);

//**********************************************************************
// ��������: ��driver module ID��Ӧ��SPIд1 ���ֽ�
// ���������	
//    modul: driver module ID
// ���ز�����
//**********************************************************************
extern void SMDrv_SPI_WriteByte(spi_module modul,uint8 u8ch);

//**********************************************************************
// ��������: ��driver module ID��Ӧ��SPIд����ֽ�
// ���������	
//    modul: driver module ID
// ���ز�����
//**********************************************************************
extern void SMDrv_SPI_WriteBytes(spi_module modul,uint8 *pData, uint16 length);

//**********************************************************************
// ��������: ��driver module ID��Ӧ��SPI������ֽ�
// ���������	
//    modul: driver module ID
// ���ز�����
//**********************************************************************
extern void SMDrv_SPI_ReadBytes(spi_module modul,uint8* pData, uint16 length);

#endif

