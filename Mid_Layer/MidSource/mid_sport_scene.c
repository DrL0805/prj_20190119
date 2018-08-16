#include "mid_sport_scene.h"
#include "algorithm_lis3dh.h"
#include "mid_accelerate.h"

/*************** macro define ********************************/
#define CALORIECON0 	13.63636
#define CALORIECON1 	0.000693
#define CALORIECON2 	0.00495

#define DISTANCECON0 	0.45


/************** variable define *****************************/
static stepSportInfo_s stepSportInfo;
static bodyInfo_s 	   bodyInfo;

static uint32 	stepOld  = 0;		// 1��ǰ�ļƲ�ֵ���뵱ǰ�Ʋ�ֵ���бȽ��Ƿ�����µļƲ�
static uint16 	step5minAcc = 0;	//5���ӼƲ�ֵ
static uint16 	stepSecAcc = 0;		//30��Ʋ�ֵ���͹��Ĵ���ʹ��
static uint16 	step10SecAcc = 0;		//10��Ʋ�ֵ���˶�����ʹ��
static uint8  	secCnt  = 0;			
static uint8 	stepCompleteRemind;

static Mid_SportScene_Param_t	Mid_SportScene;

/************** function define *****************************/

//**********************************************************************
// �������ܣ�   �Ʋ��㷨������ȡ�������ݻص�����
// ���������   ������������
// ���ز�����	��
// ���ã�25HZƵ�ʽ��е���
void Mid_SportScene_Algorithm(int16 *xyzData, uint32_t Interval)
{
    uint8     temp;
    uint8 	i;
	int8  	G_sensorBuf[3];

	if(false == Mid_SportScene.EnableFlg)
		return;
	
	Mid_SportScene.IntervalMs += Interval;
	if(Mid_SportScene.IntervalMs >= SPORT_SCENE_PERIOD_MS)
	{
		Mid_SportScene.IntervalMs -= SPORT_SCENE_PERIOD_MS;
	
		//�Ʋ��˶��㷨����
		for(i = 0; i < 3; i ++)
		{		
			G_sensorBuf[i]	= (xyzData[i]>>8);
		}
		temp  = Algorithm_Calculate_Step((uint8*)G_sensorBuf);
		
		if(temp == 1 || temp == 12)
		{
			stepSportInfo.totalStep      += temp;
			step5minAcc  += temp;
			stepSecAcc   += temp;
			step10SecAcc += temp;

			stepSportInfo.stepComplete   = (uint16)(stepSportInfo.totalStep * 100 / stepSportInfo.stepAim);
			if(stepSportInfo.stepComplete >= 100)
			{
				if(stepSportInfo.stepComplete >= 999)
				{
					stepSportInfo.stepComplete = 999;
				}
				
				if (stepCompleteRemind == 0)		//�״����ִ������
				{
					stepCompleteRemind = 1;
					
					/* �����˶�Ŀ�����¼� */

				}
			}

			stepSportInfo.heatQuantity  = stepSportInfo.totalStep * ((bodyInfo.bodyWeight - CALORIECON0) * CALORIECON1 + CALORIECON2);
			stepSportInfo.sportDistance = bodyInfo.bodyHeight * DISTANCECON0 * stepSportInfo.totalStep / 100 ;
		}
		//��Ӿ�˶��㷨����
	//	swimming_algorithm(xyzData); 		
	}
}

//**********************************************************************
// �������ܣ�   �Ʋ��˶���Ϣ��ʼ��
// ���������   ��
// ���ز�����	0x00:�ɹ�
// 				0xff:ʧ��
// ���ã���ʼ���׶ε���һ��
uint16 Mid_SportScene_Init(void)
{
	bodyInfo.bodyWeight 			= 60;
	bodyInfo.bodyHeight 			= 175;
	bodyInfo.sex 					= 0;
	bodyInfo.age 					= 18;

	stepSportInfo.totalStep 		= 0;
	stepSportInfo.stepComplete 		= 0;
	stepSportInfo.stepAim 			= 6000;

	stepSportInfo.heatQuantity 		= 0;
	stepSportInfo.sportDistance 	= 0;

	stepSportInfo.sportDuaration 	= 0;
	
	secCnt 							= 0;
	stepOld							= 0;
	stepCompleteRemind 				= 0;
	
	return 0;
}

//**********************************************************************
// �������ܣ�   �Ʋ��˶�����
// ���������   ��
// ���ز�����	0x00:�ɹ�
// 				0xff:ʧ��
// ���ã��迪���Ʋ�����ʱ
void Mid_SportScene_Start(void)
{
	Mid_Accel_ParamSet(eMidAccelSampleRate25HZ, eMidAccelSampleRange2G);
	Mid_Accel_StartSample();	
	Mid_SportScene.EnableFlg = true;
}

