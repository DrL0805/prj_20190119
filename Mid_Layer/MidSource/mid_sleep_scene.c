#include "platform_common.h"
/* FreeRTOS includes */
#include "rtos.h"
#include "algorithm_sleep.h"
#include "mid_sleep_scene.h"
#include "multimodule_task.h"





/*************** func declaration ********************************/
static void   Sleep_algorithm_process(int16 *xyzData);
static uint16 Mid_SleepScene_Start(void);
static uint16 Mid_SleepScene_Stop(void);

/*************** macro define ********************************/



/************** variable define *****************************/	
static uint16     		sleepAccelReadId;



/************** function define *****************************/

//**********************************************************************
// 函数功能：   睡眠算法处理，读取重力数据回调处理
// 输入参数：   重力三轴数据
// 返回参数：	无
static void Sleep_algorithm_process(int16 *xyzData)
{
   	sleep_algorithm(xyzData);
}

//**********************************************************************
// 函数功能：   睡眠信息初始化
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
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
static uint16 Mid_SleepScene_Start(void)
{
	multimodule_task_msg_t  msg;

    msg.id                            = ACCEL_ID;
    msg.module.accelEvent.id          = ACCEL_READ_SET;
    msg.module.accelEvent.readId      = &sleepAccelReadId;
    msg.module.accelEvent.rate        = ACCEL_1HZ;
    msg.module.accelEvent.scaleRange  = ACCEL_2G;
    msg.module.accelEvent.Cb          = Sleep_algorithm_process;
    MultiModuleTask_EventSet(msg);

    return 0;
}

//**********************************************************************
// 函数功能：    睡眠场景开启
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
static uint16 Mid_SleepScene_Stop(void)
{
	multimodule_task_msg_t  msg;

    msg.id                            = ACCEL_ID;
    msg.module.accelEvent.id          = ACCEL_READ_DELETE;
    msg.module.accelEvent.readId      = &sleepAccelReadId;
    MultiModuleTask_EventSet(msg);

    return 0;
}

//**********************************************************************
// 函数功能:	事件处理
// 输入参数:	
// msg 	   :	事件信息
// 返回参数:	无
uint16 Mid_SleepScene_EventProcess(sleep_scene_event_s* msg)
{
	switch(msg->id)
	{
		case SLEEP_SCENE_START:
		Mid_SleepScene_Start();
		break;

		case SLEEP_SCENE_STOP:
		Mid_SleepScene_Stop();
		break;

		case SLEEP_SCENE_PROCESS:
		break;

		default:
		break;
	}
	return 0;
}

