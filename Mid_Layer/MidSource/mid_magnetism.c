
/* FreeRTOS includes */
#include "rtos.h"

#include "drv_mag.h"
#include "mid_magnetism.h"

#include "drv_mtimer.h"



/*************** func declaration ********************************/
static void Mid_Mag_Sleep(void);
static uint16 Mid_Mag_HardwareSet(uint16 sampleRate, uint8 scaleRange);
static void Mid_Mag_ReadProcess(void);
static uint16 Mid_Mag_ReadSet(uint16 *id, uint16 readRate, uint8 scaleRange, void (*Cb)(int16 data[3]));
static uint16 Mid_Mag_ReadDelete(uint16 id);


/*************** macro define ********************************/
#define		MAG_NUM_MAX			8

/************** variable define *****************************/
static int16		magData[3];			// 三轴值
static uint16		magSampleRate;			// 硬件采样率
static uint16		magScaleRange;			// 硬件采样范围
static uint16		readRateMax;			// 读取的最大速率
static uint16		sensorTimerId;			// 传感器的定时器ID

typedef struct
{
	uint16	freq;						// 设置读取的频率
	uint16	cnt;						// 计数值
	uint16	aimCnt;						// 目标计数值
	void		(*ReadCb)(int16 data[3]);	// 回调函数，并传入三轴数据
}mag_para_s;


static	mag_para_s magPara[MAG_NUM_MAX];

static	void (*Mid_Mag_ReadIsrCb)(void);


/************** function define *****************************/
// 函数功能：	硬件定时器中断回调函数，
// 输入参数：	无
// 返回参数：	无
static void Mid_Mag_ReadIsr(void)
{
	Mid_Mag_ReadIsrCb();
}

//**********************************************************************
// 函数功能：	初始化硬件传感器，并设置可定时读取的回调函数
// 输入参数：	IsrCb: 传感器达到可设置的读取时间回调函数
// 返回参数：	无
//**********************************************************************
static void Mid_Mag_Sleep(void)
{
	uint16 i; 
	for(i = 0; i < MAG_NUM_MAX; i++)
	{
		magPara[i].freq		= 0;
		magPara[i].cnt		= 0;
		magPara[i].aimCnt	= 0;
	}

	Drv_MultiTimer_Delete(sensorTimerId);
	Drv_Mag_GotoSleep();


	magSampleRate			= 0;
	magScaleRange			= 0;
	readRateMax				= 0;
	sensorTimerId			= 0xffff;
}

//**********************************************************************
// 函数功能：   配置加速度传感器硬件采样频率与测量范围
// 输入参数：   IsrCb: 传感器达到可设置的读取时间回调函数
// 返回参数：	0x00:成功
// 				0xff:失败
//**********************************************************************
static uint16 Mid_Mag_HardwareSet(uint16 sampleRate, uint8 scaleRange)
{
	uint16 sampleRateTemp;
	uint8 scaleRangeTemp;

	switch(sampleRate)
	{
		case MAG_10HZ:
		sampleRateTemp	= MAG_DATA_RATE_10HZ;
		break;
		
		case MAG_50HZ:
		sampleRateTemp	= MAG_DATA_RATE_50HZ;
		break;

		case MAG_100HZ:
		sampleRateTemp	= MAG_DATA_RATE_100HZ;
		break;
		
		case MAG_200HZ:
		sampleRateTemp	= MAG_DATA_RATE_200HZ;
		break;

		default:
		return 0xff;
	}

	switch(scaleRange)
	{
		case MAG_2GS:
		scaleRangeTemp	= MAG_SCALE_RANGE_2GS;
		break;
		
		case MAG_8GS:
		scaleRangeTemp	= MAG_SCALE_RANGE_8GS;
		break;
		
		case MAG_12GS:
		scaleRangeTemp	= MAG_SCALE_RANGE_12GS;
		break;
		
		case MAG_20GS:
		scaleRangeTemp	= MAG_SCALE_RANGE_20GS;
		break;

		default:
		return 0xffff;
	}

	return Drv_Mag_Set(sampleRateTemp, scaleRangeTemp);
}

//**********************************************************************
// 函数功能：	进行读取硬件传感器值，并更新中间层缓存
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
static void Mid_Mag_ReadProcess(void)
{
	uint16 paraCnt;

	Drv_Mag_Read(magData);
	
	for(paraCnt = 0;paraCnt < MAG_NUM_MAX; paraCnt++)//每个有效的MultiTimer的Cnt都增加1
	{
		if(magPara[paraCnt].freq != 0)
		{
			magPara[paraCnt].cnt++;
			
			if(magPara[paraCnt].cnt >= magPara[paraCnt].aimCnt)
			{
				magPara[paraCnt].ReadCb(magData);
				magPara[paraCnt].cnt = 0;
			}
		}
	}
		 #ifdef TEST_DEBUG
    SEGGER_RTT_printf(0,"mag:%d,%d,%d\n", magData[0],magData[1],magData[2]);
    #endif
}

