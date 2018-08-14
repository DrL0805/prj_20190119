#ifndef	MID_GYROSCOPE_H
#define	MID_GYROSCOPE_H

#include "platform_common.h"

// Setting Options
typedef enum {
	GYRO_1HZ   = 1,
	GYRO_2HZ   = 2,
	GYRO_25HZ  = 25,
	GYRO_50HZ  = 50,
	GYRO_100HZ = 100,
	GYRO_200HZ = 200,
}gyro_freq_option_app;


// Setting Options
typedef enum {
	GYRO_250S 	= 1,
	GYRO_500S	= 2,
	GYRO_1000S	= 3,
	GYRO_2000S	= 4,
}gyro_scale_option_app;

// 事件定义
typedef enum
{
	GYRO_SLEEP,
	GYRO_HARDWARE_SET,
	GYRO_READ_PROCESS,
	GYRO_READ_SET,
	GYRO_READ_DELETE,
}GYRO_INTERFACE_E;

// 事件定义
typedef struct 
{
	uint16			id;							// 事件ID
	uint16			*readId;					// 读取事件设置与删除的ID地址
	uint16			rate;						// 硬件采样频率或读取频率
	uint8			scaleRange;					// 设置采样范围
	void			(*Cb)(int16 data[3]);		// 数据读取回调函数
}gyro_event_s;

//**********************************************************************
// 函数功能：	初始化硬件传感器，并设置可定时读取的回调函数
// 输入参数：	IsrCb: 传感器达到可设置的读取时间回调函数
// 返回参数：	无
//**********************************************************************
extern void Mid_Gyro_Init(void (*IsrCb)(void));


//**********************************************************************
// 函数功能：	读取当前加速度的最新值
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
extern void Mid_Gyro_DataRead(int16 data[3]);

//**********************************************************************
// 函数功能：	读取当前硬件的采样频率与采样范围
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
extern void Mid_Gyro_SettingRead(uint16 *sampleRate, uint8 *scaleRange);

//**********************************************************************
// 函数功能：	加速度事件处理
// 输入参数：	无
// 返回参数：	0x00:成功
// 				0xff:失败
//**********************************************************************
extern uint16 Mid_Gyro_EventProcess(gyro_event_s* msg);

//**********************************************************************
// 函数功能：	传感器自检,调用该函数时，确保该资源未使用
// 输入参数：	无
// 返回参数：	0x00:成功
// 				0xff:失败
//**********************************************************************
extern uint16 Mid_Gyro_SelfTest(void);

#endif		// GYROSCOPE_APP_H
