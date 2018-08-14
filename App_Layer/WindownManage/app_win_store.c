#include "platform_common.h"

#include "app_display.h"
#include "app_protocal.h"
#include "app_win_common.h"
#include "app_win_remind.h"
#include "app_systerm.h"
#include "app_win_store.h"


/*************** func declaration ********************************/
//罗列窗口内所有菜单的处理函数
static uint16 App_Store_Win_Tag_Remind(uint16 sysHandle,menuMessage message);
static uint16 App_Store_Win_Tag_Key(uint16 sysHandle,menuMessage message);
static uint16 App_Store_Win_Tag_WinChange(uint16 sysHandle,menuMessage message);
static uint16 App_Store_Win_Tag_IconFlick(uint16 sysHandle,menuMessage message);

/*************** macro define ********************************/
#define 		STORE_WIN_TAG_MAX 		4 			//窗口内最大菜单个数


//提醒窗口子窗口定义
typedef enum
{
	SUB_WIN_STORE_NONE 		= 0x0000,	//仓储状态
	SUB_WIN_STORE_CHARGE,				//电池充电状态
	SUB_WIN_STORE_OTA,					//ＯＴＡ状态
	SUB_WIN_STORE_STARTUP,
}StoreSubWinHandle;

/************** variable define *****************************/
//罗列即时提醒窗口内所有菜单（实体菜单、虚拟菜单）
static 	WinMenu	StoreWindowMenu[STORE_WIN_TAG_MAX] = 
				{
					{TAG_KEY,			0,0,	App_Store_Win_Tag_Key	},
					{TAG_REMIND,		0,0,	App_Store_Win_Tag_Remind	},
					{TAG_WIN_CHANGE,	0,0,	App_Store_Win_Tag_WinChange	},
					{TAG_ICON_FLICK, 	0,0,	App_Store_Win_Tag_IconFlick	},
				};
	
static uint16 	curSubWinStoreHandle 	= SUB_WIN_STORE_NONE; 	
static uint16 	SysHandleBak			= WIN_STORE_HANDLE;

/************** function define ******`***********************/

//**********************************************************************
// 函数功能：  窗口内部菜单初始化
// 输入参数：  无	

// 返回参数：  0x00:初始化成功，0xff：初始化失败
uint16 App_Store_Win_Init(void)
{
	uint16 	sysHandleTemp;
	uint8	batLevel;
	uint8   socTemp;
	multimodule_task_msg_t  msg;

	cycleFlag  = 0;
	winTimeCnt = 0;
	sysHandleTemp 		= WIN_STORE_HANDLE;

	switch(curSubWinStoreHandle)
	{
		case SUB_WIN_STORE_NONE:			
		App_PicRamClear();
		App_RoundDisp(ROUND_NONE,ROUND_VERTICAL_PIXEL);
		//显示关闭
		App_DispOff();
		App_SystermPowerOff();
		// //进入仓储作一次电量检测
		 msg.id                     = BAT_ID;
         msg.module.batEvent.id     = BAT_CHECK;
         MultiModuleTask_EventSet(msg);
		break;

		case SUB_WIN_STORE_CHARGE:
		batLevel = Mid_Bat_LevelRead();
		Mid_Bat_SocRead(&socTemp);
		App_PicRamClear();
		App_ChargeTittle(socTemp,batLevel);
		App_RoundDisp(ROUND_NONE,ROUND_VERTICAL_PIXEL);
		//关闭系统部分场景功能
		App_SystermSuspend();
		break;

		case SUB_WIN_STORE_OTA:
		App_PicRamClear();
		App_OtaTittle(systermConfig.systermLanguge,0);
		App_RoundDisp(ROUND_NONE,ROUND_VERTICAL_PIXEL);
		App_OtaSystermSuspend();
		break;

		default:break;
	}
	
	return sysHandleTemp;
}

//**********************************************************************
// 函数功能：  窗口处理回调函数，每一窗口分配一个回调函数，在回调函数中执行具体菜单功能
// 输入参数：   sysHandle ：  窗口的句柄号
//				tagMenu：     当前响应的菜单
// 				message： 	  菜单发送的信息：动作、数值、状态等		

// 返回参数：  返回菜单响应的窗口句柄号
uint16 App_Store_Win_CallBack(uint16 sysHandle,uint16 tagMenu,menuMessage message)
{
	uint8 	i;
	uint16 	handletemp;

	if (sysHandle != WIN_STORE_HANDLE)
	{
		return sysHandle;
	}

	for (i = 0; i < STORE_WIN_TAG_MAX; i++)
	{
		if (StoreWindowMenu[i].tag == tagMenu)
		{
			break;
		}
	}
	handletemp 		= sysHandle;

	if (i < STORE_WIN_TAG_MAX && StoreWindowMenu[i].callback != NULL)
	{		
		handletemp = StoreWindowMenu[i].callback(handletemp,message);
	}

	return handletemp;
}

