#ifndef MID_CLIMB_SCENE_H
#define MID_CLIMB_SCENE_H



typedef struct 
{
	uint8 	id;
	uint32  	para;
}climb_event_s;


// 事件定义
typedef enum
{
	CLIMB_FUNC_OPEN = 0,
	CLIMB_FUNC_CLOSE,
	CLIMB_FLOOR_CHECK,
}CLIMB_INTERFACE_E;


uint16 Mid_ClimbScene_Init(void);
uint16 Mid_ClimbScene_ReadFloor(uint8 *floor);
uint16 Mid_ClimbScene_EventProcess(climb_event_s* msg);

#endif

