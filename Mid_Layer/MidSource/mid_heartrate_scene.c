#include "platform_common.h"

/* FreeRTOS includes */
#include "rtos.h"

#include "mid_heartrate_scene.h"
#include "multimodule_task.h"

#if 1 //for compile
typedef enum 
{
    ON_TOUCH    = 1,
    OFF_TOUCH   = 0,
}bsp_touch_state_s;

#endif

/*************** func declaration ********************************/
static void HrmLogDuratonTimeOutProcess(TimerHandle_t xTimer);
static void HrmLogPeriodProcess(void);
static void HrmLogRestingJudge(uint16 totalStep);
static uint16 HrmLogOpen(void);
static uint16 HrmLogClose(void);


/*************** macro define ********************************/
#define 	LOG_STORAGE_RAM  			1

#define 	HEARTRATE_LOG_MAX_NUM 		3 //记录最近3次的测量结果
#define 	RESTTING_MAX_NUM 			3 //静息心率判断需要记录的个数
#define 	RESTING_HR_UP_MAX 			120 //静息心率上限
#define 	RESTING_HR_BIAS 			80  //静息心率偏差最大值
#define 	RESTING_TIME_VALID 			(3* LOG_RESTING_PERIOD)//30分钟静息，更新静息心率

#define 	LOG_HR_MAX_NUM 				5 //记录每次检测最近5个心率值
#define 	OFF_TOUCH_MAX				3 //心率监测无触摸检测最大确认次数


/************** variable define *****************************/
static  uint16 restingJudgePeriod; 				//静息判断周期	
static  uint16 restTime;						//连续静息时间
static 	uint8 restingHR;						//存储静息心率值，无运动下更新时间间隔30min	
static  uint8 restingCnt; 						//静息时间检测累积次数
static  uint16 stepLog; 						//计步（上次运动）状态
static 	uint8 restingHrTemp[RESTTING_MAX_NUM]; 	//静息心率预存值
static 	uint8 heartrateLogBuf[HEARTRATE_LOG_MAX_NUM];	//存储监测过程心率值，静息心率计算使用

#if (LOG_STORAGE_RAM == 1)
static 	uint8 LogHrBuf[LOG_HR_MAX_NUM];			//存储监测过程心率值，心率存储滤波处理用
#endif

static 	uint16 heartrateLogValidNum; 				//累计心率值
static 	uint8 logHR;
static  uint8 firstData;
static  uint8 LogHrBufCnt;
static  uint8 logHrStatus;
static  uint8 offTouchCnt;

TimerHandle_t HeartRateDuratonTimer;


/************** function define ******`***********************/

//**********************************************************************
// 函数功能：  心率监测时间到，心率关闭模块处理，启动心率存储处理
// 输入参数：   
// 返回参数：  无
static void HrmLogDuratonTimeOutProcess(TimerHandle_t xTimer)
{
	multimodule_task_msg_t msg;
	uint16 	i;
	uint8 	touchstate;
	uint8 	u8Temp;
	uint8   logHrMax;
	uint8   logHrMin;
	uint16  logAddTemp;


	logHrStatus  = HRM_LOG_NULL;	

  	Mid_Hrm_TouchStatusRead(&touchstate);
  	Mid_Hrm_Read(&u8Temp);
  	if (touchstate == OFF_TOUCH && offTouchCnt >= OFF_TOUCH_MAX)
  	{	
  		logHR 					= 0;		
  		//关闭心率
		msg.id                   = HEARTRATE_ID;     
	 	msg.module.hrmEvent.id   = HRM_STOP;
	  	MultiModuleTask_EventSet(msg); 
  		return;
  	}

	if(heartrateLogValidNum >= HEARTRATE_LOG_MAX_NUM)
	{
		for(i = 0; i < (HEARTRATE_LOG_MAX_NUM - 1); i++)
		{
			heartrateLogBuf[i]	= heartrateLogBuf[i + 1];
		}

		heartrateLogValidNum	= HEARTRATE_LOG_MAX_NUM;
		heartrateLogBuf[i]		= u8Temp;	//更新最新值			
	}
	else
	{
		heartrateLogBuf[heartrateLogValidNum]	= u8Temp;
		heartrateLogValidNum++;
	}

	//后台监测的心率作滤波处理
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

	//关闭心率
	msg.id                   = HEARTRATE_ID;     
 	msg.module.hrmEvent.id   = HRM_STOP;
  	MultiModuleTask_EventSet(msg); 
}

