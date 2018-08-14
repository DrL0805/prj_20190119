#include "platform_common.h"
#include "app_remind_manage.h"
#include "app_display.h"
#include "app_protocal.h"
#include "app_win_common.h"
#include "app_win_remind.h"
#include "app_systerm.h"



/*************** func declaration ********************************/
//罗列窗口内所有菜单的处理函数
static uint16 App_Remind_Win_Tag_Remind(uint16 sysHandle,menuMessage message);
static uint16 App_Remind_Win_Tag_Key(uint16 sysHandle,menuMessage message);
static uint16 App_Remind_Win_Tag_Action(uint16 sysHandle,menuMessage message);
static uint16 App_Remind_Win_Tag_Gesture(uint16 sysHandle,menuMessage message);
static uint16 App_Remind_Win_Tag_PhoneApp(uint16 sysHandle,menuMessage message);
static uint16 App_Remind_Win_Tag_WinChange(uint16 sysHandle,menuMessage message);


/*************** macro define ********************************/
#define 		REMIND_WIN_TAG_MAX 		10 			//窗口内最大菜单个数


//提醒窗口子窗口定义
typedef enum
{
	SUB_WIN_REMIND_NONE 				= 0x0000,
	SUB_WIN_REMIND_BAT 					= 0x0001,
	SUB_WIN_REMIND_BLE 					= 0x0002,
	SUB_WIN_REMIND_MESSAGE 				= 0x0004,
	SUB_WIN_REMIND_UASUAL_MSG 			= 0x0008,
	SUB_WIN_REMIND_PHONE 				= 0x0010,
	SUB_WIN_REMIND_MESSAGE_DETAIL 		= 0x0020,
	SUB_WIN_REMIND_UASUAL_MSG_DETAIL  	= 0x0040,
	SUB_WIN_REMIND_PHONE_DETAIL   		= 0x0080,

	SUB_WIN_REMIND_ALARM 		 		= 0x0100,
	SUB_WIN_REMIND_SPORT_COMPLETE 		= 0x0200,
	SUB_WIN_REMIND_LONG_SIT 		 	= 0x0400,
	SUB_WIN_REMIND_TAKE_PICTURE 		= 0x0800,

	SUB_WIN_REMIND_PAIRE 				= 0x1000,
}RemindSubWinHandle;


/************** variable define *****************************/
//罗列即时提醒窗口内所有菜单（实体菜单、虚拟菜单）
static 	WinMenu	RemindWindowMenu[REMIND_WIN_TAG_MAX] = 
				{
					{TAG_REMIND,		0,0,	App_Remind_Win_Tag_Remind	},
					{TAG_KEY,			0,0,	App_Remind_Win_Tag_Key		},
					{TAG_ACTION,		0,0,	App_Remind_Win_Tag_Action	},
					{TAG_GESTURE,		0,0,	App_Remind_Win_Tag_Gesture	},
					{TAG_PHONE_APP,		0,0,	App_Remind_Win_Tag_PhoneApp },
					{TAG_WIN_CHANGE,	0,0,	App_Remind_Win_Tag_WinChange},
				};
	
static uint16 	subWinRemindHandle 		= SUB_WIN_REMIND_NONE;	//用于即时提醒窗口下子窗口的标记
static uint16 	curSubWinRemindHandle 	= SUB_WIN_REMIND_NONE; 	
static uint16 	SysHandleBak 			= WIN_INVALID_HANDLE;	//标记进入即时提醒前的窗口句柄 

static uint8 	missCallCnt = 0;								//未接来电个数统计
static uint8    callflow = 0;
static uint32 	paireCode = 0;

app_remind_msg_t detailRemindMsg;
/************** function define ******`***********************/

//**********************************************************************
// 函数功能：  窗口内部菜单初始化
// 输入参数：  无	

