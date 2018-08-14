/**********************************************************************
**
**模块说明: 对接MCU WDT驱动接口
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.17  初版  ZSL  
**   V2.0   2018.8.9  增加apollo3  ZSL 
**
**********************************************************************/
#define SWD_MODULE
#include "am_mcu_apollo.h"
#include "am_util_delay.h"
#include "io_config.h"
#include "sm_sys.h"
#include "sm_timer.h"
#include "sm_gpio.h"

//**********************************************************************
// 函数功能: 使能NVIC中断
// 输入参数：无
// 返回参数：	
//**********************************************************************
void SMDrv_SYS_EnableIsr(interrupt_handle ui32Interrupt)
{
#if AM_CMSIS_REGS
    NVIC_EnableIRQ((IRQn_Type)(ui32Interrupt - 16));
#else
    am_hal_interrupt_enable(ui32Interrupt);
#endif
}

//**********************************************************************
// 函数功能: 禁止NVIC中断
// 输入参数：无
// 返回参数：	
//**********************************************************************
void SMDrv_SYS_DisableIsr(interrupt_handle ui32Interrupt)
{
#if AM_CMSIS_REGS
    NVIC_DisableIRQ((IRQn_Type)(ui32Interrupt - 16));
#else
    am_hal_interrupt_disable(ui32Interrupt);
#endif
}

//**********************************************************************
// 函数功能: Set a pending interrupt bit in the NVIC
// 输入参数：无
// 返回参数：	
//**********************************************************************
void SMDrv_SYS_SetIsrPend(interrupt_handle ui32Interrupt)
{
#if AM_CMSIS_REGS
    NVIC_SetPendingIRQ((IRQn_Type)(ui32Interrupt - 16));
#else
    am_hal_interrupt_pend_set(ui32Interrupt);
#endif
}

//**********************************************************************
// 函数功能: Clear a pending interrupt bit in the NVIC without servicing it
// 输入参数：无
// 返回参数：	
//**********************************************************************
void SMDrv_SYS_ClearIsrPend(interrupt_handle ui32Interrupt)
{
#if AM_CMSIS_REGS
    NVIC_ClearPendingIRQ((IRQn_Type)(ui32Interrupt - 16));
#else
    am_hal_interrupt_pend_clear(ui32Interrupt);
#endif
}

//**********************************************************************
// 函数功能: 设置中断优先级
// 输入参数：无
// 返回参数：	
//**********************************************************************
void SMDrv_SYS_SetIsrPrio(interrupt_handle ui32Interrupt,uint32 u32isrprio)
{
#if AM_CMSIS_REGS
    NVIC_SetPriority((IRQn_Type)(ui32Interrupt - 16),u32isrprio);
#else
    am_hal_interrupt_priority_set(ui32Interrupt,AM_HAL_INTERRUPT_PRIORITY(u32isrprio));
#endif
}

//**********************************************************************
// 函数功能: 使能中断请求
// 输入参数：无
// 返回参数：	
//**********************************************************************
void SMDrv_SYS_EnableMasterIsr(void)
{
    am_hal_interrupt_master_enable();
}

//**********************************************************************
// 函数功能: 禁止中断请求
// 输入参数：无
// 返回参数：	
//**********************************************************************
void SMDrv_SYS_DisableMasterIsr(void)
{
    am_hal_interrupt_master_disable();
}

//**********************************************************************
// 函数功能: POR Reset
// 输入参数：无
// 返回参数：	
//**********************************************************************
void SMDrv_SYS_PowerOnReset(void)
{
#if AM_APOLLO3_RESET
    am_hal_reset_control(AM_HAL_RESET_CONTROL_SWPOI, 0);
#else
    am_hal_reset_por();
#endif
}

//**********************************************************************
// 函数功能: POI reset
// 输入参数：
// 返回参数：	
//**********************************************************************
void SMDrv_SYS_Rest_POI(void)
{
#if AM_APOLLO3_RESET
    am_hal_reset_control(AM_HAL_RESET_CONTROL_SWPOI, 0);
#else
    am_hal_reset_poi();
#endif
}

