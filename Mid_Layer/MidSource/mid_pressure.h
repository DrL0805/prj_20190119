#ifndef MID_PRESSURE_H
#define MID_PRESSURE_H

//气压或水压传感器选择
//#define 	WATERPRESSURE
#define 	AIRPRESSURE

#include "platform_common.h"

typedef enum
{
	PRESSURE_FUNC_REGISTER,
	PRESSURE_FUNC_UN_REGISTER,
	PRESSURE_CAL_LEVEL_PRESS,
	PRESSURE_DATA_PROCESS,
}PRESSURE_INTERFACE_T;



typedef struct 
{
	uint16		id;			//事件id
	uint8 		*funcId;	//注册使用的功能ID
	uint8 		cvtfrq; 	//注册使用的转换频率
	uint8 		osr; 		//注册设置的下采样率
	uint8 		paratype; 	//注册需要的参数类型
	int32 		altitude; 	//海拔数据，以下在计算海平面气压时需要
	int32 		pressure; 	//压力数据
	int16 		temperature;//温度数据
}pressure_event_s;

typedef enum
{
	RATE_NULL 	= 0x00,
	RATE_1,
	RATE_2,
	RATE_4,
	RATE_8,
	RATE_16,
	RATE_32,
	RATE_64,
	RATE_128,
}pressure_sensor_rate; 


typedef enum
{
	NULL_PARA 		= 0x00,
	TEMP_PARA		= 0x01,
	PRESS_PARA  	= 0x02,
#ifdef AIRPRESSURE
	ALTITUDE_PARA	= 0x04,
#else
	DEPTH_PARA		= 0x04,
#endif
	ALL_PARA		= 0x07,
	ILLEGAL_PARA	= 0xF8,

}pressure_sensor_para; 


// Setting Options
typedef enum {
	PRESSURE_1HZ   = 1,
	PRESSURE_2HZ   = 2,
	PRESSURE_4HZ   = 4,
	PRESSURE_8HZ   = 8,
	PRESSURE_16HZ  = 16,
	PRESSURE_32HZ  = 32,
}pressure_freq_option_app;



uint16 Mid_Pressure_Init(void (*TimerCb)(TimerHandle_t xTimer));
uint16 Mid_Pressure_ReadPressure(int32 *Pressure);
uint16 Mid_Pressure_ReadTemperature(int16 *Temperature);
uint16 Mid_Pressure_ReadAltitude(int32 *Altitude);
uint16 Mid_Pressure_ReadDepth(int32 *Depth);
uint16 Mid_Pressure_SelfTest(void);
uint16 Mid_Pressure_EventProcess(pressure_event_s* msg);


#endif