// 返回参数：  0x00:初始化成功，0xff：初始化失败
//**********************************************************************
uint16 App_Remind_Win_Init(void)
{
	uint16 	sysHandleTemp;
	uint8 	u8Temp;
	time_data_t timeDate;
	//alarm_clock_t   alarmInfo;
    rtc_time_s  currtime;
	uint8 	stringbuf[2] = {0};

	cycleFlag  			= 0;
	winTimeCnt 			= 0;
	sysHandleTemp 		= WIN_REMIND_HANDLE;


	App_PicRamClear();
	switch(curSubWinRemindHandle)
	{
		case SUB_WIN_REMIND_NONE:

		break;

		case SUB_WIN_REMIND_BAT:

		break;

		case SUB_WIN_REMIND_BLE:
		AppRemindBleStateDisp(BINARY_TYPE_OFF,systermConfig.systermLanguge);
		MultiModuleTask_EventSet(BLE_DISCONNECT_MOTO);
		break;

		case SUB_WIN_REMIND_PAIRE:
		App_PairCodeDisp(paireCode);
		break;

		case SUB_WIN_REMIND_MESSAGE: 
		AppRemindPhoneMsgDisp(stringbuf,2, LETTER_TYPTE_UNICODE);
		MultiModuleTask_EventSet(MSG_REMIND_MOTO);
		break;

		case SUB_WIN_REMIND_UASUAL_MSG:
		AppRemindAppMsgDisp(stringbuf,2, LETTER_TYPTE_UNICODE);	
		MultiModuleTask_EventSet(MSG_REMIND_MOTO);
		break;

		case SUB_WIN_REMIND_PHONE:
		AppRemindCallDisp(stringbuf,2, LETTER_TYPTE_UNICODE);
		MultiModuleTask_EventSet(CALL_REMIND_MOTO);
		break;

		case SUB_WIN_REMIND_MESSAGE_DETAIL: 
		if (detailRemindMsg.remindMsg.phonenumberlen > 0) //非联系人
		{
			AppRemindPhoneMsgDisp((uint8 *)detailRemindMsg.remindMsg.phonenumber,detailRemindMsg.remindMsg.phonenumberlen * 2, LETTER_TYPTE_UNICODE);
		}
		else
		{
			AppRemindPhoneMsgDisp((uint8 *)detailRemindMsg.remindMsg.name,detailRemindMsg.remindMsg.namelen * 2, LETTER_TYPTE_UNICODE);
		}
		MultiModuleTask_EventSet(MSG_REMIND_MOTO);
		break;

		case SUB_WIN_REMIND_UASUAL_MSG_DETAIL:	
		AppRemindAppMsgDisp((uint8 *)detailRemindMsg.remindMsg.name,detailRemindMsg.remindMsg.namelen * 2, LETTER_TYPTE_UNICODE);
		MultiModuleTask_EventSet(MSG_REMIND_MOTO);
		break;

		case SUB_WIN_REMIND_PHONE_DETAIL:		
		if (detailRemindMsg.remindMsg.phonenumberlen > 0) //非联系人
		{
			AppRemindCallDisp((uint8 *)detailRemindMsg.remindMsg.phonenumber,detailRemindMsg.remindMsg.phonenumberlen * 2, LETTER_TYPTE_UNICODE);
		}
		else
		{
			AppRemindCallDisp((uint8 *)detailRemindMsg.remindMsg.name,detailRemindMsg.remindMsg.namelen * 2, LETTER_TYPTE_UNICODE);
		}	
		MultiModuleTask_EventSet(CALL_REMIND_MOTO);	
		break;


		case SUB_WIN_REMIND_ALARM:
        Mid_Rtc_TimeRead(&currtime);
        timeDate.Hour = currtime.hour;
		timeDate.Min  = currtime.min;
		AppRemindClockDisp(&timeDate,systermConfig.systermTimeType);
		MultiModuleTask_EventSet(ALARM_MOTO);		
		break;

		case SUB_WIN_REMIND_SPORT_COMPLETE:
		AppRemindGoalAttainedDisp(systermConfig.systermLanguge);
		MultiModuleTask_EventSet(SPORT_COMPLETE_MOTO);
		break;

		case SUB_WIN_REMIND_LONG_SIT:
		AppRemindLongSitDisp(systermConfig.systermLanguge);	
		MultiModuleTask_EventSet(LONG_SIT_MOTO);
		break;

		case SUB_WIN_REMIND_TAKE_PICTURE:
		App_TakepictureDisp();
		break;

		default:
		sysHandleTemp  	= SysHandleBak;
		break;
	}	
	App_RoundDisp(ROUND_NONE,ROUND_VERTICAL_PIXEL);

	return sysHandleTemp;
}

//**********************************************************************
// 函数功能：  窗口处理回调函数，每一窗口分配一个回调函数，在回调函数中执行具体菜单功能
// 输入参数：   sysHandle ：  窗口的句柄号
//				tagMenu：     当前响应的菜单
// 				message： 	  菜单发送的信息：动作、数值、状态等		

// 返回参数：  返回菜单响应的窗口句柄号
uint16 App_Remind_Win_CallBack(uint16 sysHandle,uint16 tagMenu,menuMessage message)
{
	uint8 	i;
	uint16 	handletemp;

	if (sysHandle != WIN_REMIND_HANDLE)
	{
		return sysHandle;
	}

	for (i = 0; i < REMIND_WIN_TAG_MAX; i++)
	{
		if (RemindWindowMenu[i].tag == tagMenu)
		{
			break;
		}
	}
	handletemp 		= sysHandle;

	if (i < REMIND_WIN_TAG_MAX && RemindWindowMenu[i].callback != NULL)
	{		
		handletemp = RemindWindowMenu[i].callback(handletemp,message);
	}

	return handletemp;
}

