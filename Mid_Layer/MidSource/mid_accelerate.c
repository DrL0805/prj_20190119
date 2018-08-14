/**********************************************************************
**
**模块说明: mid层加速计接口
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.25  修改流程  ZSL  
**
**********************************************************************/
/* FreeRTOS includes */
#include "rtos.h"

#include "drv_mtimer.h"
#include "drv_accelerate.h"
#include "mid_accelerate.h"

#define		ACCEL_NUM_MAX			8

static int16		accelData[3];			// 加速度三轴值
static uint16		accelSampleRate;		// 硬件采样率
static uint16		accelScaleRange;		// 硬件采样范围
static uint16		readRateMax;			// 读取的最大速率
static uint16		sensorTimerId;			// 传感器的定时器ID

typedef struct
{
	uint16	freq;						// 设置读取的频率
	uint8 	range; 						// 工作量程
	uint16	cnt;						// 计数值
	uint16	aimCnt;						// 目标计数值
	void		(*ReadCb)(int16 data[3]);	// 回调函数，并传入三轴数据
}accel_para_s;

static	accel_para_s accelPara[ACCEL_NUM_MAX];
static	void (*Mid_Accel_Read_IsrCb)(void);

/************** function define *****************************/
//**********************************************************************
// 函数功能：	硬件定时器中断回调函数，
// 输入参数：	无
// 返回参数：	无
static void Mid_Accel_ReadIsr(void)
{
    if(Mid_Accel_Read_IsrCb != NULL)
        (Mid_Accel_Read_IsrCb)();
}

//**********************************************************************
// 函数功能：	初始化硬件传感器，并设置可定时读取的回调函数
// 输入参数：	IsrCb: 传感器达到可设置的读取时间回调函数
// 返回参数：	无
//**********************************************************************
static void Mid_Accel_Sleep(void)
{
	uint16 i; 
	for(i = 0; i < ACCEL_NUM_MAX; i++)
	{
		accelPara[i].freq		= 0; 
		accelPara[i].cnt		= 0;
		accelPara[i].aimCnt		= 0;
		accelPara[i].range		= 0; 
	}

    Drv_MultiTimer_Delete(sensorTimerId);
	Drv_Accel_GoSleep();

	accelSampleRate			= 0;
	accelScaleRange			= 0;
	readRateMax				= 0;
	sensorTimerId			= 0xffff;
}

//**********************************************************************
// 函数功能：   配置加速度传感器硬件采样频率与测量范围
// 输入参数：   IsrCb: 传感器达到可设置的读取时间回调函数
// 返回参数：	0x00:成功
// 				0xff:失败
//**********************************************************************
static uint16 Mid_Accel_HardwareSet(uint16 sampleRate, uint8 scaleRange)
{
	uint16 sampleRateTemp;
	uint8 scaleRangeTemp;
	switch(sampleRate)
	{
		case ACCEL_1HZ:
		sampleRateTemp	= ACCEL_SAMPLERATE_1_56HZ;
		break;

		case ACCEL_2HZ:
		sampleRateTemp	= ACCEL_SAMPLERATE_3_12HZ;
		break;

		case ACCEL_25HZ:
		sampleRateTemp	= ACCEL_SAMPLERATE_25HZ;
		break;

		case ACCEL_50HZ:
		sampleRateTemp	= ACCEL_SAMPLERATE_50HZ;
		break;

		case ACCEL_100HZ:			
		sampleRateTemp	= ACCEL_SAMPLERATE_100HZ;
		break;
			
		case ACCEL_200HZ:
		sampleRateTemp	= ACCEL_SAMPLERATE_200HZ;
		break;

		default:
		if(sampleRate < 25)
			sampleRateTemp	= ACCEL_SAMPLERATE_25HZ;
		else if(sampleRate < 63)
			sampleRateTemp	= ACCEL_SAMPLERATE_100HZ;
		else if(sampleRate <= 250)
			sampleRateTemp	= ACCEL_SAMPLERATE_400HZ;
		else
			return 0xff;
	}

	switch(scaleRange)
	{
		case ACCEL_2G:
		scaleRangeTemp	= ACCEL_SCALERANGE_2G;
		break;

		case ACCEL_4G:
		scaleRangeTemp	= ACCEL_SCALERANGE_4G;
		break;
		
		case ACCEL_8G:
		scaleRangeTemp	= ACCEL_SCALERANGE_8G;
		break;
		
		case ACCEL_16G:
		scaleRangeTemp	= ACCEL_SCALERANGE_16G;
		break;

		default:
		return 0xffff;
	}

	return Drv_Accel_Set(sampleRateTemp, scaleRangeTemp);
}

