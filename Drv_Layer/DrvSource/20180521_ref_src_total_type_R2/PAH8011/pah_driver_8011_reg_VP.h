/*==============================================================================
* Edit History
*
* This section contains comments describing changes made to the module. Notice
* that changes are listed in reverse chronological order. Please use ISO format
* for dates.
*
* when        version  who       what, where, why
* ----------  ------   ---       -----------------------------------------------------------
* 2016-11-21   1004    jl	     - Combine 8011ES/ET PC /ET DI Setting.
* 2016-10-03   1003    jl	     - Combine 8011ES/ET Setting.
* 2016-09-08   1002    bh        - Adjust max frame period for coverage skin tone.
* 2016-07-07   1001    bh        - Initial revision.
==============================================================================*/

#ifndef __pah_driver_8011_reg_VP_h__
#define __pah_driver_8011_reg_VP_h__


#include "pah_platform_types.h"
#include "pah_driver_types.h"

#define PAH_DRIVER_8011_REG_VERSION_VP   2001


/*==============================================================================
*
*       PAH8011ES DI+wall Cover Setting
*
==============================================================================*/

#ifdef __PAH8011ES_DI_WALL_COVER
#define SETTING_VERSION_VP 0x91
static const uint8_t pah8011_init_register_array_VP[][2] = {
    { 0x7F, 0x00 },//change to bank0
    { 0x16, 0x08 },
    { 0x17, 0x02 },
    { 0x19, 0x2E },
    { 0x35, 0x08 },
    { 0x36, 0x02 },
    { 0x38, 0x2E },
    { 0x54, 0x08 },
    { 0x55, 0x02 },
    { 0x57, 0x2E },
    { 0x70, 0x01 },
    { 0x71, 0x02 },
    { 0x72, 0x06 },
    { 0x73, 0x0F },
    { 0x74, 0x0F },
    { 0x75, 0x0F },
    { 0x7F, 0x04 },//change to bank4
    { 0x15, 0x69 },
    { 0x2B, 0xFE },
    { 0x34, 0x01 },
    { 0x70, 0x18 },
    { 0x7F, 0x05 },//change to bank 5
    { 0x44, 0x04 },
    { 0x5D, 0x08 },
    { 0x60, 0x35 },
    { 0x6A, 0x6D },
    { 0x58, 0x06 },
    { 0x59, 0x06 },
    { 0x5A, 0x06 },
    { 0x5B, 0x06 },
    { 0x7F, 0x01 },//change to bank1 
    { 0x02, 0x32 },
    { 0x05, 0x04 },
    { 0x08, 0x04 },
    { 0x14, 0x04 },
    { 0x1B, 0x01 },
    { 0x15, 0x28 },
    { 0x17, 0x28 },
    { 0x19, 0x28 },
    { 0x38, 0x00 },
    { 0x40, 0x00 },
    { 0x48, 0x00 },
    { 0x21, 0x68 },
    { 0x23, 0x68 },
    { 0x25, 0x49 },
    { 0x5A, 0x02 },
    { 0x61, 0x01 }, //Cmd_encode_EnH
    { 0x6F, 0x07 }, //Cmd_encode_sel[3:0]
    { 0x74, 0x01 },
};
static const uint8_t pah8011_touch_register_array_VP[][2] = {
    { 0x7F, 0x05 }, //change to bank5
    { 0x44, 0x1F },
    { 0x7F, 0x01 }, //change to bank1
    { 0x1C, 0x01 },
    { 0x1D, 0x28 },
    { 0x26, 0xA0 },
    { 0x27, 0x0F },
    { 0x12, 0x01 },
      
};


static const uint8_t pah8011_ppg_register_array_VP[][2] = {
    { 0x7F, 0x05 }, //change to bank5
    { 0x44, 0x04 },
    { 0x7F, 0x01 }, //change to bank1
    { 0x1C, 0x01 },
    { 0x1D, 0x28 },
    { 0x26, 0x40 },
    { 0x27, 0x01 },
    { 0x12, 0x64 },

    
};

#endif



#endif	// header guard