//**********************************************************************
// 函数功能：  标记即时提醒前的窗口句柄及提醒的类型和状态等信息，以便恢复
// 输入参数：  无	
// 返回参数：  响应后的窗口句柄
uint16 App_Remind_Win_Bak(uint16 sysHandle,menuMessage message)
{
	uint16 	sysHandleTemp;

	winTimeCnt 		= 0;
	cycleFlag  		= 0;
	SysHandleBak	= sysHandle;
	sysHandleTemp 	= WIN_REMIND_HANDLE;

	switch(message.op)
	{
		case BAT_REMIND:
		subWinRemindHandle		|= SUB_WIN_REMIND_BAT;
		curSubWinRemindHandle 	= SUB_WIN_REMIND_BAT;
		break;

		case BLE_REMIND:
		if (message.state == BLE_DICONNECT_STATE)
		{
			subWinRemindHandle		|= SUB_WIN_REMIND_BLE;
			curSubWinRemindHandle 	= SUB_WIN_REMIND_BLE;
		}
		else
		{
			sysHandleTemp  			= SysHandleBak;
		}
		break;

		case PAIRE_REMIND:
		if (message.state == ENTER_PAIRE_STATE)
		{
			paireCode 	= message.val;		
			subWinRemindHandle		|= SUB_WIN_REMIND_PAIRE;
			curSubWinRemindHandle 	= SUB_WIN_REMIND_PAIRE;
		}
		else
		{
			sysHandleTemp = SysHandleBak;
			subWinRemindHandle		&= ~SUB_WIN_REMIND_PAIRE;
		}
		break;

		case MESSAGE_REMIND:
		callflow = message.val;
		if (message.state == CANCEL_REMIND_STATE)
		{
			sysHandleTemp = SysHandleBak;
			subWinRemindHandle		&= ~SUB_WIN_REMIND_MESSAGE;
		}
		else
		{
			subWinRemindHandle		|= SUB_WIN_REMIND_MESSAGE;
			curSubWinRemindHandle 	= SUB_WIN_REMIND_MESSAGE;
		}
		break;

		case UASUAL_MSG_REMIND:
		callflow = message.val;
		if (message.state == CANCEL_REMIND_STATE)
		{
			sysHandleTemp = SysHandleBak;
			subWinRemindHandle		&=  ~SUB_WIN_REMIND_UASUAL_MSG;
		}
		else
		{
			subWinRemindHandle		|= SUB_WIN_REMIND_UASUAL_MSG;
			curSubWinRemindHandle 	= SUB_WIN_REMIND_UASUAL_MSG;
		}
		break;

		case PHONE_CALL_REMIND:
		callflow = message.val;
		if (message.state == CANCEL_REMIND_STATE)
		{
			sysHandleTemp = SysHandleBak;
			subWinRemindHandle		&= ~SUB_WIN_REMIND_PHONE;
			App_Remind_Win_MissCallDelete();
		}
		else
		{
			subWinRemindHandle		|= SUB_WIN_REMIND_PHONE;
			curSubWinRemindHandle 	= SUB_WIN_REMIND_PHONE;
		}
		break;

		case MESSAGE_DETAIL_REMIND:
		callflow = message.val;
		subWinRemindHandle		|= SUB_WIN_REMIND_MESSAGE_DETAIL;
		curSubWinRemindHandle 	= SUB_WIN_REMIND_MESSAGE_DETAIL;
		App_RemindManage_DetailRemindRead(&detailRemindMsg);
		break;

		case UASUAL_MSG_DETAIL_REMIND:
		callflow = message.val;
		subWinRemindHandle		|= SUB_WIN_REMIND_UASUAL_MSG_DETAIL;
		curSubWinRemindHandle 	= SUB_WIN_REMIND_UASUAL_MSG_DETAIL;
		App_RemindManage_DetailRemindRead(&detailRemindMsg);
		break;

		case PHONE_CALL_DETAIL_REMIND:
		callflow = message.val;
		subWinRemindHandle		|= SUB_WIN_REMIND_PHONE_DETAIL;
		curSubWinRemindHandle 	= SUB_WIN_REMIND_PHONE_DETAIL;
		App_RemindManage_DetailRemindRead(&detailRemindMsg);
		break;

		case ALARM_REMIND:
		subWinRemindHandle		|= SUB_WIN_REMIND_ALARM;
		curSubWinRemindHandle 	= SUB_WIN_REMIND_ALARM;
		break;

		case SPORT_COMPLETE_REMIND:
		subWinRemindHandle		|= SUB_WIN_REMIND_SPORT_COMPLETE;
		curSubWinRemindHandle 	= SUB_WIN_REMIND_SPORT_COMPLETE;
		break;

		case LONG_SIT_REMIND:
		subWinRemindHandle		|= SUB_WIN_REMIND_LONG_SIT;
		curSubWinRemindHandle 	= SUB_WIN_REMIND_LONG_SIT;
		break;

		case TAKE_PHOTO_REMIND:
		if (message.state == ENTER_TAKE_PHOTO_MODE)
		{
			subWinRemindHandle		|= SUB_WIN_REMIND_TAKE_PICTURE;
			curSubWinRemindHandle 	= SUB_WIN_REMIND_TAKE_PICTURE;
		}
		else
		{
			sysHandleTemp = SysHandleBak;
			subWinRemindHandle		&= ~SUB_WIN_REMIND_TAKE_PICTURE;
		}	
		break;

		default:
		sysHandleTemp  			= SysHandleBak;
		break;
	}

	return sysHandleTemp;
}

