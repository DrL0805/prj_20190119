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

#ifndef __pah_vp_function_h__
#define __pah_vp_function_h__


#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "pah_driver_types.h"

void pah8series_ppg_dri_VP_init(void);
void pah8series_ppg_dri_VP_task(volatile bool* has_interrupt_pah,volatile bool* has_interrupt_button ,volatile uint64_t*    interrupt_pah_timestamp);


void pah8series_ppg_polling_VP_init(void);
void pah8series_ppg_polling_VP_task(volatile bool* has_interrupt_pah,volatile bool* has_interrupt_button );

void pah8series_ppg_dri_VP_Calibration_start(float real_sbp,float real_dbp);
//V117
void pah8series_ppg_dri_VP_Phy_Calibration_start(uint32_t user_gender,uint32_t user_age, float user_height, float user_weight);
void pah8series_ppg_dri_VP_Estimation_start(void);
//V117
void pah8series_ppg_dri_VP_Phy_Estimation_start(void);
//Without calibration
void pah8series_ppg_dri_VP_WithoutCal_Estimation_start(void);

void pah8series_ppg_VP_stop(void);
void vp_alg_task(void);


#endif  // header guard

