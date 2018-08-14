#include "platform_common.h"
#include <string.h>
#include <stdlib.h>
// pah8011
#include "pah_driver.h"

// pah
#include "pah_driver_8011_reg.h"
#include "pah8011es_user.h"

//ALG file
#include "drv_hrm.h"

/*============================================================================
STATIC VARIABLES
============================================================================*/
main_state_s _state;

static volatile bool	    has_interrupt_close = false;//button
volatile bool	            has_interrupt_ready = false;//INT1(fifo data ready intterupt)
static volatile bool	    has_interrupt_touch = false;//INT2(touch intterupt)
static volatile bool 		initflag = false;
static volatile bool	    has_interrupt_pah = false;//true;//register fifo data ready intterupt
static volatile uint64    interrupt_pah_timestamp = 0;//register interrupt occure time
static volatile bool 		driverstate= true;// false while driver error


static bool need_log_header = true;
static uint32 Expo_time_backup[3]={0};
static uint8 LEDDAC_backup[3]={0};

static uint8 gSensorRange = ALG_GSENSOR_MODE;

//量程根据应用需要改变
void pah8011_gSensor_Range_Set(uint8 newrange)
{
    gSensorRange = newrange;
}

//中断方式驱动，当检测到fifo data ready(INT1 中断)时，调用此应用
void start_healthcare_ppg_touch(uint64 tickcount)//已更新
{
    // gsensor
	#ifndef ENABLE_MEMS_ZERO 
    accelerometer_start();
   #endif 
    // PAH
    if (!pah_enter_mode(pah_ppg_touch_mode))
        Error_Handler();
    
    //usertimer register then time, recommed to use a timer
    _state.last_report_time = tickcount;
}

void start_healthcare_touch_only(void)
{
    // gsensor
    accelerometer_stop();

    // PAH
    if (!pah_enter_mode(pah_touch_mode))
        Error_Handler();
}

void stop_healthcare(void)//添加328~3行
{
    // gsensor
    accelerometer_stop();
    
    // PAH
    if (!pah_enter_mode(pah_stop_mode))//in stop mode, set int2 as touch interrrut
        Error_Handler();
}
//已更新
uint8 report_fifo_data(uint64 timestamp, uint8 *fifo_data, uint32 fifo_data_num_per_ch, uint32 ch_num, bool is_touched)
{
     uint8 hreart_cal = 0;
	//uint8 hreart_cal = 0;
    // gsensor
#if defined(ENABLE_MEMS_ZERO)
	//keep belown code for algorithm use
    _state.pxialg_data.mems_data = _state.mems_data;//here offord ALG data
    _state.pxialg_data.nf_mems = fifo_data_num_per_ch;//???
#else
    accelerometer_get_fifo(&_state.pxialg_data.mems_data, &_state.pxialg_data.nf_mems);//get accel data
#endif  // ENABLE_MEMS_ZERO
        
    _state.pxialg_data.touch_flag = is_touched ? 1 : 0;
    _state.pxialg_data.time = (uint32)(timestamp - _state.last_report_time);
    _state.pxialg_data.ppg_data = (int32*)fifo_data;
    _state.pxialg_data.nf_ppg_channel = ch_num;
    _state.pxialg_data.nf_ppg_per_channel = fifo_data_num_per_ch;
    ++_state.pxialg_data.frame_count;
    
    _state.last_report_time = timestamp;//updata timstamp
    // adjust report period 
    pah_do_timing_tuning(_state.pxialg_data.time, _state.pxialg_data.nf_ppg_per_channel);//原厂加了ifdef的开关
    
    //添加了361~370
    // log header
    if (need_log_header)
    {
        need_log_header = false;
        
        log_pah8series_data_header(ch_num, PPG_IR_CH_NUM, gSensorRange);
    } 
    // log
    log_pah8series_data(&_state.pxialg_data);
        pah8011_ae_info_check();

    // hr, calculate heartbeat
    hr_algorithm_calculate(&_state.pxialg_data, ch_num, &hreart_cal);

    return hreart_cal;
}

uint8 hrmbuf[15000];//max lenght may up to 13KB
bool hr_algorithm_calculate(pah8series_data_t *pxialg_data, uint32 ch_num,uint8 *hr_cal)
{
    bool has_updated = false;
    
#if defined(ENABLE_PXI_ALG)
    uint8 ret = 0;
	static volatile uint8 rettemp;
    
    // Algorithm support only data length >= 10
    if (pxialg_data->nf_ppg_per_channel < 10)
    {
        return false;
    }
   
    // Initialize algorithm
    if (!_state.pxialg_has_init)
    {
        static volatile uint32 pxialg_open_size = 0;
    
        pxialg_open_size = pah8series_query_open_size();
        _state.pxialg_buffer = hrmbuf;//(void*)malloc(pxialg_open_size);//cause error
        //_state.pxialg_buffer = (void*)pvPortMalloc(pxialg_open_size);
        ret = pah8series_open(_state.pxialg_buffer);
        if (ret != MSG_SUCCESS)
        {
            return false;
        }
        
        pah8series_set_param(PAH8SERIES_PARAM_IDX_PPG_CH_NUM, (float)ch_num);
        pah8series_set_param(PAH8SERIES_PARAM_IDX_HAS_IR_CH, (float)PPG_IR_CH_NUM);
        pah8series_set_param(PAH8SERIES_PARAM_IDX_GSENSOR_MODE, (float)gSensorRange);

        _state.pxialg_has_init = true;
    }
   
    // Calculate Heart Rate
    ret = pah8series_entrance(pxialg_data);
    if (ret == MSG_HR_READY)
    {
        float hr = 0.0f;
        int hr_trust_level = 0;
        int16 grade = 0;
        
        pah8series_get_hr(&hr);
        pah8series_get_hr_trust_level(&hr_trust_level);
        pah8series_get_signal_grade(&grade);
        *hr_cal = (uint8)hr;
        //RTT_PRINTF(0,"\nhr = %d, hr_trust_level = %d, grade = %d \n\n", (int)hr, hr_trust_level, grade);
        has_updated = true;
    }
    else
    {
        switch (ret)
        {
            case MSG_SUCCESS:
                //RTT_PRINTF(0,"Algorithm entrance success. \n");
                break;
            case MSG_ALG_NOT_OPEN:
                //RTT_PRINTF(0,"Algorithm is not initialized. \n");
                break;
            case MSG_MEMS_LEN_TOO_SHORT:
                //RTT_PRINTF(0,"MEMS data length is shorter than PPG data length. \n");
                break;
            case MSG_NO_TOUCH:
                //RTT_PRINTF(0,"PPG is no touch. \n");
                break;
            case MSG_PPG_LEN_TOO_SHORT:
                //RTT_PRINTF(0,"PPG data length is too short. \n");
                break;
            case MSG_FRAME_LOSS:
                //RTT_PRINTF(0,"Frame count is not continuous. \n");
                break;
            default:
                //RTT_PRINTF(0,"Algorithm unhandle error = %d \n", ret);
                break;
        }
    }
      
#endif // ENABLE_PXI_ALG
    
    return has_updated;
}

