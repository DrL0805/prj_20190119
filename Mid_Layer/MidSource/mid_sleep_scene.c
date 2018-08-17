
#include "mid_accelerate.h"
#include "mid_sleep_scene.h"


Mid_SleepScene_Param_t	Mid_SleepScene;

//**********************************************************************
// 函数功能：   睡眠算法处理，读取重力数据回调处理
// 输入参数：   重力三轴数据
// 返回参数：	无
// 调用：睡眠算法，1秒1次
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
// 函数功能：   睡眠信息初始化
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
// 调用：初始化调用一次
uint16 Mid_SleepScene_Init(void)
{
	sleep_algorithm_init();
	return 0;
}

//**********************************************************************
// 函数功能：   睡眠信息获取——供存储
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
// 调用：需存储睡眠数据时，1小时调用一次
uint16 Mid_SleepScene_GetRecord(sleep_data *sleepDataTemp, uint8 *validnum)
{
	*validnum = get_sleep_recode(sleepDataTemp);
	
	return 0;
}

//**********************************************************************
// 函数功能：   本地获取睡眠最新信息
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
// 调用：外部，用于UI交互
uint16 Mid_SleepScene_CurSleepGet(sleep_ui_info *sleepInfoTemp)
{
	*sleepInfoTemp = get_sleep_info();
	return 0;
}

//**********************************************************************
// 函数功能：   睡眠场景开启
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
void Mid_SleepScene_Start(void)
{
	Mid_Accel_StartSample(&Mid_SleepScene.SampleId, eMidAccelSampleRate1HZ, eMidAccelSampleRange2G);
	
	Mid_SleepScene.IntervalMs = 0;
	Mid_SleepScene.EnableFlg = true;
}

//**********************************************************************
// 函数功能：    睡眠场景开启
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
void Mid_SleepScene_Stop(void)
{
	Mid_Accel_StopSample(Mid_SleepScene.SampleId);
	
	Mid_SleepScene.EnableFlg = false;
}





