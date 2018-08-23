// gsensor
//#include "accelerometer.h"

// platform support
//#include "system_clock.h"

//#include "debug_log.h"

// C library
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

//#include "pah_vp_function.h"

// Platform support
#include "system_clock.h"

// pah8011_Driver
#include "pah8series_config.h"
#include "pah_driver_types.h"
#include "pah_driver.h"
#include "pah_comm.h"

//vp & hr API
#include "pxialg_vp.h"
#include "pxialg.h"

//ROM ex
//static float * addr= 0;

// #include "SEGGER_RTT.h"
// #include "SEGGER_RTT_Conf.h"

extern void (*BspHrmCalculateCompleteCb)(uint8_t HrmValue,uint8_t sbp,uint8_t dbp,uint8_t sq);

/*============================================================================
STATIC VARIABLES
============================================================================*/
static main_state_s _state;
static pah_flags_s flags;

struct Report username_rpt;
static float true_sbp;
static float true_dbp;
static float regulate_ppgsp;
static float regulate_ppgdp;
static uint32_t VP_test_count=0;
static uint8_t pah8011_VP_ALG_Mode = VP_Suspend;
static bool report_flag_ouput = 0;
//V117
static int gender;
static int age;
static float height;
static float weight;
//for static hr
static float myHR=0;
static float myHR_Back=0;
static float myHR_Out=0;
static uint8_t HR_Count = 0;

//static uint32_t Timing_Tuning_count=0;
//static uint32_t Timing_Tuning_time=0;

static bool need_log_header = true;


static uint32_t Expo_time_backup[3]={0};
static uint8_t LEDDAC_backup[3]={0};

/*============================================================================
STATIC FUNCTION PROTOTYPES
============================================================================*/
#if defined(PPG_MODE_ONLY)|| defined(ALWAYS_TOUCH)
static void start_healthcare_ppg(void);
#endif
static void start_healthcare_ppg_touch(void);
static void start_healthcare_touch_only(void);
static void stop_healthcare(void);
static void log_pah8series_data_header(uint32_t ch_num, uint32_t ir_ch_num, uint32_t g_sensor_mode);
static void log_pah8series_data(const pah8series_data_t *data);
static void log_pah8series_no_touch(void);
static void pah8011_ae_info_check(pah8series_data_t *pxialg_data, bool Tuning);
static void report_fifo_data_vp(uint64_t timestamp, uint8_t *fifo_data, uint32_t fifo_data_num_per_ch, uint32_t ch_num, bool is_touched);
static bool vp_algorithm_process(pah8series_data_t *pxialg_data, uint32_t ch_num);
static bool vp_algorithm_calculate(pah8series_data_t *pxialg_data, uint32_t ch_num);
static void vp_algorithm_close(void);

static void Error_Handler(void);

void pah8series_ppg_dri_VP_init(void)
{
	memset(&_state, 0, sizeof(_state));
	// PAH
    {
        pah_flags_default(&flags);
        flags.stream = pah_stream_dri;
        flags.alg_mode = pah_alg_mode_VP;
        if (!pah_init_with_flags(&flags))
        {
            debug_printf("pah_init_with_flags() fail. \n");
            Error_Handler();
        }
    }
	
}

void pah8series_ppg_polling_VP_init(void)
{
	memset(&_state, 0, sizeof(_state));
	// PAH
    {
        pah_flags_default(&flags);
        flags.stream = pah_stream_polling;
        flags.alg_mode = pah_alg_mode_VP;
        if (!pah_init_with_flags(&flags))
        {
            debug_printf("pah_init_with_flags() fail. \n");
            Error_Handler();
        }
    }

}

void pah8series_ppg_dri_VP_Calibration_start(float real_sbp,float real_dbp)
{
	pah8011_VP_ALG_Mode = VP_Calibration;
	need_log_header = true;
	true_sbp =real_sbp ;//Please Enter Tester Real SBP
	true_dbp =real_dbp ; //Please Enter Tester Real DBP
#if defined(PPG_MODE_ONLY)|| defined(ALWAYS_TOUCH)
    start_healthcare_ppg();
#else
    start_healthcare_touch_only();
#endif
    _state.status = main_status_start_healthcare;
}

//V117
void pah8series_ppg_dri_VP_Phy_Calibration_start(uint32_t user_gender,uint32_t user_age, float user_height, float user_weight)
{
	pah8011_VP_ALG_Mode = VP_Phy_Calibration;
	need_log_header = true;
	true_sbp = 0;
	true_dbp = 0;
	gender = user_gender;
	age = user_age;
	height = user_height;
	weight = user_weight;
#if defined(PPG_MODE_ONLY)|| defined(ALWAYS_TOUCH)
    start_healthcare_ppg();
#else
    start_healthcare_touch_only();
#endif
    _state.status = main_status_start_healthcare;
}

