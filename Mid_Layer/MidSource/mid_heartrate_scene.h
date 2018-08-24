#ifndef MID_HEARTRATE_SCENE_H
#define MID_HEARTRATE_SCENE_H

#include "platform_common.h"

#define SCENE_HRM_RTT_DEBUG	3
#if (1 == SCENE_HRM_RTT_DEBUG)	// 错误等级
#define SCENE_HRM_RTT_LOG(...)
#define SCENE_HRM_RTT_WARN(...)
#define SCENE_HRM_RTT_ERR		SEGGER_RTT_printf
#elif (2 == SCENE_HRM_RTT_DEBUG)	// 警告等级
#define SCENE_HRM_RTT_LOG(...)
#define SCENE_HRM_RTT_WARN	SEGGER_RTT_printf
#define SCENE_HRM_RTT_ERR		SEGGER_RTT_printf
#elif (3 == SCENE_HRM_RTT_DEBUG)	// 调试等级
#define SCENE_HRM_RTT_LOG		SEGGER_RTT_printf
#define SCENE_HRM_RTT_WARN	SEGGER_RTT_printf
#define SCENE_HRM_RTT_ERR		SEGGER_RTT_printf
#else							// 调试关闭
#define SCENE_HRM_RTT_LOG(...)
#define SCENE_HRM_RTT_WARN(...)
#define SCENE_HRM_RTT_ERR(...)
#endif

#define LOG_HEARTRATE_DURATON           20	//        //心率监测时间，暂定30秒
#define LOG_RESTING_PERIOD              6	//        //静息时间监测周期，暂定10分


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

void HrmLogStorageProcess(void);
void HrmLogPeriodProcess(void);
void HrmLogRestingJudge(uint16 totalStep);
uint16 Mid_HeartRateScene_Init(void);
uint16 HrmLogOpen(void);
uint16 HrmLogClose(void);
uint16 Mid_HeartRateScene_LogHR_Read(uint8* logheartrate, uint8* restingheartrate);
uint8 Mid_HeartRateScene_RestingHR_Read(void);
uint8 Mid_HeartRateScene_Clear(void);

#endif


