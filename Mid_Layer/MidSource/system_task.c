/**********************************************************************
**
**模块说明: mid层system接口
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.25  修改流程  ZSL  
**
**********************************************************************/
#include "sm_sys.h"

/* FreeRTOS includes */
#include "rtos.h"

#include "system_task.h"
#include "image_info.h"

#ifndef AM_FREERTOS_USE_STIMER_FOR_TICK
#define SLEEP_TASK_STACK_DEPTH       64
#define SLEEP_TASK_NAME              "SLEEP TASK"
#endif

//**********************************************************************
// 函数功能:	系统初始化，时钟设置、中断配置
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
void System_Init(void)
{
#if 0
    //说明:Apollo H001之前的案子采用旧的OTA升级流程，因此在此指定app代码段偏移地址
    //新的案子使用新的OTA流程，不再需要此功能
	/* Use the NVIC offset register to locate the stack. */
    //AM_REGVAL(0xE000ED08) = 0xA000;//0x10000; //jim----run @A000
    //SMDrv_SYS_WriteReg(0xE000ED08,0x8000);
#endif

    SMDrv_SYS_DisableMasterIsr();
    SMDrv_SYS_DisableIsr(INTERRUPT_ADC);
    SMDrv_SYS_DisableIsr(INTERRUPT_GPIO);
    SMDrv_SYS_DisableIsr(INTERRUPT_CTIMER);
    SMDrv_SYS_DisableIsr(INTERRUPT_UART0);
    SMDrv_SYS_DisableIsr(INTERRUPT_UART1);
#ifndef AM_FREERTOS_USE_STIMER_FOR_TICK
    SMDrv_SYS_DisableIsr(INTERRUPT_TICK);
#endif

    SMDrv_SYS_ClockInit();
    
    SMDrv_SYS_ClearIsrPend(INTERRUPT_ADC);
    SMDrv_SYS_ClearIsrPend(INTERRUPT_GPIO);
    SMDrv_SYS_ClearIsrPend(INTERRUPT_CTIMER);
    SMDrv_SYS_ClearIsrPend(INTERRUPT_UART0);
    SMDrv_SYS_ClearIsrPend(INTERRUPT_UART1);
#ifndef AM_FREERTOS_USE_STIMER_FOR_TICK
    SMDrv_SYS_ClearIsrPend(INTERRUPT_TICK);
#endif

    SMDrv_SYS_EnableIsr(INTERRUPT_ADC);
    SMDrv_SYS_EnableIsr(INTERRUPT_GPIO);
    SMDrv_SYS_EnableIsr(INTERRUPT_CTIMER);
    SMDrv_SYS_EnableIsr(INTERRUPT_UART0);
    SMDrv_SYS_EnableIsr(INTERRUPT_UART1);
#ifndef AM_FREERTOS_USE_STIMER_FOR_TICK
    SMDrv_SYS_EnableIsr(INTERRUPT_TICK);
#endif

    SMDrv_SYS_EnableMasterIsr();
}

#ifndef AM_FREERTOS_USE_STIMER_FOR_TICK
//**********************************************************************
// 函数功能:	空闲任务运行，关闭UART口低功耗
// 输入参数：	无
// 返回参数：	无
void Sleep_Task(void *pvParameters)
{
    while(1)
    {
        SMDrv_SYS_Sleep();
        taskYIELD();
    }
}

//**********************************************************************
// 函数功能:	创建空闲任务
// 输入参数：	无
// 返回参数：	无
void SystemTask_Create(void)
{
    xTaskCreate(Sleep_Task, SLEEP_TASK_NAME, SLEEP_TASK_STACK_DEPTH, NULL, TASK_PRIORITY_IDLE, NULL);
}
#endif