void pah8series_ppg_dri_VP_Estimation_start(void)
{
	pah8011_VP_ALG_Mode = VP_Estimation;
	need_log_header = true;
#if defined(PPG_MODE_ONLY)|| defined(ALWAYS_TOUCH)
    start_healthcare_ppg();
#else
    start_healthcare_touch_only();
#endif
    _state.status = main_status_start_healthcare;
}

//V117
void pah8series_ppg_dri_VP_Phy_Estimation_start(void)
{
	pah8011_VP_ALG_Mode = VP_Phy_Estimation;
	need_log_header = true;
#if defined(PPG_MODE_ONLY)|| defined(ALWAYS_TOUCH)
    start_healthcare_ppg();
#else
    start_healthcare_touch_only();
#endif
    _state.status = main_status_start_healthcare;
}
//Without calibration
void pah8series_ppg_dri_VP_WithoutCal_Estimation_start(void)
{
	pah8011_VP_ALG_Mode = VP_WithoutCal_Estimation;
	_state.vp_calibration_ready = 1;
	need_log_header = true;
#if defined(PPG_MODE_ONLY)|| defined(ALWAYS_TOUCH)
    start_healthcare_ppg();
#else
    start_healthcare_touch_only();
#endif
    _state.status = main_status_start_healthcare;
}

void pah8series_ppg_VP_stop(void)
{
// log
	log_pah8series_no_touch();
	_state.alg_status = alg_status_close ;
	_state.Tuning=0 ;
	stop_healthcare();
	_state.status = main_status_idle;

}

void pah8series_ppg_dri_VP_task(volatile bool* has_interrupt_pah,volatile bool* has_interrupt_button ,volatile uint64_t*    interrupt_pah_timestamp)
{
    
	pah_ret ret = pah_err_unknown;
	if (_state.status == main_status_start_healthcare)
	{
		if (*has_interrupt_pah)
		{
			*has_interrupt_pah = false;
		
			ret = pah_task();
			if (ret == pah_success)
			{					
				if (pah_is_ppg_mode() )
				{
					uint8_t ppg_mode_flag =0 ;
				#if defined (PPG_MODE_ONLY)|| defined(ALWAYS_TOUCH)
					ppg_mode_flag = 1 ;
				#endif
					bool is_touched = pah_is_touched();
          #ifdef ALWAYS_TOUCH
						   is_touched = 1 ;
					#endif
					if (is_touched || ppg_mode_flag)
					{
						uint8_t *fifo_data = pah_get_fifo_data();
						uint32_t fifo_data_num_per_ch = pah_fifo_data_num_per_ch();
						uint32_t fifo_ch_num = pah_get_fifo_ch_num();
            debug_printf("\n*interrupt_pah_timestamp == %d\n",*interrupt_pah_timestamp); 								
            report_fifo_data_vp(*interrupt_pah_timestamp, fifo_data, fifo_data_num_per_ch, fifo_ch_num, is_touched);
					}		
					else
					{
						// log
						log_pah8series_no_touch();
            _state.alg_status = alg_status_close ;
            _state.Tuning=0 ;
						start_healthcare_touch_only();
					}
				}
				else if (pah_touch_mode == pah_query_mode())
				{
					if (pah_is_touched())
					start_healthcare_ppg_touch();
				}
			}
			else if (ret == pah_pending)
			{
				// ignore
			}
			else
			{
				debug_printf("pah_task fail, ret = %d \n", ret);
				Error_Handler();
			}
		}
  #if 0 // unused			
		if(*has_interrupt_button)
        {
            *has_interrupt_button = false;
            pah8series_ppg_VP_stop();
        }
	#endif			
	}
	else if (_state.status == main_status_idle)
	{
	#if 0 // unused	
		if (*has_interrupt_button)
		{
			*has_interrupt_button = false;
				
		#if defined(PPG_MODE_ONLY)
			start_healthcare_ppg();
		#else
			start_healthcare_touch_only();
		#endif
			_state.status = main_status_start_healthcare;
		} 
	#endif
	}
}
void pah8series_ppg_polling_VP_task(volatile bool* has_interrupt_pah,volatile bool* has_interrupt_button )
{
	pah_ret ret = pah_err_unknown;
	// unused
        if (*has_interrupt_pah)
            *has_interrupt_pah = false;
        
        if (_state.status == main_status_start_healthcare)
        {
            ret = pah_task();
            if (ret == pah_success)
            {
                if (pah_is_ppg_mode())
                {                  
                    uint8_t ppg_mode_flag =0 ;
                #if defined(PPG_MODE_ONLY)|| defined(ALWAYS_TOUCH)
                    ppg_mode_flag = 1 ;
                #endif
                    bool is_touched = pah_is_touched();
          #ifdef ALWAYS_TOUCH
						   is_touched = 1 ;
					#endif
                    if (is_touched || ppg_mode_flag)
                    {
                            uint32_t timestamp = get_tick_count();
                            uint8_t *fifo_data = pah_get_fifo_data();
                            uint32_t fifo_data_num_per_ch = pah_fifo_data_num_per_ch();
                            uint32_t fifo_ch_num = pah_get_fifo_ch_num();
                            //bool is_touched = pah_is_touched(); 
                            report_fifo_data_vp(timestamp, fifo_data, fifo_data_num_per_ch, fifo_ch_num, is_touched);
                    }
                
                    else
                    {
                        // log
                        log_pah8series_no_touch();
							          _state.alg_status = alg_status_close ;
							          _state.Tuning=0 ;
                        start_healthcare_touch_only();
                    }
                
                }
                else if (pah_touch_mode == pah_query_mode())
                {
                        if (pah_is_touched())
                                start_healthcare_ppg_touch();
                }
            }
            else if (ret == pah_pending)
            {
                // ignore
            }
            else
            {
                debug_printf("pah_task fail, ret = %d \n", ret);
                Error_Handler();
            }
            #if 0 // unused            
            if (*has_interrupt_button)
            {
                *has_interrupt_button = false;
                pah8series_ppg_VP_stop();
            }
						#endif
        }
        else if (_state.status == main_status_idle)
        {
        #if 0 // unused					
            if (*has_interrupt_button)
            {
                *has_interrupt_button = false;
                
            #if defined(PPG_MODE_ONLY)
                start_healthcare_ppg();
            #else
                start_healthcare_touch_only();
            #endif
                _state.status = main_status_start_healthcare;
            }
        #endif
        }
}


