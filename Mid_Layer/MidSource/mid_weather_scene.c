#include "platform_common.h"


/* FreeRTOS includes */
#include "rtos.h"

#include "mid_weather_scene.h"
#include "multimodule_task.h"



/*************** func declaration ********************************/



/*************** macro define ********************************/
#define 	PRESSRUE_LEVEL_HIGH 		103000
#define 	PRESSRUE_LEVEL_MID 			101500
#define 	PRESSRUE_LEVEL_LOW 			98500

#define 	VALID_NUM_MAX 			4//用于天气预报的气压个数
#define 	SAMPLE_NUM 				6//采样气压个数

// #define 	FORECAST_PERIOD 		7210//2小时（7200秒）
// #define 	CHECK_PERIOD 			1800//30分钟检测一次
// #define 	SAMPLE_PERIOD 		 	300//5分钟采样一次

/************** variable define *****************************/
static 	int32 pressurebuf[VALID_NUM_MAX];//2小时预报一次，每30分钟检测一次气压值
static 	int32 pressuretemp[SAMPLE_NUM];//每启动一次检测，测量6个气压值，周期5分钟

static  uint8 smaplecnt;				//采样值计数
static  uint8 validcnt;				//有效值计数

static 	weahter_s weahterInfo;		//天气趋势	

static 	uint8 weatherFuncId;


/************** function define *****************************/


//**********************************************************************
// 函数功能：  天气预报功能初始化
// 输入参数：   
// 返回参数：  无
uint16 Mid_WeatherScene_Init(void)
{
	uint8 i;

	smaplecnt 			= 0;
	validcnt 			= 0;
	weahterInfo.weahterStatus 	= WEATHER_TENDENCY_CLOUDY;
	weahterInfo.weahterCurTemperature = 26;
	weahterInfo.weahterMaxTemperature = 26;
	weahterInfo.weahterMinTemperature = 26;

	for (i = 0; i < VALID_NUM_MAX; i++)
	{
		pressurebuf[i] 	= 0;
	}

	for (i = 0; i < SAMPLE_NUM; i++)
	{
		pressuretemp[i] 	= 0;
	}

	return 0;
}

//**********************************************************************
// 函数功能：  天气预报功能启动
// 输入参数：   
// 返回参数：  无
uint16 WeatherForecastOpen(void)
{
	uint8 i;
	multimodule_task_msg_t msg;

	smaplecnt 			= 0;
	validcnt 			= 0;

	for (i = 0; i < VALID_NUM_MAX; i++)
	{
		pressurebuf[i] 	= 0;
	}

	for (i = 0; i < SAMPLE_NUM; i++)
	{
		pressuretemp[i] 	= 0;
	}

	//新注册压力功能事件
    msg.id                                  = PRESSURE_ID;     
    msg.module.pressureEvent.id             = PRESSURE_FUNC_REGISTER;
	msg.module.pressureEvent.funcId 		= &weatherFuncId;
	msg.module.pressureEvent.cvtfrq 		= PRESSURE_1HZ;//转换频率
	msg.module.pressureEvent.osr 			= RATE_128;//采样率
	msg.module.pressureEvent.paratype 		= ALL_PARA;//获取参数
    MultiModuleTask_EventSet(msg); 

	return 0;
}

//**********************************************************************
// 函数功能：  天气预报功能关闭
// 输入参数：   
// 返回参数：  无
uint16 WeatherForecastClose(void)
{	
	multimodule_task_msg_t msg;

	//注销压力功能事件
	msg.id                                  = PRESSURE_ID;     
    msg.module.pressureEvent.id             = PRESSURE_FUNC_UN_REGISTER;
	msg.module.pressureEvent.funcId 		= &weatherFuncId;
	MultiModuleTask_EventSet(msg); 

	return 0;
}

