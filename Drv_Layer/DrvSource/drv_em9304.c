//*****************************************************************************
//
//! @file am_devices_em9304.c
//!
//! @brief Support functions for the EM Micro EM9304 BTLE radio.
//
//*****************************************************************************

//*****************************************************************************
//
// Copyright (c) 2017, Ambiq Micro
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is part of revision 1.2.11 of the AmbiqSuite Development Package.
//
//*****************************************************************************
#define BLE_MODULE

#include "io_config.h"
#include "sm_spi.h"
#include "sm_gpio.h"
#include "string.h"
#include "sm_sys.h"

#include "am_mcu_apollo.h"
#include "drv_em9304.h"

#define DRV_EM9304_DEBUG    0    //Add for print debug info

#if(DRV_EM9304_DEBUG == 1)
#include "SEGGER_RTT.h"
#include "SEGGER_RTT_Conf.h"
#endif
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

#define USE_IOM_NONBLOCKING    1

// SPI header byte
#define EM9304_SPI_HEADER_TX                    0x42
#define EM9304_SPI_HEADER_RX                    0x81
#define EM9304_STS1_READY_VALUE                 0xC0

// Length of RX buffer
#define EM9304_BUFFER_SIZE                      256

//定义EM9304接收数据触发callback
static comm_cb *em9403_data_cb = NULL;

// Select the EM9304 -> Set the CSN to low level
static void EM9304_SPISLAVESELECT(void)
{
    SMDrv_GPIO_BitClear(BLE_CS_PIN);
}

// Deselect the EM9304 -> Set the CSN to high level
static void EM9304_SPISLAVEDESELECT(void)
{
    SMDrv_GPIO_BitSet(BLE_CS_PIN);
}

// Indicates the EM9304 RDY pin state
static uint32 EM9304_RDY_INT(void)
{
    return SMDrv_GPIO_InBitRead(BLE_RDY_PIN);
}

#if defined(AM_PART_APOLLO) || defined(AM_PART_APOLLO2)

// SPI lock when a transmission is in progress
static uint8 spiTxInProgress;

static volatile bool gIomDone = false;

//*****************************************************************************
//
// BLE MAC address for the EM radio.
//
//*****************************************************************************
//uint8 g_BLEMacAddress[6] =
//{0x01, 0x00, 0x00, 0xCA, 0xEA, 0x80};


#if(USE_IOM_NONBLOCKING == 1)
//*****************************************************************************
//
// IOM write complete callback
//
//*****************************************************************************
static void iom_write_complete(void)
{
    gIomDone = true;
}
#endif

#if defined(AM_PART_APOLLO2)
//*****************************************************************************
//
// Checks to see if this processor is a Rev B2 device.
//
//*****************************************************************************
static bool isRevB2(void)
{
    // Check to make sure the major rev is B and the minor rev is zero.
    if ( (AM_REG(MCUCTRL, CHIPREV) & 0xFF) ==
        (AM_REG_MCUCTRL_CHIPREV_REVMAJ_B | AM_REG_MCUCTRL_CHIPREV_REVMIN_REV2) )
    {
        return true;
    }
    else
    {
        return false;
    }
}
#endif

//*****************************************************************************
//
//! @brief Start a transmit transaction to the EM9304.
//!
//! @param psDevice is a pointer to a device structure describing the EM9304.
//!
//! This function handles the initial handshake of asserting the SPI CE and
//! then waiting for the RDY signal from the EM.  It then executes a transmit
//! command to the EM9304 and receives the number of bytes that can be accepted.
//!
//! @return Number of bytes that can be written to EM9304.
//
//*****************************************************************************
static uint8 em9304_tx_starts(void)
{
    am_hal_iom_buffer(2) sCommand;
    am_hal_iom_buffer(2) sStas;
    uint32 ui32Module = SMDrv_SPI_GetModuleId(BLE_SPI_MODULE);

    // Indicates that a SPI transfer is in progress
    spiTxInProgress = 1;

    sCommand.bytes[0] = EM9304_SPI_HEADER_TX;
    sCommand.bytes[1] = 0x0;

    // Select the EM9304
    EM9304_SPISLAVESELECT();

    // Wait EM9304 RDY signal
    while ( !EM9304_RDY_INT() );

#if defined(AM_PART_APOLLO2)
    // Full-Duplex operation is only supported for Apollo2 B2 Silicon.
    if (isRevB2())
    {
        // Write to the IOM.
        am_hal_iom_spi_fullduplex(ui32Module,0, sCommand.words, sStas.words, 2, AM_HAL_IOM_RAW);

        // Check that the EM9304 is ready.
        if (sStas.bytes[0] != EM9304_STS1_READY_VALUE)
        {
            // Error
            EM9304_SPISLAVEDESELECT();
#if(DRV_EM9304_DEBUG ==1)
           SEGGER_RTT_printf(0,"HCI TX Failed to starts\n");
           SEGGER_RTT_printf(0,"%d, %d\n", sStas.bytes[0], sStas.bytes[1]);
#endif
            return 0;
        }
        return sStas.bytes[1];
    }
    else
#endif
    {
        // Write to the IOM.
        am_hal_iom_spi_write(ui32Module,0, sCommand.words, 1, AM_HAL_IOM_RAW);
        // Read from the IOM.
        am_hal_iom_spi_read(ui32Module,0, sStas.words, 1, AM_HAL_IOM_RAW);

        return sStas.bytes[0];
    }
}

