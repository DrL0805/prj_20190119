


// gsensor
#include "accelerometer.h"

// platform support
#include "board.h"
#include "gpio_ctrl.h"
#include "system_clock.h"
#include "i2c.h"
#include "spi.h"
#include "debug_log.h"


// c
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

// pah8011_Driver
#include "pah_driver.h"
#include "pah_comm.h"
#include "pah_driver_types.h"

// pah8011_HR_Function
#include "pah_hrd_function.h"
// pah8011_VP_Function
#include "pah_vp_function.h"
// pah8011_HRV_Function
#include "pah_hrv_function.h"
// pah8011_RR_Function
#include "pah_rr_function.h"

// pah

#include "pah_verify.h"
//Pixart Factory test 
#include "pah_factory_test.h"





typedef enum
{
    factory_test_light_leak,
    factory_test_led_golden_only,
    factory_test_led_target_sample,
    factory_test_touch_value,
    factory_test_INT_high,
    factory_test_INT_low,
    factory_test_power_noise,
    factory_test_stop,
    factory_test_num,
} factory_test_e;


/*============================================================================
STATIC VARIABLES
============================================================================*/


static volatile bool        has_interrupt_button = false;

static volatile bool        has_interrupt_pah = false;

static volatile uint64_t    interrupt_pah_timestamp = 0;





/*============================================================================
STATIC FUNCTION PROTOTYPES
============================================================================*/
void demo_touch_mode_dri(void);
void demo_touch_mode_polling(void);
void demo_ppg_dri_HRD(void);
void demo_ppg_polling_HRD(void);
void demo_ppg_dri_VP(void);
void demo_ppg_polling_VP(void);
void demo_ppg_dri_HRV(void);
void demo_ppg_polling_HRV(void);
void demo_ppg_dri_RR(void);
void demo_ppg_polling_RR(void);

void demo_preprocessing(void);
void demo_factory_mode(void);

static void gpio_in_interrupt_handler(GPIO_IN gpio);
static void gpio_btn_interrupt_handler(GPIO_BTN gpio);
static void Error_Handler(void);



int main(void)
{
 
    
    // bsp
    board_init();
    system_clock_init();
    gpio_init();
	//button_init; //for demo_preprocessing mode
    gpio_in_set_interrupt_callback(gpio_in_interrupt_handler);
    gpio_btn_set_interrupt_callback(gpio_btn_interrupt_handler);
    debug_log_init();
    
    // pause
    debug_printf("\n");
    debug_printf("Wait for button to start... \n");
    while (!has_interrupt_button);
    has_interrupt_button = false;
    delay_ms(1000);
    
    // gsensor
    if (!accelerometer_init())
        Error_Handler();
    
    // i2c
    if (!i2c_init(pah_get_i2c_slave_addr()))
        Error_Handler();
    //if (!spi_init())
    //    Error_Handler();
/*============================================================================
For PPG_INT_MODE
============================================================================*/    
    //demo_ppg_dri_HRD();
	//demo_ppg_dri_VP();
	//demo_ppg_dri_HRV();
	//demo_ppg_dri_RR();
/*============================================================================
For PPG_POLLING_MODE
============================================================================*/    
    //demo_ppg_polling_HRD();
	//demo_ppg_polling_VP();
	//demo_ppg_polling_HRV();
	//demo_ppg_polling_RR();
/*============================================================================
For Touch_Mode_POLLING_MODE
============================================================================*/
    //demo_touch_mode_polling();
/*============================================================================
For Touch_dri_MODE
============================================================================*/    
	//demo_touch_mode_dri();
/*============================================================================
For FACTORY_TEST_MODE
============================================================================*/
    //demo_factory_mode();
/*============================================================================
For Preprocessing_MODE
============================================================================*/
	//demo_preprocessing();

}


