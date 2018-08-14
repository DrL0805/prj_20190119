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
    pah8011_reg_array_mode_ppg_20hz,

} pah8011_reg_array_mode_e;


bool        pah8011_reg_array_init(void);
bool        pah8011_reg_array_load_init(void);
bool        pah8011_reg_array_load_mode(pah8011_reg_array_mode_e reg_array_mode);

void        pah8011_get_ppg_ch_enabled(uint8 ppg_ch_enabled[pah8011_ppg_ch_num]);
uint32    pah8011_get_ppg_ch_num(void);
uint8 pah8011_get_setting_version(void);

#endif  // header guard
