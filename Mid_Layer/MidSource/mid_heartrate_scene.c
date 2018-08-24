#include "platform_common.h"

/* FreeRTOS includes */
#include "rtos.h"
#include "mid_heartrate_scene.h"
#include "main.h"

//#if 1 //for compile
//typedef enum 
//{
//    ON_TOUCH    = 1,
//    OFF_TOUCH   = 0,
//}bsp_touch_state_s;

//#endif

/*************** macro define ********************************/
#define 	HEARTRATE_LOG_MAX_NUM 		3 //��¼���3�εĲ������
#define 	RESTTING_MAX_NUM 			3 //��Ϣ�����ж���Ҫ��¼�ĸ���
#define 	RESTING_HR_UP_MAX 			120 //��Ϣ��������
#define 	RESTING_HR_BIAS 			80  //��Ϣ����ƫ�����ֵ
#define 	RESTING_TIME_VALID 			(3* LOG_RESTING_PERIOD)//30���Ӿ�Ϣ�����¾�Ϣ����

#define 	LOG_HR_MAX_NUM 				5 //��¼ÿ�μ�����5������ֵ
#define 	OFF_TOUCH_MAX				3 //���ʼ���޴���������ȷ�ϴ���


/************** variable define *****************************/
static  uint16 restingJudgePeriod; 				//��Ϣ�ж�����	
static  uint16 restTime;						//������Ϣʱ��
static 	uint8 restingHR;						//�洢��Ϣ����ֵ�����˶��¸���ʱ����30min	
static  uint8 restingCnt; 						//��Ϣʱ�����ۻ�����
static  uint16 stepLog; 						//�Ʋ����ϴ��˶���״̬
static 	uint8 restingHrTemp[RESTTING_MAX_NUM]; 	//��Ϣ����Ԥ��ֵ
static 	uint8 heartrateLogBuf[HEARTRATE_LOG_MAX_NUM];	//�洢����������ֵ����Ϣ���ʼ���ʹ��

static 	uint8 LogHrBuf[LOG_HR_MAX_NUM];			//�洢����������ֵ�����ʴ洢�˲�������

static 	uint16 heartrateLogValidNum; 				//�ۼ�����ֵ
static 	uint8 logHR;
static  uint8 firstData;
static  uint8 LogHrBufCnt;		// log���ʴ洢���������µ�5������ֵ������ȡƽ��ֵ��Ϊ���ʴ洢���ϴ���log
static  uint8 logHrStatus;		// ���ʳ������أ��򿪻��߹ر�
static  uint8 offTouchCnt;		// ����δ��⵽����ʱ�������ÿ��+1

TimerHandle_t HeartRateDuratonTimer;


/************** function define ******`***********************/

//**********************************************************************
// �������ܣ�  ���ʼ��ʱ�䵽�����ʹر�ģ�鴦���������ʴ洢����
// ���������   
// ���ز�����  ��
// ���ã����ʶ�ʱ����ʱ�ص�����
static void HrmLogDuratonTimeOutProcess(TimerHandle_t xTimer)
{
	uint16 	i;
	uint8 	touchstate;
	uint8 	u8Temp;
	uint8   logHrMax;
	uint8   logHrMin;
	uint16  logAddTemp;

//	SCENE_HRM_RTT_LOG(0, "HrmLogDuratonTimeOutProcess \r\n");
	
	logHrStatus  = HRM_LOG_NULL;	

  	Mid_Hrm_TouchStatusRead(&touchstate);
  	Mid_Hrm_Read(&u8Temp);
  	if (touchstate == OFF_TOUCH && offTouchCnt >= OFF_TOUCH_MAX)
  	{	
  		logHR 					= 0;		
  		//�ر�����
		HrmStop();
  		return;
  	}

	// ���Ѵ洢3������ֵ��FIFO�滻�������Ѵ洢������ֵ+1
	if(heartrateLogValidNum >= HEARTRATE_LOG_MAX_NUM)
	{
		for(i = 0; i < (HEARTRATE_LOG_MAX_NUM - 1); i++)
		{
			heartrateLogBuf[i]	= heartrateLogBuf[i + 1];
		}

		heartrateLogValidNum	= HEARTRATE_LOG_MAX_NUM;
		heartrateLogBuf[i]		= u8Temp;	//��������ֵ			
	}
	else
	{
		heartrateLogBuf[heartrateLogValidNum]	= u8Temp;
		heartrateLogValidNum++;
	}

	//��̨�����������˲�����
	logHrMax   = LogHrBuf[0];
	logHrMin   = LogHrBuf[0];
	logAddTemp = 0;

	for(i = 0; i < LOG_HR_MAX_NUM; i++)
	{
		if (LogHrBuf[i] > logHrMax)
		{
			logHrMax = LogHrBuf[i];
		}

		if (LogHrBuf[i] < logHrMin)
		{
			logHrMin = LogHrBuf[i];
		}
		logAddTemp += LogHrBuf[i];
	}
	logHR 	= (logAddTemp - logHrMax - logHrMin) / (LOG_HR_MAX_NUM - 2);

	//�ر�����
	HrmStop(); 
}

