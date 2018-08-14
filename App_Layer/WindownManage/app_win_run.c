#include "platform_common.h"

#include "app_variable.h"
#include "app_display.h"
#include "app_win_common.h"
#include "app_win_remind.h"
#include "app_win_run.h"
#include "app_win_store.h"
#include "app_systerm.h"
#include "app_task.h"

/*************** func declaration ********************************/
//罗列窗口内所有菜单的处理函数
static uint16 App_Run_Win_Tag_Time(uint16 sysHandle,menuMessage message);
static uint16 App_Run_Win_Tag_Remind(uint16 sysHandle,menuMessage message);
static uint16 App_Run_Win_Tag_Key(uint16 sysHandle,menuMessage message);
static uint16 App_Run_Win_Tag_Action(uint16 sysHandle,menuMessage message);
static uint16 App_Run_Win_Tag_Gesture(uint16 sysHandle,menuMessage message);
static uint16 App_Run_Win_Tag_PhoneApp(uint16 sysHandle,menuMessage message);
static uint16 App_Run_Win_Tag_IconFlick(uint16 sysHandle,menuMessage message);
static uint16 App_Run_Win_Tag_WinChange(uint16 sysHandle,menuMessage message);


/*************** macro define ********************************/
#define 		RUN_WIN_TAG_MAX 		8 			//窗口内最大菜单个数


//提醒窗口子窗口定义
typedef enum
{
	SUB_WIN_RUN_NONE 		= 0x0000,
	SUB_WIN_RUN_COUNT_3 	= 0x0001,
	SUB_WIN_RUN_COUNT_2 	= 0x0002,
	SUB_WIN_RUN_COUNT_1 	= 0x0004,
	SUB_WIN_RUN_STEP		= 0x0010,
	SUB_WIN_RUN_SPEED		= 0x0020,
	SUB_WIN_RUN_CAL 		= 0x0040,
	SUB_WIN_RUN_TIME 		= 0x0080,
	SUB_WIN_RUN_TEMP 		= 0x0100,
	SUB_WIN_RUN_SAVEING 	= 0x0200,
	SUB_WIN_RUN_SAVED 		= 0x0400,
	SUB_WIN_RUN_SAVE_FULL 	= 0x0800, 
}RunSubWinHandle;

/************** variable define *****************************/
//罗列即时提醒窗口内所有菜单（实体菜单、虚拟菜单）
static 	WinMenu	RunWindowMenu[RUN_WIN_TAG_MAX] = 
				{
					{TAG_TIME,			0,0,	App_Run_Win_Tag_Time	},
					{TAG_REMIND,		0,0,	App_Run_Win_Tag_Remind	},
					{TAG_KEY,			0,0,	App_Run_Win_Tag_Key		},
					{TAG_ACTION,		0,0,	App_Run_Win_Tag_Action	},
					{TAG_GESTURE,		0,0,	App_Run_Win_Tag_Gesture	},
					{TAG_PHONE_APP,		0,0,	App_Run_Win_Tag_PhoneApp},
					{TAG_ICON_FLICK, 	0,0,	App_Run_Win_Tag_IconFlick},
					{TAG_WIN_CHANGE,	0,0,	App_Run_Win_Tag_WinChange},
				};
	
static uint16 	curSubWinRunHandle 	= SUB_WIN_RUN_NONE; 	
static uint8 	runWinIlde = 0;
static uint16 	runDuarationTime;
static uint8 	runSceneValid;

/************** function define ******`***********************/

//**********************************************************************
// 函数功能：  窗口内部菜单初始化
// 输入参数：  无	