//*****************************************************************************
//
//! @brief End a transmit transaction to the EM9304.
//!
//! @param None.
//!
//! This function handles the completion of a transmit to the EM9304.  After
//! the IOM has completed the transaction, the CE is deasserted and the RDY
//! interrupt is renabled.
//!
//! @return None.
//
//*****************************************************************************
static void em9304_tx_ends(void)
{
    am_hal_iom_poll_complete(SMDrv_SPI_GetModuleId(BLE_SPI_MODULE));
    // Deselect the EM9304
    EM9304_SPISLAVEDESELECT();

    // Indicates that the SPI transfer is finished
    spiTxInProgress = 0;
}

//*****************************************************************************
//
//! @brief EM9304 write function.
//!
//! @param type is the HCI command.
//! @param pui8Values is the HCI packet to send.
//! @param ui32NumBytes is the number of bytes to send (including HCI command).
//!
//! This function perform a write transaction to the EM9304.
//!
//! @return None.
//
//*****************************************************************************
void Drv_EM9304_WriteBlock(uint8 type,uint8 *pui8Values,uint32 ui32NumBytes)
{
    am_hal_iom_buffer(EM9304_BUFFER_SIZE) sData;
    uint8 em9304BufSize = 0;
    uint8 hci_type_sent = 0;
    uint32 ui32Module = SMDrv_SPI_GetModuleId(BLE_SPI_MODULE);

    // Check that we are not going to overwrite buffer.
    if ((ui32NumBytes + 1) > EM9304_BUFFER_SIZE)
    {        
#if(DRV_EM9304_DEBUG ==1)
        SEGGER_RTT_printf(0,"HCI TX Error (STATUS ERROR) Packet Too Large\n");
#endif
        return;
    }

#if(DRV_EM9304_DEBUG ==1)
    SEGGER_RTT_printf(0,"em write data=%d:\n",ui32NumBytes);
    for ( uint32 j = 0; j < ui32NumBytes; j++)
    {
        SEGGER_RTT_printf(0,"0x%x ",pui8Values[j]);
    }    
    SEGGER_RTT_printf(0,"\n");
#endif

    for ( uint32 i = 0; i < ui32NumBytes; )
    {
        if ( i < ui32NumBytes )
        {
            em9304BufSize = em9304_tx_starts();
        }

        uint32 len = (em9304BufSize < (ui32NumBytes - i)) ? em9304BufSize : (ui32NumBytes - i);

#if(DRV_EM9304_DEBUG ==1)
        SEGGER_RTT_printf(0,"len=%d,em9304BufSize =%d\n",len,em9304BufSize);
#endif

        if (len > 0)  // check again if there is room to send more data
        {
            if (hci_type_sent == 0) 
            {
                sData.bytes[0] = type;
                memcpy(&(sData.bytes[1]), pui8Values, len - 1);
                i += len -1;
                hci_type_sent = 1;
            }
            else 
            {
                memcpy(&(sData.bytes[0]), pui8Values + i, len);
                i += len;
            }

            while ( !EM9304_RDY_INT() );

#if(USE_IOM_NONBLOCKING == 1)
            gIomDone = false;
            am_hal_iom_spi_write_nb(ui32Module,0, sData.words, len,AM_HAL_IOM_RAW,iom_write_complete);
            while(1)
            {
                // Disable interrupt while we decide whether we're going to sleep.
                uint32 ui32IntStatus = am_hal_interrupt_master_disable();

                if (!gIomDone)
                {
                    // Sleep while waiting for the IOM transaction to finish.
                    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
                    // Enable interrupts
                    am_hal_interrupt_master_set(ui32IntStatus);
                }
                else
                {
                    // Enable interrupts
                    am_hal_interrupt_master_set(ui32IntStatus);
                    break;
                }
            }
#else
        am_hal_iom_spi_write(ui32Module,0, sData.words, len,AM_HAL_IOM_RAW);
#endif
        }
        em9304_tx_ends();
    }
}