//**********************************************************************
// 函数功能：  标记仓储前的窗口句柄，以便恢复
// 输入参数：  无	
// 返回参数：  响应后的窗口句柄
uint16 App_Store_Win_Bak(uint16 sysHandle,menuMessage message)
{
	uint16 	sysHandleTemp;


	SysHandleBak		= sysHandle;	
	sysHandleTemp 		= WIN_STORE_HANDLE;

	switch(message.op)
	{
		case STORE_REMIND:
		if (message.state == STORE_ENTER)
		{
			curSubWinStoreHandle = SUB_WIN_STORE_NONE;
		}
		else
		{
			sysHandleTemp = SysHandleBak;
		}
		break;

		case BAT_REMIND:
		if (message.state == BAT_CHARGING_STATE)
		{
			curSubWinStoreHandle = SUB_WIN_STORE_CHARGE;
		}else if (message.state == BAT_LOW_VOLTAGE_STATE)
		{
			curSubWinStoreHandle = SUB_WIN_STORE_NONE;
		}
		else
		{
			sysHandleTemp = SysHandleBak;
		}
		break;

		case OTA_REMIND:
		if (message.state == OTA_ENTER)
		{
			curSubWinStoreHandle = SUB_WIN_STORE_OTA;
		}
		else
		{
			sysHandleTemp = SysHandleBak;
		}
		break;

		default:
		sysHandleTemp  			= SysHandleBak;
		break;
	}
	return sysHandleTemp;
}

//**********************************************************************
// 函数功能：  窗口内部提醒菜单处理
// 输入参数：  无	
// 返回参数：  响应后的窗口句柄
static uint16 App_Store_Win_Tag_Remind(uint16 sysHandle,menuMessage message)
{
	uint16 	sysHandleTemp;
	uint8	batLevel;
	uint8   socTemp;

	sysHandleTemp 		= WIN_STORE_HANDLE;

	switch(message.op)
	{
		case STORE_REMIND:
		if (message.state == STORE_ENTER)
		{
			curSubWinStoreHandle = SUB_WIN_STORE_NONE;
		}
		else
		{
			App_SystermRecover();
			curSubWinStoreHandle = SUB_WIN_STORE_NONE;
		}
		break;

		case BAT_REMIND:
		if (message.state == BAT_CHARGING_STATE)
		{
			App_PicRamClear();
			if (curSubWinStoreHandle != SUB_WIN_STORE_OTA)
			{
				curSubWinStoreHandle = SUB_WIN_STORE_CHARGE;
			}
			batLevel = Mid_Bat_LevelRead();
			Mid_Bat_SocRead(&socTemp);
			App_ChargeTittle(socTemp,batLevel);		
			App_RoundDisp(ROUND_NONE,ROUND_VERTICAL_PIXEL);
			winTimeCnt 		= 0;
			cycleFlag 		= 0;
		}
		else if (message.state == BAT_DISCHARGE_STATE)
		{
			if (SysHandleBak == WIN_STORE_HANDLE)
			{
				sysHandleTemp = WIN_STORE_HANDLE;
			}
			else
			{
				App_SystermRecover();
				sysHandleTemp = WIN_TIME_HANDLE;
			}
			curSubWinStoreHandle = SUB_WIN_STORE_NONE;
		}
		else
		{
			curSubWinStoreHandle = SUB_WIN_STORE_CHARGE;
		}
		break;

		case OTA_REMIND:
		if (message.state == OTA_ENTER)
		{
			curSubWinStoreHandle = SUB_WIN_STORE_OTA;
			App_PicRamClear();
			App_OtaTittle(systermConfig.systermLanguge,0);
			App_RoundDisp(ROUND_NONE,ROUND_VERTICAL_PIXEL);
		}
		else
		{
			sysHandleTemp = WIN_TIME_HANDLE;
			curSubWinStoreHandle = SUB_WIN_STORE_NONE;
		}
		break;

		default:break;
	}
	return sysHandleTemp;
}

