/*==============================================================================
* Edit History
* 
* This section contains comments describing changes made to the module. Notice
* that changes are listed in reverse chronological order. Please use ISO format
* for dates.
* 
* when       who       what, where, why
* ---------- ---       -----------------------------------------------------------
* 2016-12-30 bh        - Move Setting config. and  MODE DEFINITION
* 2016-09-26 bh        - Initial revision.
==============================================================================*/

#ifndef __pah_driver_types_h__
#define __pah_driver_types_h__


#include "pah_platform_types.h"

/*============================================================================
SOME DEFINITION
============================================================================*/
#define PPG_IR_CH_NUM           0

//#define ENABLE_MEMS_ZERO
#define MAX_MEMS_SAMPLE_NUM 	100

#define ENABLE_PXI_ALG
#define ALG_GSENSOR_MODE		0//1   // 0:2G, 1:4G, 2:8G, 3:16G

//#define PPG_MODE_ONLY
//#define Timing_Tuning

/*============================================================================
Sensor & Cover Type DEFINITION
============================================================================*/
#define __PAH8011ES_DI_COVER
//#define __PAH8011ES_DI_WALL_COVER
//#define __PAH8011ET_DI_WALL_SETTING_I2C
//#define __PAH8011ET_DI_WALL_SETTING_SPI
//#define __PAH8011ET_PC_SETTING_I2C
//#define __PAH8011ET_PC_SETTING_SPI



typedef enum {

    pah_stream_default,
    pah_stream_dri = pah_stream_default,    // Data ready interrupt
    pah_stream_polling,                     // Polling

} pah_stream_e;


typedef enum {

    pah_intshape_pulse_type_default,
    pah_intshape_pulse_type_edge = pah_intshape_pulse_type_default,   // Edge-sensitive
    pah_intshape_pulse_type_level,                                    // Level-sensitive

} pah_intshape_pulse_type_e;


typedef enum {

    pah_ppg_led_on_default,
    pah_ppg_led_on_instantly = pah_ppg_led_on_default,    // Turn on LED when enabled PPG
    pah_ppg_led_on_deferred,                              // Turn on LED when enabled PPG and detected first touch 

} pah_ppg_led_on_e;


typedef enum {

    pah_none,                   // Not used

    pah_stop_mode,              // Stop mode

    pah_touch_mode,             // Touch mode

    pah_ppg_mode,               // PPG mode
    pah_ppg_200hz_mode,         // PPG(200Hz) mode

    pah_ppg_touch_mode,         // PPG + Touch mode
    pah_ppg_200hz_touch_mode,   // PPG(200Hz) + Touch mode

} pah_mode;


typedef enum {

    pah_device_ppg,             // PPG device
    pah_device_touch,           // Touch device

} pah_device;


typedef enum {

    pah_success,                // Success
    pah_pending,                // Do nothing. Usually happens when task() detected no interrupt in dri mode or fifo number is not enough.

    pah_err_unknown,            // Unknown error
    pah_err_invalid_argument,   // Invalid argument to function
    pah_err_invalid_operation,  // Invalid operation to function
    pah_err_not_implement,      // Not implemented in this driver
    pah_err_platform_fail,      // Platform function failed
    pah_err_comm_fail,          // Communication(I2C, SPI, ...) failed

    pah_err_invalid_program,    // Generally this indicates a bug in the driver

    pah_err_verify_device_fail, // Verify device failed

    pah_err_fifo_checksum_fail, // Failed to compare check sum with fifo data
                                // Usually happens to bad data communication

    pah_err_fifo_overflow,      // Fifo in device is overflow
                                // Usually happens when pah_task() was too late to be called
    
    pah_err_fifo_underflow,     // Fifo in device is underflow

} pah_ret;
#define PAH_SUCCEEDED(ret)              (ret == pah_success)
#define PAH_SUCCEEDED_OR_PENDING(ret)   (ret == pah_success || ret == pah_pending)
#define PAH_FAILED(ret)                 (ret != pah_success)

#define pah_no_interrupt    pah_pending     // deprecated name


typedef struct {

    pah_stream_e                stream;
    pah_intshape_pulse_type_e   intshape_pulse_type;
    pah_ppg_led_on_e            ppg_led_on;

} pah_flags_s;


typedef struct {

    uint8*    data;
    uint8     touch_flag;

} pah_report_fifo;

typedef void(*pah_report_fifo_handle)(void* user_data, pah_report_fifo* fifo);



#endif  // header guard
