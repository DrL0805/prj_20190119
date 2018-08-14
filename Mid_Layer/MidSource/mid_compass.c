#include "platform_common.h"
/* FreeRTOS includes */
#include "rtos.h"
#include "mid_magnetism.h"
#include "multimodule_task.h"
#include "algorithm_mag_degree.h"
#include "mid_compass.h"

enum
{
	COMPASS_IDLE,
	COMPASS_UPDATA,
	COMPASS_CALIBRATION,
};

static int8   biasAngle = 0;
static uint16	compassState;		//	指南针状态
static uint16 compassData;		//	指南针角度值，最大360度


static uint16 compassMagReadId;



// 函数功能:	初始化指南针，配置参数
// 输入参数：	无
// 返回参数：	无
void Mid_Compass_Init(void)
{
	biasAngle				= 0;
	compassMagReadId		= 0xffff;

	compassState			= COMPASS_IDLE;

}


// 函数功能:	角度值回调，根据状态进行判断处理
// 输入参数：
// data:		地磁3轴原始数据X、Y、Z
// 返回参数：	无
void CompassDataProcess(int16 data[3])
{
	switch(compassState)
	{
		case COMPASS_CALIBRATION:
		Algorithm_Calculate_Mag_Degree(data, 1);
		break;

		case COMPASS_UPDATA:
		compassData = Algorithm_Calculate_Mag_Degree(data, 0);
		if(compassData > 360)
		{
			compassData = compassData % 360;
		}
		break;

		case COMPASS_IDLE:
		break;		
	}
}


// 函数功能:	读取指南针角度
// 输入参数：	无
// 返回参数：	无
uint16 Mid_Compass_AngleRead(void)
{
	return compassData;
}


// 函数功能:	读取指南针磁偏角校正后角度
// 输入参数：	无
// 返回参数：	无
uint16 Mid_Compass_CompensatoryAngleRead(void)
{
	return (compassData + biasAngle);
}


// 函数功能:	设置指南针磁偏角
// 输入参数：	无
// 返回参数：	0x00: success
// 				0xff: fail
uint16 Mid_Compass_BiasSet(int8  dataTemp)
{
	if(dataTemp >= -90 && dataTemp <= 90)
	{
		biasAngle = dataTemp;
		return 0;
	}
	return 0xff;
}


// 函数功能:	读取指南针磁偏角
// 输入参数：	无
// 返回参数：	磁偏角设置值
int8 Mid_Compass_BiasRead(void)
{
	return biasAngle;
}


// 函数功能:	开启指南针角度更新
// 输入参数：	无
// 返回参数：	0x00: success
// 				0xff: fail
static uint16 CompassPointStart(void)
{
	multimodule_task_msg_t	msg;	
	switch(compassState)
	{
		case COMPASS_CALIBRATION:
		msg.id		= MAG_ID;
		msg.module.magEvent.id			= MAG_READ_DELETE;
		msg.module.magEvent.readId		= &compassMagReadId;

		MultiModuleTask_EventSet(msg);
		case COMPASS_IDLE:
		compassState	= COMPASS_UPDATA;
		compassData		= 0;
		msg.id			= MAG_ID;


		msg.module.magEvent.id			= MAG_READ_SET;
		msg.module.magEvent.readId		= &compassMagReadId;
		msg.module.magEvent.rate		= MAG_10HZ;
		msg.module.magEvent.scaleRange	= MAG_20GS;
		msg.module.magEvent.Cb			= CompassDataProcess;
		MultiModuleTask_EventSet(msg);
		// **********************************************
		case COMPASS_UPDATA:
		break;

		default:
		return 0xff;
	}
	return 0x00;
	
}

// 函数功能:	关闭指南针角度更新
// 输入参数：	无
// 返回参数：	0x00: success
// 				0xff: fail
static uint16 CompassPointStop(void)
{
	multimodule_task_msg_t	msg;	
	switch(compassState)
	{
		case COMPASS_UPDATA:
		msg.id		= MAG_ID;
		msg.module.magEvent.id			= MAG_READ_DELETE;
		msg.module.magEvent.readId		= &compassMagReadId;

		MultiModuleTask_EventSet(msg);
		case COMPASS_CALIBRATION:
		case COMPASS_IDLE:
		break;

		default:
		return 0xff;
	}
	compassState		= COMPASS_IDLE;
	compassData	= 0;
	return 0x00;
}


// 函数功能:	开启指南针校准
// 输入参数：	无
// 返回参数：	0x00: success
// 				0xff: fail
static uint16 CompassCalibrationStart(void)
{
	multimodule_task_msg_t	msg;	
	switch(compassState)
	{
		case COMPASS_UPDATA:
		msg.id		= MAG_ID;
		msg.module.magEvent.id			= MAG_READ_DELETE;
		msg.module.magEvent.readId		= &compassMagReadId;

		MultiModuleTask_EventSet(msg);
		case COMPASS_IDLE:
		compassState	= COMPASS_CALIBRATION;
		compassData		= 0;
		msg.id			= MAG_ID;


		msg.module.magEvent.id			= MAG_READ_SET;
		msg.module.magEvent.readId		= &compassMagReadId;
		msg.module.magEvent.rate		= MAG_50HZ;
		msg.module.magEvent.scaleRange	= MAG_20GS;
		msg.module.magEvent.Cb			= CompassDataProcess;
		MultiModuleTask_EventSet(msg);
		// **********************************************
		case COMPASS_CALIBRATION:
		break;

		default:
		return 0xff;
	}
	return 0x00;
}


// 函数功能:	关闭指南针校准
// 输入参数：	无
// 返回参数：	0x00: success
// 				0xff: fail
static uint16 CompassCalibrationStop(void)
{
	multimodule_task_msg_t	msg;	
	switch(compassState)
	{
		case COMPASS_CALIBRATION:
		msg.id		= MAG_ID;
		msg.module.magEvent.id			= MAG_READ_DELETE;
		msg.module.magEvent.readId		= &compassMagReadId;

		MultiModuleTask_EventSet(msg);
		case COMPASS_UPDATA:
		case COMPASS_IDLE:
		break;

		default:
		return 0xff;
	}
	compassState		= COMPASS_IDLE;
	compassData	= 0;
	return 0x00;
}



// 函数功能:	指南针事件处理
// 输入参数：	无
// 返回参数：	0x00: success
// 				0xff: fail
uint16 Mid_Compass_EventProcess(compass_event_s *msg)
{
	switch(msg->id)
	{
		case COMPASS_CALI_START:
		CompassCalibrationStart();
		break;

		case COMPASS_CALI_STOP:
		CompassCalibrationStop();
		break;

		case COMPASS_UPDATA_START:
		CompassPointStart();
		break;

		case COMPASS_UPDATA_STOP:
		CompassPointStop();
		break;

		default:
		return 0xff;
	}
	return 0x00;
}
