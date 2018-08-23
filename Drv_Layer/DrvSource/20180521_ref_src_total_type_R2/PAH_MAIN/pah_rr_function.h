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

#ifndef __pah_rr_function_h__
#define __pah_rr_function_h__


#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "pah_driver_types.h"

void pah8series_ppg_dri_RR_init(void);
void pah8series_ppg_dri_RR_task(volatile bool* has_interrupt_pah,volatile bool* has_interrupt_button ,volatile uint64_t*    interrupt_pah_timestamp);


void pah8series_ppg_polling_RR_init(void);
void pah8series_ppg_polling_RR_task(volatile bool* has_interrupt_pah,volatile bool* has_interrupt_button );

void pah8series_ppg_RR_start(void);
void pah8series_ppg_RR_stop(void);


/**
 * @brief Alg cal function
 *
 * When FIFO Data read success, please call this function.
 * This Function will run pixart rr alg.
 *
 * @return None.
 */
void rr_alg_task(void);

#endif  // header guard