//**********************************************************************
// �������ܣ�   �Ʋ��˶��ر�
// ���������   ��
// ���ز�����	0x00:�ɹ�
// 				0xff:ʧ��
// ���ã���رռƲ�����ʱ
void Mid_SportScene_Stop(void)
{
	Mid_Accel_StopSample();
	Mid_SportScene.EnableFlg = false;
}

//**********************************************************************
// �������ܣ�   �˶���Ϣ����
// ���������   ��
// ���ز�����	0x00:�ɹ�
// 				0xff:ʧ��
// ���ã�ÿ�����һ��
uint16 Mid_SportScene_ClearInfo(void)
{
	stepSportInfo.totalStep 		= 0;
	stepSportInfo.stepComplete 		= 0;

	stepSportInfo.heatQuantity 		= 0;
	stepSportInfo.sportDistance 	= 0;
	stepSportInfo.sportDuaration 	= 0;
	
	secCnt 							= 0;
	stepOld							= 0;
	stepCompleteRemind 				= 0;

	return 0;
}



//**********************************************************************
// �������ܣ�   ��ȡ�Ʋ��˶���¼��Ϣ
// ���������   ��
// ���ز�����	0x00:�ɹ�
// 				0xff:ʧ��
// ���ã��ⲿ
uint16 Mid_SportScene_SportInfoRead(stepSportInfo_s *stepsportinfo)
{
	*stepsportinfo 					= stepSportInfo;
	return 0;
}

//**********************************************************************
// �������ܣ�   ��ȡ�Ʋ���
// ���������   ��
// ���ز�����	0x00:�ɹ�
// 				0xff:ʧ��
// ���ã��ⲿ
uint16 Mid_SportScene_StepRead(uint32 *step)
{
	*step 		= stepSportInfo.totalStep;
	return 0;
}

//**********************************************************************
// �������ܣ�   �Ʋ����ָ�
// ���������   ��
// ���ز�����	0x00:�ɹ�
// 				0xff:ʧ��
// ���ã��ⲿ
uint16 Mid_SportScene_StepWrite(uint32 *step)
{
	stepSportInfo.totalStep = *step;
	stepSportInfo.heatQuantity  = stepSportInfo.totalStep * ((bodyInfo.bodyWeight - CALORIECON0) * CALORIECON1 + CALORIECON2);
	stepSportInfo.sportDistance = bodyInfo.bodyHeight * DISTANCECON0 * stepSportInfo.totalStep / 100 ;

	return 0;
}
//**********************************************************************
// �������ܣ�   ���üƲ��˶�Ŀ�경��
// ���������   ��
// ���ز�����	0x00:�ɹ�
// 				0xff:ʧ��
// ���ã��ⲿ���յ����ò���ָ��������ݻָ�
uint16 Mid_SportScene_StepAimSet(uint32 stepAim)
{
	stepSportInfo.stepAim 		= stepAim;
	stepSportInfo.stepComplete   = (uint16)(stepSportInfo.totalStep * 100 / stepSportInfo.stepAim);
    if(stepSportInfo.stepComplete >= 100)
    {
   	 	if(stepSportInfo.stepComplete >= 999)
		{
			stepSportInfo.stepComplete = 999;
		}
			
   	 	if (stepCompleteRemind == 0)		//�״����ִ������
   	 	{
   	 		stepCompleteRemind = 1;
   	 		
			/* �����˶�Ŀ�����¼� */
			
   	 	}
    }
    else
    {
    	stepCompleteRemind = 0;				//�������
    }

    return 0;
}

//**********************************************************************
// �������ܣ�   ��ȡ�Ʋ��˶�Ŀ�경��
// ���������   ��
// ���ز�����	0x00:�ɹ�
// 				0xff:ʧ��
// ���ã��ⲿ
uint16 Mid_SportScene_StepAimRead(uint32 *stepAim)
{
	*stepAim = stepSportInfo.stepAim;

    return 0;
}

//**********************************************************************
// �������ܣ�   �Ʋ��˶�ʱ���ۼӴ���ÿ1�����һ��
// ���������   ��
// ���ز�����	0x00:�ɹ�
// 				0xff:ʧ��
// ���ã�ÿ�����һ�Σ�����1min����һ�Ρ���Ϊֻ�в����Ʋ�������Ч�˶�
uint16 SportDuarationProcess(void)
{
	if (stepOld != stepSportInfo.totalStep)
	{
		secCnt ++;
		stepOld = stepSportInfo.totalStep;

		if (secCnt >= 60)
		{
			secCnt = 0;
			stepSportInfo.sportDuaration 	+= 1;
		}
	}

	return 0;
}

