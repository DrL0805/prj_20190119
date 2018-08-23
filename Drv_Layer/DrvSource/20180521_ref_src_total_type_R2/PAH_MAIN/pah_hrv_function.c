// gsensor
//#include "accelerometer.h"

// platform support
//#include "system_clock.h"

//#include "debug_log.h"

// c
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "pah_comm.h"
#include "pah_driver_types.h"
#include "pah_driver.h"
#include "pah8series_config.h"

// platform support
#include "system_clock.h"
#include "gpio_ctrl.h"
// HRV algorithm
#include "pxialg_hrv.h"









/*============================================================================
STATIC VARIABLES
============================================================================*/
static bool need_log_header = true;
static main_state_s _state;
static pah_flags_s flags;

static uint32_t Expo_time_backup[3]={0};
static uint8_t LEDDAC_backup[3]={0};


/*============================================================================
STATIC VARIABLES---HRV
============================================================================*/

#define HRV_ENABLE_LHR            1
#define HRV_SAMPLING_RATE        20

typedef struct
{
    void            *pBuffer;
    float           collect_duration;
    uint8_t         has_got_sdnn;
} hrv_state_s;

static hrv_state_s _hrv_state;

static uint32_t  duration_total=0;

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
static void report_fifo_data_hrv(uint64_t timestamp, uint8_t *fifo_data, uint32_t fifo_data_num_per_ch, uint32_t ch_num, bool is_touched);
static bool hrv_algorithm_process(pah8series_data_t *pxialg_data, uint32_t ch_num);
static bool hrv_algorithm_calculate(pah8series_data_t *pxialg_data, uint32_t ch_num);
static void hrv_algorithm_close(void);
static uint8_t run_hrv_pxialg(uint32_t duration, const uint32_t* ppg_data, uint32_t ppg_count_per_ch, uint32_t ppg_ch_num, uint8_t touch_flag);


static void Error_Handler(void);

void pah8series_ppg_dri_HRV_init(void)
{
	memset(&_state, 0, sizeof(_state));
	// PAH
    {
       pah_flags_default(&flags);
		flags.stream = pah_stream_dri;
       flags.alg_mode = pah_alg_mode_HRV;
        if (!pah_init_with_flags(&flags))
        {
            debug_printf("pah_init_with_flags() fail. \n");
            Error_Handler();
        }
    }

}

void pah8series_ppg_polling_HRV_init(void)
{
	memset(&_state, 0, sizeof(_state));
	// PAH
    {
        pah_flags_default(&flags);
        flags.stream = pah_stream_polling;
        flags.alg_mode = pah_alg_mode_HRV;
        if (!pah_init_with_flags(&flags))
        {
            debug_printf("pah_init_with_flags() fail. \n");
            Error_Handler();
        }
    }
}
void pah8series_ppg_HRV_start(void)
{
    need_log_header=true;
#if defined(Touch_Detect_Green_Led)	
	Set_Touch_Detect_TH( 1024 ,1024) ;  
	//  (  Touch_Th_Hi ,  Touch_Th_Lo ) //customer can set Touch_th by Using Condition
#endif	
#if defined(PPG_MODE_ONLY)|| defined(ALWAYS_TOUCH)
    start_healthcare_ppg();
#else
    start_healthcare_touch_only();
#endif
    _state.status = main_status_start_healthcare;
	
}
void pah8series_ppg_HRV_stop(void)
{
// log
	log_pah8series_no_touch();
    _state.alg_status = alg_status_close ;
    _state.Tuning=0 ;
	stop_healthcare();
	_state.status = main_status_idle;
}


void pah8series_ppg_dri_HRV_task(volatile bool* has_interrupt_pah,volatile bool* has_interrupt_button ,volatile uint64_t*    interrupt_pah_timestamp)
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
                    #if defined(PPG_MODE_ONLY)|| defined(ALWAYS_TOUCH)
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
                            report_fifo_data_hrv(*interrupt_pah_timestamp, fifo_data, fifo_data_num_per_ch, fifo_ch_num, is_touched);
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
            if (*has_interrupt_button)
            {
							  debug_printf("\npah8series_ppg_dri_HRD_task...9\n");
                *has_interrupt_button = false;        
                pah8series_ppg_HRV_stop();
            }
            #endif						
        }
        else if (_state.status == main_status_idle)
        {
			  #if 0 // unused
            if (*has_interrupt_button)
            {
                *has_interrupt_button = false;
                pah8series_ppg_HRV_start();
            }
			   #endif
        }

}
void pah8series_ppg_polling_HRV_task(volatile bool* has_interrupt_pah,volatile bool* has_interrupt_button )
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
                            
                            report_fifo_data_hrv(timestamp, fifo_data, fifo_data_num_per_ch, fifo_ch_num, is_touched);
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
                pah8series_ppg_HRV_stop();
            }
						#endif 
        }
        else if (_state.status == main_status_idle)
        {
            #if 0 // unused					
            if (*has_interrupt_button)
            {
                *has_interrupt_button = false;   
                pah8series_ppg_HRV_start();
            }
						#endif
        }

}