//**********************************************************************
// 函数功能:	系统休眠处理函数：主控休眠
// 输入参数：	无
// 返回参数：	
//**********************************************************************
void SMDrv_SYS_Sleep(void)
{
    am_hal_interrupt_master_disable();

#if 0   //BLE使用UART通信情况下，需要关闭UART
    // Go to deep sleep.
    if(uart1Handle == UART1_IDLE)
    {
         //am_util_delay_us(162);
		am_hal_uart_clock_disable(1);
        am_hal_uart_disable(1);    
        am_hal_gpio_pin_config(14,AM_HAL_PIN_INPUT |AM_HAL_GPIO_PULL24K);
		am_hal_gpio_pin_config(15,AM_HAL_PIN_INPUT |AM_HAL_GPIO_PULL24K );	
        am_hal_uart_pwrctrl_disable(1);
    }
	if(uart0Handle == UART0_IDLE)
    {
        // am_util_delay_us(162);
		am_hal_uart_clock_disable(0);
        am_hal_uart_disable(0);
        am_hal_gpio_pin_config(1,AM_HAL_PIN_INPUT |AM_HAL_GPIO_PULL24K);
		am_hal_gpio_pin_config(2,AM_HAL_PIN_INPUT |AM_HAL_GPIO_PULL24K );	     
        am_hal_uart_pwrctrl_disable(0);
    }
#endif
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
    
    // Re-enable interrupts. should not be 
    am_hal_interrupt_master_enable();
}

//**********************************************************************
// 函数功能: 延时n us
// 输入参数：u32nus:延时us数
// 返回参数：	
//**********************************************************************
void SMDrv_SYS_DelayUs(uint32 u32nus)
{
    am_util_delay_us(u32nus);
}

//**********************************************************************
// 函数功能: 延时n ms
// 输入参数：u32nms:延时ms数
// 返回参数：	
//**********************************************************************
void SMDrv_SYS_DelayMs(uint32 u32nms)
{
    am_util_delay_ms(u32nms);
}

//**********************************************************************
// 函数功能: 写寄存器
// 输入参数：u32reg:寄存器地址，u32value:要写的值
// 返回参数：	
//**********************************************************************
void SMDrv_SYS_WriteReg(uint32 u32reg, uint32 u32value)
{
    AM_REGVAL(u32reg) = u32value;
}

#ifdef AM_PART_APOLLO3
//**********************************************************************
// 函数功能: 获取MCU Rest type，即复位原因
// 输入参数：
// 返回参数：	
//**********************************************************************
Rest_Type SMDrv_SYS_GetRestType(void)
{
    uint32 u32RestType;
    am_hal_reset_status_t sStatus;

    //step 1: Get reset status
    am_hal_reset_status_get(&sStatus);
    //step 2: clear reset status 
    am_hal_reset_control(AM_HAL_RESET_CONTROL_STATUSCLEAR,NULL);

    if(sStatus.bWDTStat == TRUE)          // Watch Dog Timer reset
        return MCU_WDT_RESET;
    else if(sStatus.bSWPOIStat == TRUE)   // SW Power On Initialization reset
        return MCU_POI_RESET;
    else if(sStatus.bSWPORStat == TRUE)   // SW Power-On reset or AIRCR reset
        return MCU_POR_RESET;
    else if(sStatus.bBODStat == TRUE)     // Brown-Out reset
        return MCU_BROWNOUT_RESET;
    else if(sStatus.bPORStat == TRUE)     // Power-On reset
        return MCU_POWERON_RESET;
    else if(sStatus.bEXTStat == TRUE)     // External reset
        return MCU_EXTERNAL_RESET;
    else if(sStatus.bDBGRStat == TRUE)    // Debugger reset
        return MCU_DEBUGGER_RESET;
    else if(sStatus.bBOUnregStat == TRUE) // Unregulated Supply Brownout event
        return MCU_BOUNSUP_RESET;
    else if(sStatus.bBOCOREStat == TRUE)  // Core Regulator Brownout event
        return MCU_BOCORE_RESET;
    else if(sStatus.bBOMEMStat == TRUE)   // Memory Regulator Brownout event
        return MCU_BOMEM_RESET;
    else if(sStatus.bBOBLEStat == TRUE)   // BLE/Burst Regulator Brownout event
        return MCU_BOBLE_RESET;
    else
        ;
    return (Rest_Type)u32RestType;
}

//**********************************************************************
// 函数功能:	配置系统心跳时钟（systick）,32HZ, 31.25ms/tick
//**********************************************************************
void SMDrv_SYSTick_ClockInit(void)
{
    // Enable the timer interrupt in the NVIC, making sure to use the appropriate priority level.

    //step 1: 设置软件中断，用作systick中断
    //不用mcu内核systick原因:是因为它的功耗要比ctimer的大
    //am_hal_interrupt_pend_clear(AM_HAL_INTERRUPT_SOFTWARE0);
    //am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_SOFTWARE0, AM_HAL_INTERRUPT_PRIORITY(7));	//Systick interrupt level is 7
    //am_hal_interrupt_enable(AM_HAL_INTERRUPT_SOFTWARE0);
}

