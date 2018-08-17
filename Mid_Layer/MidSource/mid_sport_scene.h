#ifndef MID_SPORT_SCENE_H
#define MID_SPORT_SCENE_H

#include "platform_common.h"

#define SPORT_SCENE_PERIOD_MS	(40)	// �Ʋ��㷨��������

typedef struct 
{
	uint8  bodyWeight;		//���أ���λ��kg
	uint8  bodyHeight;		//��ߣ���λ��cm
	uint8  sex; 				//�Ա�
	uint8  age;	 			//����
}bodyInfo_s;

typedef struct 
{
	uint32 totalStep; 		//�˶��ܲ���
	uint32 stepAim;			//�˶�Ŀ�경��
	uint16 stepComplete;		//������ɶ�

	uint32 heatQuantity;		//�˶���·���λ��ka
	uint32 sportDistance;		//���о��룬��λ��m
	uint16 sportDuaration;		//����ʱ�䣬��λ��min
}stepSportInfo_s;

typedef struct
{
	bool		EnableFlg;		// ʹ�üƲ��㷨��־
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
