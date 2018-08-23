/*==============================================================================
* Edit History
*
* This section contains comments describing changes made to the module. Notice
* that changes are listed in reverse chronological order. Please use ISO format
* for dates.
*
* when        version  who       what, where, why
* ----------  ------   ---       -----------------------------------------------------------
* 2016-09-20   1007    bh        - Add pah_intshape_pulse_type_e flag.
*                                - Call shutdown in init.
* 2016-09-08   1006    bh        - Add functions: pah_init_with_flags().
*                                - Add pah_ppg_led_on_e flag.
*                                - Replace all update flag reg setting with _pah8011_update_flag().
*                                - Remove useless comments.
* 2016-09-05   1005    bh        - Add polling mode.
*                                - Add functions: pah_init_with_flags(), pah_get_fifo_ch_num().
* 2016-07-07   1004    bh        - Move reg array settings to new source files.
* 2016-07-01   1003    bh        - Fix undefined behaviors when the device wakes up from power down mode. Delay is necessary for the internal controller to reset the whole system and then the device is ready for the upcoming operations.
* 2016-06-07   1002    bh        - Add functions: pah_set_mode(), pah_run_device().
*                                - Add enum: pah_device.
* 2016-04-29   1001    bh        - Add PPG 200Hz modes.
*                                - Add helper functions: pah_is_ppg_mode(), pah_is_ppg_20hz_mode(), pah_fifo_data_num_per_ch().
*                                - Add pah_stop_mode
*                                - Remove pah_suspend_mode.
*                                - Fix setting pah_set_report_sample_num_per_ch() after enter_mode() causes bad behavior.
* 2016-04-14   1000    bh        - Add version.
* 2016-04-12           bh        - Add license information and revision information.
*                                - Modify to reduce the reaction time of touch/no-touch detect.
* 2016-04-07           bh        - Initial revision.
==============================================================================*/

#include "pah_driver.h"

// pah
#include "pah_driver_8011_reg_array.h"
#include "pah_8011_internal.h"
#include "pah_comm.h"
#include "pah8series_config.h"
// platform support
#include "pah_platform_functions.h"

#include "system_clock.h"

//#define _INT_PULL_LOW_I2C	
//#define _INT_PULL_LOW_SPI		




/*============================================================================
TYPE DEFINITIONS
============================================================================*/
typedef enum {

    pah_fifo_freq_none,
    pah_fifo_freq_20hz,
	pah_fifo_freq_100hz,
} pah_fifo_freq;


typedef struct {

    uint32_t        report_sample_num_per_ch;
    uint8_t         is_int2_as_touch_flag;

} pah8011_property_s;


typedef struct {

    // fifo data
    uint8_t                 fifo_data[MAX_SAMPLES_PER_READ * BYTES_PER_SAMPLE];

    // update every task
    uint32_t                fifo_data_num_per_ch;

    // flags
    pah_flags_s             flags;

    // report ppg data
    void*                   user_data;
    pah_report_fifo_handle  fp_report_fifo_handler;

    // private members
    uint8_t                 has_started_tg;
    uint8_t                 has_started_ppg;
    uint8_t                 has_started_touch;
    uint8_t                 fifo_data_contain_touch_bit;

    // read only properties
    pah_mode                mode;
    uint8_t                 touch_flag;

    //// flag options
    // pah_ppg_led_on_e
    uint8_t                 is_led_on;

    // properties
    pah8011_property_s      prop_curr;
    pah8011_property_s      prop_next;

} pah8011_state_s;


/*============================================================================
PRIVATE GLOBAL VARIABLES
============================================================================*/
static bool               g_init = false;
static pah8011_state_s    g_state;
#if defined(Touch_Detect_Green_Led)
static uint16_t Touch_Th_Hi_Value = 1024 ;
static uint16_t Touch_Th_Lo_Value = 1024 ;	
#endif	
/*============================================================================
PRIVATE FUNCTION PROTOTYPES
============================================================================*/
//// pure functions
static bool             _pah_is_ppg_mode(pah_mode mode);
static pah_fifo_freq    _pah_mode_to_freq(pah_mode mode);
static bool             _pah_is_diff_fifo_freq(pah_mode lhs_mode, pah_mode rhs_mode);

//// pah8011
static bool             _pah8011_init(void);

static bool             _pah8011_update_flag(void);

static bool             _pah8011_shutdown(void);
static bool             _pah8011_startup(void);

static bool             _pah8011_start_tg(void);

static bool             _pah8011_start_ppg(void);
static bool             _pah8011_stop_ppg(void);
static bool             _pah8011_start_touch(void);
static bool             _pah8011_stop_touch(void);

static bool             _pah8011_clear_fifo_and_int_req(void);
static bool             _pah8011_update_report_num(void);
static bool             _pah8011_set_report_num(uint32_t samples_per_read, uint32_t ch_num);
static bool             _pah8011_turn_led(bool on);

#if defined(ENABLE_FIFO_CHECKSUM)
static bool             _pah8011_cks(uint8_t *fifo, uint32_t samples_read, uint32_t cks_read);
#endif // ENABLE_FIFO_CHECKSUM

static bool             _pah8011_read_touch_flag(uint8_t *touch_flag);

static pah_ret          _pah8011_task_dri(uint8_t int_req, uint32_t ch_num);
static pah_ret          _pah8011_task_polling(uint8_t int_req, uint32_t ch_num);


/*============================================================================
PUBLIC FUNCTION DEFINITIONS
============================================================================*/
void pah_flags_default(pah_flags_s *flags)
{
    if (!flags)
        return;
    
    memset(flags, 0, sizeof(*flags));
}

bool pah_init(void)
{
    pah_flags_s flags;

    flags.stream = pah_stream_default;

    return pah_init_with_flags(&flags);
}

