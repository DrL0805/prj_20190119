#ifndef MID_SPORT_SCENE_H
#define MID_SPORT_SCENE_H


typedef struct 
{
	uint8  bodyWeight;		//体重：单位：kg
	uint8  bodyHeight;		//身高：单位：cm
	uint8  sex; 				//性别
	uint8  age;	 			//年龄
}bodyInfo_s;

typedef struct 
{
	uint32 totalStep; 		//运动总步数
	uint32 stepAim;			//运动目标步数
	uint16 stepComplete;		//运行完成度

	uint32 heatQuantity;		//运动卡路里，单位：ka
	uint32 sportDistance;		//运行距离，单位：m
	uint16 sportDuaration;	//运行时间，单位：min
}stepSportInfo_s;

//运行状态
typedef enum 
{
	STEP_UPDATA 	= 0,
	STEP_COMPLETE,
}sport_state_s;

// 事件定义
typedef enum
{
	SPORT_STEP_START 	= 0,
	SPORT_STEP_STOP,
	SPORT_STEP_DUARATION_PROCESS,
}SPORT_INTERFACE_E;


typedef struct 
{
	uint8 id;
}sport_event_s;



uint16 Mid_SportScene_Init(void);
uint16 Mid_SportScene_SportInfoRead(stepSportInfo_s *stepsportinfo);
uint16 Mid_SportScene_StepRead(uint32 *step);
uint16 Mid_SportScene_StepWrite(uint32 *step);
uint16 Mid_SportScene_StepAimSet(uint32 stepAim);
uint16 Mid_SportScene_SportInfoReset(void);
uint16 Mid_SportScene_BodyInfoRead(bodyInfo_s *bodyinfo);
uint16 Mid_SportScene_BodyInfoSet(bodyInfo_s *bodyinfo);
uint16 Mid_SportScene_EventProcess(sport_event_s* msg);
uint16 Mid_SportScene_StepAimRead(uint32 *stepAim);
uint16 Mid_SportScene_5minStepRead(void);
uint16 Mid_SportScene_5minStepClear(void);
uint16 Mid_SportScene_SportDuarationRead(uint32 *duaration);
uint16 Mid_SportScene_SportDuarationWrite(uint32 *duaration);
//**********************************************************************
// 函数功能：   运动信息清零
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
uint16 Mid_SportScene_ClearInfo(void);

//**********************************************************************
// 函数功能：   运动状态反馈回调注册
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
uint16 Mid_SportScene_SportStateCbSet(void (*cb)(uint16 status));

uint16 Mid_SportScene_SecStepRead(void);
uint16 Mid_SportScene_SecStepClear(void);

#endif 