//*****************************************************************************
//
//! @brief EM9304 read function.
//!
//! @param pui8Values is the buffer to receive the HCI packet.
//! @param ui32NumBytes is the number of bytes to send (including HCI command).
//!
//! This function a read transaction from the EM9304.
//!
//! @return Number of bytes read.
//
//*****************************************************************************
uint32 Drv_EM9304_ReadBlock(uint32 *pui32Values,uint32 ui32NumBytes)
{
    am_hal_iom_buffer(2) sCommand;
    am_hal_iom_buffer(2) sStas;
    uint8 ui8RxBytes;
    uint8 spiRxTotalBytesCount = 0;  
    uint32 ui32Module = SMDrv_SPI_GetModuleId(BLE_SPI_MODULE);

    sCommand.bytes[0] = EM9304_SPI_HEADER_RX;
    sCommand.bytes[1] = 0x0;

    // Check if the SPI is free
    if ( spiTxInProgress )
    {
        // TX in progress -> Ignore RDY interrupt        
#if(DRV_EM9304_DEBUG ==1)
        SEGGER_RTT_printf(0,"HCI TX in progress\n");
#endif
        return 0;
    }

    // Check if they are still data to read
    if ( !EM9304_RDY_INT())
    {
        // No data        
#if(DRV_EM9304_DEBUG ==1)
        SEGGER_RTT_printf(0,"HCI No data\n");
#endif
        return 0;
    }

    // Select the EM9304
    EM9304_SPISLAVESELECT();

#if defined(AM_PART_APOLLO2)
    // Full-Duplex operation is only supported for Apollo2 B2 Silicon.
    if (isRevB2())
    {
        // Write to the IOM.
        am_hal_iom_spi_fullduplex(ui32Module,0, sCommand.words, sStas.words, 2,AM_HAL_IOM_RAW);
    
        // Check that the EM9304 is ready.
        if ( sStas.bytes[0] != EM9304_STS1_READY_VALUE )
        {
            // Error
            EM9304_SPISLAVEDESELECT();            
#if(DRV_EM9304_DEBUG ==1)
            SEGGER_RTT_printf(0,"HCI RX Error (STATUS ERROR) EM9304 Not Ready\n");
            SEGGER_RTT_printf(0,"%d, %d\n", sStas.bytes[0], sStas.bytes[1]);
#endif
            return 0;
        }
    
        // Set the number of bytes to receive.
        ui8RxBytes = sStas.bytes[1];
    }
    else
#endif
    {
        // Write to the IOM.
        am_hal_iom_spi_write(ui32Module,0, sCommand.words, 1,AM_HAL_IOM_RAW);
    
        // Read from the IOM.
        am_hal_iom_spi_read(ui32Module,0, sStas.words, 1,AM_HAL_IOM_RAW);
    
        // Set the number of bytes to receive.
        ui8RxBytes = sStas.bytes[0];
    }

    while ( EM9304_RDY_INT() && (spiRxTotalBytesCount < ui8RxBytes) && (ui8RxBytes != 0))
    {
        uint32 len = 1;
  
        if ( (ui8RxBytes - spiRxTotalBytesCount) >= EM9304_BUFFER_SIZE )
        {
            // Error. Packet too large.
#if(DRV_EM9304_DEBUG ==1)
            SEGGER_RTT_printf(0,"HCI RX Error (STATUS ERROR) Packet Too Large\n");
            SEGGER_RTT_printf(0,"%d, %d\n", sStas.bytes[0], sStas.bytes[1]);
#endif
            return 0;
        }
        else
        {
            len = ui8RxBytes;
        }
  
#if(USE_IOM_NONBLOCKING == 1)
        gIomDone = false;
        am_hal_iom_spi_read_nb(ui32Module,0, pui32Values, len,AM_HAL_IOM_RAW,iom_write_complete);
        while(1)
        {
            // Disable interrupt while we decide whether we're going to sleep.
            uint32 ui32IntStatus = am_hal_interrupt_master_disable();
      
            if (!gIomDone)
            {
                // Sleep while waiting for the IOM transaction to finish.
                am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
                // Enable interrupts
                am_hal_interrupt_master_set(ui32IntStatus);
            }
            else
            {
                // Enable interrupts
                am_hal_interrupt_master_set(ui32IntStatus);
                break;
            }
        }
#else
      am_hal_iom_spi_read(ui32Module,0, pui32Values,len, AM_HAL_IOM_RAW);
#endif
      spiRxTotalBytesCount = len;
    }

#if (DRV_EM9304_DEBUG ==1)
    SEGGER_RTT_printf(0,"em read data=%d:\n",spiRxTotalBytesCount);
    for ( uint32 j = 0; j < spiRxTotalBytesCount; j++)
    {
        SEGGER_RTT_printf(0,"0x%x ",pui32Values[j]);
    }
    SEGGER_RTT_printf(0,"\n");
#endif

    // Deselect the EM9304
    EM9304_SPISLAVEDESELECT();
    return spiRxTotalBytesCount;
}