void vp_alg_task(void)
{
  switch(_state.alg_status)
	{
	case alg_status_process:
        {
	        vp_algorithm_process(&_state.pxialg_data, _state.pxialg_data.nf_ppg_channel);
            _state.alg_status = alg_status_idle;
	    }
	    break;

	case alg_status_close:
        {
	        vp_algorithm_close();
            _state.alg_status = alg_status_idle;
	    }

	case alg_status_idle:
		{
		}
		break;
		
	default:
	    break;
    }
}


static void log_pah8series_data_header(uint32_t ch_num, uint32_t ir_ch_num, uint32_t g_sensor_mode)
{
#ifdef ENABLE_PXI_ALG_VP        
    if(pah8011_VP_ALG_Mode == VP_Calibration)
    {
        // log parameters which will enter in calibration process
    	log_printf("[calibration]\n");
    	log_printf("regulate_ppgsp = 0\nregulate_ppgdp = 0\n");
    	log_printf("true_sbp = "LOG_FLOAT_MARKER" \n",LOG_FLOAT(true_sbp));
    	log_printf("true_dbp = "LOG_FLOAT_MARKER" \n",LOG_FLOAT(true_dbp));
    	log_printf("Operation Mode (0:no calibration) = 1\n");
        //log_printf("[calibration]\nregulate_ppgsp = 0\nregulate_ppgdp = 0\ntrue_sbp = %f\ntrue_dbp = %f\nOperation Mode (0:no calibration) = true\n", true_sbp, true_dbp);
    }
    //V117
    else if(pah8011_VP_ALG_Mode == VP_Phy_Calibration)
    {
        // log parameters which will enter in calibration process
    	log_printf("[calibration]\n");
    	log_printf("regulate_ppgsp = 0\nregulate_ppgdp = 0\n");
    	log_printf("true_sbp = "LOG_FLOAT_MARKER" \n",LOG_FLOAT(true_sbp));
    	log_printf("true_dbp = "LOG_FLOAT_MARKER" \n",LOG_FLOAT(true_dbp));
    	log_printf("gender = %d\n", gender);
    	log_printf("age = %d\n", age);
    	log_printf("height = "LOG_FLOAT_MARKER" \n",LOG_FLOAT(height));
    	log_printf("weight = "LOG_FLOAT_MARKER" \n",LOG_FLOAT(weight));
    	log_printf("Operation Mode (0:no calibration) = 2\n");
        //log_printf("[calibration]\nregulate_ppgsp = 0\nregulate_ppgdp = 0\ntrue_sbp = %f\ntrue_dbp = %f\ngender = %d\nage = %d\nheight = %f\nweight = %f\nOperation Mode (0:no calibration) = true\n",true_sbp,true_dbp, gender, age, height, weight);
    }
    else if(pah8011_VP_ALG_Mode == VP_Estimation)
    {
        // Aquire VP INFO from ROM
//    	ROM ex
//		addr = 0x7F00; //Address set By Customer Platform Flash Area
//    	flash_float_read( addr , &regulate_ppgsp );
//    	addr = 0x7F04; //Address set By Customer Platform Flash Area
//    	flash_float_read( addr , &regulate_ppgdp );
//    	addr = 0x7F08; //Address set By Customer Platform Flash Area
//    	flash_float_read( addr , &true_sbp );
//    	addr = 0x7F0C; //Address set By Customer Platform Flash Area
//    	flash_float_read( addr , &true_dbp );
//    	addr = 0x7F10; //Address set By Customer Platform Flash Area

    	log_printf("[Measurement]\n");
    	log_printf("regulate_ppgsp = "LOG_FLOAT_MARKER" \n",LOG_FLOAT(regulate_ppgsp));
    	log_printf("regulate_ppgdp = "LOG_FLOAT_MARKER" \n",LOG_FLOAT(regulate_ppgdp));
    	log_printf("true_sbp = "LOG_FLOAT_MARKER" \n",LOG_FLOAT(true_sbp));
    	log_printf("true_dbp = "LOG_FLOAT_MARKER" \n",LOG_FLOAT(true_dbp));
    	log_printf("Operation Mode (0:no calibration) = 1\n");
    	//log_printf("[Measurement]\nregulate_ppgsp = %f\nregulate_ppgdp = %f\ntrue_sbp = %f\ntrue_dbp = %f\nOperation Mode (0:no calibration) = true\n", regulate_ppgsp, regulate_ppgdp, true_sbp, true_dbp);

    }
    //V117
    else if(pah8011_VP_ALG_Mode == VP_Phy_Estimation)
    {
        // Aquire VP INFO from ROM
//    	ROM ex
//		addr = 0x7F00; //Address set By Customer Platform Flash Area
//    	flash_float_read( addr , &regulate_ppgsp );
//    	addr = 0x7F04; //Address set By Customer Platform Flash Area
//    	flash_float_read( addr , &regulate_ppgdp );
//    	addr = 0x7F08; //Address set By Customer Platform Flash Area
//    	flash_float_read( addr , &true_sbp );
//    	addr = 0x7F0C; //Address set By Customer Platform Flash Area
//    	flash_float_read( addr , &true_dbp );
//    	addr = 0x7F14; //Address set By Customer Platform Flash Area
//    	flash_word_read( addr , gender );
//    	addr = 0x7F18; //Address set By Customer Platform Flash Area
//    	flash_word_read( addr , age );
//    	addr = 0x7F1C; //Address set By Customer Platform Flash Area
//    	flash_float_read( addr , height );
//    	addr = 0x7F20; //Address set By Customer Platform Flash Area
//    	flash_float_read( addr , weight );

    	log_printf("[Measurement]\n");
    	log_printf("regulate_ppgsp = "LOG_FLOAT_MARKER" \n",LOG_FLOAT(regulate_ppgsp));
    	log_printf("regulate_ppgdp = "LOG_FLOAT_MARKER" \n",LOG_FLOAT(regulate_ppgdp));
    	log_printf("true_sbp = "LOG_FLOAT_MARKER" \n",LOG_FLOAT(true_sbp));
    	log_printf("true_dbp = "LOG_FLOAT_MARKER" \n",LOG_FLOAT(true_dbp));
    	log_printf("gender = %d\n", gender);
    	log_printf("age = %d\n", age);
    	log_printf("height = "LOG_FLOAT_MARKER" \n",LOG_FLOAT(height));
    	log_printf("weight = "LOG_FLOAT_MARKER" \n",LOG_FLOAT(weight));
    	log_printf("Operation Mode (0:no calibration) = 2\n");
    	//log_printf("[Measurement]\nregulate_ppgsp = %f\nregulate_ppgdp = %f\ntrue_sbp = %f\ntrue_dbp = %f\ngender = %d\nage = %d\nheight = %f\nweight = %f\nOperation Mode (0:no calibration) = true\n", regulate_ppgsp, regulate_ppgdp, true_sbp, true_dbp, gender, age, height, weight);

    }

#endif    
    
    //log pah8series data header
    // (1)Using total channel numbers;
    // (2)reserved;
    // (3)reserved;
    // (4)IR channel number;
    // (5)MEMS mode 0:2G, 1:4G, 2:8G
    log_printf("PPG CH#, %d, %d, %d, %d, %d\n", ch_num, pah8011_setting_version(), 0, ir_ch_num, g_sensor_mode);
}