//**********************************************************************
// 函数功能：  周期性存储心率值，时间暂定1秒
// 输入参数：   
// 返回参数：  无
static void HrmLogStorageProcess(void)
{
	multimodule_task_msg_t msg;
	uint16 	i;
	uint8 	touchstate;
	uint8 	u8Temp;

	if (logHrStatus == HRM_LOG_ENVT)
	{
		Mid_Hrm_TouchStatusRead(&touchstate);
	  	Mid_Hrm_Read(&u8Temp);
	  	if (touchstate == OFF_TOUCH && offTouchCnt >= OFF_TOUCH_MAX)
	  	{
	  		logHR 					= 0;
	  		logHrStatus 			= HRM_LOG_NULL;	
	  		//关闭心率
			msg.id                  = HEARTRATE_ID;     
		 	msg.module.hrmEvent.id  = HRM_STOP;
		  	MultiModuleTask_EventSet(msg);

		  	xTimerStop(HeartRateDuratonTimer, 0);
		  	return; 
	  	}
	  	else
	  	{
	  		offTouchCnt ++;
	  	}
	  	//总是存储最新的5个值
		if(LogHrBufCnt >= LOG_HR_MAX_NUM)
		{
			for(i = 0; i < (LOG_HR_MAX_NUM - 1); i++)
			{
				LogHrBuf[i]	= LogHrBuf[i + 1];
			}

			LogHrBufCnt		= LOG_HR_MAX_NUM;
			LogHrBuf[i]		= u8Temp;	//更新最新值			
		}
		else
		{
			LogHrBuf[LogHrBufCnt]	= u8Temp;
			LogHrBufCnt++;
		}
	}	
}

//**********************************************************************
// 函数功能：  周期性处理,启动心率测量，启动心率监测定时器(作为后台心率周期性监测的启动)
// 输入参数：   
// 返回参数：  无
static void HrmLogPeriodProcess(void)
{
	multimodule_task_msg_t msg;
	
	msg.id                        = HEARTRATE_ID;       
	msg.module.hrmEvent.id        = HRM_START;
	MultiModuleTask_EventSet(msg);
	xTimerStart(HeartRateDuratonTimer, 1);

	//重新存储ｌｏｇ心率
	LogHrBufCnt = 0;
	offTouchCnt = 0;
	logHrStatus = HRM_LOG_ENVT;
}

//**********************************************************************
// 函数功能：周期性检测静息状态    
// 输入参数：
// totalStep ：计步总数   
// 返回参数：无
static void HrmLogRestingJudge(uint16 totalStep)
{	
	uint16 	stepTemp;
	uint8 	hrdelta;
	uint8 	hrMinTemp;
	uint16 	u16Temp = 0;
	uint8 	i;


	stepTemp 		= totalStep;

	if(firstData)
	{
		firstData	= 0;
		restTime 	= 0;
		stepLog 	= stepTemp;
	}
	else
	{
		// 静息判断
		if(stepTemp != stepLog)//运行产生
		{
			restTime		= 0;//静息打断，重新计时
			stepLog 		= stepTemp;
		}
		else
		{
			// 静息时间累加
			restTime++;
		}

		//达到有效log个数，记录一次静息心率
		if(heartrateLogValidNum >= HEARTRATE_LOG_MAX_NUM)
		{
			u16Temp = heartrateLogBuf[0];
			for (i = 1; i < HEARTRATE_LOG_MAX_NUM; i++)
			{
				if (u16Temp > heartrateLogBuf[i])
				{
					u16Temp = heartrateLogBuf[i];
				}
			}
			//取最新三个log值的最小值
			restingHrTemp[restingCnt] = u16Temp;
			u16Temp	  = 0;			
			restingCnt ++;
			//静息心率记录个数达到
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

			 	if (hrdelta > RESTING_HR_BIAS)//心率波动大，静息打断，重新计时
			 	{
			 		restTime		= 0;
					firstData 		= 1;
			 	}
				restingCnt = 0;
			} 	
		}					

		//静息心率测量确认成功，保存静息心率值，并重新启动一次静息心率判断
		if ((restTime * restingJudgePeriod) >= RESTING_TIME_VALID)
		{
			//取平均值 
			for (i = 0; i < RESTTING_MAX_NUM; i++)
			{		
				u16Temp += restingHrTemp[i];
			}
			hrMinTemp = u16Temp / RESTTING_MAX_NUM;

			if (hrMinTemp < RESTING_HR_UP_MAX)
			{
				if (hrMinTemp < restingHR)
				{
					restingHR 		= hrMinTemp;
				}				
				restingCnt 		= 0;
				restTime 		= 0;
				firstData 		= 1;
			}				
		}
	}
}

