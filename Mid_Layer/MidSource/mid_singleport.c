/**********************************************************************
**
**模块说明: mid层common接口
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.25  修改流程  ZSL  
**
**********************************************************************/
#include "drv_common.h"
#include "mid_singleport.h"

// typedef 
//单口任务参数结构体
typedef struct 
{
    uint16 lowCountTime;
    uint16 highCountTime;
    uint16 lowTime;
    uint16 highTime;
    uint16 driveTimes;
    uint8  enable;
    uint8  state;
    uint8  portType;
}single_port_para_s;

//单口驱动结构体
typedef struct 
{
    uint8   (*PortOn)(void);
    uint8   (*PortOff)(void);
    uint8    portType;
    single_port_para_s  *drivePara;
}single_port_drive_s;

single_port_para_s   singlePortPara[SINGLE_PORT_NUM];
single_port_drive_s  singlePortDrive[SINGLE_PORT_NUM];

static SinglePortFinishCb SPortFinish_Cb = NULL;

//**********************************************************************
//单口任务实现函数
//输入参数：为单口任务实现函数。
//输出：无
//**********************************************************************
static void SinglePortDriveApp(single_port_drive_s *driveport)
{
	if(driveport->drivePara->enable)
	{
		if(driveport->drivePara->lowCountTime == 0)
		{
			driveport->drivePara->state = 1;
			if((driveport->drivePara->highCountTime) == 0)
			{
				driveport->drivePara->state = 0;
				driveport->drivePara->driveTimes--;
				if((driveport->drivePara->driveTimes) == 0)
				{
					driveport->drivePara->enable = 0;
					(SPortFinish_Cb)(driveport->portType);
				}
				else
				{
					if(driveport->drivePara->lowTime)
						driveport->drivePara->lowCountTime = driveport->drivePara->lowTime - 1;
					else
						driveport->drivePara->state = 1;
					driveport->drivePara->highCountTime = driveport->drivePara->highTime;
				}
			}
			else
			{
				driveport->drivePara->highCountTime--;
			}
		}
		else
		{
			driveport->drivePara->lowCountTime--;
		}
		if(driveport->drivePara->state)
			driveport->PortOn();
		else
			driveport->PortOff();
	}
}

//**********************************************************************
//设置单口驱动参数
//输入参数：设置单口参数结构体
//输出：无
//**********************************************************************
void Mid_SinglePort_SetPara(single_port_set_para_s *setPara)
{
    single_port_para_s tempPort;

    tempPort.enable             = 1;
    tempPort.state              = 0;
    tempPort.lowTime            = setPara->lowTime;
    tempPort.highTime           = setPara->highTime;
    tempPort.driveTimes         = setPara->driveTimes;
    tempPort.lowCountTime       = setPara->lowTime;
    tempPort.highCountTime      = setPara->highTime;
    tempPort.portType           = setPara->portType;

    if(tempPort.driveTimes == 0)
    {
        tempPort.driveTimes     = 1;
    }
    singlePortPara[tempPort.portType]   = tempPort;
}

//**********************************************************************
//把单口驱动初始化
//输入参数：设置单口参数结构体
//输出：无
//**********************************************************************
void Mid_SinglePort_Init(void)
{
	Drv_Common_Init();

	singlePortDrive[RED_LED].portType		= RED_LED;
	singlePortDrive[RED_LED].PortOn			= Drv_Common_RedLedOn;
	singlePortDrive[RED_LED].PortOff		= Drv_Common_RedLedOff;
	singlePortDrive[RED_LED].drivePara		= &singlePortPara[RED_LED];

	singlePortDrive[GREEN_LED].portType		= GREEN_LED;
	singlePortDrive[GREEN_LED].PortOn		= Drv_Common_GreenLedOn;
	singlePortDrive[GREEN_LED].PortOff		= Drv_Common_GreenLedOff;
	singlePortDrive[GREEN_LED].drivePara	= &singlePortPara[GREEN_LED];

	singlePortDrive[MOTO].portType			= MOTO;
	singlePortDrive[MOTO].PortOn			= Drv_Common_MotoOn;
	singlePortDrive[MOTO].PortOff			= Drv_Common_MotoOff;
	singlePortDrive[MOTO].drivePara			= &singlePortPara[MOTO];

	singlePortDrive[BACKLIGHT].portType		= BACKLIGHT;
	singlePortDrive[BACKLIGHT].PortOn		= Drv_Common_BackLightOn;
	singlePortDrive[BACKLIGHT].PortOff		= Drv_Common_BackLightOff;
	singlePortDrive[BACKLIGHT].drivePara	= &singlePortPara[BACKLIGHT];
}

//**********************************************************************
//函数功能：单口驱动完成回调初始化
//输入参数：设置单口参数结构体
//输出：无
//**********************************************************************
void Mid_SinglePort_SetCallBack(SinglePortFinishCb Cb)
{
    if(Cb == NULL)
        return;
    SPortFinish_Cb = Cb;
}

//**********************************************************************
//函数功能：获取单口状态
//输入参数：无
//输出：无
//**********************************************************************
uint16 Mid_SinglePort_ReadStatus(void)
{
    if((singlePortDrive[MOTO].drivePara->enable == 0) &&
        (singlePortDrive[GREEN_LED].drivePara->enable == 0) &&
        (singlePortDrive[RED_LED].drivePara->enable == 0))
    {
        return 0;
    }
    return 1;
}

//**********************************************************************
//函数功能：MOTO周期性处理
//输入参数：无
//输出：无
//**********************************************************************
void Mid_SinglePort_PeriodProcessMoto(void)
{
	SinglePortDriveApp(&singlePortDrive[MOTO]);
}

//**********************************************************************
//函数功能：GREENLED周期性处理
//输入参数：无
//输出：无
//**********************************************************************
void Mid_SinglePort_PeriodProcessGreenLed(void)
{
	SinglePortDriveApp(&singlePortDrive[GREEN_LED]);
}

//**********************************************************************
//函数功能：REDLED周期性处理
//输入参数：无
//输出：无
//**********************************************************************
void Mid_SinglePort_PeriodProcessRedLed(void)
{
	SinglePortDriveApp(&singlePortDrive[RED_LED]);
}

//**********************************************************************
//函数功能：背光周期性处理
//输入参数：无
//输出：无
//**********************************************************************
void Mid_SinglePort_PeriodProcessBackLight(void)
{
	SinglePortDriveApp(&singlePortDrive[BACKLIGHT]);
}

//**********************************************************************
//函数功能：单口ON
//输入参数：无
//输出：无
//**********************************************************************
void Mid_SinglePort_On(uint16 portType)
{
    switch(portType)
    {
    case RED_LED:
        Drv_Common_RedLedOn();
		break;
    case GREEN_LED:
		Drv_Common_GreenLedOn();
		break;
    case MOTO:
		Drv_Common_MotoOn();
		break;
    case BACKLIGHT:
		Drv_Common_BackLightOn();
		break;
	}
}

//**********************************************************************
//函数功能：单口OFF
//输入参数：无
//输出：无
//**********************************************************************
void Mid_SinglePort_Off(uint16 portType)
{
    switch(portType)
    {
    case RED_LED:
        Drv_Common_RedLedOff();
		break;
    case GREEN_LED:
		Drv_Common_GreenLedOff();
		break;
    case MOTO:
		Drv_Common_MotoOff();
		break;
    case BACKLIGHT:
		Drv_Common_BackLightOff();
		break;
    }
}

