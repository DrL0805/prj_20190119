#include "mid_nandflash.h"

#include "rtos.h"
#include "mid_scheduler.h"
#include "am_util_delay.h"

// 内部函数 ****************************************************


/*************** macro define ********************************/

static Mid_NandFlash_Param_t	Mid_NandFlash;
static uint16_t LUT[MAX_PHY_BLOCKS_PER_ZONE];

//**********************************************************************
// 函数功能:	检查某block是否为bad block
// 输入参数：	block，block索引号 0 ~ MAX_PHY_BLOCKS_PER_ZONE
//				ZONE，0 ~ ZONE_NUM
// 返回参数：	0不是坏块，1是坏块
static uint16_t LUT_IsBadBlock(uint16_t block, uint16_t ZONE)
{
	uint8_t tReadBuf[1] = {0x00};
	uint32_t tPhyAddr = 0;
	
	// 每block第1page的第2048字节，非0xFF表示坏块
	tPhyAddr = PHY_BLOCK_ADDR_SET(tPhyAddr, (block + MAX_PHY_BLOCKS_PER_ZONE*ZONE));
	tPhyAddr = PHY_BYTE_ADDR_SET(tPhyAddr, 2048);
	Drv_NandFlash_PageRead(tReadBuf, tPhyAddr, 1);
	if((0xFF != tReadBuf[0]))
		return 1;
	
	return 0;
}

//**********************************************************************
// 函数功能:	标记某块为坏块
// 输入参数：	block，block索引号 0 ~ MAX_PHY_BLOCKS_PER_ZONE
//				ZONE，0 ~ ZONE_NUM
// 返回参数：	0 成功
static uint16_t LUT_MarkBadBlock(uint16_t block, uint16_t ZONE)
{
	uint32_t tPhyAddr = 0;
	uint8_t tWriteBuf[1] = {0x61};
	
	// 每block第1page的第2048字节，非0xFF表示坏块
	tPhyAddr = PHY_BLOCK_ADDR_SET(tPhyAddr, (block + MAX_PHY_BLOCKS_PER_ZONE*ZONE));
	tPhyAddr = PHY_BYTE_ADDR_SET(tPhyAddr, 2048);
	Drv_NandFlash_PageWrite(tWriteBuf, tPhyAddr, 1);
	
	return 0;
}

//**********************************************************************
// 函数功能:	从坏块LUT备用区域中获取可用的空闲块索引号
// 输入参数：	无
// 返回参数：	0 获取失败，非0 块索引号
static uint16_t LUT_FreeBlockGet(uint16_t* LUT)
{
	uint16_t block;
	
	for(block = MAX_LOG_BLOCKS_PER_ZONE; block < MAX_PHY_BLOCKS_PER_ZONE; block++)
	{
		if(LUT[block] & FREE_BLOCK)
			return block;
	}
	
	return 0;	// 无备用块
}

//**********************************************************************
// 函数功能:	建立坏块管理表
// 输入参数：	LUT 表地址
//				ZONE zone号
// 返回参数：	0成功，非0失败
static uint16_t LUT_LUTBuild(uint16_t* LUT, uint16_t ZONE)
{
	uint16_t block;
	uint16_t backblock;
	
	// 检查备用block，并更新LUT
	for(block = MAX_LOG_BLOCKS_PER_ZONE; block < MAX_PHY_BLOCKS_PER_ZONE; block++)
	{
		if(LUT_IsBadBlock(block, ZONE))
		{
			//mark bad
			LUT[block] = block | BAD_BLOCK;
		}
		else
		{
			//mark free
			LUT[block] = block | FREE_BLOCK;
		}
	}
	
	// 检测数据block，并更新LUT
	for(block = 0; block < MAX_LOG_BLOCKS_PER_ZONE; block++)
	{
		// 若是坏块，映射到相应的备用块中
		if(LUT_IsBadBlock(block, ZONE))
		{
			//mark bad
			LUT[block] = block | BAD_BLOCK;
			backblock = LUT_FreeBlockGet(LUT);
			if(!backblock)
				return 0xFF;	// 无备用块，失败
			
			LUT[backblock] = block | BACKBAD_BLOCK | VALID_BLOCK;	// 把备用区域的块写上实际地址，并做标志
		}
		else
		{
			//mark valid
			LUT[block] = block | VALID_BLOCK;
		}
	}
 
	return 0;
}

