#include "platform_common.h"
/* FreeRTOS includes */
#include "rtos.h"
#include "mid_scene.h"
#include "multimodule_task.h"




/*************** func declaration ********************************/
static uint16 Mid_RunScene_Open(void);
static uint16 Mid_RunScene_Close(void);
static uint16 Mid_RunScene_Process(void);

static uint16 Mid_ClimbScene_Open(void);
static uint16 Mid_ClimbScene_Close(void);
static uint16 Mid_ClimbScene_Process(void);

static uint16 Mid_SwingScene_Open(void);
static uint16 Mid_SwingScene_Close(void);
static uint16 Mid_SwingScene_Process(void);


/*************** macro define ********************************/



/************** variable define *****************************/	
static 	uint8  scenePressorId;

static  scene_detail_s  sceneDetail;
static  scene_data_s  	sceneDataLog;


/************** function define *****************************/

//**********************************************************************
// 函数功能：   场景初始化
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
uint16 Mid_Scene_Init(void)
{
	memset(&sceneDetail,0,sizeof(scene_detail_s));
	memset(&sceneDataLog,0,sizeof(scene_data_s));

	return 0;
}

//**********************************************************************
// 函数功能：   跑步场景开启
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
static uint16 Mid_RunScene_Open(void)
{
	multimodule_task_msg_t  msg;
	bodyInfo_s infoTemp;
	bodyInfo_A bodyInfo;
	uint32  	utcTemp;
	uint32 		stepTemp;

	
	utcTemp = Mid_Rtc_ReadCurUtc();
	Mid_SportScene_BodyInfoRead(&infoTemp);
	Mid_SportScene_StepRead(&stepTemp);

	bodyInfo.Weight = infoTemp.bodyWeight;
	bodyInfo.Height = infoTemp.bodyHeight;
	bodyInfo.Sex 	= infoTemp.sex;
	bodyInfo.Age 	= infoTemp.age;

	running_algorithm_init(stepTemp, bodyInfo, utcTemp);
	memset(&sceneDetail,0,sizeof(scene_detail_s));
	memset(&sceneDataLog,0,sizeof(scene_data_s));
	//打开心率模块
	msg.id                                  = HEARTRATE_ID;       
	msg.module.hrmEvent.id                  = HRM_START;
	MultiModuleTask_EventSet(msg);

    return 0;
}

//**********************************************************************
// 函数功能：    跑步场景关闭
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
static uint16 Mid_RunScene_Close(void)
{
	multimodule_task_msg_t  msg;
	
	//关闭心率
	msg.id                                  = HEARTRATE_ID;     
 	msg.module.hrmEvent.id                  = HRM_STOP;
  	MultiModuleTask_EventSet(msg); 

    return 0;
}

//**********************************************************************
// 函数功能：    跑步场景处理
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
static uint16 Mid_RunScene_Process(void)
{
	runningInfo_A curRunInfoTemp;
	uint32 		stepTemp;
	uint8 		sceneHrm;


	Mid_Hrm_Read(&sceneHrm);
	Mid_SportScene_StepRead(&stepTemp);

	if (sceneHrm == 0xff)
	{
		sceneHrm = 0;
	}
	
	curRunInfoTemp =  running_algorithm(stepTemp, sceneHrm);

	sceneDetail.runDetail.startUtc      = curRunInfoTemp.StartUTC;
	sceneDetail.runDetail.duarationTotal = curRunInfoTemp.Time;
	sceneDetail.runDetail.distanceTotal = curRunInfoTemp.Distance;
	sceneDetail.runDetail.stepTotal 	= curRunInfoTemp.Step;
	sceneDetail.runDetail.calorieTotal  = curRunInfoTemp.Calorie;

	sceneDataLog.runScene.paceLog 	   = curRunInfoTemp.Recode.Pace;
	sceneDataLog.runScene.freqLog 	   = curRunInfoTemp.Recode.Freq;
	sceneDataLog.runScene.hrmLog 	   = curRunInfoTemp.Recode.HeartRate;
    return 0;
}