bool pah_init_with_flags(const pah_flags_s *flags)
{
    pah_ret ret = pah_err_unknown;
    
    debug_printf(">>>> pah_init() \n");

    if (!flags)
    {
        ret = pah_err_invalid_argument;
        goto FAIL;
    }

    pah_deinit();

    memset(&g_state, 0, sizeof(g_state));
    g_state.mode = pah_stop_mode;
    memcpy(&g_state.flags, flags, sizeof(*flags));

    // properties
    g_state.prop_curr.report_sample_num_per_ch = 0;
    g_state.prop_next.report_sample_num_per_ch = DEFAULT_REPORT_SAMPLE_NUM_PER_CH;

    ret = pah_8011_verify_product_id();
    if (PAH_FAILED(ret))
    {
        ret = pah_8011_startup();
        if (PAH_FAILED(ret))
            goto FAIL;
        ret = pah_8011_verify_product_id();
        if (PAH_FAILED(ret))
            goto FAIL;
    }
    uint8_t mode;
    // register arrays
    switch (flags->alg_mode)
    {
    case pah_alg_mode_RR:
		    mode = pah8011_alg_RR ;
        break;
    case pah_alg_mode_HRV:
		    mode = pah8011_alg_HRV ;
        break;
    case pah_alg_mode_SPO2:
		    mode = pah8011_alg_SPO2 ;
        break;
    case pah_alg_mode_VP:
		    mode = pah8011_alg_VP ;
        break;
    case pah_alg_mode_HR:
		    mode = pah8011_alg_HR ;
        break;		
    default:
        break;
    }
    if (!pah8011_reg_array_init(mode))
    {
        ret = pah_err_platform_fail;
        goto FAIL;
    }
    
    ret = pah_8011_device_init();
    if (PAH_FAILED(ret))
        goto FAIL;
	// Define pah8011_GPIO_Setting
	pah_comm_write(0x7F, 0x04);    //bank4
	pah_comm_write(0x15, 0x69);
	pah_comm_write(0x34, 0x01);
	if(((pah8011_get_setting_version()&0x30)==0x30)||((pah8011_get_setting_version()&0x50)==0x50))
	{
		//SPI Interface
		pah_comm_write(0x2B, 0xFF);
		pah_comm_write(0x70, 0x08);	
	}else
	{	
		//I2C Interface
		pah_comm_write(0x2B, 0xFE);
		pah_comm_write(0x70, 0x18);	
	}	
    _pah8011_shutdown();

    g_init = true;

    debug_printf("<<<< pah_init() \n");
    return true;

FAIL:
    debug_printf("pah_init() faied. ret = %d \n", ret);
    return false;
}

void pah_deinit(void)
{
    if (!g_init)
        return;

    debug_printf(">>>> pah_deinit() \n");

    _pah8011_shutdown();

    g_init = false;

    debug_printf("<<<< pah_deinit() \n");
}

bool pah_enter_mode(pah_mode mode)
{
    if (!pah_set_mode(mode))
        goto FAIL;

    switch (mode)
    {
    case pah_touch_mode:
        if (!pah_run_device(pah_device_touch, true))
            goto FAIL;
        break;

    case pah_ppg_mode:
         if (!pah_run_device(pah_device_ppg, true))
            goto FAIL;
        break;

    case pah_ppg_touch_mode:
         if (!pah_run_device(pah_device_ppg, true))
            goto FAIL;
        if (!pah_run_device(pah_device_touch, true))
            goto FAIL;
        break;

    default:
        break;
    }

    return true;

FAIL:
    debug_printf("pah_enter_mode(). fail \n");
    return false;
}

bool pah_set_mode(pah_mode mode)
{
    debug_printf(">>>> pah_set_mode(), mode = %d -> %d \n", g_state.mode, mode);

    if (g_state.mode == mode)
    {
        debug_printf("<<<< pah_set_mode() \n");
        return true;
    }

    if (g_state.mode == pah_stop_mode)
    {
        if (!_pah8011_startup())
            goto FAIL;
        if (!_pah8011_init())
            goto FAIL;

        // update properties
        if (!_pah8011_update_report_num())
            goto FAIL;
        g_state.prop_curr.is_int2_as_touch_flag = g_state.prop_next.is_int2_as_touch_flag;
    }

    if (pah_stop_mode == mode)
    {
        if (!_pah8011_shutdown())
            goto FAIL;
    }
    else
    {
        switch (mode)
        {
        case pah_touch_mode:
        {
            _pah8011_stop_ppg();

            if (_pah_is_ppg_mode(g_state.mode))
            {
                if (!_pah8011_clear_fifo_and_int_req())
                    goto FAIL;
            }

            if (!pah8011_reg_array_load_mode(pah8011_reg_array_mode_touch))
                goto FAIL;
        }
        break;

        case pah_ppg_mode:
        {
            _pah8011_stop_touch();

            if (_pah_is_diff_fifo_freq(g_state.mode, mode))
            {
                if (!_pah8011_stop_ppg())
                    goto FAIL;
                if (!_pah8011_clear_fifo_and_int_req())
                    goto FAIL;
            }

            if (!pah8011_reg_array_load_mode(pah8011_reg_array_mode_ppg))
                goto FAIL;
            if (!_pah8011_update_report_num())
                goto FAIL;
        }
        break;



        case pah_ppg_touch_mode:
        {
            if (_pah_is_diff_fifo_freq(g_state.mode, mode))
            {
                if (!_pah8011_stop_ppg())
                    goto FAIL;
                if (!_pah8011_clear_fifo_and_int_req())
                    goto FAIL;
            }

            if (!pah8011_reg_array_load_mode(pah8011_reg_array_mode_ppg))
                goto FAIL;
            if (!_pah8011_update_report_num())
                goto FAIL;
        }
        break;


        default:
            goto FAIL;
        }

        // pah_ppg_led_on_e flag
        if (g_state.flags.ppg_led_on == pah_ppg_led_on_deferred)
        {
            if (!_pah_is_ppg_mode(g_state.mode) && _pah_is_ppg_mode(mode))
            {
                if (!_pah8011_turn_led(false))
                    goto FAIL;
                g_state.is_led_on = false;
            }
        }
        
        if (!_pah8011_update_flag())
            goto FAIL;
    }

    g_state.mode = mode;

    debug_printf("<<<< pah_set_mode() \n");
    return true;

FAIL:
    debug_printf("pah_set_mode(). fail \n");
    return false;
}