//**********************************************************************
// 函数功能:	把坏块管理信息保存到flash中
// 输入参数：	LUT 坏块管理表地址
//				ZONE zone好
// 返回参数：	存入flash的block索引号
static uint16_t LUT_LUTStore(uint16_t* LUT, uint16_t ZONE)
{
	uint16_t block;
	uint32_t tPhyAddr = 0;
	uint8_t tWriteBuf[2048];	// 非0值
	
	// 把此LUT存入此zone的最有一个有效block的page0
	// 暂时默认最后有效block不会用作替代块
	for(block = (MAX_PHY_BLOCKS_PER_ZONE - 1);block >= MAX_LOG_BLOCKS_PER_ZONE; block--)
	{
		if(LUT[block] & FREE_BLOCK)
		{
			tPhyAddr = PHY_BLOCK_ADDR_SET(tPhyAddr, (block + MAX_PHY_BLOCKS_PER_ZONE*ZONE));
			tPhyAddr = PHY_PAGE_ADDR_SET(tPhyAddr, 0);
			tPhyAddr = PHY_BYTE_ADDR_SET(tPhyAddr, 0);
			
			for(uint32_t i = 0;i < MAX_PHY_BLOCKS_PER_ZONE;i++)
			{
				tWriteBuf[2*i] = (uint8_t)(LUT[i]);
				tWriteBuf[2*i+1] = (uint8_t)(LUT[i] >> 8);
			}
			
			// 先擦
			Drv_NandFlash_BlockErase(tPhyAddr);
			
			// 再写
			Drv_NandFlash_PageWrite(tWriteBuf, tPhyAddr, 2048);	

			return block;
		}
	}
	
	return INVALID_BLOCK_INDEX;
}

//**********************************************************************
// 函数功能:	获取zone的坏块LUT索引
// 输入参数：	ZONE
// 返回参数：	存储LUT的block索引
static uint16_t LUT_LUTBlockIndexGet(uint16_t ZONE)
{
	uint16_t block;
	uint32_t tPhyAddr = 0;
	uint8_t tReadBuf[8];
	
	for(block = (MAX_PHY_BLOCKS_PER_ZONE - 1);block >= MAX_LOG_BLOCKS_PER_ZONE; block--)
	{
		// 从后找到第一个好块
		if(!(LUT_IsBadBlock(block, ZONE)))
		{
			tPhyAddr = PHY_BLOCK_ADDR_SET(tPhyAddr, (block + MAX_PHY_BLOCKS_PER_ZONE*ZONE));
			tPhyAddr = PHY_PAGE_ADDR_SET(tPhyAddr, 0);
			tPhyAddr = PHY_BYTE_ADDR_SET(tPhyAddr, 0);
			Drv_NandFlash_PageRead(tReadBuf, tPhyAddr, 8);

			// 如果是有效地址值，说明已创建LUT表
			if((0 == tReadBuf[0]) && (1 == tReadBuf[2]) && (2 == tReadBuf[4]) && (3 == tReadBuf[6]))
			{
				return block;
			}
			else
			{
				return INVALID_BLOCK_INDEX;
			}
		}
	}
	
	return INVALID_BLOCK_INDEX;
}

// 坏块管理表初始化
static uint16_t LUT_Init(void)
{
	uint16_t tZoneNum = 0;
//	uint16_t LUT[MAX_PHY_BLOCKS_PER_ZONE];
	
	if(false == Mid_NandFlash.LUTInitFlg)
	{
		// 获取zone的LUT存储block索引
		for(tZoneNum = 0; tZoneNum < ZONE_NUM; tZoneNum++)
		{
			Mid_NandFlash.LUTBlockIndex[tZoneNum] = LUT_LUTBlockIndexGet(tZoneNum);
		}
		
		// 若IndexBlock全部有效，说明已创建过坏块LUT。否则重新创建
		if(	(INVALID_BLOCK_INDEX == Mid_NandFlash.LUTBlockIndex[0]) || 
			(INVALID_BLOCK_INDEX == Mid_NandFlash.LUTBlockIndex[1]) ||
			(INVALID_BLOCK_INDEX == Mid_NandFlash.LUTBlockIndex[2]) ||
			(INVALID_BLOCK_INDEX == Mid_NandFlash.LUTBlockIndex[3]))
		{
			for(tZoneNum = 0; tZoneNum < ZONE_NUM; tZoneNum++)
			{
				// 初始化坏块LUT数据
				memset(LUT, 0x0000, MAX_PHY_BLOCKS_PER_ZONE);
				
				// 创建此zone的坏块LUT（向LUT中写入此zone的坏块数据）
				LUT_LUTBuild(LUT, tZoneNum);
				
				// 把此LUT存入此zone的最有一个有效block的page0
				Mid_NandFlash.LUTBlockIndex[tZoneNum] = LUT_LUTStore(LUT, tZoneNum);
			}					
		}
		
		Mid_NandFlash.LUTInitFlg = true;
	}
}

