#include "platform_common.h"
#include "mid_accelerate.h"
#include "mid_sport_scene.h"
#include "mid_usual_scene.h"
#include "multimodule_task.h"

static	uint8		firstData;
static	int8		lastAccel[3];

static	uint16	sleepMotion;
static	uint32	sedentaryMotion;
static	uint16	sedentaryTime;
static	uint16	sedentarySec;
static	uint16	staystillTime;
static	uint16	staystillSec;
static	uint8	sceneInit	= 0;
static uint32   stepRecord;
static  uint16  motionVal;
static	uint32	sceneEnable;

static uint16     usualsceneAccelReadId;

#define DATA_WINDOWS_LENA    4
#define DATA_YWA             2
#define DATA_WINDOWS_LENB    2
#define DATA_YWB             1
#define DATA_MAG            10
#define THRESHOLD           2




int32_t SmoothnessB(int8_t *p)//平滑相关的不用管
{
    int32_t DataTempx = 0;
    int32_t DataTempy = 0;
    int32_t DataTempz = 0;
    static int8_t DataWindowsX[DATA_WINDOWS_LENB] = { 0 };
    static int8_t DataWindowsY[DATA_WINDOWS_LENB] = { 0 };
    static int8_t DataWindowsZ[DATA_WINDOWS_LENB] = { 0 };
    static uint8_t DataWindowsHand = 0;       
    static uint8_t Count = 0;                 
    int32_t i = 0;

    DataWindowsX[DataWindowsHand] = p[0];
    DataWindowsY[DataWindowsHand] = p[1];
    DataWindowsZ[DataWindowsHand] = p[2];

    if (Count < (DATA_WINDOWS_LENB -1))
    {
        Count++;
        DataWindowsHand++;
        if (DataWindowsHand == DATA_WINDOWS_LENA) DataWindowsHand = 0;
        return 1;
    }

    for (i = 0; i < DATA_WINDOWS_LENB; i++)
    {
        DataTempx += DataWindowsX[i];
        DataTempy += DataWindowsY[i];
        DataTempz += DataWindowsZ[i];
    }
    DataTempx = DataTempx / 2;//>> DATA_YWB;
    DataTempy = DataTempy / 2;// >> DATA_YWB;
    DataTempz = DataTempz / 2;// >> DATA_YWB;

    DataWindowsX[DataWindowsHand] = DataTempx;
    p[0] = DataTempx;
    DataWindowsY[DataWindowsHand] = DataTempy;
    p[1] = DataTempy;
    DataWindowsZ[DataWindowsHand] = DataTempz;
    p[2] = DataTempz;
    DataWindowsHand++;
    if (DataWindowsHand == DATA_WINDOWS_LENB) DataWindowsHand = 0;
    return 0;
}

static int32_t Abs(int32_t x) //求绝对值
{ 
    return (x >= 0 ? x : -x); 
}

void DataSleepProcess(int8_t *p)//平滑相关不用管
{
    static uint8_t StartTpy = 0;
    static int8_t DataOld[3] = { 0 };
    uint32_t ChangeDataTemp = 0;


    if (SmoothnessB(p) != 0) return ;

    if (StartTpy == 0)
    {
        DataOld[0] = p[0];
        DataOld[1] = p[1];
        DataOld[2] = p[2];
        StartTpy = 1;
    }

    ChangeDataTemp += (Abs(p[0] - DataOld[0])) < THRESHOLD ? 0 : (Abs(p[0] - DataOld[0]));
    ChangeDataTemp += (Abs(p[1] - DataOld[1])) < THRESHOLD ? 0 : (Abs(p[1] - DataOld[1]));
    ChangeDataTemp += (Abs(p[2] - DataOld[2])) < THRESHOLD ? 0 : (Abs(p[2] - DataOld[2]));

    sleepMotion += ChangeDataTemp;
    if(sleepMotion > 0xffff)
    	sleepMotion	= 0xffff;
    DataOld[0] = p[0];
    DataOld[1] = p[1];
    DataOld[2] = p[2];
}
// 函数功能:	日常场景算法强制初始化
// 输入参数：	无
// 返回参数：	无
void Mid_UsualScene_Reset(void)
{
	sceneInit			= 1;
	firstData			= 1;
	lastAccel[0]		= 0;
	lastAccel[1]		= 0;
	lastAccel[2]		= 0;
	sleepMotion			= 0;
	sedentaryMotion		= 0;
	sedentaryTime		= 0;
	sedentarySec		= 0;
	staystillTime		= 0;
	staystillSec		= 0;
	sceneEnable			= 0;
	stepRecord 			= 0;
	motionVal 			= 0;
}



