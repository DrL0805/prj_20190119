#include "platform_common.h"

#include "app_display.h"
#include "app_protocal.h"
#include "app_win_common.h"
#include "app_win_remind.h"
#include "app_win_heart.h"
#include "app_win_store.h"
#include "app_systerm.h"


/*************** func declaration ********************************/
//罗列窗口内所有菜单的处理函数
static uint16 App_Heart_Win_Tag_Heart(uint16 sysHandle,menuMessage message);
static uint16 App_Heart_Win_Tag_Remind(uint16 sysHandle,menuMessage message);
static uint16 App_Heart_Win_Tag_Key(uint16 sysHandle,menuMessage message);
static uint16 App_Heart_Win_Tag_Action(uint16 sysHandle,menuMessage message);
static uint16 App_Heart_Win_Tag_Gesture(uint16 sysHandle,menuMessage message);
static uint16 App_Heart_Win_Tag_PhoneApp(uint16 sysHandle,menuMessage message);
static uint16 App_Heart_Win_Tag_IconFlick(uint16 sysHandle,menuMessage message);
static uint16 App_Heart_Win_Tag_WinChange(uint16 sysHandle,menuMessage message);


/*************** macro define ********************************/
#define 		HEART_WIN_TAG_MAX 		10 			//窗口内最大菜单个数

//窗口子窗口定义
typedef enum
{
	SUB_WIN_HEART_NONE 		= 0,
	SUB_WIN_HEART_LATEST,
	SUB_WIN_HEART_CHECKING,
	SUB_WIN_HEART_DISP,
	SUB_WIN_HEART_RETREY,
}HeartSubWinHandle;


/************** variable define *****************************/
//罗列窗口内所有菜单（实体菜单、虚拟菜单）
static 	WinMenu	HeartWindowMenu[HEART_WIN_TAG_MAX] = 
				{
					{TAG_HEART,			0,0,	App_Heart_Win_Tag_Heart		},
					{TAG_REMIND,		0,0,	App_Heart_Win_Tag_Remind	},
					{TAG_KEY,			0,0,	App_Heart_Win_Tag_Key		},
					{TAG_ACTION,		0,0,	App_Heart_Win_Tag_Action	},
					{TAG_GESTURE,		0,0,	App_Heart_Win_Tag_Gesture	},
					{TAG_PHONE_APP,		0,0,	App_Heart_Win_Tag_PhoneApp	},
					{TAG_ICON_FLICK, 	0,0,	App_Heart_Win_Tag_IconFlick	},
					{TAG_WIN_CHANGE,	0,0,	App_Heart_Win_Tag_WinChange	},
						
				};
	
static HeartSubWinHandle 	subWinHeartHandle;	//用于窗口下子窗口的标记，
static uint8 winHR = 0xff;
static uint8 winRHR = 0xff;
static uint8 hrModuleState = 0;

/************** function define *****************************/

//**********************************************************************
// 函数功能：  窗口内部菜单初始化
// 输入参数：  无	

// 返回参数：  0x00:初始化成功，0xff：初始化失败
uint16 App_Heart_Win_Init(void)
{
	winTimeCnt  = 0;
	cycleFlag  = 0;

	App_PicRamClear();
	AppHeartMainInterfaceDisp(systermConfig.systermLanguge);
	App_RoundDisp(ROUND_UP,ROUND_VERTICAL_PIXEL);

	subWinHeartHandle 	= SUB_WIN_HEART_NONE;

	return WIN_HEART_HANDLE;
}

//**********************************************************************
// 函数功能：  窗口处理回调函数，每一窗口分配一个回调函数，在回调函数中执行具体菜单功能
// 输入参数：   sysHandle ：  窗口的句柄号
//				tagMenu：     当前响应的菜单
// 				message： 	  菜单发送的信息：动作、数值、状态等		

// 返回参数：  返回菜单响应的窗口句柄号
uint16 App_Heart_Win_CallBack(uint16 sysHandle,uint16 tagMenu,menuMessage message)
{
	uint8 	i;
	uint16 	handletemp;

	if (sysHandle != WIN_HEART_HANDLE)
	{
		return sysHandle;
	}

	for (i = 0; i < HEART_WIN_TAG_MAX; i++)
	{
		if (HeartWindowMenu[i].tag == tagMenu)
		{
			break;
		}
	}
	handletemp 		= sysHandle;

	if (i < HEART_WIN_TAG_MAX && HeartWindowMenu[i].callback != NULL)
	{		
		handletemp = HeartWindowMenu[i].callback(handletemp,message);
	}

	return handletemp;
}


