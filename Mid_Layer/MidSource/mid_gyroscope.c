/**********************************************************************
**
**ģ��˵��: mid�������ǽӿ�
**����汾���޸���־(ʱ�䣬����),�޸���:
**   V1.0   2018.4.25  �޸�����  ZSL  
**
**********************************************************************/
#include "rtos.h"
#include "mid_gyroscope.h"
#include "drv_mtimer.h"
#include "mid_scheduler.h"

// DEBUG���� **************************************************************
#define MID_GYRO_RTT_DEBUG_ON	1
#if MID_GYRO_RTT_DEBUG_ON
#define MID_GYRO_RTT_printf	SEGGER_RTT_printf
#else
#define MID_GYRO_RTT_printf(...)
#endif

// �ڲ����� **************************************************************
MID_GYRO_PARA_T	MID_GYRO;

//**********************************************************************
// �������ܣ�	������ʱ����ʱ�ص�����
// ���������	
// ���ز�����	
static void Mid_Accel_IsrCb(void)
{
	Mid_Schd_TaskMsg_T Msg;
	Msg.Id = eSchdTaskMsgGyro;
	
	Mid_Schd_TaskEventSet(&Msg, 1);	
}

//**********************************************************************
// �������ܣ�	���ݵ�ǰ�����ʣ����ز�������
// ���������	SampleRate ������
// ���ز�����	�������ڣ���λms
//**********************************************************************
static uint16_t Mid_Gyro_GetSamplePeriod(bsp_gyroscope_osr SampleRate)
{
	uint16_t TmpSamplePeriod;	
	
	switch (SampleRate)
	{
		case GYO_SAMPLERATE_NONE   :
			TmpSamplePeriod = 0;
			break;
		case GYO_SAMPLERATE_25HZ :	
			TmpSamplePeriod = 40;
			break;
		case GYO_SAMPLERATE_50HZ :	
			TmpSamplePeriod = 20;
			break;
		case GYO_SAMPLERATE_100HZ :	
			TmpSamplePeriod = 10;
			break;
		case GYO_SAMPLERATE_200HZ :	
		case GYO_SAMPLERATE_400HZ :	
		case GYO_SAMPLERATE_800HZ   :	
		case GYO_SAMPLERATE_1600HZ  :	
		case GYO_SAMPLERATE_3200HZ  :	
			TmpSamplePeriod = 5;
			break;
		default:
			TmpSamplePeriod = 0;
			break;
	}
	
	return TmpSamplePeriod;
}

//**********************************************************************
// �������ܣ�	��ȡ����������ʲ���
// ���������	Rate �м�������ö�ٲ���
// ���ز�����	�����������ö�ٲ���
static bsp_gyroscope_osr Mid_Gyro_DrvRateGet(eMidGyroSampleRate Rate)
{
	bsp_gyroscope_osr RetVal;
	
	switch(Rate)
	{
		case eMidGyroSampleRate1HZ:
		case eMidGyroSampleRate2HZ:
		case eMidGyroSampleRate25HZ:
			RetVal = GYO_SAMPLERATE_25HZ;
			break;
		case eMidGyroSampleRate50HZ:
			RetVal = GYO_SAMPLERATE_50HZ;
			break;
		case eMidGyroSampleRate100HZ:
			RetVal = GYO_SAMPLERATE_100HZ;
			break;
		case eMidGyroSampleRate200HZ:
			RetVal = GYO_SAMPLERATE_200HZ;
			break;
		default:
			RetVal = GYO_SAMPLERATE_25HZ;
			break;
	}
	
	return RetVal;	
}

//**********************************************************************
// �������ܣ�	��ȡ�������������
// ���������	Rate �м�������Χö�ٲ���
// ���ز�����	��������÷�Χö�ٲ���
static bsp_gyroscope_scalerange Mid_Gyro_DrvRangeGet(eMidGyroSampleRange Range)
{
	bsp_gyroscope_scalerange RetVal;
	
	switch(Range)
	{
		case eMidGyroSampleRange125S:
			RetVal = GYRO_SCALE_RANGE_125_DEG_SEC;
			break;
		case eMidGyroSampleRange250S:
			RetVal = GYRO_SCALE_RANGE_250_DEG_SEC;
			break;
		case eMidGyroSampleRange500S:
			RetVal = GYRO_SCALE_RANGE_500_DEG_SEC;
			break;
		case eMidGyroSampleRange1000S:
			RetVal = GYRO_SCALE_RANGE_1000_DEG_SEC;
			break;
		case eMidGyroSampleRange2000S:
			RetVal = GYRO_SCALE_RANGE_2000_DEG_SEC;
			break;
		default:
			RetVal = GYRO_SCALE_RANGE_500_DEG_SEC;
			break;
	}
	
	return RetVal;
}

//**********************************************************************
// �������ܣ�	��ʼ��Ӳ���������������ÿɶ�ʱ��ȡ�Ļص�����
// ���������	IsrCb: �������ﵽ�����õĶ�ȡʱ��ص�����
// ���ز�����	��
//**********************************************************************
void Mid_Gyro_Init(void)
{
	// Ӳ����ʼ������
	Drv_Gyro_GoSleep();
	
	// Ĭ�ϲ�����ʼ��
	MID_GYRO.SampleRate = eMidGyroSampleRate25HZ;
	MID_GYRO.SampleRange = eMidGyroSampleRange500S;
	MID_GYRO.SamplePeriod = 1000 / MID_GYRO.SampleRate;	
	MID_GYRO.InitedFlg = true;

	// ����������ʱ��
//	Drv_MTimer_Create(&MID_GYRO.MTiemrId, MID_GYRO.SamplePeriod, Mid_Accel_IsrCb);
}