//**********************************************************************
// 函数功能:	根据block序号，获取此block的实际地址
// 输入参数：	ZONE
// 返回参数：	存储LUT的block索引
static uint16_t LUT_ValidBlockIndexGet(uint16_t block, uint16_t ZONE)
{
	uint32_t tPhyAddr = 0;
	uint16_t tBlockMark;
	uint8_t tReadBuf[2*MAX_SPARE_BLOCKS_PER_ZONE];
	
	if(false == Mid_NandFlash.LUTInitFlg)
		return INVALID_BLOCK_INDEX;
	
	if((block >= MAX_LOG_BLOCKS_PER_ZONE) || (ZONE >= ZONE_NUM))
		return INVALID_BLOCK_INDEX;
	
	// 从LUTblock中读取当前块地址
	tPhyAddr = PHY_ADDR_SET(Mid_NandFlash.LUTBlockIndex[ZONE] + ZONE * MAX_PHY_BLOCKS_PER_ZONE, 0, block*2);
	Drv_NandFlash_PageRead(tReadBuf, tPhyAddr, 2);
	
	tBlockMark = block | VALID_BLOCK;
	if((tReadBuf[0] | (tReadBuf[1] << 8)) == tBlockMark)
	{
		return block;
	}
	else
	{
		// 从备用block中搜寻
		tPhyAddr = PHY_ADDR_SET(Mid_NandFlash.LUTBlockIndex[ZONE] + ZONE * MAX_PHY_BLOCKS_PER_ZONE, 0, 2*MAX_LOG_BLOCKS_PER_ZONE);
		Drv_NandFlash_PageRead(tReadBuf, tPhyAddr, 2*MAX_SPARE_BLOCKS_PER_ZONE);	

		tBlockMark = block | VALID_BLOCK | BACKBAD_BLOCK;
		for(uint16_t i = 0;i < MAX_SPARE_BLOCKS_PER_ZONE;i++)
		{
			if((tReadBuf[2*i] | (tReadBuf[2*i+1] << 8)) == tBlockMark)
			{
				return (MAX_LOG_BLOCKS_PER_ZONE + i);
			}
		}
	}
	
	return INVALID_BLOCK_INDEX;
}

//**********************************************************************
// 函数功能:	读，不可夸页
// 输入参数：	pdata	数据指针
//				LogAddr	逻辑地址
//				length	数据长度
// 返回参数：	
uint16_t Mid_NandFlash_PageRead(uint8_t* pdata,uint32_t LogAddr, uint32_t length)
{
	uint32_t tPhyAddr;
	uint16_t tBlock, tPage, tByte;
	uint16_t RetVal = Ret_OK;
	uint8_t	 tZone;
	
	tZone = LOG_BLOCK_ADDR_GET(LogAddr) / MAX_PHY_BLOCKS_PER_ZONE;
	tBlock = LOG_BLOCK_ADDR_GET(LogAddr) % MAX_PHY_BLOCKS_PER_ZONE;
	tPage = LOG_PAGE_ADDR_GET(LogAddr);
	tByte = LOG_BYTE_ADDR_GET(LogAddr);
	
	if(tBlock >= MAX_LOG_BLOCKS_PER_ZONE)
		return Ret_InvalidParam;
	
	if((tByte + length) > PAGE_SIZE)
		return Ret_InvalidParam;
	
	// 根据坏块表，若为坏块，映射到好块
	tBlock = LUT_ValidBlockIndexGet(tBlock, tZone);
	
	// 逻辑地址转物理地址
	tPhyAddr = PHY_ADDR_SET(tBlock + tZone*MAX_PHY_BLOCKS_PER_ZONE, tPage, tByte);
	
	RetVal |= Drv_NandFlash_PageRead(pdata, tPhyAddr, length);
	
	return RetVal;
}