//**********************************************************************
// 函数功能：	设置读取的读取数据频率、采样范围、更新的回调函数
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
static uint16 Mid_Mag_ReadSet(uint16 *id, uint16 readRate, uint8 scaleRange, void (*Cb)(int16 data[3]))
{
	uint16 i;
	uint16	idTemp, maxFreqHzTemp;

	if(readRate == 0 ) //如果频率为0Hz，则设置失败。
	{
		return 0xFF;
	}

	for(i = 0; i < MAG_NUM_MAX; i++)
	{
		if(magPara[i].freq == 0)
		{
			idTemp	= i;
			*id		= idTemp;
			break;
		}
	}

	// 所有定时器均已分配完了
	if(i == MAG_NUM_MAX)
		return 0xff;


	magScaleRange				= scaleRange;

	magPara[idTemp].freq		= readRate;
	magPara[idTemp].ReadCb	= Cb;
	

	maxFreqHzTemp	= 0;
	for(i = 0; i < MAG_NUM_MAX; i++)
	{
		if(maxFreqHzTemp < magPara[i].freq)
			maxFreqHzTemp	= magPara[i].freq;
	}

	if( maxFreqHzTemp != readRateMax )
	{
		// 硬件定时器频率改变，计数需重新调整
		readRateMax 		= maxFreqHzTemp;
		magSampleRate		= maxFreqHzTemp;

		for(i = 0; i < MAG_NUM_MAX; i++)
		{
			if(magPara[i].freq != 0)
			{
				magPara[i].cnt		= 0;
				magPara[i].aimCnt	= readRateMax / magPara[i].freq;
			}
		}

		if(0xff == Mid_Mag_HardwareSet(magSampleRate, magScaleRange))
			return 0xff;

		Drv_MultiTimer_Delete(sensorTimerId);
		Drv_MultiTimer_Set(&sensorTimerId, readRateMax, Mid_Mag_ReadIsr);
	}
	return 0x00;
}

//**********************************************************************
// 函数功能：	设置读取的读取数据频率、采样范围、更新的回调函数
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
static uint16 Mid_Mag_ReadDelete(uint16 id)
{
	uint16 i;
	uint16	maxFreqHzTemp;

	if(id >= MAG_NUM_MAX)
		return 0xff;


	magPara[id].freq			= 0;
	magPara[id].cnt			= 0;
	magPara[id].aimCnt		= 0;


	maxFreqHzTemp					= 0;

	// 查找最高读取频率
	for(i = 0; i < MAG_NUM_MAX; i++)
	{
		if(maxFreqHzTemp < magPara[i].freq)
			maxFreqHzTemp	= magPara[i].freq;
	}

	// 硬件定时器频率改变，计数需重新调整
	if(maxFreqHzTemp != readRateMax)
	{
		readRateMax			= maxFreqHzTemp;
		magSampleRate		= maxFreqHzTemp;
		
		// 所有定时器均已关闭
		if(maxFreqHzTemp == 0)
		{
			if(sensorTimerId != 0xffff)
			{
				Drv_MultiTimer_Delete(sensorTimerId);
				sensorTimerId		= 0xffff;
			}
			Drv_Mag_GotoSleep();
			return 0x00;
		}


		for(i = 0; i < MAG_NUM_MAX; i++)
		{
			if(magPara[i].freq != 0)
			{
				magPara[i].cnt		= 0;
				magPara[i].aimCnt	= readRateMax / magPara[i].freq;
			}
		}

		if(0xff == Mid_Mag_HardwareSet(magSampleRate, magScaleRange))
			return 0xff;

		Drv_MultiTimer_Delete(sensorTimerId);
		Drv_MultiTimer_Set(&sensorTimerId, readRateMax, Mid_Mag_ReadIsr);
	}
	return 0x00;
}

//**********************************************************************
// 函数功能：	初始化硬件传感器，并设置可定时读取的回调函数
// 输入参数：	IsrCb: 传感器达到可设置的读取时间回调函数
// 返回参数：	无
//**********************************************************************
void Mid_Magnetism_Init(void (*IsrCb)(void))
{
	Drv_Mag_Open();
	magSampleRate	= MAG_DATA_RATE_10HZ;
	magScaleRange	= MAG_SCALE_RANGE_20GS;
	sensorTimerId	= 0xffff;
	Mid_Mag_ReadIsrCb	= IsrCb;
	Drv_Mag_GotoSleep();
}

//**********************************************************************
// 函数功能：	读取当前加速度的最新值
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
void Mid_Magnetism_DataRead(int16 data[3])
{
	data[0]		= magData[0];
	data[1]		= magData[1];
	data[2]		= magData[2];
}

//**********************************************************************
// 函数功能：	读取当前硬件的采样频率与采样范围
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
void Mid_Magnetism_SettingRead(uint16 *sampleRate, uint8 *scaleRange)
{
	*sampleRate		= magSampleRate;
	*scaleRange		= magScaleRange;
}

//**********************************************************************
// 函数功能：	加速度事件处理
// 输入参数：	无
// 返回参数：	0x00:成功
// 				0xff:失败
//**********************************************************************
uint16 Mid_Magnetism_EventProcess(mag_event_s* msg)
{
	switch(msg->id)
	{
		case MAG_SLEEP:
		Mid_Mag_Sleep();
		break;

		case MAG_HARDWARE_SET:
		Mid_Mag_HardwareSet(msg->rate, msg->scaleRange);
		break;

		case MAG_READ_PROCESS:
		Mid_Mag_ReadProcess();
		break;

		case MAG_READ_SET:
		Mid_Mag_ReadSet(msg->readId, msg->rate,
						msg->scaleRange, msg->Cb);
		break;

		case MAG_READ_DELETE:
		Mid_Mag_ReadDelete(*msg->readId);
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
//**********************************************************************
uint16 Mid_Magnetism_SelfTest(void)
{
	return Drv_Mag_SelfTest();
}