// 函数功能:	日常场景算法初始化，若已初始化后则直接返回
// 输入参数：	无
// 返回参数：	无
void Mid_UsualScene_Init(void)
{
	if(sceneInit == 1)
		return;
	Mid_UsualScene_Reset();
}



// 函数功能:	使能对应的日常场景功能
// 输入参数：	scene:		对应的日常场景
// 返回参数：	0x00:	操作成功
// 				0xff:	操作失败
uint16 Mid_UsualScene_Enable(uint16 scene)
{
	if(scene >= USUAL_SCENE_NUM)
		return 0xff;

	sceneEnable	|= 0x01 << scene;

	switch(scene)
	{
		case USUAL_SCENE_SLEEP:
		sleepMotion		= 0;
		break;

		case USUAL_SCENE_SEDENTARY:
		sedentaryTime		= 0;
		sedentarySec		= 0;
		break;

		case USUAL_SCENE_STANDSTILL:
		staystillTime		= 0;
		staystillSec		= 0;
		break;
	}
	return 0;
}


// 函数功能:	关闭对应的日常场景功能
// 输入参数：	scene:		对应的日常场景
// 返回参数：	0x00:	操作成功
// 				0xff:	操作失败
uint16 Mid_UsualScene_Disable(uint16 scene)
{
	if(scene >= USUAL_SCENE_NUM)
		return 0xff;

	sceneEnable	&= 0xffffffff ^ (0x01 << scene);

	return 0;
}

// 函数功能:	返回日常场景开关状态，若无该场景亦返回0
// 输入参数：	scene:		对应的日常场景
// 返回参数：	0x00:	关闭
// 				0x01:	开启
uint16 Mid_UsualScene_StateRead(uint16 scene)
{
	if(scene >= USUAL_SCENE_NUM)
		return 0x00;

	if(sceneEnable & (0x01 << scene))
		return 0x01;
	else
		return 0x00;
}



// 函数功能:	日常场景处理[睡眠、静置、久坐三轴数据处理]，每秒调用1次
// 输入参数:	无
// 返回参数:	无
static void Mid_UsualScene_Process(int16 *xyzData)
{
	uint32 		  stepTemp;
	int16       axisbuf[3] = {0};
	int16		  temp, temp2;
	uint8		  i;


	Mid_SportScene_StepRead(&stepTemp);

	//先作缩小处理　
	for(i = 0; i < 3; i++)
	{
		axisbuf[i] = xyzData[i] >> 8;
	}

	if(firstData)
	{
		firstData	= 0;
	}
	else
	{
		// 计算出每秒各轴的累计差值
		temp2	= 0;
		for(i = 0; i < 3; i++)
		{
			temp = (int16)lastAccel[i] - (int16)axisbuf[i];
			if(temp < 0)
				temp = 0 - temp;

			temp2 += temp;
		}

		//每秒更新动作值　
		motionVal = temp2;
		// 睡眠算法处理
		if(sceneEnable & (0x01 << USUAL_SCENE_SLEEP))
		{
			DataSleepProcess((int8 *)axisbuf);
		}

		// 久坐算法处理
		if(sceneEnable & (0x01 << USUAL_SCENE_SEDENTARY))
		{
			if((stepTemp - stepRecord))
			{
				sedentarySec		= 0;
				sedentaryTime		= 0;
				sedentaryMotion		= 0;
			}
			else
			{
				// 久坐过程动作累加
				if((sedentaryMotion + temp2) > 0xffffffff)
					sedentaryMotion	= 0xffffffff;
				else
					sedentaryMotion += temp2;

				// 久坐时间累加
				sedentarySec++;
				if(sedentarySec >= 60)
				{
					sedentarySec	= 0;
					sedentaryTime	++;
					// 久坐时间最长为24小时，即1440分钟
					if(sedentaryTime > 1440)
					{
						sedentaryTime	= 1440;
					}

				}
			}
		}

		// 静置算法处理
		if(sceneEnable & (0x01 << USUAL_SCENE_STANDSTILL))
		{
			if(temp2 < 10)
			{
				staystillSec++;
				if(staystillSec >= 60)
				{
					staystillSec = 0;
					staystillTime++;
					// 静置时间最长为30天，即18720分钟
					if(staystillTime >= 18720)
						staystillTime	= 18720;
				}
			}
			else
			{
				staystillSec	= 0;
				staystillTime	= 0;
			}
		}
	}

	lastAccel[0]	= axisbuf[0];
	lastAccel[1]	= axisbuf[1];
	lastAccel[2]	= axisbuf[2];
	stepRecord 		= stepTemp;
}




