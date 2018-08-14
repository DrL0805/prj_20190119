#ifndef MID_SLEEP_SCENE_H
#define MID_SLEEP_SCENE_H



// 事件定义
typedef enum
{
	SLEEP_SCENE_START 	= 0,
	SLEEP_SCENE_STOP,
	SLEEP_SCENE_PROCESS,
}SLEEP_SCENE_INTERFACE_E;


typedef struct 
{
	uint8 id;
}sleep_scene_event_s;


uint16 Mid_SleepScene_Init(void);
uint16 Mid_SleepScene_GetRecord(sleep_data *sleepDataTemp, uint8 *validnum);
uint16 Mid_SleepScene_EventProcess(sleep_scene_event_s* msg);
uint16 Mid_SleepScene_CurSleepGet(sleep_ui_info *sleepInfoTemp);

#endif


