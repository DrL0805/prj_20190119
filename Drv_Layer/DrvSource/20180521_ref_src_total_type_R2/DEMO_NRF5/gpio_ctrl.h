
#ifndef __gpio_ctrl_h__
#define __gpio_ctrl_h__

/*****************************************
 * Header files including
 *****************************************/
#include <stdint.h>
#include <stdbool.h>
#include "boards.h"
#include "nrf_drv_gpiote.h"


#define GPIO_INPUT_PIN_NUMBER ARDUINO_2_PIN  /**< Pin number for output. */

#define BUTTONS_LIST_1 {BUTTON_3, BUTTON_4 }

/*****************************************
 * Global Function Declaration
 *****************************************/
// initialization
bool gpio_init(void);

bool button_init(void);

bool bsp_board_IOstate_get(uint8_t button_idx);



#endif	// header guard
