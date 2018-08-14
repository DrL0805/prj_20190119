#include "platform_common.h"

#include "app_variable.h"
#include "app_display.h"
#include "app_protocal.h"
#include "app_win_common.h"
#include "app_win_remind.h"
#include "app_win_time.h"
#include "app_win_store.h"
#include "app_systerm.h"
#include "app_sleepdata_manage.h"

/*************** func declaration ********************************/
//罗列窗口内所有菜单的处理函数
static uint16 App_Sleep_Win_Tag_Sleep(uint16 sysHandle,menuMessage message);
static uint16 App_Sleep_Win_Tag_Remind(uint16 sysHandle,menuMessage message);
static uint16 App_Sleep_Win_Tag_Key(uint16 sysHandle,menuMessage message);
static uint16 App_Sleep_Win_Tag_Action(uint16 sysHandle,menuMessage message);
static uint16 App_Sleep_Win_Tag_Gesture(uint16 sysHandle,menuMessage message);
static uint16 App_Sleep_Win_Tag_PhoneApp(uint16 sysHandle,menuMessage message);
static uint16 App_Sleep_Win_Tag_WinChange(uint16 sysHandle,menuMessage message);


/*************** macro define ********************************/
#define 		SLEEP_WIN_TAG_MAX 		8 			//窗口内最大菜单个数

//窗口子窗口定义
typedef enum
{
	SUB_WIN_SLEEP_NONE 		= 0,
	SUB_WIN_SLEEP_DUARATION,
	SUB_WIN_SLEEP_DETAIL,
	SUB_WIN_SLEEP_NULL,
}SleepSubWinHandle;


/************** variable define *****************************/
//罗列窗口内所有菜单（实体菜单、虚拟菜单）
static 	WinMenu	SleepWindowMenu[SLEEP_WIN_TAG_MAX] = 
				{
					{TAG_SLEEP,			0,0,	App_Sleep_Win_Tag_Sleep		},
					{TAG_REMIND,		0,0,	App_Sleep_Win_Tag_Remind	},
					{TAG_KEY,			0,0,	App_Sleep_Win_Tag_Key		},
					{TAG_ACTION,		0,0,	App_Sleep_Win_Tag_Action	},
					{TAG_GESTURE,		0,0,	App_Sleep_Win_Tag_Gesture	},
					{TAG_PHONE_APP,		0,0,	App_Sleep_Win_Tag_PhoneApp	},
					{TAG_WIN_CHANGE,	0,0,	App_Sleep_Win_Tag_WinChange	},
				};
	
static SleepSubWinHandle 	subWinSleepHandle;	//用于窗口下子窗口的标记，

sleep_ui_info sleepInfoTemp;

/************** function define ******`***********************/

//**********************************************************************
// 函数功能：  窗口内部菜单初始化
// 输入参数：  无	

// 返回参数：  0x00:初始化成功，0xff：初始化失败
uint16 App_Sleep_Win_Init(void)
{
	winTimeCnt = 0;
	cycleFlag  = 0;

	App_SleepData_Read(&sleepInfoTemp);
	App_PicRamClear();
	AppSleepMainInterfaceDisp(systermConfig.systermLanguge);
	App_RoundDisp(ROUND_UP,ROUND_VERTICAL_PIXEL);

	subWinSleepHandle = SUB_WIN_SLEEP_NONE;

	return WIN_SLEEP_HANDLE;
}

//**********************************************************************
// 函数功能：  窗口处理回调函数，每一窗口分配一个回调函数，在回调函数中执行具体菜单功能
// 输入参数：   sysHandle ：  窗口的句柄号
//				tagMenu：     当前响应的菜单
// 				message： 	  菜单发送的信息：动作、数值、状态等		

// 返回参数：  返回菜单响应的窗口句柄号
uint16 App_Sleep_Win_CallBack(uint16 sysHandle,uint16 tagMenu,menuMessage message)
{
	uint8 	i;
	uint16 	handletemp;

	if (sysHandle != WIN_SLEEP_HANDLE)
	{
		return sysHandle;
	}

	for (i = 0; i < SLEEP_WIN_TAG_MAX; i++)
	{
		if (SleepWindowMenu[i].tag == tagMenu)
		{
			break;
		}
	}
	handletemp 		= sysHandle;

	if (i < SLEEP_WIN_TAG_MAX && SleepWindowMenu[i].callback != NULL)
	{		
		handletemp = SleepWindowMenu[i].callback(handletemp,message);
	}

	return handletemp;
}

//**********************************************************************
// 函数功能：  窗口内部菜单处理
// 输入参数：  sysHandle:当前句柄
//			   message： 信息内容		
// 返回参数：  响应后的窗口句柄
static uint16 App_Sleep_Win_Tag_Sleep(uint16 sysHandle,menuMessage message)
{

	return sysHandle;
}

//**********************************************************************
// 函数功能：  窗口内部菜单处理
// 输入参数：  无	
// 返回参数：  响应后的窗口句柄
static uint16 App_Sleep_Win_Tag_Remind(uint16 sysHandle,menuMessage message)
{
	uint16 	sysHandleTemp;

	if (message.op == BAT_REMIND || message.op == OTA_REMIND)
	{
		if (message.state == BAT_CHARGING_STATE || message.state == BAT_LOW_VOLTAGE_STATE ||message.state == OTA_ENTER)
		{
			sysHandleTemp = App_Store_Win_Bak(sysHandle,message);
		}
		else
		{
			sysHandleTemp = App_Remind_Win_Bak(sysHandle,message);//标记即时提醒前的窗口句柄
		}
		
	}
	else
	{
		sysHandleTemp = App_Remind_Win_Bak(sysHandle,message);//标记即时提醒前的窗口句柄
	}
	return sysHandleTemp;
}

