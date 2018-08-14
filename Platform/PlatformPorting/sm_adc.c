/**********************************************************************
**
**模块说明: 对接MCU ADC驱动接口
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.17  初版  ZSL
**   V2.0   2018.8.9  增加apollo3  ZSL 
**
**********************************************************************/
#define ADC_MODULE

#include "io_config.h"
#include "am_mcu_apollo.h"
#include "sm_adc.h"

#if(ADC_PIN != IO_UNKNOW)
//define Bat ADC callback
static adc_cb Bat_Adc_CB;

#if(SMDRV_ADC_DEBUG ==1)
#define Adc_Debug(x) SEGGER_RTT_printf x
#else
#define Adc_Debug(x)
#endif

#ifdef AM_PART_APOLLO3

// ADC Device Handle.
static void *g_ADCHandle = NULL;

//**********************************************************************
//
// Interrupt handler for the ADC .
//
//**********************************************************************
void am_adc_isr(void)
{
    uint32 ui32IntMask;
    am_hal_adc_sample_t Sample;

    // Read the interrupt status.    
    if (am_hal_adc_interrupt_status(g_ADCHandle, &ui32IntMask, false) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error reading ADC interrupt status\n"));

    // Clear the ADC interrupt.
    if (am_hal_adc_interrupt_clear(g_ADCHandle, ui32IntMask) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error clearing ADC interrupt status\n"));

    // If we got a conversion completion interrupt (which should be our only
    // ADC interrupt), go ahead and read the data.
    if (ui32IntMask & AM_HAL_ADC_INT_CNVCMP)
    {
        uint32    ui32NumSamples = 1;
        if (am_hal_adc_samples_read(g_ADCHandle,NULL,&ui32NumSamples,&Sample) != AM_HAL_STATUS_SUCCESS)
            Err_Info((0,"Error - ADC sample read from FIFO failed.\n"));

        Adc_Debug((0,"ADC Slot =  %d\n", Sample.ui32Slot));
        Adc_Debug((0,"ADC Value = %8.8X\n", Sample.ui32Sample));
        if((Sample.ui32Slot == BAT_ADC_SLOT) && (Bat_Adc_CB != NULL))
        {            
            (Bat_Adc_CB)(Sample.ui32Sample);
        }
    }
}

//**********************************************************************
// 函数功能: 初始化ADC
// 输入参数：
// 返回参数：
//**********************************************************************
void SMDrv_ADC_Init(void)
{
    // Define the ADC  pin to be used.
    am_hal_gpio_pincfg_t t_AdcConfig;
    
    Bat_Adc_CB = NULL;
    // Set a pin to act as our ADC input
    t_AdcConfig.uFuncSel  = 0,  //0 is ADC function sel
    am_hal_gpio_pinconfig(ADC_PIN, t_AdcConfig);
}

//*****************************************************************************
//函数功能：打开一个ADC通道
// 输入参数：adc_slot：   adc 通道号
// 			adc_callback: callback函调函数
// 返回：	无
//*****************************************************************************
ret_type SMDrv_ADC_Open(uint32 adc_slot,adc_cb adc_callback)
{
    am_hal_adc_config_t  ADCConfig;
    am_hal_adc_slot_config_t ADCSlotConfig;

    if(adc_callback != NULL)
    {
        Bat_Adc_CB = adc_callback;
    }

    // Initialize the ADC and get the handle.
    if (am_hal_adc_initialize(0, &g_ADCHandle) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - reservation of the ADC instance failed.\n"));
    
    // Power on the ADC.
    if (am_hal_adc_power_control(g_ADCHandle,AM_HAL_SYSCTRL_WAKE,false) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - ADC power on failed.\n"));
    
    // Set up the ADC configuration parameters. These settings are reasonable
    // for accurate measurements at a low sample rate.
    ADCConfig.eClock             = AM_HAL_ADC_CLKSEL_HFRC;
    ADCConfig.ePolarity          = AM_HAL_ADC_TRIGPOL_RISING;
    ADCConfig.eTrigger           = AM_HAL_ADC_TRIGSEL_SOFTWARE;
    ADCConfig.eReference         = AM_HAL_ADC_REFSEL_INT_2P0;
    ADCConfig.eClockMode         = AM_HAL_ADC_CLKMODE_LOW_POWER;
    ADCConfig.ePowerMode         = AM_HAL_ADC_LPMODE0;
    ADCConfig.eRepeat            = AM_HAL_ADC_REPEATING_SCAN;
    if(am_hal_adc_configure(g_ADCHandle, &ADCConfig) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - configuring ADC failed.\n"));
    
    // Set up an ADC slot
    ADCSlotConfig.eMeasToAvg      = AM_HAL_ADC_SLOT_AVG_1;
    ADCSlotConfig.ePrecisionMode  = AM_HAL_ADC_SLOT_14BIT;
    ADCSlotConfig.eChannel        = AM_HAL_ADC_SLOT_CHSEL_SE0;
    ADCSlotConfig.bWindowCompare  = false;
    ADCSlotConfig.bEnabled        = true;
    if(am_hal_adc_configure_slot(g_ADCHandle, 0, &ADCSlotConfig) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - configuring ADC Slot 0 failed.\n"));
    
    // For this example, the samples will be coming in slowly. This means we
    // can afford to wake up for every conversion.
    am_hal_adc_interrupt_enable(g_ADCHandle, AM_HAL_ADC_INT_CNVCMP );
 
    return Ret_OK;
}

//*****************************************************************************
//函数功能：关闭一个打开的ADC通道
// 输入参数：adc_slot：   adc 通道号
// 			adc_callback: callback函调函数
// 返回：	无
//*****************************************************************************
ret_type SMDrv_ADC_Close(void)
{
    if(g_ADCHandle == NULL)
    {
        Err_Info((0,"Error - ADC handle is NULL\n"));
        return Ret_InvalidParam;
    }
        
    Bat_Adc_CB = NULL;
    //step 1: disable ADC
    if(am_hal_adc_disable(g_ADCHandle) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - disable ADC failed.\n"));

    //step 2: disable irq
#if AM_CMSIS_REGS
    NVIC_DisableIRQ(ADC_IRQn);
#else
    am_hal_interrupt_disable(AM_HAL_INTERRUPT_ADC);
#endif

    //step 3: Disable ADC power
    if(am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_ADC) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - disabling the ADC power domain failed.\n"));

    //step 4: deInitialize the ADC
    if(am_hal_adc_deinitialize(g_ADCHandle) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - return of the ADC instance failed.\n"));

    return Ret_OK;
}

//**********************************************************************
// 函数功能: ADC模块使能，电路上电
// 输入参数：无
// 返回参数:
//**********************************************************************
void SMDrv_ADC_Enable(void)
{
    if(g_ADCHandle == NULL)
    {
        Err_Info((0,"Error - ADC handle is NULL\n"));
        return ;
    }

    //step 1: enable ADC power
    if(am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_ADC) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - enable the ADC power failed.\n"));

    //step 2: enable ADC
    if(am_hal_adc_enable(g_ADCHandle) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - enable ADC failed.\n"));
}

//**********************************************************************
// 函数功能: ADC模块关闭，电路断电
// 输入参数：无
// 返回参数：
//**********************************************************************
void SMDrv_ADC_Disable(void)
{
    if(g_ADCHandle == NULL)
    {
        Err_Info((0,"Error - ADC handle is NULL\n"));
        return ;
    }

    //step 1: disable ADC
    if(am_hal_adc_disable(g_ADCHandle) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - disable ADC failed.\n"));

    //step 2: Disable ADC power    
    if(am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_ADC) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - disabling the ADC power domain failed.\n"));
}

//**********************************************************************
// 函数功能: 启动ADC转换
// 输入参数：无
// 返回参数：	
//**********************************************************************
void SMDrv_ADC_StartDetect(void)
{
    if(g_ADCHandle == NULL)
    {
        Err_Info((0,"Error - ADC handle is NULL\n"));
        return ;
    }

    am_hal_adc_sw_trigger(g_ADCHandle);
}

#else

//**********************************************************************
//
// Interrupt handler for the ADC .
//
//**********************************************************************
void am_adc_isr(void)
{
    uint32 ui32Status, ui32FifoData;
    volatile uint32 data;
    volatile uint32 slot;
    //volatile uint32 size;

    // Read the interrupt status.
    ui32Status = am_hal_adc_int_status_get(true);

    // If we got a conversion completion interrupt (which should be our only
    // ADC interrupt), go ahead and read the data.
    if (ui32Status & AM_HAL_ADC_INT_CNVCMP)
    {
        ui32FifoData = am_hal_adc_fifo_pop();
		
        data = AM_HAL_ADC_FIFO_SAMPLE(ui32FifoData);
        slot = AM_HAL_ADC_FIFO_SLOT(ui32FifoData);
        //size = AM_HAL_ADC_FIFO_COUNT(ui32FifoData);
	
        if((slot == BAT_ADC_SLOT) && (Bat_Adc_CB != NULL))
        {            
            Bat_Adc_CB(data);
        }
    }
    // Clear the ADC interrupt.
    am_hal_adc_int_clear(ui32Status);	
}

//**********************************************************************
// 函数功能: 初始化ADC
// 输入参数：
// 返回参数：
//**********************************************************************
void SMDrv_ADC_Init(void)
{
    am_hal_adc_config_t AdcConfig = 
    {
        .ui32Clock= AM_HAL_ADC_CLOCK_DIV2,
        .ui32TriggerConfig=AM_HAL_ADC_TRIGGER_SOFT,
        .ui32Reference=AM_HAL_ADC_REF_INT_2P0,
        .ui32ClockMode=AM_HAL_ADC_CK_LOW_POWER,
        .ui32PowerMode= AM_HAL_ADC_LPMODE_1,
        .ui32Repeat=AM_HAL_ADC_NO_REPEAT,
    };
    
    Bat_Adc_CB = NULL;

    // Set a pin to act as our ADC input
    am_hal_gpio_pin_config(ADC_PIN, 0);
        
    //power on ADC and init ADC
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_ADC);
    am_hal_adc_config(&AdcConfig);

    //set ADC interrupt type
    am_hal_adc_int_enable(AM_HAL_ADC_INT_WCINC | AM_HAL_ADC_INT_WCEXC | AM_HAL_ADC_INT_FIFOOVR2 |
                          AM_HAL_ADC_INT_FIFOOVR1 | AM_HAL_ADC_INT_SCNCMP | AM_HAL_ADC_INT_CNVCMP);
}

//*****************************************************************************
//函数功能：打开一个ADC通道
// 输入参数：adc_slot：   adc 通道号
// 			adc_callback: callback函调函数
// 返回：	无
//*****************************************************************************
ret_type SMDrv_ADC_Open(uint32 adc_slot,adc_cb adc_callback)
{
    uint32 ui32SlotConfig;

    if(adc_slot == BAT_ADC_SLOT)
    {
        if(adc_callback != NULL)
        {
            Bat_Adc_CB = adc_callback;
        }

        //config BAT slot param
        ui32SlotConfig = AM_HAL_ADC_SLOT_AVG_1 | AM_HAL_ADC_SLOT_14BIT | AM_HAL_ADC_SLOT_CHSEL_SE3 |\
            AM_HAL_ADC_SLOT_WINDOW_EN | AM_HAL_ADC_SLOT_ENABLE;
    }
    else
    {
         return Ret_InvalidParam;
    }
    
    am_hal_adc_slot_config(adc_slot, ui32SlotConfig);

    return Ret_OK;
}

//*****************************************************************************
//函数功能：关闭一个打开的ADC通道
// 输入参数：adc_slot：   adc 通道号
// 			adc_callback: callback函调函数
// 返回：	无
//*****************************************************************************
ret_type SMDrv_ADC_Close(void)
{
    Bat_Adc_CB = NULL;

    //step 2: disable ADC
    am_hal_adc_disable();

    //step 1: disable irq
#if AM_CMSIS_REGS
    NVIC_DisableIRQ(ADC_IRQn);
#else
    am_hal_interrupt_disable(AM_HAL_INTERRUPT_ADC);
#endif

    //step 3: Disable ADC power
    am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_ADC);
    return Ret_OK;
}

//**********************************************************************
// 函数功能: ADC模块使能，电路上电
// 输入参数：无
// 返回参数:
//**********************************************************************
void SMDrv_ADC_Enable(void)
{
    //step 1: enable ADC power
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_ADC);

    //step 2: enable ADC
    am_hal_adc_enable();
}

//**********************************************************************
// 函数功能: ADC模块关闭，电路断电
// 输入参数：无
// 返回参数：
//**********************************************************************
void SMDrv_ADC_Disable(void)
{	
    //step 1: disable ADC
    am_hal_adc_disable();

    //step 2: Disable ADC power
    am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_ADC);
}

//**********************************************************************
// 函数功能: 启动ADC转换
// 输入参数：无
// 返回参数：	
//**********************************************************************
void SMDrv_ADC_StartDetect(void)
{
    am_hal_adc_trigger();
}

#endif

//**********************************************************************
// 函数功能:  设置ADC中断优先级,并启动ADC中断
// 输入参数:
//     prio:中断优先级，bit0~2位值有效
// 返回参数： 无
//**********************************************************************
ret_type SMDrv_ADC_SetIsrPrio(uint32 prio)
{
    //the user interrupt priority must less than 4
#if AM_CMSIS_REGS
    NVIC_SetPriority(ADC_IRQn,prio);
    NVIC_EnableIRQ(ADC_IRQn);
#else
    am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_ADC, AM_HAL_INTERRUPT_PRIORITY(prio));
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_ADC);
#endif
    return Ret_OK;
}
#else
//**********************************************************************
// 函数功能: 初始化ADC
// 输入参数：
// 返回参数：
//**********************************************************************
void SMDrv_ADC_Init(void)
{
}
#endif

