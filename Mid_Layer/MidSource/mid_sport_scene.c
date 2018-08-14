#include "platform_common.h"
/* FreeRTOS includes */
#include "rtos.h"
#include "mid_sport_scene.h"
#include "multimodule_task.h"


/*************** func declaration ********************************/
static void 	StepSportCnt(int16 *xyzData);
static uint16 SportDuarationProcess(void);
static uint16 StepSportStart(void);
static uint16 StepSportStop(void);

static void   (*Mid_SportScene_StateCb)(uint16 status);

/*************** macro define ********************************/
#define CALORIECON0 	13.63636
#define CALORIECON1 	0.000693
#define CALORIECON2 	0.00495

#define DISTANCECON0 	0.45


/************** variable define *****************************/
static stepSportInfo_s stepSportInfo;
static bodyInfo_s 	   bodyInfo;

static uint32 	stepOld  = 0;
static uint16 	step5minAcc = 0;	//5分钟计步值
static uint16 	stepSecAcc = 0;		//30秒计步值
static uint8  	secCnt  = 0;
static uint8 	stepCompleteRemind;
	
static uint16     stepAccelReadId;



/************** function define *****************************/

//**********************************************************************
// 函数功能：   计步算法处理，读取重力数据回调处理
// 输入参数：   重力三轴数据
// 返回参数：	无
static void StepSportCnt(int16 *xyzData)
{
    uint8     temp;
    uint8 	i;
	int8  	G_sensorBuf[3];

	//计步运动算法处理　
	for(i = 0; i < 3; i ++)
	{		
		G_sensorBuf[i]	= (xyzData[i]>>8);
	}
	temp  = Algorithm_Calculate_Step((uint8*)G_sensorBuf);
	
	if(temp == 1 || temp == 12)
	{
		stepSportInfo.totalStep      += temp;
		step5minAcc += temp;
		stepSecAcc += temp;

		stepSportInfo.stepComplete   = (uint16)(stepSportInfo.totalStep * 100 / stepSportInfo.stepAim);
		if(stepSportInfo.stepComplete >= 100)
		{
			if(stepSportInfo.stepComplete >= 999)
			{
				stepSportInfo.stepComplete = 999;
			}
			
			if (stepCompleteRemind == 0)		//首次完成执行提醒
	   	 	{
	   	 		stepCompleteRemind = 1;
	   	 		if (Mid_SportScene_StateCb != NULL)
	   	 		{
	   	 			Mid_SportScene_StateCb(STEP_COMPLETE);
	   	 		}
	   	 	}
		}

		stepSportInfo.heatQuantity  = stepSportInfo.totalStep * ((bodyInfo.bodyWeight - CALORIECON0) * CALORIECON1 + CALORIECON2);
		stepSportInfo.sportDistance = bodyInfo.bodyHeight * DISTANCECON0 * stepSportInfo.totalStep / 100 ;
	}
}

//**********************************************************************
// 函数功能：   计步运动信息初始化
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
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

	Mid_SportScene_StateCb 			=NULL;

	return 0;
}

//**********************************************************************
// 函数功能：   运动信息清零
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
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
// 函数功能：   运动状态反馈回调注册
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
uint16 Mid_SportScene_SportStateCbSet(void (*cb)(uint16 status))
{
	Mid_SportScene_StateCb = cb;
	return 0;
}
//**********************************************************************
// 函数功能：   计步运动开启
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
static uint16 StepSportStart(void)
{
	multimodule_task_msg_t  msg;
    msg.id                            = ACCEL_ID;
    msg.module.accelEvent.id          = ACCEL_READ_SET;
    msg.module.accelEvent.readId      = &stepAccelReadId;
    msg.module.accelEvent.rate        = ACCEL_25HZ;
    msg.module.accelEvent.scaleRange  = ACCEL_2G;
    msg.module.accelEvent.Cb          = StepSportCnt;
    MultiModuleTask_EventSet(msg);

    return 0;
}

//**********************************************************************
// 函数功能：   计步运动关闭
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
static uint16 StepSportStop(void)
{
	multimodule_task_msg_t  msg;

    msg.id                            = ACCEL_ID;
    msg.module.accelEvent.id          = ACCEL_READ_DELETE;
    msg.module.accelEvent.readId      = &stepAccelReadId;
    MultiModuleTask_EventSet(msg);

    return 0;
}

//**********************************************************************
// 函数功能：   读取计步运动记录信息
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
uint16 Mid_SportScene_SportInfoRead(stepSportInfo_s *stepsportinfo)
{
	*stepsportinfo 					= stepSportInfo;
	return 0;
}

//**********************************************************************
// 函数功能：   读取计步数
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
uint16 Mid_SportScene_StepRead(uint32 *step)
{
	*step 		= stepSportInfo.totalStep;
	return 0;
}

