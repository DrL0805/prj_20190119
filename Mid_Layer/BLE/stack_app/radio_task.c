//*****************************************************************************
//
//! @file radio_task.c
//!
//! @brief Task to handle radio operation.
//!
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

//*****************************************************************************
//
// Global includes for this project.
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "task.h"
#include "rtos.h"
#include "portmacro.h"
#include "portable.h"
#include "semphr.h"
#include "event_groups.h"

//*****************************************************************************
// WSF standard includes.
//*****************************************************************************
#include "wsf_types.h"
#include "wsf_trace.h"
#include "wsf_buf.h"
#include "wsf_timer.h"

//*****************************************************************************
// Includes for operating the ExactLE stack.
//*****************************************************************************
#include "hci_handler.h"
#include "dm_handler.h"
#include "l2c_handler.h"
#include "att_handler.h"
#include "smp_handler.h"
#include "l2c_api.h"
#include "att_api.h"
#include "smp_api.h"
#include "app_api.h"
#include "hci_core.h"
#include "hci_drv.h"
#include "hci_drv_apollo.h"
#include "wsf_msg.h"

#ifdef AM_PART_APOLLO3
#include "am_mcu_cmsis.h"
#if AM_CMSIS_REGS
#include "apollo3.h"
#else // AM_CMSIS_REGS
#ifdef __IAR_SYSTEMS_ICC__
#include "intrinsics.h"     // __CLZ() and other intrinsics
#endif // AM_CMSIS_REGS
#endif
#include "hci_drv_apollo3.h"

#else
#include "hci_em9304.h"
#include "drv_em9304.h"
#endif

//*****************************************************************************
// Includes for the ExactLE "fit" profile.
//*****************************************************************************
#include "app_m_ble_profile.h"
#include "sm_sys.h"

#define BLE_STACK_DEBUG     1    //Add for print debug info

//*****************************************************************************
//
// Radio task handle.
//
//*****************************************************************************
TaskHandle_t radio_task_handle;

//*****************************************************************************
//
// Handle for Radio-related events.
//
//*****************************************************************************
EventGroupHandle_t xRadioEventHandle;

//*****************************************************************************
//
// Timer configuration macros.
//
//*****************************************************************************
// Configure how to driver WSF Scheduler
//Apollo提供的例程中，提供3中法式用来作WSF Scheduler: Stimer,Ctimer和FreeRTOS timer
//例程推荐使用FreeRTOS timer，其他两种为test， 因此在此driver中将其删除，若有需求
//可到Apollo提供的例子中查找
#if 1
// Preferred mode to use when using FreeRTOS
#define USE_FREERTOS_TIMER_FOR_WSF
#else
// These are only test modes.
#ifdef AM_FREERTOS_USE_STIMER_FOR_TICK
#define USE_STIMER_FOR_WSF // Reuse FreeRTOS used STimer for WSF
#else
#define USE_CTIMER_FOR_WSF
#endif
#endif

// Use FreeRTOS timer for WSF Ticks
#ifdef USE_FREERTOS_TIMER_FOR_WSF
#define CLK_TICKS_PER_WSF_TICKS     (WSF_MS_PER_TICK*configTICK_RATE_HZ/1000)
#endif

//*****************************************************************************
//
// WSF buffer pools.
//
//*****************************************************************************
#define WSF_BUF_POOLS               6 //20180508 ver 1.2.12

// Important note: the size of g_pui32BufMem should includes both overhead of internal
// buffer management structure, wsfBufPool_t (up to 16 bytes for each pool), and pool 
// description (e.g. g_psPoolDescriptors below).

// Memory for the buffer pool
// extra AMOTA_PACKET_SIZE bytes for OTA handling
static uint32_t g_pui32BufMem[4096 / sizeof(uint32_t)];  //20180508 ver 1.2.12

// Default pool descriptor.
static wsfBufPoolDesc_t g_psPoolDescriptors[WSF_BUF_POOLS] =
{
    {  16,  8 },
    {  32,  8 },
    {  64,  8 },
    { 128,  4 },
    { 280,  4 },
    { 512,  2 },
};

#ifndef AM_PART_APOLLO3
wsfHandlerId_t g_bleDataReadyHandlerId;
/*! Event types for ble rx data handler */
#define BLE_DATA_READY_EVENT                   0x01      /*! Trigger Rx data path */
#endif

//*****************************************************************************
//
// Tracking variable for the scheduler timer.
//
//*****************************************************************************
uint32_t g_ui32LastTime = 0;