/////////////////////////8011ES DRIVER MAIN////////////////////////
void demo_touch_mode_dri(void)
{
    pah8series_ppg_dri_HRD_init();	
    pah8series_touch_mode_start();
    while (1)
    {
        pah8series_touch_mode_dri_task(&has_interrupt_pah,&has_interrupt_button,&interrupt_pah_timestamp);
		
	    /**if customer want to stop PPG Sensor , please call pah8series_ppg_HRD_stop()
	     * This Function will disable the ppg sensor
		 */
		//pah8series_ppg_HRD_stop();
    }   // end of while
}


void demo_touch_mode_polling(void)
{
    pah8series_ppg_polling_HRD_init();
    pah8series_touch_mode_start();
    while (1)
    {
        pah8series_touch_mode_polling_task(&has_interrupt_pah,&has_interrupt_button);
        /**if customer want to stop PPG Sensor , please call pah8series_ppg_HRD_stop()
		 * This Function will disable the ppg sensor. */
		//pah8series_ppg_HRD_stop();		
        delay_ms(1000);
    }    // end of while
}



void demo_ppg_dri(void)
{	
	pah8series_ppg_dri_HRD_init();
	pah8series_ppg_HRD_start();
	while(1)
	{
		/**pah8series_ppg_dri_HRD_task, this function only read ppg data & set raw_data format, 
         * Customer can run this function on interrupt_handler or while loop. */
		pah8series_ppg_dri_HRD_task(&has_interrupt_pah ,&has_interrupt_button ,&interrupt_pah_timestamp);

        /**hrd_alg_task, this function will run pixart alg, Customer need run this function on 
         * low priority task before next pah8series_ppg_dri_HRD_task() */
        hrd_alg_task();

		/**if customer want to stop PPG Sensor , please call pah8series_ppg_HRD_stop()
		 * This Function will disable the ppg sensor. */
		//pah8series_ppg_HRD_stop();	
	}   // end of while
}



void demo_ppg_polling(void)
{
    pah8series_ppg_polling_HRD_init();
    pah8series_ppg_HRD_start();
	while (1)
    {
   		/**pah8series_ppg_polling_HRD_task, this function only read ppg data & set raw_data format, 
         * Customer can run this function on polling task or while loop. */ 
        pah8series_ppg_polling_HRD_task(&has_interrupt_pah,&has_interrupt_button );

        /**hrd_alg_task, this function will run pixart alg, Customer need run this function on 
         * low priority task before next pah8series_ppg_polling_HRD_task() */
        hrd_alg_task();

        /**if customer want to stop PPG Sensor , please call pah8series_ppg_HRD_stop()
        * This Function will disable the ppg sensor. */
        //pah8series_ppg_HRD_stop();	
		delay_ms(1000);
    }    // end of while
}


void demo_ppg_dri_HRV(void)
{
    //if sensor power up , please do pah8series_ppg_dri_HRV_init() 
	// This Function link register and set PPG sensor to power down
    pah8series_ppg_dri_HRV_init();

    //if customer want to use PPG Sensor , please call pah8series_ppg_HRV_start()
	// This Function will enable the ppg sensor
	pah8series_ppg_HRV_start();
    
    while (1)
    {
        pah8series_ppg_dri_HRV_task(&has_interrupt_pah,&has_interrupt_button ,&interrupt_pah_timestamp);
        
			/**hrd_alg_task, this function will run pixart alg, Customer need run this function on 
        * low priority task before next pah8series_ppg_dri_HRD_task() */
        hrv_alg_task();		
        //if customer want to stop PPG Sensor , please call pah8series_ppg_HRV_stop()
		// This Function will disable the ppg sensor
		//pah8series_ppg_HRV_stop();

    }    // end of while
}