//**********************************************************************
// �������ܣ�  �����Դ洢����ֵ��ʱ���ݶ�1��
// ���������   
// ���ز�����  ��
// ���ã�ÿ��һ��
void HrmLogStorageProcess(void)
{
	uint16 	i;
	uint8 	touchstate;
	uint8 	u8Temp;

	if (logHrStatus == HRM_LOG_ENVT)
	{
		Mid_Hrm_TouchStatusRead(&touchstate);
	  	Mid_Hrm_Read(&u8Temp);
	  	if ((touchstate == OFF_TOUCH) && (offTouchCnt >= OFF_TOUCH_MAX))
	  	{
	  		logHR 					= 0;
	  		logHrStatus 			= HRM_LOG_NULL;	
	  		
			//�ر�����
			HrmStop();

		  	xTimerStop(HeartRateDuratonTimer, 0);
		  	return; 
	  	}
	  	else
	  	{
	  		offTouchCnt ++;
	  	}
	  	//���Ǵ洢���µ�5��ֵ
		if(LogHrBufCnt >= LOG_HR_MAX_NUM)
		{
			for(i = 0; i < (LOG_HR_MAX_NUM - 1); i++)
			{
				LogHrBuf[i]	= LogHrBuf[i + 1];
			}

			LogHrBufCnt		= LOG_HR_MAX_NUM;
			LogHrBuf[i]		= u8Temp;	//��������ֵ			
		}
		else
		{
			LogHrBuf[LogHrBufCnt]	= u8Temp;
			LogHrBufCnt++;
		}
	}
}

//**********************************************************************
// �������ܣ�  �����Դ���,�������ʲ������������ʼ�ⶨʱ��(��Ϊ��̨���������Լ�������)
// ���������   
// ���ز�����  ��
// ���ã�ÿmin����һ��
void HrmLogPeriodProcess(void)
{
	static uint8_t stMinCnt = 0;
	
	/* 2min����һ��	*/
	if(++stMinCnt >= 2)
	{
		stMinCnt = 0;
		
		HrmStart();
		xTimerStart(HeartRateDuratonTimer, 1);

		//���´洢��������
		LogHrBufCnt = 0;
		offTouchCnt = 0;
		logHrStatus = HRM_LOG_ENVT;		
	}
}

//**********************************************************************
// �������ܣ������Լ�⾲Ϣ״̬    
// ���������
// totalStep ���Ʋ�����   
// ���ز�������
// ���ã�ÿ6min����һ��
void HrmLogRestingJudge(uint16 totalStep)
{	
	uint16 	stepTemp;
	uint8 	hrdelta;
	uint8 	hrMinTemp;
	uint16 	u16Temp = 0;
	uint8 	i;
	static uint8_t stMinCnt = 0;
	
	/* 6min����һ��	*/
	if(++stMinCnt >= 6)
	{
		stMinCnt = 0;

		stepTemp 		= totalStep;

		if(firstData)
		{
			firstData	= 0;
			restTime 	= 0;
			stepLog 	= stepTemp;
		}
		else
		{
			// ��Ϣ�ж�
			if(stepTemp != stepLog)//���в���
			{
				restTime		= 0;//��Ϣ��ϣ����¼�ʱ
				stepLog 		= stepTemp;
			}
			else
			{
				// ��Ϣʱ���ۼ�
				restTime++;
			}

			//�ﵽ��Чlog��������¼һ�ξ�Ϣ����
			if(heartrateLogValidNum >= HEARTRATE_LOG_MAX_NUM)
			{
				//ȡ��������logֵ����Сֵ
				u16Temp = heartrateLogBuf[0];
				for (i = 1; i < HEARTRATE_LOG_MAX_NUM; i++)
				{
					if (u16Temp > heartrateLogBuf[i])
					{
						u16Temp = heartrateLogBuf[i];
					}
				}
				//��Ϣ���ʼ�¼�����ﵽ
				if (restingCnt >= RESTTING_MAX_NUM)
				{
					if (restingHrTemp[RESTTING_MAX_NUM - 1] >= restingHrTemp[0])
					{
						hrdelta 	= restingHrTemp[RESTTING_MAX_NUM - 1] - restingHrTemp[0];
					}
					else
					{
						hrdelta 	= restingHrTemp[0] - restingHrTemp[RESTTING_MAX_NUM - 1];
					}

					if (hrdelta > RESTING_HR_BIAS)//���ʲ����󣬾�Ϣ��ϣ����¼�ʱ
					{
						restTime			 = 0;
						firstData 			 = 1;
						restingCnt 			 = 0; 
						heartrateLogValidNum = 0;
					}
					else
					{
						//��Ϣ���ʲ���ȷ�ϳɹ������澲Ϣ����ֵ������������һ�ξ�Ϣ�����ж�
						if ((restTime * restingJudgePeriod) >= RESTING_TIME_VALID)
						{
							u16Temp	  = 0;
							//ȡƽ��ֵ 
							for (i = 0; i < RESTTING_MAX_NUM; i++)
							{		
								u16Temp += restingHrTemp[i];
							}
							hrMinTemp = u16Temp / RESTTING_MAX_NUM;

							if (hrMinTemp < RESTING_HR_UP_MAX)
							{
								if (hrMinTemp < 30)
								{
									hrMinTemp = 30;
								}
								if (hrMinTemp < restingHR)
								{
									restingHR 		= hrMinTemp;
								}				
							}
							restTime 		= 0;								
							firstData 		= 1;
							restingCnt 		= 0;
							heartrateLogValidNum = 0;				
						}
					}
				}
				else
				{
					restingHrTemp[restingCnt] = u16Temp;		
					restingCnt ++;
				} 	
			}
		}		
	}
}