void hr_algorithm_close(void)
{
#if defined(ENABLE_PXI_ALG)
    if (_state.pxialg_has_init)
    {
        _state.pxialg_has_init = false;
        
        pah8series_close();
        //free(_state.pxialg_buffer);//may cause error   //原厂的没屏蔽
    }
#endif // ENABLE_PXI_ALG
}


//void Error_Handler(void)
//{
//    // while (1)
//    // {
//    // }
//    driverstate = false;

//}

void Error_Handler(void)
{
    //RTT_PRINTF(0,"GOT ERROR !!! \n");
    while (1)
    {
    }
}


//for fifo data ready
void pah_8011_INT1_Handler(void)
{
    has_interrupt_ready = true;
}

//for touch
void pah_8011_INT2_Handler(void)
{
    has_interrupt_touch = true;
}

//for button or software int
void pah_8011_Close_Handler(void)
{
	has_interrupt_close = true;
}

void log_pah8series_data(const pah8series_data_t *pxialg_data)
{
    int i = 0;
    uint32 *ppg_data = (uint32 *)pxialg_data->ppg_data;
    int16 *mems_data = pxialg_data->mems_data;
    int data_num = pxialg_data->nf_ppg_channel * pxialg_data->nf_ppg_per_channel;
    
    //RTT_PRINTF(0,"Frame Count, %d \n", pxialg_data->frame_count);
    //RTT_PRINTF(0,"Time, %d \n", pxialg_data->time);
    //RTT_PRINTF(0,"PPG, %d, %d, ", pxialg_data->touch_flag, data_num);
    for (i = 0; i < data_num; ++i)
    {
        //RTT_PRINTF(0,"%d, ", *ppg_data);
        ppg_data++;
    }
    //RTT_PRINTF(0,"\n");
    //RTT_PRINTF(0,"MEMS, %d, ", pxialg_data->nf_mems);
    for (i = 0; i < pxialg_data->nf_mems * 3; ++i)
    {
        //RTT_PRINTF(0,"%d, ", *mems_data);
        mems_data++;
    }
    //RTT_PRINTF(0,"\n");
}

void log_pah8series_data_header(uint32 ch_num, uint32 ir_ch_num, uint32 g_sensor_mode)
{
    //log pah8series data header
    // (1)Using total channel numbers;
    // (2)reserved;
    // (3)reserved;
    // (4)IR channel number;
    // (5)MEMS mode 0:2G, 1:4G, 2:8G
    //RTT_PRINTF(0,"PPG CH#, %d, %d, %d, %d, %d\n", ch_num, pah8011_setting_version(), 0, ir_ch_num, g_sensor_mode);
}

void pah8011_ae_info_check(void)
{
    uint8 i;
    float VAR_MAX=0;
    float AE_VAR=0;
    uint32 Expo_time[3]={0};
    uint8 LEDDAC[3];
    pah8011_ae_info_read( Expo_time, LEDDAC);
    for( i = 0 ; i < 3 ; i++)
    {
        if(Expo_time_backup[i]>0)
        {
            AE_VAR= ((float)Expo_time[i]-(float)Expo_time_backup[i])/(float)Expo_time_backup[i];
            AE_VAR = (AE_VAR >= 0.0f) ? AE_VAR : AE_VAR*(-1.0f); 
            VAR_MAX = (AE_VAR >= VAR_MAX) ? AE_VAR : VAR_MAX;
        }
        Expo_time_backup[i] = Expo_time[i] ;
    }
    for( i = 0 ; i < 3 ; i++)
    {
        if(LEDDAC_backup[i]>0)
        {
            AE_VAR= ((float)LEDDAC[i]-(float)LEDDAC_backup[i])/(float)LEDDAC_backup[i];
            AE_VAR = (AE_VAR >= 0.0f) ? AE_VAR : AE_VAR*(-1.0f); 
            VAR_MAX = (AE_VAR >= VAR_MAX) ? AE_VAR : VAR_MAX; 
        }
        LEDDAC_backup[i] = LEDDAC[i];
    }
    
    //RTT_PRINTF(0,"INFO, %d, %d, %d, %d, %d, %d ,VAR, %f \n", Expo_time[0],Expo_time[1],Expo_time[2],LEDDAC[0],LEDDAC[1],LEDDAC[2],VAR_MAX);
    
}

