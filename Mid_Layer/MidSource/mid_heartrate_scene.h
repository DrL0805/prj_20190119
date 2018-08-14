#ifndef MID_HEARTRATE_SCENE_H
#define MID_HEARTRATE_SCENE_H




#define LOG_HEARTRATE_DURATON           20//        //心率监测时间，暂定30秒
#define LOG_RESTING_PERIOD              6//        //静息时间监测周期，暂定10分


typedef enum
{
	HRM_LOG_NULL  = 0,
	HRM_LOG_ENVT,
}hrm_event_register;

// 事件定义
typedef enum
{
	HRM_LOG_OPEN = 0,
	HRM_LOG_CLOSE,
	HRM_LOG_RESTING_JUDGE,
	HRM_LOG_STORAGE_PROCESS,
	HRM_LOG_PERIOND_PROCESS,
}HRM_LOG_INTERFACE_E;

typedef struct 
{
	uint8 curHRM;
	uint8 restingHRM;
}hrm_log_s;

typedef struct 
{
	uint8 id;
	uint16 para;
}hrm_log_event_s;


uint16 Mid_HeartRateScene_Init(void);
uint16 Mid_HeartRateScene_EventProcess(hrm_log_event_s* msg);
uint16 Mid_HeartRateScene_LogHR_Read(uint8* logheartrate, uint8* restingheartrate);
uint8 Mid_HeartRateScene_RestingHR_Read(void);
uint8 Mid_HeartRateScene_Clear(void);

#endif


