#include "platform_common.h"

#include "app_variable.h"
#include "app_display.h"
#include "app_protocal.h"
#include "app_win_common.h"
#include "app_win_remind.h"
#include "app_win_time.h"
#include "app_win_store.h"
#include "app_systerm.h"


/*************** func declaration ********************************/
//罗列时间窗口内所有菜单的处理函数
static uint16 App_Time_Win_Tag_Time(uint16 sysHandle,menuMessage message);
static uint16 App_Time_Win_Tag_Remind(uint16 sysHandle,menuMessage message);
static uint16 App_Time_Win_Tag_Key(uint16 sysHandle,menuMessage message);
static uint16 App_Time_Win_Tag_Action(uint16 sysHandle,menuMessage message);
static uint16 App_Time_Win_Tag_Gesture(uint16 sysHandle,menuMessage message);
static uint16 App_Time_Win_Tag_PhoneApp(uint16 sysHandle,menuMessage message);
static uint16 App_Time_Win_Tag_IconFlick(uint16 sysHandle,menuMessage message);
static uint16 App_Time_Win_Tag_WinChange(uint16 sysHandle,menuMessage message);

/*************** macro define ********************************/
#define 		TIME_WIN_TAG_MAX 		10 			//窗口内最大菜单个数

//时间窗口子窗口定义
typedef enum
{
	SUB_WIN_TIME_NONE 		= 0,
	SUB_WIN_TIME_DTAE,
	SUB_WIN_TIME_RTC_TIME,
	SUB_WIN_TIME_WEATHER,
}TimeSubWinHandle;


/************** variable define *****************************/
//罗列时间窗口内所有菜单（实体菜单、虚拟菜单）
static 	WinMenu	TimeWindowMenu[TIME_WIN_TAG_MAX] = 
				{
					{TAG_TIME,			0,0,	App_Time_Win_Tag_Time		},
					{TAG_REMIND,		0,0,	App_Time_Win_Tag_Remind		},
					{TAG_KEY,			0,0,	App_Time_Win_Tag_Key		},
					{TAG_ACTION,		0,0,	App_Time_Win_Tag_Action		},
					{TAG_GESTURE,		0,0,	App_Time_Win_Tag_Gesture	},
					{TAG_PHONE_APP,		0,0,	App_Time_Win_Tag_PhoneApp	},
					{TAG_ICON_FLICK, 	0,0,	App_Time_Win_Tag_IconFlick	},
					{TAG_WIN_CHANGE,	0,0,	App_Time_Win_Tag_WinChange	},
				};	


static TimeSubWinHandle 	subWinTimeHandle;	//用于窗口下子窗口的标记，

/************** function define ******`***********************/

//**********************************************************************
// 函数功能：  窗口内部菜单初始化(显示)
// 输入参数：  无	

// 返回参数：  0x00:初始化成功，0xff：初始化失败
uint16 App_Time_Win_Init(void)
{
	rtc_time_s     timeTemp;
	uint8 		 	batLevel;
	
	winTimeCnt  = 0;
	cycleFlag 	= 0;
	App_PicRamClear();
	Mid_Rtc_TimeRead(&timeTemp);
	AppTimeDateDisp(timeTemp);
	AppTimeWeekDisp(timeTemp);
	if (App_Remind_Win_MissCallRead())
	{
		AppTimeMisscallDisp(PIC_DISP_ON);

	}
	if (bleState == BLE_BROADCAST || bleState == BLE_CONNECT || bleState == BLE_POWERON)
	{
		AppTimeBleDisp(PIC_DISP_ON);
	}
		
	batLevel 	= Mid_Bat_LevelRead();
	AppTimeBatDisp(batLevel);	
	App_RoundDisp(ROUND_UP,ROUND_VERTICAL_PIXEL);
    subWinTimeHandle 	= SUB_WIN_TIME_NONE;

	return WIN_TIME_HANDLE;
}

//**********************************************************************
// 函数功能：  窗口处理回调函数，每一窗口分配一个回调函数，在回调函数中执行具体菜单功能
// 输入参数：   sysHandle ：  窗口的句柄号
//				tagMenu：     当前响应的菜单
// 				message： 	  菜单发送的信息：动作、数值、状态等		

// 返回参数：  返回菜单响应的窗口句柄号
uint16 App_Time_Win_CallBack(uint16 sysHandle,uint16 tagMenu,menuMessage message)
{
	uint8 	i;
	uint16 	handletemp;	

	if (sysHandle != WIN_TIME_HANDLE)
	{
		return sysHandle;
	}

	for (i = 0; i < TIME_WIN_TAG_MAX; i++)
	{
		if (TimeWindowMenu[i].tag == tagMenu)
		{
			break;
		}
	}
	handletemp 		= sysHandle;

	if (i < TIME_WIN_TAG_MAX && TimeWindowMenu[i].callback != NULL)
	{		
		handletemp = TimeWindowMenu[i].callback(handletemp,message);
	}

	return handletemp;
}

