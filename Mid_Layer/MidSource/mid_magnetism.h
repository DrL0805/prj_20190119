#ifndef MID_MAGNETISM_H
#define MID_MAGNETISM_H

#include "platform_common.h"

// Setting Options
typedef enum {
	MAG_10HZ   = 10,
	MAG_50HZ   = 50,
	MAG_100HZ  = 100,
	MAG_200HZ  = 200,
}mag_freq_option_app;

// Setting Options
typedef enum 
{
	MAG_2GS 	= 2,
	MAG_8GS 	= 8,
	MAG_12GS 	= 12,
	MAG_20GS 	= 20,
}mag_scale_option_app;

// 事件定义
typedef enum
{
	MAG_SLEEP,
	MAG_HARDWARE_SET,
	MAG_READ_PROCESS,
	MAG_READ_SET,
	MAG_READ_DELETE,
}MAG_INTERFACE_E;

// 事件定义
typedef struct 
{
	uint16			id;							// 事件ID
	uint16			*readId;					// 读取事件设置与删除的ID地址
	uint16			rate;						// 硬件采样频率或读取频率
	uint8			scaleRange;					// 设置采样范围
	void			(*Cb)(int16 data[3]);		// 数据读取回调函数
}mag_event_s;

//**********************************************************************
// 函数功能：	初始化硬件传感器，并设置可定时读取的回调函数
// 输入参数：	IsrCb: 传感器达到可设置的读取时间回调函数
// 返回参数：	无
//**********************************************************************
extern void Mid_Magnetism_Init(void (*IsrCb)(void));

//**********************************************************************
// 函数功能：	读取当前地磁的最新值
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
extern void Mid_Magnetism_DataRead(int16 data[3]);

//**********************************************************************
// 函数功能：	读取当前硬件的采样频率与采样范围
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
extern void Mid_Magnetism_SettingRead(uint16 *sampleRate, uint8 *scaleRange);

//**********************************************************************
// 函数功能：	地磁事件处理
// 输入参数：	无
// 返回参数：	0x00:成功
// 				0xff:失败
//**********************************************************************
extern uint16 Mid_Magnetism_EventProcess(mag_event_s* msg);

//**********************************************************************
// 函数功能：	传感器自检,调用该函数时，确保该资源未使用
// 输入参数：	无
// 返回参数：	0x00:成功
// 				0xff:失败
//**********************************************************************
extern uint16 Mid_Magnetism_SelfTest(void);

#endif		// MAGNETISM_APP_H

