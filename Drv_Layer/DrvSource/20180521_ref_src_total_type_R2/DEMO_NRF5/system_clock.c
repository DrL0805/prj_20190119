
/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include "system_clock.h"
#include "rtos.h"
#include "am_util_delay.h"

#define SystermTickPeriod   1000/APP_1SEC_TICK      //TICK ±÷”÷‹∆⁄

uint32_t get_tick_count(void)
{
	return ( SystermTickPeriod * xTaskGetTickCount());
}

void delay_ms(uint32_t volatile number_of_ms)
{
	am_util_delay_ms(number_of_ms);
}