//**********************************************************************
// 函数功能：  窗口内部时间菜单处理
// 输入参数：  sysHandle:当前句柄
//			   message： 信息内容		
// 返回参数：  响应后的窗口句柄
static uint16 App_Time_Win_Tag_Time(uint16 sysHandle,menuMessage message)
{
	uint16 		   sysHandleTemp;

	sysHandleTemp = sysHandle;
	switch(message.op)
	{
		case RTC_HALF_SEC:

		break;

		case RTC_SEC:
		switch(subWinTimeHandle)
		{
			case SUB_WIN_TIME_NONE:

			break;

			case SUB_WIN_TIME_RTC_TIME:
			break;
			
			case SUB_WIN_TIME_WEATHER:
			
			break;

			case SUB_WIN_TIME_DTAE:

			break;
			default:break;
		}
		break;

		case RTC_MIN:

		break;

		default:break;
	}
	return sysHandleTemp;
}

//**********************************************************************
// 函数功能：  窗口内部提醒菜单处理
// 输入参数：  无	
// 返回参数：  响应后的窗口句柄
static uint16 App_Time_Win_Tag_Remind(uint16 sysHandle,menuMessage message)
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
			sysHandleTemp = App_Remind_Win_Bak(sysHandle,message);//标记即时提醒前的窗口句柄;
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
static uint16 App_Time_Win_Tag_Key(uint16 sysHandle,menuMessage message)
{
	uint16 	sysHandleTemp;

	sysHandleTemp 	= sysHandle;
	winTimeCnt 		= 0;

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
		virtualHandle 	= Virtual_Handle_None;
	}
	else
	{
		switch(subWinTimeHandle)
		{
			case SUB_WIN_TIME_NONE: 	//进入二级界面
			switch(message.val)
			{
				case PRESS_S0:
				sysHandleTemp 	= WIN_SPORT_HANDLE;
				break;

				case PRESS_S1:
				sysHandleTemp 	= WIN_SPORT_HANDLE;
				break;

				case PRESS_S2:
				sysHandleTemp 	= WIN_SPORT_HANDLE;;
				break;

				case HOLD_SHORT_S0:

				break;

				case HOLD_LONG_S0:
				sysHandleTemp = WIN_STORE_HANDLE;
				App_Systerm_Reset();
				break;
			}

			break;

			case SUB_WIN_TIME_RTC_TIME:
			switch(message.val)
			{
				case PRESS_S0:
				sysHandleTemp 	= WIN_SPORT_HANDLE;
				break;

				case PRESS_S1:
				sysHandleTemp 	= WIN_SPORT_HANDLE;
				break;

				case PRESS_S2:
				sysHandleTemp 	= WIN_SPORT_HANDLE;
				break;

				case HOLD_SHORT_S0:

				break;

				case HOLD_LONG_S0:
				sysHandleTemp = WIN_STORE_HANDLE;
				break;
			}
			break;

			case SUB_WIN_TIME_WEATHER:
			switch(message.val)
			{
				case PRESS_S0:
				sysHandleTemp 	= WIN_SPORT_HANDLE;
				break;

				case PRESS_S1:
				sysHandleTemp 	= WIN_SPORT_HANDLE;
				break;

				case PRESS_S2:
				sysHandleTemp 	= WIN_SPORT_HANDLE;
				break;

				case HOLD_SHORT_S0:

				break;

				case HOLD_LONG_S0:
				
				
				sysHandleTemp = WIN_STORE_HANDLE;
				break;
			}
			break;
			
				default:break;
		}
	}
	App_Remind_Win_MissCallClear();//有按键操作，对未接来电进行清空
	return sysHandleTemp;
}

