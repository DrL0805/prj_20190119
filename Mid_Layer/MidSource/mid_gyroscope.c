/**********************************************************************
**
**模块说明: mid层陀螺仪接口
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.25  修改流程  ZSL  
**
**********************************************************************/
#include "drv_mtimer.h"
#include "drv_gyroscope.h"
#include "mid_gyroscope.h"

#define		GYRO_NUM_MAX			8

static int16		gyroData[3];			// 三轴值
static uint16		gyroSampleRate;			// 硬件采样率
static uint16		gyroScaleRange;			// 硬件采样范围
static uint16		readRateMax;			// 读取的最大速率
static uint16		sensorTimerId;			// 传感器的定时器ID

typedef struct
{
	uint16	freq;						// 设置读取的频率
	uint16	cnt;						// 计数值
	uint16	aimCnt;						// 目标计数值
	void		(*ReadCb)(int16 data[3]);	// 回调函数，并传入三轴数据
}gyro_para_s;

static	gyro_para_s gyroPara[GYRO_NUM_MAX];
static	void (*GyroReadIsrCb)(void);


/************** function define *****************************/
//**********************************************************************
// 函数功能：	硬件定时器中断回调函数，
// 输入参数：	无
// 返回参数：	无
static void GyroReadIsr(void)
{
	GyroReadIsrCb();
}

//**********************************************************************
// 函数功能：	初始化硬件传感器，并设置可定时读取的回调函数
// 输入参数：	IsrCb: 传感器达到可设置的读取时间回调函数
// 返回参数：	无
static void GyroSleep(void)
{
	uint16 i; 
	for(i = 0; i < GYRO_NUM_MAX; i++)
	{
		gyroPara[i].freq		= 0;
		gyroPara[i].cnt		= 0;
		gyroPara[i].aimCnt	= 0;
	}

	Drv_MultiTimer_Delete(sensorTimerId);
	Drv_Gyro_GoSleep();


	gyroSampleRate			= 0;
	gyroScaleRange			= 0;
	readRateMax				= 0;
	sensorTimerId			= 0xffff;
}

//**********************************************************************
// 函数功能：   配置加速度传感器硬件采样频率与测量范围
// 输入参数：   IsrCb: 传感器达到可设置的读取时间回调函数
// 返回参数：	0x00:成功
// 				0xff:失败
static uint16 GyroHardwareSet(uint16 sampleRate, uint8 scaleRange)
{
	uint16 sampleRateTemp;
	uint8 scaleRangeTemp;
	switch(sampleRate)
	{
		case GYRO_1HZ:
		sampleRateTemp	= GYO_SAMPLERATE_25HZ;
		break;

		case GYRO_2HZ:
		sampleRateTemp	= GYO_SAMPLERATE_25HZ;
		break;

		case GYRO_25HZ:
		sampleRateTemp	= GYO_SAMPLERATE_25HZ;
		break;

		case GYRO_50HZ:
		sampleRateTemp	= GYO_SAMPLERATE_50HZ;
		break;

		case GYRO_100HZ:
		sampleRateTemp	= GYO_SAMPLERATE_200HZ;
		break;

		case GYRO_200HZ:
		sampleRateTemp	= GYO_SAMPLERATE_200HZ;
		break;

		default:
		if(sampleRate < 40)
			sampleRateTemp	= GYO_SAMPLERATE_50HZ;
		else if(sampleRate < 90)
			sampleRateTemp	= GYO_SAMPLERATE_100HZ;
		else if(sampleRate <= 250)
			sampleRateTemp	= GYO_SAMPLERATE_400HZ;
		else
			return 0xff;
	}

	switch(scaleRange)
	{
		case GYRO_250S:
		scaleRangeTemp	= GYRO_SCALE_RANGE_250_DEG_SEC;
		break;

		case GYRO_500S:
		scaleRangeTemp	= GYRO_SCALE_RANGE_500_DEG_SEC;
		break;
		
		case GYRO_1000S:
		scaleRangeTemp	= GYRO_SCALE_RANGE_1000_DEG_SEC;
		break;
		
		case GYRO_2000S:
		scaleRangeTemp	= GYRO_SCALE_RANGE_2000_DEG_SEC;
		break;

		default:
		return 0xffff;
	}

	return Drv_Gyro_Set(sampleRateTemp, scaleRangeTemp);
}

//**********************************************************************

// 函数功能：	进行读取硬件传感器值，并更新中间层缓存
// 输入参数：	无
// 返回参数：	无
static void GyroReadProcess(void)
{
	uint16 paraCnt;

	Drv_Gyro_Read(gyroData);

	for(paraCnt = 0;paraCnt < GYRO_NUM_MAX; paraCnt++)//每个有效的MultiTimer的Cnt都增加1
	{
		if(gyroPara[paraCnt].freq != 0)
		{
			gyroPara[paraCnt].cnt++;
			
			if(gyroPara[paraCnt].cnt >= gyroPara[paraCnt].aimCnt)
			{
				gyroPara[paraCnt].ReadCb(gyroData);
				gyroPara[paraCnt].cnt = 0;
			}
		}
	}
}

