#ifndef __BLE_STACK__H
#define __BLE_STACK__H

#include "platform_common.h"
#include "mid_common.h"

#define BT_ADV_OFF   0X00    //关蓝牙广播，蓝牙关闭状态
#define BT_ADV_ON    0X01    //开蓝牙广播，蓝牙开启状态
#define BT_CONNECTED 0X02    //蓝牙已连接
#define BT_POWERON   0X03    //蓝牙上电
#define BT_POWEROFF  0X04    //蓝牙掉电

typedef enum 
{
    BLE_OTA_UNKOWN =0,  //无效状态
    BLE_OTA_START,      //OTA升级开始
    BLE_OTA_RECV,       //OTA升级接收数据
    BLE_OTA_RESET,      //OTA升级复位
    BLE_OTA_ERROR       //OTA升级失败
}Ble_Ota_Status;

//定义手机类型
#define BLE_PHONE_ANDROID    0 //Android手机
#define BLE_PHONE_IOS        1 //IOS手机

#define BLE_MODE_NORMAL  0   //正常通讯模式
#define BLE_MODE_OTA     1   //OTA模式

//**********************************************************************
// 如果BLE协议栈工作在MCU内部，则设置为 1: MCU与BLE通讯以函数调用的方式
// 设置为0:MCU与BLE通讯按协议定义
//**********************************************************************
#define BLE_STACK_RUN_INMCU    1

#if(BLE_STACK_RUN_INMCU == 1)
//Ble_Interact_Type: MCU与BLE数据交互类型,对应路由:0x12
typedef enum 
{
	BLE_STATUS_SET,             //开启/关闭 广播
	BLE_STATUS_GET,             //获取蓝牙状态
	BLE_POWERONOFF,             //蓝牙模块上电/掉电
	BLE_AVD_NAME_SET,           //设置广播名
    BLE_LINKINTV_SET,           //设置连接间隔
    BLE_CLEAR_PAIRNG_INFO,      //清除配对信息，IOS配对时保存的信息
	BLE_UNKNOW,                 //未知类型
}Ble_Interact_Type;

typedef enum 
{
	BLE_CONN_STATUS_ECHO = 0,   //蓝牙连接状态发生改变
	BLE_PAIRING_PASSKEY_ECHO,   //返回蓝牙配对pass key
	BLE_PAIRING_AUTH_STATUS,    //返回蓝牙配对结果
	BLE_ANCS_DISCOVERING,       //ANCS服务发现
	BLE_OTA_ECHO,               //告知MCU在OTA中
}Ble_Echo_Type;

#endif

typedef void (*ble_recv)(const uint8 *buf,uint16 len);
typedef void (*ble_echo)(Ble_Echo_Type echo,uint32 u32Status,uint8 u8Option);

#if(BLE_STACK_RUN_INMCU == 1)

//**********************************************************************
// 函数功能: MCU与BLE之间数据交互
// 输入参数：type:交互类型
//    param: 数据参数，如设置蓝牙开关状态，连接间隔，广播名等
//    option:可选参数，如广播名长度
// 返回参数：
//**********************************************************************
extern ret_type BLE_Stack_Interact(Ble_Interact_Type type,uint8 *param,uint8 option);

#endif

//**********************************************************************
// 函数功能: 设置蓝牙Echo动作callback
// 输入参数：pfun:callback
// 返回参数：ret_type
//**********************************************************************
extern ret_type BLE_Stack_SetEcho_CallBack(ble_echo pfun);

//**********************************************************************
// 函数功能: 检查ble发送是否空闲
// 输入参数：无
// 返回参数：1:空闲，可发送；  0: 发送忙
//**********************************************************************
extern uint8 BLE_Stack_CheckSendStatus(void);

//**********************************************************************
// 函数功能: 设置MAC地址
// 输入参数：mac_buf
// 返回参数：Ret_OK:正常，  
//    Ret_InvalidParam: 参数非法
//**********************************************************************
extern ret_type BLE_Stack_SetMac(uint8 *mac_buf);

//**********************************************************************
// 函数功能: 向BLE发送数据/命令
// 输入参数：buf/len:发送数据的buffer和长度
// 返回参数：Ret_OK:正常，  
//    Ret_InvalidParam: 参数非法
//**********************************************************************
extern ret_type BLE_Stack_Send(uint8 *buf,uint8 len);

//**********************************************************************
// 函数功能: 注册上层应用的数据及命令通道接收回调函数和ota数据通道回调函数
// 输入参数：recv_data_handler:数据接收callback，
// 返回参数：Ret_OK:正常，  
//    Ret_InvalidParam: 参数非法，data和ota callback需同时有效
//**********************************************************************
extern ret_type BLE_Stack_SetRecvCallBack(ble_recv recv_data_handler);

//**********************************************************************
// 函数功能: 获取ble模式
// 输入参数: u8Mode 
// 返回参数： BLE_MODE_OTA       OTA模式
//            BLE_MODE_NORMAL   正常通讯模式
//**********************************************************************
extern uint8 BLE_Stack_GetMode(void);

//**********************************************************************
// 函数功能: 设置ble模式
// 输入参数: u8Mode = BLE_MODE_OTA OTA模式
//           =  BLE_MODE_NORMAL    正常通讯模式
// 返回参数：无
//**********************************************************************
extern void BLE_Stack_SetMode(uint8 u8Mode);

//**********************************************************************
// 函数功能: 初始化并创建BLE协议栈消息事件处理任务
// 输入参数：
// 返回参数：无
//**********************************************************************
extern void BLE_Stack_Init(void);

#endif

