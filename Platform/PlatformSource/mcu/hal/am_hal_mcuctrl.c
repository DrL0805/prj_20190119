//*****************************************************************************
//
//  am_hal_mcuctrl.c
//! @file
//!
//! @brief Functions for interfacing with the MCUCTRL.
//!
//! @addtogroup mcuctrl3 MCU Control (MCUCTRL)
//! @ingroup apollo3hal
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// Copyright (c) 2018, Ambiq Micro
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
// Third party software included in this distribution is subject to the
// additional license terms as defined in the /docs/licenses directory.
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
// This is part of revision v1.2.12-736-gdf97e703f of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"

//*****************************************************************************
//
// Global Variables.
//
//*****************************************************************************
//
// Define the flash sizes from CHIPPN.
//
const uint32_t
g_am_hal_mcuctrl_flash_size[AM_HAL_MCUCTRL_CHIPPN_FLASH_SIZE_N] =
{
     16 * 1024,             /* 0x0 0x00004000   16 KB */
     32 * 1024,             /* 0x1 0x00008000   32 KB */
     64 * 1024,             /* 0x2 0x00010000   64 KB */
    128 * 1024,             /* 0x3 0x00020000  128 KB */
    256 * 1024,             /* 0x4 0x00040000  256 KB */
    512 * 1024,             /* 0x5 0x00080000  512 KB */
      1 * 1024 * 1024,      /* 0x6 0x00100000    1 MB */
      2 * 1024 * 1024,      /* 0x7 0x00200000    2 MB */
};

const uint32_t
g_am_hal_mcuctrl_sram_size[AM_HAL_MCUCTRL_CHIPPN_SRAM_SIZE_N] =
{
     16 * 1024,             /* 0x0 0x00004000   16 KB */
     32 * 1024,             /* 0x1 0x00008000   32 KB */
     64 * 1024,             /* 0x2 0x00010000   64 KB */
    128 * 1024,             /* 0x3 0x00020000  128 KB */
    256 * 1024,             /* 0x4 0x00040000  256 KB */
    512 * 1024,             /* 0x5 0x00080000  512 KB */
      1 * 1024 * 1024,      /* 0x6 0x00100000    1 MB */
    384 * 1024,             /* 0x7 0x00200000  384 KB */
};