//**********************************************************************
// 函数功能：   登山场景开启
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
static uint16 Mid_ClimbScene_Open(void)
{
	multimodule_task_msg_t  msg;


	//打开心率模块
	msg.id                                  = HEARTRATE_ID;       
	msg.module.hrmEvent.id                  = HRM_START;
	MultiModuleTask_EventSet(msg);

	//新注册压力功能事件
    msg.id                                  = PRESSURE_ID;     
    msg.module.pressureEvent.id             = PRESSURE_FUNC_REGISTER;
	msg.module.pressureEvent.funcId 		= &scenePressorId;
	msg.module.pressureEvent.cvtfrq 		= PRESSURE_1HZ;//转换频率
	msg.module.pressureEvent.osr 			= RATE_128;//采样率
	msg.module.pressureEvent.paratype 		= ALL_PARA;//获取参数
    MultiModuleTask_EventSet(msg);

    return 0;
}

//**********************************************************************
// 函数功能：    登山场景关闭
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
static uint16 Mid_ClimbScene_Close(void)
{
	multimodule_task_msg_t  msg;
	
	//关闭心率
	msg.id                                  = HEARTRATE_ID;     
 	msg.module.hrmEvent.id                  = HRM_STOP;
  	MultiModuleTask_EventSet(msg); 

	//注销压力功能事件
	msg.id                                  = PRESSURE_ID;     
    msg.module.pressureEvent.id             = PRESSURE_FUNC_UN_REGISTER;
	msg.module.pressureEvent.funcId 		= &scenePressorId;
	MultiModuleTask_EventSet(msg); 
    return 0;
}

//**********************************************************************
// 函数功能：    登山场景处理
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
static uint16 Mid_ClimbScene_Process(void)
{

    return 0;
}


//**********************************************************************
// 函数功能：   游泳场景开启
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
static uint16 Mid_SwingScene_Open(void)
{
	multimodule_task_msg_t  msg;

	//打开心率模块
	msg.id                                  = HEARTRATE_ID;       
	msg.module.hrmEvent.id                  = HRM_START;
	MultiModuleTask_EventSet(msg);

	//新注册压力功能事件
    msg.id                                  = PRESSURE_ID;     
    msg.module.pressureEvent.id             = PRESSURE_FUNC_REGISTER;
	msg.module.pressureEvent.funcId 		= &scenePressorId;
	msg.module.pressureEvent.cvtfrq 		= PRESSURE_1HZ;//转换频率
	msg.module.pressureEvent.osr 			= RATE_128;//采样率
	msg.module.pressureEvent.paratype 		= ALL_PARA;//获取参数
    MultiModuleTask_EventSet(msg);

    return 0;
}

//**********************************************************************
// 函数功能：    游泳场景关闭
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
static uint16 Mid_SwingScene_Close(void)
{
	multimodule_task_msg_t  msg;
	//关闭心率
	msg.id                                  = HEARTRATE_ID;     
 	msg.module.hrmEvent.id                  = HRM_STOP;
  	MultiModuleTask_EventSet(msg); 

	//注销压力功能事件
	msg.id                                  = PRESSURE_ID;     
    msg.module.pressureEvent.id             = PRESSURE_FUNC_UN_REGISTER;
	msg.module.pressureEvent.funcId 		= &scenePressorId;
	MultiModuleTask_EventSet(msg); 
    return 0;
}

//**********************************************************************
// 函数功能：    游泳场景处理
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
static uint16 Mid_SwingScene_Process(void)
{

    return 0;
}

