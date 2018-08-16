#ifndef ACCELERATE_H
#define ACCELERATE_H

#include "platform_common.h"

//Accel setup define  
//量程   
typedef enum 
{
	ACCEL_SCALERANGE_2G   			 = 0x03,
	ACCEL_SCALERANGE_4G   			 = 0x05,
	ACCEL_SCALERANGE_8G   			 = 0x08,
	ACCEL_SCALERANGE_16G  		 	 = 0x0C,
}bsp_accelerate_scalerange;

//采样率
typedef enum 
{
	ACCEL_SAMPLERATE_NONE             = 0x00,
	ACCEL_SAMPLERATE_0_78HZ           = 0x01,
	ACCEL_SAMPLERATE_1_56HZ           = 0x02,
	ACCEL_SAMPLERATE_3_12HZ           = 0x03,
	ACCEL_SAMPLERATE_6_25HZ           = 0x04,
	ACCEL_SAMPLERATE_12_5HZ           = 0x05,
	ACCEL_SAMPLERATE_25HZ             = 0x06,
	ACCEL_SAMPLERATE_50HZ          	  = 0x07,
	ACCEL_SAMPLERATE_100HZ            = 0x08,
	ACCEL_SAMPLERATE_200HZ            = 0x09,
	ACCEL_SAMPLERATE_400HZ            = 0x0A,
	ACCEL_SAMPLERATE_800HZ            = 0x0B,
	ACCEL_SAMPLERATE_1600HZ           = 0x0C,
}bsp_accelerate_osr;


//**********************************************************************
// 函数功能:  重力传感器初始化
// 输入参数： 无
// 返回参数：    
// 0x00    :  初始化成功
// 0xff    :  初始化失败
//**********************************************************************
//uint8 Drv_Accel_Open(void);

//**********************************************************************
// 函数功能:    硬件关闭
// 输入参数：    无
// 返回参数：    
// 0x00    :    设置成功
// 0xff    :    设置失败
//**********************************************************************
//uint8 Drv_Accel_Close(void);

//**********************************************************************
// 函数功能:    重力传感器的采样率和量程设置
// 输入参数：    
// sampleRate    采样率，默认ACCEL_SAMPLERATE_31_25HZ
//               Valid Values for uint16_t sampleRate are:
//               ACCEL_SAMPLERATE_0_5HZ   ACCEL_SAMPLERATE_1HZ
//               ACCEL_SAMPLERATE_2HZ     ACCEL_SAMPLERATE_31_25HZ
//               ACCEL_SAMPLERATE_62_5HZ  ACCEL_SAMPLERATE_250HZ
//               ACCEL_SAMPLERATE_500HZ      
// scaleRange    测量量程设置，默认ACCEL_SCALERANGE_2G
//               Accel fullscale selection : ?à2g (00), ?à4g (01), ?à8g (10), ?à16g (11)
//               Valid Values for uint8 scaleRange are:
//               ACCEL_SCALERANGE_2G   ACCEL_SCALERANGE_4G
//               ACCEL_SCALERANGE_8G   ACCEL_SCALERANGE_16G
// 返回参数：    
// 0x00    :    初始化成功
// 0xff    :    初始化失败
//**********************************************************************
uint8 Drv_Accel_Set(uint16 sampleRate, uint8 scaleRange);

//**********************************************************************
// 函数功能:    重力传感器FIFO功能开关设置
// 输入参数：    
// setState ：   FIFO开关，0x00关闭，0x01使能
// 返回参数：    
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
uint8  Drv_Accel_SetBuffer(uint16 setState);

//**********************************************************************
// 函数功能:    读取重力传感器3轴数据，数据为二进补码形式
// 输入参数：    
// axisData ：   三轴数据指针
// 返回参数：    
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
uint8 Drv_Accel_Read(int16 *axisData);

//**********************************************************************
// 函数功能:    从重力传感器FIFO读取3轴数据
// 输入参数：    
// CacheArry ：          三轴数据指针
// CacheMaxWriteLenght:  读取的数据长度
//FIFOReadNumber：       FIFO有效数据长度
// 返回参数：    
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
uint8 Drv_Accel_BufferRead(int16 *CacheArry, uint16 CacheMaxWriteLenght, uint16 *FIFOReadNumber);

//**********************************************************************
// 函数功能:    重力传感器唤醒
// 输入参数：   无
// 返回参数：    
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
uint8 Drv_Accel_WakeUp(void);

//**********************************************************************
// 函数功能:    设置重力传感器进入睡眠状态
// 输入参数：   无
// 返回参数：    
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
uint8 Drv_Accel_GoSleep(void);

//**********************************************************************
// 函数功能:   重力传感器复位
// 输入参数：   无
// 返回参数：    
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
uint8 Drv_Accel_Reset(void);

//**********************************************************************
// 函数功能:   重力传感器自检测
// 输入参数：   无
// 返回参数：    
// 0x00    :    自检成功
// 0xff    :    自检失败
//**********************************************************************
uint8 Drv_Accel_SelfTest(void);

#endif

