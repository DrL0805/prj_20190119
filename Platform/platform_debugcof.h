#ifndef PLATFORM_DEBUG_H
#define PLATFORM_DEBUG_H

/****************************************************************************************
**                        调试信息开关说明
** 软件增加debug信息开关， 如下所有的宏:
**  PLATFORM_DEBUG_ENABLE: 设置为 1: debug 版本，增加打印，便于调试；
**                         设置为 0: relelase版本，释放版本，必须将打印全关掉 
**
** 在debug版本中: 每一个模块会定义一个宏来控制是否需要开此模块中的打印，如
** BLE_STACK_DEBUG_ENABLE: 用来控制蓝牙协议模块打印信息，设为1:开启，为0则关掉
** 其中Err_Info:打印error信息
**
** 在PLATFORM_DEBUG_ENABLE == 0的部分，必须将全部宏设为0
**   PLATFORM_DEBUG_ENABLE == 1的部分，为debug，可根据情况设1或设0
**
** 目的:在出现问题时帮助定位问题，尤其在数据通讯部分
** 打印使用:RTT
**
****************************************************************************************/
#define PLATFORM_DEBUG_ENABLE   1  

#if(PLATFORM_DEBUG_ENABLE == 1)   //debug version
#include "SEGGER_RTT.h"
#include "SEGGER_RTT_Conf.h"

//SMDrv 模块debug 开关
#define SMDRV_FLASH_DEBUG        0
#define SMDRV_ADC_DEBUG          0

//DRV 模块debug 开关

//MID层debug 开关 
#define MID_BLE_DEBUG            1    //蓝牙
#define MID_BAT_DEBUG            0    //bat
#define MID_MAG_DEBUG            0    //mag
#define MID_PRESS_DEBUG          0    //pressure
#define MID_SPORT_DEBUG          0    //sport


//APP层debug 开关
#define APP_PROTOCAL_DEBUG       0    //协议解析

//BLE STACK debug 开关
#define BLE_STACK_DEBUG_ENABLE   1  
#define WSF_DEBUG_ENABLE         0  //蓝牙协议栈 trace

#if(WSF_DEBUG_ENABLE == 1)
#define WSF_TRACE_ENABLED        1  //此处不要修改，只需修改WSF_DEBUG_ENABLE
#define AM_DEBUG_PRINTF
#endif
#define SMOTAS_DEBUG_ENABLE      1 //OTA
#define BLE_PASSKEY_DEBUG        1

//for Error info print
#define Err_Info(x) SEGGER_RTT_printf x


#else           //release version

#define SMDRV_FLASH_DEBUG        0

#define MID_BLE_DEBUG            0
#define MID_BAT_DEBUG            0
#define MID_MAG_DEBUG            0
#define MID_PRESS_DEBUG          0 

#define APP_PROTOCAL_DEBUG       0


#define BLE_STACK_DEBUG_ENABLE   0
#define WSF_DEBUG_ENABLE         0
#define SMOTAS_DEBUG_ENABLE      0
#define BLE_PASSKEY_DEBUG        0

#define Err_Info(x) 
#endif

#define PACK_DATA 				0
#define SCENE_DATA 				0

#endif

