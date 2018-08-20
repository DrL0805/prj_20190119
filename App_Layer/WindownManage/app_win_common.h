#ifndef APP_WIN_COMMON_H
#define APP_WIN_COMMON_H

#include "platform_common.h"

//#include "app_win_idle.h"
//#include "app_win_store.h"
//#include "app_win_pwron.h"
//#include "app_win_time.h"
//#include "app_win_sport.h"
//#include "app_win_heart.h"
//#include "app_win_press.h"
//#include "app_win_bodytemp.h"
//#include "app_win_sleep.h"
//#include "app_win_abc.h"
//#include "app_win_history.h"

#include "mid_key.h"

#define APP_WIN_RTT_DEBUG	3
#if (1 == APP_WIN_RTT_DEBUG)	// 错误等级
#define APP_WIN_RTT_LOG(...)
#define APP_WIN_RTT_WARN(...)
#define APP_WIN_RTT_ERR		SEGGER_RTT_printf
#elif (2 == APP_WIN_RTT_DEBUG)	// 警告等级
#define APP_WIN_RTT_LOG(...)
#define APP_WIN_RTT_WARN	SEGGER_RTT_printf
#define APP_WIN_RTT_ERR		SEGGER_RTT_printf
#elif (3 == APP_WIN_RTT_DEBUG)	// 调试等级
#define APP_WIN_RTT_LOG		SEGGER_RTT_printf
#define APP_WIN_RTT_WARN	SEGGER_RTT_printf
#define APP_WIN_RTT_ERR		SEGGER_RTT_printf
#else							// 调试关闭
#define APP_WIN_RTT_LOG(...)
#define APP_WIN_RTT_WARN(...)
#define APP_WIN_RTT_ERR(...)
#endif


//菜单类型
typedef enum 
{
	eWinMenukey,
	eWinMenuPwrOn,			// 开机完成，仅用于开机窗口
	eWinMenuSlide,			// Val 0~3：上、下、左、右
	eWinMenuClick,			// 点击
	eWinMenuLock,			// 锁屏
	eWinMenuMax,
	
//	TAG_TIME 				= 0x0000,  //时间相关:半秒、秒、
//	TAG_SPORT 				= 0x0010,//运动相关
//	TAG_HEART 				= 0x0020,//心率相关
//	TAG_WEATHER 			= 0x0030,
//	TAG_SLEEP 	 			= 0x0040,

//	TAG_REMIND 				= 0x0050,//提醒相关:电池、蓝牙、闹钟、手机来电等提醒

//	TAG_KEY 				= 0x0060,//动作相关
//	TAG_ACTION 				= 0x0061,//甩手动作
//	TAG_GESTURE				= 0x0062,//手势 

//	TAG_GPS 				= 0x0070,//位置相关
//	TAG_COMPASS 			= 0x0071,

//	TAG_WIN_CHANGE 			= 0x0080,//窗口切换
//	TAG_ICON_FLICK 			= 0x0090,//图标闪烁

//	TAG_PHONE_APP 			= 0xF000,//手机APP触发的TAG如果是不改变窗口的需特别处理，目前为拍照、授权两个
}eWinMenuTag;

// 二级窗口句柄类型
typedef enum
{
	eAppSubWinHandle0,
	eAppSubWinHandle1,
	eAppSubWinHandle2,
	eAppSubWinHandle3,
	eAppSubWinHandle4,
	eAppSubWinHandle5,
	eAppSubWinHandle6,
	eAppSubWinHandle7,
	eAppSubWinHandleNone,
}eAppSubWinHandle;

// 主窗口句柄类型
typedef enum 
{
	eLockWinHandle,
	eStoreWinHandle,		// 仓储	
	ePwronWinHandle,
	eTimeWinHandle,
	eSportWinHandle,
	eHeartWinHandle,
	ePressWinHandle,
	eBodyTempWinHandle,
	eSleepWinHandle,
	eABCWinHandle,
	eHistoryWinHandle,
	eMaxWinHandle,			// 窗口句柄数量，这个值必须准确
	eInvalidWinHandle,		// 无效窗口句柄
	eErrWinHandle,			// 错误的窗口句柄
}eAppWinHandle;

//菜单传递信息结构体
typedef struct 
{
	eWinMenuTag		MenuTag;		//菜单标识	
	uint32 			val;	//数值：如键值、数值、统计数量等
	uint8 			state;	//状态：如电池充放电状态、蓝牙状态、手机状态等
	uint8 			op;		//操作：如APP的授权拍照、甩手动作、手势、提醒等
}App_Win_Msg_T;