static void log_pah8series_data(const pah8series_data_t *pxialg_data)
{
    int i = 0;
    uint32_t *ppg_data = (uint32_t *)pxialg_data->ppg_data;
    int16_t *mems_data = pxialg_data->mems_data;
    int data_num = pxialg_data->nf_ppg_channel * pxialg_data->nf_ppg_per_channel;
    
    log_printf("Frame Count, %d \n", pxialg_data->frame_count);
    log_printf("Time, %d \n", pxialg_data->time);
    log_printf("PPG, %d, %d, ", pxialg_data->touch_flag, data_num);
    for (i = 0; i < data_num; ++i)
    {
        log_printf("%d, ", *ppg_data);
        ppg_data++;
    }
    log_printf("\n");
    log_printf("MEMS, %d, ", pxialg_data->nf_mems);
    for (i = 0; i < pxialg_data->nf_mems * 3; ++i)
    {
        log_printf("%d, ", *mems_data);
        mems_data++;
    }
    log_printf("\n");
}




static void log_pah8series_no_touch(void)
{
    static const int DUMMY_PPG_DATA_NUM = 20;
    static const int DUMMY_PPG_CH_NUM = 2;
    int i = 0;
    
    log_printf("Frame Count, 0 \n");
    log_printf("Time, %d \n", DUMMY_PPG_DATA_NUM * 50); // 20Hz = 50ms
    log_printf("PPG, 0, %d, ", DUMMY_PPG_DATA_NUM * DUMMY_PPG_CH_NUM);
    for (i = 0; i < DUMMY_PPG_DATA_NUM * DUMMY_PPG_CH_NUM; i++)
    {
        log_printf("0, ");
    }
    log_printf("\n");
    log_printf("MEMS, %d, ", DUMMY_PPG_DATA_NUM);
    for (i = 0; i < DUMMY_PPG_DATA_NUM * 3; ++i)
    {
        log_printf("0, ");
    }
    log_printf("\n");
}

