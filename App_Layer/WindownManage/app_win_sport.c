#include "platform_common.h"
#include "app_variable.h"
#include "app_picture_source.h"
#include "app_display.h"
#include "app_protocal.h"
#include "app_win_common.h"
#include "app_win_remind.h"
#include "app_win_sport.h"
#include "app_win_store.h"
#include "app_systerm.h"

/*************** func declaration ********************************/
//罗列窗口内所有菜单的处理函数
static uint16 App_Sport_Win_Tag_Sport(uint16 sysHandle,menuMessage message);
static uint16 App_Sport_Win_Tag_Remind(uint16 sysHandle,menuMessage message);
static uint16 App_Sport_Win_Tag_Key(uint16 sysHandle,menuMessage message);
static uint16 App_Sport_Win_Tag_Action(uint16 sysHandle,menuMessage message);
static uint16 App_Sport_Win_Tag_Gesture(uint16 sysHandle,menuMessage message);
static uint16 App_Sport_Win_Tag_PhoneApp(uint16 sysHandle,menuMessage message);
static uint16 App_Sport_Win_Tag_WinChange(uint16 sysHandle,menuMessage message);



/*************** macro define ********************************/
#define 		SPORT_WIN_TAG_MAX 		10 			//窗口内最大菜单个数

//窗口子窗口定义
typedef enum
{
	SUB_WIN_SPORT_NONE 		 = 0,
	SUB_WIN_SPORT_STEP_COUNT,
	SUB_WIN_SPORT_KCAL,
	SUB_WIN_SPORT_TIME,
	SUB_WIN_SPORT_DISTANCE,
}SportSubWinHandle;


/************** variable define *****************************/
//罗列窗口内所有菜单（实体菜单、虚拟菜单）
static 	WinMenu	SportWindowMenu[SPORT_WIN_TAG_MAX] = 
				{
					{TAG_SPORT,			0,0,	App_Sport_Win_Tag_Sport		},
					{TAG_REMIND,		0,0,	App_Sport_Win_Tag_Remind	},
					{TAG_KEY,			0,0,	App_Sport_Win_Tag_Key		},
					{TAG_ACTION,		0,0,	App_Sport_Win_Tag_Action	},
					{TAG_GESTURE,		0,0,	App_Sport_Win_Tag_Gesture	},
					{TAG_PHONE_APP,		0,0,	App_Sport_Win_Tag_PhoneApp	},
					{TAG_WIN_CHANGE,	0,0,	App_Sport_Win_Tag_WinChange	},
				};


static SportSubWinHandle 	subWinSportHandle;	//用于窗口下子窗口的标记，

/************** function define ******`***********************/

//**********************************************************************
// 函数功能：  窗口内部菜单初始化
// 输入参数：  无	

// 返回参数：  0x00:初始化成功，0xff：初始化失败
uint16 App_Sport_Win_Init(void)
{
	winTimeCnt = 0;
	cycleFlag  = 0;
	App_PicRamClear();
	AppStepMainInterfaceDisp(systermConfig.systermLanguge);
	App_RoundDisp(ROUND_UP,ROUND_VERTICAL_PIXEL);

	subWinSportHandle 	= SUB_WIN_SPORT_NONE;
	return WIN_SPORT_HANDLE;
}

//**********************************************************************
// 函数功能：  窗口处理回调函数，每一窗口分配一个回调函数，在回调函数中执行具体菜单功能
// 输入参数：   sysHandle ：  窗口的句柄号
//				tagMenu：     当前响应的菜单
// 				message： 	  菜单发送的信息：动作、数值、状态等		

// 返回参数：  返回菜单响应的窗口句柄号
uint16 App_Sport_Win_CallBack(uint16 sysHandle,uint16 tagMenu,menuMessage message)
{
	uint8 	i;
	uint16 	handletemp;

	if (sysHandle != WIN_SPORT_HANDLE)
	{
		return sysHandle;
	}

	for (i = 0; i < SPORT_WIN_TAG_MAX; i++)
	{
		if (SportWindowMenu[i].tag == tagMenu)
		{
			break;
		}
	}
	handletemp 		= sysHandle;

	if (i < SPORT_WIN_TAG_MAX && SportWindowMenu[i].callback != NULL)
	{		
		handletemp = SportWindowMenu[i].callback(handletemp,message);
	}

	return handletemp;
}

//**********************************************************************
// 函数功能：  窗口内部菜单处理
// 输入参数：  sysHandle:当前句柄
//			   message： 信息内容		
// 返回参数：  响应后的窗口句柄
static uint16 App_Sport_Win_Tag_Sport(uint16 sysHandle,menuMessage message)
{
	stepSportInfo_s stepsportinfo;
	
	if (subWinSportHandle == SUB_WIN_SPORT_STEP_COUNT)
	{
		Mid_SportScene_SportInfoRead(&stepsportinfo);
		AppStepCompleteDisp(stepsportinfo.totalStep, stepsportinfo.stepComplete);
	}
	return WIN_SPORT_HANDLE;
}

