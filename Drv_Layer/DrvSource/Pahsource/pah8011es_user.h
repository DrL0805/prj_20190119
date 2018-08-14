#ifndef PAH8011ES_H
#define PAH8011ES_H
//*****************************************************************************
//
// Required built-ins.
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

//*****************************************************************************
//
// Standard AmbiqSuite includes.
//
//*****************************************************************************
//#include "am_mcu_apollo.h"
//#include "am_bsp.h"
//#include "am_devices.h"
//#include "am_util.h"
//*****************************************************************************
//
// Macro defines and extern functions
//
//*****************************************************************************
#include "pah8series_api_c.h"


/*============================================================================
SOME DEFINITION
============================================================================*/
#define PPG_IR_CH_NUM           0

//#define ENABLE_MEMS_ZERO		//no gsensor， keep this define.，计算心率值不使用重力数据时，保持该定义，否则关闭
#define MAX_MEMS_SAMPLE_NUM 	100

#define ENABLE_PXI_ALG
#define ALG_GSENSOR_MODE		0//1   // 0:2G, 1:4G, 2:8G, 3:16G

typedef enum
{
    main_status_idle,
    main_status_start_healthcare,
} main_status_e;


typedef enum
{
    factory_test_light_leak,
	factory_test_ir_led_golden_only,
	factory_test_ir_led_target_sample,
	factory_test_green_led_golden_only,
    factory_test_green_led_target_sample,
	factory_test_led_golden_only,
    factory_test_led_target_sample,
    factory_test_num,
} factory_test_e;


typedef struct
{
    // status
    main_status_e       status;

	// pxialg
	void*				pxialg_buffer;
	pah8series_data_t	pxialg_data;
	bool				pxialg_has_init;
	uint64			last_report_time;
    
	// gsensor
#if defined(ENABLE_MEMS_ZERO)
	int16				mems_data[MAX_MEMS_SAMPLE_NUM * 3];
#endif
    
} main_state_s;


/*============================================================================
STATIC FUNCTION PROTOTYPES 2017年6月9日20:08:30统一修改 去掉static
============================================================================*/
//extern factory_test_e;
extern main_state_s _state;
void start_healthcare_ppg_touch(uint64 tickcount);
void start_healthcare_touch_only(void);
void stop_healthcare(void);
uint8 report_fifo_data(uint64 timestamp, uint8 *fifo_data, uint32 fifo_data_num_per_ch, uint32 ch_num, bool is_touched);
bool hr_algorithm_calculate(pah8series_data_t *pxialg_data, uint32 ch_num,uint8 *hr_cal);
void hr_algorithm_close(void);

void Error_Handler(void);
void log_pah8series_data_header(uint32 ch_num, uint32 ir_ch_num, uint32 g_sensor_mode);
void log_pah8series_data(const pah8series_data_t *data);
void pah8011_ae_info_check(void);

void pah8011_gSensor_Range_Set(uint8 newrange);
#endif