bool pah_run_device(pah_device device, bool enable)
{
    debug_printf(">>>> pah_run_device(), device = %d, enable = %d \n", device, enable);

    switch (device)
    {
    case pah_device_ppg:
        if (enable)
        {
            if (!_pah8011_update_report_num())
                goto FAIL;
            if (!_pah8011_start_ppg())
                goto FAIL;
        }
        else
        {
            if (!_pah8011_stop_ppg())
                goto FAIL;
        }
        break;

    case pah_device_touch:
        if (enable)
        {
            if (!_pah8011_start_touch())
                goto FAIL;
        }
        else
        {
            if (!_pah8011_stop_touch())
                goto FAIL;
        }
        break;

    default:
        goto FAIL;
    }

    if (!_pah8011_update_flag())
        goto FAIL;

    if (enable)
    {
        if (!_pah8011_start_tg())
            goto FAIL;
    }

    debug_printf("<<<< pah_run_device() \n");
    return true;

FAIL:
    debug_printf("pah_run_device(). fail \n");
    return false;
}

pah_mode pah_query_mode(void)
{
    return g_state.mode;
}

bool pah_is_ppg_mode(void)
{
    return _pah_is_ppg_mode(g_state.mode);
}


pah_ret pah_task(void)
{
    pah_ret ret = pah_err_unknown;
    uint8_t int_req = 0;

    debug_printf(">>>> pah_task() \n");

    // reset
    g_state.fifo_data_num_per_ch = 0;

    // read interrupt
    if (!pah_comm_write(0x7F, 0x02))    //Bank 2
    {
        ret = pah_err_comm_fail;
        goto FAIL;
    } 
	if (!pah_comm_write(0x7C, 0x00))    
    {
        ret = pah_err_comm_fail;
        goto FAIL;
    }
    if (!pah_comm_read(0x1B, &int_req))    //read interrupt status
    {
        ret = pah_err_comm_fail;
        goto FAIL;
    }
    
    debug_printf("pah_task(). int_req = %d \n", int_req);

    // fifo overflow
    if (int_req & 0x04)
    {
        // Usually happens when pah_task() was too late to be called.
        // Troubleshooting: Record timestamps before every pah_task() calls, the interval of 
        // different pah_task() calls should be regular.

        debug_printf("pah_task(). fifo overflow \n");
        ret = pah_err_fifo_overflow;
        goto FAIL;
    }

    // underflow interrupt
    if (int_req & 0x08)
    {
        debug_printf("pah_task(). fifo underflow \n");
        ret = pah_err_fifo_underflow;
        goto FAIL;
    }

    // task
    {
        uint32_t ch_num = pah_get_fifo_ch_num();

        if (!ch_num)
        {
            debug_printf("pah_task(). ch_num should never be 0 \n");
            ret = pah_err_invalid_program;
            goto FAIL;
        }

        if (g_state.flags.stream == pah_stream_dri)
            ret = _pah8011_task_dri(int_req, ch_num);
        else // if (g_state.stream_mode == pah_stream_polling)
            ret = _pah8011_task_polling(int_req, ch_num);
    }
	if((g_state.touch_flag)&&(ret==pah_pending))
			ret = pah_success;
    if (ret != pah_success && ret != pah_pending)
        goto FAIL;
	//if((int_req==0x02)&&(ret==pah_pending))
	//			ret = pah_success;
    if (int_req)
    {
        // clear interrupt
        if (!pah_comm_write(0x7F, 0x02))    //Bank 2
        {
            ret = pah_err_comm_fail;
            goto FAIL;
        }
        if (!pah_comm_write(0x1b, int_req)) //clear interrupt
        {
            ret = pah_err_comm_fail;
            goto FAIL;
        }
        
        //debug_printf("pah_task(). clear interrupt, int_req = %d \n", int_req);
    }

    // update properties
    if (!_pah8011_update_report_num())
    {
        ret = pah_err_comm_fail;
        goto FAIL;
    }
    
    // pah_ppg_led_on_e flag
    if (g_state.flags.ppg_led_on == pah_ppg_led_on_deferred && !g_state.is_led_on)
    {
        if (g_state.touch_flag && pah_is_ppg_mode())
        {
            if (!_pah8011_turn_led(true))
                goto FAIL;
            if (!_pah8011_update_flag())
                goto FAIL;
            g_state.is_led_on = true;
        }
    }

    debug_printf("<<<< pah_task() \n");
    return ret;

FAIL:
    debug_printf("pah_task(). fail, ret = %d \n", ret);
    return ret;
}

uint8_t* pah_get_fifo_data(void)
{
    return g_state.fifo_data;
}

uint32_t pah_fifo_data_num_per_ch(void)
{
    return g_state.fifo_data_num_per_ch;
}

bool pah_has_fifo_data(void)
{
    return g_state.fifo_data_num_per_ch > 0;
}

uint32_t pah_get_fifo_ch_num(void)
{
    return pah8011_get_ppg_ch_num();
}

bool pah_is_touched(void)
{
    return g_state.touch_flag > 0;
}

void pah_clear_fifo_data(void)
{
    memset(g_state.fifo_data, 0, sizeof(g_state.fifo_data));
}

void pah_set_report_fifo_callback(pah_report_fifo_handle fp_handler, void* user_data)
{
    debug_printf("==== pah_set_report_fifo_callback() \n");

    g_state.fp_report_fifo_handler = fp_handler;
    g_state.user_data = user_data;
}

uint16_t pah_get_i2c_slave_addr(void)
{
    debug_printf("==== pah_get_i2c_slave_addr() \n");

    return I2C_SLAVE_ID;
}

bool pah_set_int2_as_touch_flag(bool enable)
{
    debug_printf("==== pah_set_int2_as_touch_flag() \n");
    debug_printf("pah_set_int2_as_touch_flag(). enable = %d \n", enable);

    g_state.prop_next.is_int2_as_touch_flag = enable;
    return true;
}

void pah_set_report_sample_num_per_ch(uint32_t report_sample_num_per_ch)
{
    static const uint32_t MIN_REPORT_SAMPLE_NUM_PER_CH = 1;
    const uint32_t MAX_REPORT_SAMPLE_NUM_PER_CH = pah_get_max_report_sample_num_per_ch();

    if (report_sample_num_per_ch < MIN_REPORT_SAMPLE_NUM_PER_CH)
        report_sample_num_per_ch = MIN_REPORT_SAMPLE_NUM_PER_CH;
    else if (report_sample_num_per_ch > MAX_REPORT_SAMPLE_NUM_PER_CH)
        report_sample_num_per_ch = MAX_REPORT_SAMPLE_NUM_PER_CH;
    g_state.prop_next.report_sample_num_per_ch = report_sample_num_per_ch;

    debug_printf("pah_set_report_sample_num_per_ch(). report_sample_num_per_ch = %d \n", report_sample_num_per_ch);
}
uint32_t pah_get_report_sample_num_per_ch(void)
{
    return g_state.prop_curr.report_sample_num_per_ch;
}
uint32_t pah_get_max_report_sample_num_per_ch(void)
{
    return MAX_SAMPLES_PER_READ / MAX_CH_NUM;
}

