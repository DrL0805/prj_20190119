//*****************************************************************************
//
//  am_hal_cachectrl.c
//! @file
//!
//! @brief Functions for interfacing with the CACHE controller.
//!
//! @addtogroup clkgen3 Clock Generator (CACHE)
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
//  Default settings for the cache.
//
//*****************************************************************************
const am_hal_cachectrl_config_t am_hal_cachectrl_defaults =
{
    .bLRU                       = 0,
    .eDescript                  = AM_HAL_CACHECTRL_DESCR_1WAY_128B_1024E,
    .eMode                      = AM_HAL_CACHECTRL_CONFIG_MODE_INSTR_DATA,
    .eEnableNCregions           = AM_HAL_CACHECTRL_CONFIG_NCREG_NONE,
};

//*****************************************************************************
//
//  Configure the cache with given and recommended settings, but do not enable.
//
//*****************************************************************************
uint32_t
am_hal_cachectrl_config(const am_hal_cachectrl_config_t *psConfig)
{
    //
    // In the case where cache is currently enabled, we need to gracefully
    // bow out of that configuration before reconfiguring.  The best way to
    // accomplish that is to shut down the ID bits, leaving the cache enabled.
    // Once the instr and data caches have been disabled, we can safely set
    // any new configuration, including disabling the controller.
    //
#if AM_CMSIS_REGS
    AM_CRITICAL_BEGIN
    CACHECTRL->CACHECFG &=
        ~(CACHECTRL_CACHECFG_DCACHE_ENABLE_Msk  |
          CACHECTRL_CACHECFG_ICACHE_ENABLE_Msk);
    AM_CRITICAL_END

    CACHECTRL->CACHECFG =
        _VAL2FLD(CACHECTRL_CACHECFG_ENABLE, 0)                              |
        _VAL2FLD(CACHECTRL_CACHECFG_CACHE_CLKGATE, 1)                       |
        _VAL2FLD(CACHECTRL_CACHECFG_CACHE_LS, 0)                            |
        _VAL2FLD(CACHECTRL_CACHECFG_DATA_CLKGATE, 1)                        |
        _VAL2FLD(CACHECTRL_CACHECFG_ENABLE_MONITOR, 0)                      |
        _VAL2FLD(CACHECTRL_CACHECFG_LRU, psConfig->bLRU)                    |
        _VAL2FLD(CACHECTRL_CACHECFG_CONFIG, psConfig->eDescript)            |
        ((psConfig->eEnableNCregions << CACHECTRL_CACHECFG_ENABLE_NC0_Pos) &
            (CACHECTRL_CACHECFG_ENABLE_NC1_Msk      |
             CACHECTRL_CACHECFG_ENABLE_NC0_Msk))                            |
        ((psConfig->eMode << CACHECTRL_CACHECFG_ICACHE_ENABLE_Pos) &
            (CACHECTRL_CACHECFG_DCACHE_ENABLE_Msk   |
             CACHECTRL_CACHECFG_ICACHE_ENABLE_Msk));

#else // AM_CMSIS_REGS
    AM_REGa_CLR(CACHECTRL, CACHECFG,
        AM_REG_CACHECTRL_CACHECFG_DCACHE_ENABLE(1)  |
        AM_REG_CACHECTRL_CACHECFG_ICACHE_ENABLE(1) );

    //
    // Set the new configuration including disabling the cache controller.
    //
    AM_REG(CACHECTRL, CACHECFG) =
        AM_REG_CACHECTRL_CACHECFG_ENABLE(0)                                 |
        AM_REG_CACHECTRL_CACHECFG_CACHE_CLKGATE(1)                          |
        AM_REG_CACHECTRL_CACHECFG_CACHE_LS(0)                               |
        AM_REG_CACHECTRL_CACHECFG_DATA_CLKGATE(1)                           |
        AM_REG_CACHECTRL_CACHECFG_ENABLE_MONITOR(0)                         |
        AM_REG_CACHECTRL_CACHECFG_LRU(psConfig->bLRU)                       |
        AM_REG_CACHECTRL_CACHECFG_CONFIG(psConfig->eDescript)               |
        ((psConfig->eEnableNCregions << AM_REG_CACHECTRL_CACHECFG_ENABLE_NC0_S) &
            (AM_REG_CACHECTRL_CACHECFG_ENABLE_NC1_M                         |
             AM_REG_CACHECTRL_CACHECFG_ENABLE_NC0_M))                       |
        ((psConfig->eMode << AM_REG_CACHECTRL_CACHECFG_ICACHE_ENABLE_S) &
            (AM_REG_CACHECTRL_CACHECFG_DCACHE_ENABLE_M                      |
             AM_REG_CACHECTRL_CACHECFG_ICACHE_ENABLE_M));

#endif // AM_CMSIS_REGS

    return AM_HAL_STATUS_SUCCESS;

} // am_hal_cachectrl_enable()