//**********************************************************************
// 函数功能：  窗口内部菜单处理
// 输入参数：  sysHandle:当前句柄
//			   message： 信息内容		
// 返回参数：  响应后的窗口句柄
static uint16 App_Heart_Win_Tag_Heart(uint16 sysHandle,menuMessage message)
{
	uint16 	sysHandleTemp;
	multimodule_task_msg_t msg;

	sysHandleTemp = sysHandle;
	if (subWinHeartHandle == SUB_WIN_HEART_CHECKING || subWinHeartHandle == SUB_WIN_HEART_RETREY)
	{
		if (message.val != 0)
		{
			subWinHeartHandle = SUB_WIN_HEART_DISP;
		}
		winTimeCnt        = 0;
	}
	if (hrModuleState == 0)
	{
		hrModuleState = 1;
		//启动心率测量,心率与后台心率监测重叠时需特别处理
		 msg.id                  = HEARTRATE_ID;
	    msg.module.hrmEvent.id   = HRM_START;
	    MultiModuleTask_EventSet(msg);
	}
	return sysHandleTemp;
}

//**********************************************************************
// 函数功能：  窗口内部菜单处理
// 输入参数：  无	
// 返回参数：  响应后的窗口句柄
static uint16 App_Heart_Win_Tag_Remind(uint16 sysHandle,menuMessage message)
{
	uint16 	sysHandleTemp;
	multimodule_task_msg_t msg;

	sysHandleTemp = sysHandle;

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

	//提醒产生，退出心率测量
	if (sysHandleTemp != WIN_HEART_HANDLE)
	{
		if (hrModuleState)
		{
			//关闭心率测量
			msg.id                  = HEARTRATE_ID;
	        msg.module.hrmEvent.id  = HRM_STOP;
	        MultiModuleTask_EventSet(msg);
	        hrModuleState = 0;
		}
		subWinHeartHandle 	= SUB_WIN_HEART_NONE;
	}
	
	return sysHandleTemp;
}

