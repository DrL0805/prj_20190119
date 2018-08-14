//*****************************************************************************
//
//  am_hal_mspi.c
//! @file
//!
//! @brief Functions for interfacing with the MSPI.
//!
//! @addtogroup mcuctrl3 Multi-bit SPI (MSPI)
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
// Private Types.
//
//*****************************************************************************

#define AM_HAL_MAGIC_MSPI               0xBEBEBE
#define AM_HAL_MSPI_CHK_HANDLE(h)       ((h) && ((am_hal_handle_prefix_t *)(h))->s.bInit && (((am_hal_handle_prefix_t *)(h))->s.magic == AM_HAL_MAGIC_MSPI))
#define AM_HAL_MSPI_HW_IDX_MAX          (AM_REG_MSPI_CQCURIDX_CQCURIDX_M >> AM_REG_MSPI_CQCURIDX_CQCURIDX_S)    // 8 bit value
#define AM_HAL_MSPI_MAX_CQ_ENTRIES      (256)
#define AM_HAL_MSPI_CQUPD_INT_FLAG      (0x00000001)

//
// Command Queue entry structure for DMA transfer.
//
typedef struct
{
#if MSPI_USE_CQ
    uint32_t                    ui32PAUSENAddr;
    uint32_t                    ui32PAUSEENVal;
#endif
#if !MSPI_USE_CQ
    // Need to disable the DMA before reconfiguring it
    uint32_t                    ui32DMACFG2Addr;
    uint32_t                    ui32DMACFG2Val;
#endif
    uint32_t                    ui32DMATARGADDRAddr;
    uint32_t                    ui32DMATARGADDRVal;
    uint32_t                    ui32DMADEVADDRAddr;
    uint32_t                    ui32DMADEVADDRVal;
    uint32_t                    ui32DMATOTCOUNTAddr;
    uint32_t                    ui32DMATOTCOUNTVal;
    uint32_t                    ui32DMACFG1Addr;
    uint32_t                    ui32DMACFG1Val;
#if MSPI_USE_CQ
    // Need to disable the DMA to prepare for next reconfig
    // Need to have this following the DMAEN for CMDQ
    // As otherwise we seem to be hitting CQ Pause condition - CORVETTE-882
    uint32_t                    ui32DMACFG2Addr;
    uint32_t                    ui32DMACFG2Val;
    uint32_t                    ui32PAUSEN2Addr;
    uint32_t                    ui32PAUSEEN2Val;
    uint32_t                    ui32SETCLRAddr;
    uint32_t                    ui32SETCLRVal;
#endif
} am_hal_mspi_cq_dma_entry_t;

//
// Command Queue entry structure for PIO transfer.
//
typedef struct
{
    uint32_t                    ui32ADDRAddr;
    uint32_t                    ui32ADDRVal;
    uint32_t                    ui32INSTRAddr;
    uint32_t                    ui32INSTRVal;
    uint32_t                    ui32CTRLAddr;
    uint32_t                    ui32CTRLVal;
} am_hal_mspi_cq_pio_entry_t;

typedef struct
{
    bool        bValid;
    uint32_t    regCFG;
    uint32_t    regMSPICFG;
    uint32_t    regPADCFG;
    uint32_t    regPADOUTEN;
    uint32_t    regFLASH;
    uint32_t    regSCRAMBLING;
    uint32_t    regCQCFG;
    uint32_t    regCQADDR;
    uint32_t    regCQPAUSE;
    uint32_t    regCQCURIDX;
    uint32_t    regCQENDIDX;
    uint32_t    regINTEN;
    // TODO: May be no need to preserve these values, as they are constants anyways?
    uint32_t    regDMABCOUNT;
    uint32_t    regDMATHRESH;
} am_hal_mspi_register_state_t;

//
// Command Queue control structure.
//
typedef struct
{
    void     *pCmdQHdl;
} am_hal_mspi_CQ_t;

//
// MSPI State structure.
//
typedef struct
{
    //
    // Handle validation prefix.
    //
    am_hal_handle_prefix_t  prefix;

    //
    // Physical module number.
    //
    uint32_t                      ui32Module;

    //
    // Selected flash device configuration.
    //
    am_hal_mspi_device_e          eDeviceConfig;

    //
    // Endianess of the FIFO interface.
    //
    bool                          bBigEndian;

    //
    // Delay timeout value.
    //
    uint32_t                      waitTimeout;
    // DMA Transfer Control Buffer size in words.
    uint32_t                      ui32TCBSize;

    // DMA Transfer Control Buffer
    uint32_t                      *pTCB;

    uint32_t ui32LastIdxProcessed;
    uint32_t ui32NumCQEntries;
    uint32_t ui32TxnInt;

    //
    // Stores the CQ callbacks.
    //
    am_hal_mspi_callback_t pfnCallback[AM_HAL_MSPI_MAX_CQ_ENTRIES];

    void                   *pCallbackCtxt[AM_HAL_MSPI_MAX_CQ_ENTRIES];
#if MSPI_USE_CQ
    //
    // Command Queue.
    //
    am_hal_mspi_CQ_t              CQ;
#else
    uint32_t                      ui32MaxTransactions;
    uint32_t                      ui32NextIdx;
    am_hal_mspi_cq_dma_entry_t    *pTransactions;
#endif

    //
    // MSPI Capabilities.
    //
    am_hal_mspi_capabilities_t    capabilities;

    // Power Save-Restore register state
    am_hal_mspi_register_state_t  registerState;
} am_hal_mspi_state_t;

//*****************************************************************************
//
// Global Variables.
//
//*****************************************************************************
am_hal_mspi_state_t             g_MSPIState[AM_REG_MSPI_NUM_MODULES];


#if !MSPI_USE_CQ
void (*g_pfnDMACallback[AM_REG_MSPI_NUM_MODULES])(void);
void *g_pCallbackCtxt[AM_REG_MSPI_NUM_MODULES];
#endif //!MSPI_USE_CQ

//*****************************************************************************
//
// Internal Functions.
//
//*****************************************************************************
void
build_dma_cmdlist(am_hal_mspi_state_t *pMSPIState,
                  am_hal_mspi_trans_e eMode,
                  void  *pCQEntry,
                  void  *pTransaction)
{
    uint32_t ui32Module = pMSPIState->ui32Module;

    switch(eMode)
    {
        case AM_HAL_MSPI_TRANS_PIO:
        {
            am_hal_mspi_cq_pio_entry_t    *pPIOEntry = (am_hal_mspi_cq_pio_entry_t*)pCQEntry;
            am_hal_mspi_pio_transfer_t    *pPIOTrans = (am_hal_mspi_pio_transfer_t*)pTransaction;

#if AM_CMSIS_REGS
            //
            // Command to set the CTRL register.
            //
            pPIOEntry->ui32ADDRAddr            = (uint32_t)&MSPIn(ui32Module)->ADDR;
            pPIOEntry->ui32ADDRVal             = _VAL2FLD(MSPI_ADDR_ADDR, pPIOTrans->ui32DeviceAddr);
            pPIOEntry->ui32INSTRAddr           = (uint32_t)&MSPIn(ui32Module)->INSTR;
            pPIOEntry->ui32INSTRVal            = _VAL2FLD(MSPI_INSTR_INSTR, pPIOTrans->ui16DeviceInstr);
            pPIOEntry->ui32CTRLAddr            = (uint32_t)&MSPIn(ui32Module)->CTRL;
            pPIOEntry->ui32CTRLVal             =
                _VAL2FLD(MSPI_CTRL_XFERBYTES, pPIOTrans->ui32NumBytes)  |   // Set the number of bytes to transfer.
                _VAL2FLD(MSPI_CTRL_PIOSCRAMBLE, pPIOTrans->bScrambling) |   // Set the scrambling if selected.
                _VAL2FLD(MSPI_CTRL_TXRX, pPIOTrans->eDirection)         |   // Set transmit or receive operation.
                _VAL2FLD(MSPI_CTRL_SENDI, pPIOTrans->bSendInstr)        |   // Enable sending the instruction.
                _VAL2FLD(MSPI_CTRL_SENDA, pPIOTrans->bSendAddr)         |   // Enable sending the address.
                _VAL2FLD(MSPI_CTRL_ENTURN, pPIOTrans->bTurnaround)      |   // Set the turn-around if needed.
                _VAL2FLD(MSPI_CTRL_BIGENDIAN, pMSPIState->bBigEndian)   |   // Set the FIFO endian format.
                _VAL2FLD(MSPI_CTRL_CONT, pPIOTrans->bContinue)          |   // Set for continuation transfer if indicated.
                _VAL2FLD(MSPI_CTRL_QUADCMD, pPIOTrans->bQuadCmd)        |   // Set the Quad Command if indicated.
                _VAL2FLD(MSPI_CTRL_START, 1);                               // Start the transfer.
#else // AM_CMSIS_REGS
            // Command to set the CTRL register.
            pPIOEntry->ui32ADDRAddr            = AM_REGADDRn(MSPI, ui32Module, ADDR);
            pPIOEntry->ui32ADDRVal             = AM_REG_MSPI_ADDR_ADDR(pPIOTrans->ui32DeviceAddr);
            pPIOEntry->ui32INSTRAddr           = AM_REGADDRn(MSPI, ui32Module, INSTR);
            pPIOEntry->ui32INSTRVal            = AM_REG_MSPI_INSTR_INSTR(pPIOTrans->ui16DeviceInstr);
            pPIOEntry->ui32CTRLAddr            = AM_REGADDRn(MSPI, ui32Module, CTRL);
            pPIOEntry->ui32CTRLVal             =
            AM_REG_MSPI_CTRL_XFERBYTES(pPIOTrans->ui32NumBytes)  |                  // Set the number of bytes to transfer.
            AM_REG_MSPI_CTRL_PIOSCRAMBLE(pPIOTrans->bScrambling) |                // Set the scrambling if selected.
            AM_REG_MSPI_CTRL_TXRX(pPIOTrans->eDirection)         |              // Set transmit or receive operation.
            AM_REG_MSPI_CTRL_SENDI(pPIOTrans->bSendInstr)        |            // Enable sending the instruction.
            AM_REG_MSPI_CTRL_SENDA(pPIOTrans->bSendAddr)         |          // Enable sending the address.
            AM_REG_MSPI_CTRL_ENTURN(pPIOTrans->bTurnaround)      |        // Set the turn-around if needed.
            AM_REG_MSPI_CTRL_BIGENDIAN(pMSPIState->bBigEndian)    |      // Set the FIFO endian format.
            AM_REG_MSPI_CTRL_CONT(pPIOTrans->bContinue)          |    // Set for continuation transfer if indicated.
            AM_REG_MSPI_CTRL_QUADCMD(pPIOTrans->bQuadCmd)        |  // Set the Quad Command if indicated.
            AM_REG_MSPI_CTRL_START(1);                            // Start the transfer.
#endif // AM_CMSIS_REGS

        }
        break;
        case AM_HAL_MSPI_TRANS_DMA:
        {
            am_hal_mspi_cq_dma_entry_t    *pDMAEntry = (am_hal_mspi_cq_dma_entry_t *)pCQEntry;
            am_hal_mspi_dma_transfer_t    *pDMATrans = (am_hal_mspi_dma_transfer_t *)pTransaction;

#if AM_CMSIS_REGS
            //
            // Command to set the DMACFG to disable DMA.
            // Need to make sure we disable DMA before we can reprogram
            //
            pDMAEntry->ui32DMACFG2Addr     = (uint32_t)&MSPIn(ui32Module)->DMACFG;
            pDMAEntry->ui32DMACFG2Val      = _VAL2FLD(MSPI_DMACFG_DMAEN, 0);

            //
            // Command to set the DMATARGADDR
            //
            pDMAEntry->ui32DMATARGADDRAddr = (uint32_t)&MSPIn(ui32Module)->DMATARGADDR;
            pDMAEntry->ui32DMATARGADDRVal  = pDMATrans->ui32SRAMAddress;

            //
            // Command to set the DMADEVADDR
            //
            pDMAEntry->ui32DMADEVADDRAddr  = (uint32_t)&MSPIn(ui32Module)->DMADEVADDR;
            pDMAEntry->ui32DMADEVADDRVal   = pDMATrans->ui32DeviceAddress;

            //
            // Command to set the DMATOTALCOUNT
            //
            pDMAEntry->ui32DMATOTCOUNTAddr = (uint32_t)&MSPIn(ui32Module)->DMATOTCOUNT;
            pDMAEntry->ui32DMATOTCOUNTVal  = pDMATrans->ui32TransferCount;

            //
            // Command to set the DMACFG to start DMA.
            //
            pDMAEntry->ui32DMACFG1Addr     = (uint32_t)&MSPIn(ui32Module)->DMACFG;
            pDMAEntry->ui32DMACFG1Val      =
                _VAL2FLD(MSPI_DMACFG_DMAPWROFF, pDMATrans->bPowerOff)   |
                _VAL2FLD(MSPI_DMACFG_DMAPRI, pDMATrans->ui8Priority)    |
                _VAL2FLD(MSPI_DMACFG_DMADIR, pDMATrans->eDirection)     |
                _VAL2FLD(MSPI_DMACFG_DMAEN, 3);
            #if MSPI_USE_CQ
            pDMAEntry->ui32PAUSENAddr = pDMAEntry->ui32PAUSEN2Addr = (uint32_t)&MSPIn(ui32Module)->CQPAUSE;
            pDMAEntry->ui32PAUSEENVal = pDMATrans->ui32PauseCondition | _VAL2FLD(MSPI_CQFLAGS_CQFLAGS, MSPI_CQFLAGS_CQFLAGS_CQIDX);
            pDMAEntry->ui32PAUSEEN2Val = _VAL2FLD(MSPI_CQFLAGS_CQFLAGS, MSPI_CQFLAGS_CQFLAGS_CQIDX);
            pDMAEntry->ui32SETCLRVal = pDMATrans->ui32StatusSetClr;
            pDMAEntry->ui32SETCLRAddr = (uint32_t)&MSPIn(ui32Module)->CQSETCLEAR;
            #endif
#else // AM_CMSIS_REGS
            // Command to set the DMACFG to disable DMA.
            // Need to make sure we disable DMA before we can reprogram
            pDMAEntry->ui32DMACFG2Addr     = AM_REGADDRn(MSPI, ui32Module, DMACFG);
            pDMAEntry->ui32DMACFG2Val      = AM_REG_MSPI_DMACFG_DMAEN(0);
            // Command to set the DMATARGADDR
            pDMAEntry->ui32DMATARGADDRAddr = AM_REGADDRn(MSPI, ui32Module, DMATARGADDR);
            pDMAEntry->ui32DMATARGADDRVal  = pDMATrans->ui32SRAMAddress;

            // Command to set the DMADEVADDR
            pDMAEntry->ui32DMADEVADDRAddr  = AM_REGADDRn(MSPI, ui32Module, DMADEVADDR);
            pDMAEntry->ui32DMADEVADDRVal   = pDMATrans->ui32DeviceAddress;

            // Command to set the DMATOTALCOUNT
            pDMAEntry->ui32DMATOTCOUNTAddr = AM_REGADDRn(MSPI, ui32Module, DMATOTCOUNT);
            pDMAEntry->ui32DMATOTCOUNTVal  = pDMATrans->ui32TransferCount;

            // Command to set the DMACFG to start DMA.
            pDMAEntry->ui32DMACFG1Addr     = AM_REGADDRn(MSPI, ui32Module, DMACFG);
            pDMAEntry->ui32DMACFG1Val      =
            AM_REG_MSPI_DMACFG_DMAPWROFF(pDMATrans->bPowerOff) |
            AM_REG_MSPI_DMACFG_DMAPRI(pDMATrans->ui8Priority) |
            AM_REG_MSPI_DMACFG_DMADIR(pDMATrans->eDirection) |
            AM_REG_MSPI_DMACFG_DMAEN(3);
            #if MSPI_USE_CQ
            pDMAEntry->ui32PAUSENAddr = pDMAEntry->ui32PAUSEN2Addr = AM_REGADDRn(MSPI, ui32Module, CQPAUSE);
            pDMAEntry->ui32PAUSEENVal = pDMATrans->ui32PauseCondition | AM_REG_MSPI_CQFLAGS_CQFLAGS_CQIDX;
            pDMAEntry->ui32PAUSEEN2Val = AM_REG_MSPI_CQFLAGS_CQFLAGS_CQIDX;
            pDMAEntry->ui32SETCLRVal = pDMATrans->ui32StatusSetClr;
            pDMAEntry->ui32SETCLRAddr = AM_REGADDRn(MSPI, ui32Module, CQSETCLEAR);
            #endif
#endif // AM_CMSIS_REGS
        }
        break;
    }
}

