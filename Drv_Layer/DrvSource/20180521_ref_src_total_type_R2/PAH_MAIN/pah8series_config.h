#ifndef __PAH8SERIES_CONFIG_C_H__
#define __PAH8SERIES_CONFIG_C_H__

#include <stdint.h>
#include "pah_platform_functions.h"

/*============================================================================
SOME DEFINITION
============================================================================*/
#define PPG_IR_CH_NUM           0

//#define ENABLE_MEMS_ZERO
#define MAX_MEMS_SAMPLE_NUM 	100

#define ENABLE_PXI_ALG_HRD
#define ENABLE_PXI_LINK_HRD_REG
#define _WEAR_INDEX_EN					

//#define ENABLE_PXI_ALG_VP
//#define ENABLE_PXI_LINK_VP_REG

//#define ENABLE_PXI_ALG_HRV
//#define ENABLE_PXI_LINK_HRV_REG
//#define HRV_COLLECTION_SECONDS  180

//#define ENABLE_PXI_ALG_RR
//#define ENABLE_PXI_LINK_RR_REG


#define ALG_GSENSOR_MODE		1   // 0:2G, 1:4G, 2:8G, 3:16G


//******Always Run PPG Mode******//
//#define PPG_MODE_ONLY

//******Lock sensor report Rate to target******//
#define Timing_Tuning

//******Without IR LED ,Use Green to Detect Touch******//
//#define Touch_Detect_Green_Led


/*============================================================================
VALUE MACRO DEFINITIONS
============================================================================*/
#define PAH_DRIVER_8011_VERSION    1009

#define DEFAULT_REPORT_SAMPLE_NUM_PER_CH      20

#define MAX_SAMPLES_PER_READ    150
#define MAX_CH_NUM              3
#define BYTES_PER_SAMPLE        4

#define PPG_FRAME_INTERVAL_MS   60      // 50 * 120%

#define I2C_SLAVE_ID  0x15   //I2C 7-bit ID

/*============================================================================
OPTION MACRO DEFINITIONS
============================================================================*/
#define ENABLE_FIFO_CHECKSUM

/*============================================================================
Sensor & Cover Type DEFINITION
============================================================================*/

//-------Sensor IC Type---------//
#define __PAH8011ES
//#define __PAH8011ET

//-------Cover Type---------//
#define _DI_WALL_COVER
//#define _DI_COVER
//#define _PC_COVER
//#define _DI_5MM_COVER
//#define _PC_5MM_COVER


//-------IO Interface---------//
#define _I2C
//#define _SPI

//-------WITH/WO RUBBER---------//
#define _WO_RUBBER
//#define _WITH_RUBBER

//-------WITH/WO NOTCH---------//
#define _WO_NOTCH
//#define _WITH_NOTCH


#if defined( __PAH8011ES) 
    #define FACTORY_TEST_ES
    #if defined( _DI_COVER) 
        #define __PAH8011ES_DI_COVER
        #define SETTING_VERSION 0x01
    #endif
    #if defined( _DI_WALL_COVER)
        #define __PAH8011ES_DI_WALL_COVER
        #define SETTING_VERSION 0x11
    #endif
    #if defined( _DI_5MM_COVER)
        #define __PAH8011ES_DI_5MM_COVER
        #define SETTING_VERSION 0x18
    #endif
    #if defined( _PC_5MM_COVER)
        #define __PAH8011ES_PC_5MM_COVER
        #define SETTING_VERSION 0x1C
    #endif
#endif

#if defined( __PAH8011ET)
	#define FACTORY_TEST_ET
    #if defined( _DI_WALL_COVER) 
        #if defined( _WO_RUBBER)
            #if defined( _WITH_NOTCH)
                #define FACTORY_TEST_ET_WITH_NOTCH
                #if defined( _I2C)
                
                    #define __PAH8011ET_DI_WALL_WITH_NOTCH_SETTING_I2C
                    #define SETTING_VERSION 0x21
                #endif    
                #if defined( _SPI)
                    
                    #define __PAH8011ET_DI_WALL_WITH_NOTCH_SETTING_SPI
                    #define SETTING_VERSION 0x31
                #endif
            #endif
            #if defined( _WO_NOTCH)
                #define FACTORY_TEST_ET_WO_NOTCH
                #if defined( _I2C)
                    
                    #define __PAH8011ET_DI_WALL_WO_NOTCH_SETTING_I2C
                    #define SETTING_VERSION 0x22
                    #define ALWAYS_TOUCH
                #endif    
                #if defined( _SPI)
                    
                    #define __PAH8011ET_DI_WALL_WO_NOTCH_SETTING_SPI
                    #define SETTING_VERSION 0x32
                    #define ALWAYS_TOUCH
                #endif
        
            #endif
        #endif
        #if defined( _WITH_RUBBER)
            #if defined( _WITH_NOTCH)
                #define FACTORY_TEST_ET_WITH_NOTCH
                #if defined( _I2C)
                
                    #define __PAH8011ET_DI_WALL_RUBBER_WITH_NOTCH_SETTING_I2C
                    #define SETTING_VERSION 0x23
                #endif    
                #if defined( _SPI)
                    
                    #define __PAH8011ET_DI_WALL_RUBBER_WITH_NOTCH_SETTING_SPI
                    #define SETTING_VERSION 0x33
                #endif
            #endif
            #if defined( _WO_NOTCH)
                #define FACTORY_TEST_ET_WO_NOTCH
                #if defined( _I2C)
                    
                    #define __PAH8011ET_DI_WALL_RUBBER_WO_NOTCH_SETTING_I2C
                    #define SETTING_VERSION 0x24
                    #define ALWAYS_TOUCH
                #endif    
                #if defined( _SPI)
                    
                    #define __PAH8011ET_DI_WALL_RUBBER_WO_NOTCH_SETTING_SPI
                    #define SETTING_VERSION 0x34
                    #define ALWAYS_TOUCH
                #endif
        
            #endif
        
        #endif
    #endif    
    #if defined( _PC_COVER)
        #define FACTORY_TEST_ET_WITH_NOTCH
        #if defined( _WO_RUBBER)
            
            #if defined( _I2C)
            
                #define __PAH8011ET_PC_SETTING_I2C
                #define SETTING_VERSION 0x41
            #endif    
            #if defined( _SPI)
                
                #define __PAH8011ET_PC_SETTING_SPI
                #define SETTING_VERSION 0x51
            #endif

        #endif
        #if defined( _WITH_RUBBER)
            #if defined( _I2C)
            
                #define __PAH8011ET_PC_RUBBER_SETTING_I2C
                #define SETTING_VERSION 0x42
            #endif    
            #if defined( _SPI)
                
                #define __PAH8011ET_PC_RUBBER_SETTING_SPI
                #define SETTING_VERSION 0x52
            #endif
        
        #endif
    #endif



#endif 

#if defined(ENABLE_PXI_ALG_HRD) 
		#if defined(_WEAR_INDEX_EN) 
			#define WEAR_INDEX_EN
		#endif
#endif


#endif // __PAH8SERIES_CONFIG_C_H__