// 函数功能:	读取当前睡眠动作差值
// 输入参数:	无
// 返回参数:	睡眠动作累计
uint16 Mid_UsualScene_SleepMotionRead(void)
{
	return sleepMotion;
}


// 函数功能:	清除当前睡眠动作差值
// 输入参数:	无
// 返回参数:	无
void Mid_UsualScene_SleepMotionClear(void)
{
	sleepMotion		= 0;
}

// 函数功能:	读取当前久坐的动作差值累计
// 输入参数:	无
// 返回参数:	久坐动作差值累计
uint32 Mid_UsualScene_SedentaryMotionRead(void)
{
	return sedentaryMotion;
}

// 函数功能:	读取当前久坐的时间，单位为分钟
// 输入参数:	无
// 返回参数:	久坐时间
uint16 Mid_UsualScene_SedentaryTimeRead(void)
{
	return sedentaryTime;
}


// 函数功能:	清除当前久坐的时间与动作值（）
// 输入参数:	无
// 返回参数:	无
void Mid_UsualScene_SedentaryClear(void)
{
	sedentarySec		= 0;
	sedentaryTime		= 0;
	sedentaryMotion		= 0;
}

// 函数功能:	读取当前静置的时间，单位为分钟
// 输入参数:	无
// 返回参数:	静置时间
uint16 Mid_UsualScene_StandstillTimeRead(void)
{
	return staystillTime;
}

//**********************************************************************
// 函数功能:	清除当前久坐的时间与动作值（）
// 输入参数:	无
// 返回参数:	无
//**********************************************************************
void Mid_UsualScene_StandstillTimeClear(void)
{
	staystillSec	= 0;
	staystillTime	= 0;
}

//**********************************************************************
// 函数功能:	读取每秒动作累加值
// 输入参数:	无
// 返回参数:	无
//**********************************************************************
uint16 Mid_UsualScene_ReadMotion(void)
{
	return motionVal;
}


//**********************************************************************
// 函数功能：   开启
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
static uint16 Mid_UsualScene_Open(void)
{
	multimodule_task_msg_t  msg;
    msg.id                            = ACCEL_ID;
    msg.module.accelEvent.id          = ACCEL_READ_SET;
    msg.module.accelEvent.readId      = &usualsceneAccelReadId;
    msg.module.accelEvent.rate        = ACCEL_1HZ;
    msg.module.accelEvent.scaleRange  = ACCEL_2G;
    msg.module.accelEvent.Cb          = Mid_UsualScene_Process;
    MultiModuleTask_EventSet(msg);

    return 0;
}

//**********************************************************************
// 函数功能：   关闭
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
static uint16 Mid_UsualScene_Close(void)
{
	multimodule_task_msg_t  msg;

    msg.id                            = ACCEL_ID;
    msg.module.accelEvent.id          = ACCEL_READ_DELETE;
    msg.module.accelEvent.readId      = &usualsceneAccelReadId;
    MultiModuleTask_EventSet(msg);
    return 0;
}


//**********************************************************************
// 函数功能:    事件处理
// 输入参数：    无
// 返回参数：    0x00: success
//              0xff: fail
uint16 Mid_UsualScene_EventProcess(usual_event_s *msg)
{
     switch(msg->id)
     {
        case USUAL_SCENCE_OPEN:
         Mid_UsualScene_Open();
         break;

         case USUAL_SCENCE_CLOSE:
         Mid_UsualScene_Close();
         break;

         default:
         break;
    }
    return 0x00;
}