void hrv_alg_task(void)
{
    switch(_state.alg_status)
	{
	case alg_status_process:
        {
	        hrv_algorithm_process(&_state.pxialg_data, _state.pxialg_data.nf_ppg_channel);
            _state.alg_status = alg_status_idle;
	    }
	    break;

	case alg_status_close:
        {
	        hrv_algorithm_close();
            _state.alg_status = alg_status_idle;
	    }
      break;
			
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

static void report_fifo_data_hrv(uint64_t timestamp, uint8_t *fifo_data, uint32_t fifo_data_num_per_ch, uint32_t ch_num, bool is_touched)
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

static bool hrv_algorithm_calculate(pah8series_data_t *pxialg_data, uint32_t ch_num)
{
    bool has_updated = false;
    
#if defined(ENABLE_PXI_ALG_HRV)
    uint8_t ret = 0;
    // Algorithm support only data length >= 10
    if (pxialg_data->nf_ppg_per_channel < 10)
    {
        debug_printf("hrv_algorithm_calculate(). fifo_data_num_per_ch = %d, is not enough to run algorithm \n", pxialg_data->nf_ppg_per_channel);
        return false;
    }
    // Initialize algorithm
    if (!_state.pxialg_has_init)
    {
        int buffer_size = 0;

		//memset(&_sdnn_state, 0, sizeof(_sdnn_state));
		memset(&_hrv_state, 0, sizeof(_hrv_state));
		debug_printf("HRVGetVersion() = %d \n", HRVGetVersion());

		buffer_size = HRVGetMemorySize(HRV_ENABLE_LHR, HRV_COLLECTION_SECONDS, HRV_SAMPLING_RATE);
		debug_printf("HRVGetMemorySize(), open_size = %d \n", buffer_size);
		if (!buffer_size)
		{
			debug_printf("HRVGetMemorySize(), failed to get memory size \n");
			return 0;
		}

		_hrv_state.pBuffer = (uint8_t*)malloc(buffer_size);
		if (!_hrv_state.pBuffer)
		{
			debug_printf("Out of memory \n");
			return 0;
		}
		
		if (!HRVOpen(_hrv_state.pBuffer))
		{
			debug_printf("HRVOpen() failed. \n");
			return 0;
		}
		duration_total=0;
    _state.pxialg_has_init = true;
    }
    
		// Calculate HRV
		ret = run_hrv_pxialg(pxialg_data->time, (const uint32_t*)pxialg_data->ppg_data, pxialg_data->nf_ppg_per_channel, pxialg_data->nf_ppg_channel, pxialg_data->touch_flag);
#endif // ENABLE_PXI_ALG_HRV
    return has_updated;
}

static uint8_t run_hrv_pxialg(uint32_t duration, const uint32_t* ppg_data, uint32_t ppg_count_per_ch, uint32_t ppg_ch_num, uint8_t touch_flag)
{
    uint32_t i = 0;
    uint32_t green1_index = 0, green2_index = 0;

    if (_hrv_state.has_got_sdnn)
        return 1;

    if (ppg_ch_num == 2)
    {
        green1_index = 0;
        green2_index = 1;
    }else if (ppg_ch_num == 1)
    {
        green1_index = 0;
        green2_index = 0;
    }else
    {
        debug_printf("Not support ppg_ch_num = %d \n", ppg_ch_num);
        return 0;
    }

    HRVSetDiffTime((float)duration, ppg_count_per_ch);
    for (i = 0; i < ppg_count_per_ch; ++i)
    {
        uint32_t green1 = ppg_data[i * ppg_ch_num + green1_index];
        uint32_t green2 = ppg_data[i * ppg_ch_num + green2_index];
        float raw_data = (float)green1 + green2;
        HRVProcess(touch_flag, raw_data);
    }
			
  	duration_total+=duration;
	  if((duration_total>25000)&&(!HRVGetReadyFlag()))
	  {
		    debug_printf("PPG Raw data is to Weak\r\n");	
		    debug_printf("TEST Stop\r\n");	
		    hrv_algorithm_close();
        stop_healthcare();
        _state.status = main_status_idle;
	  }	

    if (HRVGetReadyFlag())
    {
        _hrv_state.collect_duration += duration;
        debug_printf("collect_duration = %d \n", (int)_hrv_state.collect_duration);

        if (_hrv_state.collect_duration > (HRV_COLLECTION_SECONDS * 1000))
        {
            float SDNN = 0;
            int HRAvg = 0;
            //float fLF = 0.0f, fHF = 0.0f, fLHRatio = 0.0f;

            HRAvg = HRVGetHRAvg();
            
            SDNN = HRVGetSDNN();
            debug_printf("HRAvg = %d SDNN = %d \n", HRAvg , (int)SDNN);
			
			float tt=HRVGetRRIQuality();
			if(tt>0.4f)
			{
				debug_printf("HRV_Quality Good\n");
			}else
			{
				debug_printf("HRV_Quality Weak\n");
			}
			log_pah8series_no_touch();
      hrv_algorithm_close();
			stop_healthcare();
			_state.status = main_status_idle;
      _hrv_state.has_got_sdnn = 1;
        }
    }
	return 1;
}




static void hrv_algorithm_close(void)
{
#if defined(ENABLE_PXI_ALG_HRV)
    if (_state.pxialg_has_init)
    {
        _state.pxialg_has_init = false;
        HRVClose();
        free(_hrv_state.pBuffer);
			  _hrv_state.pBuffer = NULL;
    }	
#endif // ENABLE_PXI_ALG_HRV
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

static bool hrv_algorithm_process(pah8series_data_t *pxialg_data, uint32_t ch_num)
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
    // hrv
	  if(_state.pxialg_data.touch_flag)
	  {
		   hrv_algorithm_calculate(&_state.pxialg_data, ch_num);
	  }
		return true;
}

static void Error_Handler(void)
{
    log_printf("GOT ERROR !!! \n");
    while (1)
    {
    }
}

