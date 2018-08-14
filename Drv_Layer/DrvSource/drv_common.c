/**********************************************************************
**
**模块说明: 通用模块驱动接口，主要包括马达，LED控制
**          马达或LED状态on/off取决于硬件设置，需要根据具体情况作调整
**          此版马达:高电平驱动，LED:低电平驱动
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.19  修改流程  ZSL  
**
**
**********************************************************************/
#define COMMON_MODULE
#include "io_config.h"
#include "sm_sys.h"
#include "sm_gpio.h"
#include "drv_common.h"

//**********************************************************************
// 函数功能: 初始化马达，绿灯，红灯和背光口
// 输入参数：	
// 返回参数：		
// 0x00   :    设置成功
//**********************************************************************
uint8 Drv_Common_Init(void)
{
    uint32 u32PinConfig = GPIO_PIN_OUTPUT;  //default set pin as output

    //配置motor马达IO口，默认输出低
#if(MOTOR_PIN != IO_UNKNOW)
    SMDrv_GPIO_BitClear(MOTOR_PIN);
    SMDrv_GPIO_Open(MOTOR_PIN,&u32PinConfig,NULL);
#endif
    
    //配置Green Led绿灯IO口，默认输出高
#if(LED_G_PIN != IO_UNKNOW)
    SMDrv_GPIO_BitSet(LED_G_PIN);
    SMDrv_GPIO_Open(LED_G_PIN,&u32PinConfig,NULL);
#endif
    
    //配置Red Led红灯IO口，默认输出高
#if(LED_R_PIN != IO_UNKNOW)
    SMDrv_GPIO_BitSet(LED_R_PIN);
    SMDrv_GPIO_Open(LED_R_PIN,&u32PinConfig,NULL);
#endif
    
    //配置背光IO口，默认输出高
#if(BCKLED_PIN != IO_UNKNOW)
    SMDrv_GPIO_BitSet(BCKLED_PIN);
    SMDrv_GPIO_Open(BCKLED_PIN,&u32PinConfig,NULL);
#endif
	return Ret_OK;
}

//**********************************************************************
// 函数功能: 开启马达振动
// 输入参数：无
// 返回参数：
// 0x00    : 设置成功
// 0x02    : 参数非法
// 0x05    : 设置不存在
//**********************************************************************
uint8 Drv_Common_MotoOn(void)
{
#if(MOTOR_PIN != IO_UNKNOW)
	return SMDrv_GPIO_BitSet(MOTOR_PIN);
#else
    return Ret_NoDevice;
#endif
}

//**********************************************************************
// 函数功能: 开启马达振动
// 输入参数：无
// 返回参数：
// 0x00    : 设置成功
// 0x02    : 参数非法
// 0x05    : 设置不存在
//**********************************************************************
uint8 Drv_Common_MotoOff(void)
{
#if(MOTOR_PIN != IO_UNKNOW)
	return SMDrv_GPIO_BitClear(MOTOR_PIN);
#else
    return Ret_NoDevice;
#endif
}

//**********************************************************************
// 函数功能: 点亮红灯
// 输入参数：无
// 返回参数：
// 0x00    : 设置成功
// 0x02    : 参数非法
// 0x05    : 设置不存在
//**********************************************************************
uint8 Drv_Common_RedLedOn(void)
{
#if(LED_R_PIN != IO_UNKNOW)
    return SMDrv_GPIO_BitClear(LED_R_PIN);;
#else
    return Ret_NoDevice;
#endif
}

//**********************************************************************
// 函数功能: 熄灭红灯
// 输入参数：无
// 返回参数：
// 0x00    : 设置成功
// 0x02    : 参数非法
// 0x05    : 设置不存在
//**********************************************************************
uint8 Drv_Common_RedLedOff(void)
{
#if(LED_R_PIN != IO_UNKNOW)
    return SMDrv_GPIO_BitSet(LED_R_PIN);
#else
    return Ret_NoDevice;
#endif
}

//**********************************************************************
// 函数功能: 点亮绿灯
// 输入参数：无
// 返回参数：
// 0x00    : 设置成功
// 0x02    : 参数非法
// 0x05    : 设置不存在
//**********************************************************************
uint8 Drv_Common_GreenLedOn(void)
{
#if(LED_G_PIN != IO_UNKNOW)
    return SMDrv_GPIO_BitClear(LED_G_PIN);
#else
    return Ret_NoDevice;
#endif
}

//**********************************************************************
// 函数功能: 熄灭绿灯
// 输入参数：无
// 返回参数：
// 0x00    : 设置成功
// 0x02    : 参数非法
// 0x05    : 设置不存在
//**********************************************************************
uint8 Drv_Common_GreenLedOff(void)
{
#if(LED_G_PIN != IO_UNKNOW)
    return SMDrv_GPIO_BitSet(LED_G_PIN);
#else
    return Ret_NoDevice;
#endif
}

//**********************************************************************
// 函数功能: 点亮背光灯灯
// 输入参数：无
// 返回参数：
// 0x00    : 设置成功
// 0x02    : 参数非法
// 0x05    : 设置不存在
//**********************************************************************
uint8 Drv_Common_BackLightOn(void)
{
#if(BCKLED_PIN != IO_UNKNOW)
    return SMDrv_GPIO_BitClear(BCKLED_PIN);
#else
    return Ret_NoDevice;
#endif
}

//**********************************************************************
// 函数功能: 熄灭背光灯灯
// 输入参数：无
// 返回参数：
// 0x00    : 设置成功
// 0x02    : 参数非法
// 0x05    : 设置不存在
//**********************************************************************
uint8 Drv_Common_BackLightOff(void)
{
#if(BCKLED_PIN != IO_UNKNOW)
    return SMDrv_GPIO_BitSet(BCKLED_PIN);
#else
    return Ret_NoDevice;
#endif
}


uint8 Drv_Common_SysternReset(void)
{
	SMDrv_SYS_DisableMasterIsr();
    SMDrv_SYS_PowerOnReset();  
	return Ret_OK;
}