void radio_timer_handler(void);

#ifndef AM_PART_APOLLO3
void ble_data_ready_handler(wsfEventMask_t event, wsfMsgHdr_t *pMsg)
{
    if (FALSE == HciDataReadyISR()) 
    {
        // trigger event again to handle pending data from BLE controller
        WsfSetEvent(g_bleDataReadyHandlerId, BLE_DATA_READY_EVENT);
    }
}
#endif

#ifdef USE_FREERTOS_TIMER_FOR_WSF
TimerHandle_t xWsfTimer;
//*****************************************************************************
//
// Callback handler for the FreeRTOS Timer
//
//*****************************************************************************
void wsf_timer_handler(TimerHandle_t xTimer)
{
    // Signal radio task to run 
	WsfTaskSetReady(0,0);
}
//*****************************************************************************
//
// Reuse FreeRTOS TIMER to handle the WSF scheduler.
//
//*****************************************************************************
static void scheduler_timer_init(void)
{
    // Create a FreeRTOS Timer
    xWsfTimer = xTimerCreate("WSF Timer", pdMS_TO_TICKS(WSF_MS_PER_TICK),
            pdFALSE, NULL, wsf_timer_handler);
    configASSERT(xWsfTimer);
}

//*****************************************************************************
//
// Calculate the elapsed time, and update the WSF software timers.
//
//*****************************************************************************
static void update_scheduler_timers(void)
{
    uint32_t ui32CurrentTime, ui32ElapsedTime;

    // Read the continuous timer.
    ui32CurrentTime = xTaskGetTickCount();

    // Figure out how long it has been since the last time we've read the
    // continuous timer. We should be reading often enough that we'll never
    // have more than one overflow.
    ui32ElapsedTime = ui32CurrentTime - g_ui32LastTime;

    // Check to see if any WSF ticks need to happen.
    if ((ui32ElapsedTime / CLK_TICKS_PER_WSF_TICKS) > 0 )
    {
        // Update the WSF timers and save the current time as our "last update".
        WsfTimerUpdate(ui32ElapsedTime / CLK_TICKS_PER_WSF_TICKS);

        g_ui32LastTime = ui32CurrentTime;
    }
}

//*****************************************************************************
//
// Set a timer interrupt for the next upcoming scheduler event.
//
//*****************************************************************************
static void set_next_wakeup(void)
{
    bool_t bTimerRunning;
    wsfTimerTicks_t xNextExpiration;

    // Check to see when the next timer expiration should happen.
    xNextExpiration = WsfTimerNextExpiration(&bTimerRunning);

    // If there's a pending WSF timer event, set an interrupt to wake us up in
    // time to service it.
    if ( xNextExpiration )
    {
        if(pdPASS != xTimerChangePeriod( xWsfTimer,pdMS_TO_TICKS(xNextExpiration*CLK_TICKS_PER_WSF_TICKS), 100))
        {
            Err_Info((0,"Error:BLE Radio task Timer Change period fail,then go into while(1)...\n"));
            while(1);
        }
    }
}
#endif

//*****************************************************************************
//
// Initialization for the ExactLE stack.
//
//*****************************************************************************
static void exactle_stack_init(void)
{
    wsfHandlerId_t handlerId;
    uint16_t       wsfBufMemLen;

    // Set up timers for the WSF scheduler.
    scheduler_timer_init();
    WsfTimerInit();

    // Initialize a buffer pool for WSF dynamic memory needs.
    wsfBufMemLen = WsfBufInit(sizeof(g_pui32BufMem), (uint8_t *)g_pui32BufMem, WSF_BUF_POOLS,
               g_psPoolDescriptors);

    if (wsfBufMemLen > sizeof(g_pui32BufMem))
    {
        Err_Info((0,"Memory pool is too small by %d\r\n",wsfBufMemLen - sizeof(g_pui32BufMem)));
    }

    // Initialize the WSF security service.
    SecInit();
    SecAesInit();
    SecCmacInit();
    SecEccInit();

    // Set up callback functions for the various layers of the ExactLE stack.
    handlerId = WsfOsSetNextHandler(HciHandler);
    HciHandlerInit(handlerId);

    handlerId = WsfOsSetNextHandler(DmHandler);
    DmDevVsInit(0);
    DmAdvInit();
    DmConnInit();
    DmConnSlaveInit();
    DmSecInit();
    DmSecLescInit();
    DmPrivInit();
    DmHandlerInit(handlerId);

    handlerId = WsfOsSetNextHandler(L2cSlaveHandler);
    L2cSlaveHandlerInit(handlerId);
    L2cInit();
    L2cSlaveInit();

    handlerId = WsfOsSetNextHandler(AttHandler);
    AttHandlerInit(handlerId);
    AttsInit();
    AttsIndInit();
    AttcInit();

    handlerId = WsfOsSetNextHandler(SmpHandler);
    SmpHandlerInit(handlerId);
    SmprInit();
    SmprScInit();
    HciSetMaxRxAclLen(251);

    handlerId = WsfOsSetNextHandler(AppHandler);
    AppHandlerInit(handlerId);

    //初始化ble应用handler
    handlerId = WsfOsSetNextHandler(app_m_ble_handler);
    app_m_ble_handler_init(handlerId);

#ifdef AM_PART_APOLLO3
    handlerId = WsfOsSetNextHandler(HciDrvHandler);
    HciDrvHandlerInit(handlerId);
#else
    g_bleDataReadyHandlerId = WsfOsSetNextHandler(ble_data_ready_handler);
#endif
}