//**********************************************************************
// 函数功能：  天气预报更新,2小时更新1次
// 输入参数：   
// 返回参数：  无
static uint16 WeatherForecastUpdata(void)
{
#if 0	
	uint8 i;
	int32 minp,maxp;
	int32 mim_idex,maxp_idex;

	minp 		= pressurebuf[0];
	maxp 		= pressurebuf[0];

	for (i = 0; i < VALID_NUM_MAX; i++)
	{
		if (minp > pressurebuf[i])
		{
			minp 		= pressurebuf[i];
			mim_idex 	= i;
		}
		if (maxp < pressurebuf[i])
		{
			maxp 		= pressurebuf[i];
			maxp_idex  	= i;
		}
	}

	if(mim_idex < maxp_idex)//气压升高趋势
	{
		if(minp < PRESSRUE_LEVEL_LOW)
		{
			if (maxp < PRESSRUE_LEVEL_LOW)//气压变化小
			{
				weahterInfo.weahterStatus = WEATHER_KEEP;
			}
			else if (maxp < PRESSRUE_LEVEL_HIGH)//气压回上升到晴天状态
			{
				weahterInfo.weahterStatus = WEATHER_GOOD;
			}
			else 								//气压上升急速
			{
				weahterInfo.weahterStatus = WEATHER_BAD;
			}
		}
		else if(minp < PRESSRUE_LEVEL_MID) 
		{
			if (maxp < PRESSRUE_LEVEL_HIGH)//气压晴天范围内
			{
				weahterInfo.weahterStatus = WEATHER_GOOD;
			}
			else
			{
				weahterInfo.weahterStatus = WEATHER_BAD;
			}
		}
		else if (minp < PRESSRUE_LEVEL_HIGH)//气压升至风雨区
		{
			if (maxp > PRESSRUE_LEVEL_HIGH)
			{
				weahterInfo.weahterStatus = WEATHER_WORSE;
			}
			else
			{
				weahterInfo.weahterStatus = WEATHER_BAD;
			}
		}
		else 										//气压骤变，天气恶劣
		{
			weahterInfo.weahterStatus = WEATHER_WORSE;
		}
	}
	else					//气压降低趋势
	{
		if (maxp > PRESSRUE_LEVEL_HIGH)
		{
			 if (minp > PRESSRUE_LEVEL_HIGH)//气压变化不大，处于低压区
			 {
			 	weahterInfo.weahterStatus = WEATHER_KEEP;
			 }
			 else if (minp > PRESSRUE_LEVEL_MID)//气压下低到风暴区
			 {
			 	weahterInfo.weahterStatus = WEATHER_BAD;
			 }
			 else if (minp > PRESSRUE_LEVEL_LOW)//气压降到晴天区
			 {
			 	weahterInfo.weahterStatus = WEATHER_GOOD;
			 }
			 else 								//气压骤变，天气恶劣
			 {
			 	weahterInfo.weahterStatus = WEATHER_WORSE;
			 }

		}
		else if (maxp > PRESSRUE_LEVEL_MID)
		{
			if (minp > PRESSRUE_LEVEL_MID)//气压下低到风暴区
			 {
			 	weahterInfo.weahterStatus = WEATHER_KEEP;
			 }
			 else if (minp > PRESSRUE_LEVEL_LOW)//气压降到晴天区
			 {
			 	weahterInfo.weahterStatus = WEATHER_GOOD;
			 }
			 else 								//气压骤变，天气恶劣
			 {
			 	weahterInfo.weahterStatus = WEATHER_WORSE;
			 }
		}
		else if (maxp > PRESSRUE_LEVEL_LOW)
		{
			if (minp > PRESSRUE_LEVEL_LOW)//气压降到晴天区
			 {
			 	weahterInfo.weahterStatus = WEATHER_GOOD;
			 }
			 else 								//气压骤变，天气恶劣
			 {
			 	weahterInfo.weahterStatus = WEATHER_WORSE;
			 }
		}
		else
		{
			weahterInfo.weahterStatus = WEATHER_WORSE;
		}
	}
	#endif
	validcnt 	= 0;
	smaplecnt 	= 0;
	return 0;
}

//**********************************************************************
// 函数功能：  启动气压检测，30分钟检测一次
// 输入参数：   
// 返回参数：  无
static uint16 WeatherForecastPressureCheck(void)
{
	uint8 i;
	int32 minp,maxp;
	int32 sump = 0;

	if (validcnt < VALID_NUM_MAX)
	{
		minp 		= pressuretemp[0];
		maxp 		= pressuretemp[0];
		for (i = 0; i < SAMPLE_NUM; i++)
		{
			sump += pressuretemp[i];
			if (minp > pressuretemp[i])
			{
				minp = pressuretemp[i];
			}
			if (maxp < pressuretemp[i])
			{
				maxp = pressuretemp[i];
			}
		}
		pressurebuf[validcnt] = (sump - minp - maxp) / (SAMPLE_NUM -2);
		validcnt++;
		smaplecnt 			= 0;
	}
	return 0;
}