//**********************************************************************
// 函数功能：  窗口内部甩手动作菜单处理[甩手动作菜单为虚拟菜单]
// 输入参数：  sysHandle:当前句柄
//			   message： 信息内容		
// 返回参数：  响应后的窗口句柄
static uint16 App_Time_Win_Tag_Action(uint16 sysHandle,menuMessage message)
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
static uint16 App_Time_Win_Tag_Gesture(uint16 sysHandle,menuMessage message)
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
// 函数功能：  手机APP虚拟菜单处理
// 输入参数：  sysHandle:当前句柄
//			   message： 信息内容	
// 返回参数：  响应后的窗口句柄
static uint16 App_Time_Win_Tag_PhoneApp(uint16 sysHandle,menuMessage message)
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
// 函数功能：  图标闪烁处理
// 输入参数：  sysHandle:当前句柄
//			   message： 信息内容	
// 返回参数：  响应后的窗口句柄
static uint16 App_Time_Win_Tag_IconFlick(uint16 sysHandle,menuMessage message)
{
	uint16 sysHandleTemp;
	static uint8 iconCnt = 0;

	sysHandleTemp = sysHandle;

	switch (subWinTimeHandle)
	{
		case SUB_WIN_TIME_NONE:
		{
			//250ms事件
			if (message.val % 2)
			{
				//变量产生500ms
				iconCnt ++;
				if (iconCnt >= 255)
				{
					iconCnt = 0;
				}
				if (BAT_LEVER_0 == Mid_Bat_LevelRead())
				{
					AppTimeBatFlick(PIC_DISP_ON);
				}	
			}
			else
			{
				if (BAT_LEVER_0 == Mid_Bat_LevelRead())
				{
					AppTimeBatFlick(PIC_DISP_OFF);
				}
			}

			if (bleState == BLE_BROADCAST)
			{
				if (iconCnt % 2)
				{
					AppTimeBleDisp(PIC_DISP_ON);
				}
				else
				{
					AppTimeBleDisp(PIC_DISP_OFF);
				}
			}else if (bleState == BLE_CONNECT)
			{
				AppTimeBleDisp(PIC_DISP_ON);
			}

			if (winTimeCnt >= 1)
			{
				App_RoundDisp(ROUND_NONE, ROUND_HORIZONTAL_PIXEL);	
			}		
		}
		break;

		case SUB_WIN_TIME_RTC_TIME:	
		break;

		default:
		break;
	}

	return sysHandleTemp;
}

//**********************************************************************
// 函数功能：  响应窗口切换或窗口刷新
// 输入参数：  sysHandle:当前句柄
//			   message： 信息内容			
// 返回参数：  响应后的窗口句柄
static uint16 App_Time_Win_Tag_WinChange(uint16 sysHandle,menuMessage message)
{
	uint16 sysHandleTemp;
	rtc_time_s     timeTemp;
	uint8 		 	batLevel;
	weahter_s  		weatherinfo;

	winTimeCnt ++;

	sysHandleTemp = sysHandle;


	switch(subWinTimeHandle)
	{
		case SUB_WIN_TIME_NONE:	//在主界面，切换到子窗口
		if (cycleFlag)
		{
			if(winTimeCnt >= WIN_TIME_5SEC)
			{
				winTimeCnt 		 = 0;
				cycleFlag 		 = 0;
				sysHandleTemp 	 = WIN_IDLE_HANDLE;		//休眠
				App_Remind_Win_MissCallClear();			//未接电话已查看
			}
		}
		else
		{
			if (winTimeCnt >= WIN_TIME_2SEC)
			{
				winTimeCnt 		= 0;				
				App_PicRamClear();
				Mid_Rtc_TimeRead(&timeTemp);
				AppTimeTimeDisp(timeTemp, systermConfig.systermTimeType);
				App_RoundDisp(ROUND_LEFT,ROUND_HORIZONTAL_PIXEL);
				subWinTimeHandle   = SUB_WIN_TIME_RTC_TIME;
			}
		}
		
		break;

		case SUB_WIN_TIME_RTC_TIME:
		if (winTimeCnt >= WIN_TIME_2SEC)
		{
			winTimeCnt 		 = 0;
			subWinTimeHandle = SUB_WIN_TIME_WEATHER;
			App_PicRamClear();
			Mid_WeatherScene_TendencyGet(&weatherinfo);
			AppTimeWeatherDisp(weatherinfo.weahterStatus,weatherinfo.weahterCurTemperature);
			App_RoundDisp(ROUND_LEFT,ROUND_HORIZONTAL_PIXEL);	
		}
		else
		{
			App_PicRamClear();
			Mid_Rtc_TimeRead(&timeTemp);
			AppTimeTimeDisp(timeTemp, systermConfig.systermTimeType);	
			App_RoundDisp(ROUND_NONE,ROUND_HORIZONTAL_PIXEL);
		}
		break;

		case SUB_WIN_TIME_WEATHER:
		if (winTimeCnt >= WIN_TIME_2SEC)
		{
			App_PicRamClear();
			Mid_Rtc_TimeRead(&timeTemp);

			AppTimeDateDisp(timeTemp);
			AppTimeWeekDisp(timeTemp);
			if (App_Remind_Win_MissCallRead())
			{
				AppTimeMisscallDisp(PIC_DISP_ON);
			}
			if (bleState == BLE_BROADCAST || bleState == BLE_CONNECT || bleState == BLE_POWERON)
			{
				AppTimeBleDisp(PIC_DISP_ON);
			}
			batLevel 	= Mid_Bat_LevelRead();
			AppTimeBatDisp(batLevel);	
			App_RoundDisp(ROUND_LEFT,ROUND_HORIZONTAL_PIXEL);

			winTimeCnt 		 = 0;
			cycleFlag 		 = 1;
			subWinTimeHandle = SUB_WIN_TIME_NONE;
		}		
		break;
		
		default:break;
	}

	return sysHandleTemp;
}
