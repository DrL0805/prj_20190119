
#include "gpio_ctrl.h"
#include "app_error.h"

#include "nrf_gpio.h"
#include "system_clock.h"
#include "nrf_drv_gpiote.h"
#include "boards.h"

// c
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>


static const uint8_t m_board_btn_list_1[2] = BUTTONS_LIST_1;


extern void interrupt_dummy_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);


bool gpio_init(void)
{
		uint32_t err_code = NRF_SUCCESS;
	
		err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);
		//nrf_drv_gpiote_in_config_t config ;//= GPIOTE_CONFIG_IN_SENSE_LOTOHI(true) ;
		nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true) ;
    err_code = nrf_drv_gpiote_in_init(GPIO_INPUT_PIN_NUMBER, &config, interrupt_dummy_handler);
    APP_ERROR_CHECK(err_code);
		nrf_drv_gpiote_in_event_enable(GPIO_INPUT_PIN_NUMBER, 1);
    
	return true;
}

bool button_init(void)
{
	//	uint32_t err_code = NRF_SUCCESS;
	
	//err_code = app_button_init(ARDUINO_5_PIN,1,20);
	//APP_ERROR_CHECK(err_code);
    uint32_t i;
    for(i = 0; i < 2; ++i)
    {
        nrf_gpio_cfg_input(m_board_btn_list_1[i], BUTTON_PULL);
    }
    
	return true;
}

bool bsp_board_IOstate_get(uint8_t button_idx)
{
    bool pin_set = nrf_gpio_pin_out_read(m_board_btn_list_1[button_idx]) ? true : false;
    return pin_set;
}
