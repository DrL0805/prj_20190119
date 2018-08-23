#include "scene_sedentary.h"
#include "mid_accelerate.h"
#include "main.h"

Scene_Sedentary_Param_t	Scene_Sedentary;

//**********************************************************************
// 函数功能：   算法处理，读取重力数据回调处理
// 输入参数：   重力三轴数据
// 返回参数：	无
// 调用：睡眠算法，1秒1次
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

		/* 久坐算法 */
		Mid_SportScene_StepRead(&tToTalStep);
		alg_sedentary_process(xyzData, tToTalStep);
		
		// 获取久坐时间
		tSetTime = alg_sedentary_get_time();
		SCENE_SEDENTARY_RTT_LOG(0, "tSetTime %d \r\n", tSetTime);
	}
}


//**********************************************************************
// 函数功能：   初始化
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
// 调用：初始化调用一次
uint16 Scene_Sedentary_Init(void)
{
	uint32 tToTalStep;
	
	Mid_SportScene_StepRead(&tToTalStep);
	alg_sedentary_init(tToTalStep);
	return 0;
}

//**********************************************************************
// 函数功能：   
// 输入参数：   
// 返回参数：	
void Scene_Sedentary_Start(void)
{
	Mid_Accel_StartSample(&Scene_Sedentary.SampleId, eMidAccelSampleRate1HZ, eMidAccelSampleRange2G);
	
	Scene_Sedentary.IntervalMs = 0;
	Scene_Sedentary.EnableFlg = true;
}


//**********************************************************************
// 函数功能：   
// 输入参数：
// 返回参数:
void Scene_Sedentary_Stop(void)
{
	Mid_Accel_StopSample(Scene_Sedentary.SampleId);
	
	Scene_Sedentary.EnableFlg = false;
}






