#ifdef AM_PART_APOLLO3
//*****************************************************************************
//
// Interrupt handler for BLE
//
//*****************************************************************************
void am_ble_isr(void)
{
    HciDrvIntService();

    // Signal radio task to run
    WsfTaskSetReady(0, 0);
}

#else
//*****************************************************************************
//
// Interrupt handler for the CTS pin
//
//*****************************************************************************
void radio_cts_handler(void)
{
#if (BLE_STACK_DEBUG == 1)    
    SEGGER_RTT_printf(0,"radio cts isr DataReadyHandlerId=%d\n",g_bleDataReadyHandlerId);
#endif

    //step 1: set wakeup event from the BLE radio
    WsfSetEvent(g_bleDataReadyHandlerId, BLE_DATA_READY_EVENT);

    //step 2: Signal radio task to run 
    WsfTaskSetReady(0,0);
}
#endif

//*****************************************************************************
//
// Perform initial setup for the radio task.
//
//*****************************************************************************
void RadioTaskSetup(void)
{
    // Create an event handle for our wake-up events.
    xRadioEventHandle = xEventGroupCreate();

    // Make sure we actually allocated space for the events we need.
    while( xRadioEventHandle == NULL);

    // Pass event object to WSF scheduler
    wsfOsSetEventObject((void*)xRadioEventHandle);

#ifdef AM_PART_APOLLO3
    //设置蓝牙中断优先级
#if AM_CMSIS_REGS
    NVIC_SetPriority(BLE_IRQn, NVIC_configMAX_SYSCALL_INTERRUPT_PRIORITY);
#else // AM_CMSIS_REGS
    am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_BLE, configMAX_SYSCALL_INTERRUPT_PRIORITY);
#endif // AM_CMSIS_REGS
    
#else
    HciDrvRadioSetCallBack(radio_cts_handler);
#endif
    // Boot the radio.
    HciDrvRadioBoot(0);
}

//*****************************************************************************
//
// Short Description.
//
//*****************************************************************************
void RadioTask(void *pvParameters)
{
    // Initialize the main ExactLE stack.
    exactle_stack_init();

#ifndef AM_PART_APOLLO3
    //Enable EM9304 data ready pin for interrupt
    Drv_EM9304_EnableInterrupt();
#endif

    // Start the "Amdtp" profile.
    //AmdtpStart();
    app_m_ble_main_start();

    while (1)
    {
        // Calculate the elapsed time from our free-running timer, and update
        // the software timers in the WSF scheduler.
        update_scheduler_timers();
        wsfOsDispatcher();
    
        // Enable an interrupt to wake us up next time we have a scheduled
        // event.
        set_next_wakeup();
    
        // Check to see if the WSF routines are ready to go to sleep.
        if ( wsfOsReadyToSleep() )
        {
            // Attempt to shut down the UART. If we can shut it down
            // successfully, we can go to deep sleep. Otherwise, we'll need to
            // stay awake to finish processing the current packet.
    
            // Wait for an event to be posted to the Radio Event Handle.
            xEventGroupWaitBits(xRadioEventHandle, 1, pdTRUE,
                     pdFALSE, portMAX_DELAY);
        }
    }
}

//*****************************************************************************
//Create Radio Task
//*****************************************************************************
void RadioCreateTask(void)
{
   xTaskCreate(RadioTask, "RadioTask", 1024, 0, TASK_PRIORITY_MIDDLE_HIGH, &radio_task_handle);
}

