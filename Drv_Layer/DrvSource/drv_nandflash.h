#ifndef	_DRV_NANDFLASH_H
#define	_DRV_NANDFLASH_H

#include "platform_common.h"

#define DRV_NANDFLASH_RTT_DEBUG	3
#if (1 == DRV_NANDFLASH_RTT_DEBUG)	// 错误等级
#define DRV_NANDFLASH_RTT_LOG(...)
#define DRV_NANDFLASH_RTT_WARN(...)
#define DRV_NANDFLASH_RTT_ERR		SEGGER_RTT_printf
#elif (2 == DRV_NANDFLASH_RTT_DEBUG)	// 警告等级
#define DRV_NANDFLASH_RTT_LOG(...)
#define DRV_NANDFLASH_RTT_WARN	SEGGER_RTT_printf
#define DRV_NANDFLASH_RTT_ERR		SEGGER_RTT_printf
#elif (3 == DRV_NANDFLASH_RTT_DEBUG)	// 调试等级
#define DRV_NANDFLASH_RTT_LOG		SEGGER_RTT_printf
#define DRV_NANDFLASH_RTT_WARN	SEGGER_RTT_printf
#define DRV_NANDFLASH_RTT_ERR		SEGGER_RTT_printf
#else							// 调试关闭
#define DRV_NANDFLASH_RTT_LOG(...)
#define DRV_NANDFLASH_RTT_WARN(...)
#define DRV_NANDFLASH_RTT_ERR(...)
#endif		  

#define PHY_BYTE_OF_PAGE		(2176)
#define PHY_PAGE_OF_BLOCK		(64)



// 物理地址相关宏
#define PHY_BYTE_ADDR_GET(addr)				(addr & 0xFFF)
#define PHY_PAGE_ADDR_GET(addr)      		(addr >> 12)
#define PHY_BLOCK_ADDR_GET(addr)			(addr >> 18)
#define PHY_BYTE_ADDR_SET(addr, value)		((addr&(~(0xFFF << 0))) + (value&0xFFF))			// bit0~11, 12bit
#define PHY_PAGE_ADDR_SET(addr, value)		((addr&(~(0x3F << 12))) + ((value&0x3F) << 12))	// bit12~17, 6bit
#define PHY_BLOCK_ADDR_SET(addr, value)		((addr&(~(0xFFF << 18))) + ((value&0xFFF) << 18))	// bit18~29, 12bit
#define PHY_ADDR_SET(block, page, byte)		(((block&0xFFF) << 18) | ((page&0x3F) << 12) | (byte&0xFFF))		


// 逻辑地址相关宏，
#define LOG_BYTE_ADDR_GET(addr)				(addr & 0x7FF)
#define LOG_PAGE_ADDR_GET(addr)      		((addr >> 11) & 0x3F)
#define LOG_BLOCK_ADDR_GET(addr)			((addr >> 17) & 0xFFF)
#define LOG_BYTE_ADDR_SET(addr, value)		((addr&(~(0x7FF << 0))) + (value&0x7FF))			// bit0~10, 11bit
#define LOG_PAGE_ADDR_SET(addr, value)		((addr&(~(0x3F << 11))) + ((value&0x3F) << 11))   // bit11~16, 6bit
#define LOG_BLOCK_ADDR_SET(addr, value)		((addr&(~(0xFFF << 17))) + ((value&0xFFF) << 17)) // bit17~27, 12bit
#define LOG_ADDR_SET(block, page, byte)		(((block&0xFFF) << 17) | ((page&0x3F) << 11) | (byte&0x7FF))		


// 逻辑地址转为物理地址
// 逻辑地址的column比物理地址少1bit，物理地址30bit，逻辑地址29bit（共512MB）
#define CONVERT_LOG_ADDR_TO_PHY(logAddr) (((logAddr&0xFFFFFF800)<<1) | (logAddr&0x7FF))

//feature status
typedef enum
{
	NANDFLASH_OIP_BUSY 		= 0X01,
	NANDFLASH_WEL_LAT 		= 0X02,
	NANDFLASH_E_FAIL 		= 0X04,
	NANDFLASH_P_FAIL 		= 0X08,
	NANDFLASH_ECC_S0 		= 0X10,
	NANDFLASH_ECC_S1  		= 0X20,
	NANDFLASH_ECC_S2  		= 0X40,
	NANDFLASH_CACHE_BUSY 	= 0X80,
}Status_Reg;

typedef enum
{
	//lower block
	Block_Lock_0_3 			= 0x0C,
	Block_Lock_0_7 			= 0x14,
	Block_Lock_0_15 		= 0x1C,
	Block_Lock_0_31 		= 0x24,
	Block_Lock_0_63 		= 0x2C,
	Block_Lock_0_127 		= 0x34,
	Block_Lock_0_255 		= 0x3C,
	Block_Lock_0_511 		= 0x44,
	Block_Lock_0_1023 		= 0x4C,
	Block_Lock_0_2047 		= 0x54,
	Block_Lock_ALL 			= 0x7C,

	//upper blck
	Block_Lock_NONE 		= 0x00,
	Block_Lock_4092_4095 	= 0x08,
	Block_Lock_4088_4095 	= 0x10,
	Block_Lock_4080_4095 	= 0x18,
	Block_Lock_4064_4095 	= 0x20,
	Block_Lock_4032_4095 	= 0x28,
	Block_Lock_3968_4095 	= 0x30,
	Block_Lock_3840_4095 	= 0x38,
	Block_Lock_3584_4095 	= 0x40,
	Block_Lock_3072_4095 	= 0x48,
	Block_Lock_2048_4095 	= 0x50,
}BockProtection_s;


extern uint8 Drv_NandFlash_Open(void);
extern uint8 Drv_NandFlash_Close(void);
extern uint16_t Drv_NandFlash_SoftInit(uint8_t blockLockState);
extern uint16_t Drv_NandFlash_SoftReset(void);
extern uint16_t Drv_NandFlash_BlockErase(uint32_t addr);
extern uint16_t Drv_NandFlash_PageWrite(uint8_t* pdata,uint32_t addr, uint32_t length);
extern uint16_t Drv_NandFlash_PageRead(uint8_t* pdata,uint32_t addr, uint32_t length);
extern uint16_t Drv_NandFlashIdCHeck(void);


#endif