uint32_t pah_get_bytes_per_sample(void)
{
    return BYTES_PER_SAMPLE;
}

bool pah_verify_product_id(void)
{
    uint8_t data = 0;

    debug_printf(">>>> pah_verify_product_id() \n");

    if (!pah_comm_write(0x7F, 0x00))    //bank0
        goto FAIL;
    if (!pah_comm_read(0x00, &data))
        goto FAIL;
    if (data != 0x11)
    {
        debug_printf("pah_verify_product_id() fail. data = %d \n", data);
        goto FAIL;
    }

    debug_printf("<<<< pah_verify_product_id() \n");
    return true;

FAIL:
    debug_printf("pah_verify_product_id error \n");
    return false;
}

pah_ret pah_read_touch_flag(bool *is_touched)
{
    uint8_t touch_flag = 0;

    debug_printf(">>>> pah_read_touch_flag() \n");
	  
    if (!is_touched)
    {
        debug_printf("pah_read_touch_flag() fail. invalid argument \n");
        return pah_err_invalid_argument;
    }

    if (!_pah8011_read_touch_flag(&touch_flag))
        return pah_err_comm_fail;

    *is_touched = touch_flag > 0;

    debug_printf("<<<< pah_read_touch_flag(). is_touched = %d \n", *is_touched);
    return pah_success;
}

/*============================================================================
Pure functions
============================================================================*/
static bool _pah_is_ppg_mode(pah_mode mode)
{
    return mode == pah_ppg_mode
        || mode == pah_ppg_touch_mode;
}

static pah_fifo_freq _pah_mode_to_freq(pah_mode mode)
{
    pah_fifo_freq result = pah_fifo_freq_none;
    switch (mode)
    {
    case pah_ppg_mode:
    case pah_ppg_touch_mode:
		if(g_state.flags.alg_mode == pah_alg_mode_VP){
			result = pah_fifo_freq_100hz;
    }else 
		if(g_state.flags.alg_mode == pah_alg_mode_HR){	
			result = pah_fifo_freq_20hz;
		}
        break;


    default:
        result = pah_fifo_freq_none;
        break;
    }
    return result;
}

static bool _pah_is_diff_fifo_freq(pah_mode lhs_mode, pah_mode rhs_mode)
{
    pah_fifo_freq lhs_freq = _pah_mode_to_freq(lhs_mode);
    pah_fifo_freq rhs_freq = _pah_mode_to_freq(rhs_mode);

    if (lhs_freq == pah_fifo_freq_none
        || rhs_freq == pah_fifo_freq_none)
        return false;

    return lhs_freq != rhs_freq;
}


/*============================================================================
PRIVATE FUNCTION DEFINITIONS
============================================================================*/
static bool _pah8011_init(void)
{
    uint8_t data = 0;

    debug_printf("_pah8011_init(), PAH8011 driver[v%d] \n", PAH_DRIVER_8011_VERSION);

    //// reg init settings
    pah8011_reg_array_load_init();

	if (!_pah8011_update_flag())
        goto FAIL;
    //// other init settings
    if (!pah_comm_write(0x7F, 0x01))    //bank1
        goto FAIL;

    // disable ch a
    if (!pah_comm_read(0x21, &data))
        goto FAIL;
    PAH_CLEAR_BIT(data, 0);
    if (!pah_comm_write(0x21, data))
        goto FAIL;

    // disable ch b
    if (!pah_comm_read(0x23, &data))
        goto FAIL;
    PAH_CLEAR_BIT(data, 0);
    if (!pah_comm_write(0x23, data))
        goto FAIL;

    // disable ch c
    if (!pah_comm_read(0x25, &data))
        goto FAIL;
    PAH_CLEAR_BIT(data, 0);
    if (!pah_comm_write(0x25, data))
        goto FAIL;

    // touch interrupt
    if (!pah_comm_read(0x36, &data))
        goto FAIL;
    PAH_SET_BIT(data, 0);
    PAH_CLEAR_BIT(data, 1);
    if (!pah_comm_write(0x36, data))
        goto FAIL;

    // mask
    if (!pah_comm_read(0x37, &data))
        goto FAIL;
    PAH_SET_BIT(data, 1);
    if (!pah_comm_write(0x37, data))
        goto FAIL;

    // touch bit
    if (!pah_comm_read(0x5A, &data))
        goto FAIL;
    g_state.fifo_data_contain_touch_bit = PAH_CHECK_BIT(data, 0);
    
    // intshape_pulse_type
    if (g_state.flags.intshape_pulse_type == pah_intshape_pulse_type_level)
    {
        if (!pah_comm_write(0x7F, 0x01))    //Bank 1
            goto FAIL;
        if (!pah_comm_read(0x35, &data))
            goto FAIL;
        PAH_CLEAR_BIT(data, 1);
        if (!pah_comm_write(0x35, data))
            goto FAIL;
    }
#if defined(Touch_Detect_Green_Led)		
	pah_comm_write(0x7f, 0x01);
	pah_comm_write(0x0D, 0x04);
	pah_comm_write(0x0E, 0x0F);
	pah_comm_write(0x14, 0x00);
	pah_comm_write(0x04, (Touch_Th_Hi_Value&0xff));
	pah_comm_write(0x05, (Touch_Th_Hi_Value>>8)  );
	pah_comm_write(0x07, (Touch_Th_Lo_Value&0xff));
	pah_comm_write(0x08, (Touch_Th_Lo_Value>>8)  );
	_pah8011_update_flag();
#endif	
	
    
    if (!_pah8011_update_flag())
        goto FAIL;

    return true;

FAIL:
    debug_printf("_pah8011_init fail \n");
    return false;
}