//**********************************************************************
// 函数功能:	写，不可夸页
// 输入参数：	pdata	数据指针
//				LogAddr	逻辑地址
//				length	数据长度
// 返回参数：
uint16_t Mid_NandFlash_PageWrite(uint8_t* pdata,uint32_t LogAddr, uint32_t length)
{
	uint32_t tPhyAddr;
	uint16_t tBlock, tPage, tByte;
	uint16_t RetVal = Ret_OK;
	uint8_t	 tZone;
	
	tZone = LOG_BLOCK_ADDR_GET(LogAddr) / MAX_PHY_BLOCKS_PER_ZONE;
	tBlock = LOG_BLOCK_ADDR_GET(LogAddr) % MAX_PHY_BLOCKS_PER_ZONE;
	tPage = LOG_PAGE_ADDR_GET(LogAddr);
	tByte = LOG_BYTE_ADDR_GET(LogAddr);
	
	if(tBlock >= MAX_LOG_BLOCKS_PER_ZONE)
		return Ret_InvalidParam;
	
	if((tByte + length) > PAGE_SIZE)
		return Ret_InvalidParam;	
	
	// 根据坏块表，若为坏块，映射到好块
	tBlock = LUT_ValidBlockIndexGet(tBlock, tZone);
	
	// 逻辑地址转物理地址
	tPhyAddr = PHY_ADDR_SET(tBlock + tZone*MAX_PHY_BLOCKS_PER_ZONE, tPage, tByte);
	
	RetVal |= Drv_NandFlash_PageWrite(pdata, tPhyAddr, length);
	
	return RetVal;
}

//**********************************************************************
// 函数功能:	擦1块
// 输入参数：	LogAddr	逻辑地址
// 返回参数：
uint16_t Mid_NandFlash_BlockErase(uint32_t LogAddr)
{
	uint32_t tPhyAddr;
	uint16_t tBlock, tPage, tByte;
	uint16_t RetVal = Ret_OK;	
	uint8_t	 tZone;
	
	tZone = LOG_BLOCK_ADDR_GET(LogAddr) / MAX_PHY_BLOCKS_PER_ZONE;
	tBlock = LOG_BLOCK_ADDR_GET(LogAddr) % MAX_PHY_BLOCKS_PER_ZONE;
	tPage = LOG_PAGE_ADDR_GET(LogAddr);
	tByte = LOG_BYTE_ADDR_GET(LogAddr);
	
	if(tBlock >= MAX_LOG_BLOCKS_PER_ZONE)
		return Ret_InvalidParam;	
	
	// 根据坏块表，若为坏块，映射到好块
	tBlock = LUT_ValidBlockIndexGet(tBlock, tZone);
	
	// 逻辑地址转物理地址
	tPhyAddr = PHY_ADDR_SET(tBlock + tZone*MAX_PHY_BLOCKS_PER_ZONE, tPage, tByte);
	
	RetVal |= Drv_NandFlash_BlockErase(tPhyAddr);
	
	return RetVal;	
}

// 逻辑地址有效性检测
static uint16_t Mid_NandFlash_AddrCheck(uint32_t LogAddr)
{
	uint16_t tBlock, tPage, tByte;

	tBlock = LOG_BLOCK_ADDR_GET(LogAddr);
	tPage = LOG_PAGE_ADDR_GET(LogAddr);
	tByte = LOG_BYTE_ADDR_GET(LogAddr);
	
	if((tBlock % MAX_PHY_BLOCKS_PER_ZONE) >= MAX_LOG_BLOCKS_PER_ZONE)
		return Ret_InvalidParam;
	
	if(tPage >= BLOCK_PAGES)
		return Ret_InvalidParam;

	if(tByte >= PAGE_SIZE)
		return Ret_InvalidParam;
	
	return Ret_OK;
}