//*****************************************************************************
//
//  Enable the cache.
//
//*****************************************************************************
uint32_t
am_hal_cachectrl_enable(void)
{
#if AM_CMSIS_REGS
    CACHECTRL->CACHECFG |= _VAL2FLD(CACHECTRL_CACHECFG_ENABLE, 1);
#else // AM_CMSIS_REGS
    AM_REG(CACHECTRL, CACHECFG) |= AM_REG_CACHECTRL_CACHECFG_ENABLE(1);
#endif // AM_CMSIS_REGS

    return AM_HAL_STATUS_SUCCESS;
} // am_hal_cachectrl_enable()

//*****************************************************************************
//
//  Disable the cache.
//
//*****************************************************************************
uint32_t
am_hal_cachectrl_disable(void)
{
    //
    // Shut down as gracefully as possible.
    // Disable the I/D cache enable bits first to allow a little time
    // for any in-flight transactions to hand off to the line buffer.
    // Then clear the enable.
    //
    AM_CRITICAL_BEGIN
#if AM_CMSIS_REGS
    CACHECTRL->CACHECFG &= ~(_VAL2FLD(CACHECTRL_CACHECFG_ICACHE_ENABLE, 1) |
                             _VAL2FLD(CACHECTRL_CACHECFG_DCACHE_ENABLE, 1));
    CACHECTRL->CACHECFG &= ~_VAL2FLD(CACHECTRL_CACHECFG_ENABLE, 1);
#else // AM_CMSIS_REGS
    AM_REG(CACHECTRL, CACHECFG) &=
        ~(AM_REG_CACHECTRL_CACHECFG_ICACHE_ENABLE(1)    |
          AM_REG_CACHECTRL_CACHECFG_DCACHE_ENABLE(1));

    AM_REG(CACHECTRL, CACHECFG) &= ~AM_REG_CACHECTRL_CACHECFG_ENABLE(1);
#endif // AM_CMSIS_REGS
    AM_CRITICAL_END

    return AM_HAL_STATUS_SUCCESS;
} // am_hal_cachectrl_disable()

