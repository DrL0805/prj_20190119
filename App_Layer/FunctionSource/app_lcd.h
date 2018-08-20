#ifndef	__APP_LCD_H
#define __APP_LCD_H

#include "platform_common.h"

#include "mid_Lcd.h"
#include "app_win_common.h"

#define APP_LCD_RTT_DEBUG	3
#if (1 == APP_LCD_RTT_DEBUG)	// 错误等级
#define APP_LCD_RTT_LOG(...)
#define APP_LCD_RTT_WARN(...)
#define APP_LCD_RTT_ERR		SEGGER_RTT_printf
#elif (2 == APP_LCD_RTT_DEBUG)	// 警告等级
#define APP_LCD_RTT_LOG(...)
#define APP_LCD_RTT_WARN	SEGGER_RTT_printf
#define APP_LCD_RTT_ERR		SEGGER_RTT_printf
#elif (3 == APP_LCD_RTT_DEBUG)	// 调试等级
#define APP_LCD_RTT_LOG		SEGGER_RTT_printf
#define APP_LCD_RTT_WARN	SEGGER_RTT_printf
#define APP_LCD_RTT_ERR		SEGGER_RTT_printf
#else							// 调试关闭
#define APP_LCD_RTT_LOG(...)
#define APP_LCD_RTT_WARN(...)
#define APP_LCD_RTT_ERR(...)
#endif

// LCD事件类型
typedef enum
{
	eAppLcdEventOuter,	// 外部事件，如按键事件
	eAppLcdEventInside	// 内部事件，如定时轮询事件
	/* 其他事件自行定义 */
}eAppLcdEvent;

typedef struct
{
	/* 消息内容自定义 */
	uint8_t Id;
}App_lcd_Msg_t;

/* 中间层任务调度数据结构体 */
typedef struct
{
	bool	 			RefreshFlg;		// 强制刷新标志
	eAppLcdEvent 		Id;				// lcd触发的事件类型
	eAppWinHandle 		MainWinHandle;	// 主窗口句柄
	eAppSubWinHandle 	SubWinHandle;	// 二级窗口句柄
	union
	{
		App_Win_Msg_T		Outer;			// 外部事件消息内容
		App_lcd_Msg_t		Inside;			// 内部事件消息内容
	}Param;
}App_Lcd_TaskMsg_T;

typedef enum
{
	eLcdBacklightOff,
	eLcdBacklightOn,
	eLcdBacklightWait,
}eLcdBacklightType;

typedef struct 
{
	eLcdBacklightType	BacklightType;
	uint32_t 			BacklightCnt;
}App_Lcd_Param_t;

void App_Lcd_TaskEventSet(App_Lcd_TaskMsg_T* Msg, uint8_t FromISR);
void App_Lcd_TaskCreate(void);
#endif