//**********************************************************************
// 函数功能:	读，可夸page/block
// 输入参数：	pdata	数据指针
//				LogAddr	逻辑地址
//				length	数据长度
// 返回参数：
uint16_t Mid_NandFlash_Read(uint8_t* pdata,uint32_t LogAddr, uint32_t length)
{
	uint32_t tAlreadyReadLen;		// 已读入数据长度
	uint32_t tRemainReadLen;		// 待读入数据长度
	uint32_t tPageReadableLen;		// 页剩余可读数据长度
	uint32_t tReadLen;
	uint32_t tLogAddr;				// 当前写入逻辑地址
	uint16_t tBlock, tPage, tByte;
	uint16_t RetVal = Ret_OK;

	RetVal = Mid_NandFlash_AddrCheck(LogAddr);
	if(Ret_OK != RetVal) return RetVal;
	
	tBlock = LOG_BLOCK_ADDR_GET(LogAddr);
	tPage = LOG_PAGE_ADDR_GET(LogAddr);
	tByte = LOG_BYTE_ADDR_GET(LogAddr);
	
	tAlreadyReadLen = 0;
	tRemainReadLen = length;
	tPageReadableLen = PAGE_SIZE - tByte;
	
	while(1)
	{
		// 取较小值
		tReadLen = tPageReadableLen <= tRemainReadLen ? tPageReadableLen : tRemainReadLen;		
		
		// 数据从flash读出
		tLogAddr = LOG_ADDR_SET(tBlock, tPage, tByte);
		RetVal = Mid_NandFlash_AddrCheck(LogAddr);
		if(Ret_OK != RetVal) return RetVal;		
		Mid_NandFlash_PageRead(pdata+tAlreadyReadLen, tLogAddr, tReadLen);
		
		// 更新数据
		tAlreadyReadLen += tReadLen;
		tRemainReadLen -= tReadLen;
		tPageReadableLen = PAGE_SIZE;		
		tPage++;	// 下一页
		tByte = 0;	// 下一页从地址0开始写
		
		// 跨block
		if(tPage >= BLOCK_PAGES)
		{
			tPage = 0;
			tBlock++;
		}
		
		if(0 == tRemainReadLen)
			break;
	}
	
	return RetVal;
}

//**********************************************************************
// 函数功能:	写，可夸page/block
// 输入参数：	pdata	数据指针
//				LogAddr	逻辑地址
//				length	数据长度
// 返回参数：
uint16_t Mid_NandFlash_Write(uint8_t* pdata,uint32_t LogAddr, uint32_t length)
{
	uint32_t tAlreadyWriteLen;		// 已写入数据长度
	uint32_t tRemainWriteLen;		// 待写入数据长度
	uint32_t tPageWriteableLen;		// 页剩余可写数据长度
	uint32_t tWriteLen;
	uint32_t tLogAddr;				// 当前写入逻辑地址
	uint16_t tBlock, tPage, tByte;
	uint16_t RetVal = Ret_OK;	

	RetVal = Mid_NandFlash_AddrCheck(LogAddr);
	if(Ret_OK != RetVal) return RetVal;
	
	tBlock = LOG_BLOCK_ADDR_GET(LogAddr);
	tPage = LOG_PAGE_ADDR_GET(LogAddr);
	tByte = LOG_BYTE_ADDR_GET(LogAddr);
	
	tAlreadyWriteLen = 0;
	tRemainWriteLen = length;
	tPageWriteableLen = PAGE_SIZE - tByte;
	
	while(1)
	{
		// 取较小值
		tWriteLen = tPageWriteableLen <= tRemainWriteLen ? tPageWriteableLen : tRemainWriteLen;		
		
		// 数据写入flash
		tLogAddr = LOG_ADDR_SET(tBlock, tPage, tByte);
		RetVal = Mid_NandFlash_AddrCheck(LogAddr);
		if(Ret_OK != RetVal) return RetVal;		
		Mid_NandFlash_PageWrite(pdata+tAlreadyWriteLen, tLogAddr, tWriteLen);
		
		// 更新数据
		tAlreadyWriteLen += tWriteLen;
		tRemainWriteLen -= tWriteLen;
		tPageWriteableLen = PAGE_SIZE;		
		tPage++;	// 下一页
		tByte = 0;	// 下一页从地址0开始写
		
		// 跨block
		if(tPage >= BLOCK_PAGES)
		{
			tPage = 0;
			tBlock++;
			
			// 新block写入前必须先擦除
			#if 0	// 暂时屏蔽，因为可能误擦除
			tLogAddr = LOG_ADDR_SET(tBlock, 0, 0);
			RetVal = Mid_NandFlash_AddrCheck(LogAddr);
			if(Ret_OK != RetVal) return RetVal;	
			Mid_NandFlash_BlockErase(LogAddr);
			#endif
		}
		
		if(0 == tRemainWriteLen)
			break;
	}
	
	return RetVal;
}

