/*==============================================================================
* Edit History
* 
* This section contains comments describing changes made to the module. Notice
* that changes are listed in reverse chronological order. Please use ISO format
* for dates.
* 
* when       who       what, where, why
* ---------- ---       -----------------------------------------------------------
* 2016-09-05 bh        Add checking which channels are enabled.
* 2016-07-07 bh        Initial revision.
==============================================================================*/

#include "pah_driver_8011_reg_array.h"
#include "pah8series_config.h"
// pah
#if defined (ENABLE_PXI_LINK_HRD_REG) || defined (ENABLE_PXI_LINK_HRV_REG) || defined( ENABLE_PXI_LINK_RR_REG)	
#include "pah_driver_8011_reg_HR.h"
#endif
#ifdef ENABLE_PXI_LINK_VP_REG	
#include "pah_driver_8011_reg_VP.h"
#endif

#include "pah_comm.h"

// platform support
#include "pah_platform_functions.h"


/*============================================================================
EXPRESSION MACRO DEFINITIONS
============================================================================*/
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr)         (sizeof(arr) / sizeof(arr[0]))
#endif
#define PAH_CHECK_BIT(var,pos)  ((var >> pos) & 1)

/*============================================================================
TYPE DEFINITIONS
============================================================================*/
typedef const uint8_t(*register_array_t)[2];


typedef struct {

    pah8011_reg_array_mode_e    reg_array_mode;
    uint8_t                     alg_work_mode ;
    register_array_t            init_register_array;
    uint32_t                    init_register_array_length;
    register_array_t            touch_register_array;
    register_array_t            ppg_register_array;
    uint32_t                    mode_register_array_length;
    uint32_t                    reg_version;
    uint32_t                    reg_version_VP;	
    // properties
    uint8_t                     ppg_ch_enabled[pah8011_ppg_ch_num];
	uint8_t                     fifo_ch_enabled[pah8011_ppg_ch_num];
} pah8011_reg_array_state_s;

/*============================================================================
PRIVATE GLOBAL VARIABLES
============================================================================*/
static pah8011_reg_array_state_s    g_state;


/*============================================================================
PRIVATE FUNCTION PROTOTYPES
============================================================================*/
static bool _pah8011_set_reg_array(const uint8_t reg_array[][2], uint32_t length);


/*============================================================================
PUBLIC FUNCTION DEFINITIONS
============================================================================*/
bool pah8011_reg_array_init(uint8_t mode)
{
    uint32_t i = 0;

    memset(&g_state, 0, sizeof(g_state));


#ifdef ENABLE_PXI_LINK_VP_REG
    if (mode == pah8011_alg_VP  )
		{
        g_state.alg_work_mode = mode;
		    g_state.init_register_array = pah8011_init_register_array_VP;
        g_state.touch_register_array = pah8011_touch_register_array_VP;
        g_state.ppg_register_array = pah8011_ppg_register_array_VP;
		    g_state.init_register_array_length = ARRAY_SIZE(pah8011_init_register_array_VP);
        g_state.mode_register_array_length = ARRAY_SIZE(pah8011_touch_register_array_VP);
		    g_state.reg_version_VP = PAH_DRIVER_8011_REG_VERSION_VP;
    }
    else 
#endif	
#if defined (ENABLE_PXI_LINK_HRD_REG) || defined (ENABLE_PXI_LINK_HRV_REG) || defined( ENABLE_PXI_LINK_RR_REG)	
	if ((mode == pah8011_alg_HR  ) || (mode == pah8011_alg_HRV  ) || (mode == pah8011_alg_RR) )
    {
		    g_state.alg_work_mode = mode;
        g_state.init_register_array = pah8011_init_register_array;
        g_state.touch_register_array = pah8011_touch_register_array;
        g_state.ppg_register_array = pah8011_ppg_20hz_register_array;
        g_state.init_register_array_length = ARRAY_SIZE(pah8011_init_register_array);
		    g_state.mode_register_array_length = ARRAY_SIZE(pah8011_touch_register_array);
		    g_state.reg_version = PAH_DRIVER_8011_REG_VERSION_HR;
    }
#endif	
	
    // set default ppg ch enabled
    g_state.ppg_ch_enabled[pah8011_ppg_ch_a] = false;
    g_state.ppg_ch_enabled[pah8011_ppg_ch_b] = true;
    g_state.ppg_ch_enabled[pah8011_ppg_ch_c] = true;

    // check which ppg ch enabled from reg array
    for (i = 0; i < g_state.init_register_array_length; ++i)
    {
        if (g_state.init_register_array[i][0] == 0x21)
        {
			      g_state.ppg_ch_enabled[pah8011_ppg_ch_a] = PAH_CHECK_BIT(g_state.init_register_array[i][1], 0);
			      g_state.fifo_ch_enabled[pah8011_ppg_ch_a] = PAH_CHECK_BIT(g_state.init_register_array[i][1], 3)&PAH_CHECK_BIT(g_state.init_register_array[i][1], 0);
			      debug_printf("ppg_ch_enabled[pah8011_ppg_ch_a] = %d  fifo_ch_enabled[pah8011_ppg_ch_a] = %d \n",g_state.ppg_ch_enabled[pah8011_ppg_ch_a],g_state.fifo_ch_enabled[pah8011_ppg_ch_a]);
        }else if (g_state.init_register_array[i][0] == 0x23)
        {
			      g_state.ppg_ch_enabled[pah8011_ppg_ch_b] = PAH_CHECK_BIT(g_state.init_register_array[i][1], 0);
			      g_state.fifo_ch_enabled[pah8011_ppg_ch_b] = PAH_CHECK_BIT(g_state.init_register_array[i][1], 3)&PAH_CHECK_BIT(g_state.init_register_array[i][1], 0);
			      debug_printf("ppg_ch_enabled[pah8011_ppg_ch_b] = %d  fifo_ch_enabled[pah8011_ppg_ch_b] = %d \n",g_state.ppg_ch_enabled[pah8011_ppg_ch_b],g_state.fifo_ch_enabled[pah8011_ppg_ch_b]);
        }else if (g_state.init_register_array[i][0] == 0x25)
        {    
			      g_state.ppg_ch_enabled[pah8011_ppg_ch_c] = PAH_CHECK_BIT(g_state.init_register_array[i][1], 0);
			      g_state.fifo_ch_enabled[pah8011_ppg_ch_c] = PAH_CHECK_BIT(g_state.init_register_array[i][1], 3)&PAH_CHECK_BIT(g_state.init_register_array[i][1], 0);
			      debug_printf("ppg_ch_enabled[pah8011_ppg_ch_c] = %d  fifo_ch_enabled[pah8011_ppg_ch_c] = %d \n",g_state.ppg_ch_enabled[pah8011_ppg_ch_c],g_state.fifo_ch_enabled[pah8011_ppg_ch_c]);
		    }
    }
	
	return true;
}



