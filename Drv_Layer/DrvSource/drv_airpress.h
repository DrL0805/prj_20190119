#ifndef AIR_PRESSURE_H
#define AIR_PRESSURE_H

#include "platform_common.h"

//oversampling rate
typedef enum
{
    OSR_1 	= 0x00,	//3.6ms 
    OSR_2 	= 0x01,	//5.2ms
    OSR_4 	= 0x02,	//8.4ms
    OSR_8 	= 0x03,	//14.8ms
    OSR_16 	= 0x04,	//27.6ms
    OSR_32 	= 0x05,	//53.2ms
	OSR_64 	= 0x06,	//104.4ms
    OSR_128 = 0x07,	//206.8ms
}OSR_s;

//measurement rate
typedef enum
{
    MR_1 	= 0x00, 
    MR_2 	= 0x10,
    MR_4 	= 0x20,
    MR_8 	= 0x30,
    MR_16 	= 0x40,
    MR_32 	= 0x50,
	MR_64 	= 0x60,
    MR_128 	= 0x70,
}MR_s;

//control reg status
typedef enum
{
    COEF_RDY  	= 0x80,		// 系数完成
    SENSOR_RDY  = 0x40,		// 传感器初始化完成
    TMP_RDY  	= 0x20,		// 温度新数据可读
    RPS_RDY 	= 0x10,		// 气压新数据可读
}AirPressSensorState;

// measurement mode and type
typedef enum
{
    STANDBY  		= 0x00,		// 待机模式
    PRS_MEAS  		= 0x01,		// 气压测量
    TMP_MEAS  		= 0x02,		// 温度测量
    BG_PRS_MEAS 	= 0x05,		// 周期性气压测量
	BG_TMP_MEAS 	= 0x06,		// 周期性温度测量
	BG_ALL_MEAS		= 0x07,		// 周期性气压和温度测量
}AirPressMode;

//**********************************************************************
// 函数功能:	获取传感器状态
// 输入参数：	无
// 返回参数：	传感器状态
//**********************************************************************
extern uint8 Drv_AirPress_GetStatus(void);

//**********************************************************************
// 函数功能:    使能模块使用的IO功能
// 输入参数：	无
// 返回参数：	
// 0x00    :    设置成功
// 0x01    :    设置失败
//**********************************************************************
extern uint8 Drv_AirPress_EnableIO(void);

//**********************************************************************
// 函数功能:	关闭模块使用的IO功能，实现低功耗
// 输入参数：	无
// 返回参数：	
// 0x00    :    设置成功
// 0x01    :    设置失败
//**********************************************************************
extern uint8 Drv_AirPress_DisableIO(void);

//**********************************************************************
// 函数功能:	传感器软件初始化
// 输入参数：	无
// 返回参数：	
// 0x00    :    设置成功
// 0x01    :    设置失败
//**********************************************************************
extern uint8 Drv_AirPress_Init(void);

//**********************************************************************
// 函数功能:	硬件关闭
// 输入参数：	无
// 返回参数：	
// 0x00    :    设置成功
// 0x01    :    设置失败
//**********************************************************************
extern uint8 Drv_AirPress_Close(void);

//**********************************************************************
// 函数功能:	传感器自校准
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
extern void Drv_AirPress_Calibrate(void);

//**********************************************************************
// 函数功能:	启动温度转换
// 输入参数：	
// OSR_Val ： 	使用的下采集率
// 返回参数：	
// 0x00    :  设置成功
// 0x01    :  设置失败
// 0x02    :  参数非法
//**********************************************************************
extern uint8 Drv_AirPress_TempCvt(uint8 OSR_Val);

//**********************************************************************
// 函数功能:  启动温度和压力转换
// 输入参数：	
// OSR_Val ： 使用的下采集率
// 返回参数：	
// 0x00    :  设置成功
// 0x01    :  设置失败
// 0x02    :  参数非法
//**********************************************************************
extern uint8 Drv_AirPress_TemPressCvt(uint8 OSR_Val);

//**********************************************************************
// 函数功能:	读取压力值（单位Pa），需要注意无符号数值到有符号数值的转换
// 输入参数：	
// Press ： 	压力值指针
// 返回参数：	
// 0x00    :    设置成功
// 0x01    :    设置失败
//**********************************************************************
extern uint8 Drv_AirPress_ReadPress(int32 *Press);

//**********************************************************************
// 函数功能:  读取温度值（单位C*100），需要注意无符号数值到有符号数值的转换
// 输入参数：	
// Temp    ： 温度值指针
// 返回参数：	
// 0x00    :  设置成功
// 0x01    :  设置失败
//**********************************************************************
extern uint8 Drv_AirPress_ReadTemp(int16 *Temp);

//**********************************************************************
// 函数功能:      设置海拔高度计算值的补偿量
// 输入参数：	
// LocalAvgPress：当前的标准大气压力值（单位Pa）
// 返回参数：	
// 0x00    :      设置成功
// 0xff    :      设置失败
//**********************************************************************
extern uint8 Drv_AirPress_SetAltitudeCmps(int32 LocalAvgPress);

//**********************************************************************
// 函数功能: 传感器自检
// 输入参数：无
// 返回参数：	
// 0x00    : 自检通过
// 0x01    : 自检失败
//**********************************************************************
extern uint8 Drv_AirPress_SelfTest(void);

//**********************************************************************
// 函数功能:    设置传感器standby　mode
// 输入参数：    无
// 返回参数：    
// 0x00    :    设置成功
// 0x01    :    设置失败
//**********************************************************************
uint8 Drv_AirPress_Standby(void);


uint8 Drv_AirPress_PressCvtParaConfig(uint8 OSR_Val);

uint8 Drv_AirPress_TempCvtParaConfig(uint8 OSR_Val);

void Drv_AirPress_SoftReset(void);


#endif
