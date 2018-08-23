/*==============================================================================
* Edit History
* 
* This section contains comments describing changes made to the module. Notice
* that changes are listed in reverse chronological order. Please use ISO format
* for dates.
* 
* when       who       what, where, why
* ---------- ---       -----------------------------------------------------------
* 2016-04-12 bh        Add license information and revision information
* 2016-04-07 bh        Initial revision.
==============================================================================*/

#ifndef __pah_comm_h__
#define __pah_comm_h__


#include "pah_platform_types.h"

/*****************************************
 * Global Function Declaration
 *****************************************/
/**
 * @brief This API write data from I2C module.
 *
 * This API provides an interface to write data to the I2C device. 
 * 
 * @return Return ture is success. (Need be return ture.)
 */
bool  pah_comm_write(uint8_t addr, uint8_t data);

/*****************************************
 * Global Function Declaration
 *****************************************/
/**
 * @brief This API read data from I2C module.
 *
 * This API provides an interface to read data to the I2C device. 
 * 
 * @return Return ture is success. (Need be return ture.)
 */
bool  pah_comm_read(uint8_t addr, uint8_t *data);

/*****************************************
 * Global Function Declaration
 *****************************************/
/**
 * @briefThis API reads data from I2C module.
 *
 * This API provides an interface to reads data to the I2C device. 
 * Read data maximum 255.
 *
 * @return Return ture is success. (Need be return ture.)
 */
bool  pah_comm_burst_read(uint8_t addr, uint8_t *data, uint8_t num);



/**
 * @brief Macro to be used in a formatted string to a pass float number to the log.
 *
 * Macro should be used in formatted string instead of the %f specifier together with
 * @ref LOG_FLOAT macro.
 * Example: LOG_INFO("My float number" LOG_FLOAT_MARKER "\r\n", LOG_FLOAT(f)))
 */
#define LOG_FLOAT_MARKER "%d.%02d"

/**
 * @brief Macro for dissecting a float number into two numbers (integer and residuum).
 */
#define LOG_FLOAT(val) (int32_t)(val),                                     \
                       (int32_t)(((val > 0) ? (val) - (int32_t)(val)       \
                       : (int32_t)(val) - (val))*100)


#if 1
#define debug_printf(...) 
#else
#define debug_printf(...) do {} while (0)
#endif

#if 1
#define log_printf(...) 
#else
#define log_printf(...) do {} while (0)
#endif



#endif  // header guard