//**********************************************************************
// 函数功能：	设置读取的读取数据频率、采样范围、更新的回调函数
// 输入参数：	无
// 返回参数：	无
static uint16 GyroReadSet(uint16 *id, uint16 readRate, uint8 scaleRange, void (*Cb)(int16 data[3]))
{
	uint16 i;
	uint16	idTemp, maxFreqHzTemp;

	if(readRate == 0 ) //如果频率为0Hz，则设置失败。
	{
		return 0xFF;
	}

	for(i = 0; i < GYRO_NUM_MAX; i++)
	{
		if(gyroPara[i].freq == 0)
		{
			idTemp	= i;
			*id		= idTemp;
			break;
		}
	}

	// 所有定时器均已分配完了
	if(i == GYRO_NUM_MAX)
		return 0xff;


	gyroScaleRange				= scaleRange;

	gyroPara[idTemp].freq		= readRate;
	gyroPara[idTemp].ReadCb	= Cb;
	

	maxFreqHzTemp	= 0;
	for(i = 0; i < GYRO_NUM_MAX; i++)
	{
		if(maxFreqHzTemp < gyroPara[i].freq)
			maxFreqHzTemp	= gyroPara[i].freq;
	}

	if( maxFreqHzTemp != readRateMax )
	{
		// 硬件定时器频率改变，计数需重新调整
		readRateMax 		= maxFreqHzTemp;
		gyroSampleRate		= maxFreqHzTemp;

		for(i = 0; i < GYRO_NUM_MAX; i++)
		{
			if(gyroPara[i].freq != 0)
			{
				gyroPara[i].cnt		= 0;
				gyroPara[i].aimCnt	= readRateMax / gyroPara[i].freq;
			}
		}

		if(0xff == GyroHardwareSet(gyroSampleRate, gyroScaleRange))
			return 0xff;

		Drv_MultiTimer_Delete(sensorTimerId);
		Drv_MultiTimer_Set(&sensorTimerId, readRateMax, GyroReadIsr);
	}
	return 0x00;
}

//**********************************************************************
// 函数功能：	设置读取的读取数据频率、采样范围、更新的回调函数
// 输入参数：	无
// 返回参数：	无
static uint16 GyroReadDelete(uint16 id)
{
	uint16 i;
	uint16	maxFreqHzTemp;

	if(id >= GYRO_NUM_MAX)
		return 0xff;


	gyroPara[id].freq			= 0;
	gyroPara[id].cnt			= 0;
	gyroPara[id].aimCnt		= 0;


	maxFreqHzTemp					= 0;

	// 查找最高读取频率
	for(i = 0; i < GYRO_NUM_MAX; i++)
	{
		if(maxFreqHzTemp < gyroPara[i].freq)
			maxFreqHzTemp	= gyroPara[i].freq;
	}

	// 硬件定时器频率改变，计数需重新调整
	if(maxFreqHzTemp != readRateMax)
	{
		readRateMax			= maxFreqHzTemp;
		gyroSampleRate		= maxFreqHzTemp;
		
		// 所有定时器均已关闭
		if(maxFreqHzTemp == 0)
		{
			if(sensorTimerId != 0xffff)
			{
				Drv_MultiTimer_Delete(sensorTimerId);
				sensorTimerId		= 0xffff;
			}
			Drv_Gyro_GoSleep();
			return 0x00;
		}


		for(i = 0; i < GYRO_NUM_MAX; i++)
		{
			if(gyroPara[i].freq != 0)
			{
				gyroPara[i].cnt		= 0;
				gyroPara[i].aimCnt	= readRateMax / gyroPara[i].freq;
			}
		}

		if(0xff == GyroHardwareSet(gyroSampleRate, gyroScaleRange))
			return 0xff;

		Drv_MultiTimer_Delete(sensorTimerId);
		Drv_MultiTimer_Set(&sensorTimerId, readRateMax, GyroReadIsr);
	}
	return 0x00;
}

//**********************************************************************
// 函数功能：	初始化硬件传感器，并设置可定时读取的回调函数
// 输入参数：	IsrCb: 传感器达到可设置的读取时间回调函数
// 返回参数：	无
//**********************************************************************
void Mid_Gyro_Init(void (*IsrCb)(void))
{
	Drv_Gyro_Open();
	gyroSampleRate	= GYO_SAMPLERATE_NONE;
	gyroScaleRange	= GYRO_SCALE_RANGE_125_DEG_SEC;
	sensorTimerId	= 0xffff;
	GyroReadIsrCb	= IsrCb;
	Drv_Gyro_GoSleep();
}

//**********************************************************************
// 函数功能：	读取当前加速度的最新值
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
void Mid_Gyro_DataRead(int16 data[3])
{
	data[0]		= gyroData[0];
	data[1]		= gyroData[1];
	data[2]		= gyroData[2];
}

//**********************************************************************
// 函数功能：	读取当前硬件的采样频率与采样范围
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
void Mid_Gyro_SettingRead(uint16 *sampleRate, uint8 *scaleRange)
{
	*sampleRate		= gyroSampleRate;
	*scaleRange		= gyroScaleRange;
}

//**********************************************************************
// 函数功能：	加速度事件处理
// 输入参数：	无
// 返回参数：	0x00:成功
// 				0xff:失败
//**********************************************************************
uint16 Mid_Gyro_EventProcess(gyro_event_s* msg)
{
	switch(msg->id)
	{
		case GYRO_SLEEP:
		GyroSleep();
		break;

		case GYRO_HARDWARE_SET:
		GyroHardwareSet(msg->rate, msg->scaleRange);
		break;

		case GYRO_READ_PROCESS:
		GyroReadProcess();
		break;

		case GYRO_READ_SET:
		GyroReadSet(msg->readId, msg->rate,
						msg->scaleRange, msg->Cb);
		break;

		case GYRO_READ_DELETE:
		GyroReadDelete(*msg->readId);
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
uint16 Mid_Gyro_SelfTest(void)
{
	return Drv_Gyro_SelfTest();
}