//*****************************************************************************
//
//  Select the cache configuration type.
//
//*****************************************************************************
uint32_t
am_hal_cachectrl_control(am_hal_cachectrl_control_e eControl, void *pArgs)
{
    uint32_t ui32Val;
    uint32_t ui32SetMask = 0;

#if AM_CMSIS_REGS
    switch ( eControl )
    {
        case AM_HAL_CACHECTRL_CONTROL_FLASH_CACHE_INVALIDATE:
            ui32SetMask = CACHECTRL_CTRL_INVALIDATE_Msk;
            break;
        case AM_HAL_CACHECTRL_CONTROL_STATISTICS_RESET:
            if ( !_FLD2VAL(CACHECTRL_CACHECFG_ENABLE_MONITOR, CACHECTRL->CACHECFG) )
            {
                //
                // The monitor must be enabled for the reset to have any affect.
                //
                return AM_HAL_STATUS_INVALID_OPERATION;
            }
            else
            {
                ui32SetMask = CACHECTRL_CTRL_RESET_STAT_Msk;
            }
            break;
        case AM_HAL_CACHECTRL_CONTROL_FLASH_ALL_SLEEP_ENABLE:
            ui32SetMask = CACHECTRL_CTRL_FLASH0_SLM_ENABLE_Msk      |
                          CACHECTRL_CTRL_FLASH1_SLM_ENABLE_Msk;
            break;
        case AM_HAL_CACHECTRL_CONTROL_FLASH_ALL_SLEEP_DISABLE:
            ui32SetMask = CACHECTRL_CTRL_FLASH0_SLM_DISABLE_Msk     |
                          CACHECTRL_CTRL_FLASH1_SLM_DISABLE_Msk;
            break;
        case AM_HAL_CACHECTRL_CONTROL_FLASH0_SLEEP_ENABLE:
            ui32SetMask = CACHECTRL_CTRL_FLASH0_SLM_ENABLE_Msk;
            break;
        case AM_HAL_CACHECTRL_CONTROL_FLASH0_SLEEP_DISABLE:
            ui32SetMask = CACHECTRL_CTRL_FLASH0_SLM_DISABLE_Msk;
            break;
        case AM_HAL_CACHECTRL_CONTROL_FLASH1_SLEEP_ENABLE:
            ui32SetMask = CACHECTRL_CTRL_FLASH1_SLM_ENABLE_Msk;
            break;
        case AM_HAL_CACHECTRL_CONTROL_FLASH1_SLEEP_DISABLE:
            ui32SetMask = CACHECTRL_CTRL_FLASH1_SLM_DISABLE_Msk;
            break;
        case AM_HAL_CACHECTRL_CONTROL_MONITOR_ENABLE:
            ui32SetMask = 0;
            AM_CRITICAL_BEGIN
            CACHECTRL->CACHECFG |= CACHECTRL_CACHECFG_ENABLE_MONITOR_Msk;
            AM_CRITICAL_END
            break;
        case AM_HAL_CACHECTRL_CONTROL_MONITOR_DISABLE:
            ui32SetMask = 0;
            AM_CRITICAL_BEGIN
            CACHECTRL->CACHECFG &= ~CACHECTRL_CACHECFG_ENABLE_MONITOR_Msk;
            AM_CRITICAL_END
            break;
        case AM_HAL_CACHECTRL_CONTROL_LPMMODE_SET:
            //
            // Safely set LPMMODE.
            // The new mode is passed by reference via pArgs.  That is, pArgs is
            // assumed to be a pointer to a uint32_t of the new LPMMODE value.
            //
            ui32Val = *((uint32_t*)pArgs);
            if ( ui32Val > CACHECTRL_FLASHCFG_LPMMODE_ALWAYS )
            {
                return AM_HAL_STATUS_INVALID_ARG;
            }
            AM_CRITICAL_BEGIN
            ui32Val = am_hal_flash_load_ui32((uint32_t*)&CACHECTRL->FLASHCFG);
            ui32Val &= ~CACHECTRL_FLASHCFG_LPMMODE_Msk;
            ui32Val |= _VAL2FLD(CACHECTRL_FLASHCFG_LPMMODE, *((uint32_t*)pArgs));
            am_hal_flash_store_ui32((uint32_t*)&CACHECTRL->FLASHCFG, ui32Val);
            AM_CRITICAL_END
            break;
        default:
            return AM_HAL_STATUS_INVALID_ARG;
    }

    //
    // All fields in the CACHECTRL register are write-only or read-only.
    // A write to CACHECTRL acts as a mask-set.  That is, only the bits
    // written as '1' have an effect, any bits written as '0' are unaffected.
    //
    // Important note - setting of an enable and disable simultanously has
    // unpredicable results.
    //
    if ( ui32SetMask )
    {
        CACHECTRL->CTRL = ui32SetMask;
    }
#else // AM_CMSIS_REGS
    switch ( eControl )
    {
        case AM_HAL_CACHECTRL_CONTROL_FLASH_CACHE_INVALIDATE:
            ui32SetMask = AM_REG_CACHECTRL_CTRL_INVALIDATE_M;
            break;
        case AM_HAL_CACHECTRL_CONTROL_STATISTICS_RESET:
            if ( !AM_BFR(CACHECTRL, CACHECFG, ENABLE_MONITOR) )
            {
                //
                // The monitor must be enabled for the reset to have any affect.
                //
                return AM_HAL_STATUS_INVALID_OPERATION;
            }
            else
            {
                ui32SetMask = AM_REG_CACHECTRL_CTRL_RESET_STAT_M;
            }
            break;
        case AM_HAL_CACHECTRL_CONTROL_FLASH_ALL_SLEEP_ENABLE:
            ui32SetMask = AM_REG_CACHECTRL_CTRL_FLASH0_SLM_ENABLE_M     |
                          AM_REG_CACHECTRL_CTRL_FLASH1_SLM_ENABLE_M;
            break;
        case AM_HAL_CACHECTRL_CONTROL_FLASH_ALL_SLEEP_DISABLE:
            ui32SetMask = AM_REG_CACHECTRL_CTRL_FLASH0_SLM_DISABLE_M    |
                          AM_REG_CACHECTRL_CTRL_FLASH1_SLM_DISABLE_M;
            break;
        case AM_HAL_CACHECTRL_CONTROL_FLASH0_SLEEP_ENABLE:
            ui32SetMask = AM_REG_CACHECTRL_CTRL_FLASH0_SLM_ENABLE_M;
            break;
        case AM_HAL_CACHECTRL_CONTROL_FLASH0_SLEEP_DISABLE:
            ui32SetMask = AM_REG_CACHECTRL_CTRL_FLASH0_SLM_DISABLE_M;
            break;
        case AM_HAL_CACHECTRL_CONTROL_FLASH1_SLEEP_ENABLE:
            ui32SetMask = AM_REG_CACHECTRL_CTRL_FLASH1_SLM_ENABLE_M;
            break;
        case AM_HAL_CACHECTRL_CONTROL_FLASH1_SLEEP_DISABLE:
            ui32SetMask = AM_REG_CACHECTRL_CTRL_FLASH1_SLM_DISABLE_M;
            break;
        case AM_HAL_CACHECTRL_CONTROL_MONITOR_ENABLE:
            ui32SetMask = 0;
            AM_REGa_SET(CACHECTRL, CACHECFG, AM_REG_CACHECTRL_CACHECFG_ENABLE_MONITOR_M);
            break;
        case AM_HAL_CACHECTRL_CONTROL_MONITOR_DISABLE:
            ui32SetMask = 0;
            AM_REGa_CLR(CACHECTRL, CACHECFG, AM_REG_CACHECTRL_CACHECFG_ENABLE_MONITOR_M);
            break;
        case AM_HAL_CACHECTRL_CONTROL_LPMMODE_SET:
            //
            // Safely set LPMMODE.
            // The new mode is passed by reference via pArgs.  That is, pArgs is
            // assumed to be a pointer to a uint32_t of the new LPMMODE value.
            //
            ui32Val = *((uint32_t*)pArgs);
            if ( (ui32Val > AM_REG_CACHECTRL_FLASHCFG_LPMMODE_ALWAYS) ||
                 (ui32Val & ~AM_REG_CACHECTRL_FLASHCFG_LPMMODE_M) )
            {
                return AM_HAL_STATUS_INVALID_ARG;
            }
            AM_CRITICAL_BEGIN
            ui32Val = am_hal_flash_load_ui32((uint32_t*)AM_REGADDRn(CACHECTRL, 0, FLASHCFG));
            ui32Val &= ~AM_REG_CACHECTRL_FLASHCFG_LPMMODE_M;
            ui32Val |= AM_BFV(CACHECTRL, FLASHCFG, LPMMODE, *((uint32_t*)pArgs));
            am_hal_flash_store_ui32((uint32_t*)AM_REGADDRn(CACHECTRL, 0, FLASHCFG), ui32Val);
            AM_CRITICAL_END
            break;
        default:
            return AM_HAL_STATUS_INVALID_ARG;
    }

    //
    // All fields in the CACHECTRL register are write-only or read-only.
    // A write to CACHECTRL acts as a mask-set.  That is, only the bits
    // written as '1' have an effect, any bits written as '0' are unaffected.
    //
    // Important note - setting of an enable and disable simultanously has
    // unpredicable results.
    //
    if ( ui32SetMask )
    {
        AM_REG(CACHECTRL, CTRL) = ui32SetMask;
    }
#endif // AM_CMSIS_REGS

    return AM_HAL_STATUS_SUCCESS;

} // am_hal_cachectrl_control()

