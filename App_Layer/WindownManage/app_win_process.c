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
#include "app_win_pwron.h"
#include "app_win_press.h"
#include "app_win_bodytemp.h"
#include "app_win_abc.h"


static QueueHandle_t 	Win_QueueHandle;				/**< Queue handler for flash task */
#define		WIN_TASK_QUEUE_LENGTH			3
#define		WIN_TASK_QUEUE_SIZE			sizeof(App_Win_Msg_T) 	


#define AppWinNum (sizeof(AppWin)/sizeof(AppWin[0]))		// 窗口数量
static 	APP_WIN_T	AppWin[] =
		{
			// 创建顺序无须与eAppWinHandle中的参数对应，函数调用前会先搜索索引
			{ eLockWinHandle, App_LockWin_Init, App_LockWin_Cb},
			{ eStoreWinHandle, App_StoreWin_Init, App_StoreWin_Cb},
			{ ePwronWinHandle, App_PwronWin_Init, App_PwronWin_Cb},
			{ eTimeWinHandle, App_TimeWin_Init, App_TimeWin_Cb},
			{ eSportWinHandle, App_SportWin_Init, App_SportWin_Cb},
			{ eHeartWinHandle, App_HeartWin_Init, App_HeartWin_Cb},
			{ ePressWinHandle, App_PressWin_Init, App_PressWin_Cb},
			{ eBodyTempWinHandle, App_BodyTempWin_Init, App_BodyTempWin_Cb},
			{ eSleepWinHandle, App_SleepWin_Init, App_SleepWin_Cb},
			{ eABCWinHandle, App_ABCWin_Init, App_ABCWin_Cb},
			{ eHistoryWinHandle, App_HistoryWin_Init, App_HistoryWin_Cb},
		};

// 窗口句柄参数
App_Win_Param_T	AppWinParam = 
	{
		.CurrWinHanle = eInvalidWinHandle,
		.CurrSubWinHandle = eAppSubWinHandleNone,
		.LockWinCnt = 0,
	};	


void App_Window_LockWinCnt(void)
{
	AppWinParam.LockWinCnt++;
	
//	APP_WIN_RTT_LOG(0,"LockWinCnt %d \n",AppWinParam.LockWinCnt);
	
	// 锁屏到时，发送锁屏事件
	if((AppWinParam.LockWinCnt > APP_WIN_LOCK_TIMEOUT) && (eLockWinHandle != AppWinParam.CurrWinHanle))
	{
		App_Win_Msg_T WinMsg;
		WinMsg.MenuTag = eWinMenuLock;
		WinMsg.val = 0;
		App_Win_TaskEventSet(&WinMsg);		
	}
}
	
//**********************************************************************
// 函数功能：  窗口初始化
// 输入参数：  需要创建的窗口句柄	
// 返回参数：  成功创建的窗口句柄
eAppWinHandle App_Window_Init(eAppWinHandle NewWinHandle)
{
	uint32_t TmpWinIndex;
	
	// 窗口参数有效性检验
	for(uint32_t i = 0;i < AppWinNum;i++)
	{
		if((NULL == AppWin[i].WinInit) || (NULL == AppWin[i].CallBack))
		{
			return eErrWinHandle;
		}
	}
	
	// 需创建的窗口有效性校验
	if(NewWinHandle >= eMaxWinHandle)
		return eErrWinHandle;
	
	// 搜寻句柄索引号
	for(TmpWinIndex = 0;TmpWinIndex < AppWinNum;TmpWinIndex++)
	{
		if(NewWinHandle == AppWin[TmpWinIndex].HandleIndex)
			break;
	}	
	
	// 创建当前窗口
	AppWinParam.CurrWinHanle = AppWin[TmpWinIndex].WinInit();
	
	return AppWinParam.CurrWinHanle;
}


//**********************************************************************
// 函数功能：  窗口处理函数，通过此函数操作窗口各菜单（子类），在回调中实现功能
// 输入参数：  sysHandle ： 当前窗口句柄号
// 			   tagMenu： 	操作的菜单
// 			   message：	菜单操作发送的信息：状态、结果等

