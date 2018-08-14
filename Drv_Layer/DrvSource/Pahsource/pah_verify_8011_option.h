/*==============================================================================
* Edit History
* 
* This section contains comments describing changes made to the module. Notice
* that changes are listed in reverse chronological order. Please use ISO format
* for dates.
* 
* when       who       what, where, why
* ---------- ---       -----------------------------------------------------------
* 2016-09-26 bh        - Initial revision.
==============================================================================*/

#ifndef __pah_verify_8011_option_h__
#define __pah_verify_8011_option_h__


#include "pah_platform_types.h"
#include "pah_driver_8011_reg.h"

//============================================================================
// define
//============================================================================
#ifndef UINT16_MAX
#   define UINT16_MAX   0xFFFF
#endif

//============================================================================
// options
//============================================================================
#define PAH_VERIFY_DELAY_BEFORE_READ    1000

#if defined(__PAH8011ES_DI_COVER) || defined(__PAH8011ES_DI_WALL_COVER)
//----- verify light leak ----- (Comment for not verify)
//#define PAH_LIGHT_LEAK_CH_A_MIN     0
//#define PAH_LIGHT_LEAK_CH_A_MAX     50
//#define PAH_LIGHT_LEAK_CH_B_MIN     0
#define PAH_LIGHT_LEAK_CH_B_MAX     50
//#define PAH_LIGHT_LEAK_CH_C_MIN     0
#define PAH_LIGHT_LEAK_CH_C_MAX     50
//#define PAH_LIGHT_LEAK_CH_T_MIN     0
#define PAH_LIGHT_LEAK_CH_T_MAX     30

//----- verify green led golden only ----- (Comment for not verify)
//#define PAH_GREEN_LED_GOLDEN_ONLY_CH_A_MIN      1950
//#define PAH_GREEN_LED_GOLDEN_ONLY_CH_A_MAX      2050
#define PAH_GREEN_LED_GOLDEN_ONLY_CH_B_MIN      1950
#define PAH_GREEN_LED_GOLDEN_ONLY_CH_B_MAX      2050
#define PAH_GREEN_LED_GOLDEN_ONLY_CH_C_MIN      1950
#define PAH_GREEN_LED_GOLDEN_ONLY_CH_C_MAX      2050

//----- verify green led target sample ----- (Comment for not verify)
//#define PAH_GREEN_LED_TARGET_SAMPLE_CH_A_MIN     600
//#define PAH_GREEN_LED_TARGET_SAMPLE_CH_A_MAX    4090
#define PAH_GREEN_LED_TARGET_SAMPLE_CH_B_MIN     600
#define PAH_GREEN_LED_TARGET_SAMPLE_CH_B_MAX    4090
#define PAH_GREEN_LED_TARGET_SAMPLE_CH_C_MIN     600
#define PAH_GREEN_LED_TARGET_SAMPLE_CH_C_MAX    4090




//----- verify ir led golden only ----- (Comment for not verify)
#define PAH_IR_LED_GOLDEN_ONLY_CH_T_MIN             2000
#define PAH_IR_LED_GOLDEN_ONLY_CH_T_MAX             2500

//----- verify ir led target sample -----
#define PAH_IR_LED_TARGET_SAMPLE_CH_T_GOLDEN    2200
#define PAH_IR_LED_TARGET_SAMPLE_CH_T_MIN_FACTOR    0.5
#define PAH_IR_LED_TARGET_SAMPLE_CH_T_MAX_FACTOR    1.5

//----- verify led golden only ----- (Comment for not verify)
//#define PAH_LED_GOLDEN_ONLY_CH_A_MIN      1950
//#define PAH_LED_GOLDEN_ONLY_CH_A_MAX      2050
#define PAH_LED_GOLDEN_ONLY_CH_B_MIN      1950
#define PAH_LED_GOLDEN_ONLY_CH_B_MAX      2050
#define PAH_LED_GOLDEN_ONLY_CH_C_MIN      1950
#define PAH_LED_GOLDEN_ONLY_CH_C_MAX      2050
#define PAH_LED_GOLDEN_ONLY_CH_T_MIN      2000
#define PAH_LED_GOLDEN_ONLY_CH_T_MAX      2500

//----- verify led target sample ----- (Comment for not verify)
//#define PAH_LED_TARGET_SAMPLE_CH_A_MIN     600
//#define PAH_LED_TARGET_SAMPLE_CH_A_MAX    4090
#define PAH_LED_TARGET_SAMPLE_CH_B_MIN     600
#define PAH_LED_TARGET_SAMPLE_CH_B_MAX    4090
#define PAH_LED_TARGET_SAMPLE_CH_C_MIN     600
#define PAH_LED_TARGET_SAMPLE_CH_C_MAX    4090
#define PAH_LED_TARGET_SAMPLE_CH_T_MIN    PAH_IR_LED_TARGET_SAMPLE_CH_T_GOLDEN*PAH_IR_LED_TARGET_SAMPLE_CH_T_MIN_FACTOR//1000
#define PAH_LED_TARGET_SAMPLE_CH_T_MAX    PAH_IR_LED_TARGET_SAMPLE_CH_T_GOLDEN*PAH_IR_LED_TARGET_SAMPLE_CH_T_MAX_FACTOR//3000