//**********************************************************************
// 函数功能：   场景信息获取
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
uint16 Mid_Scene_DetailRead(uint32 sceneType, scene_detail_s *sceneDetailTemp)
{
	switch(sceneType)
	{
		case RUN_SCENE_TYPE:
		sceneDetailTemp->runDetail.startUtc 		= sceneDetail.runDetail.startUtc;
		sceneDetailTemp->runDetail.duarationTotal 	= sceneDetail.runDetail.duarationTotal;
		sceneDetailTemp->runDetail.distanceTotal 	= sceneDetail.runDetail.distanceTotal;
		sceneDetailTemp->runDetail.stepTotal 		= sceneDetail.runDetail.stepTotal;
		sceneDetailTemp->runDetail.calorieTotal 	= sceneDetail.runDetail.calorieTotal;
		break;

		case CLIMB_SCENE_TYPE:
		sceneDetailTemp->climbDetail.startUtc 		= sceneDetail.climbDetail.startUtc;
		sceneDetailTemp->climbDetail.duarationTotal = sceneDetail.climbDetail.duarationTotal;
		sceneDetailTemp->climbDetail.upDistance 	= sceneDetail.climbDetail.upDistance;
		sceneDetailTemp->climbDetail.downDistance 	= sceneDetail.climbDetail.downDistance;
		sceneDetailTemp->climbDetail.altitudeHighest = sceneDetail.climbDetail.altitudeHighest;
		sceneDetailTemp->climbDetail.altitudeLowest = sceneDetail.climbDetail.altitudeLowest;
		sceneDetailTemp->climbDetail.calorieTotal 	= sceneDetail.climbDetail.calorieTotal;
		break;

		case SWING_SCENE_TYPE:
		sceneDetailTemp->swingDetail.startUtc 		= sceneDetail.swingDetail.startUtc;
		sceneDetailTemp->swingDetail.duarationTotal = sceneDetail.swingDetail.duarationTotal;
		sceneDetailTemp->swingDetail.pullTotal 		= sceneDetail.swingDetail.pullTotal;
		sceneDetailTemp->swingDetail.calorieTotal 	= sceneDetail.swingDetail.calorieTotal;
		break;

		default:
		break;
	}
    return 0;
}

//**********************************************************************
// 函数功能：   场景log数据获取
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
uint16 Mid_Scene_LogDataRead(uint32 sceneType, scene_data_s *sceneDataTemp)
{
	switch(sceneType)
	{
		case RUN_SCENE_TYPE:
		sceneDataTemp->runScene.paceLog 	= sceneDataLog.runScene.paceLog;
		sceneDataTemp->runScene.freqLog 	= sceneDataLog.runScene.paceLog;
		sceneDataTemp->runScene.hrmLog 		= sceneDataLog.runScene.hrmLog;
		break;

		case CLIMB_SCENE_TYPE:
		sceneDataTemp->climbScene.elevation = sceneDataLog.climbScene.elevation;
		sceneDataTemp->climbScene.climbSpeed = sceneDataLog.climbScene.climbSpeed;
		sceneDataTemp->climbScene.hrmLog 	= sceneDataLog.climbScene.hrmLog;
		break;

		case SWING_SCENE_TYPE:
		sceneDataTemp->swingScene.pullLog  = sceneDataLog.swingScene.pullLog;
		sceneDataTemp->swingScene.hrmLog   = sceneDataLog.swingScene.hrmLog;
		break;

		default:
		break;
	}
    return 0;
}

//**********************************************************************
// 函数功能:	事件处理
// 输入参数:	
// msg 	   :	事件信息
// 返回参数:	无
uint16 Mid_Scene_EventProcess(scene_event_s* msg)
{
	switch(msg->id)
	{
		case RUN_SCENE_OPEN:
		Mid_RunScene_Open();
		break;

		case RUN_SCENE_CLOSE:
		Mid_RunScene_Close();
		break;

		case RUN_SCENE_PROCESS:
		Mid_RunScene_Process();
		break;

		case CLIMB_SCENE_OPEN:
		Mid_ClimbScene_Open();
		break;

		case CLIMB_SCENE_CLOSE:
		Mid_ClimbScene_Close();
		break;

		case CLIMB_SCENE_PROCESS:
		Mid_ClimbScene_Process();
		break;

		case SWING_SCENE_OPEN:
		Mid_SwingScene_Open();
		break;

		case SWING_SCENE_CLOSE:
		Mid_SwingScene_Close();
		break;

		case SWING_SCENE_PROCESS:
		Mid_SwingScene_Process();
		break;

		default:
		break;
	}
	return 0;
}