//**********************************************************************
// 函数功能：	进行读取硬件传感器值，并更新中间层缓存
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
static void Mid_Accel_ReadProcess(void)
{
	uint16 paraCnt;

	Drv_Accel_Read(accelData);

	for(paraCnt = 0;paraCnt < ACCEL_NUM_MAX; paraCnt++)//每个有效的MultiTimer的Cnt都增加1
	{
		if(accelPara[paraCnt].freq != 0)
		{
			accelPara[paraCnt].cnt++;
			
			if(accelPara[paraCnt].cnt >= accelPara[paraCnt].aimCnt)
			{
				accelPara[paraCnt].ReadCb(accelData);
				accelPara[paraCnt].cnt = 0;
			}
		}
	}
}

//**********************************************************************
// 函数功能：	设置读取的读取数据频率、采样范围、更新的回调函数
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
static uint16 Mid_Accel_ReadSet(uint16 *id, uint16 readRate, uint8 scaleRange, void (*Cb)(int16 data[3]))
{
	uint16 i;
	uint16	idTemp, maxFreqHzTemp,maxRangeTemp;

	if(readRate == 0 ) //如果频率为0Hz，则设置失败。
	{
		return 0xFF;
	}

	
	for(i = 0; i < ACCEL_NUM_MAX; i++)
	{
		if(accelPara[i].freq == 0)
		{
			idTemp	= i;
			*id		= idTemp;
			break;
		}
	}

	// 所有定时器均已分配完了
	if(i == ACCEL_NUM_MAX)
	{	
		return 0xff;
	}

	accelPara[idTemp].range		= scaleRange;
	accelPara[idTemp].freq		= readRate;
	accelPara[idTemp].ReadCb	= Cb;
	

	maxFreqHzTemp	= 0;
	maxRangeTemp 	= 0;
	for(i = 0; i < ACCEL_NUM_MAX; i++)
	{
		if(maxFreqHzTemp < accelPara[i].freq)
			maxFreqHzTemp	= accelPara[i].freq;
	}

	for(i = 0; i < ACCEL_NUM_MAX; i++)
	{
		if(maxRangeTemp < accelPara[i].range)
			maxRangeTemp	= accelPara[i].range;
	}

	accelScaleRange			= maxRangeTemp;//更新最大量程


	if( maxFreqHzTemp != readRateMax )
	{
		// 硬件定时器频率改变，计数需重新调整
		readRateMax 		= maxFreqHzTemp;
		accelSampleRate		= maxFreqHzTemp;//更新采样率

		for(i = 0; i < ACCEL_NUM_MAX; i++)
		{
			if(accelPara[i].freq != 0)
			{
				accelPara[i].cnt		= 0;
				accelPara[i].aimCnt	= readRateMax / accelPara[i].freq;
			}
		}

		if(0xff == Mid_Accel_HardwareSet(accelSampleRate, accelScaleRange))
		{
			return 0xff;
		}

		Drv_MultiTimer_Delete(sensorTimerId);
		Drv_MultiTimer_Set(&sensorTimerId, readRateMax, Mid_Accel_ReadIsr);
	}
	return 0x00;
}