// 返回参数：  0x00:初始化成功，0xff：初始化失败
uint16 App_Run_Win_Init(void)
{
	uint16 	sysHandleTemp;
	scene_detail_s  sceneDetailTemp;
	scene_data_s    sceneDataTemp;

	App_PicRamClear();
	if (curSubWinRunHandle == SUB_WIN_RUN_NONE)
	{
		runWinIlde 			= 0;
		runDuarationTime 	= 0;
		runSceneValid 		= 0;
		saveState 			= SAVE_NULL;
		curSubWinRunHandle = SUB_WIN_RUN_NONE;
		AppRunMainInterfaceDisp(systermConfig.systermLanguge);
		App_RoundDisp(ROUND_UP,ROUND_VERTICAL_PIXEL);	
	}
	else
	{
		Mid_Scene_DetailRead(RUN_SCENE_TYPE, &sceneDetailTemp);
		Mid_Scene_LogDataRead(RUN_SCENE_TYPE,&sceneDataTemp);
		AppRunStepNumFrequencyDisp(sceneDetailTemp.runDetail.stepTotal, sceneDataTemp.runScene.freqLog);
		App_RoundDisp(ROUND_NONE,ROUND_HORIZONTAL_PIXEL);
		curSubWinRunHandle = SUB_WIN_RUN_STEP;
	}
	runWinIlde = 0;
	cycleFlag  			= 0;
	winTimeCnt 			= 0;
	sysHandleTemp 		= WIN_RUN_HANDLE;

	return sysHandleTemp;
}

//**********************************************************************
// 函数功能：  窗口处理回调函数，每一窗口分配一个回调函数，在回调函数中执行具体菜单功能
// 输入参数：   sysHandle ：  窗口的句柄号
//				tagMenu：     当前响应的菜单
// 				message： 	  菜单发送的信息：动作、数值、状态等		

// 返回参数：  返回菜单响应的窗口句柄号
uint16 App_Run_Win_CallBack(uint16 sysHandle,uint16 tagMenu,menuMessage message)
{
	uint8 	i;
	uint16 	handletemp;

	if (sysHandle != WIN_RUN_HANDLE)
	{
		return sysHandle;
	}

	for (i = 0; i < RUN_WIN_TAG_MAX; i++)
	{
		if (RunWindowMenu[i].tag == tagMenu)
		{
			break;
		}
	}
	handletemp 		= sysHandle;

	if (i < RUN_WIN_TAG_MAX && RunWindowMenu[i].callback != NULL)
	{		
		handletemp = RunWindowMenu[i].callback(handletemp,message);
	}

	return handletemp;
}

//**********************************************************************
// 函数功能：  窗口内部时间菜单处理
// 输入参数：  sysHandle:当前句柄
//			   message： 信息内容		
// 返回参数：  响应后的窗口句柄
static uint16 App_Run_Win_Tag_Time(uint16 sysHandle,menuMessage message)
{
	uint16 		   sysHandleTemp;

	sysHandleTemp = sysHandle;
	switch(message.op)
	{
		case RTC_SEC:
		switch(curSubWinRunHandle)
		{
			case SUB_WIN_RUN_COUNT_1:
			case SUB_WIN_RUN_CAL:
			case SUB_WIN_RUN_STEP:
			case SUB_WIN_RUN_SPEED:
			case SUB_WIN_RUN_TIME:
			case SUB_WIN_RUN_TEMP:
			runDuarationTime += 1;
			break;

			default:
			break;
		}
		break;

		default:break;
	}
	return sysHandleTemp;
}

