#ifndef __SM_SYS_H
#define __SM_SYS_H

#include "platform_common.h"

//interrupt 定义同am_hal_interrupt.h
#ifdef AM_PART_APOLLO3
typedef enum
{
	// Hardware interrupts
	INTERRUPT_RESET              = 1,
	INTERRUPT_NMI                = 2,
	INTERRUPT_HARDFAULT          = 3,
	INTERRUPT_MPUFAULT           = 4,
	INTERRUPT_BUSFAULT           = 5,
	INTERRUPT_USAGEFAULT         = 6,
	INTERRUPT_SVCALL             = 11,
	INTERRUPT_DEBUGMON           = 12,
	INTERRUPT_PENDSV             = 14,
	INTERRUPT_SYSTICK            = 15,
	
    // Begin IRQs
	INTERRUPT_BROWNOUT           = 16,
	INTERRUPT_WATCHDOG           = 17,
	INTERRUPT_RTC                = 18,
	INTERRUPT_VCOMP              = 19,
	INTERRUPT_IOSLAVE            = 20,
	INTERRUPT_IOSACC             = 21,
	INTERRUPT_IOMASTER0          = 22,
	INTERRUPT_IOMASTER1          = 23,
	INTERRUPT_IOMASTER2          = 24,
	INTERRUPT_IOMASTER3          = 25,
	INTERRUPT_IOMASTER4          = 26,
	INTERRUPT_IOMASTER5          = 27,
	INTERRUPT_BLE                = 28,
	INTERRUPT_GPIO               = 29,
	INTERRUPT_CTIMER             = 30,
	INTERRUPT_UART0              = 31,
	INTERRUPT_UART1              = 32,
	INTERRUPT_SCARD              = 33,
	INTERRUPT_ADC                = 34,
	INTERRUPT_PDM                = 35,
	INTERRUPT_MSPI               = 36,
	INTERRUPT_RESERVED0          = 37,
	INTERRUPT_STIMER             = 38,
	INTERRUPT_STIMER_CMPR0       = 39,
	INTERRUPT_STIMER_CMPR1       = 40,
	INTERRUPT_STIMER_CMPR2       = 41,
	INTERRUPT_STIMER_CMPR3       = 42,
	INTERRUPT_STIMER_CMPR4       = 43,
	INTERRUPT_STIMER_CMPR5       = 44,
	INTERRUPT_STIMER_CMPR6       = 45,
	INTERRUPT_STIMER_CMPR7       = 46,
	INTERRUPT_CLKGEN             = 47,
	INTERRUPT_SOFTWARE0          = 48,
	INTERRUPT_SOFTWARE1          = 49,
	INTERRUPT_SOFTWARE2          = 50,
	INTERRUPT_SOFTWARE3          = 51,
}interrupt_handle;
#else
typedef enum
{
	// Hardware interrupts
	INTERRUPT_RESET              = 1,
	INTERRUPT_NMI                = 2,
	INTERRUPT_HARDFAULT          = 3,
	INTERRUPT_MPUFAULT           = 4,
	INTERRUPT_BUSFAULT           = 5,
	INTERRUPT_USAGEFAULT         = 6,

	INTERRUPT_SVCALL             = 11,
	INTERRUPT_DEBUGMON           = 12,
	INTERRUPT_PENDSV             = 14,
	INTERRUPT_SYSTICK            = 15,

    // Begin IRQs
	INTERRUPT_BROWNOUT           = 16,
	INTERRUPT_WATCHDOG           = 17,
	INTERRUPT_CLKGEN             = 18,
	INTERRUPT_VCOMP              = 19,
	INTERRUPT_IOSLAVE            = 20,
	INTERRUPT_IOSACC             = 21,
	INTERRUPT_IOMASTER0          = 22,
	INTERRUPT_IOMASTER1          = 23,
	INTERRUPT_IOMASTER2          = 24,
	INTERRUPT_IOMASTER3          = 25,
	INTERRUPT_IOMASTER4          = 26,
	INTERRUPT_IOMASTER5          = 27,
	INTERRUPT_GPIO               = 28,
	INTERRUPT_CTIMER             = 29,
	INTERRUPT_UART0              = 30,
	INTERRUPT_UART1              = 31,
	//INTERRUPT_UART             =   (AM_HAL_INTERRUPT_UART0),
	INTERRUPT_ADC                = 32,
	INTERRUPT_PDM                = 33,
	INTERRUPT_STIMER             = 34,
	INTERRUPT_STIMER_CMPR0       = 35,
	INTERRUPT_STIMER_CMPR1       = 36,
	INTERRUPT_STIMER_CMPR2       = 37,
	INTERRUPT_STIMER_CMPR3       = 38,
	INTERRUPT_STIMER_CMPR4       = 39,
	INTERRUPT_STIMER_CMPR5       = 40,
	INTERRUPT_STIMER_CMPR6       = 41,
	INTERRUPT_STIMER_CMPR7       = 42,
	INTERRUPT_FLASH              = 43,

	INTERRUPT_TICK          	 = 44,//INTERRUPT_SOFTWARE0 as tick interrupt
	INTERRUPT_SOFTWARE1          = 45,
	INTERRUPT_SOFTWARE2          = 46,
	INTERRUPT_SOFTWARE3          = 47,
}interrupt_handle;
#endif

