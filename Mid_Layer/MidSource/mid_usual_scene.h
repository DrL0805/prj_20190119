#ifndef		MID_USUAL_SCENE_H
#define		MID_USUAL_SCENE_H


typedef enum
{
	USUAL_SCENE_SLEEP		= 0,
	USUAL_SCENE_SEDENTARY,
	USUAL_SCENE_STANDSTILL,
}usual_scene_type_e;

#define			USUAL_SCENE_NUM			3


// 事件定义
typedef enum
{
	USUAL_SCENCE_OPEN = 0,
	USUAL_SCENCE_CLOSE,
}USUAL_INTERFACE_E;


typedef struct 
{
	uint8 id;
}usual_event_s;



uint16 Mid_UsualScene_EventProcess(usual_event_s *msg);


void Mid_UsualScene_Reset(void);
void Mid_UsualScene_Init(void);
uint16 Mid_UsualScene_Enable(uint16 scene);
uint16 Mid_UsualScene_Disable(uint16 scenpe);
uint16 Mid_UsualScene_StateRead(uint16 scene);
uint16 Mid_UsualScene_SleepMotionRead(void);
void Mid_UsualScene_SleepMotionClear(void);
uint32 Mid_UsualScene_SedentaryMotionRead(void);
uint16 Mid_UsualScene_SedentaryTimeRead(void);
void Mid_UsualScene_SedentaryClear(void);
uint16 Mid_UsualScene_StandstillTimeRead(void);
void Mid_UsualScene_StandstillTimeClear(void);
uint16 Mid_UsualScene_ReadMotion(void);



#endif		// USUALSCENE_APP_H
