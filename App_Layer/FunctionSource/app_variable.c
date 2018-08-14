#include "platform_common.h"
#include "multimodule_task.h"
#include "app_variable.h"
#include "app_protocal.h"

#define SINGLE_PORT_1SEC 32

// *************************************************************
// Null 
const multimodule_task_msg_t NULL_RED_LED =
{
    SINGLE_PORT_ID,
    0,
    1,
    1,
    RED_LED,
};

const multimodule_task_msg_t NULL_GREEN_LED =
{
    SINGLE_PORT_ID,
    0,
    1,
    1,
    GREEN_LED,
};

const multimodule_task_msg_t NULL_MOTO =
{
    SINGLE_PORT_ID,
    0,
    1,
    1,
    // GREEN_LED,
    MOTO,
};

// *************************************************************
const multimodule_task_msg_t BAT_FULL_LED =
{
    SINGLE_PORT_ID,
    SINGLE_PORT_1SEC,
    0,
    1,
    GREEN_LED,
};

// *************************************************************
const multimodule_task_msg_t POWER_LED =
{
    SINGLE_PORT_ID,
    SINGLE_PORT_1SEC /2,
    0,
    1,
    RED_LED,
};

const multimodule_task_msg_t OP_MOTO =
{
    SINGLE_PORT_ID,
    SINGLE_PORT_1SEC/4,
    0,
    1,
    MOTO,
};

// power on 
const multimodule_task_msg_t POWERON_MOTO =
{
    SINGLE_PORT_ID,
    16,
    1,
    1,
    MOTO,
};

// *************************************************************
// enter work 
const multimodule_task_msg_t TAKE_PICTURE_REMIND_MOTO =
{
    SINGLE_PORT_ID,
    SINGLE_PORT_1SEC / 4,
    SINGLE_PORT_1SEC * 3 /4,
    1,
    MOTO,
};

const multimodule_task_msg_t FIND_PHONE_MOTO =
{
    SINGLE_PORT_ID,
    SINGLE_PORT_1SEC /4,
    SINGLE_PORT_1SEC * 3/4,
    1,
    MOTO,
};


const multimodule_task_msg_t BAT_LOW_MOTO =
{
    SINGLE_PORT_ID,
    SINGLE_PORT_1SEC /8,
    SINGLE_PORT_1SEC / 8,
    4,
    MOTO,
};

// *************************************************************
// BLE operation reflect
const multimodule_task_msg_t OPEN_BLE_MOTO =
{
    SINGLE_PORT_ID,
    8,
    8,
    3,
    MOTO,
};

const multimodule_task_msg_t CLOSE_BLE_MOTO =
{
    SINGLE_PORT_ID,
    16,
    0,
    1,
    // GREEN_LED,
    MOTO,
};


const multimodule_task_msg_t BLE_CLOSE_MOTO =
{
    SINGLE_PORT_ID,
    16,
    0,
    1,
    MOTO,
};
const multimodule_task_msg_t BLE_BROCAST_MOTO =
{
    SINGLE_PORT_ID,
    8,
    8,
    3,
    MOTO,
};

const multimodule_task_msg_t BLE_CONNECT_MOTO =
{
    SINGLE_PORT_ID,
    16,
    0,
    1,
    MOTO,
};

const multimodule_task_msg_t BLE_DISCONNECT_MOTO =
{
    SINGLE_PORT_ID,
    SINGLE_PORT_1SEC / 4,
    SINGLE_PORT_1SEC / 4,
    4,
    MOTO,
};

const multimodule_task_msg_t AUTHOR_MOTO =
{
    SINGLE_PORT_ID,
    SINGLE_PORT_1SEC / 4,
    SINGLE_PORT_1SEC / 4,
    2,
    MOTO,
};

const multimodule_task_msg_t PAIRE_MOTO =
{
    SINGLE_PORT_ID,
    SINGLE_PORT_1SEC / 4,
    SINGLE_PORT_1SEC / 4,
    2,
    MOTO,
};


// *************************************************************
//remind
const multimodule_task_msg_t CALL_REMIND_MOTO =
{
    SINGLE_PORT_ID,
    SINGLE_PORT_1SEC / 4,
    SINGLE_PORT_1SEC / 4,
    10,
    // GREEN_LED,
    MOTO,
};

const multimodule_task_msg_t CALL_REJECT_MOTO =
{
    SINGLE_PORT_ID,
    SINGLE_PORT_1SEC / 4,
    SINGLE_PORT_1SEC / 4,
    1,
    // GREEN_LED,
    MOTO,
};

