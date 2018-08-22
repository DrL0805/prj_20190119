#ifndef APP_REMIND_MANAGE_H
#define APP_REMIND_MANAGE_H

#include "ble_task.h"


#define APP_REMIND_RTT_DEBUG	3
#if (1 == APP_REMIND_RTT_DEBUG)	// 错误等级
#define APP_REMIND_RTT_LOG(...)
#define APP_REMIND_RTT_WARN(...)
#define APP_REMIND_RTT_ERR		SEGGER_RTT_printf
#elif (2 == APP_REMIND_RTT_DEBUG)	// 警告等级
#define APP_REMIND_RTT_LOG(...)
#define APP_REMIND_RTT_WARN	SEGGER_RTT_printf
#define APP_REMIND_RTT_ERR		SEGGER_RTT_printf
#elif (3 == APP_REMIND_RTT_DEBUG)	// 调试等级
#define APP_REMIND_RTT_LOG		SEGGER_RTT_printf
#define APP_REMIND_RTT_WARN	SEGGER_RTT_printf
#define APP_REMIND_RTT_ERR		SEGGER_RTT_printf
#else							// 调试关闭
#define APP_REMIND_RTT_LOG(...)
#define APP_REMIND_RTT_WARN(...)
#define APP_REMIND_RTT_ERR(...)
#endif

#define 	APP_REMIND_MSG_MAX_LENGHT 			214	//每条信息的最大长度

#define 	APP_REMIND_MSG_VALID_MAX_LENGHT  	192//信息有效内容部分的最大长度

#define 	APP_REMIND_MSG_MAX_LIST 			20 //可记录的最大信息条数[20 * 2] ： 20*198 ＝ 3960 flash一个区（剩余136bytes）

#define 	APP_REMIND_NANE_MAX_LENGHT 			20
#define 	APP_REMIND_PHONENUM_MAX_LENGHT 		20

#define 	APP_REMIND_ASCRIPTION_MAX_LENGHT 	10
#define 	APP_REMIND_DATAIL_MAX_LENGHT 		50
#define 	APP_REMIND_CONTENT_MAX_LENGHT 		62



//信息内容 62 * 2 
typedef union
{
	uint16_t msg[APP_REMIND_CONTENT_MAX_LENGHT];
	struct 
	{		
		uint16_t 	ascription[APP_REMIND_ASCRIPTION_MAX_LENGHT];//归属地（unicode 编码）
		uint16_t  	ascriptionlen;
		uint16_t 	detail[APP_REMIND_DATAIL_MAX_LENGHT]; // 消息具体内容，如：张四龙:[4条]张四龙:我是说我
		uint16_t 	detaillen;
	}u;
}msg_content_t;


//214 = 4 + 2 + [(20  + 1) * 2] + [(20 + 1 ) *2] + (62 * 2)
typedef union 
{	
	uint8_t 		data[APP_REMIND_MSG_MAX_LENGHT];
	struct 
	{	
		uint32_t		utc;
		uint16_t 		totallen;
		uint16_t 		name[APP_REMIND_NANE_MAX_LENGHT]; // 消息类型名称，如：微信，QQ等
		uint16_t 		namelen;
		uint16_t 		phonenumber[APP_REMIND_PHONENUM_MAX_LENGHT];//电话号码（unicode 编码）
		uint16_t 		phonenumberlen;
		msg_content_t 	content;
	}remindMsg;
}app_remind_msg_t;


void App_RemindManage_SaveInit(void);
void App_RemindManage_MissRemindSumRead(uint32_t msgClassify,uint8_t *listSumNum);
void App_RemindManage_MissRemindRead(app_remind_msg_t *dailyMissRemindMsg,uint32_t msgClassify,uint8_t listNum);
void App_RemindManage_MissRemindDelete(uint32_t msgClassify,uint8_t listnum);
void App_RemindManage_Process(protocal_msg_t protocal,uint8 phoneSystem);
void App_RemindManage_DetailRemindRead(app_remind_msg_t *detailRemindMsg);


#endif