// 返回参数：  无
eAppWinHandle App_Window_Process(App_Win_Msg_T message)
{
	uint32_t TmpWinIndex;				// 句柄索引号
	eAppWinHandle TmpWinHandle;			// 保存函数调用返回的句柄
	
	APP_WIN_RTT_LOG(0,"Before WinHanle %d %d \n",AppWinParam.CurrWinHanle, AppWinParam.CurrSubWinHandle);
	
	// 搜寻当前窗口句柄索引号
	for(TmpWinIndex = 0;TmpWinIndex < AppWinNum;TmpWinIndex++)
	{
		if(AppWinParam.CurrWinHanle == AppWin[TmpWinIndex].HandleIndex)
			break;
	}
	
	/* 注：(TmpWinIndex < AppWinNum)判断不可少，否则可能会有越界产生 */
	if((TmpWinIndex < AppWinNum) && (NULL != AppWin[TmpWinIndex].WinInit) && (NULL != AppWin[TmpWinIndex].CallBack))
	{
		// 窗口句柄索引回调函数处理
		TmpWinHandle = AppWinParam.CurrWinHanle;
		AppWin[TmpWinIndex].CallBack(AppWinParam.CurrWinHanle, message);
		
		// 若窗口句柄改变，需切换到新窗口
		if(TmpWinHandle != AppWinParam.CurrWinHanle)
		{
			// 搜寻需切换到的窗口句柄索引号
			for(TmpWinIndex = 0;TmpWinIndex < AppWinNum;TmpWinIndex++)
			{
				if(AppWinParam.CurrWinHanle == AppWin[TmpWinIndex].HandleIndex)
					break;
			}
			
			/* 注：(TmpWinIndex < AppWinNum)判断不可少，否则可能会有越界产生 */
			if((TmpWinIndex < AppWinNum) && (NULL != AppWin[TmpWinIndex].WinInit) && (NULL != AppWin[TmpWinIndex].CallBack))
			{
				// 新窗口初始化
				AppWinParam.CurrWinHanle = AppWin[TmpWinIndex].WinInit();
			}
		}
	}
	
	// 处理完后，打印当前窗口句柄
	APP_WIN_RTT_LOG(0,"After WinHanle %d %d \n",AppWinParam.CurrWinHanle, AppWinParam.CurrSubWinHandle);
	
	// 如果是按键点击屏幕等，重置锁屏计数
	if( eWinMenukey == message.MenuTag || 
	    eWinMenuSlide == message.MenuTag || 
		eWinMenuClick == message.MenuTag )
	{
		AppWinParam.LockWinCnt = 0;
	}
	
	return AppWinParam.CurrWinHanle;
}



void App_Win_TaskEventSet(App_Win_Msg_T* msg)
{
	if(pdTRUE != xQueueSendToBack(Win_QueueHandle, msg, 10))
	{
		APP_WIN_RTT_ERR(0,"MotorTask_EventSet Err \n");
	}
}

static void App_Win_TaskProcess(void *pvParameters)
{
	App_Win_Msg_T	Msg;
	
	Win_QueueHandle = xQueueCreate(WIN_TASK_QUEUE_LENGTH, WIN_TASK_QUEUE_SIZE);
	if(Win_QueueHandle == NULL)
	{
		APP_WIN_RTT_ERR(0,"WIN Queue Create Err \r\n");
	}
	
	APP_WIN_RTT_LOG(0,"App_Win_TaskCreate Suc \r\n");
	
	while(1)
	{
		if(xQueueReceive(Win_QueueHandle, &Msg, portMAX_DELAY))
		{
			APP_WIN_RTT_LOG(0,"AppWin Msg %02X %02X ---------------------\r\n",Msg.MenuTag, Msg.val);
			
			if(eErrWinHandle == App_Window_Process(Msg))
			{
				APP_WIN_RTT_ERR(0,"App_Window_Process Err \r\n");
			}
		}
	}
}

void App_Win_TaskCreate(void)
{
    xTaskCreate(App_Win_TaskProcess, "App_Win_Task", TASK_STACKDEPTH_APP_WIN, NULL, TASK_PRIORITY_APP_WIN, NULL);
}












