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
#define UART_EVENT_NONE       0XFFFFFFFF               //无中断
#define UART_EVENT_TX         AM_HAL_UART_INT_TX       //发送中断
#define UART_EVENT_RX         AM_HAL_UART_INT_RX       //接收中断
#define UART_EVENT_RX_TIMEOUT AM_HAL_UART_INT_RX_TMOUT //接收超时中断

//定义driver层通用的触发uart中断时FIFO长度
#define UART_FIFO_NONE   0XFFFFFFFF               //不设置FIFO
//tx fifo depth
#define UART_TX_FIFO_1_8 AM_HAL_UART_TX_FIFO_1_8  //数据到1/8 FIFO时，触发发送中断
#define UART_TX_FIFO_1_4 AM_HAL_UART_TX_FIFO_1_4  //数据到1/4 FIFO时，触发发送中断
#define UART_TX_FIFO_1_2 AM_HAL_UART_TX_FIFO_1_2  //数据到1/2 FIFO时，触发发送中断
#define UART_TX_FIFO_3_4 AM_HAL_UART_TX_FIFO_3_4  //数据到3/4 FIFO时，触发发送中断
#define UART_TX_FIFO_7_8 AM_HAL_UART_TX_FIFO_7_8  //数据到7/8 FIFO时，触发发送中断
//rx fifo depth
#define UART_RX_FIFO_1_8 AM_HAL_UART_RX_FIFO_1_8  //数据到1/8 FIFO时，触发接收中断
#define UART_RX_FIFO_1_4 AM_HAL_UART_RX_FIFO_1_4  //数据到1/4 FIFO时，触发接收中断
#define UART_RX_FIFO_1_2 AM_HAL_UART_RX_FIFO_1_2  //数据到1/2 FIFO时，触发接收中断
#define UART_RX_FIFO_3_4 AM_HAL_UART_RX_FIFO_3_4  //数据到3/4 FIFO时，触发接收中断
#define UART_RX_FIFO_7_8 AM_HAL_UART_RX_FIFO_7_8  //数据到7/8 FIFO时，触发接收中断

typedef struct
{
    uint32 u32event_type;  //值参考UART_EVENT_XXX
    uint32 u32fifo_type;   //值参考UART_TX_FIFO_XXX或UART_RX_FIFO_XXX
}uart_openinfo;


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
//
// 例子: 使能uart接收发生中断，接收fifo中数据为1/8时触发接收中断
//      uart_openinfo type_info;
//      type_info.u32event_type = UART_EVENT_RX | UART_EVENT_TX;
//      type_info.u32fifo_type =  UART_RX_FIFO_1_8;
//      SMDrv_UART_Open(GPS_UART_MODULE,&type_info,ut_callback);
//**********************************************************************
extern ret_type SMDrv_UART_Open(uart_module modul,uart_openinfo *ptype_info,uart_cb ut_callback);

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

#endif