//*****************************************************************************
//
//  Cache controller status function
//
//*****************************************************************************
uint32_t
am_hal_cachectrl_status_get(am_hal_cachectrl_status_t *psStatus)
{
    uint32_t ui32Status;

    if ( psStatus == NULL )
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

#if AM_CMSIS_REGS
    ui32Status = CACHECTRL->CTRL;

    psStatus->bFlash0SleepMode =
        _FLD2VAL(CACHECTRL_CTRL_FLASH0_SLM_STATUS, ui32Status);
    psStatus->bFlash1SleepMode =
        _FLD2VAL(CACHECTRL_CTRL_FLASH1_SLM_STATUS, ui32Status);
    psStatus->bCacheReady =
        _FLD2VAL(CACHECTRL_CTRL_CACHE_READY, ui32Status);
#else // AM_CMSIS_REGS
    ui32Status = AM_REG(CACHECTRL, CTRL);

    psStatus->bFlash0SleepMode =
        AM_BFX(CACHECTRL, CTRL, FLASH0_SLM_STATUS, ui32Status);
    psStatus->bFlash1SleepMode =
        AM_BFX(CACHECTRL, CTRL, FLASH1_SLM_STATUS, ui32Status);
    psStatus->bCacheReady =
        AM_BFX(CACHECTRL, CTRL, CACHE_READY, ui32Status);
#endif // AM_CMSIS_REGS

    return AM_HAL_STATUS_SUCCESS;

} // am_hal_cachectrl_status_get()


//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
