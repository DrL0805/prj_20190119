/**********************************************************************
**
**模块说明: mid层气压/水压接口
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.25  修改流程  ZSL  
**
**********************************************************************/
#include "mid_interface.h"

#ifdef AIRPRESSURE
#include "drv_airpress.h"     //气压
#else
#include "drv_waterpress.h"   //水压
#endif

#if 0
/*************** func declaration ******************/
static uint16 PressSensorSetOSR(uint8 newrate);
static uint16 PressSensorCvnRateSet(uint8 frq);
static uint16 PressSensorParaSet(uint8 presssensorevnt);
static uint16 PressSensoreStart(void);
static uint16 PressSensorStop(void);

static uint16 PressSensorDataProcess(void);
static int32 PressAvr(void);
static int32 AltitudeAvr(void);
static void PressInsert(int32 p);

#ifdef AIRPRESSURE
static void AltitudeInsert(int32 altitude);
static void PressureToAltitude(int32 pressure,int16 tempreture);
#else
static void PressureToDepth(int32 pressure);
#endif

static uint16 PressSensorCalLevelPressure(int32 cur_height,int32 cur_pressure,int16 cur_temperature);
#endif

/*******************macro define*******************/
//定义传感器海拔功能
// #define 	AltitudeFunc

#define PRESSURE_NUM_MAX (5)

#define SUM_NUM			(8)

#define 	ln10 		2.30258
#define 	G_com 		9.800//重力加速度，常量,N/kg

typedef enum
{
	PressSensorStateIdle 		= 0x00,
	PressSensorStateTemp,
	PressSensorStatePress,
}pressure_sensor_state; 


/*******************variable define*******************/
//压力（气压/水压）传感器处理系统定时器
static TimerHandle_t PressureProcessTimer = NULL;

static uint64 LevelPressure 	= 101325;//pa
static uint32 Waterdensity	= 1000;///水密度，1000kg/m3

//信息存储变量
int32 mAltitude 				= 0;
int32 mDepth 					= 0;
int16 mTemperature	    		= 0;	
int32 mPressure 				= 0;

//取平均相关变量 
uint8 PressureIndex			=	0;
uint8 AltitudeIndex 			= 	0;
uint8 PressureReady			=	0;
uint8 AltitudeReady			=	0;

int32 PressureGroup[SUM_NUM]	=	{101325};
int32 AltitudeGroup[SUM_NUM]	=	{0};

//转换配置相关变量
static uint8  PressureOSR 		= RATE_NULL;
static uint8  TemperaturOSR 		= RATE_NULL;
static uint8  sensorRate 			= RATE_NULL;

//返回信息类型开关：可选温度、压力、海拔、深度
static uint8  PressSensorPara 	= NULL_PARA;
//传感器状态记录
static uint8 	PressSensorState 	= PressSensorStateIdle;


typedef struct
{
	uint16	samplerate;			// 设置工作的下采样
	uint8 	paraswitch; 		// 设置的参数开关
	uint8 	sensorfrq;			// 设置的传感器转换频率
}pressrue_para_s;

static	pressrue_para_s pressurePara[PRESSURE_NUM_MAX];

/*******************function define*******************/

//**********************************************************************
// 函数功能:	计算压力值的平均
// 输入参数：	无
// 返回参数：	压力的平均值
static int32 PressAvr(void)
{
	int64 sum=0;
	uint8 i;
	
	if(!PressureReady)	
	{
		if(PressureIndex == 0)
		{
			return LevelPressure;//return latest pressure if less than eight value
		}
		else
		{
			return PressureGroup[(PressureIndex-1)&0x7];//return latest pressure if less than eight value
		}
		
	}

	for(i=0;i<SUM_NUM;i++)
	{
		sum += PressureGroup[i];
	}
	return (int32)sum/SUM_NUM;
}

//**********************************************************************
// 函数功能:	计算海拔值的平均
// 输入参数：	无
// 返回参数：	海拔值
static int32 AltitudeAvr(void)
{
	int64 sum=0;
	uint8 i;
	
	if(!AltitudeReady)	return AltitudeGroup[(AltitudeIndex-1)&0x7];//return latest altitude if less than eight value

	for(i=0;i<SUM_NUM;i++)
	{
		sum += AltitudeGroup[i];
	}
	return (int32)sum/SUM_NUM;
}