const multimodule_task_msg_t MSG_REMIND_MOTO =
{
    SINGLE_PORT_ID,
    SINGLE_PORT_1SEC / 4,
    SINGLE_PORT_1SEC / 4,
    4,
    // GREEN_LED,
    MOTO,
};


const multimodule_task_msg_t ACTIION_MOTO =
{
    SINGLE_PORT_ID,
    4,
    28,
    1,
    MOTO,
};

const multimodule_task_msg_t APP_EXPMSG_MOTO =
{
    SINGLE_PORT_ID,
    4,
    4,
    4 * 60,
    // GREEN_LED,
    MOTO,
};

const multimodule_task_msg_t APP_MSG_MOTO =
{
    SINGLE_PORT_ID,
    SINGLE_PORT_1SEC / 4,
    SINGLE_PORT_1SEC / 4,
    4,
    MOTO,
};

//alarm
const multimodule_task_msg_t ALARM_MOTO =
{
    SINGLE_PORT_ID,
    SINGLE_PORT_1SEC / 4,
    SINGLE_PORT_1SEC / 4,
    20,
    MOTO,
};

//step complete
const multimodule_task_msg_t SPORT_COMPLETE_MOTO =
{
    SINGLE_PORT_ID,
    SINGLE_PORT_1SEC / 4,
    SINGLE_PORT_1SEC / 4,
    10,
    MOTO,
};

//warn
const multimodule_task_msg_t LONG_SIT_MOTO =
{
    SINGLE_PORT_ID,
    SINGLE_PORT_1SEC / 4,
    SINGLE_PORT_1SEC / 4,
    4,
    MOTO,
};
const multimodule_task_msg_t FACTORY_MOTO = 
{
    SINGLE_PORT_ID,
    16,
    16,
    1,
    MOTO,
};

const multimodule_task_msg_t FACTORY_S1_MOTO =
{
    SINGLE_PORT_ID,
    16,
    16,
    1,
    MOTO,
};

//warn
const multimodule_task_msg_t SAVE_FULL_MOTO =
{
    SINGLE_PORT_ID,
    SINGLE_PORT_1SEC / 4,
    SINGLE_PORT_1SEC / 4,
    2,
    MOTO,
};



// *************************系统配置信息************************************
sys_config_s    systermConfig =
{
    .stepCountSwitch            = SWITCH_ON,
    .heartrateSwitch            = SWITCH_ON,
    .weatherSwitch              = SWITCH_OFF,
    .stepCompleteRemindSwitch   = SWITCH_ON,
    .bleDiscRemindSwitch        = SWITCH_OFF,
    .longSitRemindSwitch        = SWITCH_ON,
    .notDisturbSwitch           = SWITCH_OFF,
    .gestureSwitch              = SWITCH_ON, 
    .appRemindSwitch            = 0xffffffff,
    .appDetailRemindSwitch      = 0xffffffff,
    .systermLanguge             = SYS_ENGLISH_TYPE,
    .systermTimeType            = SYS_TIME_24_TYPE,
};


watchdog_s taskWatchdog =
{
    .lastCnt         = 0,
    .curCnt          = 0,
    .taskId          = 0,
    .taskWdtState    = 0,   
};

//手机状态结构体
phoneState_s    phoneState =
{
    .state      = PHONE_STATE_NORMAL,
    .timeCnt    = 0,
    .timeMax    = 30,   //最大计时30秒
};

//校针状态
movtState_s     movtState =
{
    .state      = MOVT_STATE_NORMAL,
    .timeCnt    = 0,
    .timeMax    = 15,   //最大计时30秒
};

uint8  sceneSwitch;

//蓝牙状态
uint8    bleState = BLE_BROADCAST;;

// *************************************************************
// 久坐
longsit_s appLongSitInfo =
{
    .StartTimeHour          = 8,
    .StartTimeMin           = 0,
    .StopTimeHour           = 22,
    .StopTimeMin            = 0,
    .DisturdStartTimehour   = 22,
    .DisturdStartTimeMin    = 1,
    .DisturdStopTimehour    = 7,
    .DisturdStopTimeMin     = 59,
    .intv_mimute            = 60,
};

// *************************************************************
// 勿扰
not_disturd_s appNotDisturdTimeInfo =
{
    .StartHour              = 9,
    .StartMin               = 0,
    .StopHour               = 17,
    .StopMin                = 0,
};

// *************************************************************
// 第二城市时间RTC
worldrtc_s  WorldRtc;

//手机提醒
uint32      appRemindState;

//计步状态
uint8       stepState;

uint8       resetStatus;

//后台计数
uint32 backstageTimeCounterSec;

uint32 backstageTimeCounterMin;


// *************************************************************
//窗口句柄
uint16 WinDowHandle;