//**********************************************************************
// �������ܣ�   ��ȡ������Ϣ
// ���������   ��
// ���ز�����	0x00:�ɹ�
// 				0xff:ʧ��
// ���ã��ⲿ
uint16 Mid_SportScene_BodyInfoRead(bodyInfo_s *bodyinfo)
{
	bodyinfo->bodyWeight 	= bodyInfo.bodyWeight;
	bodyinfo->bodyHeight 	= bodyInfo.bodyHeight;
	bodyinfo->sex 			= bodyInfo.sex;
	bodyinfo->age 			= bodyInfo.age;

	
	return 0;
}

//**********************************************************************
// �������ܣ�   ����������Ϣ
// ���������   ��
// ���ز�����	0x00:�ɹ�
// 				0xff:ʧ��
// ���ã��ⲿ���յ�Э��ָ��������ݻָ�
uint16 Mid_SportScene_BodyInfoSet(bodyInfo_s *bodyinfo)
{	
	bodyInfo.bodyWeight 		= bodyinfo->bodyWeight;
	bodyInfo.bodyHeight 		= bodyinfo->bodyHeight;
	bodyInfo.sex 				= bodyinfo->sex;;
	bodyInfo.age 				= bodyinfo->age;;

	return 0;
}

//**********************************************************************
// �������ܣ�   ��ȡ5���ӼƲ��ۼ�ֵ
// ���������   ��
// ���ز�����	0x00:�ɹ�
// 				0xff:ʧ��
// ���ã��ⲿ��ÿ5���Ӵ洢һ�μƲ�ֵ
uint16 Mid_SportScene_5minStepRead(void)
{
	return step5minAcc;
}

//**********************************************************************
// �������ܣ�   ���5���ӼƲ��ۼ�ֵ
// ���������   ��
// ���ز�����	0x00:�ɹ�
// 				0xff:ʧ��
// ���ã��洢����ô˺������
uint16 Mid_SportScene_5minStepClear(void)
{
	step5minAcc = 0;
	
	return 0;
}

//**********************************************************************
// �������ܣ�   ��ȡ5���ӼƲ��ۼ�ֵ
// ���������   ��
// ���ز�����	0x00:�ɹ�
// 				0xff:ʧ��
// ���ã��ⲿ���Ʋ�׼��״̬�£���ʱ30�룬����Ƿ��мƲ�����,�л����Ʋ������״̬
uint16 Mid_SportScene_SecStepRead(void)
{
	return stepSecAcc;
}

//**********************************************************************
// �������ܣ�   ���5���ӼƲ��ۼ�ֵ
// ���������   ��
// ���ز�����	0x00:�ɹ�
// 				0xff:ʧ��
// ���ã��ⲿ���Ʋ�׼��״̬�£���ʱ30�룬����Ƿ��мƲ�����,�л����Ʋ������״̬
uint16 Mid_SportScene_SecStepClear(void)
{
	stepSecAcc = 0;
	
	return 0;
}

//**********************************************************************
// �������ܣ�   ��ȡ10��Ʋ��ۼ�ֵ
// ���������   ��
// ���ز�����	0x00:�ɹ�
// 				0xff:ʧ��
// ���ã������ܲ�����
uint16 Mid_SportScene_10SecStepRead(void)
{
	return step10SecAcc;
}

//**********************************************************************
// �������ܣ�   ���10��Ʋ��ۼ�ֵ
// ���������   ��
// ���ز�����	0x00:�ɹ�
// 				0xff:ʧ��
uint16 Mid_SportScene_10SecStepClear(void)
{
	step10SecAcc = 0;
	
	return 0;
}
//**********************************************************************
// �������ܣ�   ��ȡ�˶�ʱ��
// ���������   ��
// ���ز�����	0x00:�ɹ�
// 				0xff:ʧ��
uint16 Mid_SportScene_SportDuarationRead(uint32 *duaration)
{
	*duaration = stepSportInfo.sportDuaration;
	return 0;
}

//**********************************************************************
// �������ܣ�   �ָ��˶�ʱ��
// ���������   ��
// ���ز�����	0x00:�ɹ�
// 				0xff:ʧ��
uint16 Mid_SportScene_SportDuarationWrite(uint32 *duaration)
{
	stepSportInfo.sportDuaration = *duaration;
	return 0;
}