//**********************************************************************
// 函数功能: 初始化ambiq系统时钟：主频48M，开启外部晶体振荡器，
//           并使能低功耗电路
// 输入参数：	无
// 返回参数：	
//**********************************************************************
void SMDrv_SYS_ClockInit(void)
{
    am_hal_gpio_pincfg_t tSwdconfig;

    // am_hal_interrupt_master_disable();

    //step 1: Set the clock frequency
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_XTAL_START,NULL);
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX, NULL);

    //step 2:  Set the default cache configuration
    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();

#ifndef NOFPU
    // Enable the floating point module, and configure the core for lazy stacking.
    am_hal_sysctrl_fpu_enable();
    am_hal_sysctrl_fpu_stacking_enable(true);
#else
    am_hal_sysctrl_fpu_disable();
#endif
    am_util_delay_ms(500);
    am_util_delay_ms(500);
    am_util_delay_ms(500);

    // am_hal_interrupt_master_enable();

    // 外部烧录口内部上拉，防止悬浮漏电
    tSwdconfig.uFuncSel = AM_HAL_PIN_20_SWDCK;
    tSwdconfig.ePullup  = AM_HAL_GPIO_PIN_PULLUP_12K;
    am_hal_gpio_pinconfig(SWD_CK_PIN, tSwdconfig);
    tSwdconfig.uFuncSel = AM_HAL_PIN_21_SWDIO;
    tSwdconfig.ePullup  = AM_HAL_GPIO_PIN_PULLUP_12K;
    am_hal_gpio_pinconfig(SWD_CK_PIN, tSwdconfig);
}
#else    // apollo/apollo2
//**********************************************************************
// 函数功能: 获取MCU Rest type，即复位原因
// 输入参数：
// 返回参数：	
//**********************************************************************
Rest_Type SMDrv_SYS_GetRestType(void)
{
    uint32 u32RestType;

    //step 1: Get reset status
    u32RestType = am_hal_reset_status_get();
    //step 2: clear reset status 
    am_hal_reset_status_clear();

    if(u32RestType == AM_REG_RSTGEN_STAT_WDRSTAT_M)
        return MCU_WDT_RESET;
    else if(u32RestType == AM_REG_RSTGEN_STAT_POIRSTAT_M)
        return MCU_POI_RESET;
    else if(u32RestType == AM_REG_RSTGEN_STAT_SWRSTAT_M)
        return MCU_POR_RESET;
    else if(u32RestType == AM_REG_RSTGEN_STAT_BORSTAT_M)
        return MCU_BROWNOUT_RESET;
    else if(u32RestType == AM_REG_RSTGEN_STAT_PORSTAT_M)
        return MCU_POWERON_RESET;
    else if(u32RestType == AM_REG_RSTGEN_STAT_EXRSTAT_M)
        return MCU_EXTERNAL_RESET;
    else if(u32RestType == AM_REG_RSTGEN_STAT_DBGRSTAT_M)
        return MCU_DEBUGGER_RESET;
    else
        ;
    return (Rest_Type)u32RestType;
}

//**********************************************************************
// 函数功能:	配置系统心跳时钟（systick）,32HZ, 31.25ms/tick
//**********************************************************************
void SMDrv_SYSTick_ClockInit(void)
{
    // Enable the timer interrupt in the NVIC, making sure to use the appropriate priority level.

    //step 1: 设置软件中断，用作systick中断
    //不用mcu内核systick原因:是因为它的功耗要比ctimer的大
    am_hal_interrupt_pend_clear(AM_HAL_INTERRUPT_SOFTWARE0);
    am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_SOFTWARE0, AM_HAL_INTERRUPT_PRIORITY(7));	//Systick interrupt level is 7
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_SOFTWARE0);
}

//**********************************************************************
// 函数功能: 初始化ambiq系统时钟：主频48M，开启外部晶体振荡器，
//           并使能低功耗电路
// 输入参数：	无
// 返回参数：	
//**********************************************************************
void SMDrv_SYS_ClockInit(void)
{
    // am_hal_interrupt_master_disable();

    am_hal_clkgen_osc_start(AM_HAL_CLKGEN_OSC_XT);
    am_hal_clkgen_sysclk_select(AM_HAL_CLKGEN_SYSCLK_MAX);
    am_hal_pwrctrl_bucks_enable();
    am_util_delay_ms(500);
    am_util_delay_ms(500);
    am_util_delay_ms(500);

    // am_hal_interrupt_master_enable();

    // 外部烧录口内部上拉，防止悬浮漏电
    SMDrv_GPIO_SetIOPad(SWD_CK_PIN,AM_HAL_PIN_20_SWDCK | AM_HAL_GPIO_PULL12K);
    SMDrv_GPIO_SetIOPad(SWD_IO_PIN,AM_HAL_PIN_21_SWDIO | AM_HAL_GPIO_PULL12K);
}
#endif