//**********************************************************************
// �������ܣ�   ���ʼ�¼���ܳ�ʼ�� 
// ���������   ��
// ���ز�����	
// 0x00   :  �����ɹ�
// 0xFF   :  ����ʧ��
// ���ã���ʼ���׶ε���һ��
uint16 Mid_HeartRateScene_Init(void)
{
	uint16 i;
	
	restingCnt 				= 0;
	restingHR 				= 0xff;
	logHR 					= 0xff;
	restTime 				= 0;
	stepLog 				= 0;
	firstData 				= 1;
	heartrateLogValidNum 	= 0;
	restingJudgePeriod 		= LOG_RESTING_PERIOD;
	LogHrBufCnt 			= 0;
	offTouchCnt 			= 0;
	logHrStatus 			= HRM_LOG_NULL;
	
	for(i = 0; i < 3; i ++)
	{
		restingHrTemp[i] = 0;
	}
	
	for(i = 0;  i< HEARTRATE_LOG_MAX_NUM; i++)
	{
		heartrateLogBuf[i] = 0;
	}

	for(i = 0;  i< LOG_HR_MAX_NUM; i++)
	{
		LogHrBuf[i] = 0;
	}

	HeartRateDuratonTimer 	= xTimerCreate("hrduratontimer", APP_1SEC_TICK * LOG_HEARTRATE_DURATON, pdFALSE, 0, HrmLogDuratonTimeOutProcess);//���ʼ��ʱ�䶨ʱ������Ϊ����

	return 0;
}


//**********************************************************************
// �������ܣ�   �����ʼ�¼���� 
// ���������   ��
// ���ز�����
// 0x00   :  �����ɹ�
// 0xFF   :  ����ʧ��
// ���ã����ʼ�¼��������ʱ����
uint16 HrmLogOpen(void)
{
	uint16 i;

	restingCnt 				= 0;
	// restingHR 				= 0xff;//�����þ�Ϣ����
	logHR 					= 0;
	restTime 				= 0;
	stepLog 				= 0;
	firstData 				= 1;
	heartrateLogValidNum 	= 0;
	LogHrBufCnt				= 0;
	offTouchCnt 			= 0;
	logHrStatus 			= HRM_LOG_ENVT;

	
	for(i = 0; i < 3; i ++)
	{
		restingHrTemp[i] = 0;
	}
	
	for(i = 0; i< HEARTRATE_LOG_MAX_NUM; i++)
	{
		heartrateLogBuf[i] = 0;
	}

	//������ģ��
	HrmStart();

	//������ⶨʱ��
	xTimerReset(HeartRateDuratonTimer, 0);
	xTimerStart(HeartRateDuratonTimer, 0);

	return 0;
}


//**********************************************************************
// �������ܣ� ֹͣ���ʼ�¼���� 
// ��������� ��  
// ���ز�����
// 0x00   :  �����ɹ�
// 0xFF   :  ����ʧ��
// ���ã����ʼ�¼�����ر�ʱ����
uint16 HrmLogClose(void)
{
	logHrStatus 			= HRM_LOG_NULL;

	HrmStop();

	xTimerStop(HeartRateDuratonTimer, 0);

	return 0;
}

//**********************************************************************
// ��������:	��ȡ����ֵ��log����ֵ����Ϣ����ֵ
// ���������		
// logheartrate:	 ���ʼ�¼
// restingheartrate: ��Ϣ����
// ���ز�����	��
// ���ã���洢����log����ʱ���ã�ÿ2minһ��
uint16 Mid_HeartRateScene_LogHR_Read(uint8* logheartrate, uint8* restingheartrate)
{
	*logheartrate			= logHR;
	*restingheartrate		= restingHR;
	return 0;
}

//**********************************************************************
// ��������:	��ȡ��Ϣ����ֵ
// ���������	��
// ���ز�����	��Ϣ����
// ���ã�����ʾ��Ϣ����ʱ����
uint8 Mid_HeartRateScene_RestingHR_Read(void)
{
	return restingHR;
}

//**********************************************************************
// ��������:	����ֵ��¼��0
// ���������	��
// ���ز�����	��
// ���ã�ÿ�����һ�Σ�����������ݣ������
uint8 Mid_HeartRateScene_Clear(void)
{
	restingHR 				= 0xff;
	logHR 					= 0xff;
	return 0;
}