static bool _pah8011_update_flag(void)
{
    // update flag
    if (!pah_comm_write(0x7F, 0x01))    //bank1
        goto FAIL;
    if (!pah_comm_write(0x24, 0x01))
        goto FAIL;
	delay_ms(2);
    return true;

FAIL:
    debug_printf("_pah8011_update_flag fail \n");
    return false;
}

static bool _pah8011_shutdown(void)
{
    debug_printf("_pah8011_shutdown() \n");

    if (!pah_comm_write(0x7F, 0x04))    //bank4
        goto FAIL;
#ifdef _INT_PULL_LOW_I2C
		if (!pah_comm_write(0x2B, 0xF8))//If Customer Platform IO can't pull high/low (I2C)
        goto FAIL;
#endif
#ifdef _INT_PULL_LOW_SPI
		if (!pah_comm_write(0x2B, 0xF9))//If Customer Platform IO can't pull high/low (SPI)
        goto FAIL;
#endif
    if (!pah_comm_write(0x69, 0x01))
        goto FAIL;

    // reset states
    g_state.has_started_tg = 0;
    g_state.has_started_ppg = 0;
    g_state.has_started_touch = 0;

    // reset properties
    g_state.prop_curr.report_sample_num_per_ch = 0;
	delay_ms(1);
    return true;

FAIL:
    debug_printf("_pah8011_shutdown error \n");
    return false;
}

static bool _pah8011_startup(void)
{
    debug_printf("_pah8011_startup() \n");

    if (!pah_comm_write(0x7F, 0x04))    //bank4
        goto FAIL;
    if (!pah_comm_write(0x69, 0x00))
        goto FAIL;

    // When the device wakes up from power down mode, delay is necessary for the internal controller to reset the whole system and then the device is ready for the upcoming operations.
    // Without delay, the driver would be unable to write settings correctly.
    delay_ms(2);

    return true;

FAIL:
    debug_printf("_pah8011_startup error \n");
    return false;
}

static bool _pah8011_start_tg(void)
{
    uint8_t data = 0;

    if (g_state.has_started_tg)
        return true;

    debug_printf("_pah8011_start_tg() \n");

    // must sleep between update flag and TG enable.
    delay_ms(1);

    // mask
    if (!pah_comm_write(0x7f, 0x01))
        goto FAIL;
    if (!pah_comm_read(0x37, &data))    //mask
        goto FAIL;
    PAH_CLEAR_BIT(data, 0);             // IntSramFIFO_MaskInt
    PAH_CLEAR_BIT(data, 2);             // int_sramfifo_overflow_mask
    PAH_CLEAR_BIT(data, 3);             // int_sramfifo_underflow_mask
    if (!pah_comm_write(0x37, data))
        goto FAIL;

    // TG enable
    if (!pah_comm_write(0x30, 0x01))    //TG enable
        goto FAIL;

    // wait for clk
    delay_ms(1);

    // clear int_req
    if (!pah_comm_write(0x7F, 0x02))    //Bank 2
        goto FAIL;
    if (!pah_comm_write(0x1B, 0xFF))    //int_req_array
        goto FAIL;

    // mask
    if (!pah_comm_write(0x7f, 0x01))
        goto FAIL;
    if (!pah_comm_read(0x37, &data))    //mask
        goto FAIL;
    PAH_CLEAR_BIT(data, 4);             // AllInt_MaskInt
    if (!pah_comm_write(0x37, data))
        goto FAIL;

    g_state.has_started_tg = 1;
    return true;

FAIL:
    debug_printf("_pah8011_start_tg error \n");
    return false;
}

static bool _pah8011_start_ppg(void)
{
    uint8_t data = 0;
    uint8_t ppg_ch_enabled[pah8011_ppg_ch_num];

    if (g_state.has_started_ppg)
        return true;

    debug_printf("_pah8011_start_ppg() \n");

    pah8011_get_ppg_ch_enabled(ppg_ch_enabled);

    if (!pah_comm_write(0x7F, 0x01))    //bank1
        goto FAIL;

    //channel a
    if (ppg_ch_enabled[pah8011_ppg_ch_a])
    {
        if (!pah_comm_read(0x21, &data))
            goto FAIL;
        PAH_SET_BIT(data, 0);
        if (!pah_comm_write(0x21, data))
            goto FAIL;
    }

    //channel b
    if (ppg_ch_enabled[pah8011_ppg_ch_b])
    {
        if (!pah_comm_read(0x23, &data))
            goto FAIL;
        PAH_SET_BIT(data, 0);
        if (!pah_comm_write(0x23, data))
            goto FAIL;
    }

    //channel c
    if (ppg_ch_enabled[pah8011_ppg_ch_c])
    {
        if (!pah_comm_read(0x25, &data))
            goto FAIL;
        PAH_SET_BIT(data, 0);
        if (!pah_comm_write(0x25, data))
            goto FAIL;
    }

    g_state.has_started_ppg = 1;
    return true;

FAIL:
    debug_printf("_pah8011_start_ppg error \n");
    return false;
}

static bool _pah8011_stop_ppg(void)
{
    uint8_t data = 0;

    if (!g_state.has_started_ppg)
        return true;

    debug_printf("_pah8011_stop_ppg() \n");

    if (!pah_comm_write(0x7F, 0x01))    //bank1
        goto FAIL;

    //channel a
    if (!pah_comm_read(0x21, &data))
        goto FAIL;
    PAH_CLEAR_BIT(data, 0);
    if (!pah_comm_write(0x21, data))
        goto FAIL;

    //channel b
    if (!pah_comm_read(0x23, &data))
        goto FAIL;
    PAH_CLEAR_BIT(data, 0);
    if (!pah_comm_write(0x23, data))
        goto FAIL;

    //channel c
    if (!pah_comm_read(0x25, &data))
        goto FAIL;
    PAH_CLEAR_BIT(data, 0);
    if (!pah_comm_write(0x25, data))
        goto FAIL;

    g_state.has_started_ppg = 0;
    return true;

FAIL:
    debug_printf("_pah8011_stop_ppg error \n");
    return false;
}

