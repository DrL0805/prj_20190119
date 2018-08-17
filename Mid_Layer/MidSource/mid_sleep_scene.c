
#include "mid_accelerate.h"
#include "mid_sleep_scene.h"


Mid_SleepScene_Param_t	Mid_SleepScene;

//**********************************************************************
// �������ܣ�   ˯���㷨������ȡ�������ݻص�����
// ���������   ������������
// ���ز�����	��
// ���ã�˯���㷨��1��1��
void Mid_SleepScene_algorithm(int16 *xyzData, uint32_t Interval)
{
	int8  	G_sensorBuf[3];
	
	if(false == Mid_SleepScene.EnableFlg)
		return;	
	
	Mid_SleepScene.IntervalMs += Interval;
	if(Mid_SleepScene.IntervalMs >= SLEEP_SCENE_PERIOD_MS)
	{
		Mid_SleepScene.IntervalMs -= SLEEP_SCENE_PERIOD_MS;
		
		sleep_algorithm(xyzData);
	}
}

//**********************************************************************
// �������ܣ�   ˯����Ϣ��ʼ��
// ���������   ��
// ���ز�����	0x00:�ɹ�
// 				0xff:ʧ��
// ���ã���ʼ������һ��
uint16 Mid_SleepScene_Init(void)
{
	sleep_algorithm_init();
	return 0;
}

//**********************************************************************
// �������ܣ�   ˯����Ϣ��ȡ�������洢
// ���������   ��
// ���ز�����	0x00:�ɹ�
// 				0xff:ʧ��
// ���ã���洢˯������ʱ��1Сʱ����һ��
uint16 Mid_SleepScene_GetRecord(sleep_data *sleepDataTemp, uint8 *validnum)
{
	*validnum = get_sleep_recode(sleepDataTemp);
	
	return 0;
}

//**********************************************************************
// �������ܣ�   ���ػ�ȡ˯��������Ϣ
// ���������   ��
// ���ز�����	0x00:�ɹ�
// 				0xff:ʧ��
// ���ã��ⲿ������UI����
uint16 Mid_SleepScene_CurSleepGet(sleep_ui_info *sleepInfoTemp)
{
	*sleepInfoTemp = get_sleep_info();
	return 0;
}

//**********************************************************************
// �������ܣ�   ˯�߳�������
// ���������   ��
// ���ز�����	0x00:�ɹ�
// 				0xff:ʧ��
void Mid_SleepScene_Start(void)
{
	Mid_Accel_StartSample(&Mid_SleepScene.SampleId, eMidAccelSampleRate1HZ, eMidAccelSampleRange2G);
	
	Mid_SleepScene.IntervalMs = 0;
	Mid_SleepScene.EnableFlg = true;
}

//**********************************************************************
// �������ܣ�    ˯�߳�������
// ���������   ��
// ���ز�����	0x00:�ɹ�
// 				0xff:ʧ��
void Mid_SleepScene_Stop(void)
{
	Mid_Accel_StopSample(Mid_SleepScene.SampleId);
	
	Mid_SleepScene.EnableFlg = false;
}





