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

#include "pah_comm.h"

// platform support
#include "pah_platform_functions.h"


/*============================================================================
STATIC VARIABLE DEFINITIONS
============================================================================*/

// valid bank range: 0x00 ~ 0x03
static uint8 _curr_bank = 0xFF;


/*============================================================================
PUBLIC FUNCTION DEFINITIONS
============================================================================*/
bool pah_comm_write(uint8 addr, uint8 data)
{
    if (addr == 0x7F)
    {
        if (_curr_bank == data)
            return true;

        SMDrv_SWI2C_Write(HR_IIC_MODULE,0x2A, 0x7F, &data, 1);
        _curr_bank = data;
        return true;
    }
    
    SMDrv_SWI2C_Write(HR_IIC_MODULE,0x2A, addr, &data, 1);
    return true;
}

bool pah_comm_read(uint8 addr, uint8 *data)
{
    SMDrv_SWI2C_Read(HR_IIC_MODULE,0x2A, addr, data, 1);
	return true;
}

bool pah_comm_burst_read(uint8 addr, uint8 *data, uint16 num)
{
    SMDrv_SWI2C_Read(HR_IIC_MODULE,0x2A, addr, data, num);
	return true;
}
