#ifndef         APP_VARIABLE_H
#define         APP_VARIABLE_H

#include "platform_common.h"
#include "mid_interface.h"

#include "main.h"


typedef enum
{
    BLE_SLEEP,               //蓝牙睡眠状态,关广播
    BLE_BROADCAST,           //蓝牙广播状态,开广播
    BLE_CONNECT,             //蓝牙连接状态
    BLE_POWERON,             //蓝牙上电
    BLE_POWEROFF,            //蓝牙掉电
}ble_state_t;

typedef enum 
{
    PHONE_STATE_NORMAL = 0,
    PHONE_STATE_PHOTO,
    PHONE_STATE_AUTHOR,
    PHONE_STATE_PAIRE,
    PHONE_STATE_HRM,
}phne_state_t;

typedef enum
{
    STEP_IDLE_STATE = 0,
    STEP_PREPAIRE_STATE,
    STEP_PROCESS_STATE,
}step_state_t;


// APP的消息通知
typedef enum
{
    APP_REMIND_NONE                 = 0x00000000, //没有通知消息
    APP_REMIND_QQ                   = 0x00000001, //QQ通知消息
    APP_REMIND_WECHAT               = 0x00000002, //微信通知消息
    APP_REMIND_TXWEIBO              = 0x00000004, //腾讯微博通知消息
    APP_REMIND_SKYPE                = 0x00000008, //Skype通知消息
    APP_REMIND_XLWEIBO              = 0x00000010, //新浪微博通知消息
    APP_REMIND_FACEBOOK             = 0x00000020, //facebook通知消息
    APP_REMIND_TWITTER              = 0x00000040, //twitter通知消息
    APP_REMIND_WHATAPP              = 0x00000080, //whatapp通知消息
    APP_REMIND_LINE                 = 0x00000100, //line通知消息
    APP_REMIND_OTHER1               = 0x00000200, //其他1通知消息
    APP_REMIND_CALL                 = 0x00000400, //来电通知消息
    APP_REMIND_MSG                  = 0x00000800, //短信通知消息
    APP_REMIND_MISSCALL             = 0x00001000, //未接来电通知消息
    APP_REMIND_CALENDARMSG          = 0x00002000, //日历信息通知消息
    APP_REMIND_EMAIL                = 0x00004000, //email通知消息
    APP_REMIND_OTHER2               = 0x00008000, //其他2通知消息
    APP_REMIND_PERSON               = 0x00010000,
    APP_REMIND_TIM                  = 0x00020000, //TIM
    // APP_REMIND_ALL                  = 0xffffffff, 
}app_remind_type;


typedef enum 
{
    SWITCH_OFF = 0,
    SWITCH_ON  = 1,
}system_config_t;

typedef enum
{
    SYS_ENGLISH_TYPE    = 0,
    SYS_CHINESE_TYPE,   
}systerm_languge_t;

typedef enum
{
    SYS_TIME_24_TYPE    = 0,
    SYS_TIME_12_TYPE,   
}systerm_timetype_t;


typedef enum 
{
    MOVT_STATE_NORMAL  = 0,
    MOVT_STATE_STOP,
    MOVT_STATE_ADJUST,
}movt_state_t;

typedef enum 
{
    APP_HRM_OFF_TOUCH  = 0,
    APP_HRM_ON_TOUCH  = 1,
}hrm_state_t;


typedef enum 
{
    SCENE_NULL      = 0x00,
    SCENE_RUNNING   = 0x01,
    SCENE_SWING     = 0x02,
    SCENE_CLIMB     = 0x04,
}scene_switch_t;

extern uint8  sceneSwitch;
// *************************************************************
//功能开关配置
typedef struct 
{
    //计步 开关
    uint8_t     stepCountSwitch;
    //心率 开关
    uint8_t     heartrateSwitch;
     //天气 开关
    uint8_t     weatherSwitch;
    //运动完成度开关
    uint8_t     stepCompleteRemindSwitch;
    //蓝牙断开连接是否提醒的开关
    uint8_t     bleDiscRemindSwitch;
    //久坐提醒的开关
    uint8_t     longSitRemindSwitch;
    //勿扰开关
    uint8_t     notDisturbSwitch;
    //手势开关
    uint8       gestureSwitch;
    //app提醒开关
    uint32      appRemindSwitch;
    //ＡＰＰ详情提醒开关
    uint32      appDetailRemindSwitch;
    //系统语言
    uint8       systermLanguge;
    //系统时间制式
    uint8       systermTimeType;
}sys_config_s;