//**********************************************************************
// 函数功能：  窗口内部菜单处理
// 输入参数：  无	
// 返回参数：  响应后的窗口句柄
static uint16 App_Sport_Win_Tag_Remind(uint16 sysHandle,menuMessage message)
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
static uint16 App_Sport_Win_Tag_Key(uint16 sysHandle,menuMessage message)
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
		switch(subWinSportHandle)
		{
			case SUB_WIN_SPORT_NONE:
			case SUB_WIN_SPORT_TIME:
			case SUB_WIN_SPORT_STEP_COUNT:
			case SUB_WIN_SPORT_KCAL:
			case SUB_WIN_SPORT_DISTANCE:
			switch(message.val)
			{
				case PRESS_S0:
				sysHandleTemp 	= WIN_HEART_HANDLE;
				break;

				case PRESS_S1:
				sysHandleTemp 	= WIN_HEART_HANDLE;
				
				break;

				case PRESS_S2:
				sysHandleTemp 	= WIN_HEART_HANDLE;
				
				break;

				case HOLD_SHORT_S0:

				break;

				case HOLD_LONG_S0:
				sysHandleTemp = WIN_STORE_HANDLE;
				App_Systerm_Reset();
				break;
			}
			break;

			default:break;
		}
	}
	return sysHandleTemp;
}

//**********************************************************************
// 函数功能：  窗口内部甩手动作菜单处理[甩手动作菜单为虚拟菜单]
// 输入参数：  sysHandle:当前句柄
//			   message： 信息内容		
// 返回参数：  响应后的窗口句柄
static uint16 App_Sport_Win_Tag_Action(uint16 sysHandle,menuMessage message)
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
static uint16 App_Sport_Win_Tag_Gesture(uint16 sysHandle,menuMessage message)
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
static uint16 App_Sport_Win_Tag_PhoneApp(uint16 sysHandle,menuMessage message)
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
static uint16 App_Sport_Win_Tag_WinChange(uint16 sysHandle,menuMessage message)
{
	uint16 sysHandleTemp;
	stepSportInfo_s infoTemp;

	winTimeCnt ++;
	sysHandleTemp = sysHandle;

	switch(subWinSportHandle)
	{
		case SUB_WIN_SPORT_NONE:
		if (winTimeCnt >= WIN_TIME_2SEC)
		{
			winTimeCnt 		  = 0;
			subWinSportHandle = SUB_WIN_SPORT_STEP_COUNT;
			App_PicRamClear();
			Mid_SportScene_SportInfoRead(&infoTemp);
			AppStepCompleteDisp(infoTemp.totalStep, infoTemp.stepComplete);
			App_RoundDisp(ROUND_LEFT,ROUND_HORIZONTAL_PIXEL);
		}
		break;

		case SUB_WIN_SPORT_STEP_COUNT:
		if (cycleFlag)
		{
			if(winTimeCnt >= WIN_TIME_5SEC)
			{
				winTimeCnt 		 = 0;
				cycleFlag 		 = 0;
				sysHandleTemp 	 = WIN_IDLE_HANDLE;		//休眠
			}
		}
		else
		{
			if (winTimeCnt >= WIN_TIME_2SEC)
			{
				winTimeCnt 		  = 0;
				subWinSportHandle = SUB_WIN_SPORT_KCAL;
				App_PicRamClear();
				Mid_SportScene_SportInfoRead(&infoTemp);
				AppStepKcalDisp(infoTemp.heatQuantity);
				App_RoundDisp(ROUND_LEFT,ROUND_HORIZONTAL_PIXEL);
			}
		}

		break;

		case SUB_WIN_SPORT_KCAL:
		if (winTimeCnt >= WIN_TIME_2SEC)
		{
			winTimeCnt 		  = 0;
			subWinSportHandle = SUB_WIN_SPORT_TIME;
			App_PicRamClear();
			Mid_SportScene_SportInfoRead(&infoTemp);
			AppStepDurationDisp(infoTemp.sportDuaration);
			App_RoundDisp(ROUND_LEFT,ROUND_HORIZONTAL_PIXEL);
		}
		break;

		case SUB_WIN_SPORT_TIME:
		if (winTimeCnt >= WIN_TIME_2SEC)
		{
			winTimeCnt 		  = 0;
			subWinSportHandle = SUB_WIN_SPORT_DISTANCE;
			App_PicRamClear();
			Mid_SportScene_SportInfoRead(&infoTemp);
			AppStepDistanceDisp(infoTemp.sportDistance);
			App_RoundDisp(ROUND_LEFT,ROUND_HORIZONTAL_PIXEL);
		}
		break;

		case SUB_WIN_SPORT_DISTANCE:
		if (winTimeCnt >= WIN_TIME_2SEC)
		{
			winTimeCnt 		  = 0;
			cycleFlag 		  = 1;
			subWinSportHandle = SUB_WIN_SPORT_STEP_COUNT;
			App_PicRamClear();
			Mid_SportScene_SportInfoRead(&infoTemp);
			AppStepCompleteDisp(infoTemp.totalStep, infoTemp.stepComplete);
			App_RoundDisp(ROUND_LEFT,ROUND_HORIZONTAL_PIXEL);
		}
		break;

		default:
		break;
	}
	
	return sysHandleTemp;
}


