#include "scene_sedentary.h"
#include "mid_accelerate.h"
#include "main.h"

Scene_Sedentary_Param_t	Scene_Sedentary;

//**********************************************************************
// �������ܣ�   �㷨������ȡ�������ݻص�����
// ���������   ������������
// ���ز�����	��
// ���ã�˯���㷨��1��1��
void Scene_Sedentary_algorithm(int16 *xyzData, uint32_t Interval)
{
	uint32 tToTalStep;
	uint16 tSetTime;
	
	if(false == Scene_Sedentary.EnableFlg)
		return;	
	
	Scene_Sedentary.IntervalMs += Interval;
	if(Scene_Sedentary.IntervalMs >= GESTURE_SCENE_PERIOD_MS)
	{
		Scene_Sedentary.IntervalMs -= GESTURE_SCENE_PERIOD_MS;

		/* �����㷨 */
		Mid_SportScene_StepRead(&tToTalStep);
		alg_sedentary_process(xyzData, tToTalStep);
		
		// ��ȡ����ʱ��
		tSetTime = alg_sedentary_get_time();
		SCENE_SEDENTARY_RTT_LOG(0, "tSetTime %d \r\n", tSetTime);
	}
}


//**********************************************************************
// �������ܣ�   ��ʼ��
// ���������   ��
// ���ز�����	0x00:�ɹ�
// 				0xff:ʧ��
// ���ã���ʼ������һ��
uint16 Scene_Sedentary_Init(void)
{
	uint32 tToTalStep;
	
	Mid_SportScene_StepRead(&tToTalStep);
	alg_sedentary_init(tToTalStep);
	return 0;
}

//**********************************************************************
// �������ܣ�   
// ���������   
// ���ز�����	
void Scene_Sedentary_Start(void)
{
	Mid_Accel_StartSample(&Scene_Sedentary.SampleId, eMidAccelSampleRate1HZ, eMidAccelSampleRange2G);
	
	Scene_Sedentary.IntervalMs = 0;
	Scene_Sedentary.EnableFlg = true;
}


//**********************************************************************
// �������ܣ�   
// ���������
// ���ز���:
void Scene_Sedentary_Stop(void)
{
	Mid_Accel_StopSample(Scene_Sedentary.SampleId);
	
	Scene_Sedentary.EnableFlg = false;
}






















