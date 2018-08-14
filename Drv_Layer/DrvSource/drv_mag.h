#ifndef MAGNETISM_H
#define MAGNETISM_H

#include "platform_common.h"

// magnetism work mode
typedef enum 
{
	MAG_STANDBY_MODE 		= 0x00,
	MAG_CONTINUOUS_MODE		= 0x01,
	MAG_SELFTEST_MODE 		= 0x02,
}bsp_mag_mode;

//Full Scale range   
typedef enum 
{
	MAG_SCALE_RANGE_2GS   	= 0x00,
	MAG_SCALE_RANGE_8GS   	= 0x01,
	MAG_SCALE_RANGE_12GS   	= 0x02,
	MAG_SCALE_RANGE_20GS  	= 0x03,
}bsp_mag_scalerange;

//Output Data Rate  
typedef enum 
{
	MAG_DATA_RATE_10HZ   	= 0x00,
	MAG_DATA_RATE_50HZ   	= 0x01,
	MAG_DATA_RATE_100HZ   	= 0x02,
	MAG_DATA_RATE_200HZ  	= 0x03,
}bsp_mag_datarate;

//Over  sample Rate 
typedef enum 
{
	MAG_OVER_SAMPLE_RATE_512   	= 0x00,
	MAG_OVER_SAMPLE_RATE_256   	= 0x01,
	MAG_OVER_SAMPLE_RATE_128   	= 0x02,
	MAG_OVER_SAMPLE_RATE_64  	= 0x03,
}bsp_mag_osr;

//**********************************************************************
// 函数功能:    地磁传感器初始化
// 输入参数：    无
// 返回参数：    
// 0x00    :    初始化成功
// 0xff    :    初始化失败
//**********************************************************************
extern uint8 Drv_Mag_Open(void);
//**********************************************************************
// 函数功能:    地磁传感器硬件关闭
// 输入参数：    无
// 返回参数：    
// 0x00    :    设置成功
// 0xff    :    设置失败
//**********************************************************************
extern uint8 Drv_Mag_Close(void);

//**********************************************************************
// 函数功能:    地磁传感器采样率及量程设置
// 输入参数：    
// sampleRate    采样率
//               Valid Values for uint16 sampleRate are:
//               MAG_DATA_RATE_10HZ
//               MAG_DATA_RATE_50HZ
//               MAG_DATA_RATE_100HZ
//               MAG_DATA_RATE_200HZ
// scaleRange    测量量程
//               fullscale selection 
//               Valid Values for uint8 scaleRange are:
//               MAG_SCALE_RANGE_2GS
//               MAG_SCALE_RANGE_8GS
//               MAG_SCALE_RANGE_12GS
//               MAG_SCALE_RANGE_20GS
// 返回参数：    
// 0x00    :    初始化成功
// 0xff    :    初始化失败
//**********************************************************************
extern uint8 Drv_Mag_Set(uint16 sampleRate, uint8 scaleRange);

//**********************************************************************
// 函数功能:  读取地磁传感器3轴数据，，数据为二进补码形式
// 输入参数：    
// axisData ：三轴数据指针
// 返回参数：    
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
extern uint8 Drv_Mag_Read(int16 xyzData[3]);

//**********************************************************************
// 函数功能:    地磁传感器唤醒
// 输入参数：   无
// 返回参数：    
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
extern uint8 Drv_Mag_WakeUp(void);

//**********************************************************************
// 函数功能:    设置地磁传感器进入睡眠状态
// 输入参数：   无
// 返回参数：    
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
extern uint8 Drv_Mag_GotoSleep(void);

//**********************************************************************
// 函数功能:    地磁传感器自检
// 输入参数：    无
// 返回参数：    
// 0x00    :    自检成功
// 0x01    :    自检失败
//**********************************************************************
extern uint8 Drv_Mag_SelfTest(void);

#endif

