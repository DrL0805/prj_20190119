/**********************************************************************
**
**ģ��˵��: �Խ�MCU SPI�����ӿڣ���������Ŀԭ��ͼ������
**����汾���޸���־(ʱ�䣬����),�޸���:
**   V1.0   2018.8.16  ����  ZSL 
**
**********************************************************************/
//�ڴ�ģ����ʹ��pdm IO�ڣ�����趨��PDM_MODULE
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
// ��������: ����PDM pin
// ���������	
// ���ز�����Ret_InvalidParam��Ret_OK
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
// ��������: open pdm
// ���������	
// ���ز�����Ret_InvalidParam��Ret_OK
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
// ��������: start pdm
// ���������	
// ���ز�����Ret_InvalidParam��Ret_OK
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
// ��������: stop pdm
// ���������	
// ���ز�����Ret_InvalidParam��Ret_OK
//**********************************************************************
ret_type SMDrv_PDM_Stop(void)
{
    am_hal_pdm_disable(PDMHandle);
    return Ret_OK;
}

#else

#endif

#endif

