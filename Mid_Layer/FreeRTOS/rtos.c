#include "platform_common.h"
#include "hal/am_hal_interrupt.h"
#include "sm_sys.h"
#include "am_mcu_apollo.h"

/* FreeRTOS includes */
#include "rtos.h"

#ifndef AM_FREERTOS_USE_STIMER_FOR_TICK   //Use Ctimer as system tick
//**********************************************************************
// 函数功能:	系统心跳时钟（systick）中断处理：（1，A）定时器中断->设置SOFTWARE0中断
//				->SOFTWARE0中断触发rtos_tick->回调SysTickIsr->回调RtosTickIsr
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
void rtos_tick(void)
{
    //清除软中断标志
	SMDrv_SYS_ClearIsrPend(AM_HAL_INTERRUPT_SOFTWARE0);

    // This is a timer a0 interrupt, perform the necessary functions for the tick ISR.
	(void) portSET_INTERRUPT_MASK_FROM_ISR();

    // Addition for support of SystemView Profiler
    traceISR_ENTER();
	// End addition
	{
		// Increment RTOS tick
		if ( xTaskIncrementTick() != pdFALSE )
		{
        	// Addition for support of SystemView Profiler
        	traceISR_EXIT_TO_SCHEDULER();
			// End addition

			// A context switch is required.  Context switching is
			// performed in the PendSV interrupt. Pend the PendSV
			// interrupt.
			portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;
		}
        // Addition for support of SystemView Profiler
        else
        {
        	traceISR_EXIT();
        }
		// End addition
	}
	portCLEAR_INTERRUPT_MASK_FROM_ISR(0);
}
#endif

//*****************************************************************************
//
// Sleep function called from FreeRTOS IDLE task.
// Do necessary application specific Power down operations here
// Return 0 if this function also incorporates the WFI, else return value same
// as idleTime
//
//*****************************************************************************
uint32_t am_freertos_sleep(uint32_t idleTime)
{
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
    return 0;
}

//*****************************************************************************
//
// Recovery function called from FreeRTOS IDLE task, after waking up from Sleep
// Do necessary 'wakeup' operations here, e.g. to power up/enable peripherals etc.
//
//*****************************************************************************
void am_freertos_wakeup(uint32_t idleTime)
{
    return;
}

//*****************************************************************************
//
// FreeRTOS debugging functions.
//
//*****************************************************************************
void
vApplicationMallocFailedHook(void)
{
	//
	// Called if a call to pvPortMalloc() fails because there is insufficient
	// free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	// internally by FreeRTOS API functions that create tasks, queues, software
	// timers, and semaphores.  The size of the FreeRTOS heap is set by the
	// configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h.
	//
	while (1);
}

void
vApplicationStackOverflowHook(void * pxTask, char *pcTaskName)
{
	(void) pcTaskName;
	(void) pxTask;

	//
	// Run time stack overflow checking is performed if
	// configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	// function is called if a stack overflow is detected.
	//
	while (1) {
    //jim        __asm("BKPT #0\n") ; // Break into the debugger
	}
}