typedef struct 
{
    uint8_t StartTimeHour;
    uint8_t StartTimeMin;
    uint8_t StopTimeHour;
    uint8_t StopTimeMin;
    uint8_t DisturdStartTimehour;
    uint8_t DisturdStartTimeMin;
    uint8_t DisturdStopTimehour;
    uint8_t DisturdStopTimeMin;      
    uint16_t intv_mimute;
}longsit_s;


typedef struct 
{
    uint8_t StartHour;
    uint8_t StartMin;
    uint8_t StopHour;
    uint8_t StopMin;
}not_disturd_s;

typedef struct 
{
    // 第二城市时间RTC
    rtc_time_s   rtc;
    uint16       cityCode;
}worldrtc_s;

//看门狗监测结构体
typedef struct 
{
    uint8_t     lastCnt;
    uint8_t     curCnt;
    uint8_t     taskId;
    uint8_t     taskWdtState;   
}watchdog_s;

//手机状态计步结构体
typedef struct 
{
    uint8 state;
    uint8 timeCnt;
    uint8 timeMax;
}phoneState_s;

//手机状态计步结构体
typedef struct 
{
    uint8 state;
    uint8 timeCnt;
    uint8 timeMax;
}movtState_s;


extern phoneState_s    phoneState;
extern movtState_s     movtState;

extern watchdog_s taskWatchdog;
// ***********************系统配置**************************************
extern sys_config_s    systermConfig;

// 蓝牙状态
extern uint8 bleState;

// 第二城市时间RTC
extern worldrtc_s  WorldRtc;

// *************************************************************
//手机提醒
extern uint32      appRemindState;

//计步状态
extern uint8       stepState;

extern uint8       resetStatus;

//后台计数
extern uint32 backstageTimeCounterSec;
extern uint32 backstageTimeCounterMin;

extern uint16     WinDowHandle;

// *************************************************************
// 久坐
extern longsit_s appLongSitInfo;

// *************************************************************
// 勿扰
extern not_disturd_s appNotDisturdTimeInfo;

// *************************************************************
// multimodule port drive

// *************************************************************
//extern const multimodule_task_msg_t NULL_RED_LED;
//extern const multimodule_task_msg_t NULL_GREEN_LED;
//extern const multimodule_task_msg_t NULL_MOTO;
//extern const multimodule_task_msg_t BAT_FULL_LED;
////operate 
//extern const multimodule_task_msg_t OP_MOTO;
//// power on 

//extern const multimodule_task_msg_t POWERON_MOTO;

// *************************************************************
// bat 
//extern const multimodule_task_msg_t TAKE_PICTURE_REMIND_MOTO;


//extern const multimodule_task_msg_t BAT_LOW_MOTO;
//// *************************************************************
//// BLE operation reflect

//extern const multimodule_task_msg_t OPEN_BLE_MOTO;


//extern const multimodule_task_msg_t CLOSE_BLE_MOTO;


//extern const multimodule_task_msg_t BLE_CLOSE_MOTO;
//extern const multimodule_task_msg_t BLE_BROCAST_MOTO;
//extern const multimodule_task_msg_t BLE_CONNECT_MOTO;
//extern const multimodule_task_msg_t BLE_DISCONNECT_MOTO;

// *************************************************************
// author
//extern const multimodule_task_msg_t AUTHOR_MOTO;
////pair
//extern const multimodule_task_msg_t PAIRE_MOTO;

//extern const multimodule_task_msg_t CALL_REMIND_MOTO;

//extern const multimodule_task_msg_t CALL_REJECT_MOTO;


//extern const multimodule_task_msg_t MSG_REMIND_MOTO;



//extern const multimodule_task_msg_t FIND_PHONE_MOTO;

//extern const multimodule_task_msg_t ACTIION_MOTO;

////remind
//extern const multimodule_task_msg_t APP_EXPMSG_MOTO;

//extern const multimodule_task_msg_t APP_MSG_MOTO;
////alarm
//extern const multimodule_task_msg_t ALARM_MOTO;

//extern const multimodule_task_msg_t SPORT_COMPLETE_MOTO;
////warn
//extern const multimodule_task_msg_t LONG_SIT_MOTO;

////factory mode 
//extern const multimodule_task_msg_t FACTORY_MOTO;

//extern const multimodule_task_msg_t FACTORY_S1_MOTO;
//extern const multimodule_task_msg_t POWER_LED;

//extern const multimodule_task_msg_t SAVE_FULL_MOTO;


#endif          //APP_VARIABLE_H