//**********************************************************************
// 函数功能:	多block连续擦除
// 输入参数：	LogStartAddr	开始地址
//				LogEndAddr		结束地址
// 返回参数：
uint16_t Mid_NandFlash_Erase(uint32_t LogStartAddr, uint32_t LogEndAddr)
{
	uint16_t RetVal = Ret_OK;
	uint16_t tStartBlock, tEndBlock;
	uint32_t tLogAddr = 0;
	
	RetVal = Mid_NandFlash_AddrCheck(LogStartAddr);
	if(Ret_OK != RetVal) return RetVal;

	RetVal = Mid_NandFlash_AddrCheck(LogEndAddr);
	if(Ret_OK != RetVal) return RetVal;
	
	tStartBlock = LOG_BLOCK_ADDR_GET(LogStartAddr);
	tEndBlock = LOG_BLOCK_ADDR_GET(LogEndAddr);
	if(tStartBlock > tEndBlock)
		return Ret_InvalidParam;
	
	for(uint16_t tBlock = tStartBlock;tBlock <= tEndBlock;tBlock++)
	{
		tLogAddr = LOG_ADDR_SET(tBlock, 0, 0);
		Mid_NandFlash_BlockErase(tLogAddr);
	}
	
	return RetVal;
}

//**********************************************************************
// 函数功能:	nandflash软件初始化
// 输入参数：	
// 返回参数：	
// 0x00    :    操作成功
// 0xff    :    操作失败
uint16_t Mid_NandFlash_Init(void)
{
	uint16_t RetVal = Ret_OK;
	
	Drv_NandFlash_SoftReset();
	Drv_NandFlash_SoftInit(Block_Lock_NONE);
	
	// 坏块管理初始化
	LUT_Init();
	
	return RetVal;	
}

