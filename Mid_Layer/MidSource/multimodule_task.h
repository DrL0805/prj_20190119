#ifndef			MULTIMODULE_TASK_H
#define			MULTIMODULE_TASK_H

#include "mid_interface.h"

typedef enum
{
	SINGLE_PORT_ID			= 0x0001,
	STOPWATCH_ID,
	RTC_ID,
	ACCEL_ID,
	MAG_ID,
	GYRO_ID,
	COMPASS_ID,
	UARTAPP_ID,
	HEARTRATE_ID,
	PRESSURE_ID,
	HEARTRATE_LOG_ID,
	SPORT_ID,
	WEATHER_ID,
	CLIMB_ID,
	GESTURE_ID,
	BAT_ID,
	SLEEP_ID,
	USUAL_ID,
	SCENE_ID,
}module_id;


typedef struct 
{
	uint16		id;
	union
	{
		single_port_set_para_s		singlePortPara;
		stopwatch_time_s			stopwatchTime;
		rtc_time_s					rtcTime;
		compass_event_s             compassEvent;
		uartapp_event_s             uartappEvent;
		accel_event_s 				accelEvent;
		gyro_event_s                gyroEvent;
		mag_event_s					magEvent;
		hrm_event_s					hrmEvent;
		pressure_event_s 			pressureEvent;
		uartapp_event_s				uartEvent;
		hrm_log_event_s 			hrmLogEvent;
		sport_event_s 				sportEvent;
		weather_event_s 			weatherEvent;
		climb_event_s 				climbEvent;
		gesture_event_s 			gestureEvent;
		bat_event_s  				batEvent;
		sleep_scene_event_s 		sleepEvent;
		usual_event_s 				usualEvent;
		scene_event_s 				sceneEvent;
	}module;
}multimodule_task_msg_t;


extern uint32 SceneApplicationSwitch;

uint16 MultiModuleTask_EventSet(multimodule_task_msg_t msg);
void MultiModuleTask_Create(void);




#endif			// MULTIMODULE_APP_H