// ****************************************************************************
//
//  device_info_get()
//  Gets all relevant device information.
//
// ****************************************************************************
static void
device_info_get(am_hal_mcuctrl_device_t *psDevice)
{
#if AM_CMSIS_REGS
    //
    // Read the Part Number.
    //
    psDevice->ui32ChipPN = MCUCTRL->CHIPPN;

    //
    // Read the Chip ID0.
    //
    psDevice->ui32ChipID0 = MCUCTRL->CHIPID0;

    //
    // Read the Chip ID1.
    //
    psDevice->ui32ChipID1 = MCUCTRL->CHIPID1;

    //
    // Read the Chip Revision.
    //
    psDevice->ui32ChipRev = MCUCTRL->CHIPREV;

    //
    // Read the Chip VENDOR ID.
    //
    psDevice->ui32VendorID = MCUCTRL->VENDORID;

    //
    // Read the SKU (new for Apollo3).
    //
    psDevice->ui32SKU = MCUCTRL->SKU;

    //
    // Qualified from Part Number.
    //
    psDevice->ui32Qualified = (psDevice->ui32ChipPN >> MCUCTRL_CHIPPN_PARTNUM_QUAL_S) & 0x1;

    //
    // Flash size from Part Number.
    //
    psDevice->ui32FlashSize =
        g_am_hal_mcuctrl_flash_size[
            (psDevice->ui32ChipPN & MCUCTRL_CHIPPN_PARTNUM_FLASHSIZE_M) >>
            MCUCTRL_CHIPPN_PARTNUM_FLASHSIZE_S];

    //
    // SRAM size from Part Number.
    //
    psDevice->ui32SRAMSize =
        g_am_hal_mcuctrl_sram_size[
            (psDevice->ui32ChipPN & MCUCTRL_CHIPPN_PARTNUM_SRAMSIZE_M) >>
             MCUCTRL_CHIPPN_PARTNUM_SRAMSIZE_S];

    //
    // Now, let's look at the JEDEC info.
    // The full partnumber is 12 bits total, but is scattered across 2 registers.
    // Bits [11:8] are 0xE.
    // Bits [7:4] are 0xE for Apollo, 0xD for Apollo2.
    // Bits [3:0] are defined differently for Apollo and Apollo2.
    //   For Apollo, the low nibble is 0x0.
    //   For Apollo2, the low nibble indicates flash and SRAM size.
    //
    psDevice->ui32JedecPN  = JEDEC->PID0_b.PNL8 << 0;
    psDevice->ui32JedecPN |= JEDEC->PID1_b.PNH4 << 8;

    //
    // JEPID is the JEP-106 Manufacturer ID Code, which is assigned to Ambiq as
    //  0x1B, with parity bit is 0x9B.  It is 8 bits located across 2 registers.
    //
    psDevice->ui32JedecJEPID  = JEDEC->PID1_b.JEPIDL << 0;
    psDevice->ui32JedecJEPID |= JEDEC->PID2_b.JEPIDH << 4;

    //
    // CHIPREV is 8 bits located across 2 registers.
    //
    psDevice->ui32JedecCHIPREV  = JEDEC->PID2_b.CHIPREVH4 << 4;
    psDevice->ui32JedecCHIPREV |= JEDEC->PID3_b.CHIPREVL4 << 0;

    //
    // Let's get the Coresight ID (32-bits across 4 registers)
    // For Apollo and Apollo2, it's expected to be 0xB105100D.
    //
    psDevice->ui32JedecCID  = JEDEC->CID3_b.CID << 24;
    psDevice->ui32JedecCID |= JEDEC->CID2_b.CID << 16;
    psDevice->ui32JedecCID |= JEDEC->CID1_b.CID <<  8;
    psDevice->ui32JedecCID |= JEDEC->CID0_b.CID <<  0;
#else // AM_CMSIS_REGS
    //
    // Read the Part Number.
    //
    psDevice->ui32ChipPN = AM_REG(MCUCTRL, CHIPPN);

    //
    // Read the Chip ID0.
    //
    psDevice->ui32ChipID0 = AM_REG(MCUCTRL, CHIPID0);

    //
    // Read the Chip ID1.
    //
    psDevice->ui32ChipID1 = AM_REG(MCUCTRL, CHIPID1);

    //
    // Read the Chip Revision.
    //
    psDevice->ui32ChipRev = AM_REG(MCUCTRL, CHIPREV);

    //
    // Read the Chip VENDOR ID.
    //
    psDevice->ui32VendorID = AM_REG(MCUCTRL, VENDORID);

    //
    // Read the SKU (new for Apollo3).
    //
    psDevice->ui32SKU = AM_REG(MCUCTRL, SKU);

    //
    // Qualified from Part Number.
    //
    psDevice->ui32Qualified =
            (psDevice->ui32ChipPN & AM_REG_MCUCTRL_CHIPPN_PARTNUM_QUAL_M) >>
             AM_REG_MCUCTRL_CHIPPN_PARTNUM_QUAL_S;

    //
    // Flash size from Part Number.
    //
    psDevice->ui32FlashSize =
        g_am_hal_mcuctrl_flash_size[
            (psDevice->ui32ChipPN & AM_REG_MCUCTRL_CHIPPN_PARTNUM_FLASHSIZE_M) >>
            AM_REG_MCUCTRL_CHIPPN_PARTNUM_FLASHSIZE_S];

    //
    // SRAM size from Part Number.
    //
    psDevice->ui32SRAMSize =
        g_am_hal_mcuctrl_sram_size[
            (psDevice->ui32ChipPN & AM_REG_MCUCTRL_CHIPPN_PARTNUM_SRAMSIZE_M) >>
            AM_REG_MCUCTRL_CHIPPN_PARTNUM_SRAMSIZE_S];

    //
    // Now, let's look at the JEDEC info.
    // The full partnumber is 12 bits total, but is scattered across 2 registers.
    // Bits [11:8] are 0xE.
    // Bits [7:4] are 0xE for Apollo, 0xD for Apollo2.
    // Bits [3:0] are defined differently for Apollo and Apollo2.
    //   For Apollo, the low nibble is 0x0.
    //   For Apollo2, the low nibble indicates flash and SRAM size.
    //
    psDevice->ui32JedecPN  = (AM_BFR(JEDEC, PID0, PNL8) << 0);
    psDevice->ui32JedecPN |= (AM_BFR(JEDEC, PID1, PNH4) << 8);

    //
    // JEPID is the JEP-106 Manufacturer ID Code, which is assigned to Ambiq as
    //  0x1B, with parity bit is 0x9B.  It is 8 bits located across 2 registers.
    //
    psDevice->ui32JedecJEPID  = (AM_BFR(JEDEC, PID1, JEPIDL) << 0);
    psDevice->ui32JedecJEPID |= (AM_BFR(JEDEC, PID2, JEPIDH) << 4);

    //
    // CHIPREV is 8 bits located across 2 registers.
    //
    psDevice->ui32JedecCHIPREV  = (AM_BFR(JEDEC, PID2, CHIPREVH4) << 4);
    psDevice->ui32JedecCHIPREV |= (AM_BFR(JEDEC, PID3, CHIPREVL4) << 0);

    //
    // Let's get the Coresight ID (32-bits across 4 registers)
    // For Apollo and Apollo2, it's expected to be 0xB105100D.
    //
    psDevice->ui32JedecCID  = (AM_BFR(JEDEC, CID3, CID) << 24);
    psDevice->ui32JedecCID |= (AM_BFR(JEDEC, CID2, CID) << 16);
    psDevice->ui32JedecCID |= (AM_BFR(JEDEC, CID1, CID) <<  8);
    psDevice->ui32JedecCID |= (AM_BFR(JEDEC, CID0, CID) <<  0);
#endif // AM_CMSIS_REGS
} // device_info_get()