uint32_t Drv_EM9304_GetChipPN(void)
{
    // Device identification
    return AM_REG(MCUCTRL, CHIP_INFO);
}

void Drv_EM9304_Set32Khz(void)
{
    // GPIO 41 in Apollo2-blue connected to LFCLK in EM9304
    if(BLE_32KHz != IO_UNKNOW)
    {
        am_hal_gpio_pin_config(BLE_32KHz, AM_HAL_PIN_41_CLKOUT);
        am_hal_clkgen_osc_start(AM_HAL_CLKGEN_OSC_XT);
        SMDrv_SYS_DelayMs(500);
        am_hal_clkgen_clkout_enable(AM_HAL_CLKGEN_CLKOUT_CKSEL_XT);
    }
}

//DATA ready pin interrupt isr
void em9304_rdy_isr(uint32 u32PinNum)
{
    //radio_task  am_gpio_isr
    if(u32PinNum == BLE_RDY_PIN)
    {
         if(em9403_data_cb != NULL)
         {
            (em9403_data_cb)();
         }
    }
}

//*****************************************************************************
//
//! @brief EM9304 SPI/IOM initialization function and module pin config.
//!
//! @param ui32Module is the IOM module to be used for EM9304.
//! @param psIomConfig is the configuration information for the IOM.
//!
//! This function initializes the IOM for operation with the EM9304.
//!
//! @return None.
//*****************************************************************************
void Drv_EM9304_SpiInit(void)
{
    uint32 u32pin_config = AM_HAL_PIN_INPUT;
    uint32 ui32Module = SMDrv_SPI_GetModuleId(BLE_SPI_MODULE);
    if ( AM_REGn(IOMSTR, ui32Module, CFG) & AM_REG_IOMSTR_CFG_IFCEN_M )
    {
        return;
    }

    //config rst/en pin,and output log
    SMDrv_GPIO_Open(BLE_EN_PIN,NULL,NULL);
    SMDrv_GPIO_BitClear(BLE_EN_PIN);

    //open spi for EM9304
    SMDrv_SPI_Open(BLE_SPI_MODULE);
    //config CS pin as output, and output high
    SMDrv_GPIO_Open(BLE_CS_PIN,NULL,NULL);
    SMDrv_GPIO_BitSet(BLE_CS_PIN);

    //config RDY pin as output
    SMDrv_GPIO_Open(BLE_RDY_PIN,&u32pin_config,em9304_rdy_isr); 

    //set GPIO interrupt prio,不设中断优先级，中断不能正常 
    SMDrv_GPIO_SetIrqPrio(2);

#if (USE_IOM_NONBLOCKING == 1)
    // Reset the IOM Done flag.
    gIomDone = false;
#endif

    //delay 200ms to reset EM9304
    SMDrv_SYS_DelayMs(200);

    // Enable the IOM and GPIO interrupt handlers.
    SMDrv_GPIO_BitSet(BLE_EN_PIN);
}

//*****************************************************************************
//function: set data ready pin isr callback
//*****************************************************************************
void Drv_EM9304_SetCallBack(comm_cb *cts_cb)
{
    if(cts_cb != NULL)
    {
        em9403_data_cb = cts_cb;
    }
}