//**********************************************************************
// 函数功能：  窗口内部按键菜单处理
// 输入参数：  sysHandle:当前句柄
//			   message： 信息内容		
// 返回参数：  响应后的窗口句柄
static uint16 App_Store_Win_Tag_Key(uint16 sysHandle,menuMessage message)
{
	uint16 	sysHandleTemp;
	uint8 	batLevel;
	uint8   socTemp;

	winTimeCnt 		= 0;
	cycleFlag 		= 0;
	sysHandleTemp 	= sysHandle;

	switch(message.val)
	{
		case PRESS_S0:
		switch(curSubWinStoreHandle)
		{
			case SUB_WIN_STORE_NONE:
			batLevel 	= Mid_Bat_LevelRead();
			App_PicRamClear();
			if (batLevel <= MID_BAT_LEVER_2)
			{			
				App_StoreBatLow();
			}
			else
			{
				App_StoreTittle(systermConfig.systermLanguge);
			}		
			App_RoundDisp(ROUND_NONE,ROUND_VERTICAL_PIXEL);
			break;
			
			case SUB_WIN_STORE_CHARGE:
			batLevel = Mid_Bat_LevelRead();
			Mid_Bat_SocRead(&socTemp);
			App_PicRamClear();
			App_ChargeTittle(socTemp,batLevel);
			App_RoundDisp(ROUND_NONE,ROUND_VERTICAL_PIXEL);
			break;

			case SUB_WIN_STORE_OTA:
			App_PicRamClear();
			App_OtaTittle(systermConfig.systermLanguge,0);
			App_RoundDisp(ROUND_NONE,ROUND_VERTICAL_PIXEL);
			break;

			default:break;
		}
		break;

		case PRESS_S1:	
		switch(curSubWinStoreHandle)
		{
			case SUB_WIN_STORE_NONE:


			break;


			case SUB_WIN_STORE_CHARGE:

			break;

			case SUB_WIN_STORE_OTA:

			break;

			default:break;
		}
		break;

		case PRESS_S2:	
		switch(curSubWinStoreHandle)
		{
			case SUB_WIN_STORE_NONE:


			break;


			case SUB_WIN_STORE_CHARGE:

			break;

			case SUB_WIN_STORE_OTA:

			break;

			default:break;
		}
		break;

		case HOLD_SHORT_S0:	
		if (curSubWinStoreHandle == SUB_WIN_STORE_NONE)//在仓储状态下，长按开机
		{
			App_PicRamClear();
			App_PowerUpTittle(systermConfig.systermLanguge);
			App_RoundDisp(ROUND_NONE,ROUND_VERTICAL_PIXEL);
			curSubWinStoreHandle = SUB_WIN_STORE_STARTUP;
		}
		break;

		case HOLD_LONG_S0:

		if (curSubWinStoreHandle != SUB_WIN_STORE_NONE)
		{
			curSubWinStoreHandle = SUB_WIN_STORE_NONE;
		}
		
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
static uint16 App_Store_Win_Tag_IconFlick(uint16 sysHandle,menuMessage message)
{
	uint16 sysHandleTemp;
	uint8  socTemp;
	uint8  chargeState;
	static uint8 flickCnt = 0;

	sysHandleTemp = sysHandle;
	switch(curSubWinStoreHandle)
	{
		case SUB_WIN_STORE_NONE:

		break;

		case SUB_WIN_STORE_CHARGE:
		if (cycleFlag == 0)
		{
			Mid_Bat_ChargeStateRead(&chargeState);
			Mid_Bat_SocRead(&socTemp);
			App_PicRamClear();
			if (chargeState == MID_BAT_FULL_CHARGE)
			{
				App_ChargeTittle(socTemp,BAT_LEVER_5);
			}
			else
			{
				App_ChargeTittle(socTemp,flickCnt % 6);
			}
			App_RoundDisp(ROUND_NONE,ROUND_VERTICAL_PIXEL);
		}
		break;

		case SUB_WIN_STORE_OTA:
		if (cycleFlag == 0)
		{
			App_PicRamClear();
			App_OtaTittle(systermConfig.systermLanguge,flickCnt % 4);
			App_RoundDisp(ROUND_NONE,ROUND_VERTICAL_PIXEL);
		}
		break;

		default:break;
	}
	flickCnt ++;
	return sysHandleTemp;
}

//**********************************************************************
// 函数功能：  响应窗口切换或窗口刷新
// 输入参数：  sysHandle:当前句柄
//			   message： 信息内容			
// 返回参数：  响应后的窗口句柄
static uint16 App_Store_Win_Tag_WinChange(uint16 sysHandle,menuMessage message)
{
	uint16 			sysHandleTemp;

	winTimeCnt ++;
	sysHandleTemp 		= sysHandle;

	switch(curSubWinStoreHandle)
	{
		case SUB_WIN_STORE_NONE:
		if (winTimeCnt >= WIN_TIME_1SEC)
		{
			//显示关闭
			App_DispOff();
		}
		break;

		case SUB_WIN_STORE_STARTUP:
		if (winTimeCnt >= WIN_TIME_2SEC)
		{
			sysHandleTemp = WIN_TIME_HANDLE;
			curSubWinStoreHandle = SUB_WIN_STORE_NONE;
			App_SystermPowerOn();
		}
		break;

		case SUB_WIN_STORE_CHARGE:
		if (winTimeCnt >= WIN_TIME_5SEC)
		{
			cycleFlag = 1;
			//显示关闭
			App_DispOff();
		}
		break;

		case SUB_WIN_STORE_OTA:
		if (winTimeCnt >= WIN_TIME_5SEC)
		{
			cycleFlag = 1;
			//显示关闭
			App_DispOff();
		}
		break;

		default:break;
	}

	return sysHandleTemp;
}