//*****************************************************************************
//
//  mcuctrl_fault_status()
//  Gets the fault status and capture registers.
//
//*****************************************************************************
static void
mcuctrl_fault_status(am_hal_mcuctrl_fault_t *psFault)
{
    uint32_t ui32FaultStat;

    //
    // Read the Fault Status Register.
    //
#if AM_CMSIS_REGS
    ui32FaultStat = MCUCTRL->FAULTSTATUS;
    psFault->bICODE = (bool)(ui32FaultStat & MCUCTRL_FAULTSTATUS_ICODEFAULT_Msk);
    psFault->bDCODE = (bool)(ui32FaultStat & MCUCTRL_FAULTSTATUS_DCODEFAULT_Msk);
    psFault->bSYS   = (bool)(ui32FaultStat & MCUCTRL_FAULTSTATUS_SYSFAULT_Msk);

    //
    // Read the DCODE fault capture address register.
    //
    psFault->ui32DCODE = MCUCTRL->DCODEFAULTADDR;

    //
    // Read the ICODE fault capture address register.
    //
    psFault->ui32ICODE |= MCUCTRL->ICODEFAULTADDR;

    //
    // Read the ICODE fault capture address register.
    //
    psFault->ui32SYS |= MCUCTRL->SYSFAULTADDR;
#else // AM_CMSIS_REGS
    ui32FaultStat = AM_REG(MCUCTRL, FAULTSTATUS);
    psFault->bICODE = (ui32FaultStat & AM_REG_MCUCTRL_FAULTSTATUS_ICODEFAULT_M);
    psFault->bDCODE = (ui32FaultStat & AM_REG_MCUCTRL_FAULTSTATUS_DCODEFAULT_M);
    psFault->bSYS = (ui32FaultStat & AM_REG_MCUCTRL_FAULTSTATUS_SYSFAULT_M);

    //
    // Read the DCODE fault capture address register.
    //
    psFault->ui32DCODE = AM_REG(MCUCTRL, DCODEFAULTADDR);

    //
    // Read the ICODE fault capture address register.
    //
    psFault->ui32ICODE |= AM_REG(MCUCTRL, ICODEFAULTADDR);

    //
    // Read the ICODE fault capture address register.
    //
    psFault->ui32SYS |= AM_REG(MCUCTRL, SYSFAULTADDR);
#endif // AM_CMSIS_REGS
} // mcuctrl_fault_status()

