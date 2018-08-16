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

static uint32 	stepOld  = 0;		// 1秒前的计步值，与当前计步值进行比较是否产生新的计步
static uint16 	step5minAcc = 0;	//5分钟计步值
static uint16 	stepSecAcc = 0;		//30秒计步值，低功耗处理使用
static uint16 	step10SecAcc = 0;		//10秒计步值，运动场景使用
static uint8  	secCnt  = 0;			
static uint8 	stepCompleteRemind;

static Mid_SportScene_Param_t	Mid_SportScene;

/************** function define *****************************/

//**********************************************************************
// 函数功能：   计步算法处理，读取重力数据回调处理
// 输入参数：   重力三轴数据
// 返回参数：	无
// 调用：25HZ频率进行调用
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
	
		//计步运动算法处理　
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
				
				if (stepCompleteRemind == 0)		//首次完成执行提醒
				{
					stepCompleteRemind = 1;
					
					/* 发送运动目标达成事件 */

				}
			}

			stepSportInfo.heatQuantity  = stepSportInfo.totalStep * ((bodyInfo.bodyWeight - CALORIECON0) * CALORIECON1 + CALORIECON2);
			stepSportInfo.sportDistance = bodyInfo.bodyHeight * DISTANCECON0 * stepSportInfo.totalStep / 100 ;
		}
		//游泳运动算法处理
	//	swimming_algorithm(xyzData); 		
	}
}

//**********************************************************************
// 函数功能：   计步运动信息初始化
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
// 调用：初始化阶段调用一次
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
// 函数功能：   计步运动开启
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
// 调用：需开启计步功能时
void Mid_SportScene_Start(void)
{
	Mid_Accel_ParamSet(eMidAccelSampleRate25HZ, eMidAccelSampleRange2G);
	Mid_Accel_StartSample();	
	Mid_SportScene.EnableFlg = true;
}

//**********************************************************************
// 函数功能：   计步运动关闭
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
// 调用：需关闭计步功能时
void Mid_SportScene_Stop(void)
{
	Mid_Accel_StopSample();
	Mid_SportScene.EnableFlg = false;
}

//**********************************************************************
// 函数功能：   运动信息清零
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
// 调用：每天调用一次
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
// 函数功能：   读取计步运动记录信息
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
// 调用：外部
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
// 调用：外部
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
// 调用：外部
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
// 调用：外部，收到设置步数指令或者数据恢复
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
   	 		
			/* 发送运动目标达成事件 */
			
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
// 调用：外部
uint16 Mid_SportScene_StepAimRead(uint32 *stepAim)
{
	*stepAim = stepSportInfo.stepAim;

    return 0;
}

//**********************************************************************
// 函数功能：   计步运动时长累加处理，每1秒调用一次
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
// 调用：每秒调用一次，不可1min调用一次。因为只有产生计步才算有效运动
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
// 函数功能：   获取身体信息
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
// 调用：外部
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
// 调用：外部，收到协议指令或者数据恢复
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
// 调用：外部，每5分钟存储一次计步值
uint16 Mid_SportScene_5minStepRead(void)
{
	return step5minAcc;
}

//**********************************************************************
// 函数功能：   清除5分钟计步累加值
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
// 调用：存储后调用此函数清除
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
// 调用：外部，计步准备状态下，计时30秒，检查是否有计步产生,切换到计步或空闲状态
uint16 Mid_SportScene_SecStepRead(void)
{
	return stepSecAcc;
}

//**********************************************************************
// 函数功能：   清除5分钟计步累加值
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
// 调用：外部，计步准备状态下，计时30秒，检查是否有计步产生,切换到计步或空闲状态
uint16 Mid_SportScene_SecStepClear(void)
{
	stepSecAcc = 0;
	
	return 0;
}

//**********************************************************************
// 函数功能：   获取10秒计步累加值
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
// 调用：用于跑步场景
uint16 Mid_SportScene_10SecStepRead(void)
{
	return step10SecAcc;
}

//**********************************************************************
// 函数功能：   清除10秒计步累加值
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
uint16 Mid_SportScene_10SecStepClear(void)
{
	step10SecAcc = 0;
	
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