void demo_ppg_dri_RR(void)
{
    //if sensor power up , please do pah8series_ppg_dri_RR_init() 
	// This Function link register and set PPG sensor to power down
   pah8series_ppg_dri_RR_init();

    //if customer want to use PPG Sensor , please call pah8series_ppg_RR_start()
	// This Function will enable the ppg sensor
	pah8series_ppg_RR_start();
    
    while (1)
    {
        pah8series_ppg_dri_RR_task(&has_interrupt_pah,&has_interrupt_button ,&interrupt_pah_timestamp);
        
			/**rr_alg_task, this function will run pixart alg, Customer need run this function on 
        * low priority task before next pah8series_ppg_dri_rr_task() */
        rr_alg_task();		
        //if customer want to stop PPG Sensor , please call pah8series_ppg_RR_stop()
		// This Function will disable the ppg sensor
		//pah8series_ppg_RR_stop();

    }    // end of while
}



void demo_ppg_polling_RR(void)
{
    //if sensor power up , please do pah8series_ppg_polling_RR_init() 
	// This Function link register and set PPG sensor to power down
    pah8series_ppg_polling_RR_init();

    //if customer want to use PPG Sensor , please call pah8series_ppg_RR_start()
	// This Function will enable the ppg sensor
	pah8series_ppg_RR_start();

    while (1)
    {
        pah8series_ppg_polling_RR_task(&has_interrupt_pah,&has_interrupt_button );
			
			/**rr_alg_task, this function will run pixart alg, Customer need run this function on 
        * low priority task before next pah8series_ppg_dri_rr_task() */
        rr_alg_task();				
			
        //if customer want to stop PPG Sensor , please call pah8series_ppg_RR_stop()
		// This Function will disable the ppg sensor
		//pah8series_ppg_RR_stop();
        delay_ms(1000);
    }    // end of while
}

void demo_ppg_polling_HRV(void)
{
    //if sensor power up , please do pah8series_ppg_polling_HRV_init() 
	// This Function link register and set PPG sensor to power down
    pah8series_ppg_polling_HRV_init();

    //if customer want to use PPG Sensor , please call pah8series_ppg_HRV_start()
	// This Function will enable the ppg sensor
	pah8series_ppg_HRV_start();

    while (1)
    {
        pah8series_ppg_polling_HRV_task(&has_interrupt_pah,&has_interrupt_button );
			
			/**hrd_alg_task, this function will run pixart alg, Customer need run this function on 
        * low priority task before next pah8series_ppg_dri_HRD_task() */
        hrv_alg_task();				
			
        //if customer want to stop PPG Sensor , please call pah8series_ppg_HRV_stop()
		// This Function will disable the ppg sensor
		//pah8series_ppg_HRV_stop();
        delay_ms(1000);
    }    // end of while
}



void demo_ppg_dri_VP(void)
{
	//if sensor power up , please do pah8series_ppg_dri_VP_init()
	//This Function link register and set PPG sensor to power down
    pah8series_ppg_dri_VP_init();

    //Calibration Mode//
    //if customer want to start VP calibration, please call pah8series_ppg_dri_VP_Calibration_start(real_sbp,real_dbp)
    //This Function will enable the ppg sensor, and do calibration with real sbp/dbp(ex:from omron)
    pah8series_ppg_dri_VP_Calibration_start(120,60);

    //Physical Calibration Mode//
    //if customer want to start VP calibration, please call pah8series_ppg_dri_VP_Calibration_start(user_gender,user_age,user_height,user_weight)
    //This Function will enable the ppg sensor, and do calibration with physical information(gender,age,height,weight)), 1:male, 0:female
    //pah8series_ppg_dri_VP_Phy_Calibration_start(1,25,170,60);

    //Estimation Mode//
    //if customer want to start VP estimation, please call pah8series_ppg_dri_VP_Estimation_start();
    //This Function will enable the ppg sensor, and should be used after clibration is done
    //pah8series_ppg_dri_VP_Estimation_start();

    //Physical Estimation Mode//
    //if customer want to start VP estimation, please call pah8series_ppg_dri_VP_Estimation_start();
    //This Function will enable the ppg sensor, and should be used after physical clibration is done
    //pah8series_ppg_dri_VP_Phy_Estimation_start();
    
    //WithoutCal Estimation Mode//
    //pah8series_ppg_dri_VP_WithoutCal_Estimation_start();

    while (1)
    {      
        pah8series_ppg_dri_VP_task(&has_interrupt_pah,&has_interrupt_button ,&interrupt_pah_timestamp);
		
		
		vp_alg_task();
		//if customer want to stop PPG Sensor , please call pah8series_ppg_VP_stop()
		//This Function will disable the ppg sensor
		//pah8series_ppg_VP_stop();
        
    }    // end of while
}