// ****************************************************************************
//
//  am_hal_mcuctrl_control()
//  Apply various specific commands/controls on the MCUCTRL module.
//
// ****************************************************************************
uint32_t
am_hal_mcuctrl_control(am_hal_mcuctrl_control_e eControl, void *pArgs)
{
#if AM_CMSIS_REGS
    switch ( eControl )
    {
        case AM_HAL_MCUCTRL_CONTROL_FAULT_CAPTURE_ENABLE:
            //
            // Enable the Fault Capture registers.
            //
            MCUCTRL->FAULTCAPTUREEN_b.FAULTCAPTUREEN = 1;
            break;

        case AM_HAL_MCUCTRL_CONTROL_FAULT_CAPTURE_DISABLE:
            //
            // Disable the Fault Capture registers.
            //
            MCUCTRL->FAULTCAPTUREEN_b.FAULTCAPTUREEN = 0;
            break;

        case AM_HAL_MCUCTRL_CONTROL_EXTCLK32K_ENABLE:
            //
            // Configure the bits in XTALCTRL that enable external 32KHz clock.
            //
            MCUCTRL->XTALCTRL &=
                ~(MCUCTRL_XTALCTRL_PDNBCMPRXTAL_Msk                 |
                  MCUCTRL_XTALCTRL_PDNBCOREXTAL_Msk                 |
                  MCUCTRL_XTALCTRL_BYPCMPRXTAL_Msk                  |
                  MCUCTRL_XTALCTRL_FDBKDSBLXTAL_Msk                 |
                  MCUCTRL_XTALCTRL_XTALSWE_Msk);

            MCUCTRL->XTALCTRL |=
                (uint32_t)MCUCTRL_XTALCTRL_PDNBCMPRXTAL_PWRDNCOMP   |
                (uint32_t)MCUCTRL_XTALCTRL_PDNBCOREXTAL_PWRDNCORE   |
                (uint32_t)MCUCTRL_XTALCTRL_BYPCMPRXTAL_BYPCOMP      |
                (uint32_t)MCUCTRL_XTALCTRL_FDBKDSBLXTAL_DIS         |
                (uint32_t)MCUCTRL_XTALCTRL_XTALSWE_OVERRIDE_EN;
            break;

        case AM_HAL_MCUCTRL_CONTROL_EXTCLK32K_DISABLE:
            //
            // Configure the bits in XTALCTRL that disable external 32KHz
            // clock, thus re-configuring for the crystal.
            //
            MCUCTRL->XTALCTRL &=
                ~(MCUCTRL_XTALCTRL_PDNBCMPRXTAL_Msk                 |
                  MCUCTRL_XTALCTRL_PDNBCOREXTAL_Msk                 |
                  MCUCTRL_XTALCTRL_BYPCMPRXTAL_Msk                  |
                  MCUCTRL_XTALCTRL_FDBKDSBLXTAL_Msk                 |
                  MCUCTRL_XTALCTRL_XTALSWE_Msk);

            MCUCTRL->XTALCTRL |=
                (uint32_t)MCUCTRL_XTALCTRL_PDNBCMPRXTAL_PWRUPCOMP   |
                (uint32_t)MCUCTRL_XTALCTRL_PDNBCOREXTAL_PWRUPCORE   |
                (uint32_t)MCUCTRL_XTALCTRL_BYPCMPRXTAL_USECOMP      |
                (uint32_t)MCUCTRL_XTALCTRL_FDBKDSBLXTAL_EN          |
                (uint32_t)MCUCTRL_XTALCTRL_XTALSWE_OVERRIDE_DIS;
            break;

        default:
            return AM_HAL_STATUS_INVALID_ARG;
    }
#else // AM_CMSIS_REGS
    switch ( eControl )
    {
        case AM_HAL_MCUCTRL_CONTROL_FAULT_CAPTURE_ENABLE:
            //
            // Enable the Fault Capture registers.
            //
            AM_BFW(MCUCTRL, FAULTCAPTUREEN, FAULTCAPTUREEN, 1);
            break;

        case AM_HAL_MCUCTRL_CONTROL_FAULT_CAPTURE_DISABLE:
            //
            // Disable the Fault Capture registers.
            //
            AM_BFW(MCUCTRL, FAULTCAPTUREEN, FAULTCAPTUREEN, 0);
            break;

        case AM_HAL_MCUCTRL_CONTROL_EXTCLK32K_ENABLE:
            //
            // Configure the bits in XTALCTRL that enable external 32KHz clock.
            //
            AM_REG(MCUCTRL, XTALCTRL) &=
                ~(AM_REG_MCUCTRL_XTALCTRL_PDNBCMPRXTAL_M        |
                  AM_REG_MCUCTRL_XTALCTRL_PDNBCOREXTAL_M        |
                  AM_REG_MCUCTRL_XTALCTRL_BYPCMPRXTAL_M         |
                  AM_REG_MCUCTRL_XTALCTRL_FDBKDSBLXTAL_M        |
                  AM_REG_MCUCTRL_XTALCTRL_XTALSWE_M);

            AM_REG(MCUCTRL, XTALCTRL) |=
                AM_REG_MCUCTRL_XTALCTRL_PDNBCMPRXTAL_PWRDNCOMP  |
                AM_REG_MCUCTRL_XTALCTRL_PDNBCOREXTAL_PWRDNCORE  |
                AM_REG_MCUCTRL_XTALCTRL_BYPCMPRXTAL_BYPCOMP     |
                AM_REG_MCUCTRL_XTALCTRL_FDBKDSBLXTAL_DIS        |
                AM_REG_MCUCTRL_XTALCTRL_XTALSWE_OVERRIDE_EN;
            break;

        case AM_HAL_MCUCTRL_CONTROL_EXTCLK32K_DISABLE:
            //
            // Configure the bits in XTALCTRL that disable external 32KHz
            // clock, thus re-configuring for the crystal.
            //
            AM_REG(MCUCTRL, XTALCTRL) &=
                ~(AM_REG_MCUCTRL_XTALCTRL_PDNBCMPRXTAL_M        |
                  AM_REG_MCUCTRL_XTALCTRL_PDNBCOREXTAL_M        |
                  AM_REG_MCUCTRL_XTALCTRL_BYPCMPRXTAL_M         |
                  AM_REG_MCUCTRL_XTALCTRL_FDBKDSBLXTAL_M        |
                  AM_REG_MCUCTRL_XTALCTRL_XTALSWE_M);

            AM_REG(MCUCTRL, XTALCTRL) |=
                AM_REG_MCUCTRL_XTALCTRL_PDNBCMPRXTAL_PWRUPCOMP  |
                AM_REG_MCUCTRL_XTALCTRL_PDNBCOREXTAL_PWRUPCORE  |
                AM_REG_MCUCTRL_XTALCTRL_BYPCMPRXTAL_USECOMP     |
                AM_REG_MCUCTRL_XTALCTRL_FDBKDSBLXTAL_EN         |
                AM_REG_MCUCTRL_XTALCTRL_XTALSWE_OVERRIDE_DIS;
            break;

        default:
            return AM_HAL_STATUS_INVALID_ARG;
    }
#endif // AM_CMSIS_REGS

    //
    // Return success status.
    //
    return AM_HAL_STATUS_SUCCESS;

} // am_hal_mcuctrl_control()