//**********************************************************************
// 函数功能:	压力值入缓存数组
// 输入参数：	无
// 返回参数：	无
static void PressInsert(int32 p)
{
	if(p <= 0)
		return;
	PressureGroup[(PressureIndex++&0x7)] = p;
	if(PressureIndex == SUM_NUM) 
		PressureReady	=	1;
}

//**********************************************************************
// 函数功能:	海拔值入缓存数组
// 输入参数：	无
// 返回参数：	无
static void AltitudeInsert(int32 altitude)
{
	AltitudeGroup[(AltitudeIndex++&0x7)] = altitude;
	if(AltitudeIndex == SUM_NUM) 
		AltitudeReady	=	1;
}

//**********************************************************************
// 函数功能:	根据当前海拔(m)、气压(pa)、温度(C)计算海平面标准大气压
//	h=18400*（1+t/273）*lg( P0/P2) ==> P0 = P * exp{H / [(1+t/273)* 18400]*ln10}
// 输入参数：	
//cur_height: 		当前海拔点高度
//cur_pressure： 	当前海拔点气压
//cur_temperature： 当前海拔点温度
// 返回参数：	
// 0x00    :    设置成功
// 0xff    :    设置失败
static uint16 PressSensorCalLevelPressure(int32 cur_height,int32 cur_pressure,int16 cur_temperature)
{
	float  cal_height;
	float  Cal_P0;
	float  P0_err;
	float  exp_x;

	exp_x = ( cal_height / ((1 + cur_temperature / 273.0) * 18400 )) * ln10;	
	
	Cal_P0 = cur_pressure * Algorithm_exp(exp_x);

	P0_err = (0.0009869f * Cal_P0 - 99.9985396f) * (101325.0f / 100.0f);
	
	Cal_P0 = Cal_P0 - P0_err;
	
	LevelPressure = Cal_P0;//update the level pressure,Unit Pa

	#ifdef AIRPRESSURE
	Drv_AirPress_SetAltitudeCmps(LevelPressure);//气压传感器进行海拔校正
	#endif
	return 0;
}

//**********************************************************************
// 函数功能:	根据压高议程计算海拔高度
// H = 0.001237*cal_height*cal_height/100, cal_height = 18400*（1+t/273）*lg( P0/P2)
// 输入参数：	
// pressure： 	气压值
// tempreture 	温度值 
// 返回参数：	无
static void PressureToAltitude(int32 pressure,int16 tempreture)
{
	float cal_height=0;
	float Err = 0;
	float temp;
	
	temp 	= (float)tempreture / 100.0f;
	
	cal_height = ((float)18400 *(1+temp/273)*(Algorithm_lg((float)LevelPressure / pressure)));

	if(cal_height < 0)
		Err = (-0.0013 * cal_height - 0.0926) *cal_height / 100; //height compensationty
	else if(cal_height <= 1500)
		Err = (-0.001264 * cal_height+0.0589) *cal_height / 100; 
	else if(cal_height <= 2500)
		Err = (-0.0013 * cal_height + 0.1059) *cal_height / 100; 
	else if(cal_height <= 3600)
		Err = (-0.0014 * cal_height + 0.3556) *cal_height / 100; 
	else if(cal_height <= 4700)
		Err = (-0.0015 * cal_height + 0.7054) *cal_height / 100; 
	else if(cal_height <= 5500)
		Err = (-0.0016 * cal_height + 1.1511) *cal_height / 100; 
	else if(cal_height <= 8750)
		Err = (-0.0019 * cal_height + 2.8285) *cal_height / 100;
	else
		Err = (-0.0019 * cal_height + 2.4681) *cal_height / 100;
	
	cal_height -= Err;

	AltitudeInsert((int32)(cal_height*100));
	mAltitude 	= AltitudeAvr();
}

//**********************************************************************
// 函数功能:	根据水压强公式计算深度
// 输入参数：	
// pressure：  	压力值 
// 返回参数：	无
static void PressureToDepth(int32 pressure)
{
	mDepth 	= (float)((float)(pressure-LevelPressure)/((float)Waterdensity * (float)G_com) );
}

