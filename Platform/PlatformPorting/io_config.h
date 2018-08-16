#ifndef __MCU_IO_H
#define __MCU_IO_H

/**********************************************************************
**
**模块说明: H003 Apollo2 IO口功能重定义
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.6.22  初版  ZSL  
**
**********************************************************************/
#define IO_UNKNOW  0XFF

#define KEY_MODULE
#define COMMON_MODULE
#define IIC_MODULE

//GPIO
#ifdef SWD_MODULE
#define SWD_CK_PIN      20    //SWD debug pin
#define SWD_IO_PIN      21
#endif

#ifdef MOVT_MODULE
#define MOVT_M_A_PIN    IO_UNKNOW  //表针
#define MOVT_M_B_PIN    IO_UNKNOW
#define MOVT_3_A_PIN    IO_UNKNOW
#define MOVT_3_B_PIN    IO_UNKNOW
#define MOVT_6_A_PIN    IO_UNKNOW
#define MOVT_6_B_PIN    IO_UNKNOW
#define MOVT_9_A_PIN    IO_UNKNOW
#define MOVT_9_B_PIN    IO_UNKNOW
#endif

#ifdef HEART_RATE
#define HR_INT1_PIN     38    //心率
#define HR_EN_PIN       1    //enable or power
#define HR_INT2_PIN     0
#endif

#ifdef OP_MODULE
#define OP_TEST         17
#define OP_EN           21
#define OP_OUTPUT       32
#endif

#ifdef TEM_MODULE       //体温
#define TEM_VDD         36
#define TEM_TEST        35
#endif

#ifdef BAT_MODULE
#define BAT_CHG_PIN     16    //BAT
#define BAT_PG_PIN      17
#define BAT_INT_PIN     29
#endif

#ifdef KEY_MODULE
#define KEY_S0_PIN      3    //按键	//16//
#define KEY_S1_PIN      IO_UNKNOW
#define KEY_S2_PIN      IO_UNKNOW
#endif

#ifdef BLE_MODULE
#define BLE_EN_PIN      IO_UNKNOW    //BLE
#define BLE_CS_PIN      IO_UNKNOW
#define BLE_RDY_PIN     IO_UNKNOW
#define BLE_32KHz       IO_UNKNOW
#endif

#if 0//def BLE_MODULE
#define BLE_EN_PIN      3    //BLE
#define BLE_CS_PIN      45
#define BLE_RDY_PIN     4
#define BLE_32KHz       IO_UNKNOW
#endif

#ifdef LCD_MODULE
#define LCD_EXIN_PIN    13    //LCD
#define LCD_DISP_PIN    19
#define LCD_SCS_PIN     26
#define LCD_EN_PIN      31
#define LCD_SI_PIN      32
#define LCD_SCLK_PIN    33
#endif

#ifdef TOUCH_MODULE
#define TOUCH_RST_PIN   29    //触摸屏
#define TOUCH_INT_PIN   30
#define TOUCH_EN_PIN    18
#endif

#ifdef OLED_MODULE		  //OLED   
#define OLED_CLK_PIN    IO_UNKNOW   
#define OLED_DI_PIN     IO_UNKNOW
#define OLED_CS_PIN     IO_UNKNOW
#define OLED_CD_PIN     IO_UNKNOW   
#define OLED_EN_PIN     IO_UNKNOW
#define OLED_RST_PIN    IO_UNKNOW
#endif

#ifdef GPS_MODULE
#define GPS_EN_PIN      46    //GPS
#endif

#ifdef AIRPRESS_MODULE
#define AIRPRS_CSB      IO_UNKNOW    //气压
#endif

#ifdef COMMON_MODULE          //通用模块
#define MOTOR_PIN       4     //马达
#define LED_G_PIN       10//IO_UNKNOW    //绿灯
#define LED_R_PIN       IO_UNKNOW    //红灯
#define BCKLED_PIN      47    //背光
#endif

#ifdef FLASH_MODULE
#define FLASH_CS_PIN    37    //4G flash 片选
#endif

#ifdef FONT_MODULE
#define FONT_CS_PIN     28    //字库片选 GT
#endif

//定义ADC
#ifdef ADC_MODULE
#define ADC_PIN       IO_UNKNOW
#endif

//UART 需要在使用到uart的文件中定义宏 UART_MODULE
#ifdef  UART_MODULE
#define UART0_TX    22    //UART0
#define UART0_RX    23
#define UART1_TX    14           //UART1 for GPS
#define UART1_RX    15
#endif

//SPI 需要在使用到SPI的文件中定义宏 SPI_MODULE
#ifdef SPI_MODULE
#define SPI0_SCK    5    //SPI0 for font
#define SPI0_MISO   6
#define SPI0_MOSI   7

#define SPI1_SCK    IO_UNKNOW    //SPI1
#define SPI1_MISO   IO_UNKNOW
#define SPI1_MOSI   IO_UNKNOW

#define SPI2_SCK    IO_UNKNOW    //SPI2
#define SPI2_MOSI   IO_UNKNOW
#define SPI2_MISO   IO_UNKNOW

#define SPI3_SCK    IO_UNKNOW    //SPI3
#define SPI3_MISO   IO_UNKNOW
#define SPI3_MOSI   IO_UNKNOW

#define SPI4_SCK    IO_UNKNOW    //SPI4
#define SPI4_MISO   IO_UNKNOW
#define SPI4_MOSI   IO_UNKNOW

#define SPI5_SCK    IO_UNKNOW    //SPI5
#define SPI5_MISO   IO_UNKNOW
#define SPI5_MOSI   IO_UNKNOW
#endif

#define LCD_SI_PIN      32
#define LCD_SCLK_PIN    33

#ifdef SWSPI_MODULE
#define SW_SPI_CLK    33          //LCD_SCLK_PIN
#define SW_SPI_MOSI   32          //LCD_SI_PIN      
#define SW_SPI_MISO   IO_UNKNOW
#endif

//IIC 需要在使用到IIC的文件中定义宏 IIC_MODULE
#ifdef IIC_MODULE
#define IIC0_SCL    IO_UNKNOW    //IIC0
#define IIC0_SDA    IO_UNKNOW
#define IIC1_SCL    8    //IIC1 for mag
#define IIC1_SDA    9
#define IIC2_SCL    27    //IIC2  for BMI
#define IIC2_SDA    25
#define IIC3_SCL    42    //IIC3
#define IIC3_SDA    43 
#define IIC4_SCL    39    //IIC4  for touch
#define IIC4_SDA    40
#define IIC5_SCL    48    //IIC5 for touch
#define IIC5_SDA    49
#endif

//PDM  需要在使用到PDM的文件中定义宏 PDM_MODULE
#ifdef PDM_MODULE
#define  PDM_DATA    11
#define  PDM_CLK     12
#endif

#endif