void demo_ppg_polling_VP(void)
{
    //if sensor power up , please do pah8series_ppg_polling_VP_init()
	//This Function link register and set PPG sensor to power down
    pah8series_ppg_polling_VP_init();

    //Calibration Mode//
    //if customer want to start VP calibration, please call pah8series_ppg_dri_VP_Calibration_start(real_sbp,real_dbp)
    //This Function will enable the ppg sensor, and do calibration with real sbp/dbp(ex:from omron)
	pah8series_ppg_dri_VP_Calibration_start(120,60); 

    //Physical Calibration Mode//
    //if customer want to start VP calibration, please call pah8series_ppg_dri_VP_Calibration_start(user_gender,user_age,user_height,user_weight)
    //This Function will enable the ppg sensor, and do calibration with physical information(gender,age,height,weight)), 1:male, 0:female
    //pah8series_ppg_dri_VP_Phy_Calibration_start(1,25,170,60);

	//Estimation Mode//
	//if customer want to start VP estimation, please call pah8series_ppg_polling_VP_Estimation_start();
	//This Function will enable the ppg sensor, and should be used after clibration is done
	//pah8series_ppg_dri_VP_Estimation_start();

    //Physical Estimation Mode//
    //if customer want to start VP estimation, please call pah8series_ppg_dri_VP_Estimation_start();
    //This Function will enable the ppg sensor, and should be used after physical clibration is done
    //pah8series_ppg_dri_VP_Phy_Estimation_start();
    
    //WithoutCal Estimation Mode//
    //pah8series_ppg_dri_VP_WithoutCal_Estimation_start();

    while (1)
    {
        pah8series_ppg_polling_VP_task(&has_interrupt_pah,&has_interrupt_button );
		
		
		vp_alg_task();
		//if customer want to stop PPG Sensor , please call pah8series_ppg_VP_stop()
		//This Function will disable the ppg sensor
		//pah8series_ppg_VP_stop();
        delay_ms(200);
    }    // end of while
}