//**********************************************************************
// 函数功能:	压力传感器转换使能
// 输入参数：	无
// 返回参数：	
// 0x00    :    初始化成功
// 0xff    :    初始化失败
static uint16 PressSensoreStart(void)
{
	PressSensorState 	= PressSensorStateIdle;
	PressureIndex		= 0;
	AltitudeIndex 		= 0;
	PressureReady		= 0;
	AltitudeReady		= 0;


	#ifdef AIRPRESSURE
	if (Drv_AirPress_EnableIO())
	{
		return 0xff;
	}
	#else
	if (bspWaterPress.Enable();)
	{
		return 0xff;
	}
	#endif

	xTimerReset(PressureProcessTimer, 0);        
	xTimerStart(PressureProcessTimer, 0);

	return 0;
}

//**********************************************************************
// 函数功能:	压力传感器转换关闭
// 输入参数：	无
// 返回参数：	
// 0x00    :    设置成功
// 0xff    :    设置失败
static uint16 PressSensorStop(void)
{
	PressSensorState 	= PressSensorStateIdle;

	xTimerStop(PressureProcessTimer, 0);
	#ifdef AIRPRESSURE
	if (Drv_AirPress_DisableIO())
	{
		return 0xff;
	}

	#else
	if (bspWaterPress.Disable();)
	{
		return 0xff;
	}
	#endif

	return 0;
}

//**********************************************************************
// 函数功能:	设置压力传感器（下）采样率，
// 输入参数：	无
// 返回参数：	
// 0x00    :    设置成功
// 0xff    :    设置失败
static uint16 PressSensorSetOSR(uint8 newrate)
{
	#ifdef AIRPRESSURE
	if (newrate == RATE_NULL || newrate >= RATE_128)
	{
		return 0xff;
	}
	switch(newrate)
	{
		case RATE_1:
		PressureOSR 	= OSR_1;
		TemperaturOSR 	= OSR_1;
		break;

		case RATE_2:
		PressureOSR 	= OSR_2;
		TemperaturOSR 	= OSR_2;
		break;

		case RATE_4:
		PressureOSR 	= OSR_4;
		TemperaturOSR 	= OSR_4;
		break;

		case RATE_8:
		PressureOSR 	= OSR_8;
		TemperaturOSR 	= OSR_8;
		break;

		case RATE_16:
		PressureOSR 	= OSR_16;
		TemperaturOSR 	= OSR_16;
		break;

		case RATE_32:
		PressureOSR 	= OSR_32;
		TemperaturOSR 	= OSR_32;
		break;
		
		case RATE_64:
		PressureOSR 	= OSR_64;
		TemperaturOSR 	= OSR_64;
		break;
		
		case RATE_128:
		PressureOSR 	= OSR_128;
		TemperaturOSR 	= OSR_128;
		break;

		default:
		break;
	}
	#else
	if (newrate == RATE_NULL || newrate > RATE_8192)
	{
		return 0xff;
	}
	switch(newrate)
	{
		case RATE_256:
		PressureOSR 	= PRESS_D1_OSR_256;
		TemperaturOSR 	= TEMP_D2_OSR_256;
		break;

		case RATE_512:
		PressureOSR 	= PRESS_D1_OSR_512;
		TemperaturOSR 	= TEMP_D2_OSR_512;
		break;

		case RATE_1024:
		PressureOSR 	= PRESS_D1_OSR_1024;
		TemperaturOSR 	= TEMP_D2_OSR_1024;
		break;

		case RATE_2048:
		PressureOSR 	= PRESS_D1_OSR_2048;
		TemperaturOSR 	= TEMP_D2_OSR_2048;
		break;

		case RATE_4096:
		PressureOSR 	= PRESS_D1_OSR_4096;
		TemperaturOSR 	= TEMP_D2_OSR_4096;
		break;

		case RATE_8192:
		PressureOSR 	= PRESS_D1_OSR_8192;
		TemperaturOSR 	= TEMP_D2_OSR_8192;
		break;

		default:
		break;
	}
	#endif
	return 0;
}

//**********************************************************************
// 函数功能:	设置压力传感器的转换频率
// 输入参数：	
// frq:  		压力传感器的转换频率：1~8HZ
// 返回参数：	无
static uint16 PressSensorCvnRateSet(uint8 frq)
{
	if (frq == 0)
	{
		return 0xff;
	}
	sensorRate = frq;
	xTimerChangePeriod(PressureProcessTimer, APP_1SEC_TICK / sensorRate, 0);
	return 0;
}
		