//**********************************************************************
// 函数功能：   计步数恢复
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
uint16 Mid_SportScene_StepWrite(uint32 *step)
{
	stepSportInfo.totalStep = *step;
	stepSportInfo.heatQuantity  = stepSportInfo.totalStep * ((bodyInfo.bodyWeight - CALORIECON0) * CALORIECON1 + CALORIECON2);
	stepSportInfo.sportDistance = bodyInfo.bodyHeight * DISTANCECON0 * stepSportInfo.totalStep / 100 ;

	return 0;
}
//**********************************************************************
// 函数功能：   设置计步运动目标步数
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
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
			
   	 	if (stepCompleteRemind == 0)		//首次完成执行提醒
   	 	{
   	 		stepCompleteRemind = 1;
   	 		if (Mid_SportScene_StateCb != NULL)
   	 		{
   	 			Mid_SportScene_StateCb(STEP_COMPLETE);
   	 		}
   	 	}
    }
    else
    {
    	stepCompleteRemind = 0;				//标记清零
    }

    return 0;
}

//**********************************************************************
// 函数功能：   获取计步运动目标步数
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
uint16 Mid_SportScene_StepAimRead(uint32 *stepAim)
{
	*stepAim = stepSportInfo.stepAim;

    return 0;
}

//**********************************************************************
// 函数功能：   计步运动记录信息复位（清零）
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
uint16 Mid_SportScene_SportInfoReset(void)
{
	stepSportInfo.totalStep 		= 0;
	stepSportInfo.stepComplete 		= 0;
	stepSportInfo.stepAim 			= 0xffffffff;

	stepSportInfo.heatQuantity 		= 0;
	stepSportInfo.sportDistance 	= 0;

	stepSportInfo.sportDuaration 	= 0;
	
	secCnt 							= 0;
	stepOld							= 0;
	
	return 0;
}

//**********************************************************************
// 函数功能：   计步运动时长累加处理，每1秒调用一次
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
static uint16 SportDuarationProcess(void)
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
// 函数功能：   获取身体信息
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
uint16 Mid_SportScene_BodyInfoRead(bodyInfo_s *bodyinfo)
{
	bodyinfo->bodyWeight 	= bodyInfo.bodyWeight;
	bodyinfo->bodyHeight 	= bodyInfo.bodyHeight;
	bodyinfo->sex 			= bodyInfo.sex;
	bodyinfo->age 			= bodyInfo.age;

	
	return 0;
}

//**********************************************************************
// 函数功能：   设置身体信息
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
uint16 Mid_SportScene_BodyInfoSet(bodyInfo_s *bodyinfo)
{	
	bodyInfo.bodyWeight 		= bodyinfo->bodyWeight;
	bodyInfo.bodyHeight 		= bodyinfo->bodyHeight;
	bodyInfo.sex 				= bodyinfo->sex;;
	bodyInfo.age 				= bodyinfo->age;;

	return 0;
}

//**********************************************************************
// 函数功能：   获取5分钟计步累加值
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
uint16 Mid_SportScene_5minStepRead(void)
{
	return step5minAcc;
}

//**********************************************************************
// 函数功能：   清除5分钟计步累加值
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
uint16 Mid_SportScene_5minStepClear(void)
{
	step5minAcc = 0;
	
	return 0;
}

//**********************************************************************
// 函数功能：   获取5分钟计步累加值
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
uint16 Mid_SportScene_SecStepRead(void)
{
	return stepSecAcc;
}

//**********************************************************************
// 函数功能：   清除5分钟计步累加值
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
uint16 Mid_SportScene_SecStepClear(void)
{
	stepSecAcc = 0;
	
	return 0;
}

//**********************************************************************
// 函数功能：   获取运动时间
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
uint16 Mid_SportScene_SportDuarationRead(uint32 *duaration)
{
	*duaration = stepSportInfo.sportDuaration;
	return 0;
}

//**********************************************************************
// 函数功能：   恢复运动时间
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
uint16 Mid_SportScene_SportDuarationWrite(uint32 *duaration)
{
	stepSportInfo.sportDuaration = *duaration;
	return 0;
}
//**********************************************************************
// 函数功能:	事件处理
// 输入参数:	
// msg 	   :	事件信息
// 返回参数:	无
uint16 Mid_SportScene_EventProcess(sport_event_s* msg)
{
	switch(msg->id)
	{
		case SPORT_STEP_START:
		StepSportStart();
		break;

		case SPORT_STEP_STOP:
		StepSportStop();
		break;

		case SPORT_STEP_DUARATION_PROCESS:
		SportDuarationProcess();
		break;

		default:
		break;
	}
	return 0;
}

