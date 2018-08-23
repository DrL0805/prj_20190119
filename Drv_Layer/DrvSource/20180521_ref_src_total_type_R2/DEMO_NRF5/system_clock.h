
#ifndef __SYSTEM_CLOCK_H__
#define __SYSTEM_CLOCK_H__


/*****************************************
 * Header files including
 *****************************************/
#include <stdbool.h>
#include <stdint.h>

/*****************************************
 * Global Function Declaration
 *****************************************/
uint32_t	get_tick_count(void);
void		delay_ms(uint32_t ms);



#endif	// header guard
