//*****************************************************************************
//
//! @file am_devices_em9304.h
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
#ifndef AM_DEVICES_EM9304_H
#define AM_DEVICES_EM9304_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "platform_common.h"

//*****************************************************************************
//
// EM9304 device structure
//
//*****************************************************************************
typedef struct
{
    //
    // MODE UART vs IOM SPI
    //
    uint32_t ui32Mode;

    //
    // IOM Module #
    //
    uint32_t ui32IOMModule;

    //
    // IOM Chip Select NOTE: this driver uses GPIO for chip selects
    //
    uint32_t ui32IOMChipSelect;

    //
    // GPIO # for EM9304 DREADY signal
    //
    uint32_t ui32DREADY;
}
am_devices_em9304_t;

extern const am_devices_em9304_t g_sEm9304;
#if defined(AM_PART_APOLLO) || defined(AM_PART_APOLLO2)

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
extern void Drv_EM9304_WriteBlock(uint8 type,uint8 *pui8Values,uint32 ui32NumBytes);

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
extern uint32 Drv_EM9304_ReadBlock(uint32 *pui32Values,uint32 ui32NumBytes);

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
extern void Drv_EM9304_SpiInit(void);

//*****************************************************************************
//function: set data ready pin isr callback
//*****************************************************************************
void Drv_EM9304_SetCallBack(comm_cb *cts_cb);

//*****************************************************************************
//function: set data ready pin isr callback
//*****************************************************************************
extern void Drv_EM9304_SetWakeCallBack(comm_cb wake_cb);

//*****************************************************************************
//function: enable/disable EM9304 chip
// u8Enable =1: enable  =0:disable 
//*****************************************************************************
extern void Drv_EM9304_EnableChip(uint8 u8Enable);

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
extern void Drv_EM9304_WakeUp(void);

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
extern void Drv_EM9304_Sleep(void);

extern void Drv_EM9304_EnableInterrupt(void);
extern void Drv_EM9304_DisableInterrupt(void);

extern void Drv_EM9304_Set32Khz(void);
extern uint32_t Drv_EM9304_GetChipPN(void);

#endif // defined(AM_PART_APOLLO) || defined(AM_PART_APOLLO2)

#if defined(AM_PART_APOLLO3)

typedef enum
{
    AM_DEVICES_EM9304_STATUS_SUCCESS,
    AM_DEVICES_EM9304_STATUS_ERROR
} am_devices_em9304_status_t;


extern /*const*/ am_hal_iom_config_t g_sEm9304IOMConfigSPI;

extern uint32_t am_devices_em9304_spi_init(uint32_t ui32Module, am_hal_iom_config_t *psIomConfig);
extern uint32_t am_devices_em9304_fullduplex(uint8_t *pui8TxBuffer,
                                             uint32_t ui32WriteAddress,
                                             uint32_t ui32TxNumBytes,
                                             am_hal_iom_callback_t pfnTxCallback,
                                             uint8_t *pui8RxBuffer,
                                             uint32_t ui32ReadAddress,
                                             uint32_t ui32RxNumBytes,
                                             am_hal_iom_callback_t pfnRxCallback);
extern void am_devices_em9304_fullduplex_ends(void);

#endif // defined(AM_PART_APOLLO3)

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_EM9304_H