//**********************************************************************
// �������ܣ�	��������
// ���������	Rate ������
//				Range ��������
// ���ز�����	��
void Mid_Gyro_ParamSet(eMidGyroSampleRate Rate, eMidGyroSampleRange Range)
{
	MID_GYRO.SampleRate = Rate;
	MID_GYRO.SampleRange = Range;
	MID_GYRO.SamplePeriod = 1000 / MID_GYRO.SampleRate;		
}

//**********************************************************************
// �������ܣ�	��ʼ����
// ���������	�������ںͲ�����Χ
// ���ز�����	
void Mid_Gyro_StartSample(void)
{
	// Ӳ������
	Drv_Gyro_Set(Mid_Gyro_DrvRateGet(MID_GYRO.SampleRate), Mid_Gyro_DrvRangeGet(MID_GYRO.SampleRange));
	
	// ���²�����ʱ������������
//	Drv_MTimer_Stop(MID_GYRO.MTiemrId);	
//	Drv_MTimer_Start(MID_GYRO.MTiemrId, MID_GYRO.SamplePeriod);
	
	MID_GYRO.SamplingFlg = true;	
}

//**********************************************************************
// �������ܣ�	ֹͣ����
// ���������	
// ���ز�����	
void Mid_Gyro_StopSample(void)
{
	// ֹͣӲ������
	Drv_Gyro_GoSleep();
	
	// ֹͣ������ʱ��
//	Drv_MTimer_Stop(MID_GYRO.MTiemrId);	
	
	MID_GYRO.SamplingFlg = false;
}

//**********************************************************************
// �������ܣ�	��ȡһ��Ӳ�����ݲ����£��ȴ�����Ҫ����������ȡ
// ���������	
// ���ز�����
void Mid_Gyro_DataUpdate(void)
{
	Drv_Gyro_Read(MID_GYRO.LatestData);
}

//**********************************************************************
// �������ܣ�	��ȡ�������ò���
// ���������	��
// ���ز�����	��
void Mid_Gyro_ParamGet(MID_GYRO_PARA_T* MID_GYRO_PARA)
{
	memcpy(MID_GYRO_PARA, &MID_GYRO, sizeof(MID_GYRO_PARA_T));
}

//**********************************************************************
// �������ܣ�	��ȡ��ǰ���ٶȵ�����ֵ
// ���������	��
// ���ز�����	��
//**********************************************************************
void Mid_Gyro_DataRead(int16 data[3])
{
	data[0]		= MID_GYRO.LatestData[0];
	data[1]		= MID_GYRO.LatestData[1];
	data[2]		= MID_GYRO.LatestData[2];
}

//**********************************************************************
// �������ܣ�	�������Լ�,���øú���ʱ��ȷ������Դδʹ��
// ���������	��
// ���ز�����	0x00:�ɹ�
// 				0xff:ʧ��
//**********************************************************************
uint16 Mid_Gyro_SelfTest(void)
{
	return Drv_Gyro_SelfTest();
}

// ***********************************************************************
//	������������ȴ���
// ***********************************************************************
// ������ʱ���ص�����
#if 0
static void Gyro_TimerCallback(TimerHandle_t xTimer)
{
	 // ���������ź�����֪ͨTask�ɶ�ȡ�µ�����
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	vTaskNotifyGiveFromISR(Gyro_TaskHandle, &xHigherPriorityTaskWoken);

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);	
}

static void GyroTask_Process(void *pvParameters)
{	
	// ����������ʱ��
    Gyro_TimerHandle=xTimerCreate(
					(const char*		)"Gyro_TimerHandle",
					(TickType_t			)(Gyro_TimerPeriod),
					(UBaseType_t		)pdTRUE,
					(void*				)Gyro_TimerID,
					(TimerCallbackFunction_t)Gyro_TimerCallback); //���ڶ�ʱ��������1s(1000��ʱ�ӽ���)������ģʽ	
	if(Gyro_TimerHandle == NULL)
	{
		MID_GYRO_RTT_printf(0,"Gyro_Timer Create Err \r\n");
	}	
					
	while(1)
	{
		// �ȴ������ź���
		ulTaskNotifyTake(pdFALSE, portMAX_DELAY);			
		
		// ��ȡ�������ݣ��ȴ�����
		Drv_Gyro_Read(MID_GYRO.LatestData);

		MID_GYRO_RTT_printf(0,"Gyro: %d, %d, %d \r\n",MID_GYRO.LatestData[0],MID_GYRO.LatestData[1],MID_GYRO.LatestData[2]);
		
	}
}

void GyroTask_Create(void)
{
    if(pdPASS != xTaskCreate(GyroTask_Process, "GyroTask", TASK_STACKDEPTH_GYRO, NULL, TASK_PRIORITY_GYRO, &Gyro_TaskHandle))
	{
		MID_GYRO_RTT_printf(0,"GyroTask_Create Err \r\n");
	}
}
#endif