//**********************************************************************
// 函数功能：	设置读取的读取数据频率、采样范围、更新的回调函数
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
static uint16 Mid_Accel_ReadDelete(uint16 id)
{
	uint16 i;
	uint16	maxFreqHzTemp,maxRangeTemp;

	if(id >= ACCEL_NUM_MAX)
		return 0xff;

	accelPara[id].freq			= 0;
	accelPara[id].cnt			= 0;
	accelPara[id].aimCnt		= 0;


	maxFreqHzTemp				= 0;
	maxRangeTemp 				= 0;

	// 查找最高读取频率
	for(i = 0; i < ACCEL_NUM_MAX; i++)
	{
		if(maxFreqHzTemp < accelPara[i].freq)
			maxFreqHzTemp	= accelPara[i].freq;
	}

	for(i = 0; i < ACCEL_NUM_MAX; i++)
	{
		if(maxRangeTemp < accelPara[i].range)
			maxRangeTemp	= accelPara[i].range;
	}
	accelScaleRange			= maxRangeTemp;//更新最大量程

	// 硬件定时器频率改变，计数需重新调整
	if(maxFreqHzTemp != readRateMax)
	{
		readRateMax			= maxFreqHzTemp;
		accelSampleRate		= maxFreqHzTemp;
		
		// 所有定时器均已关闭
		if(maxFreqHzTemp == 0)
		{
			if(sensorTimerId != 0xffff)
			{
				Drv_MultiTimer_Delete(sensorTimerId);
				sensorTimerId		= 0xffff;
			}
			Drv_Accel_GoSleep();
			return 0x00;
		}


		for(i = 0; i < ACCEL_NUM_MAX; i++)
		{
			if(accelPara[i].freq != 0)
			{
				accelPara[i].cnt		= 0;
				accelPara[i].aimCnt	= readRateMax / accelPara[i].freq;
			}
		}

		if(0xff == Mid_Accel_HardwareSet(accelSampleRate, accelScaleRange))
		{
			return 0xff;
		}

		Drv_MultiTimer_Delete(sensorTimerId);
		Drv_MultiTimer_Set(&sensorTimerId, readRateMax, Mid_Accel_ReadIsr);
	}
	return 0x00;
}

//**********************************************************************
// 函数功能：	初始化硬件传感器，并设置可定时读取的回调函数
// 输入参数：	IsrCb: 传感器达到可设置的读取时间回调函数
// 返回参数：	无
//**********************************************************************
void Mid_Accel_Init(comm_cb *Read_IsrCb)
{
	Drv_Accel_Open();
	accelSampleRate	= ACCEL_SAMPLERATE_NONE;
	accelScaleRange	= ACCEL_SCALERANGE_2G;
	sensorTimerId	= 0xffff;
	Mid_Accel_Read_IsrCb = Read_IsrCb;

	Drv_Accel_GoSleep();
}

//**********************************************************************
// 函数功能：	读取当前硬件的采样频率与采样范围
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
void Mid_Accel_SettingRead(uint16 *sampleRate, uint8 *scaleRange)
{
	*sampleRate		= accelSampleRate;
	*scaleRange		= accelScaleRange;	
}

//**********************************************************************
// 函数功能：	读取当前加速度的最新值
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
void Mid_Accel_DataRead(int16 data[3])
{
	data[0]		= accelData[0];
	data[1]		= accelData[1];
	data[2]		= accelData[2];
}

//**********************************************************************
// 函数功能：	加速度事件处理
// 输入参数：	msg	事件指针
// 返回参数：	0x00: 操作成功
//				0xff: 操作失败
//**********************************************************************
uint16 Mid_Accel_EventProcess(accel_event_s* msg)
{
    switch(msg->id)
    {
    case ACCEL_SLEEP:
		Mid_Accel_Sleep();
		break;
    case ACCEL_HARDWARE_SET:
		Mid_Accel_HardwareSet(msg->rate, msg->scaleRange);
		break;
    case ACCEL_READ_PROCESS:
		Mid_Accel_ReadProcess();
		break;
    case ACCEL_READ_SET:
		Mid_Accel_ReadSet(msg->readId, msg->rate,
						msg->scaleRange, msg->Cb);
		break;
    case ACCEL_READ_DELETE:
		Mid_Accel_ReadDelete(*msg->readId);
		break;
    default:
		return 0xff;
	}
	return 0x00;
}

//**********************************************************************
// 函数功能：	传感器自检,调用该函数时，确保该资源未使用
// 输入参数：	无
// 返回参数：	0x00:成功
// 				0xff:失败
uint16 Mid_Accel_SelfTest(void)
{
	return Drv_Accel_SelfTest();
}

