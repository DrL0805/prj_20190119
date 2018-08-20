/**********************************************************************
**
**模块说明: 对接MCU ADC驱动接口
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.17  初版  ZSL
**   V2.0   2018.8.9  增加apollo3  ZSL 
**
**********************************************************************/
#include "io_config.h"
#include "am_mcu_apollo.h"
#include "sm_adc.h"

#if((BAT_ADC_PIN != IO_UNKNOW) || (TEMP_ADC_PIN != IO_UNKNOW))
//define ADC callback
static adc_cb adc_slot_cb[MAX_ADC_MODULE] = {NULL};

#if 1//(SMDRV_ADC_DEBUG ==1)
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
    uint32 ui32IntMask,u32slot;
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
        Adc_Debug((0,"ADC Value = %d\n", Sample.ui32Sample));
        for(u32slot = 0; u32slot < MAX_ADC_MODULE; u32slot++)
        {
            if((Sample.ui32Slot == u32slot) && (adc_slot_cb[u32slot] != NULL))
            {
                (adc_slot_cb[u32slot])(Sample.ui32Sample);
            }
        }
    }
}

//**********************************************************************
// 函数功能: 通过Module配置slot Channel，pin
//**********************************************************************
static uint8 adc_modul2info(adc_module modul,am_hal_adc_slot_config_t *pSlotConfig,uint32 *adc_pin)
{
    if((pSlotConfig == NULL) || (adc_pin == NULL))
        return FALSE;

    //step 1: config slot,adc pin,需根据模块配置
    switch(modul)
    {
    case BAT_ADC_MODULE:  //BAT Adc
        pSlotConfig->eMeasToAvg      = AM_HAL_ADC_SLOT_AVG_1;
        pSlotConfig->ePrecisionMode  = AM_HAL_ADC_SLOT_14BIT;
        *adc_pin = BAT_ADC_PIN;
        break;
    case BODY_TEMP_ADC_MODULE:   //体温，热敏
        pSlotConfig->eMeasToAvg      = AM_HAL_ADC_SLOT_AVG_1;
        pSlotConfig->ePrecisionMode  = AM_HAL_ADC_SLOT_14BIT;
        *adc_pin = TEMP_ADC_PIN;
        break;
    default:
        return FALSE;
    }

    //step 2: select channel，此部分是mcu决定，无需改动
    if(*adc_pin == 11)
        pSlotConfig->eChannel = AM_HAL_ADC_SLOT_CHSEL_SE2;
    else if(*adc_pin == 12)
        pSlotConfig->eChannel = AM_HAL_ADC_SLOT_CHSEL_SE9;
    else if(*adc_pin == 13)
        pSlotConfig->eChannel = AM_HAL_ADC_SLOT_CHSEL_SE8;
    else if(*adc_pin == 16)
        pSlotConfig->eChannel = AM_HAL_ADC_SLOT_CHSEL_SE0;
    else if(*adc_pin == 29)
        pSlotConfig->eChannel = AM_HAL_ADC_SLOT_CHSEL_SE1;
    else if(*adc_pin == 31)
        pSlotConfig->eChannel = AM_HAL_ADC_SLOT_CHSEL_SE3;
    else if(*adc_pin == 32)
        pSlotConfig->eChannel = AM_HAL_ADC_SLOT_CHSEL_SE4;
    else if(*adc_pin == 33)
        pSlotConfig->eChannel = AM_HAL_ADC_SLOT_CHSEL_SE5;
    else if(*adc_pin == 34)
        pSlotConfig->eChannel = AM_HAL_ADC_SLOT_CHSEL_SE6;
    else if(*adc_pin == 35)
        pSlotConfig->eChannel = AM_HAL_ADC_SLOT_CHSEL_SE7;
    else
        return FALSE;

    return TRUE;
}