static void report_fifo_data_vp(uint64_t timestamp, uint8_t *fifo_data, uint32_t fifo_data_num_per_ch, uint32_t ch_num, bool is_touched)
{
    // gsensor
#if defined(ENABLE_MEMS_ZERO)
    _state.pxialg_data.mems_data = _state.mems_data;
    _state.pxialg_data.nf_mems = fifo_data_num_per_ch;
#else
    accelerometer_get_fifo(&_state.pxialg_data.mems_data, &_state.pxialg_data.nf_mems);
#endif  // ENABLE_MEMS_ZERO
        
    _state.pxialg_data.touch_flag = is_touched ? 1 : 0;
    _state.pxialg_data.time = (uint32_t)(timestamp - _state.last_report_time);
    _state.pxialg_data.ppg_data = (int32_t*)fifo_data;
    _state.pxialg_data.nf_ppg_channel = ch_num;
    _state.pxialg_data.nf_ppg_per_channel = fifo_data_num_per_ch;
    ++_state.pxialg_data.frame_count;
    
    _state.last_report_time = timestamp;
#ifdef Timing_Tuning
    if(_state.Tuning)
    {
        pah_do_timing_tuning(_state.pxialg_data.time, _state.pxialg_data.nf_ppg_per_channel,50.0f);
    }
    _state.Tuning=1;
#endif
    debug_printf("\n_state.pxialg_data.time == %d\n", _state.pxialg_data.time);
    
		pah8011_ae_info_read(_state.Expo_time, _state.LEDDAC, &_state.Touch_data);
    _state.alg_status = alg_status_process ;			
}