static bool _pah8011_start_touch(void)
{
    uint8_t data = 0;

    if (g_state.has_started_touch)
        return true;

    debug_printf("_pah8011_start_touch() \n");

    if (!pah_comm_write(0x7F, 0x01))    //bank1
        goto FAIL;

    // 0x36
    if (!pah_comm_read(0x36, &data))
        goto FAIL;
    if (g_state.prop_curr.is_int2_as_touch_flag)
    {
        PAH_CLEAR_BIT(data, 0);
        PAH_SET_BIT(data, 1);
    }
    else
    {
        PAH_SET_BIT(data, 0);
        PAH_CLEAR_BIT(data, 1);
    }
    if (!pah_comm_write(0x36, data))
        goto FAIL;

    // 0x37
    if (!pah_comm_read(0x37, &data))
        goto FAIL;
    if (g_state.prop_curr.is_int2_as_touch_flag)
    {
        PAH_SET_BIT(data, 1);
    }
    else
    {
        PAH_CLEAR_BIT(data, 1);
    }
    if (!pah_comm_write(0x37, data))
        goto FAIL;

    g_state.has_started_touch = 1;
    return true;

FAIL:
    debug_printf("_pah8011_start_touch error \n");
    return false;
}
static bool _pah8011_stop_touch(void)
{
    uint8_t data = 0;

    if (!g_state.has_started_touch)
        return true;

    debug_printf("_pah8011_stop_touch() \n");

    if (!pah_comm_write(0x7F, 0x01))    //bank1
        goto FAIL;

    // 0x36
    if (!pah_comm_read(0x36, &data))
        goto FAIL;
    PAH_SET_BIT(data, 0);
    PAH_CLEAR_BIT(data, 1);
    if (!pah_comm_write(0x36, data))
        goto FAIL;

    // 0x37
    if (!pah_comm_read(0x37, &data))
        goto FAIL;
    if (g_state.prop_curr.is_int2_as_touch_flag)
    {
        // empty
    }
    else
    {
        PAH_SET_BIT(data, 1);
    }
    if (!pah_comm_write(0x37, data))
        goto FAIL;

    g_state.has_started_touch = 0;
    return true;

FAIL:
    debug_printf("_pah8011_stop_touch error \n");
    return false;
}

static bool _pah8011_clear_fifo_and_int_req(void)
{
    uint8_t int_req = 0;

    debug_printf("_pah8011_clear_fifo_and_int_req() \n");

    if (!pah_comm_write(0x7F, 0x01))    //bank1
        goto FAIL;

    // clear FIFO
    if (!pah_comm_write(0x70, 0x01))    // clear FIFO
        goto FAIL;
    if (!_pah8011_update_flag())
        goto FAIL;
    delay_ms(PPG_FRAME_INTERVAL_MS);
    if (!pah_comm_write(0x70, 0x00))    // clear FIFO
        goto FAIL;
    if (!_pah8011_update_flag())
        goto FAIL;

    //clear interrupt
    if (!pah_comm_write(0x7F, 0x02))    //Bank 2
        goto FAIL;
    if (!pah_comm_read(0x1B, &int_req)) //read interrupt status
        goto FAIL;
    if (int_req > 0)
    {
        if (!pah_comm_write(0x1B, int_req)) //clear interrupt
            goto FAIL;
    }

    return true;

FAIL:
    debug_printf("_pah8011_clear_fifo_and_int_req() error \n");
    return false;
}

static bool _pah8011_update_report_num(void)
{
    if (g_state.prop_curr.report_sample_num_per_ch != g_state.prop_next.report_sample_num_per_ch)
    {
        debug_printf("_pah8011_update_report_num(). report_sample_num_per_ch %d -> %d \n", g_state.prop_curr.report_sample_num_per_ch, g_state.prop_next.report_sample_num_per_ch);

        if (!_pah8011_set_report_num(g_state.prop_next.report_sample_num_per_ch , pah_get_fifo_ch_num()))
            goto FAIL;
        g_state.prop_curr.report_sample_num_per_ch = g_state.prop_next.report_sample_num_per_ch;
    }

    return true;

FAIL:
    debug_printf("_pah8011_update_report_num() fail \n");
    return false;
}

static bool _pah8011_set_report_num(uint32_t samples_per_read, uint32_t ch_num)
{
    uint8_t data = 0;
    uint32_t samples_per_read_plus1 = 0;
    
    if(ch_num==3)
        samples_per_read_plus1 = ch_num*samples_per_read + 2;
    else	
        samples_per_read_plus1 = ch_num*samples_per_read + 1;

    debug_printf("_pah8011_set_report_num(). samples_per_read_plus1 = %d \n", samples_per_read_plus1);

    if (!pah_comm_write(0x7F, 0x01))    //bank1
        goto FAIL;

    //Rpt num. write MSB first.
    data = (((samples_per_read_plus1) >> 8) & 0x01);
    if (!pah_comm_write(0x57, data))
        goto FAIL;

    //Rpt num. write LSB last.
    data = ((samples_per_read_plus1)& 0xFF);
    if (!pah_comm_write(0x56, data))
        goto FAIL;

    if (!_pah8011_update_flag())
        goto FAIL;

    return true;

FAIL:
    debug_printf("_pah8011_set_report_num() fail \n");
    return false;
}

static bool _pah8011_turn_led(bool on)
{
    debug_printf(">>>> _pah8011_turn_led(), on = %d \n", on);

    if (!pah_comm_write(0x7F, 0x00))    //bank0
        goto FAIL;
    if (on)
    {
        if (!pah_comm_write(0x70, 0x01))
            goto FAIL;
        if (!pah_comm_write(0x71, 0x02))
            goto FAIL;
        if (!pah_comm_write(0x72, 0x04))
            goto FAIL;
    }
    else
    {
        if (!pah_comm_write(0x70, 0x00))
            goto FAIL;
        if (!pah_comm_write(0x71, 0x00))
            goto FAIL;
        if (!pah_comm_write(0x72, 0x00))
            goto FAIL;
    }

    debug_printf("<<<< _pah8011_turn_led() \n");
    return true;

FAIL:
    debug_printf("_pah8011_turn_led error \n");
    return false;
}