//**********************************************************************
// 函数功能：  窗口内部按键菜单处理
// 输入参数：  sysHandle:当前句柄
//			   message： 信息内容		
// 返回参数：  响应后的窗口句柄
static uint16 App_Sleep_Win_Tag_Key(uint16 sysHandle,menuMessage message)
{
	uint16 	sysHandleTemp;

	sysHandleTemp 		 = sysHandle;

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
		switch(subWinSleepHandle)
		{
			case SUB_WIN_SLEEP_NONE:
			case SUB_WIN_SLEEP_DUARATION:
			case SUB_WIN_SLEEP_NULL:
			case SUB_WIN_SLEEP_DETAIL:
			switch(message.val)
			{
				case PRESS_S0:
				sysHandleTemp 	= WIN_ALARM_HANDLE;
				break;

				case PRESS_S1:
				sysHandleTemp 	= WIN_ALARM_HANDLE;
				
				break;

				case PRESS_S2:
				sysHandleTemp 	= WIN_ALARM_HANDLE;
				
				break;

				case HOLD_SHORT_S0:

				break;
				
				case HOLD_LONG_S0:
				sysHandleTemp = WIN_STORE_HANDLE;
				App_Systerm_Reset();
				break;
			}
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
static uint16 App_Sleep_Win_Tag_Action(uint16 sysHandle,menuMessage message)
{	
	if (virtualHandle 	!= Virtual_Handle_None)
	{
		switch(virtualHandle)
		{
			case Virtual_Handle_Take_Photo:
			//App_Protocal_TakePhoto();
			break;

			case Virtual_Handle_Author:
			//App_Protocal_AuthorPass();
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
static uint16 App_Sleep_Win_Tag_Gesture(uint16 sysHandle,menuMessage message)
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
static uint16 App_Sleep_Win_Tag_PhoneApp(uint16 sysHandle,menuMessage message)
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
static uint16 App_Sleep_Win_Tag_WinChange(uint16 sysHandle,menuMessage message)
{
	uint16 sysHandleTemp;
	
	time_data_t sleepStartTemp;
	time_data_t sleepEndTemp;

	
	winTimeCnt ++;
	sysHandleTemp	  = sysHandle;

	switch(subWinSleepHandle)
	{
		case SUB_WIN_SLEEP_NONE:
		if (winTimeCnt >= WIN_TIME_2SEC)
		{	
			App_PicRamClear();
			winTimeCnt = 0;
			if (sleepInfoTemp.DurationM == 0)
			{
				subWinSleepHandle = SUB_WIN_SLEEP_NULL;
				AppSleepNodataDisp(systermConfig.systermLanguge);
			}
			else
			{
				subWinSleepHandle = SUB_WIN_SLEEP_DUARATION;		
				AppSleepDurationDisp(sleepInfoTemp.DurationM, sleepInfoTemp.Quality);
			}
			App_RoundDisp(ROUND_LEFT,ROUND_HORIZONTAL_PIXEL);
		}
		break;

		case SUB_WIN_SLEEP_DUARATION:

		if (cycleFlag)
		{
			if (winTimeCnt >= WIN_TIME_5SEC)
			{
				winTimeCnt        = 0;
				sysHandleTemp     = WIN_IDLE_HANDLE;
				subWinSleepHandle = SUB_WIN_SLEEP_NONE;				
			}
		}
		else
		{
			if (winTimeCnt >= WIN_TIME_2SEC)
			{
				sleepStartTemp.Hour = sleepInfoTemp.StartHour;
				sleepStartTemp.Min 	= sleepInfoTemp.StartMin;
				sleepEndTemp.Hour 	= sleepInfoTemp.StopHour;
				sleepEndTemp.Min 	= sleepInfoTemp.StopMin;

				App_PicRamClear();
				winTimeCnt        = 0;
				subWinSleepHandle = SUB_WIN_SLEEP_DETAIL;
				AppSleepPointDisp(&sleepStartTemp, &sleepEndTemp, systermConfig.systermTimeType);
				App_RoundDisp(ROUND_LEFT,ROUND_HORIZONTAL_PIXEL);
			}
		}		
		break;

		case SUB_WIN_SLEEP_DETAIL:
		if (winTimeCnt >= WIN_TIME_2SEC)
		{
			winTimeCnt        = 0;
			cycleFlag 		  = 1;
			subWinSleepHandle = SUB_WIN_SLEEP_DUARATION;
			App_PicRamClear();
			AppSleepDurationDisp(sleepInfoTemp.DurationM, sleepInfoTemp.Quality);
			App_RoundDisp(ROUND_LEFT,ROUND_HORIZONTAL_PIXEL);
		}
		break;

		case SUB_WIN_SLEEP_NULL:
		if (winTimeCnt >= WIN_TIME_2SEC)
		{
			winTimeCnt        = 0;
			sysHandleTemp     = WIN_IDLE_HANDLE;
			subWinSleepHandle = SUB_WIN_SLEEP_NONE;
		}
		break;

		default:break;
	}

	return sysHandleTemp;
}
