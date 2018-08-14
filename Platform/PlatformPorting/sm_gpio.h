#ifndef __SM_GPIO_H
#define __SM_GPIO_H

#include "am_mcu_apollo.h"
#include "platform_common.h"

#ifdef AM_PART_APOLLO3
#define GPIO_PIN_DISABLE  0x00   //禁止，低功耗下
#define GPIO_PIN_INPUT    0x01     //输入
#define GPIO_PIN_OUTPUT   0x02    //输出
#define GPIO_PIN_OD       0x04 //开漏
#define GPIO_PIN_3STATE   0x08    //高组态

#else
//设置属性1和2，可以作或组合，如GPIO_PIN_INPUT|AM_HAL_GPIO_PULL1_5K
//设置属性1
#define GPIO_PIN_DISABLE  AM_HAL_PIN_DISABLE   //禁止，低功耗下
#define GPIO_PIN_INPUT    AM_HAL_PIN_INPUT     //输入
#define GPIO_PIN_OUTPUT   AM_HAL_PIN_OUTPUT    //输出
#define GPIO_PIN_OD       AM_HAL_PIN_OPENDRAIN //开漏
#define GPIO_PIN_3STATE   AM_HAL_PIN_3STATE    //高组态

//设置属性2
#define GPIO_PIN_POWER    AM_HAL_GPIO_POWER
#define GPIO_PIN_HIGH_DRV AM_HAL_GPIO_HIGH_DRIVE
#define GPIO_PIN_LOW_DRV  AM_HAL_GPIO_LOW_DRIVE
#define GPIO_PIN_PULLUP   AM_HAL_GPIO_PULLUP     //上拉
#define GPIO_PIN_PULL1_5K AM_HAL_GPIO_PULL1_5K   //上拉
#define GPIO_PIN_PULL6K   AM_HAL_GPIO_PULL6K     //上拉6K
#define GPIO_PIN_PULL12K  AM_HAL_GPIO_PULL12K    //上拉12K
#define GPIO_PIN_PULL24K  AM_HAL_GPIO_PULL24K    //上拉24K
#endif

typedef void (*gpio_cb)(uint32 u32PinNum);

//**********************************************************************
// 函数功能:    GPIO初始化
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
extern void SMDrv_GPIO_Init(void);

//**********************************************************************
// 函数功能: 根据GPIO module ID配置GPIO属性:输入/输出，若为中断，则设置
//           则设置中断服务程序
// 输入参数：u32PinNum: gpio num, 值参考io_config.h中IO定义
//    config_opt:设置GPIO属性，若在程序中改变GPIO属性时使用，如用GPIO模拟
//      IIC时，SDA脚一会为输入，一会为输出。
//      == NULL: 使用默认的属性配置， != NULL: 则使用传入的属性,值参考GPIO_PIN_xxx
//    g_callback:中断服务函数callback
// 返回参数: Ret_OK，Ret_InvalidParam
//**********************************************************************
extern ret_type SMDrv_GPIO_Open(uint32 u32PinNum,uint32 *config_opt,gpio_cb g_callback);

//**********************************************************************
// 函数功能:  重新设置中断触发条件
// 输入参数： u32PinNum: gpio num,
//   u32pol:  =1: 上升沿，=0: 下降沿
// 返回参数： 无
//**********************************************************************
extern void SMDrv_GPIO_ReConfigIrq(uint32 u32PinNum,uint32 u32pol);

//**********************************************************************
// 函数功能:  开启/关闭pin脚中断
// 输入参数： u32PinNum: gpio num,
// u8Enable:  =1: enable中断，=0: disable中断
// 返回参数： 无
//**********************************************************************
extern void SMDrv_GPIO_EnableInterrupt(uint32 u32PinNum,uint8 u8Enable);

//**********************************************************************
// 函数功能:  设置GPIO中断优先级
// 输入参数： prio:中断优先级，bit0~2位值有效
// 返回参数： 无
//**********************************************************************
extern void SMDrv_GPIO_SetIrqPrio(uint32 prio);

//**********************************************************************
// 函数功能:  设置GPIO为其他模式，如uart，iic，spi，pdm等
// 输入参数： u32pin:   gpio num, 
//            u32config: gpio pad
// 返回参数： 无
//**********************************************************************
extern void SMDrv_GPIO_SetIOPad(uint32 u32pin,uint32 u32config);

//**********************************************************************
// 函数功能:  将GPIO disable掉，以实现低功耗
// 输入参数： u32PinNum: gpio num, 值参考io_config.h中IO定义
// 返回参数： Ret_InvalidParam,Ret_OK
//**********************************************************************
extern ret_type SMDrv_GPIO_Close(uint32 u32PinNum);

//**********************************************************************
// 函数功能:  设置pin脚输出高电平
// 输入参数： u32PinNum: gpio num, 值参考io_config.h中IO定义
// 返回参数： Ret_InvalidParam,Ret_OK
//**********************************************************************
extern ret_type SMDrv_GPIO_BitSet(uint32 u32PinNum);

//**********************************************************************
// 函数功能:  设置pin脚输出低电平
// 输入参数： u32PinNum: gpio num, 值参考io_config.h中IO定义
// 返回参数： Ret_InvalidParam,Ret_OK
//**********************************************************************
extern ret_type SMDrv_GPIO_BitClear(uint32 u32PinNum);

//**********************************************************************
// 函数功能:  将pin脚输出电平反转
// 输入参数： u32PinNum: gpio num, 值参考io_config.h中IO定义
// 返回参数： Ret_InvalidParam,Ret_OK
//**********************************************************************
extern ret_type SMDrv_GPIO_BitToggle(uint32 u32PinNum);

//**********************************************************************
// 函数功能:  从输入读取寄存器中获取pin脚电平值
// 输入参数： u32PinNum: gpio num, 值参考io_config.h中IO定义
// 返回参数： 0xFFFFFFFF:参数错误，其他值为读取到的值
//**********************************************************************
extern uint32 SMDrv_GPIO_InBitRead(uint32 u32PinNum);

//**********************************************************************
// 函数功能:  从数据输出寄存器中获取pin脚电平值
// 输入参数： u32PinNum: gpio num, 值参考io_config.h中IO定义
// 返回参数： 0xFFFFFFFF:参数错误，其他值为读取到的值
//**********************************************************************
extern uint32 SMDrv_GPIO_OutBitRead(uint32 u32PinNum);

#endif