//*****************************************************************************
//function: enable/disable EM9304 chip
// u8Enable =1: enable  =0:disable 
//*****************************************************************************
void Drv_EM9304_EnableChip(uint8 u8Enable)
{
    SMDrv_GPIO_Open(BLE_EN_PIN,NULL,NULL);
    if(u8Enable == 1)
    {
        // Enable the IOM and GPIO interrupt handlers.
        SMDrv_GPIO_BitSet(BLE_EN_PIN);
    }
    else
    {
        //disable em9304 enable pin
        SMDrv_GPIO_BitClear(BLE_EN_PIN);
    }
}

//*****************************************************************************
//
//! @brief EM9304 SPI/IOM wakeup function.
//!
//! @param ui32Module is the IOM module to be used for EM9304.
//!
//! This function restores the IOM operation after sleep.
//!
//! @return None.
//
//*****************************************************************************
void Drv_EM9304_WakeUp(void)
{
    uint32 ui32Module = SMDrv_SPI_GetModuleId(BLE_SPI_MODULE);
    
    if (AM_REGn(IOMSTR, ui32Module, CFG) & AM_REG_IOMSTR_CFG_IFCEN_M )
    {
        return;
    }

    SMDrv_SPI_Wake(BLE_SPI_MODULE); //awke spi for EM9304
}

//*****************************************************************************
//
//! @brief EM9304 SPI/IOM sleep function.
//!
//! @param ui32Module is the IOM module to be used for EM9304.
//!
//! This function prepares the IOM for sleep.
//!
//! @return None.
//
//*****************************************************************************
void Drv_EM9304_Sleep(void)
{
    uint32 ui32Module = SMDrv_SPI_GetModuleId(BLE_SPI_MODULE);

    if (!(AM_REGn(IOMSTR, ui32Module, CFG) & AM_REG_IOMSTR_CFG_IFCEN_M) )
    {
        return;
    }
    SMDrv_SPI_Sleep(BLE_SPI_MODULE);
}
    
void Drv_EM9304_EnableInterrupt(void)
{
    SMDrv_GPIO_EnableInterrupt(BLE_RDY_PIN,TRUE);
}
    
void Drv_EM9304_DisableInterrupt(void)
{
    SMDrv_GPIO_EnableInterrupt(BLE_RDY_PIN,FALSE);
}

#endif // defined(AM_PART_APOLLO) || defined(AM_PART_APOLLO2)


#if defined(AM_PART_APOLLO3)

//*****************************************************************************
//
// Global variables.
//
//*****************************************************************************
extern void *g_pIOMHandle;

//*****************************************************************************
//
// IOM SPI Configuration for EM9304
//
//*****************************************************************************
/*const*/ am_hal_iom_config_t g_sEm9304IOMConfigSPI =
{
    .eInterfaceMode     = AM_HAL_IOM_SPI_MODE,
    .ui32ClockFreq      = AM_HAL_IOM_8MHZ,
    .eSpiMode           = AM_HAL_IOM_SPI_MODE_0,
    .ui32WrThreshold    = 20,
    .ui32RdThreshold    = 20,
    .bFullDuplex        = true,
    .bLSBfirst          = false,
};

//*****************************************************************************
//
// EM9304 Device Configuration
//
//*****************************************************************************
const am_devices_em9304_t g_sEm9304 =
{
    .ui32IOMModule      = AM_BSP_EM9304_IOM,
    .ui32IOMChipSelect  = BLE_CS_PIN,
    .ui32DREADY         = BLE_RDY_PIN
};

//*****************************************************************************
//
//! @brief Configure EM9304 pins.
//!
//! @param None.
//!
//! This function initializes the GPIOs for communication with the EM9304.
//!
//! @return None.
//
//*****************************************************************************
void
am_devices_em9304_config_pins(void)
{
    am_bsp_pin_enable(EM9304_CS);
    am_bsp_pin_enable(EM9304_INT);

    am_hal_gpio_out_bit_set(BLE_CS_PIN);

    am_hal_gpio_int_polarity_bit_set(BLE_RDY_PIN, AM_HAL_GPIO_RISING);
    am_hal_gpio_int_clear(AM_HAL_GPIO_BIT(BLE_RDY_PIN));
    am_hal_gpio_int_enable(AM_HAL_GPIO_BIT(BLE_RDY_PIN));
}

//*****************************************************************************
//
//! @brief EM9304 SPI/IOM initialization function.
//!
//! @param ui32Module is the IOM module to be used for EM9304.
//! @param psIomConfig is the configuration information for the IOM.
//!
//! This function initializes the IOM for operation with the EM9304.
//!
//! @return None.
//
//*****************************************************************************
uint32

