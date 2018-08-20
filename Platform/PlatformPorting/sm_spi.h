#ifndef __SM_SPI_H
#define __SM_SPI_H

#include "platform_common.h"

//定义使用spi的模块类型
typedef enum
{
    BLE_SPI_MODULE,    //使用SPI通讯的BLE蓝牙    
    FONT_SPI_MODULE,   //字库
    FLASH_SPI_MODULE,  //SPI FLASH
    MAX_SPI_MODULE,    
}spi_module;

//**********************************************************************
// 函数功能: 初始化SPI
// 输入参数：	
// 返回参数：
//**********************************************************************
extern void SMDrv_SPI_Init(void);

//**********************************************************************
// 函数功能: 根据driver module ID打开硬件对应的SPI
// 输入参数：	
//    modul: driver module ID
// 返回参数：Ret_InvalidParam或Ret_OK
//**********************************************************************
extern ret_type SMDrv_SPI_Open(spi_module modul);

//**********************************************************************
// 函数功能: 关闭driver module ID硬件对应的SPI
// 输入参数：	
//    modul: driver module ID
// 返回参数：Ret_InvalidParam或Ret_OK
//**********************************************************************
extern ret_type SMDrv_SPI_Close(spi_module modul);

//**********************************************************************
// 函数功能: 唤醒driver module ID打开硬件对应的SPI
// 输入参数：	
//    modul: driver module ID,值参考spi_module
// 返回参数：Ret_InvalidParam或Ret_OK
//**********************************************************************
extern ret_type SMDrv_SPI_Wake(spi_module modul);

//**********************************************************************
// 函数功能: 将driver module ID硬件对应的SPI进入sleep模式
// 输入参数:
//    modul: driver module ID,值参考spi_module
// 返回参数：Ret_InvalidParam或Ret_OK
//**********************************************************************
extern ret_type SMDrv_SPI_Sleep(spi_module modul);

//**********************************************************************
// 函数功能: 根据driver module获取spi module ID
// 输入参数：	
//    modul: driver module ID,值参考spi_module
// 返回参数：spi Module ID
//**********************************************************************
extern uint32 SMDrv_SPI_GetModuleId(spi_module modul);

//**********************************************************************
// 函数功能: 向driver module ID对应的SPI写1 个字节
// 输入参数：	
//    modul: driver module ID
// 返回参数：
//**********************************************************************
extern void SMDrv_SPI_WriteByte(spi_module modul,uint8 u8ch);

//**********************************************************************
// 函数功能: 向driver module ID对应的SPI写多个字节
// 输入参数：	
//    modul: driver module ID
// 返回参数：
//**********************************************************************
extern void SMDrv_SPI_WriteBytes(spi_module modul,uint8 *pData, uint16 length);

//**********************************************************************
// 函数功能: 向driver module ID对应的SPI读多个字节
// 输入参数：	
//    modul: driver module ID
// 返回参数：
//**********************************************************************
extern void SMDrv_SPI_ReadBytes(spi_module modul,uint8* pData, uint16 length);

#endif

