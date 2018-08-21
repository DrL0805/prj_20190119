#ifndef		_FONT_H
#define		_FONT_H

#include "platform_common.h"

typedef enum
{
    FONT_WAKEUP_CMD,  //ÏÂwake upÃüÁî
    FONT_SLEEP_CMD,   //ÏÂsleep ÃüÁî
}font_cmd;

typedef enum 
{
	ASCII_SIZE_5X7 	    = 1,
	ASCII_SIZE_7X8 	    = 2,
	ASCII_SIZE_6X12 	= 3,
	ASCII_SIZE_12_B_A 	= 4,
	ASCII_SIZE_12_B_T 	= 5,
	ASCII_SIZE_8X16 	= 6,
	ASCII_SIZE_16_A 	= 7,
	ASCII_SIZE_16_T 	= 8,
	ASCII_SIZE_12X24 	= 9,
	ASCII_SIZE_24_B 	= 10,
	ASCII_SIZE_16X32 	= 11,
	ASCII_SIZE_32_B 	= 12,
	GB_SIZE_16X16 	    = 13,
	GB_SIZE_24X24 	    = 14,
}Wordstock_size_s;  


extern uint8 Drv_Font_Open(void);
extern uint8 Drv_Font_Close(void);
extern uint8 Drv_Font_ReadGB2313(uint16 GB, uint8 gb_size,uint8 *pdata);	// ¶ÁÈ¡GBK±àÂë
extern uint16 Drv_Font_Unicode2GB(uint16 unicode);							// unicode±àÂë×ªGBK±àÂë
extern uint8 Drv_Font_ReadASCII(uint8 ascii, uint8 ascii_size, uint8 *pdata);	// ¶ÁÈ¡ASCII±àÂë
extern uint8 Drv_Font_ReadUI(uint8 uisequence, uint8 *pdata);
extern uint8 Drv_Font_SendCmd(font_cmd ft_cmd);
extern uint8 Drv_Font_SelfTest(void);
extern uint8 Drv_Font_Init(void);

#endif