//*****************************************************************************
//
//! @brief Writes data to the MSPI FIFO.
//!
//! @param ui32Module - Selects the MSPI module to use (zero or one).
//! @param pui32Data - Pointer to an array of the data to be written.
//! @param ui32NumBytes - Number of BYTES to copy into the FIFO.
//!
//! This function copies data from the array \e pui32Data into the MSPI FIFO.
//!
//! @return HAL status of the operation.
//
//*****************************************************************************
static uint32_t
mspi_fifo_write(uint32_t ui32Module, uint32_t *pui32Data,
                uint32_t ui32NumBytes, uint32_t ui32Timeout)
{
    uint32_t ui32Index;
    uint32_t ui32Status = AM_HAL_STATUS_SUCCESS;

    //
    // Validate parameters
    //
    if ( ui32Module >= AM_REG_MSPI_NUM_MODULES )
    {
        return AM_HAL_STATUS_OUT_OF_RANGE;
    }

    //
    // Loop over the words in the array until we have the correct number of
    // bytes.
    //
    for ( ui32Index = 0; (4 * ui32Index) < ui32NumBytes; ui32Index++ )
    {
        //
        // Write the word to the FIFO.
        //
#if AM_CMSIS_REGS
        MSPIn(ui32Module)->TXFIFO = pui32Data[ui32Index];

        //
        // Wait for the word to go out if there is no room in the FIFO.
        //
        ui32Status = am_hal_flash_delay_status_check(ui32Timeout,
                                                     (uint32_t)&MSPIn(ui32Module)->TXENTRIES,
                                                     MSPI_TXENTRIES_TXENTRIES_Msk,
                                                     _VAL2FLD(MSPI_TXENTRIES_TXENTRIES, AM_HAL_MSPI_MAX_FIFO_SIZE),
                                                     false);
#else // AM_CMSIS_REGS
        AM_REGn(MSPI, ui32Module, TXFIFO) = pui32Data[ui32Index];

        //
        // Wait for the word to go out if there is no room in the FIFO.
        //
        ui32Status = am_hal_flash_delay_status_check(ui32Timeout,
                                                     AM_REGADDRn(MSPI, ui32Module, TXENTRIES),
                                                     AM_REG_MSPI_TXENTRIES_TXENTRIES_M,
                                                     AM_REG_MSPI_TXENTRIES_TXENTRIES(AM_HAL_MSPI_MAX_FIFO_SIZE),
                                                     false);
#endif // AM_CMSIS_REGS
    }

    //
    // Return the status.
    //
    return ui32Status;
}

//*****************************************************************************
//
//! @brief Reads data from the MSPI FIFO.
//!
//! @param ui32Module - Selects the IOM module to use (zero or one).
//! @param pui32Data - Pointer to an array where the FIFO data will be copied.
//! @param ui32NumBytes - Number of bytes to copy into array.
//!
//! This function copies data from the MSPI FIFO into the array \e pui32Data.
//!
//! @return HAL status of the operation.
//
//*****************************************************************************
static uint32_t
mspi_fifo_read(uint32_t ui32Module, uint32_t *pui32Data,
               uint32_t ui32NumBytes, uint32_t ui32Timeout)
{
    am_hal_mspi_buffer(4) sTempBuffer;
    uint32_t i, ui32NumWords, ui32Leftovers;
    uint32_t ui32Status;

    //
    // Validate parameters
    //
    if ( ui32Module >= AM_REG_MSPI_NUM_MODULES )
    {
        return AM_HAL_STATUS_OUT_OF_RANGE;
    }

    //
    // Figure out how many whole words we're reading from the fifo, and how
    // many bytes will be left over when we're done.
    //
    ui32NumWords = ui32NumBytes / 4;
    ui32Leftovers = ui32NumBytes - (ui32NumWords * 4);

    //
    // Copy out as many full words as we can.
    //
    for ( i = 0; i < ui32NumWords; i++ )
    {
        //
        // Wait for additinal entries in the MSPI RX FIFO.
        //
#if AM_CMSIS_REGS
        ui32Status = am_hal_flash_delay_status_check(ui32Timeout,
                                                     (uint32_t)&MSPIn(ui32Module)->RXENTRIES,
                                                     MSPI_RXENTRIES_RXENTRIES_Msk,
                                                     _VAL2FLD(MSPI_RXENTRIES_RXENTRIES, 0),
                                                     false);

#else // AM_CMSIS_REGS
        ui32Status = am_hal_flash_delay_status_check(ui32Timeout,
                                                     AM_REGADDRn(MSPI, ui32Module, RXENTRIES),
                                                     AM_REG_MSPI_RXENTRIES_RXENTRIES_M,
                                                     AM_REG_MSPI_RXENTRIES_RXENTRIES(0),
                                                     false);
#endif // AM_CMSIS_REGS
        //
        // Check for timeout
        //
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return ui32Status;
        }

        //
        // Copy data out of the FIFO, one word at a time.
        //
#if AM_CMSIS_REGS
        pui32Data[i] = MSPIn(ui32Module)->RXFIFO;
#else // AM_CMSIS_REGS
        pui32Data[i] = AM_REGn(MSPI, ui32Module, RXFIFO);
#endif // AM_CMSIS_REGS
    }

    //
    // If there were leftovers, we'll copy them carefully. Pull the last word
    // from the fifo (there should only be one) into a temporary buffer. Also,
    // create an 8-bit pointer to help us copy the remaining bytes one at a
    // time.
    //
    // Note: If the data buffer we were given was truly a word pointer like the
    // definition requests, we wouldn't need to do this. It's possible to call
    // this function with a re-cast or packed pointer instead though. If that
    // happens, we want to be careful not to overwrite any data that might be
    // sitting just past the end of the destination array.
    //
    if ( ui32Leftovers )
    {
        //
        // Wait for additinal entries in the MSPI RX FIFO.
        //
#if AM_CMSIS_REGS
        ui32Status = am_hal_flash_delay_status_check(ui32Timeout,
                                                     (uint32_t)&MSPIn(ui32Module)->RXENTRIES,
                                                     MSPI_RXENTRIES_RXENTRIES_Msk,
                                                     _VAL2FLD(MSPI_RXENTRIES_RXENTRIES, 0),
                                                     false);
#else // AM_CMSIS_REGS
        ui32Status = am_hal_flash_delay_status_check(ui32Timeout,
                                                     AM_REGADDRn(MSPI, ui32Module, RXENTRIES),
                                                     AM_REG_MSPI_RXENTRIES_RXENTRIES_M,
                                                     AM_REG_MSPI_RXENTRIES_RXENTRIES(0),
                                                     false);
#endif // AM_CMSIS_REGS

        //
        // Check for timeout
        //
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return ui32Status;
        }

        //
        // Read the next word from the RX FIFO.
        //
#if AM_CMSIS_REGS
        sTempBuffer.words[0] = MSPIn(ui32Module)->RXFIFO;
#else // AM_CMSIS_REGS
        sTempBuffer.words[0] = AM_REGn(MSPI, ui32Module, RXFIFO);
#endif // AM_CMSIS_REGS
        uint8_t *pui8Data;
        pui8Data = (uint8_t *) (&pui32Data[i]);

        //
        // If we had leftover bytes, copy them out one byte at a time.
        //
        for ( int j = 0; j < ui32Leftovers; j++ )
        {
            pui8Data[j] = sTempBuffer.bytes[j];
        }
    }

    return AM_HAL_STATUS_SUCCESS;
}

