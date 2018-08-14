#include "platform_common.h"
#include "rtos.h"
#include "App_win_common.h"
#include "App_win_heart.h"
#include "App_win_remind.h"
#include "App_win_sleep.h"
#include "App_win_sport.h"
#include "App_win_time.h"
#include "App_win_history.h"
#include "app_win_alarm.h"
#include "app_win_swing.h"
#include "app_win_run.h"
#include "app_win_climb.h"
#include "app_win_setting.h"
#include "app_win_idle.h"
#include "app_win_store.h"
#include "app_win_factory.h"
#include "App_win_process.h"





/*************** func declaration ********************************/




/*************** macro define ********************************/
#define 		SYS_HANDLE_MAX 		13 			//窗口句柄最大个数




/************** variable define *****************************/
static 	SysWin	SysWindow[SYS_HANDLE_MAX] =
		{
			{ WIN_TIME_HANDLE, 	 	App_Time_Win_Init,    	App_Time_Win_CallBack	},
			{ WIN_SPORT_HANDLE,	 	App_Sport_Win_Init, 	App_Sport_Win_CallBack 	},
			{ WIN_HEART_HANDLE,  	App_Heart_Win_Init, 	App_Heart_Win_CallBack 	},
			{ WIN_SLEEP_HANDLE,  	App_Sleep_Win_Init, 	App_Sleep_Win_CallBack 	},
			{ WIN_REMIND_HANDLE, 	App_Remind_Win_Init, 	App_Remind_Win_CallBack	},
			{ WIN_HISTORY_HANDLE, 	App_History_Win_Init,	App_History_Win_CallBack},
			{ WIN_SETTING_HANDLE,   App_Setting_Win_Init,   App_Setting_Win_CallBack},
			{ WIN_ALARM_HANDLE, 	App_Alarm_Win_Init, 	App_Alarm_Win_CallBack},
			{ WIN_RUN_HANDLE, 	 	App_Run_Win_Init, 	 	App_Run_Win_CallBack},	
			{ WIN_CLIMB_HANDLE, 	 App_Climb_Win_Init, 	App_Climb_Win_CallBack},
			{ WIN_SWING_HANDLE, 	 App_Swing_Win_Init, 	App_Swing_Win_CallBack},
			{ WIN_STORE_HANDLE, 	 App_Store_Win_Init, 	App_Store_Win_CallBack},
			{ WIN_IDLE_HANDLE, 	 	App_Idle_Win_Init, 	    App_Idle_Win_CallBack},				
		};

static 	uint8 SumWinNum			= 0;
static 	uint16 CurrentSysHandle 	= 0;		
static  SemaphoreHandle_t    WinProcessMutex;

/************** function define ******`***********************/

//**********************************************************************
// 函数功能：  窗口初始化
// 输入参数：   需要创建的窗口句柄	
// 返回参数：  成功创建的窗口的句柄
uint16 App_Window_Create(SysWinHandle sysHandleNew)
{
	uint16 i;
	uint16 sysHandleIndex;

	SumWinNum			= 0;
	CurrentSysHandle 	= WIN_INVALID_HANDLE;
	virtualHandle 		= Virtual_Handle_None;

	for (i = 0; i < SYS_HANDLE_MAX; i++)
	{
		if (SysWindow[i].WinInit != NULL && SysWindow[i].CallBack != NULL)
		{
			SumWinNum ++;			
		}

		if (SysWindow[i].HandleIndex == sysHandleNew)
		{
			sysHandleIndex = i;
		}
	}

	if (SumWinNum == 0)
	{
		CurrentSysHandle 	= WIN_INVALID_HANDLE;
	}
	else if (sysHandleIndex < SYS_HANDLE_MAX)
	{			
		CurrentSysHandle 	= SysWindow[sysHandleIndex].WinInit();//窗口初始化	
	}

	// 创建二值量,资源互斥保护
	WinProcessMutex = xSemaphoreCreateMutex();//追寻互斥信号量
	
	return CurrentSysHandle;
}

//**********************************************************************
// 函数功能：  窗口句柄注册函数，新增窗口（界面）时，需要新增注册句柄
// 输入参数：   SysHandleNew ：  新增窗口的句柄号
//				SysCallBack：    新增窗口的菜单操作函数的回调		

// 返回参数：  0:注册成功，0xff注册失败
uint16 App_Window_Register(uint16 SysHandleNew,WinInitFunc *InitFunc,CallBackFunc *SysCallBack)
{
	xSemaphoreTake(WinProcessMutex, portMAX_DELAY);

	if (SysHandleNew >= SYS_HANDLE_MAX)
	{
		return CurrentSysHandle;
	}

	SysWindow[SumWinNum].HandleIndex 	= SysHandleNew;
	SysWindow[SumWinNum].WinInit 		= InitFunc;
	SysWindow[SumWinNum].CallBack 		= SysCallBack;
	SumWinNum ++;

	xSemaphoreGive(WinProcessMutex);//释放互斥信号量（归还）
	return SysHandleNew;
}


//**********************************************************************
// 函数功能：  窗口处理函数，通过此函数操作窗口各菜单（子类），在回调中实现功能
// 输入参数：  sysHandle ： 当前窗口句柄号
// 			   tagMenu： 	操作的菜单
// 			   message：	菜单操作发送的信息：状态、结果等

// 返回参数：  无
uint16 App_Window_Process(uint16 sysHandle,MenuTag tagMenu,menuMessage message)
{
	uint8 i;
	uint8 sysHandleTemp;

	xSemaphoreTake(WinProcessMutex, portMAX_DELAY);

	if ((CurrentSysHandle != sysHandle) ||(sysHandle >= SumWinNum))
	{
		xSemaphoreGive(WinProcessMutex);//释放互斥信号量（归还）
		return CurrentSysHandle;
	}

	for (i = 0; i < SumWinNum; i++)
	{
		if (SysWindow[i].HandleIndex == sysHandle)
		{
			break;
		}
	}
	if (i < SumWinNum && SysWindow[i].WinInit != NULL && SysWindow[i].CallBack != NULL)
	{
		sysHandleTemp 		= sysHandle;
		CurrentSysHandle 	= SysWindow[i].CallBack(sysHandleTemp,tagMenu,message);

		if (sysHandleTemp != CurrentSysHandle)//窗口有改变，需切换到新窗口
		{
			CurrentSysHandle 	= SysWindow[CurrentSysHandle].WinInit();//窗口初始化
		}
	}
	else
	{
		CurrentSysHandle 	= sysHandle;
	}
	
	xSemaphoreGive(WinProcessMutex);//释放互斥信号量（归还）
	return CurrentSysHandle;
}


