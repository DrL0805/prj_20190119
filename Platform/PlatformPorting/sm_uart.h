#ifndef __SM_UART_H
#define __SM_UART_H

#include "platform_common.h"

//定义使用UART的模块类型
typedef enum
{
    BASE_UART_MODULE,   //基本通讯用，如调试，机芯夹具
    GPS_UART_MODULE,    //GPS
    BLE_UART_MODULE,    //使用UART通讯的BLE蓝牙
}uart_module;

//定义driver层通用的uart中断类型
#define UART_EVENT_NONE       0X00        //无中断
#define UART_EVENT_TX         0x01        //发送中断
#define UART_EVENT_RX         0x02        //接收中断
#define UART_EVENT_RX_TIMEOUT 0x04        //接收超时中断

typedef void (*uart_cb)(uint32 uart_event);

//**********************************************************************
// 函数功能: 初始化UART
// 输入参数：	
// 返回参数：
//**********************************************************************
extern void SMDrv_UART_Init(void);

//**********************************************************************
// 函数功能: 根据driver module ID打开硬件对应的UART
// 输入参数：	
//    modul: driver module ID,值参考uart_module
//    ptype_info:要设置的uart中断类型，FIFO类型, 若不需中断，则设置为NULL
//    ut_callback:上层注册的中断回调函数
// 返回参数：Ret_InvalidParam或Ret_OK
//**********************************************************************
extern ret_type SMDrv_UART_Open(uart_module modul,uart_cb ut_callback);

//**********************************************************************
// 函数功能: 关闭driver module ID硬件对应的UART,以实现低功耗
// 输入参数：	
//    modul: driver module ID,值参考uart_module
// 返回参数：Ret_InvalidParam或Ret_OK
//**********************************************************************
extern ret_type SMDrv_UART_Close(uart_module modul);

//**********************************************************************
// 函数功能:  设置GPIO中断优先级,并启动uart中断
// 输入参数:
//     modul: driver module ID,值参考uart_module
//     prio:中断优先级，bit0~2位值有效
// 返回参数： 无
//**********************************************************************
extern ret_type SMDrv_UART_SetIsrPrio(uart_module modul,uint32 prio);

//**********************************************************************
// 函数功能: 使能/失能UART某个类型的中断
// 输入参数:
//     modul: driver module ID,值参考uart_module
//     irq_type:中断类型
//     benable: 使能/失能UART某类型中断  =1--使能， =0--失能
// 返回参数： 无
//**********************************************************************
extern ret_type SMDrv_UART_EnableIrq(uart_module modul,uint32 irq_type,uint8 benable);

//**********************************************************************
// 函数功能: 向driver module ID对应的UART写度n个字节
// 输入参数：	
//    modul: driver module ID
//    pData: 要发生的数据
//    len: 要发生的数据的长度
// 返回参数：
//    pu16LenWritten: 实际发送的长度
//**********************************************************************
extern ret_type SMDrv_UART_WriteBytes(uart_module modul,uint8 *pData,uint16 len,uint16 *pu16LenWritten);

//**********************************************************************
// 函数功能: 向driver module ID对应的UART写n个字节
// 输入参数：	
//    modul: driver module ID
//    pBuffer: 要读取的数据缓存buffer
//    len: 要读取的数据的长度
// 返回参数：
//    pu16LenWritten: 实际读取的数据长度
//**********************************************************************
extern ret_type SMDrv_UART_ReadBytes(uart_module modul,uint8 *pBuffer,uint16 len,uint16 *pu16ReadLen);

//**********************************************************************
// 函数功能: 获取module使用UART发送缓冲区大小
// 输入参数：	
//    modul: driver module ID
// 返回参数：发送缓冲区大小
//**********************************************************************
extern uint32 SMDrv_UART_GetTxBuffSize(uart_module modul);

//**********************************************************************
// 函数功能: 获取module使用UART接收缓冲区大小
// 输入参数：	
//    modul: driver module ID
// 返回参数：接收缓冲区大小
//**********************************************************************
extern uint32 SMDrv_UART_GetRxBuffSize(uart_module modul);

#endif

