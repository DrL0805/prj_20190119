
// c
#include <string.h>
#include <stdlib.h>
// pah
#include "pah_verify.h"
#include "pah_comm.h"
#include "pah_driver_8011_reg.h"

#include "pah_factory_test.h"


/*============================================================================
STATIC FUNCTION PROTOTYPES
============================================================================*/
static bool demo_factory_test(factory_test_e factory_test,bool expo_en,uint8 expo_ch_b,uint8 expo_ch_c);
static void demo_factory_test_print_adc_data(const pah_verify_adc_data_s *adc_data);
static void demo_factory_test_print_adc_data_verify_failed(const pah_verify_adc_data_s *adc_data);
void factory_test_mode(factory_test_e test_type,bool expo_en,uint8 expo_ch_b,uint8 expo_ch_c);
//static void gpio_btn_interrupt_handler(GPIO_BTN gpio);
static void Error_Handler(void);

//static void demo_factory_mode(void)
//{
////    factory_test_mode(factory_test_led_golden_only,1,0x42,0x42);
////    factory_test_mode(factory_test_led_target_sample,0,0,0);
//    factory_test_mode(factory_test_light_leak,0,0,0);   
//}
/**************      APP层调用      ****************/
void HrmFactoryTest(uint16 ui16lightleak[3])
{
    pah_verify_adc_data_s adc_data;
    pah_ret  ret = pah_err_unknown;

    ret = pah_verify_init();
    if (PAH_FAILED(ret))
        Error_Handler();

    ret = pah_verify_light_leak_read_adc_data(&adc_data);
    
    if (PAH_FAILED(ret))
        Error_Handler();
    
    ui16lightleak[0] = adc_data.ch_adc_data[0].value;
	ui16lightleak[1] = adc_data.ch_adc_data[2].value;
	ui16lightleak[2] = adc_data.ch_adc_data[3].value;

//    //打印测量值
//    demo_factory_test_print_adc_data(adc_data);   
    
//    //验证测量值范围
//    ret = pah_verify_adc_data_test(adc_data);
				
//	if (PAH_FAILED(ret))
//    {
//        demo_factory_test_print_adc_data_verify_failed(adc_data);
//        //return false;
//    }    
    //return true;    
}

//pah_verify_adc_data_s adc_data;

//HrmFactoryTest(&adc_data);



void factory_test_mode(factory_test_e test_type,bool expo_en,uint8 expo_ch_b,uint8 expo_ch_c)
{    
      
    demo_factory_test((factory_test_e)test_type,expo_en,expo_ch_b,expo_ch_c);

}

static volatile bool 		factoryinitflag = false;