uint8_t test_count=0;
uint8_t hrmbuf[15000];
static bool vp_algorithm_calculate(pah8series_data_t *pxialg_data, uint32_t ch_num)
{
    bool has_updated = false;
    
#if defined(ENABLE_PXI_ALG_VP)
    
	// Algorithm support only data length >= 10
	if (pxialg_data->nf_ppg_per_channel < 10)
	{
		debug_printf("vp_algorithm_calculate(). fifo_data_num_per_ch = %d, is not enough to run algorithm \n", pxialg_data->nf_ppg_per_channel);
		return false;
	}
    
    // Initialize algorithm
	if (!_state.pxialg_has_init)
	{
		int vp_size = PxiAlg_VP_GetMemorySize();
		debug_printf("PxiAlg_VP_GetMemorySize() = %d \n", vp_size);
		_state.pxialg_buffer  = (unsigned char *)hrmbuf; 

		//V117
		if((pah8011_VP_ALG_Mode == VP_Calibration)||(pah8011_VP_ALG_Mode == VP_Phy_Calibration))
		{
			_state.vp_calibration_ready = 0 ;

			//ROM ex
//			addr = 0x7F08; //Address set By Customer Platform Flash Area
//			flash_float_write( addr , true_sbp );
//			addr = 0x7F0C; //Address set By Customer Platform Flash Area
//			flash_float_write( addr , true_dbp );
//			addr = 0x7F10; //Address set By Customer Platform Flash Area
//			flash_float_write( addr , _state.vp_calibration_ready );

			if(pah8011_VP_ALG_Mode == VP_Phy_Calibration)
			{
				//ROM ex
//				flash_word_write( addr , gender );
//				addr = 0x7F18; //Address set By Customer Platform Flash Area
//				flash_word_write( addr , age );
//				addr = 0x7F1C; //Address set By Customer Platform Flash Area
//				flash_float_write( addr , height );
//				addr = 0x7F20; //Address set By Customer Platform Flash Area
//				flash_float_write( addr , weight );

				if (!PxiAlg_VP_Open(0.0f, 0.0f, true_sbp, true_dbp , 100, _state.pxialg_buffer))
				{
					debug_printf("PxiAlg_VP_Open() failed. \n");
					return false;
				}

				PxiAlg_VP_SetOperationMode(2);
				PxiAlg_VP_SetPhysicalInfo(gender, age, height, weight);
			}
			else if(pah8011_VP_ALG_Mode == VP_Calibration)
			{
				if (!PxiAlg_VP_Open(0.0f, 0.0f, true_sbp, true_dbp , 100, _state.pxialg_buffer))
				{
					debug_printf("PxiAlg_VP_Open() failed. \n");
					return false;
				}
			}
		}
		else if ((pah8011_VP_ALG_Mode == VP_Estimation)||(pah8011_VP_ALG_Mode == VP_Phy_Estimation)||(pah8011_VP_ALG_Mode == VP_WithoutCal_Estimation))//Without calibration
		{
//			ROM ex
//			addr = 0x7F00; //Address set By Customer Platform Flash Area
//			flash_float_read( addr , &regulate_ppgsp );
//			addr = 0x7F04; //Address set By Customer Platform Flash Area
//			flash_float_read( addr , &regulate_ppgdp );
//			addr = 0x7F08; //Address set By Customer Platform Flash Area
//			flash_float_read( addr , &true_sbp );
//			addr = 0x7F0C; //Address set By Customer Platform Flash Area
//			flash_float_read( addr , &true_dbp );
//			addr = 0x7F10; //Address set By Customer Platform Flash Area
//			flash_float_read( addr , &_state.vp_calibration_ready );

			if(_state.vp_calibration_ready == 0)
			{
				debug_printf("PxiAlg_VP_calibration_not_ready. \n");
				return false;
			}

			if(pah8011_VP_ALG_Mode == VP_Phy_Estimation)
			{
				//ROM ex
//				addr = 0x7F14; //Address set By Customer Platform Flash Area
//				flash_word_read( addr , gender );
//				addr = 0x7F18; //Address set By Customer Platform Flash Area
//				flash_word_read( addr , age );
//				addr = 0x7F1C; //Address set By Customer Platform Flash Area
//				flash_float_read( addr , height );
//				addr = 0x7F20; //Address set By Customer Platform Flash Area
//				flash_float_read( addr , weight );

				if (!PxiAlg_VP_Open(regulate_ppgsp, regulate_ppgdp, true_sbp, true_dbp , 100, _state.pxialg_buffer))
				{
					debug_printf("PxiAlg_VP_Open() failed. \n");
					return false;
				}

				PxiAlg_VP_SetOperationMode(2);
				PxiAlg_VP_SetPhysicalInfo(gender, age, height, weight);

			}
			else if(pah8011_VP_ALG_Mode == VP_Estimation)
			{
				if (!PxiAlg_VP_Open(regulate_ppgsp, regulate_ppgdp, true_sbp, true_dbp , 100, _state.pxialg_buffer))
				{
					debug_printf("PxiAlg_VP_Open() failed. \n");
					return false;
				}
			}
            else if(pah8011_VP_ALG_Mode == VP_WithoutCal_Estimation)	//Without calibration
			{
				if (!PxiAlg_VP_Open(0.0f, 0.0f, 0.0f, 0.0f , 100, _state.pxialg_buffer))
				{
					debug_printf("PxiAlg_VP_Open() failed. \n");
					return false;
				}
				PxiAlg_VP_SetOperationMode(0);
			}
		}

		PxiAlg_VP_SetAnomalyTime(1);
		PxiAlg_VP_SetTimeoutLength(2000);
		//PxiAlg_VP_SetModel(0.023f, 103.0f, 0.05f, 63.0f);     // 8011ES_DI+Wall(3.7mm)
		PxiAlg_VP_SetModel(-0.108f, 188.0f, -0.16f, 85.7f);     // 8011ES_DI+Wall(3.7mm)
		
		VP_test_count = 0 ;
		report_flag_ouput = 0 ;
        
        //for static hr
        PxiAlg_Close();
		PxiAlg_Open_SamplingRate(100);
        
		_state.pxialg_has_init = true;
	}
	
	uint32_t *ppg_data = (uint32_t *)pxialg_data->ppg_data;
	uint8_t report_flag;
    float hr_time;  //for static hr
	int i ;
	
    hr_time = ((float)pxialg_data->time) / pxialg_data->nf_ppg_per_channel;
	for(i= 0 ; i < pxialg_data->nf_ppg_channel * pxialg_data->nf_ppg_per_channel;i++)
	{
        PxiAlg_DecodeProcess(*ppg_data, 1, hr_time);    //for static hr
		report_flag = PxiAlg_VP_DecodeProcess(*ppg_data, 1);
		if(report_flag)
		{		
			report_flag_ouput = 1 ;
		}
		ppg_data++;
		VP_test_count++;
	}
	
	//for static hr
    PxiAlg_HrGet(&myHR);
	if((myHR!=0)&&(myHR!=myHR_Back))
	{
		myHR_Out = (float)(myHR_Out * HR_Count + myHR)/(HR_Count+1);
		HR_Count++;
		debug_printf("HR_Count = %d \n", HR_Count);
		//debug_printf("Heart Rate = %f \n", myHR_Out);
		debug_printf("Heart Rate = "LOG_FLOAT_MARKER" \n",LOG_FLOAT(myHR));
		myHR_Back = myHR ;
	}
    
	debug_printf("VP_test_count = %d report_flag_ouput = %d\n", VP_test_count,report_flag_ouput);
	
	if (report_flag_ouput == 1)
	{
			
		float signal_quality = 0;
		int ANO_flag = 0;
		
		signal_quality = PxiAlg_VP_GetSignalQuality();
		ANO_flag = PxiAlg_VP_GetAnomalyFlag();
		PxiAlg_VP_GetReport(&username_rpt);
			
		if ((VP_test_count>2000)||(username_rpt.timeout_flag == 1) )
		{
			// printf("Time Out!\n");
			if(!_state.vp_calibration_ready)
			{
				_state.vp_calibration_ready = 1 ;
				
			}
            
            //debug_printf("Heart Rate = %f \n", myHR_Out);
			//debug_printf("signal_quality = %f \n", signal_quality);
            debug_printf("Avg. Heart Rate = "LOG_FLOAT_MARKER" \n",LOG_FLOAT(myHR_Out));
            debug_printf("signal_quality = "LOG_FLOAT_MARKER" \n",LOG_FLOAT(signal_quality));
            debug_printf("ANO_flag = %d \n", ANO_flag);
			debug_printf("timeout_flag = %d \n", username_rpt.timeout_flag);
            
			//V117
			if((pah8011_VP_ALG_Mode == VP_Calibration)||(pah8011_VP_ALG_Mode == VP_Phy_Calibration))
			{
                regulate_ppgsp = username_rpt.regulate_ppgsp;   //Store in ROM
                regulate_ppgdp = username_rpt.regulate_ppgdp;   //Store in ROM

                  //ROM ex
//                addr = 0x7F00;  //Address set By Customer Platform Flash Area
//                flash_float_write( addr , username_rpt.regulate_ppgsp );
//                addr = 0x7F04;  //Address set By Customer Platform Flash Area
//                flash_float_write( addr , username_rpt.regulate_ppgdp );
//                addr = 0x7F08;  //Address set By Customer Platform Flash Area
//                flash_float_write( addr , true_sbp );
//                addr = 0x7F0C;  //Address set By Customer Platform Flash Area
//                flash_float_write( addr , true_dbp );
//                addr = 0x7F10;  //Address set By Customer Platform Flash Area
//                flash_float_write( addr , _state.vp_calibration_ready );

                if(pah8011_VP_ALG_Mode == VP_Phy_Calibration)
                {
                	//ROM ex
//                	flash_word_write( addr , gender );
//                	addr = 0x7F18;  //Address set By Customer Platform Flash Area
//                	flash_word_write( addr , age );
//                	addr = 0x7F1C;  //Address set By Customer Platform Flash Area
//                	flash_float_write( addr , height );
//                	addr = 0x7F20;  //Address set By Customer Platform Flash Area
//                	flash_float_write( addr , weight );
                }

    			//debug_printf("regulate_ppgsp = %f \n", regulate_ppgsp);
                //debug_printf("regulate_ppgdp = %f \n", regulate_ppgdp);
                debug_printf("regulate_ppgsp = "LOG_FLOAT_MARKER" \n",LOG_FLOAT(regulate_ppgsp));
                debug_printf("regulate_ppgdp = "LOG_FLOAT_MARKER" \n",LOG_FLOAT(regulate_ppgdp));
    			debug_printf("report_valid_flag = %d \n", username_rpt.report_valid_flag);		//v1.16
                
			}
			else if ((pah8011_VP_ALG_Mode == VP_Estimation)||(pah8011_VP_ALG_Mode == VP_Phy_Estimation)||(pah8011_VP_ALG_Mode == VP_WithoutCal_Estimation))
			{
				//debug_printf("estimate_ppgsp = %f \n", username_rpt.estimate_ppgsp);
				//debug_printf("estimate_ppgdp = %f \n", username_rpt.estimate_ppgdp);
                debug_printf("estimate_ppgsp = "LOG_FLOAT_MARKER" \n",LOG_FLOAT(username_rpt.estimate_ppgsp));
                debug_printf("estimate_ppgdp = "LOG_FLOAT_MARKER" \n",LOG_FLOAT(username_rpt.estimate_ppgdp));
    			debug_printf("report_valid_flag = %d \n", username_rpt.report_valid_flag);		//v1.16

			}
			
			if ((signal_quality < 0.5f) && (ANO_flag == 1))
			{
				BspHrmCalculateCompleteCb(0,0,0,0);
				debug_printf("Measurement may be disturbed by motion, it is recommended to keep static and measure again \n");
			}
			else
			{
				BspHrmCalculateCompleteCb((uint8_t)myHR_Out, (uint8_t)username_rpt.estimate_ppgsp,(uint8_t)username_rpt.estimate_ppgdp,(uint8_t)(signal_quality * 100));
			}

			vp_algorithm_close();		//set "_state.pxialg_has_init" = false
			stop_healthcare();
			pah8011_VP_ALG_Mode = VP_Suspend ;

			/*----------------------------------------for continue test----------------------------------------*/
//			test_count++;
//			if(test_count<3)
//			{
//				pah8series_ppg_dri_VP_Estimation_start();
				pah8series_ppg_dri_VP_Phy_Estimation_start();
//			}
			/*----------------------------------------for continue test----------------------------------------*/

			has_updated = true;
		}
	}
  
#endif // ENABLE_PXI_ALG_VP
    
    return has_updated;
}

