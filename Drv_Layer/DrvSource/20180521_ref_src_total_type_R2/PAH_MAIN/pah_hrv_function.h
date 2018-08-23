/*==============================================================================
* Edit History
* 
* This section contains comments describing changes made to the module. Notice
* that changes are listed in reverse chronological order. Please use ISO format
* for dates.
* 
* when       who       what, where, why
* ---------- ---       -----------------------------------------------------------
* 2016-04-29 bell      Initial revision.
==============================================================================*/

#ifndef __pah_hrv_function_h__
#define __pah_hrv_function_h__


#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "pah_driver_types.h"

/*****************************************
 * Global Function Declaration
 *****************************************/
/**
 * @brief HRV init Function.
 *
 * If sensor power up , please do pah8series_ppg_dri_HRV_init() 
 * This Function link register and set PPG sensor to power down
 *
 * @return None.
 */
void pah8series_ppg_dri_HRV_init(void);
/**
 * @brief PPG HRV task Function.
 * @param has_interrupt_pah pointer to GPIO interrupt handler.
 * @param has_interrupt_button pointer to GPIO(button) interrupt handler.
 * @param interrupt_pah_timestamp pointer to System tick count.
 * @return None.
 */
void pah8series_ppg_dri_HRV_task(volatile bool* has_interrupt_pah,volatile bool* has_interrupt_button ,volatile uint64_t*    interrupt_pah_timestamp);

/**
 * @brief Polling HRV init Function.
 *
 * If sensor power up , please do pah8series_ppg_polling_HRV_init() 
 * This Function link register and set PPG sensor to power down
 *
 * @return None.
 */
void pah8series_ppg_polling_HRV_init(void);
/**
 * @brief Polling HRV task Function.
 * @param has_interrupt_pah pointer to GPIO interrupt handler.
 * @param has_interrupt_button pointer to GPIO(button) interrupt handler.
 * @return None.
 */
void pah8series_ppg_polling_HRV_task(volatile bool* has_interrupt_pah,volatile bool* has_interrupt_button );

/**
 * @brief HRV Start Function.
 *
 * If customer want to use PPG Sensor , please call pah8series_ppg_HRV_start()
 * This Function will enable the ppg sensor
 *
 * @return None.
 */
void pah8series_ppg_HRV_start(void);
/**
 * @brief HRV STOP Function.
 *
 * If customer want to stop PPG Sensor , please call pah8series_ppg_HRV_stop()
 * This Function will disable the ppg sensor
 *
 * @return None.
 */
void pah8series_ppg_HRV_stop(void);

/**
 * @brief Alg cal function
 *
 * When FIFO Data read success, please call this function.
 * This Function will run pixart hrv alg.
 *
 * @return None.
 */
void hrv_alg_task(void);


#endif  // header guard