void demo_preprocessing(void)
{	
	pah8series_ppg_dri_HRD_init();
	pah8series_ppg_HRD_start();	
	while(1)
	{
        /**pah8series_ppg_preproc_HRD_task, this function only read ppg data & set raw_data format, 
         * Customer can run this function on polling task or while loop. */ 	
         pah8series_ppg_preproc_HRD_task(&has_interrupt_pah ,&has_interrupt_button ,&interrupt_pah_timestamp);

		/**hrd_alg_task, this function will run pixart alg, Customer need run this function on 
		 * low priority task before next pah8series_ppg_preproc_HRD_task() */
        hrd_alg_task();

        /**if customer want to stop PPG Sensor , please call pah8series_ppg_HRD_stop()
        * This Function will disable the ppg sensor. */
        //pah8series_ppg_HRD_stop();		
	}    // end of while
}



 
static void demo_factory_mode(void)
{
#ifdef FACTORY_TEST_ES
    //factory_test_mode(factory_test_led_golden_only,Enable,expo_ch_b,expo_ch_c);
    //Customer can turn expo time to Get ADC_Data 2000
    factory_test_mode(factory_test_led_golden_only,1,0x42,0x42);

    //Get Correct value for the expo_ch_b,expo_ch_c
    //Please Modify the pah_verify_8011_reg.h pah8011_verify_led_brightness_array[][2] array
    factory_test_mode(factory_test_led_target_sample,0,0,0);
	
    factory_test_mode(factory_test_light_leak,0,0,0);
	
    //For golden sample only
    factory_test_mode(factory_test_touch_value,0,0,0);	
	
    //Set Int Pin to LOW, Customer Check GPIO was low level
    factory_test_mode(factory_test_INT_low,0,0,0);	
	delay_ms(50);
	uint32_t in_level1 = nrf_gpio_pin_read(GPIO_INPUT_PIN_NUMBER);
	log_printf("in_level1 = %d   \n",in_level1);
	pah_enter_mode(pah_stop_mode);
	
	//Set Int Pin to HIGH, Customer Check GPIO was high level
	factory_test_mode(factory_test_INT_high,0,0,0);
	delay_ms(50);
	uint32_t in_level2 = nrf_gpio_pin_read(GPIO_INPUT_PIN_NUMBER);
	log_printf(" in_level2 = %d \n",in_level2);
	pah_enter_mode(pah_stop_mode);

    /** Set LED ExpoT&DAC Maximum used for test power noise. When the test is completed
     *  call pah_enter_mode(pah_stop_mode) this the function to leave power noise test.
     *  Exmaple: pah_enter_mode(pah_stop_mode);
     */
	factory_test_mode(factory_test_power_noise,0,0,0);
	
#endif

#ifdef FACTORY_TEST_ET
    //factory_test_mode(factory_test_led_golden_only,Enable,expo_ch_b,expo_ch_c);
    //Customer can turn expo time to Get ADC_Data 2000
    factory_test_mode(factory_test_led_golden_only,1,0x32,0x32);

    //Get Correct value for the expo_ch_b,expo_ch_c
    //Please Modify the pah_verify_8011_reg.h pah8011_verify_led_brightness_array[][2] array
    factory_test_mode(factory_test_led_target_sample,0,0,0);

    factory_test_mode(factory_test_light_leak,0,0,0);

    //For golden sample only
    factory_test_mode(factory_test_touch_value,0,0,0);
	
    //Set Int Pin to LOW, Customer Check GPIO was low level
    factory_test_mode(factory_test_INT_low,0,0,0);
	delay_ms(50);
	uint32_t in_level1 = nrf_gpio_pin_read(GPIO_INPUT_PIN_NUMBER);
	log_printf("in_level1 = %d   \n",in_level1);
	pah_enter_mode(pah_stop_mode);
	
	//Set Int Pin to HIGH, Customer Check GPIO was high level
	factory_test_mode(factory_test_INT_high,0,0,0);
	delay_ms(50);
	uint32_t in_level2 = nrf_gpio_pin_read(GPIO_INPUT_PIN_NUMBER);
	log_printf(" in_level2 = %d \n",in_level2);
	pah_enter_mode(pah_stop_mode);

    /** Set LED ExpoT&DAC Maximum used for test power noise. When the test is completed
     *  call pah_enter_mode(pah_stop_mode) this the function to leave power noise test.
     *  Exmaple: pah_enter_mode(pah_stop_mode);
     */
	factory_test_mode(factory_test_power_noise,0,0,0);

#endif

}


static void gpio_in_interrupt_handler(GPIO_IN gpio)
{
    if (gpio == GPIO_IN_1)
    {
        has_interrupt_pah = true ;
		interrupt_pah_timestamp = get_tick_count();
    }
}

static void gpio_btn_interrupt_handler(GPIO_BTN gpio)
{
    if (gpio == GPIO_BTN_1)
    {
		has_interrupt_button = true ;
	}
}

#if 0
/**@brief Function for the Power Management. 
 */ 
static void power_manage(void) 
{ 
    // Use directly __WFE and __SEV macros since the SoftDevice is not available. 
    
    // Wait for event. 
    __WFE(); 

    // Clear Event Register. 
    __SEV(); 
    __WFE(); 
} 
#endif

static void Error_Handler(void)
{
    debug_printf("GOT ERROR !!! \n");
    while (1)
    {
    }
}