//**********************************************************************
// 函数功能:	设置压力传感器的转换（事件）类型
// 输入参数：	
// presssensorpara: 压力传感器转换事件类型：可选温度、压力、海拔、深度
// 返回参数：	无
static uint16 PressSensorParaSet(uint8 presssensorpara)
{
	if (presssensorpara & ILLEGAL_PARA)
	{
		return 0xff;
	}
	PressSensorPara 	|=	 presssensorpara;
	return 0;
}

//**********************************************************************
// 函数功能:	压力传感器工作状态处理
// 输入参数：	无
// 返回参数：	
// 0x00    :    操作成功
// 0xff    :    操作失败
static uint16 PressSensorDataProcess(void)
{
	volatile uint16 ret  = 0;
	
	Drv_AirPress_EnableIO();
	
	switch(PressSensorState)
	{
		case PressSensorStateIdle:
		if (PressSensorPara)
		{
			#ifdef AIRPRESSURE
			if (PressSensorPara & (~TEMP_PARA))
			{
				ret |= Drv_AirPress_TemPressCvt(PressureOSR);//需要除温度外的测量，同时启动温度与压力转换
			}
			else
			{
				Drv_AirPress_TempCvt(TemperaturOSR);//只需要温度测量
			}			
			#else
			Drv_WaterPress_TempCvt(TemperaturOSR);
			#endif	
			PressSensorState = PressSensorStateTemp;
		}
		else
		{
			PressSensorState = PressSensorStateIdle;
		}
		break;

		case PressSensorStateTemp:
		#ifdef AIRPRESSURE
		if (PressSensorPara & (~TEMP_PARA))//同时启动温度与压力转换，转换未完成
		{
			PressSensorState = PressSensorStatePress;
		}
		else
		{
			ret |= Drv_AirPress_ReadTemp(&mTemperature);//只启动温度转换，转换完成
			
			PressSensorState = PressSensorStateIdle;
		}
		#else
		ret  |= bspWaterPress.ReadTemp(&mTemperature);

		if (PressSensorPara & (~TEMP_PARA))//需要启动温度外的转换
		{
			ret |= Drv_WaterPress_PressCvt(PressureOSR);
			PressSensorState = PressSensorStatePress;
		}
		else
		{	
			PressSensorState = PressSensorStateIdle;
		}		
		#endif
		break;

		case PressSensorStatePress:
		#ifdef AIRPRESSURE
		ret |= Drv_AirPress_ReadTemp(&mTemperature);
		ret |= Drv_AirPress_ReadPress(&mPressure);
		PressInsert(mPressure);

		if (PressSensorPara & ALTITUDE_PARA)//需要海拔
		{
			#ifdef AltitudeFunc				//气压传感器自带海拔计算
			int32 AltitudeTemp   = 0;
			ret |= Drv_AirPress_ReadAltitude(&AltitudeTemp);
			AltitudeInsert(AltitudeTemp);
			mAltitude 	= AltitudeAvr();
			#else
			PressureToAltitude(PressAvr(),mTemperature);//算法计算海拔
			#endif
		}
		#else
		ret  |= bspWaterPress.ReadPress(&mPressure);
		PressInsert(mPressure);
		if (PressSensorPara & ALTITUDE_PARA)//需要深度
		{
			PressureToDepth(PressAvr());
		}
		#endif
		PressSensorState = PressSensorStateIdle;
		break;

		default:
		break;
	}
	Drv_AirPress_DisableIO();
	#ifdef TEST_DEBUG
	SEGGER_RTT_printf(0,"mPressure:%d\n", mPressure);
	SEGGER_RTT_printf(0,"mTemperature:%d\n", mTemperature);
	#endif
	return ret;
}

