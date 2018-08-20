/**********************************************************************
**
**模块说明: 对接MCU SPI驱动接口，并根据项目原理图作配置
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.8.16  初版  ZSL 
**
**********************************************************************/
//在此模块中使用pdm IO口，因此需定义PDM_MODULE
#define PDM_MODULE
#include "io_config.h"

#if((PDM_CLK != IO_UNKNOW) && (PDM_DATA != IO_UNKNOW))
#include "am_mcu_apollo.h"
#include "sm_sys.h"
#include "sm_pdm.h"

#define PDM_BUF_SIZE                256

#ifdef AM_PART_APOLLO3
void *PDMHandle = NULL;
uint32_t g_ui32PDMDataBuffer[PDM_BUF_SIZE];

// PDM interrupt handler.
void am_pdm_isr(void)
{
    uint32_t ui32Status;

    // Read the interrupt status.
    am_hal_pdm_interrupt_status_get(PDMHandle, &ui32Status, true);
    am_hal_pdm_interrupt_clear(PDMHandle, ui32Status);

    // Once our DMA transaction completes, we will disable the PDM and send a
    // flag back down to the main routine. Disabling the PDM is only necessary
    // because this example only implemented a single buffer for storing FFT
    // data. More complex programs could use a system of multiple buffers to
    // allow the CPU to run the FFT in one buffer while the DMA pulls PCM data
    // into another buffer.
    //
    if (ui32Status & AM_HAL_PDM_INT_DCMP)
    {
        //am_hal_pdm_disable(PDMHandle);
        //g_bPDMDataReady = true;
    }
}

//**********************************************************************
// 函数功能: 配置PDM pin
// 输入参数：	
// 返回参数：Ret_InvalidParam或Ret_OK
//**********************************************************************
static void smdrv_pdm_pinconfig(void)
{
    // Configure the necessary pins.
    am_hal_gpio_pincfg_t sPinCfg = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    if(PDM_CLK == 10)
        sPinCfg.uFuncSel = AM_HAL_PIN_10_PDMCLK;
    else if(PDM_CLK == 12)
        sPinCfg.uFuncSel = AM_HAL_PIN_12_PDMCLK;
    else if(PDM_CLK == 14)
        sPinCfg.uFuncSel = AM_HAL_PIN_14_PDMCLK;
    else if(PDM_CLK == 22)
        sPinCfg.uFuncSel = AM_HAL_PIN_22_PDMCLK;
    else if(PDM_CLK == 37)
        sPinCfg.uFuncSel = AM_HAL_PIN_37_PDMCLK;
    else if(PDM_CLK == 46)
        sPinCfg.uFuncSel = AM_HAL_PIN_46_PDMCLK;
    am_hal_gpio_pinconfig(PDM_CLK, sPinCfg);

    if(PDM_CLK == 11)
        sPinCfg.uFuncSel = AM_HAL_PIN_11_PDMDATA;
    else if(PDM_CLK == 15)
        sPinCfg.uFuncSel = AM_HAL_PIN_15_PDMDATA;
    else if(PDM_CLK == 29)
        sPinCfg.uFuncSel = AM_HAL_PIN_29_PDMDATA;
    else if(PDM_CLK == 34)
        sPinCfg.uFuncSel = AM_HAL_PIN_34_PDMDATA;
    else if(PDM_CLK == 36)
        sPinCfg.uFuncSel = AM_HAL_PIN_36_PDMDATA;
    else if(PDM_CLK == 45)
        sPinCfg.uFuncSel = AM_HAL_PIN_45_PDMDATA;
    am_hal_gpio_pinconfig(PDM_DATA, sPinCfg);
}

//**********************************************************************
// 函数功能: open pdm
// 输入参数：	
// 返回参数：Ret_InvalidParam或Ret_OK
//**********************************************************************
ret_type SMDrv_PDM_Open(void)
{
    uint32 u32IntMask;
    am_hal_pdm_config_t g_sPdmConfig =
    {
        .eClkDivider = AM_HAL_PDM_MCLKDIV_1,
        .eLeftGain = AM_HAL_PDM_GAIN_P405DB,
        .eRightGain = AM_HAL_PDM_GAIN_P405DB,
        .ui32DecimationRate = 0x30,
        .bHighPassEnable = 0,
        .ui32HighPassCutoff = 0xB,
        .ePDMClkSpeed = AM_HAL_PDM_CLK_6MHZ,
        .bInvertI2SBCLK = 0,
        .ePDMClkSource = AM_HAL_PDM_INTERNAL_CLK,
        .bPDMSampleDelay = 0,
        .bDataPacking = 1,
        .ePCMChannels = AM_HAL_PDM_CHANNEL_RIGHT,
        .bLRSwap = 0,
    };

    //step 1: Initialize, power-up, and configure the PDM.
    am_hal_pdm_initialize(0, &PDMHandle);
    am_hal_pdm_power_control(PDMHandle, AM_HAL_PDM_POWER_ON, false);
    am_hal_pdm_configure(PDMHandle, &g_sPdmConfig);
    am_hal_pdm_enable(PDMHandle);
    
    //step 12: Configure and enable PDM interrupts (set up to trigger on DMA completion).
    u32IntMask = AM_HAL_PDM_INT_DERR | AM_HAL_PDM_INT_DCMP | AM_HAL_PDM_INT_UNDFL | AM_HAL_PDM_INT_OVF;
    am_hal_pdm_interrupt_enable(PDMHandle, u32IntMask);
    SMDrv_SYS_EnableIsr(INTERRUPT_PDM);

    am_hal_pdm_fifo_flush(PDMHandle);

    return Ret_OK;
}

//**********************************************************************
// 函数功能: start pdm
// 输入参数：	
// 返回参数：Ret_InvalidParam或Ret_OK
//**********************************************************************
ret_type SMDrv_PDM_Start(void)
{
    // Configure DMA and target address.
    am_hal_pdm_transfer_t sTransfer;
    sTransfer.ui32TargetAddr = (uint32_t )g_ui32PDMDataBuffer;
    sTransfer.ui32TotalCount = PDM_BUF_SIZE;

    // Start the data transfer.
    am_hal_pdm_dma_start(PDMHandle, &sTransfer);
    am_hal_pdm_enable(PDMHandle);
    return Ret_OK;
}

//**********************************************************************
// 函数功能: stop pdm
// 输入参数：	
// 返回参数：Ret_InvalidParam或Ret_OK
//**********************************************************************
ret_type SMDrv_PDM_Stop(void)
{
    am_hal_pdm_disable(PDMHandle);
    return Ret_OK;
}

#else

#endif

#endif

