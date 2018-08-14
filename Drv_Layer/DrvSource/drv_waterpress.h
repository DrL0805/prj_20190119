/**********************************************************/
//  V1.0   2017-7-12    created by yudongyou
/*********************************************************/
#ifndef WATER_PRESSURE_H
#define WATER_PRESSURE_H

#include "platform_common.h"

//sample rate
// pressure 
typedef enum{
	PRESS_D1_OSR_256 = 0x40, //conversion time:0.54ms
	PRESS_D1_OSR_RESERVE1,
	PRESS_D1_OSR_512,        //1.06ms
	PRESS_D1_OSR_RESERVE2,
	PRESS_D1_OSR_1024,   	//2.08ms
	PRESS_D1_OSR_RESERVE3,
	PRESS_D1_OSR_2048,		//4.13ms
	PRESS_D1_OSR_RESERVE4,
	PRESS_D1_OSR_4096,		//8.22ms
	PRESS_D1_OSR_RESERVE5,
	PRESS_D1_OSR_8192,		//16.44ms
}Press_OSR_s;

// temperature 
typedef enum{	
	TEMP_D2_OSR_256 = 0x50,	//conversion time:0.54ms
	TEMP_D2_OSR_RESERVE1,
	TEMP_D2_OSR_512,		//1.06ms
	TEMP_D2_OSR_RESERVE2,
	TEMP_D2_OSR_1024,		//2.08ms
	TEMP_D2_OSR_RESERVE3,
	TEMP_D2_OSR_2048,		//4.13ms
	TEMP_D2_OSR_RESERVE4,
	TEMP_D2_OSR_4096,		//8.22ms
	TEMP_D2_OSR_RESERVE5,
	TEMP_D2_OSR_8192,		//16.44ms
}Temp_OSR_s;

typedef enum 
{
	SensorIdle 	= 0x00,
	PressCVT	= 0x01,
	TempCVT		= 0x02,	
}WaterPressSensorState;

//**********************************************************************
// 函数功能:    使能模块使用的IO功能
// 输入参数：	无
// 返回参数：	
// 0x00    :    设置成功
// 0x01    :    设置失败
//**********************************************************************
extern uint8 Drv_WaterPress_EnableIO(void);

//**********************************************************************
// 函数功能:	关闭模块使用的IO功能，实现低功耗
// 输入参数：	无
// 返回参数：	
// 0x00    :    设置成功
// 0x01    :    设置失败
//**********************************************************************
extern uint8 Drv_WaterPress_DisableIO(void);

//**********************************************************************
// 函数功能: 传感器软件初始化
// 输入参数：无
// 返回参数：	
// 0x00    : 设置成功
// 0x01    : 设置失败
//**********************************************************************
extern uint8 Drv_WaterPress_Init(void);

//**********************************************************************
// 函数功能: 硬件关闭
// 输入参数：无
// 返回参数：	
// 0x00    : 设置成功
// 0xff    : 设置失败
//**********************************************************************
extern uint8 Drv_WaterPress_Close(void);

//**********************************************************************
// 函数功能: 启动传感器压力ADC转换
// 输入参数：无
// 返回参数：	
// 0x00    : 设置成功
// 0x01    : 设置失败
// 0x03    : 设备忙
//**********************************************************************
extern uint8 Drv_WaterPress_PressCvt(uint8 PressOSR);

//**********************************************************************
// 函数功能: 启动传感器温度ADC转换
// 输入参数：无
// 返回参数：	
// 0x00    : 设置成功
// 0x01    : 设置失败
// 0x03    : 设备忙
//**********************************************************************
extern uint8 Drv_WaterPress_TempCvt(uint8 TempOSR);

//**********************************************************************
// 函数功能:  读取传感器AD值（温度/压力AD值）
// 输入参数： 无
// 返回参数：	
// 0x00    :  设置成功
// 0x01    :  设置失败
//**********************************************************************
extern uint8 Drv_WaterPress_ReadADC(uint32 *adcval);

//**********************************************************************
// 函数功能: 根据温度AD值计算温度值（单位C*100）
// 输入参数：无
// 返回参数：	
// 0x00    : 设置成功
// 0x01    : 设置失败
//**********************************************************************
extern uint8 Drv_WaterPress_CalTemp(uint32 tempDval,int32 *Temp);

//**********************************************************************
// 函数功能: 根据温度及压力AD值计算压力值（单位Pa）
// 输入参数：无
// 返回参数：	
// 0x00    : 设置成功
// 0x01    : 设置失败
//**********************************************************************
extern uint8 Drv_WaterPress_CalPress(uint32 tempDval, uint32 pressDval, int32 *Press);

//**********************************************************************
// 函数功能: 设置传感器空闲状态
// 输入参数：无
// 返回参数：	
// 0x00    : 设置成功
// 0x01    : 设置失败
//**********************************************************************
extern uint8 Drv_WaterPress_SetIdle(void);

//**********************************************************************
// 函数功能: 传感器自检
// 输入参数：无
// 返回参数：	
// 0x00    : 自检通过
// 0x01    : 自检失败
//**********************************************************************
extern uint8 Drv_WaterPress_SelfTest(void);
#endif