am_devices_em9304_spi_init(uint32 ui32Module, am_hal_iom_config_t *psIomConfig)
{
    //
    // Fill in the config structure with some device specific items
    //
    psIomConfig->bFullDuplex   = true;
    psIomConfig->bLSBfirst     = false;
    psIomConfig->bI2C10bitAddr = false;

    //
    // Enable power to the IOM instance.
    //
#if AM_PART_APOLLO3
    am_hal_pwrctrl_periph_enable((am_hal_pwrctrl_periph_e)
                                 (AM_HAL_PWRCTRL_PERIPH_IOM0 + ui32Module));
#else
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_IOM0 << ui32Module);
#endif

    //
    // Configure the IOM pins.
    //
    am_bsp_iom_pins_enable(ui32Module, AM_HAL_IOM_SPI_MODE);

    //
    // Configure the EM9304 pins
    //
    am_devices_em9304_config_pins();

    //
    // Enable fault detection.
    //
    am_hal_mcuctrl_fault_capture_enable();

    //
    // Initialize the IOM instance.
    //
    am_hal_iom_Initialize(ui32Module, &g_pIOMHandle);

    //
    // Configure the IOM for Serial operation during initialization.
    //
    am_hal_iom_Configure(g_pIOMHandle, psIomConfig);

    //
    // Enable the IOM.
    //
    am_hal_iom_Enable(&g_pIOMHandle);

    //
    // Return the status.
    //
    return AM_DEVICES_EM9304_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief End a transmit transaction to the EM9304.
//!
//! @param None.
//!
//! This function handles the completion of a transmit to the EM9304.  After
//! the IOM has completed the transaction, the CE is deasserted and the RDY
//! interrupt is renabled.
//!
//! @return None.
//
//*****************************************************************************
void
am_devices_em9304_fullduplex_ends(void)
{
    //am_hal_iom_poll_complete(g_sEm9304.ui32IOMModule);

    // Deselect the EM9304
    EM9304_SPISLAVEDESELECT();

    // Indicates that the SPI transfer is finished
    //spiTxInProgress = 0;
}

//*****************************************************************************
//
//! @brief Apollo3 full duplex SPI transaction.
//!
//! @param None.
//!
//! This function handles a full duplex transaction on the EM9304.
//!
//! @return None.
//
//*****************************************************************************
uint32 am_devices_em9304_fullduplex(uint8 *pui8TxBuffer,
                             uint32 ui32WriteAddress,
                             uint32 ui32TxNumBytes,
                             am_hal_iom_callback_t pfnTxCallback,
                             uint8 *pui8RxBuffer,
                             uint32 ui32ReadAddress,
                             uint32 ui32RxNumBytes,
                             am_hal_iom_callback_t pfnRxCallback)
{
    am_hal_iom_spifullduplex_transfer_t Transaction;

    //
    // Set up the full-duplex transaction.
    //
    Transaction.ui32ChipSelect      = g_sEm9304.ui32IOMChipSelect;
    Transaction.bTxContinue         = false;
    Transaction.ui32TxInstrLen      = 0;
    Transaction.ui32TxInstr         = ((ui32WriteAddress & 0x0000FFFF) << 8);
    Transaction.ui32TxXferCnt       = ui32TxNumBytes;
    Transaction.ui32TxSRAMAddr      = (uint32)pui8TxBuffer;

    Transaction.bRxContinue         = false;
    Transaction.ui32RxInstrLen      = 0;
    Transaction.ui32RxInstr         = ((ui32WriteAddress & 0x0000FFFF) << 8);
    Transaction.ui32RxXferCnt       = ui32RxNumBytes;
    Transaction.ui32RxSRAMAddr      = (uint32)pui8RxBuffer;

//  am_devices_em9304_fullduplex_starts(g_sEm9304);

    // Select the EM9304
    EM9304_SPISLAVESELECT();

    // Wait until ready (RDY goes high)
    while ( !EM9304_RDY_INT() );

    //
    // Do the transaction.
    // Header information is assumed to be in the TX buffer.
    //
    am_hal_iom_spi_fullduplex(g_pIOMHandle,
                              &Transaction,
                              pfnTxCallback,
                              pfnRxCallback);
    //
    // Return the status.
    //
    return 0;
}
#endif // defined(AM_PART_APOLLO3)