//窗口内菜单信息结构体
typedef struct 
{
	eWinMenuTag		MenuTag;		//菜单标识	
	#if 0
	uint16 		axisX;		//菜单坐标
	uint16 		axisY;		//菜单坐标
	#endif
	eAppWinHandle	(*callback)(eAppWinHandle WinHandle,App_Win_Msg_T message);	//菜单回调处理函数
}App_Win_Menu_T;



//虚拟窗口句柄类型
typedef enum
{
	Virtual_Handle_None 		= 0,
	Virtual_Handle_Take_Photo 	= 1,
	Virtual_Handle_Author 		= 2,
}VirtualHanleList;



//键值
typedef enum 
{
	PRESS_S0 				= 0x00000000, //按键相关
	PRESS_S1 				= 0x00000001,
	PRESS_S2 				= 0x00000002,
	HOLD_SHORT_S0 			= 0x00000010,
	HOLD_SHORT_S1 			= 0x00000011,
	HOLD_SHORT_S2 			= 0x00000012,
	HOLD_LONG_S0 			= 0x00000020,
	HOLD_LONG_S1 			= 0x00000021,
	HOLD_LONG_S2 			= 0x00000022,
	DOUBLE_PRESS_S0 		= 0x00000030,
	DOUBLE_PRESS_S1 		= 0x00000031,		
	DOUBLE_PRESS_S2 		= 0x00000032,	
}TagValue;

//动作
typedef enum 
{
	OP_NONE  					= 0x00,		
//TAG_TIME
	RTC_HALF_SEC,
	RTC_SEC,
	RTC_MIN,
//TAG_GESTURE
	GESTURE_ACTION_FORWARD,	  //翻腕-前
	GESTURE_ACTION_BACKWARD,  //翻腕-后
	GESTURE_ACTION_LEFT,	  //翻腕-左
	GESTURE_ACTION_RIGHT,	  //翻腕-右
	GESTURE_ACTION_HAND_LIFT, //抬起手
	GESTURE_ACTION_HAND_DOWN,	//放下手

//TAG_REMIND
	BAT_REMIND,
	BLE_REMIND,					//0x0b
	SPORT_COMPLETE_REMIND,
	LONG_SIT_REMIND,

	MESSAGE_REMIND,
	UASUAL_MSG_REMIND,
	PHONE_CALL_REMIND,
	PHONE_MISS_CALL,

	MESSAGE_DETAIL_REMIND,
	UASUAL_MSG_DETAIL_REMIND,
	PHONE_CALL_DETAIL_REMIND,
	PHONE_MISS_CALL_DETAIL,

	ALARM_REMIND,
	STORE_REMIND,
	OTA_REMIND,

	TAKE_PHOTO_REMIND,
	AUTHOR_REMIND,

	PAIRE_REMIND,

	SAVE_REMIND,	//数据保存完成提醒

}TagOp;

//状态
typedef enum 
{
	CANCEL_REMIND_STATE      = 0x00,//0
    NEW_REMIND_STATE,			//1

	BAT_NORMAL_STATE,
	BAT_LOW_VOLTAGE_STATE,
	BAT_CHARGING_STATE,
	BAT_DISCHARGE_STATE,
	BAT_FULL_STATE,

	BLE_DICONNECT_STATE,

	ENTER_PAIRE_STATE,
	EXIT_PAIRE_STATE,

	//TAG_PHONE_APP
	ENTER_TAKE_PHOTO_MODE,    //拍照动作
	EXIT_TAKE_PHOTO_MODE,
	
	ENTER_AUTHOR_MODE,       //授权动作
	EXIT_AUTHOR_MODE,

	STORE_ENTER,
	STORE_EXIT,

	OTA_ENTER,
	OTA_EXIT,

	SAVE_SUCCESS_STATE,
	SAVE_FAIL_STATE,
	SAVE_SPACE_FULL_STATE,
}TagState;

typedef enum 
{
	SAVE_NULL 	 = 0,
	SAVING_STATE,
	SAVE_OK,
}save_state_t;


extern uint8 	virtualHandle;
extern uint32 	winTimeCnt;
extern uint8 	cycleFlag;
extern uint8 	saveState;

#endif

