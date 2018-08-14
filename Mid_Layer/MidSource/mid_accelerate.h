#ifndef	MID_ACCELERATE_H
#define	MID_ACCELERATE_H

#include "platform_common.h"

// Setting Options
typedef enum {
	ACCEL_1HZ   = 1,
	ACCEL_2HZ   = 2,
	ACCEL_25HZ  = 25,
	ACCEL_50HZ  = 50,
	ACCEL_100HZ = 100,
	ACCEL_200HZ = 200,
}accel_freq_option_app;


// Setting Options
typedef enum {
	ACCEL_2G 	= 0,
	ACCEL_4G	= 1,
	ACCEL_8G	= 2,
	ACCEL_16G	= 3,
}accel_scale_option_app;


// 事件定义
typedef enum
{
	ACCEL_SLEEP,
	ACCEL_HARDWARE_SET,
	ACCEL_READ_PROCESS,
	ACCEL_READ_SET,
	ACCEL_READ_DELETE,
}ACCELERATE_INTERFACE_E;

// 事件定义
typedef struct 
{
	uint16			id;							// 事件ID
	uint16			*readId;					// 读取事件设置与删除的ID地址
	uint16			rate;						// 硬件采样频率或读取频率
	uint8			scaleRange;					// 设置采样范围
	void			(*Cb)(int16 data[3]);		// 数据读取回调函数
}accel_event_s;

//**********************************************************************
// 函数功能：	初始化硬件传感器，并设置可定时读取的回调函数
// 输入参数：	IsrCb: 传感器达到可设置的读取时间回调函数
// 返回参数：	无
//**********************************************************************
extern void Mid_Accel_Init(comm_cb *Read_IsrCb);

//**********************************************************************
// 函数功能：	读取当前硬件的采样频率与采样范围
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
extern void Mid_Accel_SettingRead(uint16 *sampleRate, uint8 *scaleRange);

//**********************************************************************
// 函数功能：	读取当前加速度的最新值
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
extern void Mid_Accel_DataRead(int16 data[3]);

//**********************************************************************
// 函数功能：	加速度事件处理
// 输入参数：	msg	事件指针
// 返回参数：	0x00: 操作成功
//				0xff: 操作失败
//**********************************************************************
extern uint16 Mid_Accel_EventProcess(accel_event_s* msg);

//**********************************************************************
// 函数功能：	传感器自检,调用该函数时，确保该资源未使用
// 输入参数：	无
// 返回参数：	0x00:成功
// 				0xff:失败
//**********************************************************************
extern uint16 Mid_Accel_SelfTest(void);

#endif		// ACCELERATE_APP_H