// ****************************************************************************
//
//  am_hal_mcuctrl_status_get()
//! This function returns  current status of the MCUCTRL as obtained from
//! various registers of the MCUCTRL block.
//
// ****************************************************************************
uint32_t
am_hal_mcuctrl_status_get(am_hal_mcuctrl_status_t *psStatus)
{
    uint32_t ui32Status;

    if ( psStatus == NULL )
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

#if AM_CMSIS_REGS
    ui32Status = MCUCTRL->FEATUREENABLE;
    psStatus->bBurstAck =
        _FLD2VAL(MCUCTRL_FEATUREENABLE_BURSTACK, ui32Status);
    psStatus->bBLEAck =
        _FLD2VAL(MCUCTRL_FEATUREENABLE_BLEACK, ui32Status);

    psStatus->bDebuggerLockout =
        _FLD2VAL(MCUCTRL_DEBUGGER_LOCKOUT, MCUCTRL->DEBUGGER);

    psStatus->bADCcalibrated =
        _FLD2VAL(MCUCTRL_ADCCAL_ADCCALIBRATED, MCUCTRL->ADCCAL);

    psStatus->bBattLoadEnabled =
        _FLD2VAL(MCUCTRL_ADCBATTLOAD_BATTLOAD, MCUCTRL->ADCBATTLOAD);

    ui32Status = MCUCTRL->BOOTLOADER;
    psStatus->bSecBootOnColdRst =
        _FLD2VAL(MCUCTRL_BOOTLOADER_SECBOOT, ui32Status);
    psStatus->bSecBootOnWarmRst =
        _FLD2VAL(MCUCTRL_BOOTLOADER_SECBOOTONRST, ui32Status);
#else // AM_CMSIS_REGS
    ui32Status = AM_REG(MCUCTRL, FEATUREENABLE);
    psStatus->bBurstAck =
                AM_BFX(MCUCTRL, FEATUREENABLE, BURSTACK, ui32Status);
    psStatus->bBLEAck =
                AM_BFX(MCUCTRL, FEATUREENABLE, BLEACK, ui32Status);

    ui32Status = AM_REG(MCUCTRL, DEBUGGER);
    psStatus->bDebuggerLockout =
                    AM_BFX(MCUCTRL, DEBUGGER, LOCKOUT, ui32Status);

    ui32Status = AM_REG(MCUCTRL, ADCCAL);
    psStatus->bADCcalibrated =
                    AM_BFX(MCUCTRL, ADCCAL, ADCCALIBRATED, ui32Status);

    ui32Status = AM_REG(MCUCTRL, ADCBATTLOAD);
    psStatus->bBattLoadEnabled =
                    AM_BFX(MCUCTRL, ADCBATTLOAD, BATTLOAD, ui32Status);

    ui32Status = AM_REG(MCUCTRL, BOOTLOADER);
    psStatus->bSecBootOnColdRst =
                    AM_BFX(MCUCTRL, BOOTLOADER, SECBOOT, ui32Status);
    psStatus->bSecBootOnWarmRst =
                    AM_BFX(MCUCTRL, BOOTLOADER, SECBOOTONRST, ui32Status);
#endif // AM_CMSIS_REGS

    return AM_HAL_STATUS_SUCCESS;

} // am_hal_mcuctrl_status_get()

