#ifndef MID_GESTURE_SCENE_H
#define MID_GESTURE_SCENE_H


//手势类型
typedef enum 
{
	GESTURE_FORWARD 	= 1,//翻腕-前
	GESTURE_BACKWARD, 		//翻腕-后
	GESTURE_LEFT,			//翻腕-左
	GESTURE_RIGHT,			//翻腕-右
	GESTURE_HAND_LIFT, 		//抬手
	GESTURE_HAND_DOWN,		//放手
}enum_gesture_type;


// 事件定义
typedef enum
{
	GESTURE_SCENCE_OPEN = 0,
	GESTURE_SCENCE_CLOSE,
}GESTURE_INTERFACE_E;


typedef struct 
{
	uint8 id;
}gesture_event_s;


uint16 Mid_GestureScene_Init(void(*Cb)(uint8 gesturetye),void(*actionCb)(uint8 *xyzdata));
uint16 Mid_GestureScene_EventProcess(gesture_event_s *msg);

#endif

