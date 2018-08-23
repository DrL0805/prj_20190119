/*==============================================================================
* Edit History
* 
* This section contains comments describing changes made to the module. Notice
* that changes are listed in reverse chronological order. Please use ISO format
* for dates.
* 
* when       who       what, where, why
* ---------- ---       -----------------------------------------------------------
* 2016-07-07 bh        Initial revision.
==============================================================================*/

#ifndef __pah_driver_8011_reg_array_h__
#define __pah_driver_8011_reg_array_h__


#include "pah_platform_types.h"


typedef enum {
    pah8011_ppg_ch_a,
    pah8011_ppg_ch_b,
    pah8011_ppg_ch_c,
    pah8011_ppg_ch_num,
} pah8011_ppg_ch;


typedef enum {

    pah8011_reg_array_mode_none,
    pah8011_reg_array_mode_touch,
    pah8011_reg_array_mode_ppg,

} pah8011_reg_array_mode_e;

typedef enum {
	
    pah8011_alg_HR,
    pah8011_alg_VP,  
	  pah8011_alg_SPO2,
	  pah8011_alg_HRV,
	  pah8011_alg_RR,
} pah8011_alg_mode;

bool        pah8011_reg_array_init(uint8_t mode);
bool        pah8011_reg_array_load_init(void);
bool        pah8011_reg_array_load_mode(pah8011_reg_array_mode_e reg_array_mode);

void        pah8011_get_ppg_ch_enabled(uint8_t ppg_ch_enabled[pah8011_ppg_ch_num]);
uint32_t    pah8011_get_ppg_ch_num(void);
uint8_t pah8011_get_setting_version(void);
uint8_t pah8011_get_setting_version_VP(void);

#endif  // header guard
