#ifndef DRV_RGB_LCD_H
#define DRV_RGB_LCD_H

#include "platform_common.h"
#include "io_config.h"

#define RGB_LCD_PIXEL_WIDTH		(176)
#define RGB_LCD_PIXEL_HIGHT		(176)
#define RGB_LCD_RAM_LEN			(15488)  //(RGB_LCD_PIXEL_WIDTH * RGB_LCD_PIXEL_HIGHT / 2)


// 上电使能，低电平使能
#define RGB_LCD_EN_PIN_SET()			//am_hal_gpio_out_bit_set(RGB_LCD_EN_PIN)
#define RGB_LCD_EN_PIN_CLEAR()			//am_hal_gpio_out_bit_clear(RGB_LCD_EN_PIN)
										//
// 时钟脚                               //
#define RGB_LCD_SCLK_PIN_SET()			//am_hal_gpio_out_bit_set(RGB_LCD_SCLK_PIN)
#define RGB_LCD_SCLK_PIN_CLEAR()		//am_hal_gpio_out_bit_clear(RGB_LCD_SCLK_PIN)
										//
// 数据脚                               //
#define RGB_LCD_SI_PIN_SET()			//am_hal_gpio_out_bit_set(RGB_LCD_SI_PIN)
#define RGB_LCD_SI_PIN_CLEAR()			//am_hal_gpio_out_bit_clear(RGB_LCD_SI_PIN)
										//
// 片选脚，高电平片选                   //
#define RGB_LCD_SCS_PIN_SET()			//am_hal_gpio_out_bit_set(RGB_LCD_SCS_PIN)
#define RGB_LCD_SCS_PIN_CLEAR()			//am_hal_gpio_out_bit_clear(RGB_LCD_SCS_PIN)
										//
// 信号脉冲脚，最低脉冲2us              //
#define RGB_LCD_EXTCOMIN_PIN_SET()		//am_hal_gpio_out_bit_set(RGB_LCD_EXTCOMIN_PIN)
#define RGB_LCD_EXTCOMIN_PIN_CLEAR()	//am_hal_gpio_out_bit_clear(RGB_LCD_EXTCOMIN_PIN)
										//
// 显示使能脚，高电平使能               //
#define RGB_LCD_DISP_PIN_SET()			//am_hal_gpio_out_bit_set(RGB_LCD_DISP_PIN)
#define RGB_LCD_DISP_PIN_CLEAR()		//am_hal_gpio_out_bit_clear(RGB_LCD_DISP_PIN)

// 背光开关脚
#define RGB_LCD_BCKLED_PIN_SET()		//am_hal_gpio_out_bit_set(RGB_LCD_BCKLED_PIN)
#define RGB_LCD_BCKLED_PIN_CLEAR()		//am_hal_gpio_out_bit_clear(RGB_LCD_BCKLED_PIN)

extern void Drv_RGBLcd_Inversion(void);
extern uint32_t Drv_RGBLcd_Init(void);
extern void Drv_RGBLcd_RefreshAll(uint8_t* DotBuf);
extern uint32_t Drv_RGBLcd_Test(void);

#endif