#if defined(ENABLE_FIFO_CHECKSUM)
static bool _pah8011_cks(uint8_t *fifo, uint32_t samples_read, uint32_t cks_read)
{
    uint32_t *s = (uint32_t *)fifo;
    uint32_t raw;
    uint32_t cks_cal = 0;
    uint32_t i;

    //checksum compare
    for (i = 0; i < samples_read; i++)
    {
        raw = *(s);
        cks_cal = cks_cal ^ raw;
        s++;
    }

    if (cks_cal != cks_read)
    {
        s = (uint32_t *)fifo;

        debug_printf("fifo = { \n");
        for (i = 0; i < samples_read; i++)
        {
            raw = *(s);
            debug_printf("%X, ", raw);
            s++;
        }
        debug_printf("} \n");
        debug_printf("cks_cal == %X, cks_read == %X \n", cks_cal, cks_read);

        return false;
    }
    return true;
}
#endif // ENABLE_FIFO_CHECKSUM

static bool _pah8011_read_touch_flag(uint8_t *touch_flag)
{
    uint8_t data = 0;

    if (!touch_flag)
        goto FAIL;

    if (!pah_comm_write(0x7F, 0x02))    //Bank 2
        goto FAIL;
    if (!pah_comm_read(0x00, &data))
        goto FAIL;

    *touch_flag = data & 0x01;

    return true;

FAIL:
    debug_printf("_pah8011_read_touch_flag() fail \n");
    return false;
}

static bool _pah8011_enable_fifo_clk(bool enable)
{
    if (enable)
    {
        if (!pah_comm_write(0x7F, 0x01))    //Bank 1
            goto FAIL;
        if (!pah_comm_write(0x71, 0x00))
            goto FAIL;
        if (!pah_comm_write(0x7F, 0x00))    //Bank 0
            goto FAIL;
        if (!pah_comm_write(0x10, 0x03))
            goto FAIL;
    }
    else // disable
    {
        if (!pah_comm_write(0x7F, 0x01))    //Bank 1
            goto FAIL;
        if (!pah_comm_write(0x71, 0x01))
            goto FAIL;
        if (!pah_comm_write(0x7F, 0x00))    //Bank 0
            goto FAIL;
        if (!pah_comm_write(0x10, 0x00))
            goto FAIL;
    }

    return true;

FAIL:
    debug_printf("_pah8011_enable_fifo_clk error \n");
    return false;
}

static pah_ret _pah8011_read_ppg_fifo(uint32_t samples_per_read_per_ch, uint32_t ch_num)
{
    pah_ret ret = pah_err_unknown;
    uint32_t samples_per_read = samples_per_read_per_ch * ch_num;

    //debug_printf("_pah8011_read_ppg_fifo(). samples_per_read = %d \n", samples_per_read);

    // read fifo
    if (!pah_comm_write(0x7F, 0x03))    //Bank 3
    {
        ret = pah_err_comm_fail;
        goto FAIL;
    }
    else if (!pah_comm_burst_read(0, g_state.fifo_data, samples_per_read * 4))  //read FIFO
    {
        ret = pah_err_comm_fail;
        goto FAIL;
    }
    else
    {
        uint32_t *s = (uint32_t *)g_state.fifo_data;

        // update touch_flag
        if (g_state.fifo_data_contain_touch_bit)
        {
            g_state.touch_flag = s[(samples_per_read - 1)] & 0x01;
        }
        else
        {
            if (!_pah8011_read_touch_flag(&g_state.touch_flag))
            {
                ret = pah_err_comm_fail;
                goto FAIL;
            }
        }

#if defined(ENABLE_FIFO_CHECKSUM)
        {
            uint8_t cks[8] = { 0 };
            if (!pah_comm_write(0x7F, 0x02))
            {
                ret = pah_err_comm_fail;
                goto FAIL;
            }
            if (!pah_comm_burst_read(0x1C, cks, 4))  //checksum
            {
                ret = pah_err_comm_fail;
                goto FAIL;
            }
            if (!_pah8011_cks(g_state.fifo_data, samples_per_read, *(uint32_t *)cks))
            {
                debug_printf("_pah8011_read_ppg_fifo(). _pah8011_cks fail\n");
                ret = pah_err_fifo_checksum_fail;
                goto FAIL;
            }
            debug_printf("_pah8011_read_ppg_fifo(). _pah8011_cks success\n");
        }
#endif  // ENABLE_FIFO_CHECKSUM
        
        // report fifo
        if (g_state.fp_report_fifo_handler)
        {
            pah_report_fifo fifo;
            fifo.data = g_state.fifo_data;
            fifo.touch_flag = g_state.touch_flag;
            g_state.fp_report_fifo_handler(g_state.user_data, &fifo);
        }

        g_state.fifo_data_num_per_ch = samples_per_read_per_ch;
    }

    return pah_success;

FAIL:
    debug_printf("_pah8011_read_ppg_fifo(). fail, ret = %d \n", ret);
    return ret;
}

static pah_ret _pah8011_task_dri(uint8_t int_req, uint32_t ch_num)
{
    pah_ret ret = pah_err_unknown;
	
    // Touch Flag
	  if (!_pah8011_read_touch_flag(&g_state.touch_flag))
    {
        ret = pah_err_comm_fail;
		    goto FAIL;
    }
   
    // no interrupt
    if (!int_req)
    {
        // ignore
        debug_printf("_pah8011_task_dri(). No interrupt. \n");
        return pah_pending;
    }

    // fifo interrupt
    if (int_req & 0x01)
    {
        uint32_t samples_per_read = g_state.prop_curr.report_sample_num_per_ch * ch_num;
        uint16_t fifo_data_number = 0;

        if (!pah_comm_write(0x7F, 0x02))    //Bank 2
        {
            ret = pah_err_comm_fail;
            goto FAIL;
        }
        if (!pah_comm_burst_read(0x25, (uint8_t*)&fifo_data_number, 2))
        {
            ret = pah_err_comm_fail;
            goto FAIL;
        }

        //debug_printf("_pah8011_task_dri(). fifo_data_number = %d, samples_per_read = %d. \n", fifo_data_number, samples_per_read);
        
        if (fifo_data_number < samples_per_read)
            return pah_pending;

        ret = _pah8011_read_ppg_fifo(g_state.prop_curr.report_sample_num_per_ch, ch_num);
        if (ret != pah_success)
            goto FAIL;
    }

    return pah_success;

FAIL:
    debug_printf("_pah8011_task_dri(). fail, ret = %d \n", ret);
    return ret;
}