//**********************************************************************
// 函数功能:	压力传感器初始化
// 输入参数：	无
// 返回参数：	
// 0x00    :    初始化成功
// 0xff    :    初始化失败
uint16 Mid_Pressure_Init(void (*TimerCb)(TimerHandle_t xTimer))
{
	static uint8 	SoftInit = 0;

	PressSensorPara 		= NULL_PARA;//初始化所有参数转换
	PressureOSR 			= RATE_NULL;
	TemperaturOSR 			= RATE_NULL;
	sensorRate 				= 0;
	PressSensorState 		= PressSensorStateIdle;
	PressureIndex			= 0;
	AltitudeIndex 			= 0;
	PressureReady			= 0;
	AltitudeReady			= 0;
	
	PressureProcessTimer    = xTimerCreate("Pressrue", APP_1SEC_TICK, pdTRUE, 0, TimerCb);
	
	#ifdef AIRPRESSURE
    if(Drv_AirPress_EnableIO() != Ret_OK)
	{
		return 0xff;
	}
	if (!SoftInit)
	{
		if (Drv_AirPress_Init())
		{
			return 0xff;
		}		
		SoftInit  = 1;
		Drv_AirPress_SetAltitudeCmps(LevelPressure);
	}
	#else
	if (Drv_WaterPress_EnableIO() != Ret_OK)
	{
		return 0xff;
	}
	if (!SoftInit)
	{
		if(Drv_WaterPress_Init())
		{
			return 0xff;
		}
		SoftInit  = 1;
	}
	#endif

	PressSensorStop();//休眠状态
	
	return 0;
}

//**********************************************************************
// 函数功能:	读取压力值（单位Pa）
// 输入参数：	无
// 返回参数：	
// 0x00    :    操作成功
// 0xff    :    操作失败
uint16 Mid_Pressure_ReadPressure(int32 *Pressure)
{
	*Pressure 	=  mPressure;
	return 0;
}

//**********************************************************************
// 函数功能:	读取温度值（单位C*100）
// 输入参数：	无
// 返回参数：	
// 0x00    :    操作成功
// 0xff    :    操作失败
uint16 Mid_Pressure_ReadTemperature(int16 *Temperature)
{
	*Temperature 	= mTemperature;
	return 0;
}

//**********************************************************************
// 函数功能:	读取海拔高度值 （单位cm）
// 输入参数：	无
// 返回参数：	
// 0x00    :    操作成功
// 0xff    :    操作失败
uint16 Mid_Pressure_ReadAltitude(int32 *Altitude)
{
	*Altitude 	= mAltitude;
	return 0;
}
//**********************************************************************
// 函数功能:	读取深度值（单位cm）
// 输入参数：	无
// 返回参数：	
// 0x00    :    操作成功
// 0xff    :    操作失败
uint16 Mid_Pressure_ReadDepth(int32 *Depth)
{
	*Depth 		= mDepth;
	return 0;
}

//**********************************************************************
// 函数功能:	压力传感器自检 
// 输入参数：	无
// 返回参数：	
// 0x00    :    自检成功
// 0xff    :    自检失败
uint16 Mid_Pressure_SelfTest(void)
{
	#ifdef AIRPRESSURE
	return Drv_AirPress_SelfTest();
	#else
	return Drv_WaterPress_SelfTest();
	#endif
}

//**********************************************************************
// 函数功能:	压力传感器自检 
// 输入参数：	无
// 返回参数：	
// 0x00    :    自检成功
// 0xff    :    自检失败
uint16 PressureFuncRegister(uint8 *funcid, uint8 osr, uint8 paratype,uint8 frq)
{
	uint8 i;
	uint8 idTemp,paraTemp,osrMaxTemp,sensorCvtMaxFrq;

	if (osr == RATE_NULL)
	{
		return 0xff;
	}	

	if (frq == 0)
	{
		return 0xff;
	}
	//id分配
	for (i = 0; i < PRESSURE_NUM_MAX; i++)
	{
		if (pressurePara[i].sensorfrq == 0)
		{
			idTemp 		= i;
			*funcid 	= idTemp;
			break;
		}
	}
	//超出功能上限，注册失败
	if (i == PRESSURE_NUM_MAX)
	{
		return 0xff;
	}

	pressurePara[idTemp].samplerate = osr;
	pressurePara[idTemp].paraswitch = paratype;
	pressurePara[idTemp].sensorfrq  = frq;

	osrMaxTemp 						= RATE_NULL;
	paraTemp 						= NULL_PARA;
	sensorCvtMaxFrq	 				= 0;

	//检查配置参数是否需要修改
	for (i = 0; i < PRESSURE_NUM_MAX; i++)
	{
		if (pressurePara[i].samplerate > osrMaxTemp)
		{
			osrMaxTemp = pressurePara[i].samplerate;
		}
	}

	for (i = 0; i < PRESSURE_NUM_MAX; i++)
	{
		paraTemp |= pressurePara[idTemp].paraswitch & ALL_PARA;
	}

	for (i = 0; i < PRESSURE_NUM_MAX; i++)
	{
		if (pressurePara[i].sensorfrq > sensorCvtMaxFrq)
		{
			sensorCvtMaxFrq = pressurePara[i].sensorfrq;
		}
	}
	//传感器暂停工作
	PressSensorStop();

	//最大采样率改变
	if (osrMaxTemp != PressureOSR)
	{	
		PressSensorSetOSR(osrMaxTemp);	
	}

	if (paraTemp != PressSensorPara)
	{
		PressSensorParaSet(paraTemp);	
	}

	if (sensorCvtMaxFrq != sensorRate)
	{
		PressSensorCvnRateSet(sensorCvtMaxFrq);	
	}

	//重启传感器工作
	PressSensoreStart();

	return 0;
}