//**********************************************************************
// 函数功能：  来接来电个数读取
// 输入参数：  无	
// 返回参数：  未接来电
uint16 App_Remind_Win_MissCallRead(void)
{
	return missCallCnt;
}

//**********************************************************************
// 函数功能：  来接来电个数增加1
// 输入参数：  无	
// 返回参数：  无
uint16 App_Remind_Win_MissCallAdd(void)
{
	missCallCnt ++; 

	return 0;
}

//**********************************************************************
// 函数功能：  来接来电个数减少1
// 输入参数：  无	
// 返回参数：  无
uint16 App_Remind_Win_MissCallDelete(void)
{
	if (missCallCnt > 0)
	{
		missCallCnt --;

		return 0;
	}
	
	return 0xff;
}

//**********************************************************************
// 函数功能：  来接来电个数清零
// 输入参数：  无	
// 返回参数：  无
void App_Remind_Win_MissCallClear(void)
{
	missCallCnt = 0;
}

//**********************************************************************
// 函数功能：  窗口内部菜单处理
// 输入参数：  无	
// 返回参数：  响应后的窗口句柄
static uint16 App_Remind_Win_Tag_Remind(uint16 sysHandle,menuMessage message)
{
	uint16 	sysHandleTemp;
	time_data_t timeDate;
	rtc_time_s  currtime;
	uint8 	stringbuf[2] = {0};

	winTimeCnt 	  = 0;
	cycleFlag  	  = 0;
	sysHandleTemp = WIN_REMIND_HANDLE;

	App_PicRamClear();
	switch(message.op)
	{
		case BAT_REMIND:
		subWinRemindHandle		|= SUB_WIN_REMIND_BAT;
		curSubWinRemindHandle 	= SUB_WIN_REMIND_BAT;
		break;

		case BLE_REMIND:
		if (message.state == BLE_DICONNECT_STATE)
		{
			subWinRemindHandle		|= SUB_WIN_REMIND_BLE;
			curSubWinRemindHandle 	= SUB_WIN_REMIND_BLE;
			AppRemindBleStateDisp(BINARY_TYPE_OFF,systermConfig.systermLanguge);	
			MultiModuleTask_EventSet(BLE_DISCONNECT_MOTO);		
		}
		else
		{
			sysHandleTemp  			= SysHandleBak;
		}
		break;

		case PAIRE_REMIND:
		if (message.state == ENTER_PAIRE_STATE)
		{
			paireCode 	= message.val;		
			subWinRemindHandle		|= SUB_WIN_REMIND_PAIRE;
			curSubWinRemindHandle 	= SUB_WIN_REMIND_PAIRE;
			App_PairCodeDisp(paireCode);
			curSubWinRemindHandle = SUB_WIN_REMIND_NONE;
		}
		else
		{
			sysHandleTemp = SysHandleBak;
			subWinRemindHandle		&= ~SUB_WIN_REMIND_PAIRE;
		}
		break;

		case MESSAGE_REMIND:
		callflow = message.val;
		if (message.state == CANCEL_REMIND_STATE)
		{
			sysHandleTemp = SysHandleBak;
			subWinRemindHandle		&= ~SUB_WIN_REMIND_MESSAGE;
			App_Systerm_RemindCancel();	
			curSubWinRemindHandle = SUB_WIN_REMIND_NONE;
		}
		else
		{
			subWinRemindHandle		|= SUB_WIN_REMIND_MESSAGE;
			curSubWinRemindHandle 	= SUB_WIN_REMIND_MESSAGE;
			AppRemindPhoneMsgDisp(stringbuf,2, LETTER_TYPTE_UNICODE);
			MultiModuleTask_EventSet(MSG_REMIND_MOTO);
		}
		break;

		case UASUAL_MSG_REMIND:
		callflow = message.val;
		if (message.state == CANCEL_REMIND_STATE)
		{
			sysHandleTemp = SysHandleBak;
			subWinRemindHandle		&=  ~SUB_WIN_REMIND_UASUAL_MSG;
			App_Systerm_RemindCancel();	
			curSubWinRemindHandle = SUB_WIN_REMIND_NONE;
		}
		else
		{
			subWinRemindHandle		|= SUB_WIN_REMIND_UASUAL_MSG;
			curSubWinRemindHandle 	= SUB_WIN_REMIND_UASUAL_MSG;
			AppRemindAppMsgDisp(stringbuf,2, LETTER_TYPTE_UNICODE);		
			MultiModuleTask_EventSet(MSG_REMIND_MOTO);
		}
		break;

		case PHONE_CALL_REMIND:
		callflow = message.val;
		if (message.state == CANCEL_REMIND_STATE)
		{
			sysHandleTemp = SysHandleBak;
			subWinRemindHandle		&= ~SUB_WIN_REMIND_PHONE;
			App_Remind_Win_MissCallDelete();
			App_Systerm_RemindCancel();	
			curSubWinRemindHandle = SUB_WIN_REMIND_NONE;
		}
		else
		{
			subWinRemindHandle		|= SUB_WIN_REMIND_PHONE;
			curSubWinRemindHandle 	= SUB_WIN_REMIND_PHONE;
			App_Remind_Win_MissCallAdd();
			AppRemindCallDisp(stringbuf,2, LETTER_TYPTE_UNICODE);	
			MultiModuleTask_EventSet(CALL_REMIND_MOTO);
		}
		break;

		case MESSAGE_DETAIL_REMIND:
		callflow = message.val;
		subWinRemindHandle		|= SUB_WIN_REMIND_MESSAGE_DETAIL;
		curSubWinRemindHandle 	= SUB_WIN_REMIND_MESSAGE_DETAIL;
		App_RemindManage_DetailRemindRead(&detailRemindMsg);
		if (detailRemindMsg.remindMsg.phonenumberlen > 0) //非联系人
		{
			AppRemindPhoneMsgDisp((uint8 *)detailRemindMsg.remindMsg.phonenumber,detailRemindMsg.remindMsg.phonenumberlen * 2, LETTER_TYPTE_UNICODE);
		}
		else
		{
			AppRemindPhoneMsgDisp((uint8 *)detailRemindMsg.remindMsg.name,detailRemindMsg.remindMsg.namelen * 2, LETTER_TYPTE_UNICODE);
		}	
		MultiModuleTask_EventSet(MSG_REMIND_MOTO);
		break;

		case UASUAL_MSG_DETAIL_REMIND:
		callflow = message.val;
		subWinRemindHandle		|= SUB_WIN_REMIND_UASUAL_MSG_DETAIL;
		curSubWinRemindHandle 	= SUB_WIN_REMIND_UASUAL_MSG_DETAIL;
		App_RemindManage_DetailRemindRead(&detailRemindMsg);
		AppRemindAppMsgDisp((uint8 *)detailRemindMsg.remindMsg.name,detailRemindMsg.remindMsg.namelen * 2, LETTER_TYPTE_UNICODE);
		MultiModuleTask_EventSet(MSG_REMIND_MOTO);
		break;

		case PHONE_CALL_DETAIL_REMIND:
		callflow = message.val;
		subWinRemindHandle		|= SUB_WIN_REMIND_PHONE_DETAIL;
		curSubWinRemindHandle 	= SUB_WIN_REMIND_PHONE_DETAIL;
		App_RemindManage_DetailRemindRead(&detailRemindMsg);
		if (detailRemindMsg.remindMsg.phonenumberlen > 0) //非联系人
		{
			AppRemindCallDisp((uint8 *)detailRemindMsg.remindMsg.phonenumber,detailRemindMsg.remindMsg.phonenumberlen * 2, LETTER_TYPTE_UNICODE);
		}
		else
		{
			AppRemindCallDisp((uint8 *)detailRemindMsg.remindMsg.name,detailRemindMsg.remindMsg.namelen * 2, LETTER_TYPTE_UNICODE);
		}
		MultiModuleTask_EventSet(CALL_REMIND_MOTO);
		break;

		case ALARM_REMIND:
		subWinRemindHandle		|= SUB_WIN_REMIND_ALARM;
		curSubWinRemindHandle 	= SUB_WIN_REMIND_ALARM;
        Mid_Rtc_TimeRead(&currtime);
        timeDate.Hour = currtime.hour;
		timeDate.Min  = currtime.min;
		AppRemindClockDisp(&timeDate,systermConfig.systermTimeType);
		MultiModuleTask_EventSet(ALARM_MOTO);
		break;

		case SPORT_COMPLETE_REMIND:
		subWinRemindHandle		|= SUB_WIN_REMIND_SPORT_COMPLETE;
		curSubWinRemindHandle 	= SUB_WIN_REMIND_SPORT_COMPLETE;
		AppRemindGoalAttainedDisp(systermConfig.systermLanguge);
		MultiModuleTask_EventSet(SPORT_COMPLETE_MOTO);
		break;

		case LONG_SIT_REMIND:
		subWinRemindHandle		|= SUB_WIN_REMIND_LONG_SIT;
		curSubWinRemindHandle 	= SUB_WIN_REMIND_LONG_SIT;
		AppRemindLongSitDisp(systermConfig.systermLanguge);
		MultiModuleTask_EventSet(LONG_SIT_MOTO);
		break;

		case TAKE_PHOTO_REMIND:
		if (message.state == ENTER_TAKE_PHOTO_MODE)
		{
			subWinRemindHandle		|= SUB_WIN_REMIND_TAKE_PICTURE;
			curSubWinRemindHandle 	= SUB_WIN_REMIND_TAKE_PICTURE;
			App_TakepictureDisp();
		}
		else
		{
			sysHandleTemp 		= SysHandleBak;
			subWinRemindHandle	&= ~SUB_WIN_REMIND_TAKE_PICTURE;
		}
		break;

		default:
		sysHandleTemp  			= SysHandleBak;
		break;
	}
	App_RoundDisp(ROUND_NONE,ROUND_VERTICAL_PIXEL);
	return sysHandleTemp;
}
		
