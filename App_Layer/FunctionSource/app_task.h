#ifndef     APP_TASK_H
#define     APP_TASK_H


extern TimerHandle_t dailyMonitorSwitchTimer;

extern TimerHandle_t DailyBleDiscTimer;

extern TimerHandle_t DailyCompassTimeOutTimer;

extern TimerHandle_t AuthTimeOutTimer;

extern TimerHandle_t KeyS0releaseHandle;

extern TimerHandle_t KeyS0pressHandle;


extern TimerHandle_t DailyPhotoTimeOutTimer;


#define  APP_APP_TASK_QUEUE_SIZE       64




typedef enum _daily_app_task_msg_id{
    
// KEY
    APP_MSG_KEY_ISR                      = 0x0001,

// RTC
    APP_MSG_RTC_SEC_ISR                = 0x0101,
    APP_MSG_RTC_HALF_SEC_ISR,
    APP_MSG_RTC_MIN,
    APP_MSG_RTC_HOUR,
    APP_MSG_RTC_DAY,
    APP_MSG_RTC_MONTH,
    APP_MSG_RTC_YEAR,

    //BLE 
    APP_MSG_BLE_DISCONNCET           = 0x0201,

    //OTA
    APP_MSG_BLE_OTA                  = 0x0301,

    // bat
    APP_MSG_BAT_CHARGE_EVENT         = 0x0401,

    APP_MSG_SWITCH_MODE_TIMER,

    
    //photo
    APP_TAKE_PHOTO_TIMER_OUT           = 0x0601,
    //movt adjust
    APP_MOVT_ADJUST_TIMER_OUT          = 0x0701,
    
    //step
    APP_SPORT_EVENT                    = 0x0801,

	APP_MSG_UART_RECEIVE               = 0x0902,
    APP_MSG_UART_SENDFINISH            = 0x0903,
	APP_SENSOR_UPLOAD                  = 0x0904,
    APP_CLIMB_PROCESS                  = 0x0905,
    APP_GPS_PROCESS                    = 0x0A00,  
    APP_HRM_PROCESS                    = 0x0B00, 
    APP_PRESSSURE_PROCESS              = 0x0C00,  
	
	APP_GESTURE_PROCESS 				= 0x0D00, 
    APP_DISP_ROUND_PROCESS              = 0x0E00,

    APP_SWITCH_WIN_PROCESS              = 0x0F00,

    APP_PACK_DATA_UPLOAD_PROCESS        = 0x1000,
    APP_SLEEP_DATA_UPLOAD_PROCESS       = 0x1001,
    APP_SCENE_DATA_UPLOAD_PROCESS       = 0x1002,
    APP_SCENE_DATA_SAVE                 = 0x1003,   //数据存储
    APP_SCENE_DATA_SAVE_CHECK           = 0x1004,   //数据存储空间检查
    APP_CACEH_DATA_SAVE                 = 0x1005,   //缓存数据存储
} app_task_msg_id;


typedef struct
{
    uint16    id;
    uint32    para;
}app_task_msg_t;

void App_BleDisConnCb(void);
void App_BleOTA_Cb(uint16 u16status);

void App_Task_EventSet(app_task_msg_t msg);
void AppTask_Create(void);
#endif      //APP_APP_H