//**********************************************************************
// 函数功能:	压力传感器自检 
// 输入参数：	无
// 返回参数：	
// 0x00    :    自检成功
// 0xff    :    自检失败
uint16 PressureFuncUnRegister(uint8 funcid)
{
	uint8 i;
	uint8 idTemp,paraTemp,osrMaxTemp,sensorCvtMaxFrq;

	if (funcid >= PRESSURE_NUM_MAX)
	{
		return 0xff;
	}

	pressurePara[funcid].samplerate 		= RATE_NULL;
	pressurePara[funcid].paraswitch 		= NULL_PARA;
	pressurePara[funcid].sensorfrq 			= 0;

	osrMaxTemp 						= RATE_NULL;
	paraTemp 						= NULL_PARA;
	sensorCvtMaxFrq	 				= 0;

	//检查配置参数是否需要修改
	for (i = 0; i < PRESSURE_NUM_MAX; i++)
	{
		if (pressurePara[i].samplerate > osrMaxTemp)
		{
			osrMaxTemp = pressurePara[i].samplerate;
		}
	}

	for (i = 0; i < PRESSURE_NUM_MAX; i++)
	{
		paraTemp |= pressurePara[idTemp].paraswitch & ALL_PARA;
	}

	for (i = 0; i < PRESSURE_NUM_MAX; i++)
	{
		if (pressurePara[i].sensorfrq > sensorCvtMaxFrq)
		{
			sensorCvtMaxFrq = pressurePara[i].sensorfrq;
		}
	}
	//传感器暂停工作
	PressSensorStop();

	//最大采样率改变
	if (sensorCvtMaxFrq != 0)
	{
		if (osrMaxTemp != PressureOSR)
		{	
			PressSensorSetOSR(osrMaxTemp);	
		}

		if (paraTemp != PressSensorPara)
		{
			PressSensorParaSet(paraTemp);	
		}

		if (sensorCvtMaxFrq != sensorRate)
		{
			PressSensorCvnRateSet(sensorCvtMaxFrq);	
		}		
		//重启传感器工作
		PressSensoreStart();
	}
	else
	{
		sensorRate  = 0;
	}
	return 0;
}


//**********************************************************************
// 函数功能：	加速度事件处理
// 输入参数：	无
// 返回参数：	0x00:成功
// 				0xff:失败
uint16 Mid_Pressure_EventProcess(pressure_event_s* msg)
{
   switch(msg->id)
   {
	   case PRESSURE_FUNC_REGISTER:
		PressureFuncRegister(msg->funcId, msg->osr, msg->paratype,msg->cvtfrq);
	   break;

	   case PRESSURE_FUNC_UN_REGISTER:
	   PressureFuncUnRegister(*msg->funcId);
	   break;

	   case PRESSURE_CAL_LEVEL_PRESS:
       PressSensorCalLevelPressure(msg->altitude,msg->pressure,msg->temperature);
       break;
       
       case PRESSURE_DATA_PROCESS:
       PressSensorDataProcess();
       break;

       default:
       break;
   }
   return 0;
}



