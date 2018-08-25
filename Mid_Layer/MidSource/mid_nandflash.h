#ifndef	MID_NANDFLASH_H
#define	MID_NANDFLASH_H

#include "platform_common.h"
#include "drv_nandflash.h"

#define MID_NANDFLASH_RTT_DEBUG	3
#if (1 == MID_NANDFLASH_RTT_DEBUG)	// 错误等级
#define MID_NANDFLASH_RTT_LOG(...)
#define MID_NANDFLASH_RTT_WARN(...)
#define MID_NANDFLASH_RTT_ERR		SEGGER_RTT_printf
#elif (2 == MID_NANDFLASH_RTT_DEBUG)	// 警告等级
#define MID_NANDFLASH_RTT_LOG(...)
#define MID_NANDFLASH_RTT_WARN	SEGGER_RTT_printf
#define MID_NANDFLASH_RTT_ERR		SEGGER_RTT_printf
#elif (3 == MID_NANDFLASH_RTT_DEBUG)	// 调试等级
#define MID_NANDFLASH_RTT_LOG		SEGGER_RTT_printf
#define MID_NANDFLASH_RTT_WARN	SEGGER_RTT_printf
#define MID_NANDFLASH_RTT_ERR		SEGGER_RTT_printf
#else							// 调试关闭
#define MID_NANDFLASH_RTT_LOG(...)
#define MID_NANDFLASH_RTT_WARN(...)
#define MID_NANDFLASH_RTT_ERR(...)
#endif

#define PAGE_SIZE            2048  
#define BLOCK_PAGES          64  
#define BLOCKS_PER_ZONE      1024  	// 每1024block用一个LUT[]进行管理
 
/*  
H003所用NandFlash有4096块，分为4个ZONE，分为多ZONE原因：
1）方便ram管理，不同nandflash芯片移植
2）LUT[]操作时不会突然申请太大ram块，导致系统崩溃
*/
#define	ZONE_NUM			(4)		

#define MAX_PHY_BLOCKS_PER_ZONE     BLOCKS_PER_ZONE  	// 物理层块数量
#define MAX_LOG_BLOCKS_PER_ZONE     1000  				// 剔除坏块后可使用的block数量
#define MAX_SPARE_BLOCKS_PER_ZONE	(MAX_PHY_BLOCKS_PER_ZONE - MAX_LOG_BLOCKS_PER_ZONE)	// 备用block数量

#define ERASE_BLOCKSZIE         (BLOCK_PAGES*PAGE_SIZE)  

#define GET_BLOCK_FROM_ADDRESS(x)   (x>>17)  

#define VALID_BLOCK         (1<<12)  
#define FREE_BLOCK          (1<<13)  
#define BACKBAD_BLOCK       (1<<14)  
#define BAD_BLOCK           (1<<15) 

#define LOG_CONVERT_PHY_FAILED_ADDR 0xFFFFFFFF
#define INVALID_BLOCK_INDEX			0xFFFF

typedef struct
{
	bool 		LUTInitFlg;					// 坏块管理表创建标志
	uint16_t	LUTBlockIndex[ZONE_NUM];	// 每个zone的坏块LUT索引，即此zone的坏块LUT存在哪个block中
}Mid_NandFlash_Param_t;

extern uint16_t Mid_NandFlash_PageRead(uint8_t* pdata,uint32_t LogAddr, uint32_t length);
extern uint16_t Mid_NandFlash_PageWrite(uint8_t* pdata,uint32_t LogAddr, uint32_t length);
extern uint16_t Mid_NandFlash_BlockErase(uint32_t LogAddr);

extern uint16_t Mid_NandFlash_Read(uint8_t* pdata,uint32_t LogAddr, uint32_t length);
extern uint16_t Mid_NandFlash_Write(uint8_t* pdata,uint32_t LogAddr, uint32_t length);
extern uint16_t Mid_NandFlash_Erase(uint32_t LogStartAddr, uint32_t LogEndAddr);
extern uint16_t Mid_NandFlash_Init(void);
extern uint16_t Mid_NandFlash_SelfTest(void);


#endif