bool pah8011_reg_array_load_init(void)
{	
#if defined (ENABLE_PXI_LINK_HRD_REG) || defined (ENABLE_PXI_LINK_HRV_REG) || defined( ENABLE_PXI_LINK_RR_REG)	
    debug_printf("pah8011_reg_array_load_init(), reg[v%d] \n", g_state.reg_version);
#endif	
#ifdef ENABLE_PXI_LINK_VP_REG
    debug_printf("pah8011_reg_array_load_init_for_VP(), reg[v%d] \n", g_state.reg_version_VP);
#endif
	
    g_state.reg_array_mode = pah8011_reg_array_mode_none;

    if (!_pah8011_set_reg_array(g_state.init_register_array, g_state.init_register_array_length))
        return false;

    return true;
}

bool pah8011_reg_array_load_mode(pah8011_reg_array_mode_e reg_array_mode)
{
    if (g_state.reg_array_mode != reg_array_mode)
    {
        switch (reg_array_mode)
        {
        case pah8011_reg_array_mode_touch:
            debug_printf("pah8011_reg_array_load_reg_setting(). g_state.touch_register_array \n");
            if (!_pah8011_set_reg_array(g_state.touch_register_array, g_state.mode_register_array_length))
                goto FAIL;
            break;

        case pah8011_reg_array_mode_ppg:
            debug_printf("pah8011_reg_array_load_reg_setting(). g_state.ppg_register_array \n");
            if (!_pah8011_set_reg_array(g_state.ppg_register_array, g_state.mode_register_array_length))
                goto FAIL;
            break;

        default:
            debug_printf("pah8011_reg_array_load_reg_setting(). not implemented, reg_array_mode = %d \n", reg_array_mode);
            goto FAIL;
        }

        g_state.reg_array_mode = reg_array_mode;
    }

    return true;

FAIL:
    debug_printf("_pah8011_init fail \n");
    return false;
}

void pah8011_get_ppg_ch_enabled(uint8_t ppg_ch_enabled[pah8011_ppg_ch_num])
{
    memcpy(ppg_ch_enabled, g_state.ppg_ch_enabled, sizeof(g_state.ppg_ch_enabled));
}

uint32_t pah8011_get_ppg_ch_num(void)
{
    uint32_t ch_num = 0;

    if (g_state.fifo_ch_enabled[pah8011_ppg_ch_a])
        ++ch_num;
    if (g_state.fifo_ch_enabled[pah8011_ppg_ch_b])
        ++ch_num;
    if (g_state.fifo_ch_enabled[pah8011_ppg_ch_c])
        ++ch_num;

    return ch_num;
}

uint8_t pah8011_get_setting_version(void)
{
    return SETTING_VERSION;
}

#ifdef ENABLE_PXI_LINK_VP_REG	
uint8_t pah8011_get_setting_version_VP(void)
{
    return SETTING_VERSION_VP;
}
#endif
/*============================================================================
PRIVATE FUNCTION DEFINITIONS
============================================================================*/
static bool _pah8011_set_reg_array(const uint8_t reg_array[][2], uint32_t length)
{
    uint32_t i = 0;

    for (i = 0; i < length; ++i)
    {
        if (!pah_comm_write(reg_array[i][0], reg_array[i][1]))
        {
            debug_printf("_pah8011_set_reg_array(). pah_comm_write fail, i = %d \n", i);
            return false;
        }
    }
    return true;
}
