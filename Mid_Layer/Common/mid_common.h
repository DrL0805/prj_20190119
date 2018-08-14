#ifndef     SYS_VARIABLE_H
#define     SYS_VARIABLE_H

#define		PROTOCAL_LENGTH			32

//通信队列中的通信ID
typedef enum 
{
    BLE_MSG_SEND_WITH_RESEND,  //重发
    BLE_MSG_SEND,              //发送数据/CMD
    BLE_MSG_RECEIVE,           //接受完数据
    BLE_MSG_RESEND_TIMEOUT,    //重发超时
} ble_msg_id;

//数据块联合体
typedef union
{
    uint8  data[PROTOCAL_LENGTH - 5];
    struct 
    {
        uint8 interfaceType;                  //接口类型 
        uint8 interfaceIndex1;                //接口索引1
        uint8 interfaceIndex2;                //接口索引2
        uint8 parameter[PROTOCAL_LENGTH - 8]; //返回参数
    }content;
}load_t;

//BLE数据传输帧联合体
typedef union
{
    uint8 data[PROTOCAL_LENGTH];
    struct 
    {
        uint8 startSign;  //帧头
        uint8 version;    //版本
        uint8 loadLength; //长度
        uint8 routeMsg;   //路由信息
        uint8 flowControl;//流控信息
        load_t load;       //接口类型及参数
    }att;
    uint32 resendToutId;//重发消息的ID号
} protocal_msg_t;

//蓝牙数据交互帧
typedef struct
{
    ble_msg_id  id;
    protocal_msg_t packet;  //BLE数据传输帧
}ble_msg_t;

typedef void (*protocal_cb)(ble_msg_t *protocal);

#define 	DebugUartModule  		1//
#define     SYS_APP_HANDLE_NUM      20



uint32 VersionRead(void);

uint8 Mid_SystermReset(void);

#endif      // SYS_VARIABLE_H
