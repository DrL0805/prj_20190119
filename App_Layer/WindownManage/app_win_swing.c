#include "platform_common.h"

#include "app_variable.h"
#include "app_display.h"
#include "app_protocal.h"
#include "app_win_common.h"
#include "app_win_remind.h"
#include "app_win_swing.h"
#include "app_win_store.h"
#include "app_systerm.h"

/*************** func declaration ********************************/
//罗列窗口内所有菜单的处理函数
static uint16 App_Swing_Win_Tag_Remind(uint16 sysHandle,menuMessage message);
static uint16 App_Swing_Win_Tag_Key(uint16 sysHandle,menuMessage message);
static uint16 App_Swing_Win_Tag_Action(uint16 sysHandle,menuMessage message);
static uint16 App_Swing_Win_Tag_Gesture(uint16 sysHandle,menuMessage message);
static uint16 App_Swing_Win_Tag_PhoneApp(uint16 sysHandle,menuMessage message);
static uint16 App_Swing_Win_Tag_WinChange(uint16 sysHandle,menuMessage message);


/*************** macro define ********************************/
#define 		SWING_WIN_TAG_MAX 		10 			//窗口内最大菜单个数


//提醒窗口子窗口定义
typedef enum
{
	SUB_WIN_SWING_NONE 			= 0x0000,
	SUB_WIN_SWING_COUNT_3 		= 0x0001,
	SUB_WIN_SWING_COUNT_2 		= 0x0002,
	SUB_WIN_SWING_COUNT_1 		= 0x0004,
	SUB_WIN_SWING_FREQ			= 0x0010,
	SUB_WIN_SWING_CAL 			= 0x0040,
	SUB_WIN_SWING_TIME 			= 0x0080,
	SUB_WIN_SWING_TEMP 			= 0x0100,
	SUB_WIN_SWING_SAVE 			= 0x0200,
	SUB_WIN_SWING_SAVED 		= 0x0400,
}SwingSubWinHandle;

/************** variable define *****************************/
//罗列即时提醒窗口内所有菜单（实体菜单、虚拟菜单）
static 	WinMenu	SwingWindowMenu[SWING_WIN_TAG_MAX] = 
				{
					{TAG_REMIND,		0,0,	App_Swing_Win_Tag_Remind	},
					{TAG_KEY,			0,0,	App_Swing_Win_Tag_Key		},
					{TAG_ACTION,		0,0,	App_Swing_Win_Tag_Action	},
					{TAG_GESTURE,		0,0,	App_Swing_Win_Tag_Gesture	},
					{TAG_PHONE_APP,		0,0,	App_Swing_Win_Tag_PhoneApp  },
					{TAG_WIN_CHANGE,	0,0,	App_Swing_Win_Tag_WinChange	},
					{TAG_WIN_CHANGE,	0,0,	App_Swing_Win_Tag_WinChange	},
				};
	
static uint16 	curSubWinSwingHandle 	= SUB_WIN_SWING_NONE; 	
static uint8 	swingWinIdle;

/************** function define ******`***********************/

//**********************************************************************
// 函数功能：  窗口内部菜单初始化
// 输入参数：  无	

// 返回参数：  0x00:初始化成功，0xff：初始化失败
uint16 App_Swing_Win_Init(void)
{
	uint16 	sysHandleTemp;

	cycleFlag  		= 0;
	winTimeCnt 		= 0;
	swingWinIdle	= 0;

	App_PicRamClear();
	sysHandleTemp 		= WIN_SWING_HANDLE;
	curSubWinSwingHandle = SUB_WIN_SWING_NONE;
	AppSwimMainInterfaceDisp(systermConfig.systermLanguge);
	App_RoundDisp(ROUND_UP,ROUND_VERTICAL_PIXEL);

	return sysHandleTemp;
}

//**********************************************************************
// 函数功能：  窗口处理回调函数，每一窗口分配一个回调函数，在回调函数中执行具体菜单功能
// 输入参数：   sysHandle ：  窗口的句柄号
//				tagMenu：     当前响应的菜单
// 				message： 	  菜单发送的信息：动作、数值、状态等		

// 返回参数：  返回菜单响应的窗口句柄号
uint16 App_Swing_Win_CallBack(uint16 sysHandle,uint16 tagMenu,menuMessage message)
{
	uint8 	i;
	uint16 	handletemp;

	if (sysHandle != WIN_SWING_HANDLE)
	{
		return sysHandle;
	}

	for (i = 0; i < SWING_WIN_TAG_MAX; i++)
	{
		if (SwingWindowMenu[i].tag == tagMenu)
		{
			break;
		}
	}
	handletemp 		= sysHandle;

	if (i < SWING_WIN_TAG_MAX && SwingWindowMenu[i].callback != NULL)
	{		
		handletemp = SwingWindowMenu[i].callback(handletemp,message);
	}

	return handletemp;
}