//**********************************************************************
// 函数功能：  气压采样，5分钟一次,采样6个值
// 输入参数：   
// 返回参数：  无
static uint16 WeatherForecastPressureSample(void)
{
	if (smaplecnt < SAMPLE_NUM)//启动气压转换事件
	{
		Mid_Pressure_ReadPressure(&pressuretemp[smaplecnt]);
		smaplecnt ++;
	}
	return 0;
}

//**********************************************************************
// 函数功能：  气温采样，1分钟一次
// 输入参数：   
// 返回参数：  无
static uint16 WeatherForecastTempSample(void)
{
	static uint8 firstTempData = 1;
	int16 temperatureTemp;

	Mid_Pressure_ReadTemperature(&temperatureTemp);

	temperatureTemp /= 100;
	if (firstTempData)
	{
		weahterInfo.weahterCurTemperature = temperatureTemp;
		weahterInfo.weahterMaxTemperature = temperatureTemp;
		weahterInfo.weahterMinTemperature = temperatureTemp;
		firstTempData = 0;
	}
	else
	{
		weahterInfo.weahterCurTemperature = temperatureTemp;

		if (temperatureTemp > weahterInfo.weahterMaxTemperature)
		{
			weahterInfo.weahterMaxTemperature = temperatureTemp;
		}

		if (temperatureTemp < weahterInfo.weahterMinTemperature)
		{
			weahterInfo.weahterMinTemperature = temperatureTemp;
		}
	}

	return 0;
}
//**********************************************************************
// 函数功能：  天气预报事件处理
// 输入参数：   
// 返回参数：  无
uint16 Mid_WeatherScene_EventProcess(weather_event_s* msg)
{
	switch(msg->id)
	{
		case WEATHER_FORECAST_OPEN:
		WeatherForecastOpen();
		break;

		case WEATHER_FORECAST_CLOSE:
		WeatherForecastClose();
		break;

		case WEATHER_FORECAST_PRESS_SAMPLE:
		#if(AIR_PRESSURE_SUPPORT == 1)
		WeatherForecastPressureSample();
		#endif
		break;

		case WEATHER_FORECAST_TEMP_SAMPLE:
		#if(AIR_PRESSURE_SUPPORT == 1)
		WeatherForecastTempSample();
		#endif
		break;

		case WEATHER_FORECAST_CHECK:
		#if(AIR_PRESSURE_SUPPORT == 1)
		WeatherForecastPressureCheck();
		#endif
		break;

		case WEATHER_FORECAST_UPDATA:
		#if(AIR_PRESSURE_SUPPORT == 1)
		WeatherForecastUpdata();
		#endif
		break;	

		default:
		break;		
	}
	return 0;
}

//**********************************************************************
// 函数功能：  获取天气趋势
// 输入参数：   
// 返回参数：  无
//**********************************************************************
uint16 Mid_WeatherScene_TendencyGet(weahter_s *weatherinfo)
{
	weatherinfo->weahterStatus 		= weahterInfo.weahterStatus;
	weatherinfo->weahterCurTemperature = weahterInfo.weahterCurTemperature;
	weatherinfo->weahterMaxTemperature  = weahterInfo.weahterMaxTemperature;
	weatherinfo->weahterMinTemperature = weahterInfo.weahterMinTemperature;
	return 0;
}

//**********************************************************************
// 函数功能：  设置天气趋势
// 输入参数：   
// 返回参数：  无
//**********************************************************************
uint16 Mid_WeatherScene_TendencySet(weahter_s *weatherinfo)
{
	weahterInfo.weahterStatus 		  = weatherinfo->weahterStatus;
	//不带气压时可由外部设置
	#if(AIR_PRESSURE_SUPPORT == 0)
	weahterInfo.weahterCurTemperature = weatherinfo->weahterCurTemperature;
	weahterInfo.weahterMaxTemperature = weatherinfo->weahterMaxTemperature;
	weahterInfo.weahterMinTemperature = weatherinfo->weahterMinTemperature;
	#endif
	return 0; 
}


