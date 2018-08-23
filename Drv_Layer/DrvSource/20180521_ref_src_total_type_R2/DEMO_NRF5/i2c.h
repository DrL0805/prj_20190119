#ifndef __I2C_H__
#define __I2C_H__

/*****************************************
 * Global Function Declaration
 *****************************************/
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/*****************************************
 * Global Function Declaration
 *****************************************/	
bool 		i2c_init(void);
uint8_t pah8011_i2c_write(size_t addr, uint8_t const * pdata, size_t size);
uint8_t pah8011_i2c_read (uint8_t addr, uint8_t * pdata, uint8_t size);
	
/*
bool		i2c_write_reg(uint8_t addr, uint8_t data);
bool		i2c_read_reg(uint8_t addr, uint8_t *data);
bool		i2c_burst_read_reg(uint8_t addr, uint8_t *data, uint32_t rx_size);
*/

#endif