static void vp_algorithm_close(void)
{
#if defined(ENABLE_PXI_ALG_VP)
    if (_state.pxialg_has_init)
    {
        _state.pxialg_has_init = false;
        
        PxiAlg_VP_Close();
        PxiAlg_Close();     //for static hr
    }
//    free(_state.pxialg_buffer);
    debug_printf("PxiAlg_VP_Close(), _state.pxialg_has_init = %d \n", _state.pxialg_has_init);
#endif // ENABLE_PXI_ALG_VP
}

#if defined(PPG_MODE_ONLY)|| defined(ALWAYS_TOUCH)
static void start_healthcare_ppg(void)
{
    // gsensor
    #ifndef ENABLE_MEMS_ZERO  
    accelerometer_start();
  #endif  
    // PAH
    if (!pah_enter_mode(pah_ppg_mode))
        Error_Handler();
    
    _state.last_report_time = get_tick_count();
}
#endif

static void start_healthcare_ppg_touch(void)
{
    // gsensor
#ifndef ENABLE_MEMS_ZERO 
    accelerometer_start();
#endif 
    // PAH
    if (!pah_enter_mode(pah_ppg_touch_mode))
        Error_Handler();
    
    _state.last_report_time = get_tick_count();
}

static void start_healthcare_touch_only(void)
{
    // gsensor
#ifndef ENABLE_MEMS_ZERO 
    accelerometer_stop();
#endif 
    
    // PAH
    if (!pah_enter_mode(pah_touch_mode))
        Error_Handler();
}