//**********************************************************************
// 函数功能：  窗口内部菜单处理
// 输入参数：  无	
// 返回参数：  响应后的窗口句柄
static uint16 App_Swing_Win_Tag_Remind(uint16 sysHandle,menuMessage message)
{
	uint16 	sysHandleTemp;

	if (message.op == BAT_REMIND || message.op == OTA_REMIND)
	{
		if (message.state == BAT_CHARGING_STATE || message.state == BAT_LOW_VOLTAGE_STATE ||message.state == OTA_ENTER)
		{
			sysHandleTemp = App_Store_Win_Bak(sysHandle,message);
			// sysHandleTemp = WIN_STORE_HANDLE;
		}
		else
		{
			sysHandleTemp = App_Remind_Win_Bak(sysHandle,message);//标记即时提醒前的窗口句柄
			// sysHandleTemp = WIN_REMIND_HANDLE;
		}	
	}
	else
	{
		sysHandleTemp = App_Remind_Win_Bak(sysHandle,message);//标记即时提醒前的窗口句柄
		// sysHandleTemp = WIN_REMIND_HANDLE;
	}
	
	return sysHandleTemp;
}

//**********************************************************************
// 函数功能：  窗口内部按键菜单处理
// 输入参数：  sysHandle:当前句柄
//			   message： 信息内容		
// 返回参数：  响应后的窗口句柄
static uint16 App_Swing_Win_Tag_Key(uint16 sysHandle,menuMessage message)
{
	uint16 	sysHandleTemp;

	winTimeCnt 		= 0;
	sysHandleTemp 	= sysHandle;

	if (virtualHandle 	!= Virtual_Handle_None)
	{
		switch(virtualHandle)
		{
			case Virtual_Handle_Take_Photo:
			switch(message.val)
			{
				case PRESS_S0:
				case PRESS_S1:
				case PRESS_S2:
				App_Protocal_TakePhoto();
				break;
			}

			break;

			case Virtual_Handle_Author:
			switch(message.val)
			{
				case PRESS_S0:
				case PRESS_S1:
				case PRESS_S2:
				App_Protocal_AuthorPass();
				break;
			}
			break;
		}
	}
	else
	{
		switch(message.val)
		{
			case PRESS_S0:
			if (swingWinIdle)
			{
				App_PicRamClear();
				AppSwimbAltitudeSpeedDisp(150, 35);
				App_RoundDisp(ROUND_NONE,ROUND_HORIZONTAL_PIXEL);
				curSubWinSwingHandle = SUB_WIN_SWING_FREQ;
			}
			else
			{
				sysHandleTemp = WIN_SETTING_HANDLE;
			}
			swingWinIdle = 0;
			cycleFlag  = 0;
			break;

			case PRESS_S1:	
			sysHandleTemp = WIN_SETTING_HANDLE;		
			break;

			case PRESS_S2:	
			sysHandleTemp = WIN_SETTING_HANDLE;		
			break;

			case HOLD_LONG_S0:
			sysHandleTemp = WIN_STORE_HANDLE;
			App_Systerm_Reset();
			break;
			
		}		
	}

	return sysHandleTemp;
}

//**********************************************************************
// 函数功能：  窗口内部甩手动作菜单处理[甩手动作菜单为虚拟菜单]
// 输入参数：  sysHandle:当前句柄
//			   message： 信息内容		
// 返回参数：  响应后的窗口句柄
static uint16 App_Swing_Win_Tag_Action(uint16 sysHandle,menuMessage message)
{	
	if (virtualHandle 	!= Virtual_Handle_None)
	{
		switch(virtualHandle)
		{
			case Virtual_Handle_Take_Photo:
			App_Protocal_TakePhoto();
			break;

			case Virtual_Handle_Author:
			App_Protocal_AuthorPass();
			break;
		}
	}
	return sysHandle;
}

//**********************************************************************
// 函数功能：  窗口内部手势菜单处理[手势菜单为虚拟菜单]
// 输入参数：  sysHandle:当前句柄
//			   message： 信息内容		
// 返回参数：  响应后的窗口句柄
static uint16 App_Swing_Win_Tag_Gesture(uint16 sysHandle,menuMessage message)
{
	switch(message.op)
	{
		case GESTURE_ACTION_FORWARD:
		/* 前翻腕处理代码*/
		break;
	}
	return sysHandle;
}

//**********************************************************************
// 函数功能：  APP拍照虚拟菜单处理
// 输入参数：  sysHandle:当前句柄
//			   message： 信息内容	
// 返回参数：  响应后的窗口句柄
static uint16 App_Swing_Win_Tag_PhoneApp(uint16 sysHandle,menuMessage message)
{
	uint16 sysHandleTemp;

	sysHandleTemp = sysHandle;
	switch(message.op)
	{
		case ENTER_TAKE_PHOTO_MODE:
		virtualHandle 	= Virtual_Handle_Take_Photo;
		break;

		case ENTER_AUTHOR_MODE:
		virtualHandle 	= Virtual_Handle_Author;
		break;

		case EXIT_TAKE_PHOTO_MODE:
		case EXIT_AUTHOR_MODE:
		virtualHandle 	= Virtual_Handle_None;
		break;

		default:break;
	}
	return sysHandleTemp;
}