#if !MSPI_USE_CQ
static void
run_txn_cmdlist(void *pCQEntry, uint32_t numEntries)
{
    uint32_t ix;
    am_hal_cmdq_entry_t *pCmd = (am_hal_cmdq_entry_t *)pCQEntry;

    for ( ix = 0; ix < numEntries; ix++, pCmd++ )
    {
        *((uint32_t *)pCmd->address) = pCmd->value;
    }
} // run_txn_cmdlist()

static uint32_t
mspi_dma_add_transaction(void *pHandle,
                         am_hal_mspi_dma_transfer_t *psTransaction,
                         am_hal_mspi_callback_t pfnCallback,
                         void *pCallbackCtxt)
{
    am_hal_mspi_state_t         *pMSPIState = (am_hal_mspi_state_t *)pHandle;
    am_hal_mspi_cq_dma_entry_t  *pCQEntry;
    uint32_t  index = pMSPIState->ui32NextIdx % pMSPIState->ui32MaxTransactions;

    //
    // Check to see if there is enough room in the queue
    //
    if ( pMSPIState->ui32NumCQEntries == pMSPIState->ui32MaxTransactions )
    {
        return AM_HAL_STATUS_OUT_OF_RANGE;
    }

    pCQEntry = &pMSPIState->pTransactions[index];


    build_dma_cmdlist(pMSPIState, AM_HAL_MSPI_TRANS_DMA, pCQEntry, (void *)psTransaction);

    //
    // Store the callback function pointer.
    //
    pMSPIState->pfnCallback[index] = pfnCallback;
    pMSPIState->pCallbackCtxt[index] = pCallbackCtxt;
    pMSPIState->ui32NextIdx++;
    return AM_HAL_STATUS_SUCCESS;
} // am_hal_mspi_DmaAddTransaction()

#else


//*****************************************************************************
//
//! @brief Initializes the MSPI Command Queue.
//!
//! @param handle       - handle for the interface.
//! @param ui32Length   - length of the SRAM Command Queue buffer in words.
//! @param pTCB         - pointer to the SRAM to use for the Command Queue.
//!
//! This function initializes the global command queue structure.
//!
//! @return HAL status of the operation.
//
//
//*****************************************************************************
static uint32_t
mspi_cq_init(uint32_t ui32Module, uint32_t ui32Length,
             uint32_t *pTCB)
{
    am_hal_cmdq_cfg_t cqCfg;

    cqCfg.pCmdQBuf = pTCB;
    cqCfg.cmdQSize = ui32Length / 2;
    cqCfg.priority = AM_HAL_CMDQ_PRIO_HI;
    return am_hal_cmdq_init((am_hal_cmdq_if_e)(AM_HAL_CMDQ_IF_MSPI + ui32Module),
                            &cqCfg, &g_MSPIState[ui32Module].CQ.pCmdQHdl);
}