//**********************************************************************
// 函数功能：  窗口内部按键菜单处理
// 输入参数：  sysHandle:当前句柄
//			   message： 信息内容		
// 返回参数：  响应后的窗口句柄
static uint16 App_Remind_Win_Tag_Key(uint16 sysHandle,menuMessage message)
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
			switch(curSubWinRemindHandle)
			{
				case SUB_WIN_REMIND_NONE:
				sysHandleTemp = SysHandleBak;
				break;

				case SUB_WIN_REMIND_MESSAGE:
				case SUB_WIN_REMIND_MESSAGE_DETAIL:
				case SUB_WIN_REMIND_UASUAL_MSG_DETAIL:
				case SUB_WIN_REMIND_UASUAL_MSG:
				case SUB_WIN_REMIND_PHONE:
				case SUB_WIN_REMIND_PHONE_DETAIL:
				App_Remind_Win_MissCallDelete();
				App_Protocal_PhoneCallRet(callflow,0);
				if (subWinRemindHandle & SUB_WIN_REMIND_TAKE_PICTURE)
				{
					App_PicRamClear();
					subWinRemindHandle		|= SUB_WIN_REMIND_TAKE_PICTURE;
					curSubWinRemindHandle 	= SUB_WIN_REMIND_TAKE_PICTURE;
					App_TakepictureDisp();
					App_RoundDisp(ROUND_NONE,ROUND_VERTICAL_PIXEL);
				}
				else
				{
					sysHandleTemp = SysHandleBak;
					App_Systerm_RemindCancel();	
					curSubWinRemindHandle = SUB_WIN_REMIND_NONE;
				}		
				break;

				case SUB_WIN_REMIND_ALARM:
				Mid_AlarmClock_DelayRing_Close();
				if (subWinRemindHandle & SUB_WIN_REMIND_TAKE_PICTURE)
				{
					App_PicRamClear();
					subWinRemindHandle		|= SUB_WIN_REMIND_TAKE_PICTURE;
					curSubWinRemindHandle 	= SUB_WIN_REMIND_TAKE_PICTURE;
					App_TakepictureDisp();
					App_RoundDisp(ROUND_NONE,ROUND_VERTICAL_PIXEL);
				}
				else
				{
					sysHandleTemp = SysHandleBak;
					App_Systerm_RemindCancel();	
					curSubWinRemindHandle = SUB_WIN_REMIND_NONE;	
				}
				break;

				case SUB_WIN_REMIND_TAKE_PICTURE:
				App_Protocal_TakePhoto();
            	MultiModuleTask_EventSet(OP_MOTO);
				break;

				case SUB_WIN_REMIND_PAIRE:
				MultiModuleTask_EventSet(OP_MOTO);
				break;

				default:
				
				if (SysHandleBak == WIN_RUN_HANDLE || SysHandleBak == WIN_CLIMB_HANDLE ||SysHandleBak == WIN_SWING_HANDLE)
				{
					sysHandleTemp  			= SysHandleBak;
					// App_PicRamClear();
					// App_RoundDisp(ROUND_NONE,ROUND_VERTICAL_PIXEL);
				}
				else
				{
					sysHandleTemp  			= WIN_IDLE_HANDLE;
				}
				break;
			}
			break;

			case PRESS_S1:
			switch(curSubWinRemindHandle)
			{
				case SUB_WIN_REMIND_NONE:
				
				break;

				case SUB_WIN_REMIND_ALARM:
				Mid_AlarmClock_DelayRing_Close();
				sysHandleTemp = SysHandleBak;
				break;

				default:
				sysHandleTemp = SysHandleBak;
				break;
			}
			break;

			case PRESS_S2:
			switch(curSubWinRemindHandle)
			{
				case SUB_WIN_REMIND_NONE:
				
				break;

				case SUB_WIN_REMIND_ALARM:
				Mid_AlarmClock_DelayRing_Close();
				sysHandleTemp = SysHandleBak;
				break;

				default:
				sysHandleTemp = SysHandleBak;
				break;
			}
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
static uint16 App_Remind_Win_Tag_Action(uint16 sysHandle,menuMessage message)
{	
	if (virtualHandle 	!= Virtual_Handle_None)
	{
		switch(virtualHandle)
		{
			case Virtual_Handle_Take_Photo:
			/* 拍照处理代码 */
			break;

			case Virtual_Handle_Author:
			/* 授权处理代码 */
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
static uint16 App_Remind_Win_Tag_Gesture(uint16 sysHandle,menuMessage message)
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
static uint16 App_Remind_Win_Tag_PhoneApp(uint16 sysHandle,menuMessage message)
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
static uint16 App_Remind_Win_Tag_WinChange(uint16 sysHandle,menuMessage message)
{
	uint16 			sysHandleTemp;
	uint8 			u8Temp;

	winTimeCnt ++;
	sysHandleTemp 		= sysHandle;

	if (winTimeCnt >= WIN_TIME_8SEC)
	{
		switch(curSubWinRemindHandle)
		{
			case SUB_WIN_REMIND_BAT:
			if (SysHandleBak == WIN_RUN_HANDLE || SysHandleBak == WIN_CLIMB_HANDLE ||SysHandleBak == WIN_SWING_HANDLE)
			{
				sysHandleTemp  			= SysHandleBak;
			}
			else
			{
				sysHandleTemp  			= WIN_IDLE_HANDLE;
			}
			break;

			case SUB_WIN_REMIND_BLE:
			if (SysHandleBak == WIN_RUN_HANDLE || SysHandleBak == WIN_CLIMB_HANDLE ||SysHandleBak == WIN_SWING_HANDLE)
			{
				sysHandleTemp  			= SysHandleBak;
			}
			else
			{
				sysHandleTemp  			= WIN_IDLE_HANDLE;
			}
			break;

			case SUB_WIN_REMIND_MESSAGE:
			case SUB_WIN_REMIND_MESSAGE_DETAIL:
			case SUB_WIN_REMIND_UASUAL_MSG_DETAIL:
			case SUB_WIN_REMIND_UASUAL_MSG:
			if (subWinRemindHandle & SUB_WIN_REMIND_TAKE_PICTURE)
			{
				App_PicRamClear();
				subWinRemindHandle		|= SUB_WIN_REMIND_TAKE_PICTURE;
				curSubWinRemindHandle 	= SUB_WIN_REMIND_TAKE_PICTURE;
				App_TakepictureDisp();
				App_RoundDisp(ROUND_NONE,ROUND_VERTICAL_PIXEL);
			}
			else
			{
				if (SysHandleBak == WIN_RUN_HANDLE || SysHandleBak == WIN_CLIMB_HANDLE ||SysHandleBak == WIN_SWING_HANDLE)
				{
					sysHandleTemp  			= SysHandleBak;
				}
				else
				{
					sysHandleTemp  			= WIN_IDLE_HANDLE;
				}
			}
			break;

			case SUB_WIN_REMIND_PHONE:
			case SUB_WIN_REMIND_PHONE_DETAIL:
			App_Remind_Win_MissCallAdd();
			if (subWinRemindHandle & SUB_WIN_REMIND_TAKE_PICTURE)
			{
				App_PicRamClear();
				subWinRemindHandle		|= SUB_WIN_REMIND_TAKE_PICTURE;
				curSubWinRemindHandle 	= SUB_WIN_REMIND_TAKE_PICTURE;
				App_TakepictureDisp();
				App_RoundDisp(ROUND_NONE,ROUND_VERTICAL_PIXEL);
			}
			else
			{
				if (SysHandleBak == WIN_RUN_HANDLE || SysHandleBak == WIN_CLIMB_HANDLE ||SysHandleBak == WIN_SWING_HANDLE)
				{
					sysHandleTemp  			= SysHandleBak;
				}
				else
				{
					sysHandleTemp  			= WIN_IDLE_HANDLE;
				}
			}	
			break;

			case SUB_WIN_REMIND_ALARM:
			cycleFlag = 1;
			winTimeCnt = 0;
			break;

			case SUB_WIN_REMIND_SPORT_COMPLETE:
			if (SysHandleBak == WIN_RUN_HANDLE || SysHandleBak == WIN_CLIMB_HANDLE ||SysHandleBak == WIN_SWING_HANDLE)
			{
				sysHandleTemp  			= SysHandleBak;
			}
			else
			{
				sysHandleTemp  			= WIN_IDLE_HANDLE;
			}
			break;

			case SUB_WIN_REMIND_LONG_SIT:
			if (SysHandleBak == WIN_RUN_HANDLE || SysHandleBak == WIN_CLIMB_HANDLE ||SysHandleBak == WIN_SWING_HANDLE)
			{
				sysHandleTemp  			= SysHandleBak;
			}
			else
			{
				sysHandleTemp  			= WIN_IDLE_HANDLE;
			}
			break;

			case SUB_WIN_REMIND_TAKE_PICTURE:
			winTimeCnt = 0;
			break;

			default:
			sysHandleTemp  	   = WIN_IDLE_HANDLE;//SysHandleBak;
			break;
		}
		if (curSubWinRemindHandle != SUB_WIN_REMIND_ALARM && curSubWinRemindHandle != SUB_WIN_REMIND_TAKE_PICTURE)
		{
			App_Systerm_RemindCancel();
			App_PicRamClear();
			App_RoundDisp(ROUND_NONE,ROUND_VERTICAL_PIXEL);
			curSubWinRemindHandle = SUB_WIN_REMIND_NONE;	
		}
	}
	else
	{
		if (winTimeCnt % 4 == 0)
		{
			switch(curSubWinRemindHandle)
			{
				case SUB_WIN_REMIND_ALARM:
				if (cycleFlag)
				{	
					if (SysHandleBak == WIN_RUN_HANDLE || SysHandleBak == WIN_CLIMB_HANDLE ||SysHandleBak == WIN_SWING_HANDLE)
					{
						sysHandleTemp  			= SysHandleBak;
					}
					else
					{
						sysHandleTemp  	   = WIN_IDLE_HANDLE;//SysHandleBak;
						App_PicRamClear();
						App_RoundDisp(ROUND_NONE,ROUND_VERTICAL_PIXEL);
						// App_Systerm_RemindCancel();
					}
					curSubWinRemindHandle = SUB_WIN_REMIND_NONE;
				}
				break;

				case SUB_WIN_REMIND_MESSAGE_DETAIL:				
				if (cycleFlag)
				{
					// cycleFlag = 0;
					// if (detailRemindMsg.remindMsg.phonenumberlen > 0) //非联系人
					// {
					// 	AppRemindPhoneMsgDisp((uint8 *)detailRemindMsg.remindMsg.phonenumber,detailRemindMsg.remindMsg.phonenumberlen * 2, LETTER_TYPTE_UNICODE);
					// }
					// else
					// {
					// 	AppRemindPhoneMsgDisp((uint8 *)detailRemindMsg.remindMsg.name,detailRemindMsg.remindMsg.namelen * 2, LETTER_TYPTE_UNICODE);
					// }
				}
				else
				{
					App_PicRamClear();
 					cycleFlag = 1;
					 AppRemindMsgContentDisp((uint8 *)detailRemindMsg.remindMsg.content.u.detail,detailRemindMsg.remindMsg.content.u.detaillen * 2, LETTER_TYPTE_UNICODE);					
					App_RoundDisp(ROUND_UP,ROUND_VERTICAL_PIXEL);
				}	
				break;

				case SUB_WIN_REMIND_UASUAL_MSG_DETAIL:				
				if (cycleFlag)
				{
					//cycleFlag = 0;
					//AppRemindAppMsgDisp((uint8 *)detailRemindMsg.remindMsg.name,detailRemindMsg.remindMsg.namelen * 2, LETTER_TYPTE_UNICODE);
				}
				else
				{
					App_PicRamClear();
					cycleFlag = 1;
					AppRemindMsgContentDisp((uint8 *)detailRemindMsg.remindMsg.content.u.detail,detailRemindMsg.remindMsg.content.u.detaillen * 2, LETTER_TYPTE_UNICODE);
					App_RoundDisp(ROUND_UP,ROUND_VERTICAL_PIXEL);
				}
				
				break;

				case SUB_WIN_REMIND_PHONE_DETAIL:
				
				break; 

				case SUB_WIN_REMIND_TAKE_PICTURE:
				winTimeCnt = 0;
				break;

				case SUB_WIN_REMIND_PAIRE:
				winTimeCnt = 0;
				break;
			}	
		}	
	}
	if (curSubWinRemindHandle == ALARM_REMIND && winTimeCnt % WIN_TIME_5SEC == 0)
	{
		u8Temp = Mid_AlarmClock_CurRingAlarmIdReturn();
		App_Protocal_AlarmRing(u8Temp);
	}

	return sysHandleTemp;
}
