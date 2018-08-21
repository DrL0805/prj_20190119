
#include "rtos.h"
#include "mid_weather_scene.h"

static 	Mid_Weahter_Param_t Mid_Weahter;			//天气趋势	

/************** function define *****************************/


//**********************************************************************
// 函数功能：  天气预报功能初始化
// 输入参数：   
// 返回参数：  无
uint16 Mid_WeatherScene_Init(void)
{
	Mid_Weahter.Status 	= WEATHER_TENDENCY_CLOUDY;
	Mid_Weahter.CurTemperature = 26;
	Mid_Weahter.MaxTemperature = 26;
	Mid_Weahter.MinTemperature = 26;

	return 0;
}

//**********************************************************************
// 函数功能：  天气预报功能启动
// 输入参数：   
// 返回参数：  无
uint16 WeatherForecastOpen(void)
{
	
	return 0;
}

//**********************************************************************
// 函数功能：  天气预报功能关闭
// 输入参数：   
// 返回参数：  无
uint16 WeatherForecastClose(void)
{	

	return 0;
}

//**********************************************************************
// 函数功能：  天气预报更新,2小时更新1次
// 输入参数：   
// 返回参数：  无
static uint16 WeatherForecastUpdata(void)
{

	return 0;
}

//**********************************************************************
// 函数功能：  启动气压检测，30分钟检测一次
// 输入参数：   
// 返回参数：  无
static uint16 WeatherForecastPressureCheck(void)
{

	return 0;
}

//**********************************************************************
// 函数功能：  气压采样，5分钟一次,采样6个值
// 输入参数：   
// 返回参数：  无
static uint16 WeatherForecastPressureSample(void)
{

	return 0;
}

//**********************************************************************
// 函数功能：  气温采样，1分钟一次
// 输入参数：   
// 返回参数：  无
static uint16 WeatherForecastTempSample(void)
{

	return 0;
}

//**********************************************************************
// 函数功能：  获取天气趋势
// 输入参数：   
// 返回参数：  无
//**********************************************************************
uint16 Mid_WeatherScene_TendencyGet(Mid_Weahter_Param_t *weatherinfo)
{
	weatherinfo->Status 		= Mid_Weahter.Status;
	weatherinfo->CurTemperature = Mid_Weahter.CurTemperature;
	weatherinfo->MaxTemperature  = Mid_Weahter.MaxTemperature;
	weatherinfo->MinTemperature = Mid_Weahter.MinTemperature;
	return 0;
}

//**********************************************************************
// 函数功能：  设置天气趋势
// 输入参数：   
// 返回参数：  无
//**********************************************************************
uint16 Mid_WeatherScene_TendencySet(Mid_Weahter_Param_t *weatherinfo)
{
	Mid_Weahter.Status 		  = weatherinfo->Status;

	Mid_Weahter.CurTemperature = weatherinfo->CurTemperature;
	Mid_Weahter.MaxTemperature = weatherinfo->MaxTemperature;
	Mid_Weahter.MinTemperature = weatherinfo->MinTemperature;
	return 0; 
}


