//*****************************************************************************
//
//! @file em9304_init.h
//!
//! @brief Initialization functions for the EM Micro EM9304 BLE radio.
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
#ifndef HCI_EM9304_H
#define HCI_EM9304_H

typedef void (*ble_rdy)(void);

//*****************************************************************************
//
// EM9304 vendor specific events
//
//*****************************************************************************
typedef struct
{
    uint32_t    EM_ActiveStateEntered;
    uint32_t    EM_TestModeEntered;
    uint32_t    EM_HalNotification;
    uint32_t    EM_DebugPrint;
    uint32_t    EM_DebugStackUsage;
    uint32_t    EM_DebugBacktrace;
    uint32_t    EM_DebugAssert;
} g_EMVSEvent_t;

// Tx power level in dBm.
typedef enum
{
  TX_POWER_LEVEL_MINOR_33P5_dBm, // 0, not compliant with BT spec
  TX_POWER_LEVEL_MINOR_29P0_dBm, // 1, not compliant with BT spec
  TX_POWER_LEVEL_MINOR_17P9_dBm, // 2
  TX_POWER_LEVEL_MINOR_16P4_dBm, // 3
  TX_POWER_LEVEL_MINOR_14P6_dBm, // 4
  TX_POWER_LEVEL_MINOR_13P1_dBm, // 5
  TX_POWER_LEVEL_MINOR_11P4_dBm, // 6
  TX_POWER_LEVEL_MINOR_9P9_dBm, // 7
  TX_POWER_LEVEL_MINOR_8P4_dBm, // 8
  TX_POWER_LEVEL_MINOR_6P9_dBm, // 9
  TX_POWER_LEVEL_MINOR_5P5_dBm, // 10
  TX_POWER_LEVEL_MINOR_4P0_dBm, // 11
  TX_POWER_LEVEL_MINOR_2P6_dBm, // 12
  TX_POWER_LEVEL_MINOR_1P4_dBm, // 13
  TX_POWER_LEVEL_PLUS_0P4_dBm, // 14
  TX_POWER_LEVEL_PLUS_2P5_dBm, // 15
  TX_POWER_LEVEL_PLUS_4P6_dBm, // 16
  TX_POWER_LEVEL_PLUS_6P2_dBm, // 17
  TX_POWER_LEVEL_INVALID
}txPowerLevel_t;

//*****************************************************************************
// Set Mac addr.
//*****************************************************************************
extern void hciDrvSetMac(uint8_t *pui8MacAddress);

//*****************************************************************************
// Get Mac addr.
//*****************************************************************************
extern uint8_t *hciDrvGetMac(void);

//*****************************************************************************
//function: set data ready pin isr callback
//*****************************************************************************
extern void HciDrvRadioSetCallBack(ble_rdy cts_cb);

//*****************************************************************************
//
//! @brief Get the EM9304 vendor specific event counters.
//!
//! @return Returns a pointer to the EM9304 vendor specific event counters.
//
//*****************************************************************************
extern g_EMVSEvent_t *getEM9304VSEventCounters(void);

/*************************************************************************************************/
/*!
 *  \fn     HciVsSetRfPowerLevelEx
 *
 *  \brief  Vendor-specific command for settting Radio transmit power level
 *          for EM9304.
 *
 *  \param  txPowerlevel    valid range from 0 to 17 in decimal.
 *
 *  \return true when success, otherwise false
 */
/*************************************************************************************************/
extern bool_t HciVsEM_SetRfPowerLevelEx(txPowerLevel_t txPowerlevel);

/*************************************************************************************************/
/*!
 *  \fn     HciVsEM_TransmitterTest
 *
 *  \brief  Vendor-specific command for start transmitter testing
 *
 *  \param  test_mode       refer to em9304 datasheet
 *  \param  channel_number  refer to em9304 datasheet
 *  \param  packet_len      refer to em9304 datasheet
 *  \param  packet_payload_type    refer to em9304 datasheet
 *
 *  \return None
 */
/*************************************************************************************************/
extern void HciVsEM_TransmitterTest(uint8_t test_mode, uint8_t channel_number, uint8_t packet_len, uint8_t packet_payload_type);

/*************************************************************************************************/
/*!
 *  \fn     HciVsEM_TransmitterTestEnd
 *
 *  \brief  Vendor-specific command for ending Radio transmitter testing.
 *
 *  \param  None    
 *
 *  \return None
 */
/*************************************************************************************************/
extern void HciVsEM_TransmitterTestEnd(void);

//*****************************************************************************
//
// Configure the necessary pins and start the EM9304 radio.
//
//*****************************************************************************
extern void HciDrvRadioBoot(uint32_t ui32UartModule);
extern void HciDrvRadioShutdown(void);

#endif // EM9304_INIT_H
