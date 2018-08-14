

#ifndef MID_FRONT_H
#define MID_FRONT_H

#include "platform_common.h"
#include "drv_font.h"

#define FRONT_TYPE_NUM 15

typedef enum
{
	FRONT_EVENT_ENABLE				= 0,
	FRONT_EVENT_DISABLE,
	FRONT_EVENT_READ_GB,
	FRONT_EVENT_READ_ASCII,
	FRONT_EVENT_READ_UI,
	FRONT_EVENT_WAKEUP,
	FRONT_EVENT_SLEEP,
	FRONT_EVENT_SELFTEST,
	FRONT_EVENT_READ_UNICODE,
	FRONT_EVENT_READ_FRONT_SIZE,
}front_event_id;

typedef enum 
{
	UI_FIRE 	= 0,
	UI_SHOE,
	UI_ALARM,
	UI_BAT_FULL,	
	UI_BAT_MID,
	UI_BAT_LESS,
	UI_BAT_LOW,
	UI_BAT_CHARGE,
	UI_WECHART,
	UI_NIGHT,
	UI_PHONE,
	UI_BLE,
	UI_MESG,
	UI_LOCATION,
	UI_MONBILE_PHONE,
	UI_MENU_UP,
	UI_MENU_DOWN,
	UI_MENU_LEFT,
	UI_MENU_RIGHT,
	UI_CAREFUL,
	UI_WEATHER,
	UI_SUN,
	UI_MAG,
	UI_HEARTRATE,
	UI_REST,
	UI_SPORT,
}ui_codeindex_s;

typedef struct
{
	uint8	wordWidth;//字宽（存储宽度）
	uint8	wordHeight;//字高（存储高度）
	uint8 validWidth;//字的有效宽度（根据实际字型小于或等于wordWidth）
	uint8 dataStringLengh;//数据流长度
}front_size_t;

typedef union
{
	uint16	codeGB;
	uint16 	codeUnicode;
	uint8 	codeASCII;
	uint8	codeIndexUI;
}code_t;
	

typedef struct 
{
	uint16_t			result;
	uint8_t*			dataAddr;
	uint8_t 			sizeKind;//input para
	uint8_t				cbId;
	code_t 				code;//input para
	front_size_t* 	wordSize;//return para
	void				(*Cb)(uint8_t* queueId, uint8_t result);
}front_para_t;

typedef struct 
{
	uint16_t			id;
	front_para_t 	para;
}front_event_t;



void   Mid_Front_Init(void);
uint16 Mid_Front_EventSetPre(front_event_t *msg);
uint16 Mid_Front_EventSetFail(front_event_t *msg);
uint16 Mid_Front_WaitComplete(front_event_t *msg);
uint16 Mid_Front_EventProcess(front_event_t *msg);

#endif


