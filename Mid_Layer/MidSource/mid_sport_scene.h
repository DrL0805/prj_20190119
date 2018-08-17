#ifndef MID_SPORT_SCENE_H
#define MID_SPORT_SCENE_H

#include "platform_common.h"

#define SPORT_SCENE_PERIOD_MS	(40)	// 计步算法调用周期

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
	uint16 sportDuaration;		//运行时间，单位：min
}stepSportInfo_s;

typedef struct
{
	bool		EnableFlg;		// 使用计步算法标志
	uint8_t		SampleId;
	uint32_t 	IntervalMs;		
}Mid_SportScene_Param_t;

extern void Mid_SportScene_Algorithm(int16 *xyzData, uint32_t Interval);
extern uint16 Mid_SportScene_Init(void);
extern void Mid_SportScene_Start(void);
extern void Mid_SportScene_Stop(void);
extern uint16 Mid_SportScene_ClearInfo(void);
extern uint16 Mid_SportScene_SportInfoRead(stepSportInfo_s *stepsportinfo);
extern uint16 Mid_SportScene_StepRead(uint32 *step);
extern uint16 Mid_SportScene_StepWrite(uint32 *step);
extern uint16 Mid_SportScene_StepAimSet(uint32 stepAim);
extern uint16 Mid_SportScene_StepAimRead(uint32 *stepAim);
extern uint16 SportDuarationProcess(void);
extern uint16 Mid_SportScene_BodyInfoRead(bodyInfo_s *bodyinfo);
extern uint16 Mid_SportScene_BodyInfoSet(bodyInfo_s *bodyinfo);
extern uint16 Mid_SportScene_5minStepRead(void);
extern uint16 Mid_SportScene_5minStepClear(void);
extern uint16 Mid_SportScene_SecStepRead(void);
extern uint16 Mid_SportScene_SecStepClear(void);
extern uint16 Mid_SportScene_10SecStepRead(void);
extern uint16 Mid_SportScene_10SecStepClear(void);
extern uint16 Mid_SportScene_SportDuarationRead(uint32 *duaration);
extern uint16 Mid_SportScene_SportDuarationWrite(uint32 *duaration);

#endif 