//**********************************************************************
// 函数功能：  响应窗口切换或窗口刷新
// 输入参数：  sysHandle:当前句柄
//			   message： 信息内容			
// 返回参数：  响应后的窗口句柄
static uint16 App_Swing_Win_Tag_WinChange(uint16 sysHandle,menuMessage message)
{
	uint16 			sysHandleTemp;
	rtc_time_s     timeTemp;
	weahter_s  		weatherinfo;

	winTimeCnt ++;
	sysHandleTemp 		= sysHandle;

	if (!swingWinIdle)
	{
		switch(curSubWinSwingHandle)
		{
			case SUB_WIN_SWING_NONE:
			if (winTimeCnt >= WIN_TIME_2SEC)
			{
				winTimeCnt = 0;
				App_PicRamClear();
				AppCommonCountDownStateDisp(3);
				App_RoundDisp(ROUND_NONE,ROUND_HORIZONTAL_PIXEL);
				curSubWinSwingHandle = SUB_WIN_SWING_COUNT_3;
			}
			break;

			case SUB_WIN_SWING_COUNT_3:
			if (winTimeCnt >= WIN_TIME_1SEC)
			{
				winTimeCnt = 0;
				App_PicRamClear();
				AppCommonCountDownStateDisp(2);
				App_RoundDisp(ROUND_NONE,ROUND_HORIZONTAL_PIXEL);
				curSubWinSwingHandle = SUB_WIN_SWING_COUNT_2;
			}
			break;

			case SUB_WIN_SWING_COUNT_2:
			if (winTimeCnt >= WIN_TIME_1SEC)
			{
				winTimeCnt = 0;
				App_PicRamClear();
				AppCommonCountDownStateDisp(1);
				App_RoundDisp(ROUND_NONE,ROUND_HORIZONTAL_PIXEL);
				curSubWinSwingHandle = SUB_WIN_SWING_COUNT_1;
			}
			break;

			case SUB_WIN_SWING_COUNT_1:
			if (winTimeCnt >= WIN_TIME_1SEC)
			{
				winTimeCnt = 0;
				MultiModuleTask_EventSet(OP_MOTO);
				App_PicRamClear();
				AppSwimbAltitudeSpeedDisp(150, 35);
				App_RoundDisp(ROUND_LEFT,ROUND_HORIZONTAL_PIXEL);
				curSubWinSwingHandle = SUB_WIN_SWING_FREQ;
			}
			break;

			case SUB_WIN_SWING_FREQ:
			if (cycleFlag)
			{
				if (winTimeCnt >= WIN_TIME_5SEC)
				{
					cycleFlag 		 	 = 0;
					swingWinIdle 		 = 1;
					//显示关闭
					App_DispOff();
				}
			}
			else
			{
				if (winTimeCnt >= WIN_TIME_2SEC)
				{
					winTimeCnt = 0;
					App_PicRamClear();
					AppCommonHrdKcalDisp(158, 12305);
					App_RoundDisp(ROUND_LEFT,ROUND_HORIZONTAL_PIXEL);
					curSubWinSwingHandle = SUB_WIN_SWING_CAL;
				}
			}
			break;

			case SUB_WIN_SWING_CAL:
			if (winTimeCnt >= WIN_TIME_2SEC)
			{
				winTimeCnt = 0;
				App_PicRamClear();
				AppCommonSportTimeDisp(12305);
				App_RoundDisp(ROUND_LEFT,ROUND_HORIZONTAL_PIXEL);
				curSubWinSwingHandle = SUB_WIN_SWING_TIME;
			}
			break;

			case SUB_WIN_SWING_TIME:
			if (winTimeCnt >= WIN_TIME_2SEC)
			{
				winTimeCnt = 0;
				App_PicRamClear();
				Mid_Rtc_TimeRead(&timeTemp);
				Mid_WeatherScene_TendencyGet(&weatherinfo);
				AppCommontimeTemperatureDisp(timeTemp, TIME_FORMAT_12, weatherinfo.weahterCurTemperature);
				App_RoundDisp(ROUND_LEFT,ROUND_HORIZONTAL_PIXEL);
				curSubWinSwingHandle = SUB_WIN_SWING_TEMP;
			}
			break;

			case SUB_WIN_SWING_TEMP:
			if (winTimeCnt >= WIN_TIME_2SEC)
			{
				winTimeCnt = 0;
				cycleFlag  = 1;		
				App_PicRamClear();
				AppSwimbAltitudeSpeedDisp(150, 35);
				App_RoundDisp(ROUND_LEFT,ROUND_HORIZONTAL_PIXEL);
				curSubWinSwingHandle = SUB_WIN_SWING_FREQ;
			}
			break;

			case SUB_WIN_SWING_SAVED:
			if (winTimeCnt >= WIN_TIME_2SEC)
			{
				cycleFlag 		  = 0;
				sysHandleTemp 	  = WIN_IDLE_HANDLE;		//休眠
				curSubWinSwingHandle = SUB_WIN_SWING_NONE;
			}
			break;

			default:
			cycleFlag 		  = 0;
			sysHandleTemp 	  = WIN_IDLE_HANDLE;		//休眠
			curSubWinSwingHandle = SUB_WIN_SWING_NONE;
			break;
		}
	}
	return sysHandleTemp;
}


