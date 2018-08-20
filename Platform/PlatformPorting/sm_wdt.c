/**********************************************************************
**
**模块说明: 对接MCU WDT驱动接口
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.17  初版  ZSL  
**
**********************************************************************/
#include "am_mcu_apollo.h"
#include "platform_common.h"
#include "sm_wdt.h"
#include "sm_sys.h"

static comm_cb *pWdt_CallBack = NULL;

//看门狗中断优先级
#define     WDT_PRIO        2

//*****************************************************************************
//
// Interrupt handler for the watchdog .
//
//*****************************************************************************
void am_watchdog_isr(void)
{
	am_hal_wdt_int_clear();
    if(pWdt_CallBack != NULL)
    {
        (pWdt_CallBack)();
    }
}

//**********************************************************************
// 函数功能: 初始化WatchDog Timer
// 输入参数：	
// 返回参数：
//**********************************************************************
void SMDrv_WDT_Init(void)
{
    pWdt_CallBack = NULL;
}

//*****************************************************************************
//函数功能：打开看门狗
// 输入：	resetCount：	复位计数，时钟源为128HZ，最大设置值为255，即2秒
// 							需比中断计数大
//			interruptCount：中断计数，时钟源为128HZ，最大设置值为255，即2秒
//							需比复位计数小
// 			IsrCb:			看门狗中断复位回调
// 返回：	无
//*****************************************************************************
ret_type SMDrv_WDT_Open(uint16 resetCount,uint16 interruptCount,comm_cb *wdt_callback)
{
    am_hal_wdt_config_t configTemp;

    //step 1: setting config to init wdt,include count, enable reset and irq flag
    configTemp.ui16InterruptCount   = interruptCount;
    configTemp.ui16ResetCount       = resetCount;
#if(defined(AM_PART_APOLLO2) || defined(AM_PART_APOLLO3))
    configTemp.ui32Config           = WDT_CFG_CLKSEL_128HZ | AM_HAL_WDT_ENABLE_RESET | AM_HAL_WDT_ENABLE_INTERRUPT;
#else
    configTemp.ui32Config           = AM_HAL_WDT_ENABLE_RESET | AM_HAL_WDT_ENABLE_INTERRUPT;
#endif
    am_hal_wdt_init(&configTemp);
 
    if(wdt_callback != NULL)
    {
        pWdt_CallBack = wdt_callback;
    }

    //step 2:设置中断优化先级,并启动中断
    SMDrv_SYS_SetIsrPrio(INTERRUPT_WATCHDOG,WDT_PRIO);
    SMDrv_SYS_EnableIsr(INTERRUPT_WATCHDOG);
    return Ret_OK; 
}

//*****************************************************************************
//函数功能：关闭看门狗，重新开启需重新初始化
// 输入：	无
// 返回：	无
//*****************************************************************************
ret_type SMDrv_WDT_Close(void)
{
    am_hal_wdt_config_t configTemp;

	//step 1: Stops the watchdog timer
	am_hal_wdt_start();
    am_hal_wdt_halt();

    //step 2: disable wdt's reset and irq flag
    configTemp.ui16InterruptCount  = 128;
    configTemp.ui16ResetCount      = 255;
    configTemp.ui32Config		   = AM_HAL_WDT_DISABLE_RESET | AM_HAL_WDT_DISABLE_INTERRUPT;
    am_hal_wdt_init(&configTemp);

    pWdt_CallBack = NULL;
    
    //step 3: disable wdt interrupt
    SMDrv_SYS_DisableIsr(INTERRUPT_WATCHDOG);
    return Ret_OK;
}

//**********************************************************************
// 函数功能:  设置WDT中断优先级,并启动WDT中断
// 输入参数:
//     prio:中断优先级，bit0~2位值有效
// 返回参数： 无
//**********************************************************************
ret_type SMDrv_WDT_SetIsrPrio(uint32 prio)
{
    //step :设置中断优化先级,并启动中断
    SMDrv_SYS_SetIsrPrio(INTERRUPT_WATCHDOG,prio);
    SMDrv_SYS_EnableIsr(INTERRUPT_WATCHDOG);
    return Ret_OK;
}

//*****************************************************************************
//函数功能：复位看门狗
// 输入：	无
// 返回：	无
//*****************************************************************************
void SMDrv_WDT_ReStart(void)
{
    //reset wdt first,and then start wdt
    am_hal_wdt_start();
}

