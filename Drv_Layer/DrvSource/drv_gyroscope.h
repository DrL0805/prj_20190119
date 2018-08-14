#ifndef GYROSCOPE_H
#define GYROSCOPE_H

#include "platform_common.h"

// 带宽及OSR设置
typedef enum
{
	GYO_SAMPLERATE_NONE 	= 0x00,
	GYO_SAMPLERATE_25HZ		= 0x06,
	GYO_SAMPLERATE_50HZ		= 0x07,
	GYO_SAMPLERATE_100HZ	= 0x08,
	GYO_SAMPLERATE_200HZ	= 0x09,
	GYO_SAMPLERATE_400HZ	= 0x0A,
	GYO_SAMPLERATE_800HZ	= 0x0B,
	GYO_SAMPLERATE_1600HZ	= 0x0C,
	GYO_SAMPLERATE_3200HZ	= 0x0D,
}bsp_gyroscope_osr;


//量程设置
typedef enum
{
	GYRO_SCALE_RANGE_2000_DEG_SEC          = 0x00,
	GYRO_SCALE_RANGE_1000_DEG_SEC          = 0x01,
	GYRO_SCALE_RANGE_500_DEG_SEC           = 0x02,
	GYRO_SCALE_RANGE_250_DEG_SEC           = 0x03,	
	GYRO_SCALE_RANGE_125_DEG_SEC           = 0x04,	
}bsp_gyroscope_scalerange;

#define GYRO_BUFFER_DISABLE 0x00
#define GYRO_BUFFER_ENABLE  0x01


//**********************************************************************
// 函数功能:    陀磥仪传感器初始化
// 输入参数：    无
// 返回参数：    
// 0x00    :    初始化成功
// 0xff    :    初始化失败
//**********************************************************************
extern uint8 Drv_Gyro_Open(void);

//**********************************************************************
// 函数功能:    硬件关闭
// 输入参数：    无
// 返回参数：    
// 0x00    :    设置成功
// 0xff    :    设置失败
//**********************************************************************
extern uint8 Drv_Gyro_Close(void);

//**********************************************************************
// 函数功能:    陀磥仪传感器采样率和量程设置
// 输入参数：    
// sampleRate    采样率
//               Valid Values for uint16_t sampleRate are:
//               GYO_SAMPLERATE_5HZ_1KBW        GYO_SAMPLERATE_10HZ_1KBW
//               GYO_SAMPLERATE_20HZ_1KBW       GYO_SAMPLERATE_41HZ_1KBW
//               GYO_SAMPLERATE_92HZ_1KBW       GYO_SAMPLERATE_184HZ_1KBW
//               GYO_SAMPLERATE_250HZ_8KBW      GYO_SAMPLERATE_3K6HZ_8KBW
//               GYO_SAMPLERATE_3K6HZ_32KBW     GYO_SAMPLERATE_8K8HZ_32KBW
// scaleRange    测量量程
//               Gyro fullscale selection : +250dps (00), +500dps (01), +1000dps (10), +2000dps (11)
//               Valid Values for uint8_t scaleRange are:
//               GYOR_SCALE_RANGE_250    GYOR_SCALE_RANGE_500
//               GYOR_SCALE_RANGE_1000   GYOR_SCALE_RANGE_2000
// 返回参数：    
// 0x00    :    初始化成功
// 0xff    :    初始化失败
//**********************************************************************
extern uint8 Drv_Gyro_Set(uint16 sampleRate, uint8 scaleRange);

//**********************************************************************
// 函数功能:    陀螺仪传感器FIFO功能开关设置
// 输入参数：    
// setState ：   FIFO开关，0x00关闭，0x01使能
// 返回参数：    
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
extern uint8 Drv_Gyro_SetBuffer(uint16 setState);

//**********************************************************************
// 函数功能:    读取陀螺仪传感器3轴数据，数据为二进补码形式
// 输入参数：    
// axisData ：   三轴数据指针
// 返回参数：    
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
extern uint8 Drv_Gyro_Read(int16 xyzData[3]);

//**********************************************************************
// 函数功能:    从陀螺仪传感器FIFO读取3轴数据
// 输入参数：    
// CacheArry ：          三轴数据指针
// CacheMaxWriteLenght:  读取的数据长度
//FIFOReadNumber：       FIFO有效数据长度
// 返回参数：    
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
extern uint8 Drv_Gyro_ReadBuffer(int16 *CacheArry, uint16 CacheMaxWriteLenght, uint16 *FIFOReadNumber);

//**********************************************************************
// 函数功能:    陀螺仪传感器唤醒
// 输入参数：   无
// 返回参数：    
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
extern uint8 Drv_Gyro_WakeUp(void);

//**********************************************************************
// 函数功能:    设置陀螺仪传感器进入睡眠状态
// 输入参数：   无
// 返回参数：    
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
extern uint8 Drv_Gyro_GoSleep(void);

//**********************************************************************
// 函数功能:   陀螺仪传感器自检测
// 输入参数：   无
// 返回参数：    
// 0x00    :    自检成功
// 0xff    :    自检失败
//**********************************************************************
extern uint8 Drv_Gyro_SelfTest(void);

#endif