//**********************************************************************
// 函数功能：  窗口内部按键菜单处理
// 输入参数：  sysHandle:当前句柄
//			   message： 信息内容		
// 返回参数：  响应后的窗口句柄
static uint16 App_Heart_Win_Tag_Key(uint16 sysHandle,menuMessage message)
{
	uint16 	sysHandleTemp;
	multimodule_task_msg_t msg;

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
	}
	else
	{
		switch(subWinHeartHandle)
		{
			case SUB_WIN_HEART_NONE:
			case SUB_WIN_HEART_RETREY:
			case SUB_WIN_HEART_DISP:
			case SUB_WIN_HEART_LATEST:
			case SUB_WIN_HEART_CHECKING:
			switch(message.val)
			{
				case PRESS_S0:
				case PRESS_S1:
				case PRESS_S2:
				sysHandleTemp 	= WIN_SLEEP_HANDLE;
				if (hrModuleState)
				{
					//关闭心率测量
					 msg.id                  = HEARTRATE_ID;
		            msg.module.hrmEvent.id   = HRM_STOP;
		            MultiModuleTask_EventSet(msg);
		            hrModuleState = 0;
				}
				break;

				case HOLD_SHORT_S0:

				break;

				case HOLD_LONG_S0:
				if (hrModuleState)
				{
					//关闭心率测量
					 msg.id                  = HEARTRATE_ID;
		            msg.module.hrmEvent.id   = HRM_STOP;
		            MultiModuleTask_EventSet(msg);
		            hrModuleState = 0;
				}
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
static uint16 App_Heart_Win_Tag_Action(uint16 sysHandle,menuMessage message)
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
static uint16 App_Heart_Win_Tag_Gesture(uint16 sysHandle,menuMessage message)
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
static uint16 App_Heart_Win_Tag_PhoneApp(uint16 sysHandle,menuMessage message)
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
static uint16 App_Heart_Win_Tag_IconFlick(uint16 sysHandle,menuMessage message)
{
	uint16 sysHandleTemp;
	uint8  u8temp;

	sysHandleTemp = sysHandle;

	if (subWinHeartHandle == SUB_WIN_HEART_CHECKING)
	{	
		App_PicRamClear();	
		if (message.val)
		{				
			AppHeartCheckFlick(BINARY_TYPE_ON,winRHR);		
		}
		else
		{
			AppHeartCheckFlick(BINARY_TYPE_OFF,winRHR);
		}
		App_RoundDisp(ROUND_NONE, ROUND_HORIZONTAL_PIXEL);	
	}
	else if (subWinHeartHandle == SUB_WIN_HEART_DISP)
	{
		Mid_Hrm_Read(&u8temp);
		if (u8temp != 0)
		{
			winHR = u8temp;
		}
		winRHR = Mid_HeartRateScene_RestingHR_Read();
		App_PicRamClear();				
		AppHeartDataDisp(BINARY_TYPE_ON, winRHR, winHR);
		App_RoundDisp(ROUND_NONE, ROUND_HORIZONTAL_PIXEL);
	}	

	return sysHandleTemp;
}


//**********************************************************************
// 函数功能：  响应窗口切换或窗口刷新
// 输入参数：  sysHandle:当前句柄
//			   message： 信息内容			
// 返回参数：  响应后的窗口句柄
static uint16 App_Heart_Win_Tag_WinChange(uint16 sysHandle,menuMessage message)
{
	uint16 sysHandleTemp;
	uint8  u8temp;
	multimodule_task_msg_t msg;

	winTimeCnt ++;
	sysHandleTemp = sysHandle;
	switch(subWinHeartHandle)
	{
		case SUB_WIN_HEART_NONE:
		if (winTimeCnt >= WIN_TIME_2SEC)
		{
			winTimeCnt        = 0;
			subWinHeartHandle = SUB_WIN_HEART_CHECKING;

			if (hrModuleState == 0)
			{
				hrModuleState = 1;
				//启动心率测量
				 msg.id                  = HEARTRATE_ID;
	            msg.module.hrmEvent.id   = HRM_START;
	            MultiModuleTask_EventSet(msg);
			}
			//重新获取值
			winHR = 0xff;
			winRHR = Mid_HeartRateScene_RestingHR_Read();
			App_PicRamClear();
			AppHeartDataDisp(BINARY_TYPE_ON, winRHR, winHR);
			App_RoundDisp(ROUND_LEFT,ROUND_HORIZONTAL_PIXEL);
		}
		break;

		case SUB_WIN_HEART_LATEST:
		if (winTimeCnt >= WIN_TIME_1SEC)
		{
			winTimeCnt        = 0;
			App_PicRamClear();
			AppHeartDataDisp(BINARY_TYPE_ON, winRHR, winHR);
			App_RoundDisp(ROUND_NONE,ROUND_HORIZONTAL_PIXEL);

			subWinHeartHandle = SUB_WIN_HEART_CHECKING;
		}
		break;

		case SUB_WIN_HEART_CHECKING:
		if (winTimeCnt >= WIN_TIME_10SEC)
		{
			/*
			Mid_Hrm_TouchStatusRead(&u8temp);	//检测到未触摸或佩戴
			if (u8temp == APP_HRM_OFF_TOUCH)
			{
				*/
				winTimeCnt        = 0;
				subWinHeartHandle = SUB_WIN_HEART_RETREY;
				App_PicRamClear();
				AppHeartNoWearDisp(systermConfig.systermLanguge);
				App_RoundDisp(ROUND_NONE,ROUND_HORIZONTAL_PIXEL);
			/*}

			
			//测量超时
			if (winTimeCnt >= (WIN_TIME_10SEC + WIN_TIME_5SEC))	
			{
				winTimeCnt = 0;
				subWinHeartHandle = SUB_WIN_HEART_NONE;
				if (hrModuleState)
				{
					//关闭心率测量
					 msg.id                  = HEARTRATE_ID;
		            msg.module.hrmEvent.id   = HRM_STOP;
		            MultiModuleTask_EventSet(msg);
		            hrModuleState = 0;
				}
				App_PicRamClear();
				sysHandleTemp = WIN_IDLE_HANDLE;
			}*/	
		}
		else
		{
			Mid_Hrm_TouchStatusRead(&u8temp);	//检测到未触摸或佩戴
			if (u8temp == APP_HRM_OFF_TOUCH)
			{
				
				winTimeCnt        = 0;
				subWinHeartHandle = SUB_WIN_HEART_RETREY;
				App_PicRamClear();
				AppHeartNoWearDisp(systermConfig.systermLanguge);
				App_RoundDisp(ROUND_NONE,ROUND_HORIZONTAL_PIXEL);
			}
		}
		break;

		case SUB_WIN_HEART_RETREY:
		if (winTimeCnt >= WIN_TIME_10SEC)	//10秒仍检测到未佩戴好
		{
			winTimeCnt 		  = 0;
			sysHandleTemp 	  = WIN_IDLE_HANDLE;
			subWinHeartHandle = SUB_WIN_HEART_NONE;

			//关闭心率测量
			 msg.id                  = HEARTRATE_ID;
            msg.module.hrmEvent.id   = HRM_STOP;
            MultiModuleTask_EventSet(msg);
            hrModuleState = 0;
		}
		break;

		case SUB_WIN_HEART_DISP:
		if (winTimeCnt >= (WIN_TIME_5SEC + WIN_TIME_1SEC))
		{
			winTimeCnt 		  = 0;
			sysHandleTemp 	  = WIN_IDLE_HANDLE;
			subWinHeartHandle = SUB_WIN_HEART_NONE;

			//关闭心率测量
			 msg.id                  = HEARTRATE_ID;
            msg.module.hrmEvent.id   = HRM_STOP;
            MultiModuleTask_EventSet(msg);
            hrModuleState = 0;
		}
		break;

		default:break;
	}

	return sysHandleTemp;
}

