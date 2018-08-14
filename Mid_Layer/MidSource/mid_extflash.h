#ifndef	MID_EXTFLASH_H
#define	MID_EXTFLASH_H

#include "platform_common.h"

// Note: 252--255 section was used for test and SN / MAC
typedef enum 
{
	EXTFLASH_EVENT_ENABLE	= 0x0001,
	EXTFLASH_EVENT_DISABLE,
	EXTFLASH_EVENT_SELFTEST,
	EXTFLASH_EVENT_WRITE_SN,
	EXTFLASH_EVENT_READ_SN,
	EXTFLASH_EVENT_WRITE_BLEMAC,
	EXTFLASH_EVENT_READ_BLEMAC,
	EXTFLASH_EVENT_READ_MAC,
	EXTFLASH_EVENT_SLEEP,
	EXTFLASH_EVENT_WAKEUP,
	EXTFLASH_EVENT_4K_ERASE,
	EXTFLASH_EVENT_READ,
	EXTFLASH_EVENT_WRITE,
	EXTFLASH_EVENT_WRITE_WITH_BUF,
	EXTFLASH_EVENT_WRITE_BLE_BROCAST,
	EXTFLASH_EVENT_READ_BLE_BROCAST,
	EXTFLASH_EVENT_WRITE_SN_CUSTOM,
	EXTFLASH_EVENT_READ_SN_CUSTOM,
	EXTFLASH_EVENT_WRITE_BLE_PASSKEY,   //ble passkey
	EXTFLASH_EVENT_READ_BLE_PASSKEY,
} extflash_event_id;

#define	PROCESSTYPE_EVENT       0
#define	PROCESSTYPE_CALL        1

#define	EXTFLASH_SECTOR_LENGTH	4096

typedef struct 
{
	uint16	result;
	uint8*	dataAddr;
	uint8	data[16];
	uint32	length;
	uint32	startAddr;
	uint32	endAddr;
	uint8   cbId;
	void   (*Cb)(uint8* queueId, uint8 result);
}extflash_para_t;

typedef	struct
{
	uint16			id;
	extflash_para_t 	para;
}extflash_event_t;

//**********************************************************************
// 函数功能:	8Mflash 软件初始化，队列创建
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
void Mid_ExtFlash_Init(void);

//**********************************************************************
// 函数功能:  extflash事件放入队列前预处理
// 输入参数：	
// msg 	:	  事件指针	
// 返回参数：  0x00:操作成功
// 			   0xff:操作失败
//**********************************************************************
uint16 Mid_ExtFlash_EventSetPre(extflash_event_t *msg);

//**********************************************************************
// 函数功能:	任务队列放置不进时，需要进行清除二值资源，对其解除占用
// 输入参数：	
// msg 	:		事件指针	
// 返回参数：	0x00:操作成功
// 				0xff:操作失败
//**********************************************************************
uint16 Mid_ExtFlash_EventSetFail(extflash_event_t *msg);

//**********************************************************************
// 函数功能:  等待时间在flash任务中执行结束
// 输入参数：	
// queueId :  指定二值同步信号id	
// 返回参数： 0x00:操作成功
// 			  0xff:操作失败
//**********************************************************************
uint16 Mid_ExtFlash_WaitComplete(extflash_event_t *msg);

//**********************************************************************
// 函数功能:  获取BLE MAC/flash uuid
// 输入参数：	
// 返回参数： 0x00:操作成功
// 			  0xff:操作失败
//**********************************************************************
uint16 Mid_ExtFlash_ReadBleInfo(extflash_event_id event,extflash_para_t *para);

//**********************************************************************
// 函数功能:  根据事件ID进行处理
// 输入参数：	
// queueId :  指定二值同步信号id	
// 返回参数： 0x00:操作成功
// 		      0xff:操作失败
//**********************************************************************
uint16 Mid_ExtFlash_EventProcess(extflash_event_t *msg);

#endif		//	EXTFLASH_APP_H