#endif

#if defined(__PAH8011ET_DI_WALL_SETTING_I2C) || defined(__PAH8011ET_DI_WALL_SETTING_SPI)

//----- verify light leak ----- (Comment for not verify)
//#define PAH_LIGHT_LEAK_CH_A_MIN     0
#define PAH_LIGHT_LEAK_CH_A_MAX     50
//#define PAH_LIGHT_LEAK_CH_B_MIN     0
#define PAH_LIGHT_LEAK_CH_B_MAX     50
//#define PAH_LIGHT_LEAK_CH_C_MIN     0
#define PAH_LIGHT_LEAK_CH_C_MAX     50
//#define PAH_LIGHT_LEAK_CH_T_MIN     0
#define PAH_LIGHT_LEAK_CH_T_MAX     50


//----- verify led golden only ----- (Comment for not verify)
//#define PAH_LED_GOLDEN_ONLY_CH_A_MIN      1950
//#define PAH_LED_GOLDEN_ONLY_CH_A_MAX      2050
#define PAH_LED_GOLDEN_ONLY_CH_B_MIN      1950
#define PAH_LED_GOLDEN_ONLY_CH_B_MAX      2050
#define PAH_LED_GOLDEN_ONLY_CH_C_MIN      1950
#define PAH_LED_GOLDEN_ONLY_CH_C_MAX      2050
#define PAH_LED_GOLDEN_ONLY_CH_T_MIN      1950
#define PAH_LED_GOLDEN_ONLY_CH_T_MAX      2050

//----- verify led target sample ----- (Comment for not verify)
//#define PAH_LED_TARGET_SAMPLE_CH_A_MIN     600
//#define PAH_LED_TARGET_SAMPLE_CH_A_MAX    4090
#define PAH_LED_TARGET_SAMPLE_CH_B_MIN     600
#define PAH_LED_TARGET_SAMPLE_CH_B_MAX    4090
#define PAH_LED_TARGET_SAMPLE_CH_C_MIN     600
#define PAH_LED_TARGET_SAMPLE_CH_C_MAX    4090
#define PAH_LED_TARGET_SAMPLE_CH_T_MIN     1000
#define PAH_LED_TARGET_SAMPLE_CH_T_MAX    3000

#endif

#if defined(__PAH8011ET_PC_SETTING_I2C) || defined(__PAH8011ET_PC_SETTING_SPI)
//----- verify light leak ----- (Comment for not verify)
//#define PAH_LIGHT_LEAK_CH_A_MIN     0
#define PAH_LIGHT_LEAK_CH_A_MAX     50
//#define PAH_LIGHT_LEAK_CH_B_MIN     0
#define PAH_LIGHT_LEAK_CH_B_MAX     50
//#define PAH_LIGHT_LEAK_CH_C_MIN     0
#define PAH_LIGHT_LEAK_CH_C_MAX     50
//#define PAH_LIGHT_LEAK_CH_T_MIN     0
#define PAH_LIGHT_LEAK_CH_T_MAX     50


//----- verify led golden only ----- (Comment for not verify)
//#define PAH_LED_GOLDEN_ONLY_CH_A_MIN      1950
//#define PAH_LED_GOLDEN_ONLY_CH_A_MAX      2050
#define PAH_LED_GOLDEN_ONLY_CH_B_MIN      1950
#define PAH_LED_GOLDEN_ONLY_CH_B_MAX      2050
#define PAH_LED_GOLDEN_ONLY_CH_C_MIN      1950
#define PAH_LED_GOLDEN_ONLY_CH_C_MAX      2050
#define PAH_LED_GOLDEN_ONLY_CH_T_MIN      1950
#define PAH_LED_GOLDEN_ONLY_CH_T_MAX      2050

//----- verify led target sample ----- (Comment for not verify)
//#define PAH_LED_TARGET_SAMPLE_CH_A_MIN     600
//#define PAH_LED_TARGET_SAMPLE_CH_A_MAX    4090
#define PAH_LED_TARGET_SAMPLE_CH_B_MIN     600
#define PAH_LED_TARGET_SAMPLE_CH_B_MAX    4090
#define PAH_LED_TARGET_SAMPLE_CH_C_MIN     600
#define PAH_LED_TARGET_SAMPLE_CH_C_MAX    4090
#define PAH_LED_TARGET_SAMPLE_CH_T_MIN     1000
#define PAH_LED_TARGET_SAMPLE_CH_T_MAX    3000

#endif


#endif  // header guard