//**********************************************************************
// 函数功能：  窗口内部菜单处理
// 输入参数：  无	
// 返回参数：  响应后的窗口句柄
static uint16 App_Run_Win_Tag_Remind(uint16 sysHandle,menuMessage message)
{
	uint16 	sysHandleTemp;
	multimodule_task_msg_t  msg;

	if (message.op == BAT_REMIND || message.op == OTA_REMIND)
	{
		if (message.state == BAT_CHARGING_STATE || message.state == BAT_LOW_VOLTAGE_STATE ||message.state == OTA_ENTER)
		{
			sysHandleTemp = App_Store_Win_Bak(sysHandle,message);

			//进入仓储前退出跑步场景
			msg.id                       = SCENE_ID;
		    msg.module.sceneEvent.id     = RUN_SCENE_CLOSE;
		    MultiModuleTask_EventSet(msg);

		    sceneSwitch = SCENE_NULL;
			App_PicRamClear();
			App_RoundDisp(ROUND_NONE,ROUND_HORIZONTAL_PIXEL);
			winTimeCnt 		 = 0;
			cycleFlag  		 = 0;
			runSceneValid 	 = 0;
			curSubWinRunHandle = SUB_WIN_RUN_NONE;
		}
		else
		{
			sysHandleTemp = App_Remind_Win_Bak(sysHandle,message);//标记即时提醒前的窗口句柄
		}	
	} else if (message.op == SAVE_REMIND)
	{
		if (message.state == SAVE_SUCCESS_STATE)
		{
			saveState  	= SAVE_OK;
		}else if (message.state == SAVE_SPACE_FULL_STATE)
		{
			winTimeCnt = 0;
			App_PicRamClear();
			AppRemindPlzUploadData(systermConfig.systermLanguge);
			App_RoundDisp(ROUND_NONE,ROUND_HORIZONTAL_PIXEL);
			curSubWinRunHandle = SUB_WIN_RUN_SAVE_FULL;
			MultiModuleTask_EventSet(SAVE_FULL_MOTO);	
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
static uint16 App_Run_Win_Tag_Key(uint16 sysHandle,menuMessage message)
{
	uint16 			sysHandleTemp;
	scene_detail_s  sceneDetailTemp;
	scene_data_s    sceneDataTemp;
	app_task_msg_t  appMsg;
	multimodule_task_msg_t  msg;

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
				//App_Protocal_TakePhoto();
				break;
			}

			break;

			case Virtual_Handle_Author:
			switch(message.val)
			{
				case PRESS_S0:
				case PRESS_S1:
				case PRESS_S2:
				//App_Protocal_AuthorPass();
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
			if (runWinIlde)
			{
				cycleFlag  = 0;
				App_PicRamClear();
				Mid_Scene_DetailRead(RUN_SCENE_TYPE, &sceneDetailTemp);
				Mid_Scene_LogDataRead(RUN_SCENE_TYPE,&sceneDataTemp);
				AppRunStepNumFrequencyDisp(sceneDetailTemp.runDetail.stepTotal, sceneDataTemp.runScene.freqLog);
				App_RoundDisp(ROUND_NONE,ROUND_HORIZONTAL_PIXEL);
				curSubWinRunHandle = SUB_WIN_RUN_STEP;
			}
			else
			{
				if (curSubWinRunHandle == SUB_WIN_RUN_NONE || curSubWinRunHandle == SUB_WIN_RUN_COUNT_3 ||
					curSubWinRunHandle == SUB_WIN_RUN_COUNT_2 || curSubWinRunHandle == SUB_WIN_RUN_COUNT_1)
				{
					sysHandleTemp 		= WIN_SETTING_HANDLE;
					curSubWinRunHandle  = SUB_WIN_RUN_NONE;
				}else if (curSubWinRunHandle == SUB_WIN_RUN_SAVE_FULL)
				{
					if (runSceneValid)
					{
						//先关闭场景相关功能模块
						msg.id                       = SCENE_ID;
					    msg.module.sceneEvent.id     = RUN_SCENE_CLOSE;
					    MultiModuleTask_EventSet(msg);
					    sceneSwitch = SCENE_NULL;
					}
					App_PicRamClear();
					App_RoundDisp(ROUND_NONE,ROUND_HORIZONTAL_PIXEL);
					winTimeCnt = 0;
					cycleFlag  = 0;
					runSceneValid 	 = 0;
					sysHandleTemp 	 = WIN_IDLE_HANDLE;		//休眠
					curSubWinRunHandle = SUB_WIN_RUN_NONE;
				}
			}
			runWinIlde = 0;
			break;

			case PRESS_S1:	
			sysHandleTemp = WIN_CLIMB_HANDLE;		
			break;

			case PRESS_S2:	
			sysHandleTemp = WIN_CLIMB_HANDLE;		
			break;

			case HOLD_SHORT_S0:
			//先关闭场景相关功能模块
			msg.id                       = SCENE_ID;
		    msg.module.sceneEvent.id     = RUN_SCENE_CLOSE;
		    MultiModuleTask_EventSet(msg);
		    sceneSwitch = SCENE_NULL;

		    //设置数据保存事件
		    appMsg.id 	= APP_SCENE_DATA_SAVE;
		    appMsg.para = RUN_SCENE_TYPE;
		    App_Task_EventSet(appMsg);

		    saveState  	= SAVING_STATE;
			App_PicRamClear();
			AppCommonSaveStateDisp(3, systermConfig.systermLanguge);
			App_RoundDisp(ROUND_NONE,ROUND_HORIZONTAL_PIXEL);
			curSubWinRunHandle = SUB_WIN_RUN_SAVEING;
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
static uint16 App_Run_Win_Tag_Action(uint16 sysHandle,menuMessage message)
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
static uint16 App_Run_Win_Tag_Gesture(uint16 sysHandle,menuMessage message)
{
	uint16 	sysHandleTemp;
	scene_detail_s sceneDetailTemp;
	scene_data_s   sceneDataTemp;

	switch(message.op)
	{
		case GESTURE_ACTION_HAND_LIFT:
		sysHandleTemp = WIN_RUN_HANDLE;
		if (runWinIlde)
		{
			winTimeCnt = 0;
			runWinIlde = 0;
			cycleFlag  = 0;
			App_PicRamClear();
			Mid_Scene_DetailRead(RUN_SCENE_TYPE, &sceneDetailTemp);
			Mid_Scene_LogDataRead(RUN_SCENE_TYPE,&sceneDataTemp);
			AppRunStepNumFrequencyDisp(sceneDetailTemp.runDetail.stepTotal, sceneDataTemp.runScene.freqLog);
			App_RoundDisp(ROUND_NONE,ROUND_HORIZONTAL_PIXEL);
			curSubWinRunHandle = SUB_WIN_RUN_STEP;
		}
		break;

		default:
		break;
	}
	return sysHandleTemp;
}

//**********************************************************************
// 函数功能：  APP拍照虚拟菜单处理
// 输入参数：  sysHandle:当前句柄
//			   message： 信息内容	
// 返回参数：  响应后的窗口句柄
static uint16 App_Run_Win_Tag_PhoneApp(uint16 sysHandle,menuMessage message)
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
static uint16 App_Run_Win_Tag_IconFlick(uint16 sysHandle,menuMessage message)
{
	uint16 sysHandleTemp;
	static uint8 flickCnt = 1 ;

	sysHandleTemp = sysHandle;
	switch(curSubWinRunHandle)
	{
		case SUB_WIN_RUN_NONE:

		break;

		case SUB_WIN_RUN_SAVEING:
		App_PicRamClear();
		AppCommonSaveStateDisp(flickCnt % 4, systermConfig.systermLanguge);
		App_RoundDisp(ROUND_NONE,ROUND_VERTICAL_PIXEL);
		break;

		default:break;
	}
	flickCnt ++;
	if (flickCnt >= 4)
	{
		flickCnt = 1;
	}
	return sysHandleTemp;
}

//**********************************************************************
// 函数功能：  响应窗口切换或窗口刷新
// 输入参数：  sysHandle:当前句柄
//			   message： 信息内容			
// 返回参数：  响应后的窗口句柄
static uint16 App_Run_Win_Tag_WinChange(uint16 sysHandle,menuMessage message)
{
	uint16 			sysHandleTemp;
	rtc_time_s      timeTemp;
	weahter_s  		weatherinfo;
	scene_detail_s  sceneDetailTemp;
	scene_data_s    sceneDataTemp;
	uint8  			hrmval;
	multimodule_task_msg_t  msg;
	app_task_msg_t  appMsg;


	winTimeCnt ++;
	sysHandleTemp 		= sysHandle;

	if (!runWinIlde)
	{
		switch(curSubWinRunHandle)
		{
			case SUB_WIN_RUN_NONE:
			if (winTimeCnt >= WIN_TIME_2SEC)
			{
				winTimeCnt = 0;
				App_PicRamClear();
				AppCommonCountDownStateDisp(3);
				App_RoundDisp(ROUND_NONE,ROUND_HORIZONTAL_PIXEL);
				curSubWinRunHandle = SUB_WIN_RUN_COUNT_3;
			}else if (winTimeCnt == WIN_TIME_1SEC)
			{
				//检查存储空间事件
			    appMsg.id 	= APP_SCENE_DATA_SAVE_CHECK;
			    appMsg.para = RUN_SCENE_TYPE;
			    App_Task_EventSet(appMsg);
			}
			break;

			case SUB_WIN_RUN_COUNT_3:
			if (winTimeCnt >= WIN_TIME_1SEC - 1)
			{
				winTimeCnt = 0;
				App_PicRamClear();
				AppCommonCountDownStateDisp(2);
				App_RoundDisp(ROUND_NONE,ROUND_HORIZONTAL_PIXEL);
				curSubWinRunHandle = SUB_WIN_RUN_COUNT_2;
			}
			break;

			case SUB_WIN_RUN_COUNT_2:
			if (winTimeCnt >= WIN_TIME_1SEC - 1)
			{
				winTimeCnt = 0;
				App_PicRamClear();
				AppCommonCountDownStateDisp(1);
				App_RoundDisp(ROUND_NONE,ROUND_HORIZONTAL_PIXEL);
				curSubWinRunHandle = SUB_WIN_RUN_COUNT_1;
			}
			break;

			case SUB_WIN_RUN_COUNT_1:
			if (winTimeCnt >= WIN_TIME_1SEC - 1)
			{
				winTimeCnt 			= 0;

				if (!runSceneValid)
				{
					runSceneValid 	= 1;
					sceneSwitch 	= SCENE_RUNNING;
					msg.id                       = SCENE_ID;
				    msg.module.sceneEvent.id     = RUN_SCENE_OPEN;
				    MultiModuleTask_EventSet(msg);
				}
				MultiModuleTask_EventSet(OP_MOTO);
				App_PicRamClear();
				Mid_Scene_DetailRead(RUN_SCENE_TYPE, &sceneDetailTemp);
				Mid_Scene_LogDataRead(RUN_SCENE_TYPE,&sceneDataTemp);
				AppRunStepNumFrequencyDisp(sceneDetailTemp.runDetail.stepTotal, sceneDataTemp.runScene.freqLog);
				App_RoundDisp(ROUND_LEFT,ROUND_HORIZONTAL_PIXEL);
				curSubWinRunHandle = SUB_WIN_RUN_STEP;
			}
			break;

			case SUB_WIN_RUN_STEP:
			if (cycleFlag)
			{
				if (winTimeCnt >= WIN_TIME_5SEC)
				{
					cycleFlag 		 = 0;
					runWinIlde 		 = 1;
					App_PicRamClear();
					App_RoundDisp(ROUND_NONE,ROUND_HORIZONTAL_PIXEL);
					//显示关闭
					App_DispOff();
				}	
			}
			else
			{
				if (winTimeCnt >= WIN_TIME_3SEC +1)
				{
					winTimeCnt = 0;
					App_PicRamClear();
					Mid_Scene_DetailRead(RUN_SCENE_TYPE, &sceneDetailTemp);
					Mid_Scene_LogDataRead(RUN_SCENE_TYPE,&sceneDataTemp);
					AppRunDistanceSpeedDisp(sceneDetailTemp.runDetail.distanceTotal, sceneDataTemp.runScene.paceLog);
					App_RoundDisp(ROUND_LEFT,ROUND_HORIZONTAL_PIXEL);
					curSubWinRunHandle = SUB_WIN_RUN_SPEED;
				}
			}
			break;

			case SUB_WIN_RUN_SPEED:
			if (winTimeCnt >= WIN_TIME_3SEC + 1)
			{
				winTimeCnt = 0;
				App_PicRamClear();
				Mid_Scene_DetailRead(RUN_SCENE_TYPE, &sceneDetailTemp);
				Mid_Hrm_Read(&hrmval);
				AppCommonHrdKcalDisp(hrmval, sceneDetailTemp.runDetail.calorieTotal);
				App_RoundDisp(ROUND_LEFT,ROUND_HORIZONTAL_PIXEL);
				curSubWinRunHandle = SUB_WIN_RUN_CAL;
			}
			break;

			case SUB_WIN_RUN_CAL:
			if (winTimeCnt >= WIN_TIME_3SEC + 1)
			{
				winTimeCnt = 0;
				App_PicRamClear();
				AppCommonSportTimeDisp(runDuarationTime);
				App_RoundDisp(ROUND_LEFT,ROUND_HORIZONTAL_PIXEL);
				curSubWinRunHandle = SUB_WIN_RUN_TIME;
			}
			else
			{
				App_PicRamClear();
				Mid_Scene_DetailRead(RUN_SCENE_TYPE, &sceneDetailTemp);
				Mid_Hrm_Read(&hrmval);
				AppCommonHrdKcalDisp(hrmval, sceneDetailTemp.runDetail.calorieTotal);
				App_RoundDisp(ROUND_NONE,ROUND_HORIZONTAL_PIXEL);
			}
			break;

			case SUB_WIN_RUN_TIME:
			if (winTimeCnt >= WIN_TIME_3SEC + 1)
			{
				winTimeCnt = 0;
				App_PicRamClear();
				Mid_Rtc_TimeRead(&timeTemp);
				Mid_WeatherScene_TendencyGet(&weatherinfo);
				AppCommontimeTemperatureDisp(timeTemp, systermConfig.systermTimeType, weatherinfo.weahterCurTemperature);
				App_RoundDisp(ROUND_LEFT,ROUND_HORIZONTAL_PIXEL);
				curSubWinRunHandle = SUB_WIN_RUN_TEMP;
			}
			else
			{
				App_PicRamClear();
				AppCommonSportTimeDisp(runDuarationTime);
				App_RoundDisp(ROUND_NONE,ROUND_HORIZONTAL_PIXEL);
			}
			break;

			case SUB_WIN_RUN_TEMP:
			if (winTimeCnt >= WIN_TIME_3SEC + 1)
			{
				winTimeCnt = 0;
				cycleFlag  = 1;
				App_PicRamClear();
				Mid_Scene_DetailRead(RUN_SCENE_TYPE, &sceneDetailTemp);
				Mid_Scene_LogDataRead(RUN_SCENE_TYPE,&sceneDataTemp);
				AppRunStepNumFrequencyDisp(sceneDetailTemp.runDetail.stepTotal, sceneDataTemp.runScene.freqLog);
				App_RoundDisp(ROUND_LEFT,ROUND_HORIZONTAL_PIXEL);
				curSubWinRunHandle = SUB_WIN_RUN_STEP;
			}
			break;

			case SUB_WIN_RUN_SAVEING:
			if ((winTimeCnt >= WIN_TIME_3SEC) && (saveState  == SAVE_OK))
			{
				winTimeCnt = 0;
				App_PicRamClear();
				AppCommonSaveStateDisp(0, systermConfig.systermLanguge);
				App_RoundDisp(ROUND_NONE,ROUND_VERTICAL_PIXEL);
				curSubWinRunHandle = SUB_WIN_RUN_SAVED;
			}

			if (winTimeCnt >= WIN_TIME_10SEC)
			{
				winTimeCnt = 0;
				App_PicRamClear();
				AppCommonSaveStateDisp(0, systermConfig.systermLanguge);
				App_RoundDisp(ROUND_NONE,ROUND_VERTICAL_PIXEL);
				curSubWinRunHandle = SUB_WIN_RUN_SAVED;
			}
			break;

			case SUB_WIN_RUN_SAVED:
			if (winTimeCnt >= WIN_TIME_2SEC)
			{
				winTimeCnt = 0;
				cycleFlag  = 0;
				saveState  = SAVE_NULL;
				sysHandleTemp 	   = WIN_IDLE_HANDLE;		//休眠
				curSubWinRunHandle = SUB_WIN_RUN_NONE;
			}
			break;

			case SUB_WIN_RUN_SAVE_FULL:
			if (winTimeCnt >= WIN_TIME_8SEC)
			{
				if (runSceneValid)
				{
					msg.id                       = SCENE_ID;
				    msg.module.sceneEvent.id     = RUN_SCENE_CLOSE;
				    MultiModuleTask_EventSet(msg);
				    sceneSwitch = SCENE_NULL;
				}
				App_PicRamClear();
				App_RoundDisp(ROUND_NONE,ROUND_HORIZONTAL_PIXEL);

				winTimeCnt = 0;
				cycleFlag  = 0;
				runSceneValid 	 = 0;
				sysHandleTemp 	 = WIN_IDLE_HANDLE;		//休眠
				curSubWinRunHandle = SUB_WIN_RUN_NONE;
			}
			break;

			default:
			msg.id                       = SCENE_ID;
		    msg.module.sceneEvent.id     = RUN_SCENE_CLOSE;
		    MultiModuleTask_EventSet(msg);
		    sceneSwitch = SCENE_NULL;

			App_PicRamClear();
			App_RoundDisp(ROUND_NONE,ROUND_HORIZONTAL_PIXEL);

			winTimeCnt = 0;
			cycleFlag  = 0;
			runSceneValid 	 = 0;
			sysHandleTemp 	 = WIN_IDLE_HANDLE;		//休眠
			curSubWinRunHandle = SUB_WIN_RUN_NONE;
			break;
		}
	}
	return sysHandleTemp;
}