// ****************************************************************************
//
//  am_hal_mcuctrl_info_get()
//  Get information of the given MCUCTRL item.
//
// ****************************************************************************
uint32_t
am_hal_mcuctrl_info_get(am_hal_mcuctrl_infoget_e eInfoGet, void *pInfo)
{
    am_hal_mcuctrl_feature_t *psFeature;
    uint32_t ui32Feature;

    if ( pInfo == NULL )
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

#if AM_CMSIS_REGS
    switch ( eInfoGet )
    {
        case AM_HAL_MCUCTRL_INFO_FEATURES_AVAIL:
            psFeature = (am_hal_mcuctrl_feature_t*)pInfo;
            ui32Feature = MCUCTRL->FEATUREENABLE;
            psFeature->bBurstAvail =
                _FLD2VAL(MCUCTRL_FEATUREENABLE_BURSTAVAIL, ui32Feature);
            psFeature->bBLEavail =
                _FLD2VAL(MCUCTRL_FEATUREENABLE_BLEAVAIL, ui32Feature);

            ui32Feature = MCUCTRL->BOOTLOADER;
            psFeature->ui8SecBootFeature =
                _FLD2VAL(MCUCTRL_BOOTLOADER_SECBOOTFEATURE, ui32Feature);

            ui32Feature = MCUCTRL->SKU;
            psFeature->bBLEFeature =
                _FLD2VAL(MCUCTRL_SKU_ALLOWBLE, ui32Feature);
            psFeature->bBurstFeature =
                _FLD2VAL(MCUCTRL_SKU_ALLOWBURST, ui32Feature);
            break;

        case AM_HAL_MCUCTRL_INFO_DEVICEID:
            device_info_get((am_hal_mcuctrl_device_t *)pInfo);
            break;

        case AM_HAL_MCUCTRL_INFO_FAULT_STATUS:
            mcuctrl_fault_status((am_hal_mcuctrl_fault_t*)pInfo);
            break;

        default:
            return AM_HAL_STATUS_INVALID_ARG;
    }
#else // AM_CMSIS_REGS
    switch ( eInfoGet )
    {
        case AM_HAL_MCUCTRL_INFO_FEATURES_AVAIL:
            psFeature = (am_hal_mcuctrl_feature_t*)pInfo;
            ui32Feature = AM_REG(MCUCTRL, FEATUREENABLE);
            psFeature->bBurstAvail =
                        AM_BFX(MCUCTRL, FEATUREENABLE, BURSTAVAIL, ui32Feature);
            psFeature->bBLEavail =
                        AM_BFX(MCUCTRL, FEATUREENABLE, BLEAVAIL, ui32Feature);

            ui32Feature = AM_REG(MCUCTRL, BOOTLOADER);
            psFeature->ui8SecBootFeature =
                        AM_BFX(MCUCTRL, BOOTLOADER, SECBOOTFEATURE, ui32Feature);

            ui32Feature = AM_REG(MCUCTRL, SKU);
            psFeature->bBLEFeature =
                        AM_BFX(MCUCTRL, SKU, ALLOWBLE, ui32Feature);
            psFeature->bBurstFeature =
                        AM_BFX(MCUCTRL, SKU, ALLOWBURST, ui32Feature);
            break;

        case AM_HAL_MCUCTRL_INFO_DEVICEID:
            device_info_get((am_hal_mcuctrl_device_t *)pInfo);
            break;

        case AM_HAL_MCUCTRL_INFO_FAULT_STATUS:
            mcuctrl_fault_status((am_hal_mcuctrl_fault_t*)pInfo);
            break;

        default:
            return AM_HAL_STATUS_INVALID_ARG;
    }
#endif // AM_CMSIS_REGS
    //
    // Return success status.
    //
    return AM_HAL_STATUS_SUCCESS;

} // am_hal_mcuctrl_info_get()

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