static bool demo_factory_test(factory_test_e factory_test,bool expo_en,uint8 expo_ch_b,uint8 expo_ch_c)
{
//    //OLED显示代码用：
//    static sPOS pos_1n ={6,9};
//	static sPOS pos_2n ={16,9};
//	static sPOS pos_3n ={26,9};
//	static sPOS pos_4n ={36,9};
//	static sPOS pos_5n ={46,9};
//	static sPOS pos_6n ={56,9};
//	static sPOS pos_7n ={66,9};
//	static sPOS pos_8n ={76,9};
////	static sPOS pos_9n ={86,9};
//    unsigned char oledbuff[8];
    
    pah_ret                 ret = pah_err_unknown;
    pah_verify_adc_data_s   adc_data;

    //RTT_PRINTF(0,"\n\n");
    
//    if(!factoryinitflag)
//    {
//        factoryinitflag = true;
//        ret = pah_verify_init();
//        if (PAH_FAILED(ret))
//            Error_Handler();
//    }
    
    ret = pah_verify_init();
    if (PAH_FAILED(ret))
        Error_Handler();
    
		pah_verify_expo_time_s  expo_time_data;
		expo_time_data.expo_time_enabled_verify = expo_en ;
		expo_time_data.ch_b_expo_time = expo_ch_b;
		expo_time_data.ch_c_expo_time = expo_ch_c;
    switch (factory_test)
    {
		case factory_test_light_leak:
        //RTT_PRINTF(0,"===== Factory test: Light Leak ===== \n");
        ret = pah_verify_light_leak_read_adc_data(&adc_data);
        
//        //我的OLED显示代码：
//        oled_pic_erase((sRECTANGLE){0,0,96,32});
//        Hex_to_dec(adc_data.ch_adc_data[0].value,oledbuff);
//        oled_pic_draw(&letters[oledbuff[4] + '0' - ' '], (sPOS*)&pos_1n, Replace);
//        oled_pic_draw(&letters[oledbuff[5] + '0' - ' '], (sPOS*)&pos_2n, Replace);
//		oled_pic_draw(&letters['-' - ' '], (sPOS*)&pos_3n, Replace);

//        Hex_to_dec(adc_data.ch_adc_data[2].value,oledbuff);
//        oled_pic_draw(&letters[oledbuff[4] + '0' - ' '], (sPOS*)&pos_4n, Replace);
//        oled_pic_draw(&letters[oledbuff[5] + '0' - ' '], (sPOS*)&pos_5n, Replace);
//		oled_pic_draw(&letters['-' - ' '], (sPOS*)&pos_6n, Replace);

//        Hex_to_dec(adc_data.ch_adc_data[3].value,oledbuff);
//        oled_pic_draw(&letters[oledbuff[4] + '0' - ' '], (sPOS*)&pos_7n, Replace);
//        oled_pic_draw(&letters[oledbuff[5] + '0' - ' '], (sPOS*)&pos_8n, Replace);
//        oled_update(0,96);
        
        if (PAH_FAILED(ret))
            Error_Handler();
        break;
#ifdef FACTORY_TEST_ES    
    case factory_test_green_led_golden_only:
        //RTT_PRINTF(0,"===== Factory test: Green LED golden only ===== \n");
        ret = pah_verify_green_led_golden_only_read_adc_data(&adc_data,&expo_time_data);
        if (PAH_FAILED(ret))
            Error_Handler();
        break;
    case factory_test_ir_led_golden_only:
        //RTT_PRINTF(0,"===== Factory test: IR LED golden only ===== \n");
        ret = pah_verify_ir_led_golden_only_read_adc_data(&adc_data);
        if (PAH_FAILED(ret))
            Error_Handler();
        break;
    case factory_test_ir_led_target_sample:
        ////RTT_PRINTF(0,"===== Factory test: IR LED target sample ===== \n");
        ret = pah_verify_ir_led_target_sample_read_adc_data(&adc_data);
        if (PAH_FAILED(ret))
            Error_Handler();
        break;
    case factory_test_green_led_target_sample:
//        RTT_PRINTF(0,"===== Factory test: Green LED target sample ===== \n");
        ret = pah_verify_green_led_target_sample_read_adc_data(&adc_data);
        if (PAH_FAILED(ret))
            Error_Handler();
        break;
		case factory_test_led_target_sample:
//        RTT_PRINTF(0,"===== Factory test: LED target sample ===== \n");
        ret = pah_verify_led_target_sample_read_adc_data(&adc_data);
        if (PAH_FAILED(ret))
            Error_Handler();
        break;
    case factory_test_led_golden_only:
//        RTT_PRINTF(0,"===== Factory test: LED golden only ===== \n");
		
        ret = pah_verify_led_golden_only_read_adc_data(&adc_data,&expo_time_data);
        if (PAH_FAILED(ret))
            Error_Handler();
        break;
#endif		
#ifdef FACTORY_TEST_ET				
		case factory_test_led_target_sample:
//        RTT_PRINTF(0,"===== Factory test: LED target sample ===== \n");
        ret = pah_verify_led_target_sample_read_adc_data(&adc_data);
        if (PAH_FAILED(ret))
            Error_Handler();
        break;
    case factory_test_led_golden_only:
//        RTT_PRINTF(0,"===== Factory test: LED golden only ===== \n");
		
        ret = pah_verify_led_golden_only_read_adc_data(&adc_data,&expo_time_data);
        if (PAH_FAILED(ret))
            Error_Handler();
        break;
#endif				
    default:
//        RTT_PRINTF(0,"Not implemented yet \n");
        return false;
    }
    
    //打印测量值
    demo_factory_test_print_adc_data(&adc_data);   
    
    //验证测量值范围
    ret = pah_verify_adc_data_test(&adc_data);
		
		
	if (PAH_FAILED(ret))
    {
        demo_factory_test_print_adc_data_verify_failed(&adc_data);
        return false;
    }
    
    return true;
}

static void demo_factory_test_print_adc_data(const pah_verify_adc_data_s *adc_data)
{
    int ch = 0;
    
    if (!adc_data)
        Error_Handler();
    
    for (ch = 0; ch < pah_verify_ch_num; ++ch)
    {
        if (adc_data->ch_adc_data[ch].min_enabled_verify && adc_data->ch_adc_data[ch].max_enabled_verify)
        {
//            RTT_PRINTF(0,"factory test. ch = %d, (min, value, max) = (%d, %d, %d) \n",
//                ch,
//                adc_data->ch_adc_data[ch].min,
//                adc_data->ch_adc_data[ch].value,
//                adc_data->ch_adc_data[ch].max);
        }
        else if (adc_data->ch_adc_data[ch].min_enabled_verify)
        {
           // RTT_PRINTF(0,"factory test. ch = %d, (min, value) = (%d, %d) \n",
//                ch,
//                adc_data->ch_adc_data[ch].min,
//                adc_data->ch_adc_data[ch].value);
        }
        else if (adc_data->ch_adc_data[ch].max_enabled_verify)
        {
            //RTT_PRINTF(0,"factory test. ch = %d, (value, max) = (%d, %d) \n",
//                ch,
//                adc_data->ch_adc_data[ch].value,
//                adc_data->ch_adc_data[ch].max);
        }
    }
}

static void demo_factory_test_print_adc_data_verify_failed(const pah_verify_adc_data_s *adc_data)
{
    int ch = 0;
    
    if (!adc_data)
        Error_Handler();
    
    for (ch = 0; ch < pah_verify_ch_num; ++ch)
    {
        // min
        if (adc_data->ch_adc_data[ch].min_enabled_verify && !adc_data->ch_adc_data[ch].min_verified_success)
		{
            //RTT_PRINTF(0,"factory test failed. ch = %d, value = %d, min = %d \n", ch, adc_data->ch_adc_data[ch].value, adc_data->ch_adc_data[ch].min);
        }
        // max
        if (adc_data->ch_adc_data[ch].max_enabled_verify && !adc_data->ch_adc_data[ch].max_verified_success)
		{
			 //RTT_PRINTF(0,"factory test failed. ch = %d, value = %d, max = %d \n", ch, adc_data->ch_adc_data[ch].value, adc_data->ch_adc_data[ch].max);
		}
           
    }
}

static void Error_Handler(void)
{
    while (1)
    {
    }
}