static pah_ret _pah8011_task_polling(uint8_t int_req, uint32_t ch_num)
{
    pah_ret ret = pah_err_unknown;
    uint16_t fifo_data_number = 0;
    uint32_t samples_per_read_per_ch = 0;
	
    if (!pah_comm_write(0x7F, 0x02))    //Bank 2
    {
        ret = pah_err_comm_fail;
        goto FAIL;
    }
    if (!pah_comm_burst_read(0x25, (uint8_t*)&fifo_data_number, 2))
    {
        ret = pah_err_comm_fail;
        goto FAIL;
    }
		// touch interrupt
    
    if (!_pah8011_read_touch_flag(&g_state.touch_flag))
    {
        ret = pah_err_comm_fail;
        goto FAIL;
    }
    
    if (fifo_data_number == 0)
       return pah_pending;

    debug_printf("_pah8011_task_polling(). fifo_data_number = %d \n", fifo_data_number);

    samples_per_read_per_ch = (fifo_data_number - 1) / ch_num;
    if (samples_per_read_per_ch < 1)
        return pah_pending;

    // enable clk
    if (!_pah8011_enable_fifo_clk(true))
    {
        ret = pah_err_comm_fail;
        goto FAIL;
    }

    ret = _pah8011_read_ppg_fifo(samples_per_read_per_ch, ch_num);

    // disable clk
    if (!_pah8011_enable_fifo_clk(false))
    {
        ret = pah_err_comm_fail;
        goto FAIL;
    }

    if (ret != pah_success)
        goto FAIL;

    return pah_success;

FAIL:
    debug_printf("_pah8011_task_polling(). fail, ret = %d \n", ret);
    return ret;
}



static float time_target_back = 50.00f;
static float time_target_avg = 50.00f;
static bool time_target_init = 0;


void pah_do_timing_tuning(int time_diff/*ms*/, int samples_per_ch, float _time_target)
{
    float exp_sampling_time ;
    float exp_packet_time ;
    float f_tuning_factor ;
    float _time_thd_high = _time_target+0.02f ;
    float _time_thd_low =  _time_target-0.02f ;
    float _time_target_20_percent = _time_target*0.20f; //20% mistake
	
    int i_tuninf_factor = 0;
    uint8_t u8_tuning_factor_h ;
    uint8_t u8_tuning_factor_l ;
    float actual_sampling_time = (float)time_diff / samples_per_ch ;
    //float tuning_cutoff;
		
    if(time_target_init==0)
    {
        time_target_back=actual_sampling_time;
		    time_target_init=1;
    }
    if (g_state.flags.stream == pah_stream_dri)
        time_target_avg = ((time_target_back*3)+(actual_sampling_time))/4;
    else
        time_target_avg = ((time_target_back*15)+(actual_sampling_time))/16;
		
    //debug_printf("time_diff= %d  samples_per_ch =%d  actual_sampling_time = %f time_target_back =  %f time_target_avg =  %f\n", time_diff,samples_per_ch,actual_sampling_time,time_target_back,time_target_avg);
    time_target_back= time_target_avg ;
    actual_sampling_time= time_target_avg;
    if(actual_sampling_time > _time_target){
        if(actual_sampling_time > (_time_target + _time_target_20_percent))
            return ;
    }
    else{
        if(actual_sampling_time < (_time_target - _time_target_20_percent))
            return ;
    }	
		
    if(actual_sampling_time > _time_thd_low &&
       actual_sampling_time < _time_thd_high ){
        return ;
    }

    pah_comm_write(0x7f, 0x01);
    pah_comm_burst_read(0x26, (uint8_t *)(&i_tuninf_factor), 2);
		
    exp_sampling_time = _time_target;
    exp_packet_time = (float)(exp_sampling_time * samples_per_ch) ;
    f_tuning_factor = (float)(exp_packet_time * i_tuninf_factor) / time_diff;
    i_tuninf_factor = (int)(f_tuning_factor + 0.5f) ;
    u8_tuning_factor_h = (i_tuninf_factor >> 8) & 0xFF ;
    u8_tuning_factor_l = (i_tuninf_factor & 0xFF) ;
	
    pah_comm_write(0x26, u8_tuning_factor_l);
    pah_comm_write(0x27, u8_tuning_factor_h);
    _pah8011_update_flag();
}


uint8_t pah8011_setting_version(void)
{
return pah8011_get_setting_version();
}
#ifdef ENABLE_PXI_LINK_VP_REG	
uint8_t pah8011_setting_version_VP(void)
{
return pah8011_get_setting_version_VP();
}
#endif

uint8_t pah8011_ae_info_read(uint32_t* Expo_time, uint8_t* LEDDAC, uint16_t *touch_data)
{
	pah_ret ret = pah_err_unknown;		
    uint8_t data_LByte,data_HByte=0;

    if (!pah_comm_write(0x7F, 0x02))    //Bank 2
    {
        ret = pah_err_comm_fail;
        goto FAIL;
    }
    if (!pah_comm_burst_read(0x03, (uint8_t*)&Expo_time[0], 3))
        goto FAIL;
    if (!pah_comm_burst_read(0x06, (uint8_t*)&Expo_time[1], 3))
        goto FAIL;
    if (!pah_comm_burst_read(0x09, (uint8_t*)&Expo_time[2], 3))
        goto FAIL;
    if (!pah_comm_read(0x0C, &LEDDAC[0]))
        goto FAIL;
	if (!pah_comm_read(0x0D, &LEDDAC[1]))
	    goto FAIL;
	if (!pah_comm_read(0x0E, &LEDDAC[2]))
		goto FAIL;
    if (!pah_comm_read(0x00, &data_LByte))
        goto FAIL;
    if (!pah_comm_read(0x01, &data_HByte))
        goto FAIL;	
    *touch_data = (data_HByte<<8)|(data_LByte&0xFE);
		
	return true;

FAIL:
    debug_printf("pah8011_ae_info_read(). fail, ret = %d \n", ret);
    return ret;
}
#if defined(Touch_Detect_Green_Led)
void Set_Touch_Detect_TH(uint16_t Touch_Th_Hi, uint16_t Touch_Th_Lo)
{
    debug_printf("Set_Green_Led_Touch_Detect  Touch_Th_Hi= %d , Touch_Th_Lo= %d\n",Touch_Th_Hi ,Touch_Th_Lo);
	Touch_Th_Hi_Value = Touch_Th_Hi;
	Touch_Th_Lo_Value = Touch_Th_Lo;
}
#endif