//*****************************************************************************
//
//! @brief Resets the MSPI Command Queue.
//!
//! @param ui32Module   - MSPI instance.
//!
//! This function resets the global command queue structure.
//!
//! @return HAL status of the operation.
//
//
//*****************************************************************************
static uint32_t
mspi_cq_reset(void *pHandle)
{
    am_hal_mspi_state_t           *pMSPIState = (am_hal_mspi_state_t *)pHandle;
    uint32_t                      ui32Module = pMSPIState->ui32Module;

    if (g_MSPIState[ui32Module].CQ.pCmdQHdl)
    {
        am_hal_cmdq_term(g_MSPIState[ui32Module].CQ.pCmdQHdl, true);
        g_MSPIState[ui32Module].CQ.pCmdQHdl = NULL;
    }

    //
    // Return the status.
    //
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief Adds a transaction the MSPI Command Queue.
//!
//! @param handle       - handle for the interface.
//! @param pTransaction - transaction to add to the CQ
//! @param pfnCallback  - pointer the callback function to be executed when
//!                       transaction is complete.
//! @param pCallbackCtxt- pointer to the state/context to pass to callback
//!                       function.
//!
//! This function copies data from the IOM FIFO into the array \e pui32Data.
//! This is how input data from SPI or I2C transactions may be retrieved.
//!
//!
//! @return HAL status of the operation.
//
//
//*****************************************************************************
static uint32_t
mspi_cq_add_transaction(void *pHandle,
                        void *pTransaction,
                        am_hal_mspi_trans_e eMode,
                        am_hal_mspi_callback_t pfnCallback,
                        void *pCallbackCtxt)
{
    am_hal_mspi_state_t     *pMSPIState = (am_hal_mspi_state_t *)pHandle;
    am_hal_cmdq_entry_t     *pCQBlock;
    uint32_t                index;
    uint32_t                 size;
    am_hal_mspi_CQ_t        *pCQ = &pMSPIState->CQ;
    uint32_t                ui32Status = AM_HAL_STATUS_SUCCESS;

    //
    // Determine the transfer mode and set up accordingly.
    //
    switch(eMode)
    {
        case AM_HAL_MSPI_TRANS_PIO:
            size = sizeof(am_hal_mspi_cq_pio_entry_t);
            break;
        case AM_HAL_MSPI_TRANS_DMA:
            size = sizeof(am_hal_mspi_cq_dma_entry_t);
            break;
    }

    //
    // Check to see if there is enough room in the CQ
    //
    if ((pMSPIState->ui32NumCQEntries == AM_HAL_MSPI_MAX_CQ_ENTRIES) ||
        (am_hal_cmdq_alloc_block(pCQ->pCmdQHdl, size / 8, &pCQBlock, &index)))
    {
        return AM_HAL_STATUS_OUT_OF_RANGE;
    }

    build_dma_cmdlist(pMSPIState, eMode, pCQBlock, pTransaction);

    //
    // Because we set AM_HAL_IOM_CQUPD_INT_FLAG, an interrupt will occur once
    // we reach this point in the Command Queue.  In the service routine, we'll
    // look for the appropriate callback.
    //
    // If ENDIDX has been reached, the CQ will pause here. Otherwise will
    // continue with the next CQ entry.
    //

    //
    // Store the callback function pointer.
    //
    pMSPIState->pfnCallback[index & (AM_HAL_MSPI_MAX_CQ_ENTRIES - 1)] = pfnCallback;
    pMSPIState->pCallbackCtxt[index & (AM_HAL_MSPI_MAX_CQ_ENTRIES - 1)] = pCallbackCtxt;

    //
    // Return the status.
    //
    return ui32Status;
}

//*****************************************************************************
//
//! @brief Enable the Command Queue operation.
//!
//! @param handle       - handle for the interface.
//!
//! This function enables Command Queue operation.
//!
//!
//! @return HAL status of the operation.
//
//
//*****************************************************************************
static uint32_t
mspi_cq_enable(void *pHandle)
{
    am_hal_mspi_state_t *pMSPIState = (am_hal_mspi_state_t *)pHandle;
    uint32_t            ui32Module = pMSPIState->ui32Module;

    //
    // Enable the Command Queue operation
    //
    return am_hal_cmdq_enable(g_MSPIState[ui32Module].CQ.pCmdQHdl);
}
//*****************************************************************************
//
//! @brief Disable the Command Queue operation.
//!
//! @param handle       - handle for the interface.
//!
//! This function disables the Command Queue operation.
//!
//!
//! @return HAL status of the operation.
//
//
//*****************************************************************************
static uint32_t
mspi_cq_disable(void *pHandle)
{
    am_hal_mspi_state_t  *pMSPIState = (am_hal_mspi_state_t *)pHandle;
    uint32_t            ui32Module = pMSPIState->ui32Module;

    //
    // Disable the Command Queue operation
    //
    return am_hal_cmdq_disable(g_MSPIState[ui32Module].CQ.pCmdQHdl);
}

#endif

//*****************************************************************************
//
//! @brief Configure the internal PADs.
//!
//! @param handle       - handle for the interface.
//! @param eMSPIDevice  - external device configuration for MSPI
//! This function configures the internal pads based on the external device
//! configuration.
//!
//!
//! @return HAL status of the operation.
//
//
//*****************************************************************************
static uint32_t
mspi_pad_configure(void *pHandle,
                   am_hal_mspi_device_e eMSPIDevice)
{
    am_hal_mspi_state_t           *pMSPIState = (am_hal_mspi_state_t *)pHandle;
    uint32_t                      ui32Module = pMSPIState->ui32Module;

    // MSPI PADOUTEN:
    // Bit 0 - 7 : DS0 - DS7
    // Bit 8     : CLK
    switch ( eMSPIDevice )
    {
        case AM_HAL_MSPI_FLASH_SERIAL_CE0:
        case AM_HAL_MSPI_FLASH_DUAL_CE0:
#if AM_CMSIS_REGS
            MSPIn(ui32Module)->PADCFG = 0;
            MSPIn(ui32Module)->PADOUTEN = 0x103;
            break;
        case AM_HAL_MSPI_FLASH_SERIAL_CE1:
        case AM_HAL_MSPI_FLASH_DUAL_CE1:
            MSPIn(ui32Module)->PADCFG = 0;
            MSPIn(ui32Module)->PADOUTEN = 0x130;
            break;
        case AM_HAL_MSPI_FLASH_QUAD_CE0:
            MSPIn(ui32Module)->PADCFG = 0;
            MSPIn(ui32Module)->PADOUTEN = 0x10F;
            break;
        case AM_HAL_MSPI_FLASH_QUAD_CE1:
            MSPIn(ui32Module)->PADCFG = 0;
            MSPIn(ui32Module)->PADOUTEN = 0x1F0;
            break;
        case AM_HAL_MSPI_FLASH_OCTAL_CE0:
        case AM_HAL_MSPI_FLASH_OCTAL_CE1:
        case AM_HAL_MSPI_FLASH_QUADPAIRED:
            MSPIn(ui32Module)->PADCFG = 0;
            MSPIn(ui32Module)->PADOUTEN = 0x1FF;
            break;
        case AM_HAL_MSPI_FLASH_QUADPAIRED_SERIAL:
            MSPIn(ui32Module)->PADCFG = 0;
            MSPIn(ui32Module)->PADOUTEN = 0x133;
            break;
#else // AM_CMSIS_REGS
            AM_REGn(MSPI, ui32Module, PADCFG) = 0;
            AM_REGn(MSPI, ui32Module, PADOUTEN) = 0x103;
            break;
        case AM_HAL_MSPI_FLASH_SERIAL_CE1:
        case AM_HAL_MSPI_FLASH_DUAL_CE1:
            AM_REGn(MSPI, ui32Module, PADCFG) = 0;
            AM_REGn(MSPI, ui32Module, PADOUTEN) = 0x130;
            break;
        case AM_HAL_MSPI_FLASH_QUAD_CE0:
            AM_REGn(MSPI, ui32Module, PADCFG) = 0;
            AM_REGn(MSPI, ui32Module, PADOUTEN) = 0x10F;
            break;
        case AM_HAL_MSPI_FLASH_QUAD_CE1:
            AM_REGn(MSPI, ui32Module, PADCFG) = 0;
            AM_REGn(MSPI, ui32Module, PADOUTEN) = 0x1F0;
            break;
        case AM_HAL_MSPI_FLASH_OCTAL_CE0:
        case AM_HAL_MSPI_FLASH_OCTAL_CE1:
        case AM_HAL_MSPI_FLASH_QUADPAIRED:
            AM_REGn(MSPI, ui32Module, PADCFG) = 0;
            AM_REGn(MSPI, ui32Module, PADOUTEN) = 0x1FF;
            break;
        case AM_HAL_MSPI_FLASH_QUADPAIRED_SERIAL:
            AM_REGn(MSPI, ui32Module, PADCFG) = 0;
            AM_REGn(MSPI, ui32Module, PADOUTEN) = 0x133;
            break;
#endif // AM_CMSIS_REGS
    }

    //
    // Return the status.
    //
    return AM_HAL_STATUS_SUCCESS;
}
//*****************************************************************************
//
// External Functions.
//
//*****************************************************************************

//
// MSPI initialization function
//
uint32_t
am_hal_mspi_initialize(uint32_t ui32Module, void **ppHandle)
{
#ifndef AM_HAL_DISABLE_API_VALIDATION
    //
    // Check that the request module is in range.
    //
    if (ui32Module >= AM_REG_MSPI_NUM_MODULES )
    {
        return AM_HAL_STATUS_OUT_OF_RANGE;
    }

    //
    // Check for valid arguements.
    //
    if (!ppHandle)
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    //
    // Check if the handle is unallocated.
    //
    if (g_MSPIState[ui32Module].prefix.s.bInit)
    {
        return AM_HAL_STATUS_INVALID_OPERATION;
    }
#endif

    //
    // Initialize the handle.
    //
    g_MSPIState[ui32Module].prefix.s.bInit = true;
    g_MSPIState[ui32Module].prefix.s.magic = AM_HAL_MAGIC_MSPI;
    g_MSPIState[ui32Module].ui32Module = ui32Module;

    //
    // Return the handle.
    //
    *ppHandle = (void *)&g_MSPIState[ui32Module];

    //
    // Return the status.
    //
    return AM_HAL_STATUS_SUCCESS;
}

//
// MSPI Disable function
//
uint32_t
am_hal_mspi_disable(void *pHandle)
{
    am_hal_mspi_state_t           *pMSPIState = (am_hal_mspi_state_t *)pHandle;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    //
    // Check the handle.
    //
    if (!AM_HAL_MSPI_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif

    if (!pMSPIState->prefix.s.bEnable)
    {
        return AM_HAL_STATUS_SUCCESS;
    }

#if MSPI_USE_CQ

    //
    // Disable the Command Queue.
    //
    mspi_cq_disable(pHandle);

    //
    // Reset the Command Queue.
    //
    mspi_cq_reset(pHandle);

#endif // MSPI_USE_CQ

    pMSPIState->prefix.s.bEnable = false;

    //
    // Return the status.
    //
    return AM_HAL_STATUS_SUCCESS;
}


//
// MSPI Deinitialize function
//
uint32_t
am_hal_mspi_deinitialize(void *pHandle)
{
    am_hal_mspi_state_t           *pMSPIState = (am_hal_mspi_state_t *)pHandle;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    //
    // Check the handle.
    //
    if (!AM_HAL_MSPI_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif

    if (pMSPIState->prefix.s.bEnable)
    {
        am_hal_mspi_disable(pHandle);
    }

    //
    // Reset the handle.
    //
    pMSPIState->prefix.s.bInit = false;
    pMSPIState->ui32Module = 0;
    pMSPIState->eDeviceConfig = (am_hal_mspi_device_e)0;

    //
    // Return the status.
    //
    return AM_HAL_STATUS_SUCCESS;
}

//
// MSPI device configuration function
//
uint32_t
am_hal_mspi_device_configure(void *pHandle,
                             am_hal_mspi_dev_config_t *pConfig)
{
    am_hal_mspi_state_t           *pMSPIState = (am_hal_mspi_state_t *)pHandle;
    uint32_t                      ui32Module = pMSPIState->ui32Module;
    uint32_t                      ui32Config = 0;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    //
    // Check the handle.
    //
    if (!AM_HAL_MSPI_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }

    //
    // Configure not allowed in Enabled state
    //
    if (pMSPIState->prefix.s.bEnable)
    {
        return AM_HAL_STATUS_INVALID_OPERATION;
    }
#endif

    //
    // Set the external flash device configuration.
    //
#if AM_CMSIS_REGS
    ui32Config = _VAL2FLD(MSPI_CFG_DEVCFG, pConfig->eDeviceConfig);

    //
    // If separate MOSI/MISO, then configure.
    //
    if ( pConfig->bSeparateIO )
    {
        ui32Config |= _VAL2FLD(MSPI_CFG_SEPIO, 1);
    }

    //
    // Set the clock polarity and phase based on SPI mode.
    //
    switch(pConfig->eSpiMode)
    {
        case AM_HAL_MSPI_SPI_MODE_0:                  // CPOL = 0; CPHA = 0
            ui32Config |= _VAL2FLD(MSPI_CFG_CPOL, 0)    |
                          _VAL2FLD(MSPI_CFG_CPHA, 0);
            break;
        case AM_HAL_MSPI_SPI_MODE_2:                  // CPOL = 1; CPHA = 0
            ui32Config |= _VAL2FLD(MSPI_CFG_CPOL, 1)    |
                          _VAL2FLD(MSPI_CFG_CPHA, 0);
            break;
        case AM_HAL_MSPI_SPI_MODE_1:                  // CPOL = 0; CPHA = 1
            ui32Config |= _VAL2FLD(MSPI_CFG_CPOL, 0)    |
                          _VAL2FLD(MSPI_CFG_CPHA, 1);
            break;
        case AM_HAL_MSPI_SPI_MODE_3:                  // CPOL = 1; CPHA = 1
            ui32Config |= _VAL2FLD(MSPI_CFG_CPOL, 1)    |
                          _VAL2FLD(MSPI_CFG_CPHA, 1);
            break;
    }

    //
    // Set the number of turn-around cycles.
    //
    ui32Config |= _VAL2FLD(MSPI_CFG_TURNAROUND, pConfig->ui8TurnAround);

    //
    // Set the address configuration.
    //
    ui32Config |= _VAL2FLD(MSPI_CFG_ASIZE, pConfig->eAddrCfg);

    //
    // Set the instruction configuration.
    //
    ui32Config |= _VAL2FLD(MSPI_CFG_ISIZE, pConfig->eInstrCfg);

    //
    // Set the configuration in the MSPI peripheral.
    //
    MSPIn(ui32Module)->CFG = ui32Config;

    //
    // Set the clock divisor to get the desired MSPI clock frequency.
    //
    MSPIn(ui32Module)->MSPICFG_b.CLKDIV = pConfig->eClockFreq;

    //
    // Adjust the clock edge configuration depending upon the clock frequency.
    //
    if ( pConfig->eClockFreq == AM_HAL_MSPI_CLK_48MHZ )
    {
        MSPIn(ui32Module)->MSPICFG_b.TXNEG = 1;
        MSPIn(ui32Module)->MSPICFG_b.RXNEG = 0;
        MSPIn(ui32Module)->MSPICFG_b.RXCAP = 1;
    }
    else
    {
        MSPIn(ui32Module)->MSPICFG_b.TXNEG = 0;
        MSPIn(ui32Module)->MSPICFG_b.RXNEG = 0;
        MSPIn(ui32Module)->MSPICFG_b.RXCAP = 1;
    }

    //
    // Set the APBCLK for continuous operation.
    //
    MSPIn(ui32Module)->MSPICFG_b.APBCLK = 0;

    //
    // Reset the register storage for next write.
    //
    ui32Config = 0;

    //
    // Set whether to send an instruction.
    //
    if ( pConfig->bSendInstr )
    {
        ui32Config |= _VAL2FLD(MSPI_FLASH_XIPSENDI, 1);
    }

    //
    // Set whether to send an address.
    //
    if ( pConfig->bSendAddr )
    {
        ui32Config |= _VAL2FLD(MSPI_FLASH_XIPSENDA, 1);
    }

    //
    // Set whether to enable the TX to RX turnaround.
    //
    if ( pConfig->bTurnaround )
    {
        ui32Config |= _VAL2FLD(MSPI_FLASH_XIPENTURN, 1);
    }

    //
    // Set to Little Endian mode by default.
    //
    ui32Config |= _VAL2FLD(MSPI_FLASH_XIPBIGENDIAN, pMSPIState->bBigEndian);

    //
    // Set the XIP ACK value to default to 1's during latency period.
    //
    ui32Config |= _VAL2FLD(MSPI_FLASH_XIPACK, MSPI_FLASH_XIPACK_TERMINATE);

    //
    // Set the read instruction.
    //
    ui32Config |= _VAL2FLD(MSPI_FLASH_READINSTR, pConfig->ui8ReadInstr);

    //
    // Set the write instruction.
    //
    ui32Config |= _VAL2FLD(MSPI_FLASH_WRITEINSTR, pConfig->ui8WriteInstr);

    //
    // Set the configuration in the MSPI peripheral.
    //
    MSPIn(ui32Module)->FLASH = ui32Config;

    g_MSPIState[ui32Module].pTCB = pConfig->pTCB;
    g_MSPIState[ui32Module].ui32TCBSize = pConfig->ui32TCBSize;

    if (pConfig->pTCB)
    {
        // set the DMABCOUNT
        MSPIn(ui32Module)->DMABCOUNT = AM_HAL_MSPI_DEFAULT_BURST_COUNT;

        // set the DMATHRESH
        MSPIn(ui32Module)->DMATHRESH = AM_HAL_MSPI_DEFAULT_BURST_COUNT >> 2;
    }
    //
    // Reset the register storage for next write.
    //
    ui32Config = 0;

    //
    // Set the scrambling start and end addresses aligned to 64K region.
    //
    MSPIn(ui32Module)->SCRAMBLING =
        _VAL2FLD(MSPI_SCRAMBLING_SCRSTART, pConfig->scramblingStartAddr >> 16) |
        _VAL2FLD(MSPI_SCRAMBLING_SCREND, pConfig->scramblingEndAddr >> 16);

    //
    // Set the selected IOM to disable.
    //
    MSPIn(ui32Module)->MSPICFG_b.IOMSEL = 7;
#else // AM_CMSIS_REGS
    ui32Config = AM_REG_MSPI_CFG_DEVCFG(pConfig->eDeviceConfig);

    //
    // If separate MOSI/MISO, then configure.
    //
    if ( pConfig->bSeparateIO )
    {
        ui32Config |= AM_REG_MSPI_CFG_SEPIO(1);
    }

    //
    // Set the clock polarity and phase based on SPI mode.
    //
    switch(pConfig->eSpiMode)
    {
        case AM_HAL_MSPI_SPI_MODE_0:                  // CPOL = 0; CPHA = 0
            ui32Config |= AM_REG_MSPI_CFG_CPOL(0) | AM_REG_MSPI_CFG_CPHA(0);
            break;
        case AM_HAL_MSPI_SPI_MODE_2:                  // CPOL = 1; CPHA = 0
            ui32Config |= AM_REG_MSPI_CFG_CPOL(1) | AM_REG_MSPI_CFG_CPHA(0);
            break;
        case AM_HAL_MSPI_SPI_MODE_1:                  // CPOL = 0; CPHA = 1
            ui32Config |= AM_REG_MSPI_CFG_CPOL(0) | AM_REG_MSPI_CFG_CPHA(1);
            break;
        case AM_HAL_MSPI_SPI_MODE_3:                  // CPOL = 1; CPHA = 1
            ui32Config |= AM_REG_MSPI_CFG_CPOL(1) | AM_REG_MSPI_CFG_CPHA(1);
            break;
    }

    //
    // Set the number of turn-around cycles.
    //
    ui32Config |= AM_REG_MSPI_CFG_TURNAROUND(pConfig->ui8TurnAround);

    //
    // Set the address configuration.
    //
    ui32Config |= AM_REG_MSPI_CFG_ASIZE(pConfig->eAddrCfg);

    //
    // Set the instruction configuration.
    //
    ui32Config |= AM_REG_MSPI_CFG_ISIZE(pConfig->eInstrCfg);

    //
    // Set the configuration in the MSPI peripheral.
    //
    AM_REGn(MSPI, ui32Module, CFG) = ui32Config;

    //
    // Set the clock divisor to get the desired MSPI clock frequency.
    //
    AM_BFWn(MSPI, ui32Module, MSPICFG, CLKDIV, pConfig->eClockFreq);

    //
    // Adjust the clock edge configuration depending upon the clock frequency.
    //
    if ( pConfig->eClockFreq == AM_HAL_MSPI_CLK_48MHZ )
    {
        AM_BFWn(MSPI, ui32Module, MSPICFG, TXNEG, 1);
        AM_BFWn(MSPI, ui32Module, MSPICFG, RXNEG, 0);
        AM_BFWn(MSPI, ui32Module, MSPICFG, RXCAP, 1);
    }
    else
    {
        AM_BFWn(MSPI, ui32Module, MSPICFG, TXNEG, 0);
        AM_BFWn(MSPI, ui32Module, MSPICFG, RXNEG, 0);
        AM_BFWn(MSPI, ui32Module, MSPICFG, RXCAP, 1);
    }

    //
    // Set the APBCLK for continuous operation.
    //
  AM_BFWn(MSPI, ui32Module, MSPICFG, APBCLK, 0);

    //
    // Reset the register storage for next write.
    //
    ui32Config = 0;

    //
    // Set whether to send an instruction.
    //
    if ( pConfig->bSendInstr )
    {
        ui32Config |= AM_REG_MSPI_FLASH_XIPSENDI(1);
    }

    //
    // Set whether to send an address.
    //
    if ( pConfig->bSendAddr )
    {
        ui32Config |= AM_REG_MSPI_FLASH_XIPSENDA(1);
    }

    //
    // Set whether to enable the TX to RX turnaround.
    //
    if ( pConfig->bTurnaround )
    {
        ui32Config |= AM_REG_MSPI_FLASH_XIPENTURN(1);
    }

    //
    // Set to Little Endian mode by default.
    //
    ui32Config |= AM_REG_MSPI_FLASH_XIPBIGENDIAN(pMSPIState->bBigEndian);

    //
    // Set the XIP ACK value to default to 1's during latency period.
    //
    ui32Config |= AM_REG_MSPI_FLASH_XIPACK_TERMINATE;

    //
    // Set the read instruction.
    //
    ui32Config |= AM_REG_MSPI_FLASH_READINSTR(pConfig->ui8ReadInstr);

    //
    // Set the write instruction.
    //
    ui32Config |= AM_REG_MSPI_FLASH_WRITEINSTR(pConfig->ui8WriteInstr);

    //
    // Set the configuration in the MSPI peripheral.
    //
    AM_REGn(MSPI, ui32Module, FLASH) = ui32Config;

    g_MSPIState[ui32Module].pTCB = pConfig->pTCB;
    g_MSPIState[ui32Module].ui32TCBSize = pConfig->ui32TCBSize;

    if (pConfig->pTCB)
    {
        // set the DMABCOUNT
        AM_REGn(MSPI, ui32Module, DMABCOUNT) = AM_HAL_MSPI_DEFAULT_BURST_COUNT;

        // set the DMATHRESH
        AM_REGn(MSPI, ui32Module, DMATHRESH) = AM_HAL_MSPI_DEFAULT_BURST_COUNT >> 2;
    }

    //
    // Reset the register storage for next write.
    //
    ui32Config = 0;

    //
    // Set the scrambling start and end addresses aligned to 64K region.
    //
    AM_REGn(MSPI, ui32Module, SCRAMBLING) = AM_REG_MSPI_SCRAMBLING_SCRSTART(pConfig->scramblingStartAddr >> 16) |
    AM_REG_MSPI_SCRAMBLING_SCREND(pConfig->scramblingEndAddr >> 16);

    //
    // Set the selected IOM to disable.
    //
    AM_BFWn(MSPI, ui32Module, MSPICFG, IOMSEL, 7);
#endif // AM_CMSIS_REGS

    //
    // Configure the MSPI PADs.
    //
    mspi_pad_configure(pHandle, pConfig->eDeviceConfig);

    //
    // Set the default endianess for the FIFO.
    //
    pMSPIState->bBigEndian = false;

    //
    // Set the default maximum delay timeout value.
    //
    pMSPIState->waitTimeout = 10000;

    //
    // Return the status.
    //
    return AM_HAL_STATUS_SUCCESS;
}

//
// MSPI device configuration function
//
uint32_t
am_hal_mspi_enable(void *pHandle)
{
    am_hal_mspi_state_t           *pMSPIState = (am_hal_mspi_state_t *)pHandle;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    //
    // Check the handle.
    //
    if (!AM_HAL_MSPI_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif

    if (pMSPIState->pTCB)
    {
        pMSPIState->ui32LastIdxProcessed = 0;
        pMSPIState->ui32NumCQEntries = 0;

#if MSPI_USE_CQ

        //
        // Initialize the Command Queue service with memory supplied by the application.
        //
        mspi_cq_init(pMSPIState->ui32Module, pMSPIState->ui32TCBSize, pMSPIState->pTCB);

#else
        // Use the buffer for software queuing for DMA
        // Determine the maximum number of transactions based on the memory provided
        pMSPIState->ui32MaxTransactions = pMSPIState->ui32TCBSize * 4 / sizeof(am_hal_mspi_cq_dma_entry_t);
        if (pMSPIState->ui32MaxTransactions > 0)
        {
            if (pMSPIState->ui32MaxTransactions > AM_HAL_MSPI_MAX_CQ_ENTRIES)
            {
                pMSPIState->ui32MaxTransactions = AM_HAL_MSPI_MAX_CQ_ENTRIES;
            }
            pMSPIState->ui32NextIdx = pMSPIState->ui32LastIdxProcessed + 1;
            pMSPIState->pTransactions = (am_hal_mspi_cq_dma_entry_t *)pMSPIState->pTCB;
        }

#endif // MSPI_USE_CQ
    }

    pMSPIState->prefix.s.bEnable = true;

    //
    // Return the status.
    //
    return AM_HAL_STATUS_SUCCESS;
}

//
// MSPI device specific control function.
//
uint32_t am_hal_mspi_control(void *pHandle,
                             am_hal_mspi_request_e eRequest,
                             void *pConfig)
{
    am_hal_mspi_state_t           *pMSPIState = (am_hal_mspi_state_t *)pHandle;
    uint32_t                      ui32Module = pMSPIState->ui32Module;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    //
    // Check the handle.
    //
    if ( !AM_HAL_MSPI_CHK_HANDLE(pHandle) )
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }

    //
    // Validate the parameters
    //
    if (eRequest > AM_HAL_MSPI_REQ_MAX)
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }
#endif

#if AM_CMSIS_REGS
    switch(eRequest)
    {
        case AM_HAL_MSPI_REQ_APBCLK:
            if (!pConfig)
            {
                return AM_HAL_STATUS_INVALID_ARG;
            }

            //
            // Enable/Disable APBCLK.
            //
            MSPIn(ui32Module)->MSPICFG_b.APBCLK = *((uint32_t *)pConfig);
            break;

        case AM_HAL_MSPI_REQ_FLAG_SETCLR:
            if (!pConfig)
            {
                return AM_HAL_STATUS_INVALID_ARG;
            }

            MSPIn(ui32Module)->CQSETCLEAR = *((uint32_t *)pConfig);
            break;

        case AM_HAL_MSPI_REQ_LINK_IOM:
            if (!pConfig)
            {
                return AM_HAL_STATUS_INVALID_ARG;
            }

            //
            // Set the Linked IOM
            //
            MSPIn(ui32Module)->MSPICFG_b.IOMSEL = *((uint32_t *)pConfig);
            break;

        case AM_HAL_MSPI_REQ_SCRAMB_DIS:
            //
            // Disable scrambling.
            //
            MSPIn(ui32Module)->SCRAMBLING_b.SCRENABLE = 0;
            break;

        case AM_HAL_MSPI_REQ_SCRAMB_EN:
            //
            // Enable scrambling.
            //
            MSPIn(ui32Module)->SCRAMBLING_b.SCRENABLE = 1;
            break;

        case AM_HAL_MSPI_REQ_XIPACK:
            if (!pConfig)
            {
                return AM_HAL_STATUS_INVALID_ARG;
            }
            //
            // Enable/Disable XIPACK.
            //
            MSPIn(ui32Module)->FLASH_b.XIPACK = *((uint32_t *)pConfig);
            break;

        case AM_HAL_MSPI_REQ_XIP_DIS:
            //
            // Disable XIP.
            //
            MSPIn(ui32Module)->FLASH_b.XIPEN = 0;
            break;

        case AM_HAL_MSPI_REQ_XIP_EN:
            //
            // Enable XIP.
            //
            MSPIn(ui32Module)->FLASH_b.XIPEN = 1;
            break;

        default:
            return AM_HAL_STATUS_INVALID_ARG;
    }
#else // AM_CMSIS_REGS
    switch(eRequest)
    {
        case AM_HAL_MSPI_REQ_APBCLK:
            if (!pConfig)
            {
                return AM_HAL_STATUS_INVALID_ARG;
            }

            //
            // Enable/Disable APBCLK.
            //
            AM_BFWn(MSPI, ui32Module, MSPICFG, APBCLK, *((uint32_t *)pConfig));
            break;

        case AM_HAL_MSPI_REQ_FLAG_SETCLR:
            if (!pConfig)
            {
                return AM_HAL_STATUS_INVALID_ARG;
            }
            AM_REGn(MSPI, ui32Module, CQSETCLEAR) = *((uint32_t *)pConfig);
            break;

        case AM_HAL_MSPI_REQ_LINK_IOM:
            if (!pConfig)
            {
                return AM_HAL_STATUS_INVALID_ARG;
            }
            //
            // Set the Linked IOM
            //
            AM_BFWn(MSPI, ui32Module, MSPICFG, IOMSEL, *((uint32_t *)pConfig));
            break;

        case AM_HAL_MSPI_REQ_SCRAMB_DIS:
            //
            // Disable scrambling.
            //
            AM_BFWn(MSPI, ui32Module, SCRAMBLING, SCRENABLE, 0);
            break;

        case AM_HAL_MSPI_REQ_SCRAMB_EN:
            //
            // Enable scrambling.
            //
            AM_BFWn(MSPI, ui32Module, SCRAMBLING, SCRENABLE, 1);
            break;

        case AM_HAL_MSPI_REQ_XIPACK:
            if (!pConfig)
            {
                return AM_HAL_STATUS_INVALID_ARG;
            }
            //
            // Enable/Disable XIPACK.
            //
            AM_BFWn(MSPI, ui32Module, FLASH, XIPACK, *((uint32_t *)pConfig));
            break;

        case AM_HAL_MSPI_REQ_XIP_DIS:
            //
            // Disable XIP.
            //
            AM_BFWn(MSPI, ui32Module, FLASH, XIPEN, 0);
            break;

        case AM_HAL_MSPI_REQ_XIP_EN:
            //
            // Enable XIP.
            //
            AM_BFWn(MSPI, ui32Module, FLASH, XIPEN, 1);
            break;

        default:
            return AM_HAL_STATUS_INVALID_ARG;
    }
#endif // AM_CMSIS_REGS

    //
    // Return the status.
    //
    return AM_HAL_STATUS_SUCCESS;
}

//
// MSPI get capabilities
//
uint32_t am_hal_mspi_capabilities_get(void *pHandle,
                                      am_hal_mspi_capabilities_t **pCapabilities)
{
  am_hal_mspi_state_t           *pMSPIState = (am_hal_mspi_state_t *)pHandle;
  uint32_t                      ui32Module = pMSPIState->ui32Module;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    //
    // Check the handle.
    //
    if (!AM_HAL_MSPI_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif

    //
    // copy the pointer the MSPI instance capabilities into the passed pointer
    //
    *pCapabilities = &g_MSPIState[ui32Module].capabilities;

    //
    // Return the status.
    //
    return AM_HAL_STATUS_SUCCESS;
}

//
// MSPI blocking transfer function
//
uint32_t am_hal_mspi_blocking_transfer(void *pHandle,
                                       am_hal_mspi_pio_transfer_t *pTransaction,
                                       uint32_t ui32Timeout)
{
    am_hal_mspi_state_t           *pMSPIState = (am_hal_mspi_state_t *)pHandle;
    uint32_t                      ui32Module = pMSPIState->ui32Module;
    uint32_t                      ui32Control = 0;
    uint32_t                      ui32Status = AM_HAL_STATUS_SUCCESS;
    uint32_t                      intMask;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    //
    // Check the handle.
    //
    if (!AM_HAL_MSPI_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif

    // Make sure there is no non-blocking transaction in progress
    if (pMSPIState->ui32NumCQEntries)
    {
        return AM_HAL_STATUS_INVALID_OPERATION;
    }

#if AM_CMSIS_REGS
    //
    // Set the number of bytes to transfer.
    //
    ui32Control |= _VAL2FLD(MSPI_CTRL_XFERBYTES, pTransaction->ui32NumBytes);

    //
    // Set the PIO default to scrambling disabled.
    //
    ui32Control |= _VAL2FLD(MSPI_CTRL_PIOSCRAMBLE, pTransaction->bScrambling);

    //
    // Set transmit or receive operation.
    //
    ui32Control |= _VAL2FLD(MSPI_CTRL_TXRX, pTransaction->eDirection);

    //
    // Set the indication to send an instruction and set the instruction value if
    // we have a valid instruction.
    //
    if ( pTransaction->bSendInstr )
    {
        ui32Control |= _VAL2FLD(MSPI_CTRL_SENDI, 1);
        MSPIn(ui32Module)->INSTR =
            _VAL2FLD(MSPI_INSTR_INSTR, pTransaction->ui16DeviceInstr);
    }

    //
    // Set the inidication to send an address and set the address value if we have
    // a valid address.
    //
    if ( pTransaction->bSendAddr )
    {
        ui32Control |= _VAL2FLD(MSPI_CTRL_SENDA, 1);
        MSPIn(ui32Module)->ADDR =
            _VAL2FLD(MSPI_ADDR_ADDR, pTransaction->ui32DeviceAddr);
    }

    //
    // Set the turn-around if needed.
    //
    if ( pTransaction->bTurnaround )
    {
        ui32Control |= _VAL2FLD(MSPI_CTRL_ENTURN, 1);
    }

    //
    // Set the default FIFO Little Endian format.
    //
    ui32Control |= _VAL2FLD(MSPI_CTRL_BIGENDIAN, pMSPIState->bBigEndian);

    //
    // Set for default of no continuation.
    //
    ui32Control |= _VAL2FLD(MSPI_CTRL_CONT, pTransaction->bContinue);

    //
    // Set the Quad Command if this is transmit and the device is configured
    // for Dual Quad mode.
    //
    if ( pTransaction->bQuadCmd )
    {
        ui32Control |= _VAL2FLD(MSPI_CTRL_QUADCMD, 1);
    }

    //
    // Start the Transfer.
    //
    ui32Control |= _VAL2FLD(MSPI_CTRL_START, 1);

    // Disable all interrupts
    intMask = MSPIn(ui32Module)->INTEN;
    MSPIn(ui32Module)->INTEN = 0;
    MSPIn(ui32Module)->INTCLR = AM_HAL_MSPI_INT_ALL;

    //
    // Initiate the Transfer.
    //
    MSPIn(ui32Module)->CTRL = ui32Control;
#else // AM_CMSIS_REGS
    //
    // Set the number of bytes to transfer.
    //
    ui32Control |= AM_REG_MSPI_CTRL_XFERBYTES(pTransaction->ui32NumBytes);

    //
    // Set the PIO default to scrambling disabled.
    //
    ui32Control |= AM_REG_MSPI_CTRL_PIOSCRAMBLE(pTransaction->bScrambling);

    //
    // Set transmit or receive operation.
    //
    ui32Control |= AM_REG_MSPI_CTRL_TXRX(pTransaction->eDirection);

    //
    // Set the indication to send an instruction and set the instruction value if
    // we have a valid instruction.
    //
    if ( pTransaction->bSendInstr )
    {
        ui32Control |= AM_REG_MSPI_CTRL_SENDI(1);
        AM_REGn(MSPI, ui32Module, INSTR) =
          AM_REG_MSPI_INSTR_INSTR(pTransaction->ui16DeviceInstr);
    }

    //
    // Set the inidication to send an address and set the address value if we have
    // a valid address.
    //
    if ( pTransaction->bSendAddr )
    {
        ui32Control |= AM_REG_MSPI_CTRL_SENDA(1);
        AM_REGn(MSPI, ui32Module, ADDR) =
          AM_REG_MSPI_ADDR_ADDR(pTransaction->ui32DeviceAddr);
    }

    //
    // Set the turn-around if needed.
    //
    if ( pTransaction->bTurnaround )
    {
        ui32Control |= AM_REG_MSPI_CTRL_ENTURN(1);
    }

    //
    // Set the default FIFO Little Endian format.
    //
    ui32Control |= AM_REG_MSPI_CTRL_BIGENDIAN(pMSPIState->bBigEndian);

    //
    // Set for default of no continuation.
    //
    ui32Control |= AM_REG_MSPI_CTRL_CONT(pTransaction->bContinue);

    //
    // Set the Quad Command if this is transmit and the device is configured
    // for Dual Quad mode.
    //
    if ( pTransaction->bQuadCmd )
    {
        ui32Control |= AM_REG_MSPI_CTRL_QUADCMD(1);
    }

    //
    // Start the Transfer.
    //
    ui32Control |= AM_REG_MSPI_CTRL_START(1);

    // Disable all interrupts
    intMask = AM_REGn(MSPI, ui32Module, INTEN);
    AM_REGn(MSPI, ui32Module, INTEN) = 0;
    AM_REGn(MSPI, ui32Module, INTCLR) = AM_HAL_MSPI_INT_ALL;

    //
    // Initiate the Transfer.
    //
    AM_REGn(MSPI, ui32Module, CTRL) = ui32Control;
#endif // AM_CMSIS_REGS

    //
    // Read or Feed the FIFOs.
    //
    if ( AM_HAL_MSPI_RX == pTransaction->eDirection )
    {
        ui32Status = mspi_fifo_read(ui32Module, pTransaction->pui32Buffer,
                                    pTransaction->ui32NumBytes, pMSPIState->waitTimeout);
    }
    else if ( AM_HAL_MSPI_TX == pTransaction->eDirection )
    {
        ui32Status = mspi_fifo_write(ui32Module, pTransaction->pui32Buffer,
                                     pTransaction->ui32NumBytes, pMSPIState->waitTimeout );
    }

#if AM_CMSIS_REGS
    //
    // Check status.
    //
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        //
        // Restore interrupts
        //
        MSPIn(ui32Module)->INTCLR = AM_HAL_MSPI_INT_ALL;
        MSPIn(ui32Module)->INTEN = intMask;
        return ui32Status;
    }

    //
    // Wait for the command to complete.
    //
    ui32Status = am_hal_flash_delay_status_check(ui32Timeout,
                                                 (uint32_t)&MSPIn(ui32Module)->CTRL,
                                                 MSPI_CTRL_STATUS_Msk,
                                                 _VAL2FLD(MSPI_CTRL_STATUS, 1),
                                                 true);

    //
    // Restore interrupts
    //
    MSPIn(ui32Module)->INTCLR = AM_HAL_MSPI_INT_ALL;
    MSPIn(ui32Module)->INTEN = intMask;
#else // AM_CMSIS_REGS
    //
    // Check status.
    //
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        // Restore interrupts
        AM_REGn(MSPI, ui32Module, INTCLR) = AM_HAL_MSPI_INT_ALL;
        AM_REGn(MSPI, ui32Module, INTEN) = intMask;
        return ui32Status;
    }

    //
    // Wait for the command to complete.
    //
    ui32Status = am_hal_flash_delay_status_check(ui32Timeout,
                                                 AM_REGADDRn(MSPI, ui32Module, CTRL),
                                                 AM_REG_MSPI_CTRL_STATUS_M,
                                                 AM_REG_MSPI_CTRL_STATUS(1),
                                                 true);

    // Restore interrupts
    AM_REGn(MSPI, ui32Module, INTCLR) = AM_HAL_MSPI_INT_ALL;
    AM_REGn(MSPI, ui32Module, INTEN) = intMask;
#endif // AM_CMSIS_REGS

    //
    // Return the status.
    //
    return ui32Status;

}

//
// MSPI Non-Blocking transfer function
//
uint32_t am_hal_mspi_nonblocking_transfer(void *pHandle,
                                          void *pTransfer,
                                          am_hal_mspi_trans_e eMode,
                                          am_hal_mspi_callback_t pfnCallback,
                                          void *pCallbackCtxt)
{
    am_hal_mspi_state_t           *pMSPIState = (am_hal_mspi_state_t *)pHandle;
    uint32_t                      ui32Status = AM_HAL_STATUS_SUCCESS;
    uint32_t ui32NumPend;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    //
    // Check the handle.
    //
    if (!AM_HAL_MSPI_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif

#if MSPI_USE_CQ

    //
    // DMA defaults to using the Command Queue
    //
    ui32Status = mspi_cq_add_transaction(pHandle, pTransfer, eMode, pfnCallback, pCallbackCtxt);

    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return ui32Status;
    }
    else
    {

        uint32_t ui32Critical = 0;
        //
        // Start a critical section.
        //
        ui32Critical = am_hal_interrupt_master_disable();

        //
        // Post the transaction to the CQ.
        //
        ui32Status = am_hal_cmdq_post_block(pMSPIState->CQ.pCmdQHdl, pfnCallback);

        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            //
            // End the critical section.
            //
            am_hal_interrupt_master_set(ui32Critical);

            am_hal_cmdq_release_block(pMSPIState->CQ.pCmdQHdl);
        }
        else
        {
            ui32NumPend = pMSPIState->ui32NumCQEntries++;
            //
            // End the critical section.
            //
            am_hal_interrupt_master_set(ui32Critical);
            if (ui32NumPend == 0)
            {
                //
                // Enable the Command Queue
                //
                ui32Status = mspi_cq_enable(pHandle);
                if (AM_HAL_STATUS_SUCCESS != ui32Status)
                {
                    return ui32Status;
                }
            }
        }
    }

#else
    if (pMSPIState->ui32MaxTransactions == 0)
    {
        return AM_HAL_STATUS_INVALID_OPERATION;
    }

    if (AM_HAL_MSPI_TRANS_DMA != eMode)
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    ui32Status = mspi_dma_add_transaction(pHandle, pTransfer, pfnCallback, pCallbackCtxt);

    if (ui32Status == AM_HAL_STATUS_SUCCESS)
    {
#if AM_CMSIS_REGS
        //
        // Start a critical section.
        //
        AM_CRITICAL_BEGIN

        ui32NumPend = pMSPIState->ui32NumCQEntries++;

        //
        // End the critical section.
        //
        AM_CRITICAL_END
#else // AM_CMSIS_REGS
        uint32_t ui32Critical = 0;

        //
        // Start a critical section.
        //
        ui32Critical = am_hal_interrupt_master_disable();

        ui32NumPend = pMSPIState->ui32NumCQEntries++;

        //
        // End the critical section.
        //
        am_hal_interrupt_master_set(ui32Critical);
#endif // AM_CMSIS_REGS


        if (0 == ui32NumPend)
        {
            uint32_t index = (pMSPIState->ui32LastIdxProcessed + 1) % pMSPIState->ui32MaxTransactions;

            pMSPIState->ui32TxnInt = 0;
            //
            // Run the command list
            //
            run_txn_cmdlist(&pMSPIState->pTransactions[index], sizeof(am_hal_mspi_cq_dma_entry_t) / sizeof(am_hal_cmdq_entry_t));
        }
    }

#endif // !MSPI_USE_CQ

    //
    // Return the status.
    //
    return ui32Status;
}

//
// MSPI status function
//
uint32_t am_hal_mspi_status_get(void *pHandle,
                                am_hal_mspi_status_t *pStatus )
{
    am_hal_mspi_state_t           *pMSPIState = (am_hal_mspi_state_t *)pHandle;
    uint32_t                      ui32Module = pMSPIState->ui32Module;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    //
    // Check the handle.
    //
    if (!AM_HAL_MSPI_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif

    //
    // Get the Command Complete status.
    //
    // TODO: Need to implement.

    //
    // Get the FIFO status.
    //
    // TODO: Need to implement.

    //
    //  Get the DMA status.
    //
#if AM_CMSIS_REGS
    pStatus->bErr = ((MSPIn(ui32Module)->DMASTAT & MSPI_DMASTAT_DMAERR_Msk) > 0);
    pStatus->bCmp = ((MSPIn(ui32Module)->DMASTAT & MSPI_DMASTAT_DMACPL_Msk) > 0);
    pStatus->bTIP = ((MSPIn(ui32Module)->DMASTAT & MSPI_DMASTAT_DMATIP_Msk) > 0);
#else // AM_CMSIS_REGS
    pStatus->bErr = ((AM_REGn(MSPI, ui32Module, DMASTAT) & AM_REG_MSPI_DMASTAT_DMAERR_M) > 0);
    pStatus->bCmp = ((AM_REGn(MSPI, ui32Module, DMASTAT) & AM_REG_MSPI_DMASTAT_DMACPL_M) > 0);
    pStatus->bTIP = ((AM_REGn(MSPI, ui32Module, DMASTAT) & AM_REG_MSPI_DMASTAT_DMATIP_M) > 0);
#endif // AM_CMSIS_REGS

    //
    // Get the CQ status.
    //
    // TODO: Need to implement.
    pStatus->ui32NumCQEntries = pMSPIState->ui32NumCQEntries;

    //
    // Get the scrambling status.
    //
    // TODO: Need to implement.

    //
    // Return the status.
    //
    return AM_HAL_STATUS_SUCCESS;
}

//
// MSPI enable interrupts function
//
uint32_t am_hal_mspi_interrupt_enable(void *pHandle,
                                      uint32_t ui32IntMask)
{
    am_hal_mspi_state_t           *pMSPIState = (am_hal_mspi_state_t *)pHandle;
    uint32_t                      ui32Module = pMSPIState->ui32Module;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    //
    // Check the handle.
    //
    if (!AM_HAL_MSPI_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif

    //
    // Set the interrupt enables according to the mask.
    //
#if AM_CMSIS_REGS
    MSPIn(ui32Module)->INTEN |= ui32IntMask;
#else // AM_CMSIS_REGS
    AM_REGn(MSPI, ui32Module, INTEN) |= ui32IntMask;
#endif // AM_CMSIS_REGS

    //
    // Return the status.
    //
    return AM_HAL_STATUS_SUCCESS;
}

//
// MSPI disable interrupts function
//
uint32_t am_hal_mspi_interrupt_disable(void *pHandle,
                                       uint32_t ui32IntMask)
{
    am_hal_mspi_state_t           *pMSPIState = (am_hal_mspi_state_t *)pHandle;
    uint32_t                      ui32Module = pMSPIState->ui32Module;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    //
    // Check the handle.
    //
    if (!AM_HAL_MSPI_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif

#if AM_CMSIS_REGS
    //
    // Clear the interrupt enables according to the mask.
    //
    MSPIn(ui32Module)->INTEN &= ~ui32IntMask;
#else // AM_CMSIS_REGS
    // Clear the interrupt enables according to the mask.
    AM_REGn(MSPI, ui32Module, INTEN) &= ~ui32IntMask;
#endif // AM_CMSIS_REGS

    //
    // Return the status.
    //
    return AM_HAL_STATUS_SUCCESS;
}

//
// MSPI interrupt status function
//
uint32_t am_hal_mspi_interrupt_status_get(void *pHandle,
                                          uint32_t  *pui32Status,
                                          bool bEnabledOnly)
{
    am_hal_mspi_state_t           *pMSPIState = (am_hal_mspi_state_t *)pHandle;
    uint32_t                      ui32Module = pMSPIState->ui32Module;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    //
    // Check the handle.
    //
    if (!AM_HAL_MSPI_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif

    //
    // if requested, only return the interrupts that are enabled.
    //
#if AM_CMSIS_REGS
    if ( bEnabledOnly )
    {
        uint32_t ui32RetVal = MSPIn(ui32Module)->INTSTAT;
        *pui32Status = ui32RetVal & MSPIn(ui32Module)->INTEN;
    }
    else
    {
        *pui32Status = MSPIn(ui32Module)->INTSTAT;
    }
#else // AM_CMSIS_REGS
    if ( bEnabledOnly )
    {
        uint32_t ui32RetVal = AM_REGn(MSPI, ui32Module, INTSTAT);
        *pui32Status = ui32RetVal & AM_REGn(MSPI, ui32Module, INTEN);
    }
    else
    {
        *pui32Status = AM_REGn(MSPI, ui32Module, INTSTAT);
    }
#endif // AM_CMSIS_REGS

    return AM_HAL_STATUS_SUCCESS;
}

//
// MSPI interrupt clear
//
uint32_t am_hal_mspi_interrupt_clear(void *pHandle,
                                     uint32_t ui32IntMask)
{
    am_hal_mspi_state_t           *pMSPIState = (am_hal_mspi_state_t *)pHandle;
    uint32_t                      ui32Module = pMSPIState->ui32Module;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    //
    // Check the handle.
    //
    if ( !AM_HAL_MSPI_CHK_HANDLE(pHandle) )
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif

    //
    // clear the requested interrupts.
    //
#if AM_CMSIS_REGS
    MSPIn(ui32Module)->INTCLR = ui32IntMask;
#else // AM_CMSIS_REGS
    AM_REGn(MSPI, ui32Module, INTCLR) = ui32IntMask;
#endif // AM_CMSIS_REGS

    //
    // Return the status.
    //
    return AM_HAL_STATUS_SUCCESS;
}

//
// MSPI interrupt service routine
//
uint32_t am_hal_mspi_interrupt_service(void *pHandle, uint32_t ui32IntStatus)
{
    am_hal_mspi_state_t           *pMSPIState = (am_hal_mspi_state_t *)pHandle;
    uint32_t                      ui32Module = pMSPIState->ui32Module;
    uint32_t                      ui32Status;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    //
    // Check the handle.
    //
    if (!AM_HAL_MSPI_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif

    //
    // Add a delay to help make the service function work.
    // TODO - why do we need this?
    //
    am_hal_flash_delay(FLASH_CYCLES_US(10));

    //
    // Need to check if there is an ongoing transaction
    // This is needed because we may get interrupts even for the XIP transactions
    //
    if (pMSPIState->ui32NumCQEntries)
    {
#if MSPI_USE_CQ
        am_hal_cmdq_status_t status;
        uint32_t index;
        am_hal_mspi_CQ_t *pCQ = &g_MSPIState[ui32Module].CQ;

        //
        // Get the current and last indexes.
        //
        if (pCQ->pCmdQHdl)
        {
            ui32Status = am_hal_cmdq_get_status(pCQ->pCmdQHdl, &status);

            if (AM_HAL_STATUS_SUCCESS == ui32Status)
            {

                //
                // Figure out which callbacks need to be handled.
                //
                while (pMSPIState->ui32LastIdxProcessed != status.lastIdxProcessed)
                {

                    pMSPIState->ui32LastIdxProcessed++;
                    pMSPIState->ui32NumCQEntries--;
                    index = pMSPIState->ui32LastIdxProcessed & (AM_HAL_MSPI_MAX_CQ_ENTRIES - 1);
                    if ( pMSPIState->pfnCallback[index] != NULL )
                    {
                        pMSPIState->pfnCallback[index](pMSPIState->pCallbackCtxt[index], AM_HAL_STATUS_SUCCESS);
                        pMSPIState->pfnCallback[index] = NULL;
                    }
                }

                //
                // Process one extra callback if there was an error.
                //
                if ( (ui32IntStatus & AM_HAL_MSPI_INT_ERR) || (status.bErr) )
                {
                    pMSPIState->ui32LastIdxProcessed++;
                    pMSPIState->ui32NumCQEntries--;
                    index = pMSPIState->ui32LastIdxProcessed & (AM_HAL_MSPI_MAX_CQ_ENTRIES - 1);
                    if ( pMSPIState->pfnCallback[index] != NULL )
                    {
                        pMSPIState->pfnCallback[index](pMSPIState->pCallbackCtxt[index], AM_HAL_STATUS_FAIL);
                        pMSPIState->pfnCallback[index] = NULL;
                    }
                    // Disable DMA
#if AM_CMSIS_REGS
                    MSPIn(ui32Module)->DMACFG_b.DMAEN = 0;

                    // Must reset xfer block
                    MSPIn(ui32Module)->MSPICFG_b.IPRSTN = 0;  // in reset
                    MSPIn(ui32Module)->MSPICFG_b.IPRSTN = 1;  // back out -- clears current transfer

                    // Clear the CQ error.
                    MSPIn(ui32Module)->CQSTAT |= _VAL2FLD(MSPI_CQSTAT_CQERR, 0);
#else // AM_CMSIS_REGS
                    AM_BFWn(MSPI, ui32Module, DMACFG, DMAEN, 0);
                    // Must reset xfer block
                    AM_BFWn(MSPI, ui32Module, MSPICFG, IPRSTN, 0);  // in reset
                    AM_BFWn(MSPI, ui32Module, MSPICFG, IPRSTN, 1);  // back out -- clears current transfer

                    // Clear the CQ error.
                    AM_REGn(MSPI, ui32Module, CQSTAT) |= AM_REG_MSPI_CQSTAT_CQERR(0);
#endif // AM_CMSIS_REGS
                    am_hal_cmdq_error_resume(pCQ->pCmdQHdl);
                }
                if (pMSPIState->ui32NumCQEntries == 0)
                {
                    // Disable CQ
                    mspi_cq_disable(pMSPIState);
                }
            }
        }
#else
        //
        // Accumulate the INTSTAT for this transaction
        //
        pMSPIState->ui32TxnInt |= ui32IntStatus;

        //
        // We need to wait for the DMA complete as well
        //
#if AM_CMSIS_REGS
        if (pMSPIState->ui32TxnInt & (AM_HAL_MSPI_INT_DMACMP | AM_HAL_MSPI_INT_ERR))
        {
            uint32_t index;

            //
            // Wait for the command completion
            //
            if (!(pMSPIState->ui32TxnInt & AM_HAL_MSPI_INT_CMDCMP))
            {
                // TODO - We are waiting for CMDCMP indefinetely in the ISR
                // May need to re-evaluate
                while (!MSPIn(ui32Module)->INTSTAT_b.CMDCMP);
            }
            pMSPIState->ui32TxnInt |= MSPIn(ui32Module)->INTSTAT;

            //
            // Clear the interrupt status
            //
            MSPIn(ui32Module)->INTCLR = AM_HAL_MSPI_INT_ALL;

            //
            // Need to determine the error, call the callback with proper status
            //
            if (pMSPIState->ui32TxnInt & AM_HAL_MSPI_INT_ERR)
            {
                ui32Status = AM_HAL_STATUS_FAIL;

                //
                // Disable DMA
                //
                MSPIn(ui32Module)->DMACFG_b.DMAEN = 0;

                //
                // Must reset xfer block
                //
                MSPIn(ui32Module)->MSPICFG_b.IPRSTN = 0;  // in reset
                MSPIn(ui32Module)->MSPICFG_b.IPRSTN = 1;  // back out -- clears current transfer
            }
            else
            {
                ui32Status = AM_HAL_STATUS_SUCCESS;
            }

            pMSPIState->ui32LastIdxProcessed++;
            pMSPIState->ui32NumCQEntries--;
            index = pMSPIState->ui32LastIdxProcessed % pMSPIState->ui32MaxTransactions;

            //
            // Call the callback
            //
            if ( pMSPIState->pfnCallback[index] != NULL )
            {
                pMSPIState->pfnCallback[index](pMSPIState->pCallbackCtxt[index], ui32Status);
                pMSPIState->pfnCallback[index] = NULL;
            }

            //
            // Post next transaction if queue is not empty
            //
            if (pMSPIState->ui32NumCQEntries)
            {
                index = (pMSPIState->ui32LastIdxProcessed + 1) % pMSPIState->ui32MaxTransactions;

                pMSPIState->ui32TxnInt = 0;
                run_txn_cmdlist(&pMSPIState->pTransactions[index], sizeof(am_hal_mspi_cq_dma_entry_t) / sizeof(am_hal_cmdq_entry_t));
            }
        }
#else // AM_CMSIS_REGS
        if (pMSPIState->ui32TxnInt & (AM_HAL_MSPI_INT_DMACMP | AM_HAL_MSPI_INT_ERR))
        {
            uint32_t index;

            //
            // Wait for the command completion
            //
            if (!(pMSPIState->ui32TxnInt & AM_HAL_MSPI_INT_CMDCMP))
            {
                // TODO - We are waiting for CMDCMP indefinetely in the ISR
                // May need to re-evaluate
                while (!AM_BFRn(MSPI, ui32Module, INTSTAT, CMDCMP));
            }
            pMSPIState->ui32TxnInt |= AM_REGn(MSPI, ui32Module, INTSTAT);
            // Clear the interrupt status
            AM_REGn(MSPI, ui32Module, INTCLR) = AM_HAL_MSPI_INT_ALL;
            // Need to determine the error, call the callback with proper status
            if (pMSPIState->ui32TxnInt & AM_HAL_MSPI_INT_ERR)
            {
                ui32Status = AM_HAL_STATUS_FAIL;
                // Disable DMA
                AM_BFWn(MSPI, ui32Module, DMACFG, DMAEN, 0);
                // Must reset xfer block
                AM_BFWn(MSPI, ui32Module, MSPICFG, IPRSTN, 0);  // in reset
                AM_BFWn(MSPI, ui32Module, MSPICFG, IPRSTN, 1);  // back out -- clears current transfer
            }
            else
            {
                ui32Status = AM_HAL_STATUS_SUCCESS;
            }
            pMSPIState->ui32LastIdxProcessed++;
            pMSPIState->ui32NumCQEntries--;
            index = pMSPIState->ui32LastIdxProcessed % pMSPIState->ui32MaxTransactions;
            // Call the callback
            if ( pMSPIState->pfnCallback[index] != NULL )
            {
                pMSPIState->pfnCallback[index](pMSPIState->pCallbackCtxt[index], ui32Status);
                pMSPIState->pfnCallback[index] = NULL;
            }

            //
            // Post next transaction if queue is not empty
            //
            if (pMSPIState->ui32NumCQEntries)
            {
                index = (pMSPIState->ui32LastIdxProcessed + 1) % pMSPIState->ui32MaxTransactions;

                pMSPIState->ui32TxnInt = 0;
                run_txn_cmdlist(&pMSPIState->pTransactions[index], sizeof(am_hal_mspi_cq_dma_entry_t) / sizeof(am_hal_cmdq_entry_t));
            }
        }
#endif // AM_CMSIS_REGS

#endif // !MSPI_USE_CQ

        if (pMSPIState->ui32NumCQEntries == 0)
        {
            // Disable DMA
#if AM_CMSIS_REGS
            MSPIn(ui32Module)->DMACFG_b.DMAEN = 0;
#else // AM_CMSIS_REGS
            AM_BFWn(MSPI, ui32Module, DMACFG, DMAEN, 0);
#endif // AM_CMSIS_REGS
        }
    }

  //
  // Return the status.
  //
  return AM_HAL_STATUS_SUCCESS;
}

//
// MSPI power control function
//
uint32_t am_hal_mspi_power_control(void *pHandle,
                                   am_hal_sysctrl_power_state_e ePowerState,
                                   bool bRetainState)
{
    am_hal_mspi_state_t           *pMSPIState = (am_hal_mspi_state_t *)pHandle;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    //
    // Check the handle.
    //
    if (!AM_HAL_MSPI_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif

    //
    // Decode the requested power state and update MSPI operation accordingly.
    //
    switch (ePowerState)
    {
        case AM_HAL_SYSCTRL_WAKE:

            if (bRetainState && !pMSPIState->registerState.bValid)
            {
                return AM_HAL_STATUS_INVALID_OPERATION;
            }

            //
            // Enable power control.
            //
            am_hal_pwrctrl_periph_enable((am_hal_pwrctrl_periph_e)(AM_HAL_PWRCTRL_PERIPH_MSPI));

            if (bRetainState)
            {
                //
                // Restore MSPI registers
                //
#if AM_CMSIS_REGS
                MSPIn(pMSPIState->ui32Module)->CFG        = pMSPIState->registerState.regCFG;
                MSPIn(pMSPIState->ui32Module)->MSPICFG    = pMSPIState->registerState.regMSPICFG;
                MSPIn(pMSPIState->ui32Module)->PADCFG     = pMSPIState->registerState.regPADCFG;
                MSPIn(pMSPIState->ui32Module)->PADOUTEN   = pMSPIState->registerState.regPADOUTEN;
                MSPIn(pMSPIState->ui32Module)->FLASH      = pMSPIState->registerState.regFLASH;
                MSPIn(pMSPIState->ui32Module)->SCRAMBLING = pMSPIState->registerState.regSCRAMBLING;
                MSPIn(pMSPIState->ui32Module)->CQCFG      = pMSPIState->registerState.regCQCFG;
                MSPIn(pMSPIState->ui32Module)->CQADDR     = pMSPIState->registerState.regCQADDR;
                MSPIn(pMSPIState->ui32Module)->CQPAUSE    = pMSPIState->registerState.regCQPAUSE;
                MSPIn(pMSPIState->ui32Module)->CQCURIDX   = pMSPIState->registerState.regCQCURIDX;
                MSPIn(pMSPIState->ui32Module)->CQENDIDX   = pMSPIState->registerState.regCQENDIDX;
                MSPIn(pMSPIState->ui32Module)->INTEN      = pMSPIState->registerState.regINTEN;

                // TODO: May be we can just set these values, as they are constants anyways?
                MSPIn(pMSPIState->ui32Module)->DMABCOUNT  = pMSPIState->registerState.regDMABCOUNT;
                MSPIn(pMSPIState->ui32Module)->DMATHRESH  = pMSPIState->registerState.regDMATHRESH;
#else // AM_CMSIS_REGS
                AM_REGn(MSPI, pMSPIState->ui32Module, CFG) = pMSPIState->registerState.regCFG;
                AM_REGn(MSPI, pMSPIState->ui32Module, MSPICFG) = pMSPIState->registerState.regMSPICFG;
                AM_REGn(MSPI, pMSPIState->ui32Module, PADCFG) = pMSPIState->registerState.regPADCFG;
                AM_REGn(MSPI, pMSPIState->ui32Module, PADOUTEN) = pMSPIState->registerState.regPADOUTEN;
                AM_REGn(MSPI, pMSPIState->ui32Module, FLASH) = pMSPIState->registerState.regFLASH;
                AM_REGn(MSPI, pMSPIState->ui32Module, SCRAMBLING) = pMSPIState->registerState.regSCRAMBLING;
                AM_REGn(MSPI, pMSPIState->ui32Module, CQCFG) = pMSPIState->registerState.regCQCFG;
                AM_REGn(MSPI, pMSPIState->ui32Module, CQADDR) = pMSPIState->registerState.regCQADDR;
                AM_REGn(MSPI, pMSPIState->ui32Module, CQPAUSE) = pMSPIState->registerState.regCQPAUSE;
                AM_REGn(MSPI, pMSPIState->ui32Module, CQCURIDX) = pMSPIState->registerState.regCQCURIDX;
                AM_REGn(MSPI, pMSPIState->ui32Module, CQENDIDX) = pMSPIState->registerState.regCQENDIDX;
                AM_REGn(MSPI, pMSPIState->ui32Module, INTEN) = pMSPIState->registerState.regINTEN;
                // TODO: May be we can just set these values, as they are constants anyways?
                AM_REGn(MSPI, pMSPIState->ui32Module, DMABCOUNT) = pMSPIState->registerState.regDMABCOUNT;
                AM_REGn(MSPI, pMSPIState->ui32Module, DMATHRESH) = pMSPIState->registerState.regDMATHRESH;
#endif // AM_CMSIS_REGS

                pMSPIState->registerState.bValid = false;
            }
            break;

        case AM_HAL_SYSCTRL_NORMALSLEEP:
        case AM_HAL_SYSCTRL_DEEPSLEEP:
#if AM_CMSIS_REGS
            if (bRetainState)
            {
                //
                // Save MSPI Registers
                //
                pMSPIState->registerState.regCFG        = MSPIn(pMSPIState->ui32Module)->CFG;
                pMSPIState->registerState.regMSPICFG    = MSPIn(pMSPIState->ui32Module)->MSPICFG;
                pMSPIState->registerState.regPADCFG     = MSPIn(pMSPIState->ui32Module)->PADCFG;
                pMSPIState->registerState.regPADOUTEN   = MSPIn(pMSPIState->ui32Module)->PADOUTEN;
                pMSPIState->registerState.regFLASH      = MSPIn(pMSPIState->ui32Module)->FLASH;
                pMSPIState->registerState.regSCRAMBLING = MSPIn(pMSPIState->ui32Module)->SCRAMBLING;
                pMSPIState->registerState.regCQADDR     = MSPIn(pMSPIState->ui32Module)->CQADDR;
                pMSPIState->registerState.regCQPAUSE    = MSPIn(pMSPIState->ui32Module)->CQPAUSE;
                pMSPIState->registerState.regCQCURIDX   = MSPIn(pMSPIState->ui32Module)->CQCURIDX;
                pMSPIState->registerState.regCQENDIDX   = MSPIn(pMSPIState->ui32Module)->CQENDIDX;
                pMSPIState->registerState.regINTEN      = MSPIn(pMSPIState->ui32Module)->INTEN;

                // TODO: May be no need to store these values, as they are constants anyways?
                pMSPIState->registerState.regDMABCOUNT  = MSPIn(pMSPIState->ui32Module)->DMABCOUNT;
                pMSPIState->registerState.regDMATHRESH  = MSPIn(pMSPIState->ui32Module)->DMATHRESH;

                //
                // Set the CQCFG last
                //
                pMSPIState->registerState.regCQCFG      = MSPIn(pMSPIState->ui32Module)->CQCFG;
                pMSPIState->registerState.bValid        = true;
            }

            //
            // Disable power control.
            //
            am_hal_pwrctrl_periph_disable((am_hal_pwrctrl_periph_e)(AM_HAL_PWRCTRL_PERIPH_MSPI));
            break;
#else // AM_CMSIS_REGS
            if (bRetainState)
            {
                // Save MSPI Registers
                pMSPIState->registerState.regCFG = AM_REGn(MSPI, pMSPIState->ui32Module, CFG);
                pMSPIState->registerState.regMSPICFG = AM_REGn(MSPI, pMSPIState->ui32Module, MSPICFG);
                pMSPIState->registerState.regPADCFG = AM_REGn(MSPI, pMSPIState->ui32Module, PADCFG);
                pMSPIState->registerState.regPADOUTEN = AM_REGn(MSPI, pMSPIState->ui32Module, PADOUTEN);
                pMSPIState->registerState.regFLASH = AM_REGn(MSPI, pMSPIState->ui32Module, FLASH);
                pMSPIState->registerState.regSCRAMBLING = AM_REGn(MSPI, pMSPIState->ui32Module, SCRAMBLING);
                pMSPIState->registerState.regCQADDR = AM_REGn(MSPI, pMSPIState->ui32Module, CQADDR);
                pMSPIState->registerState.regCQPAUSE = AM_REGn(MSPI, pMSPIState->ui32Module, CQPAUSE);
                pMSPIState->registerState.regCQCURIDX = AM_REGn(MSPI, pMSPIState->ui32Module, CQCURIDX);
                pMSPIState->registerState.regCQENDIDX = AM_REGn(MSPI, pMSPIState->ui32Module, CQENDIDX);
                pMSPIState->registerState.regINTEN = AM_REGn(MSPI, pMSPIState->ui32Module, INTEN);
                // TODO: May be no need to store these values, as they are constants anyways?
                pMSPIState->registerState.regDMABCOUNT = AM_REGn(MSPI, pMSPIState->ui32Module, DMABCOUNT);
                pMSPIState->registerState.regDMATHRESH = AM_REGn(MSPI, pMSPIState->ui32Module, DMATHRESH);
                // Set the CQCFG last
                pMSPIState->registerState.regCQCFG = AM_REGn(MSPI, pMSPIState->ui32Module, CQCFG);

                pMSPIState->registerState.bValid = true;
            }

            //
            // Disable power control.
            //
            am_hal_pwrctrl_periph_disable((am_hal_pwrctrl_periph_e)(AM_HAL_PWRCTRL_PERIPH_MSPI));
            break;
#endif // AM_CMSIS_REGS

        default:
            return AM_HAL_STATUS_INVALID_ARG;
    }

    //
    // Return the status.
    //
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
