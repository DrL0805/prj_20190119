#ifndef	__APP_LCD_H
#define __APP_LCD_H

#include "platform_common.h"

#include "mid_Lcd.h"
#include "app_win_common.h"

#define APP_LCD_RTT_DEBUG	3
#if (1 == APP_LCD_RTT_DEBUG)	// ����ȼ�
#define APP_LCD_RTT_LOG(...)
#define APP_LCD_RTT_WARN(...)
#define APP_LCD_RTT_ERR		SEGGER_RTT_printf
#elif (2 == APP_LCD_RTT_DEBUG)	// ����ȼ�
#define APP_LCD_RTT_LOG(...)
#define APP_LCD_RTT_WARN	SEGGER_RTT_printf
#define APP_LCD_RTT_ERR		SEGGER_RTT_printf
#elif (3 == APP_LCD_RTT_DEBUG)	// ���Եȼ�
#define APP_LCD_RTT_LOG		SEGGER_RTT_printf
#define APP_LCD_RTT_WARN	SEGGER_RTT_printf
#define APP_LCD_RTT_ERR		SEGGER_RTT_printf
#else							// ���Թر�
#define APP_LCD_RTT_LOG(...)
#define APP_LCD_RTT_WARN(...)
#define APP_LCD_RTT_ERR(...)
#endif

// LCD�¼�����
typedef enum
{
	eAppLcdEventOuter,	// �ⲿ�¼����簴���¼�
	eAppLcdEventInside	// �ڲ��¼����綨ʱ��ѯ�¼�
	/* �����¼����ж��� */
}eAppLcdEvent;

typedef struct
{
	/* ��Ϣ�����Զ��� */
	uint8_t Id;
}App_lcd_Msg_t;

/* �м������������ݽṹ�� */
typedef struct
{
	bool	 			RefreshFlg;		// ǿ��ˢ�±�־
	eAppLcdEvent 		Id;				// lcd�������¼�����
	eAppWinHandle 		MainWinHandle;	// �����ھ��
	eAppSubWinHandle 	SubWinHandle;	// �������ھ��
	union
	{
		App_Win_Msg_T		Outer;			// �ⲿ�¼���Ϣ����
		App_lcd_Msg_t		Inside;			// �ڲ��¼���Ϣ����
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



