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

// pah
#include "pah_driver_8011_reg.h"
#include "pah_comm.h"

// platform support
#include "pah_platform_functions.h"


/*============================================================================
EXPRESSION MACRO DEFINITIONS
============================================================================*/
#define ARRAY_SIZE(arr)         (sizeof(arr) / sizeof(arr[0]))

#define PAH_CHECK_BIT(var,pos)  ((var >> pos) & 1)

/*============================================================================
TYPE DEFINITIONS
============================================================================*/
typedef struct {

    pah8011_reg_array_mode_e    reg_array_mode;

    // properties
    uint8                     ppg_ch_enabled[pah8011_ppg_ch_num];

} pah8011_reg_array_state_s;

/*============================================================================
PRIVATE GLOBAL VARIABLES
============================================================================*/
static pah8011_reg_array_state_s    g_state;


/*============================================================================
PRIVATE FUNCTION PROTOTYPES
============================================================================*/
static bool             _pah8011_set_reg_array(const uint8 reg_array[][2], uint32 length);


/*============================================================================
PUBLIC FUNCTION DEFINITIONS
============================================================================*/
bool pah8011_reg_array_init(void)
{
    uint32 i = 0;

    memset(&g_state, 0, sizeof(g_state));

    // set default ppg ch enabled
    g_state.ppg_ch_enabled[pah8011_ppg_ch_a] = false;
    g_state.ppg_ch_enabled[pah8011_ppg_ch_b] = true;
    g_state.ppg_ch_enabled[pah8011_ppg_ch_c] = true;

    // check which ppg ch enabled from reg array
    for (i = 0; i < ARRAY_SIZE(pah8011_init_register_array); ++i)
    {
        if (pah8011_init_register_array[i][0] == 0x21)
            g_state.ppg_ch_enabled[pah8011_ppg_ch_a] = PAH_CHECK_BIT(pah8011_init_register_array[i][1], 0);
        else if (pah8011_init_register_array[i][0] == 0x23)
            g_state.ppg_ch_enabled[pah8011_ppg_ch_b] = PAH_CHECK_BIT(pah8011_init_register_array[i][1], 0);
        else if (pah8011_init_register_array[i][0] == 0x25)
            g_state.ppg_ch_enabled[pah8011_ppg_ch_c] = PAH_CHECK_BIT(pah8011_init_register_array[i][1], 0);
    }

    return true;
}

bool pah8011_reg_array_load_init(void)
{
//    debug_printf_1("pah8011_reg_array_load_init(), reg[v%d] \n", PAH_DRIVER_8011_REG_VERSION);

    g_state.reg_array_mode = pah8011_reg_array_mode_none;

    if (!_pah8011_set_reg_array(pah8011_init_register_array, ARRAY_SIZE(pah8011_init_register_array)))
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
//            debug_printf("pah8011_reg_array_load_reg_setting(). pah8011_touch_register_array \n");
            if (!_pah8011_set_reg_array(pah8011_touch_register_array, ARRAY_SIZE(pah8011_touch_register_array)))
                goto FAIL;
            break;

        case pah8011_reg_array_mode_ppg_20hz:
//            debug_printf("pah8011_reg_array_load_reg_setting(). pah8011_ppg_20hz_register_array \n");
            if (!_pah8011_set_reg_array(pah8011_ppg_20hz_register_array, ARRAY_SIZE(pah8011_ppg_20hz_register_array)))
                goto FAIL;
            break;

        default:
//            debug_printf_1("pah8011_reg_array_load_reg_setting(). not implemented, reg_array_mode = %d \n", reg_array_mode);
            goto FAIL;
        }

        g_state.reg_array_mode = reg_array_mode;
    }

    return true;

FAIL:
//    debug_printf("_pah8011_init fail \n");
    return false;
}

void pah8011_get_ppg_ch_enabled(uint8 ppg_ch_enabled[pah8011_ppg_ch_num])
{
    memcpy(ppg_ch_enabled, g_state.ppg_ch_enabled, sizeof(g_state.ppg_ch_enabled));
}

uint32 pah8011_get_ppg_ch_num(void)
{
    uint32 ch_num = 0;

    if (g_state.ppg_ch_enabled[pah8011_ppg_ch_a])
        ++ch_num;
    if (g_state.ppg_ch_enabled[pah8011_ppg_ch_b])
        ++ch_num;
    if (g_state.ppg_ch_enabled[pah8011_ppg_ch_c])
        ++ch_num;

    return ch_num;
}

uint8 pah8011_get_setting_version(void)
{
    return SETTING_VERSION;
}

/*============================================================================
PRIVATE FUNCTION DEFINITIONS
============================================================================*/
static bool _pah8011_set_reg_array(const uint8 reg_array[][2], uint32 length)
{
    uint32 i = 0;

    for (i = 0; i < length; ++i)
    {
        if (!pah_comm_write(reg_array[i][0], reg_array[i][1]))
        {
//            debug_printf_1("_pah8011_set_reg_array(). pah_comm_write fail, i = %d \n", i);
            return false;
        }
    }
    return true;
}