static void stop_healthcare(void)
{
    // gsensor
#ifndef ENABLE_MEMS_ZERO 
    accelerometer_stop();
#endif     
    // PAH
    if (!pah_enter_mode(pah_stop_mode))
        Error_Handler();
}

static void pah8011_ae_info_check(pah8series_data_t *pxialg_data, bool Tuning)
{
    uint8_t i;
    uint32_t MIN=0;
    float VAR_MAX=0;
    float AE_VAR=0;
    uint32_t *Expo_time  = _state.Expo_time;
    uint8_t  *LEDDAC     = _state.LEDDAC;
    uint16_t Touch_data  = _state.Touch_data;

    for( i = 0 ; i < 3 ; i++)
    {
        if(Expo_time_backup[i]>0)
        {
            MIN = (Expo_time_backup[i] >= Expo_time[i]) ? Expo_time[i] : Expo_time_backup[i];  
            AE_VAR = ((float)Expo_time_backup[i] - (float)Expo_time[i]);
            AE_VAR = ((AE_VAR >= 0.0f) ? AE_VAR : AE_VAR*(-1))/(float)MIN;
            VAR_MAX = (AE_VAR >= VAR_MAX) ? AE_VAR : VAR_MAX;					
        }
        Expo_time_backup[i] = Expo_time[i];	
    }
    for( i = 0 ; i < 3 ; i++)
    {
        if(LEDDAC_backup[i]>0)
        {
            MIN = (LEDDAC_backup[i] >= LEDDAC[i]) ? LEDDAC[i] : LEDDAC_backup[i];
            AE_VAR = ((float)LEDDAC_backup[i] - (float)LEDDAC[i]);
            AE_VAR = ((AE_VAR >= 0.0f) ? AE_VAR : AE_VAR*(-1))/(float)MIN;
            VAR_MAX = (AE_VAR >= VAR_MAX) ? AE_VAR : VAR_MAX;
        }
        LEDDAC_backup[i] = LEDDAC[i];
    }
    log_printf("INFO, %d, %d, %d, %d, %d, %d, ", Expo_time[0],Expo_time[1],Expo_time[2],LEDDAC[0],LEDDAC[1],LEDDAC[2]);
    log_printf("VAR " LOG_FLOAT_MARKER", Tuning %d, T_Data %d, \n", LOG_FLOAT(VAR_MAX),Tuning,Touch_data);
}

static bool vp_algorithm_process(pah8series_data_t *pxialg_data, uint32_t ch_num)
{
		// log header
    if (need_log_header)
    {
        need_log_header = false;
        log_pah8series_data_header(ch_num, PPG_IR_CH_NUM, ALG_GSENSOR_MODE);
    }
    pah8011_ae_info_check(&_state.pxialg_data, _state.Tuning); 
    // log
    log_pah8series_data(&_state.pxialg_data);   
    // vp
    if(_state.pxialg_data.touch_flag)
    {
        vp_algorithm_calculate(&_state.pxialg_data, ch_num );
    }		
    return true;
}



static void Error_Handler(void)
{
    log_printf("GOT ERROR !!! \n");
//    while (1)
//    {
//    }
}