uint16_t Mid_NandFlash_SelfTest(void)
{
	uint16_t RetVal = Ret_OK;
	
	RetVal |= Drv_NandFlashIdCHeck();
	
	#if 0	// 跨page/block擦写读测试
	uint32_t tLogAddr = 0;
	uint8_t tWriteBuf[32] = {0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68};
	uint8_t tReadBuf[32];	
	
	tLogAddr = LOG_ADDR_SET(2005,63,2045);
	
	Mid_NandFlash_Erase(tLogAddr, tLogAddr+32);
	
//	Mid_NandFlash_Write(tWriteBuf, tLogAddr, 32);	
	
	Mid_NandFlash_Read(tReadBuf, tLogAddr, 32);
	for(uint32_t i = 0;i < 32;i++)
	{
		MID_NANDFLASH_RTT_LOG(0,"%02X ", tReadBuf[i]);
	}MID_NANDFLASH_RTT_LOG(0,"\r\n");	
	#endif
	
	#if 0	// 逻辑层所有block擦写读测试
	uint32_t tLogAddr = 0;
	uint8_t tWriteBuf[8] = {61,62,63,64};
	uint8_t tReadBuf[8];

	for(uint16_t i = 0;i <4096;i++)
	{
		tLogAddr = LOG_ADDR_SET(i,3,116);
		Mid_NandFlash_BlockErase(tLogAddr);
	}
	
	for(uint16_t i = 0;i <4096;i++)
	{
		tWriteBuf[0] = (uint8_t)i;
		tWriteBuf[1] = (uint8_t)(i >> 8);
		tLogAddr = LOG_ADDR_SET(i,3,116);
		Mid_NandFlash_PageWrite(tWriteBuf, tLogAddr, 8);
	}
	
	for(uint16_t i = 0;i <4096;i++)
	{
		memset(tReadBuf, 0x00, 8);
		
		tLogAddr = LOG_ADDR_SET(i,3,116);
		Mid_NandFlash_PageRead(tReadBuf, tLogAddr, 8);
		if(i != (tReadBuf[0] | (tReadBuf[1] << 8)))
			MID_NANDFLASH_RTT_LOG(0,"Read Err %d \r\n", i);
		
//		for(uint32_t j = 0;j < 8;j++)
//		{
//			MID_NANDFLASH_RTT_LOG(0,"%02X ", tReadBuf[j]);
//		}MID_NANDFLASH_RTT_LOG(0,"\r\n");		
	}	
	
	#endif
	
	#if 0	// 逻辑层flash读写测试
	uint32_t tLogAddr = 0;
	uint8_t tWriteBuf[8] = {61,62,63,64};
	uint8_t tReadBuf[8];

	tLogAddr = LOG_ADDR_SET(1133, 100, 1568);
	Mid_NandFlash_PageWrite(tWriteBuf, tLogAddr, 8);
	Mid_NandFlash_PageRead(tReadBuf, tLogAddr, 8);
	for(uint32_t i= 0;i < 8;i++)
	{
		MID_NANDFLASH_RTT_LOG(0,"%02X ", tReadBuf[i]);
	}MID_NANDFLASH_RTT_LOG(0,"\r\n");
	#endif
	
	#if 0	// 逻辑block与实际block索引测试
	uint16_t tBlock;
	
	for(uint32_t tZone = 0; tZone < 4;tZone++)
	{
		for(uint32_t i = 0;i < MAX_LOG_BLOCKS_PER_ZONE;i++)
		{
			tBlock = LUT_ValidBlockIndexGet(i, tZone);
			if(i != tBlock)
			{
				MID_NANDFLASH_RTT_LOG(0,"%d tZone %d block Index is %d \n", tZone, i, tBlock);
			}
		}		
	}
	
	#endif
	
	#if 0	// 坏块标记测试
	uint32_t tPhyAddr = 0;
	
//	for(uint32_t i = 0;i < 4096;i++)
//	{
//		tPhyAddr = PHY_BLOCK_ADDR_SET(tPhyAddr, i);	
//		Drv_NandFlash_BlockErase(tPhyAddr);
//	}
	
	LUT_MarkBadBlock(51, 2);	//2099
	LUT_MarkBadBlock(39, 0);
	LUT_MarkBadBlock(261, 3);
	LUT_MarkBadBlock(669, 1);
	LUT_MarkBadBlock(512, 2);
	
	for(uint32_t tZone = 0; tZone < 4;tZone++)
	{
		for(uint32_t i= 0;i < 1024;i++)
		{
			if(LUT_IsBadBlock(i, tZone))
			{
				MID_NANDFLASH_RTT_LOG(0,"Zone %d block %d is bad \n", tZone, i);
			}
		}		
	}
	
	#endif
	
	#if 0	// 读取某page值并打印
	uint32_t tPhyAddr = 0;
	uint8_t tReadBuf[2100];
	uint8_t tWriteBuf[8] = {61,62,63,64};
	uint16	tBlockAddr = 39;

	tPhyAddr = PHY_ADDR_SET(tBlockAddr, 0,0);
	Drv_NandFlash_PageRead(tReadBuf, tPhyAddr, 2100);
	for(uint32_t i = 0;i < 2100;i++)
	{
		if((0 == i%16) && (0 != i))
			MID_NANDFLASH_RTT_LOG(0,"\r\n");		
		
		MID_NANDFLASH_RTT_LOG(0,"%02X ", tReadBuf[i]);
	}
	
	#endif

	
	#if 0	// 4096块擦写读测试
	uint8_t tReadBuf[256] = {0x66};
	uint8_t tWriteBuf[256] = {0x66,0x88};
	uint32_t tByteAddr = 0x42;
	uint32_t tPhyAddr = 0;
	
	for(uint32_t i = 0;i < 4096;i++)
	{
		tPhyAddr = PHY_BLOCK_ADDR_SET(tPhyAddr, i);	
		Drv_NandFlash_BlockErase(tPhyAddr);
	}
	
	for(uint32_t i = 0;i < 4096;i++)
	{
		tWriteBuf[0] = (uint8_t)i;
		tWriteBuf[1] = (uint8_t)(i >> 8);
		tPhyAddr = PHY_ADDR_SET(i, 0, tByteAddr);
		Drv_NandFlash_PageWrite(tWriteBuf, tPhyAddr, 4);
	}
	
	for(uint32_t i = 0;i < 4096;i++)	// 共2048 block
	{
		tPhyAddr = PHY_ADDR_SET(i, 0, tByteAddr);
		Drv_NandFlash_PageRead(tReadBuf, tPhyAddr, 4);
		MID_NANDFLASH_RTT_LOG(0,"%d \r\n",(tReadBuf[0] | (tReadBuf[1] << 8)));
	}
	MID_NANDFLASH_RTT_LOG(0,"NandFlash Test End \n");
	#endif
	
	return RetVal;
}