//**********************************************************************
// 函数功能: 初始化ADC
// 输入参数：
// 返回参数：
//**********************************************************************
void SMDrv_ADC_Init(void)
{
    am_hal_adc_config_t  ADCConfig;
    uint32 ret;

    memset(adc_slot_cb,NULL,sizeof(adc_slot_cb));

    //step 1: Initialize the ADC and get the handle.
    if((ret = am_hal_adc_initialize(0, &g_ADCHandle)) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - reservation of the ADC instance failed:%d\n",ret));
    
    //step 2: Power on the ADC.
    if((ret = am_hal_adc_power_control(g_ADCHandle,AM_HAL_SYSCTRL_WAKE,false)) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - ADC power on failed:%d\n",ret));
    
    //step 3: Set up the ADC configuration parameters. These settings are reasonable
    // for accurate measurements at a low sample rate.
    ADCConfig.eClock             = AM_HAL_ADC_CLKSEL_HFRC;
    ADCConfig.ePolarity          = AM_HAL_ADC_TRIGPOL_RISING;
    ADCConfig.eTrigger           = AM_HAL_ADC_TRIGSEL_SOFTWARE;
    ADCConfig.eReference         = AM_HAL_ADC_REFSEL_INT_2P0;
    ADCConfig.eClockMode         = AM_HAL_ADC_CLKMODE_LOW_POWER;
    ADCConfig.ePowerMode         = AM_HAL_ADC_LPMODE0;
    ADCConfig.eRepeat            = AM_HAL_ADC_SINGLE_SCAN;
    if((ret = am_hal_adc_configure(g_ADCHandle, &ADCConfig)) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - configuring ADC failed:%d\n",ret));

    //step 4: For this example, the samples will be coming in slowly. This means we
    // can afford to wake up for every conversion.
    am_hal_adc_interrupt_enable(g_ADCHandle, AM_HAL_ADC_INT_CNVCMP);
}

//**********************************************************************
// 函数功能: 解除ADC
// 输入参数：
// 返回参数：
//**********************************************************************
void SMDrv_ADC_DeInit(void)
{
    uint32 ret;
    if(g_ADCHandle == NULL)
    {
        Err_Info((0,"Error - ADC handle is NULL\n"));
        return ;
    }

    //step 1: disable ADC
    if((ret = am_hal_adc_disable(g_ADCHandle)) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - disable ADC failed:%d\n",ret));
    
    //step 2: disable irq
#if AM_CMSIS_REGS
    NVIC_DisableIRQ(ADC_IRQn);
#else
    am_hal_interrupt_disable(AM_HAL_INTERRUPT_ADC);
#endif
    
    //step 3: Disable ADC power
    if((ret = am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_ADC)) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - disabling the ADC power domain failed:%d\n",ret));
    
    if((ret = am_hal_adc_power_control(g_ADCHandle,AM_HAL_SYSCTRL_DEEPSLEEP,false)) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - ADC power on failed:%d\n",ret));

    //step 4: deInitialize the ADC
    if((ret = am_hal_adc_deinitialize(g_ADCHandle)) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - return of the ADC instance failed:%d\n",ret));

    g_ADCHandle =NULL;
}

//*****************************************************************************
//函数功能：打开一个ADC通道
// 输入参数：module：module id,同时也是slot ID,因此值需小于8，最多支持8个slot
// 			adc_callback: callback函调函数
// 返回：	无
//*****************************************************************************
ret_type SMDrv_ADC_Open(adc_module module,adc_cb adc_callback)
{
    am_hal_adc_slot_config_t ADCSlotConfig;
    // Define the ADC  pin to be used.
    am_hal_gpio_pincfg_t t_AdcConfig ={0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint32 adc_pin;
    uint32 ret;

    if(module >= MAX_ADC_MODULE)
    {
        Err_Info((0,"Error - ADC module error.\n"));
        return Ret_InvalidParam;
    }
    //step 1: get module's slot/pin info
    if(adc_modul2info(module,&ADCSlotConfig,&adc_pin) == FALSE)
    {
        Err_Info((0,"Error - ADC Slot pin error.\n"));
        return Ret_InvalidParam;
    }

    //step 2: Set a pin to act as our ADC input
    t_AdcConfig.uFuncSel  = 0;  //0 is ADC function sel
    if((ret = am_hal_gpio_pinconfig(adc_pin, t_AdcConfig)) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - configuring ADC pin failed:%d\n",ret));

    //step 3: Set up an ADC slot,set callback
    ADCSlotConfig.bWindowCompare  = false;
    ADCSlotConfig.bEnabled        = true;
    if((ret = am_hal_adc_configure_slot(g_ADCHandle, module, &ADCSlotConfig)) != AM_HAL_STATUS_SUCCESS)
    {
        Err_Info((0,"Error - configuring ADC Slot 0 failed=%d\n",ret));
        return Ret_Fail;
    }

    if(adc_callback != NULL)
    {
        adc_slot_cb[module] = adc_callback;
    }
    return Ret_OK;
}

//*****************************************************************************
//函数功能：关闭一个打开的ADC通道
// 输入参数：module：
// 返回：	无
//*****************************************************************************
ret_type SMDrv_ADC_Close(adc_module module)
{
    am_hal_adc_slot_config_t ADCSlotConfig;
    // Define the ADC  pin to be used.
    am_hal_gpio_pincfg_t t_AdcConfig ={0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint32 adc_pin;
    uint32 ret;

    if(g_ADCHandle == NULL)
    {
        Err_Info((0,"Error - ADC handle is NULL\n"));
        return Ret_InvalidParam;
    }
    if(module >= MAX_ADC_MODULE)
    {
        Err_Info((0,"Error - ADC module error.\n"));
        return Ret_InvalidParam;
    }
        
    //step 1: get module's slot/pin info
    if(adc_modul2info(module,&ADCSlotConfig,&adc_pin) == FALSE)
    {
        Err_Info((0,"Error - ADC Slot pin error.\n"));
        return Ret_InvalidParam;
    }
        
    //step 2: disable adc pin
    t_AdcConfig.uFuncSel       = 3;
    t_AdcConfig.eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_2MA;
    t_AdcConfig.eGPOutcfg      = AM_HAL_GPIO_PIN_OUTCFG_DISABLE;
    if((ret = am_hal_gpio_pinconfig(adc_pin, t_AdcConfig)) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - configuring ADC pin failed:%d\n",ret));
        
    //step 3: Set up an ADC slot,set callback
    ADCSlotConfig.bWindowCompare  = false;
    ADCSlotConfig.bEnabled        = false;     //disable ADC
    if((ret = am_hal_adc_configure_slot(g_ADCHandle, module, &ADCSlotConfig)) != AM_HAL_STATUS_SUCCESS)
    {
        Err_Info((0,"Error - configuring ADC Slot 0 failed=%d\n",ret));
        return Ret_Fail;
    }

    adc_slot_cb[module] = NULL;
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
// 输入参数：adc_slot：   adc 通道号,值小于 8
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

#endif