//**********************************************************************
// 函数功能：   心率记录功能初始化 
// 输入参数：   无
// 返回参数：	
// 0x00   :  操作成功
// 0xFF   :  操作失败
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

	HeartRateDuratonTimer 	= xTimerCreate("hrduratontimer", APP_1SEC_TICK * LOG_HEARTRATE_DURATON, pdFALSE, 0, HrmLogDuratonTimeOutProcess);//心率监测时间定时器设置为单次

	return 0;
}


//**********************************************************************
// 函数功能：   打开心率记录功能 
// 输入参数：   无
// 返回参数：
// 0x00   :  操作成功
// 0xFF   :  操作失败
static uint16 HrmLogOpen(void)
{
	multimodule_task_msg_t msg;
	uint16 i;

	restingCnt 				= 0;
	restingHR 				= 0xff;
	logHR 					= 0xff;
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

	//打开心率模块
	msg.id                                  = HEARTRATE_ID;       
	msg.module.hrmEvent.id                  = HRM_START;
	MultiModuleTask_EventSet(msg);

	//启动监测定时器
	xTimerReset(HeartRateDuratonTimer, 0);
	xTimerStart(HeartRateDuratonTimer, 0);

	return 0;
}


//**********************************************************************
// 函数功能： 停止心率记录功能 
// 输入参数： 无  
// 返回参数：
// 0x00   :  操作成功
// 0xFF   :  操作失败
static uint16 HrmLogClose(void)
{
	multimodule_task_msg_t msg;
	
	logHrStatus 			= HRM_LOG_NULL;

	msg.id                  = HEARTRATE_ID;       
	msg.module.hrmEvent.id  = HRM_STOP;
	MultiModuleTask_EventSet(msg);

	xTimerStop(HeartRateDuratonTimer, 0);

	return 0;
}

//**********************************************************************
// 函数功能:	心率记录场景事件处理
// 输入参数:	
// msg 	   :	心率记录事件信息
// 返回参数:	无
uint16 Mid_HeartRateScene_EventProcess(hrm_log_event_s* msg)
{
	switch(msg->id)
	{
		case HRM_LOG_OPEN:
		HrmLogOpen();
		break;

		case HRM_LOG_CLOSE:
		HrmLogClose();
		break;

		case HRM_LOG_STORAGE_PROCESS:
		HrmLogStorageProcess();
		break;

		case HRM_LOG_RESTING_JUDGE:
		HrmLogRestingJudge(msg->para);
		break;

		case HRM_LOG_PERIOND_PROCESS:
		HrmLogPeriodProcess();
		break;

		default:
		break;
	}
	return 0;
}

//**********************************************************************
// 函数功能:	读取心率值：log心率值、静息心率值
// 输入参数：		
// logheartrate:	 心率记录
// restingheartrate: 静息心率
// 返回参数：	无
uint16 Mid_HeartRateScene_LogHR_Read(uint8* logheartrate, uint8* restingheartrate)
{
	*logheartrate			= logHR;
	*restingheartrate		= restingHR;
	return 0;
}

//**********************************************************************
// 函数功能:	读取静息心率值
// 输入参数：	无
// 返回参数：	静息心率
uint8 Mid_HeartRateScene_RestingHR_Read(void)
{
	return restingHR;
}

//**********************************************************************
// 函数功能:	心率值记录清0
// 输入参数：	无
// 返回参数：	无
uint8 Mid_HeartRateScene_Clear(void)
{
	restingHR 				= 0xff;
	logHR 					= 0xff;
	return 0;
}
