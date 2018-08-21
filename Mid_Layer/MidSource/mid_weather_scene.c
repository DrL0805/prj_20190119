
#include "rtos.h"
#include "mid_weather_scene.h"

static 	Mid_Weahter_Param_t Mid_Weahter;			//��������	

/************** function define *****************************/


//**********************************************************************
// �������ܣ�  ����Ԥ�����ܳ�ʼ��
// ���������   
// ���ز�����  ��
uint16 Mid_WeatherScene_Init(void)
{
	Mid_Weahter.Status 	= WEATHER_TENDENCY_CLOUDY;
	Mid_Weahter.CurTemperature = 26;
	Mid_Weahter.MaxTemperature = 26;
	Mid_Weahter.MinTemperature = 26;

	return 0;
}

//**********************************************************************
// �������ܣ�  ����Ԥ����������
// ���������   
// ���ز�����  ��
uint16 WeatherForecastOpen(void)
{
	
	return 0;
}

//**********************************************************************
// �������ܣ�  ����Ԥ�����ܹر�
// ���������   
// ���ز�����  ��
uint16 WeatherForecastClose(void)
{	

	return 0;
}

//**********************************************************************
// �������ܣ�  ����Ԥ������,2Сʱ����1��
// ���������   
// ���ز�����  ��
static uint16 WeatherForecastUpdata(void)
{

	return 0;
}

//**********************************************************************
// �������ܣ�  ������ѹ��⣬30���Ӽ��һ��
// ���������   
// ���ز�����  ��
static uint16 WeatherForecastPressureCheck(void)
{

	return 0;
}

//**********************************************************************
// �������ܣ�  ��ѹ������5����һ��,����6��ֵ
// ���������   
// ���ز�����  ��
static uint16 WeatherForecastPressureSample(void)
{

	return 0;
}

//**********************************************************************
// �������ܣ�  ���²�����1����һ��
// ���������   
// ���ز�����  ��
static uint16 WeatherForecastTempSample(void)
{

	return 0;
}

//**********************************************************************
// �������ܣ�  ��ȡ��������
// ���������   
// ���ز�����  ��
//**********************************************************************
uint16 Mid_WeatherScene_TendencyGet(Mid_Weahter_Param_t *weatherinfo)
{
	weatherinfo->Status 		= Mid_Weahter.Status;
	weatherinfo->CurTemperature = Mid_Weahter.CurTemperature;
	weatherinfo->MaxTemperature  = Mid_Weahter.MaxTemperature;
	weatherinfo->MinTemperature = Mid_Weahter.MinTemperature;
	return 0;
}

//**********************************************************************
// �������ܣ�  ������������
// ���������   
// ���ز�����  ��
//**********************************************************************
uint16 Mid_WeatherScene_TendencySet(Mid_Weahter_Param_t *weatherinfo)
{
	Mid_Weahter.Status 		  = weatherinfo->Status;

	Mid_Weahter.CurTemperature = weatherinfo->CurTemperature;
	Mid_Weahter.MaxTemperature = weatherinfo->MaxTemperature;
	Mid_Weahter.MinTemperature = weatherinfo->MinTemperature;
	return 0; 
}