typedef enum
{
    MCU_EXTERNAL_RESET,
    MCU_POWERON_RESET,
    MCU_BROWNOUT_RESET,
    MCU_POR_RESET,
    MCU_POI_RESET,
    MCU_DEBUGGER_RESET,
    MCU_WDT_RESET,
    MCU_BOUNSUP_RESET,
    MCU_BOCORE_RESET,
    MCU_BOMEM_RESET,
    MCU_BOBLE_RESET,
    MCU_WDT_MAX  =0xFF,
}Rest_Type;

//**********************************************************************
// 函数功能: 初始化ambiq系统时钟：主频48M，开启外部晶体振荡器，
//           并使能低功耗电路
// 输入参数：	无
// 返回参数：	
//**********************************************************************
extern void SMDrv_SYS_ClockInit(void);

//**********************************************************************
// 函数功能: 使能NVIC中断
// 输入参数：无
// 返回参数：	
//**********************************************************************
extern void SMDrv_SYS_EnableIsr(interrupt_handle ui32Interrupt);

//**********************************************************************
// 函数功能: 禁止NVIC中断
// 输入参数：无
// 返回参数：	
//**********************************************************************
extern void SMDrv_SYS_DisableIsr(interrupt_handle ui32Interrupt);

//**********************************************************************
// 函数功能: Set a pending interrupt bit in the NVIC
// 输入参数：无
// 返回参数：	
//**********************************************************************
extern void SMDrv_SYS_SetIsrPend(interrupt_handle ui32Interrupt);

//**********************************************************************
// 函数功能: Clear a pending interrupt bit in the NVIC without servicing it
// 输入参数：无
// 返回参数：	
//**********************************************************************
extern void SMDrv_SYS_ClearIsrPend(interrupt_handle ui32Interrupt);

//**********************************************************************
// 函数功能: 设置中断优先级
// 输入参数：无
// 返回参数：	
//**********************************************************************
extern void SMDrv_SYS_SetIsrPrio(interrupt_handle ui32Interrupt,uint32 u32isrprio);

//**********************************************************************
// 函数功能: 使能中断请求
// 输入参数：无
// 返回参数：	
//**********************************************************************
extern void SMDrv_SYS_EnableMasterIsr(void);

//**********************************************************************
// 函数功能: 禁止中断请求
// 输入参数：无
// 返回参数：	
//**********************************************************************
extern void SMDrv_SYS_DisableMasterIsr(void);

//**********************************************************************
// 函数功能: POR Reset
// 输入参数：无
// 返回参数：	
//**********************************************************************
extern void SMDrv_SYS_PowerOnReset(void);

//**********************************************************************
// 函数功能: POI reset
// 输入参数：
// 返回参数：	
//**********************************************************************
extern void SMDrv_SYS_Rest_POI(void);

//**********************************************************************
// 函数功能:	系统休眠处理函数：主控休眠
// 输入参数：	无
// 返回参数：	
//**********************************************************************
extern void SMDrv_SYS_Sleep(void);

//**********************************************************************
// 函数功能: 延时n us
// 输入参数：u32nus:延时us数
// 返回参数：	
//**********************************************************************
extern void SMDrv_SYS_DelayUs(uint32 u32nus);

//**********************************************************************
// 函数功能: 延时n ms
// 输入参数：u32nms:延时ms数
// 返回参数：	
//**********************************************************************
extern void SMDrv_SYS_DelayMs(uint32 u32nms);

//**********************************************************************
// 函数功能: 写寄存器
// 输入参数：u32reg:寄存器地址，u32value:要写的值
// 返回参数：	
//**********************************************************************
extern void SMDrv_SYS_WriteReg(uint32 u32reg, uint32 u32value);

//**********************************************************************
// 函数功能: 获取MCU Rest type，即复位原因
// 输入参数：
// 返回参数：	
//**********************************************************************
extern Rest_Type SMDrv_SYS_GetRestType(void);

//**********************************************************************
// 函数功能:	配置系统心跳时钟（systick）,32HZ, 31.25ms/tick
//**********************************************************************
extern void SMDrv_SYSTick_ClockInit(void);

#endif

