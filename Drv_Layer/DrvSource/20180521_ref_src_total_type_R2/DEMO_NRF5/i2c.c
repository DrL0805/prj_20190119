#include "i2c.h"

#include "../config.h"
#include <stdbool.h>
#include <string.h>
#include "nrf_drv_twi.h"
#include "app_util_platform.h"
#include "app_error.h"

/*============================================================================
SOME DEFINITION
============================================================================*/
//#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
	
/*============================================================================
STATIC VARIABLE DEFINITIONS
============================================================================*/

/*
static bool _i2c_transmit(uint8_t *data, uint16_t tx_size);
static bool _i2c_receive(uint8_t *data, uint16_t rx_size);
*/
/**
 * @brief TWI master instance
 *
 * Instance of TWI master driver that would be used for communication with simulated
 * eeprom memory.
 */
static const nrf_drv_twi_t m_twi_master = NRF_DRV_TWI_INSTANCE(MASTER_TWI_INST);
 static ret_code_t twi_master_init(void)
{
    ret_code_t ret;
    const nrf_drv_twi_config_t config =
    {
       .scl                = TWI_SCL_M,
       .sda                = TWI_SDA_M,
       .frequency          = NRF_TWI_FREQ_400K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = false
    };

    ret = nrf_drv_twi_init(&m_twi_master, &config, NULL, NULL);

    if (NRF_SUCCESS == ret)
    {
        nrf_drv_twi_enable(&m_twi_master);
    }

    return ret;
}


bool	i2c_init(void)
{
		uint32_t err_code = NRF_SUCCESS;
		/* Initializing TWI master interface for Sensor*/
	  err_code = twi_master_init();
    APP_ERROR_CHECK(err_code);
		return true;
}

uint8_t pah8011_i2c_write(size_t addr, uint8_t const * pdata, size_t size)
{
    ret_code_t ret;
    /* Memory device supports only limited number of bytes written in sequence */
    if(size > (EEPROM_SIM_SEQ_WRITE_MAX))
    {
        return NRF_ERROR_INVALID_LENGTH;
    }
    /* All written data has to be in the same page */
    if((addr/(EEPROM_SIM_SEQ_WRITE_MAX)) != ((addr+size-1)/(EEPROM_SIM_SEQ_WRITE_MAX)))
    {
        return NRF_ERROR_INVALID_ADDR;
    }
    do
    {
        uint8_t buffer[1 + EEPROM_SIM_SEQ_WRITE_MAX]; /* Addr + data */
        buffer[0] = (uint8_t)addr;
        memcpy(buffer+1, pdata, size);
        ret = nrf_drv_twi_tx(&m_twi_master, PAH8011_ADDR, buffer, size+1, false);
    }while(0);
    return ret;
}

uint8_t pah8011_i2c_read (uint8_t addr, uint8_t * pdata, uint8_t size)
{
    ret_code_t ret;
    if(size > (EEPROM_SIM_SIZE))
    {
        return NRF_ERROR_INVALID_LENGTH;
    }
    do
    {
       uint8_t addr8 = (uint8_t)addr;
       ret = nrf_drv_twi_tx(&m_twi_master, PAH8011_ADDR, &addr8, 1, true);
       if(NRF_SUCCESS != ret)
       {
           break;
       }
       ret = nrf_drv_twi_rx(&m_twi_master, PAH8011_ADDR, pdata, size);
    }while(0);
    return ret;
}





