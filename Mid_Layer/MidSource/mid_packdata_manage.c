#include "platform_common.h"

/* FreeRTOS includes */
#include "rtos.h"

#include "multimodule_task.h"
#include "mid_extflash.h"
#include "flash_task.h"
#include "mid_packdata_manage.h"


static const catalogInfo_u catalogWrite0Data = 
{
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
};


// 数据区域分配管理
typedef struct 
{
	uint32	startAddr;			// 数据区域开始地址
	uint32	validStartAddr;		// 可进行分配开始地址
	uint32	validLength;		// 剩余可分配长度
}data_allocation_s;

static data_allocation_s	catalogAreaManage;
static data_allocation_s	dataAreaManage;

// 数据信息
typedef struct
{
	uint8		init;						// 数据是否初始化了		1：已初始化 	0:未初始化
	uint8		occupy;						// 是否已新建目录正在存储，且未进行封存。1：占用 0：未占用
	uint8		catalogCycle;				// 目录是否已经循环存储。1：循环 0：未循环
	uint8		dataCycle;					// 数据是否已经循环存储。1：循环 0：未循环


	uint32	catalogAreaStartAddr;		// 目录在外部flash分配区域的开始地址
	uint32	catalogAreaEndAddr;			// 目录在外部flash分配区域的结束地址,包含该地址
	uint32	catalogWritableStartAddr;	// 目录在外部flash可写开始地址
	uint32	catalogWritableLength;		// 目录在外部flash可写长度

	uint32	catalogValidStartAddr;		// 目录在外部flash的有效数据开始地址
	uint32	catalogValidEndAddr;		// 目录在外部flash的有效数据开始地址,不包含该地址

	uint32	dataAreaStartAddr;			// 数据在外部flash分配区域的开始地址
	uint32	dataAreaEndAddr;			// 数据在外部flash分配区域的结束地址,包含该地址
	uint32	dataWritableStartAddr;		// 数据在外部flash可写开始地址
	uint32	dataWritableLength;			// 数据在外部flash可写长度

	uint32	dataValidStartAddr;			// 数据在外部flash的有效数据开始地址
	uint32	dataValidEndAddr;			// 数据在外部flash的有效数据结束地址,不包含该地址

	catalogInfo_s	catalog;				// 最新开辟的目录信息
}classify_data_info_s;



static classify_data_info_s		dataClassifyInfo[DM_APP_DATA_CLASSIFY_NUM];


typedef struct 
{
	uint8			valid;							// 总信息是否有效
	uint16		totalCatalog;					// 目录数量
	uint16		dataClassify;					// 数据分类

	catalogInfo_s	catalogInfo[CATALOG_TOTAL_MAX_NUM];		// 总数据缓存
	uint32		catalogAddr[CATALOG_TOTAL_MAX_NUM];		// 目录所在外部flash位置
}catalog_total_info_s;

static catalog_total_info_s catalogTotalInfo;


//**********************************************************************
// 函数功能:	返回指定数据类型存储的内存位置与长度
// 输入参数：	dataPoint:	返回数据的指针
// 				dataLength:	返回数据的长度
// 返回参数：	0x00:操作成功
// 				0xff:操作失败
//**********************************************************************
uint16 Mid_PackData_ReadInfo(uint32 dataClassify, uint8 **dataPoint, uint16 *dataLength)
{
	switch(dataClassify)
	{
		case STEPDATA_CLASSIFY:
		case SLEEPDATA_CLASSIFY:
		(*dataPoint)	= (uint8*)(&dataClassifyInfo[dataClassify]);
		*dataLength		= sizeof(classify_data_info_s);
		break;

		default:
		return 0xff;		
	}
	return 0;
}

//**********************************************************************
// 函数功能:	初始化数据管理模块数据
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
void Mid_PackData_Init(void)
{
	uint16 i;

	catalogAreaManage.startAddr					= DM_APP_CATALOG_AREA_START_ADD;
	catalogAreaManage.validStartAddr			= catalogAreaManage.startAddr;
	catalogAreaManage.validLength				= DM_APP_CATALOG_AREA_LENGTH;

	dataAreaManage.startAddr					= DM_APP_DATA_AREA_START_ADD;
	dataAreaManage.validStartAddr				= dataAreaManage.startAddr;
	dataAreaManage.validLength					= DM_APP_DATA_AREA_LENGTH;

	// 总信息数据初始化，未读取总信息不可进行数据读取
	catalogTotalInfo.valid					= 0;

	for(i = 0; i < DM_APP_DATA_CLASSIFY_NUM; i++)
	{
		dataClassifyInfo[i].init			= 0;
	}
}

//**********************************************************************
// 函数功能:	初始化分类数据的目录与数据区域
// 输入参数：	dataClassify:	数据类型
// 				catalogSize:	目录分配长度，为DM_APP_SECTOR_LENGTH整数倍
// 				dataSize:		数据分配长度，为DM_APP_SECTOR_LENGTH整数倍
// 返回参数：	0x00: 初始化成功
// 				0xff: 区域分配失败
//**********************************************************************
uint16 Mid_PackData_ClassifyDataInit(uint32 dataClassify, uint32 catalogSize, uint32 dataSize)
{
	// 未定义数据
	if(dataClassify >= DM_APP_DATA_CLASSIFY_NUM)
		return 0xff;

	// 输入数据不合法
	if((catalogSize % DM_APP_SECTOR_LENGTH) || (dataSize % DM_APP_SECTOR_LENGTH))
		return 0xff;

	// 该类数据已初始化或该类数据分配长度超出
	if( (dataClassifyInfo[dataClassify].init == 1) ||
		(dataSize > dataAreaManage.validLength) ||
		(catalogSize > catalogAreaManage.validLength))
		return 0xff;

	// 目录区域分配 
	dataClassifyInfo[dataClassify].init							= 1;
	dataClassifyInfo[dataClassify].occupy						= 0;
	dataClassifyInfo[dataClassify].catalogCycle					= 0;
	dataClassifyInfo[dataClassify].dataCycle					= 0;

	dataClassifyInfo[dataClassify].catalogAreaStartAddr			= catalogAreaManage.validStartAddr;
	dataClassifyInfo[dataClassify].catalogAreaEndAddr			= dataClassifyInfo[dataClassify].catalogAreaStartAddr + catalogSize - 1;
	dataClassifyInfo[dataClassify].catalogWritableStartAddr		= dataClassifyInfo[dataClassify].catalogAreaStartAddr;
	dataClassifyInfo[dataClassify].catalogWritableLength		= 0;
	dataClassifyInfo[dataClassify].catalogValidStartAddr		= dataClassifyInfo[dataClassify].catalogAreaStartAddr;
	dataClassifyInfo[dataClassify].catalogValidEndAddr			= dataClassifyInfo[dataClassify].catalogAreaStartAddr;

	// 目录分配有效区域更新
	catalogAreaManage.validStartAddr			= catalogAreaManage.validStartAddr + catalogSize;
	catalogAreaManage.validLength				= catalogAreaManage.validLength - catalogSize;

	// 数据区域分配
	dataClassifyInfo[dataClassify].dataAreaStartAddr			= dataAreaManage.validStartAddr;
	dataClassifyInfo[dataClassify].dataAreaEndAddr				= dataClassifyInfo[dataClassify].dataAreaStartAddr + dataSize - 1;
	dataClassifyInfo[dataClassify].dataWritableStartAddr		= dataClassifyInfo[dataClassify].dataAreaStartAddr;
	dataClassifyInfo[dataClassify].dataWritableLength			= 0;
	dataClassifyInfo[dataClassify].dataValidStartAddr			= dataClassifyInfo[dataClassify].dataAreaStartAddr;
	dataClassifyInfo[dataClassify].dataValidEndAddr				= dataClassifyInfo[dataClassify].dataAreaStartAddr;

	// 数据分配有效区域更新
	dataAreaManage.validStartAddr				= dataAreaManage.validStartAddr + dataSize;
	dataAreaManage.validLength					= dataAreaManage.validLength - dataSize;

	return 0x00;
}

//**********************************************************************
// 函数功能:	开辟新目录，并分配外部存储的起始地址，将UTC、数据分类、采样单位、
// 				采样间隔、等写入。若过程中有其他同类数据正在存储则会报错。保存后
// 				未开启新的目录禁止进行存储。
// 输入参数：	dataInfo:	目录信息
// 返回参数：	0x00: 初始化成功
// 				0xff: 目录分配失败
//**********************************************************************
uint16 Mid_PackData_CreateCatalog(catalogInfo_s *dataInfo)
{
	catalogInfo_u 			dataInfoTemp;
	flash_task_msg_t		flashMsg;
	uint32				classifyTemp;
	uint32				eraseStartAddr;
	uint32				eraseEndAddr;
	uint16				result;
	uint16				i;


	classifyTemp = dataInfo->dataClassify;
	// 检查数据是否合法
	if(classifyTemp >= DM_APP_DATA_CLASSIFY_NUM)
		return 0xff;

	// 检查是否分配区域了
	if(dataClassifyInfo[classifyTemp].init == 0)
		return 0xff;

	// 检查是否已有新建未完成的目录
	if(dataClassifyInfo[classifyTemp].occupy == 1)
		return 0xff;

	for(i = 0; i < CATALOG_INFO_LENGTH; i++)
	{
		dataInfoTemp.databuf[i] = 0xff;
	}


	dataInfoTemp.u.utc				= dataInfo->utc;
	dataInfoTemp.u.startAddr		= dataClassifyInfo[classifyTemp].dataWritableStartAddr;
	dataInfoTemp.u.dataLength		= 0xffffffff;			//  长度暂不写入，
	dataInfoTemp.u.dataClassify		= classifyTemp;
	dataInfoTemp.u.sampleUnit		= dataInfo->sampleUnit;
	dataInfoTemp.u.sampleInterval	= dataInfo->sampleInterval;
	dataInfoTemp.u.unitLength		= dataInfo->unitLength;


	dataClassifyInfo[classifyTemp].catalog				= dataInfoTemp.u;
	dataClassifyInfo[classifyTemp].catalog.dataLength	= 0;


	// 将目录信息存入外部flash

	// 检查是否需要进行擦除
	if(dataClassifyInfo[classifyTemp].catalogWritableLength < CATALOG_INFO_LENGTH)
	{


		// 检查下一个有效区域是否超出目录范围
		if((dataClassifyInfo[classifyTemp].catalogWritableStartAddr + CATALOG_INFO_LENGTH)
			> dataClassifyInfo[classifyTemp].catalogAreaEndAddr)
		{
			// 跨首尾后，不论尾部剩余多少数据，均进行舍弃。重新赋值可写位置为开始地址
			dataClassifyInfo[classifyTemp].catalogWritableStartAddr = 
					dataClassifyInfo[classifyTemp].catalogAreaStartAddr;


			// 擦除区域
            flashMsg.id 									= EXTFLASH_ID;
			flashMsg.flash.extflashEvent.para.startAddr 	= dataClassifyInfo[classifyTemp].catalogWritableStartAddr;
			flashMsg.flash.extflashEvent.para.length 		= DM_APP_SECTOR_LENGTH;		
			flashMsg.flash.extflashEvent.para.endAddr 		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
			flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_4K_ERASE;	
			
			flashMsg.flash.extflashEvent.para.result 		= FlashTask_EventSet(&flashMsg);
					
			result											= flashMsg.flash.extflashEvent.para.result;	
			if(result)
				return 0xff;

			dataClassifyInfo[classifyTemp].catalogCycle				= 1;

			// 跨首尾后，不论尾部剩余多少数据，均进行舍弃。重新赋值可写长度
			dataClassifyInfo[classifyTemp].catalogWritableLength 	= DM_APP_SECTOR_LENGTH;
		}
		else
		{
			// 擦除区域
			flashMsg.id 									= EXTFLASH_ID;
			flashMsg.flash.extflashEvent.para.startAddr 	= dataClassifyInfo[classifyTemp].catalogWritableStartAddr + dataClassifyInfo[classifyTemp].catalogWritableLength;
			flashMsg.flash.extflashEvent.para.length 		= DM_APP_SECTOR_LENGTH;		
			flashMsg.flash.extflashEvent.para.endAddr 		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
			flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_4K_ERASE;	
			
			flashMsg.flash.extflashEvent.para.result 		= FlashTask_EventSet(&flashMsg);
								
			result											= flashMsg.flash.extflashEvent.para.result;

			if(result)
				return 0xff;

			dataClassifyInfo[classifyTemp].catalogWritableLength	+= DM_APP_SECTOR_LENGTH;
		}


		// 进行循环擦除时需要将目录的开始有效地址进行更新
		if(dataClassifyInfo[classifyTemp].catalogCycle == 1)
		{
			// 擦除的开始与结束地址
			eraseStartAddr			= flashMsg.flash.extflashEvent.para.startAddr - (flashMsg.flash.extflashEvent.para.startAddr % DM_APP_SECTOR_LENGTH);
			eraseEndAddr			= eraseStartAddr + DM_APP_SECTOR_LENGTH - 1;

			// 目录的有效开始位置是否在本次擦写的区域内
			// 目录数据需片对齐，否则将会出错
			if((dataClassifyInfo[classifyTemp].catalogValidStartAddr >= eraseStartAddr) && 
				(dataClassifyInfo[classifyTemp].catalogValidStartAddr <= eraseEndAddr))
			{
				dataClassifyInfo[classifyTemp].catalogValidStartAddr = eraseEndAddr + 1;
				if(dataClassifyInfo[classifyTemp].catalogValidStartAddr > dataClassifyInfo[classifyTemp].catalogAreaEndAddr)
				{
					dataClassifyInfo[classifyTemp].catalogValidStartAddr = dataClassifyInfo[classifyTemp].catalogAreaStartAddr;
				}
			}

			// 目录的有效结束位置是否在本次擦写的区域内
			// 目录数据需片对齐，否则将会出错
			if((dataClassifyInfo[classifyTemp].catalogAreaStartAddr >= eraseStartAddr) && 
				(dataClassifyInfo[classifyTemp].catalogValidEndAddr <= eraseEndAddr))
			{
				dataClassifyInfo[classifyTemp].catalogValidEndAddr = eraseEndAddr + 1;
				if(dataClassifyInfo[classifyTemp].catalogValidEndAddr > dataClassifyInfo[classifyTemp].catalogAreaEndAddr)
				{
					dataClassifyInfo[classifyTemp].catalogValidEndAddr = dataClassifyInfo[classifyTemp].catalogAreaStartAddr;
				}
			}
		}
	}

	// 写入目录信息至外部flash
	flashMsg.id 									= EXTFLASH_ID;
	flashMsg.flash.extflashEvent.para.startAddr 	= dataClassifyInfo[classifyTemp].catalogWritableStartAddr;
	flashMsg.flash.extflashEvent.para.dataAddr 		= (uint8*)(dataInfoTemp.databuf);
	flashMsg.flash.extflashEvent.para.length 		= CATALOG_INFO_LENGTH;		
	flashMsg.flash.extflashEvent.para.endAddr 		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
	flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_WRITE;	
	
	dataClassifyInfo[classifyTemp].catalogWritableLength	-= CATALOG_INFO_LENGTH;
	FlashTask_EventSet(&flashMsg);


	dataClassifyInfo[classifyTemp].occupy		= 1;

	return 0x00;
}

//**********************************************************************
// 函数功能:	保存目录长度，并更新有效目录的结束位置，解除目录占用
// 输入参数：	dataClassify:	分类数据
// 返回参数：	0x00: 操作成功
// 				0xff: 操作失败
//**********************************************************************
uint16 Mid_PackData_DataSaveEnd(uint32 dataClassify)
{
	catalogInfo_u 			dataInfoTemp;
	flash_task_msg_t		flashMsg;
	uint16				i;

	// 检查数据是否合法
	if(dataClassify >= DM_APP_DATA_CLASSIFY_NUM)
		return 0xff;

	// 检查是否分配区域了
	if(dataClassifyInfo[dataClassify].init == 0)
		return 0xff;

	// 检查是否未新建目录
	if(dataClassifyInfo[dataClassify].occupy == 0)
		return 0xff;

	for(i = 0; i < CATALOG_INFO_LENGTH; i++)
	{
		dataInfoTemp.databuf[i] = 0xff;
	}

	dataInfoTemp.u				= dataClassifyInfo[dataClassify].catalog;

	// 将长度存入
    flashMsg.id 									= EXTFLASH_ID;
	flashMsg.flash.extflashEvent.para.startAddr 	= dataClassifyInfo[dataClassify].catalogWritableStartAddr;
	flashMsg.flash.extflashEvent.para.dataAddr 		= (uint8*)(dataInfoTemp.databuf);
	flashMsg.flash.extflashEvent.para.length 		= CATALOG_INFO_LENGTH;		
	flashMsg.flash.extflashEvent.para.endAddr 		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
	flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_WRITE;	
	
	FlashTask_EventSet(&flashMsg);

	// 将可写位置往后移动
	dataClassifyInfo[dataClassify].catalogWritableStartAddr	+= CATALOG_INFO_LENGTH;

	// 目录有效结束地址数据位置更新
	dataClassifyInfo[dataClassify].catalogValidEndAddr		= dataClassifyInfo[dataClassify].catalogWritableStartAddr;
	if(dataClassifyInfo[dataClassify].catalogValidEndAddr > dataClassifyInfo[dataClassify].catalogAreaEndAddr)
		dataClassifyInfo[dataClassify].catalogValidEndAddr		= dataClassifyInfo[dataClassify].catalogAreaStartAddr;

	// 数据有效结束地址更新
	dataClassifyInfo[dataClassify].dataValidEndAddr		= dataClassifyInfo[dataClassify].dataWritableStartAddr;
	if(dataClassifyInfo[dataClassify].dataValidEndAddr > dataClassifyInfo[dataClassify].dataAreaEndAddr)
		dataClassifyInfo[dataClassify].dataValidEndAddr		= dataClassifyInfo[dataClassify].dataAreaStartAddr;

	// 解除目录占用
	dataClassifyInfo[dataClassify].occupy		= 0;

	return 0x00;
}

//**********************************************************************
// 函数功能:	根据目录信息转换当前的UTC时间
// 输入参数：	catalogTemp:	目录信息
// 返回参数：	0x00: 操作成功
// 				0xff: 操作失败
//**********************************************************************
static uint32 UtcTranslation(catalogInfo_s *catalogTemp)
{
	uint32 utcTemp;
	uint32 unitNum;

	unitNum		= catalogTemp->dataLength/catalogTemp->unitLength;
	utcTemp		= 0;
	switch(catalogTemp->sampleUnit)
	{
		case DATASAMPLE_UNIT_1US:
		utcTemp		= (unitNum * catalogTemp->sampleInterval) / 1000000;
		break;

		case DATASAMPLE_UNIT_1MS:
		utcTemp		= (unitNum * catalogTemp->sampleInterval) / 1000;
		break;

		case DATASAMPLE_UNIT_1S:
		utcTemp		= (unitNum * catalogTemp->sampleInterval);
		break;

		case DATASAMPLE_UNIT_10US:
		utcTemp		= (unitNum * catalogTemp->sampleInterval) / 100000;
		break;

		case DATASAMPLE_UNIT_10MS:
		utcTemp		= (unitNum * catalogTemp->sampleInterval) / 100;
		break;

		case DATASAMPLE_UNIT_10S:
		utcTemp		= (unitNum * catalogTemp->sampleInterval) * 10;
		break;
	}


	utcTemp += catalogTemp->utc;

	return utcTemp;
}

//**********************************************************************
// 函数功能:	根据分类数据进行数据存储
// 输入参数：	data:			原始数据指针
// 				length:			数据长度，不能超过DM_APP_SECTOR_LENGTH
// 				dataClassify:	分类数据
// 返回参数：	0x00: 操作成功
// 				0xff: 操作失败
//**********************************************************************
uint16 Mid_PackData_ClassifyDataSave(uint8 *data, uint32 length, uint32 dataClassify)
{
	flash_task_msg_t	flashMsg;
	uint32				eraseStartAddr;
	uint32				eraseEndAddr;
	uint32				addrRecord;


	catalogInfo_u			catalogTemp;
	uint32				dataValidStartAddrTemp;
	uint32				dataValidEndAddrTemp;
	uint32				readAddrTemp;
	uint16				lengthTemp;
	uint16				result;

	// 检查数据是否合法
	if(dataClassify >= DM_APP_DATA_CLASSIFY_NUM)
		return 0xff;

	// 单次写入的数据不能超过DM_APP_SECTOR_LENGTH
	if(length > DM_APP_SECTOR_LENGTH)
		return 0xff;

	// 检查是否分配区域了
	if(dataClassifyInfo[dataClassify].init == 0)
		return 0xff;

	// 检查是否未新建目录
	if(dataClassifyInfo[dataClassify].occupy == 0)
		return 0xff;

	lengthTemp				= 0;
	addrRecord 				= 0;


	// 检测本次数据是否会超出最大存储量
	if((dataClassifyInfo[dataClassify].catalog.dataLength + length)
		> DATA_MAX_LENGTH)
	{
		// 进行目录封存
		Mid_PackData_DataSaveEnd(dataClassify);

		// 重新计算utc并新建目录
		dataClassifyInfo[dataClassify].catalog.utc = UtcTranslation(&(dataClassifyInfo[dataClassify].catalog));

		// 重新开辟新目录
		Mid_PackData_CreateCatalog(&(dataClassifyInfo[dataClassify].catalog));
	}


	// 检查是否需要进行擦除
	if(dataClassifyInfo[dataClassify].dataWritableLength < length)
	{
		// 检查下一个有效区域是否超出目录范围
		if((dataClassifyInfo[dataClassify].dataWritableStartAddr + length)
			> dataClassifyInfo[dataClassify].dataAreaEndAddr)
		{
			// 跨首尾后，将尾部剩余部分写入
			if(dataClassifyInfo[dataClassify].dataWritableLength > 0)
			{
				// 写入最小部分
				flashMsg.id 									= EXTFLASH_ID;
				flashMsg.flash.extflashEvent.para.startAddr 	= dataClassifyInfo[dataClassify].dataWritableStartAddr;
				flashMsg.flash.extflashEvent.para.dataAddr 		= (uint8*)(data);
				flashMsg.flash.extflashEvent.para.length 		= dataClassifyInfo[dataClassify].dataWritableLength;;		
				flashMsg.flash.extflashEvent.para.endAddr 		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
				flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_WRITE;	
				
				lengthTemp										= flashMsg.flash.extflashEvent.para.length;
				dataClassifyInfo[dataClassify].dataWritableLength		-= flashMsg.flash.extflashEvent.para.length;

				dataClassifyInfo[dataClassify].dataWritableStartAddr	+= flashMsg.flash.extflashEvent.para.length;
				FlashTask_EventSet(&flashMsg);

				// 将写入的长度加入已经写入的长度中。
				dataClassifyInfo[dataClassify].catalog.dataLength	+= lengthTemp;

				// 将写入的长度减去
				length												-= lengthTemp;
			}

			// 跨首尾后，剩余未写地址小于数据最小单位，尾部进行舍弃
			dataClassifyInfo[dataClassify].dataWritableStartAddr = 
					dataClassifyInfo[dataClassify].dataAreaStartAddr;


			// 擦除区域
			flashMsg.id 									= EXTFLASH_ID;
			flashMsg.flash.extflashEvent.para.startAddr 	= dataClassifyInfo[dataClassify].dataWritableStartAddr;
			flashMsg.flash.extflashEvent.para.length 		= DM_APP_SECTOR_LENGTH;		
			flashMsg.flash.extflashEvent.para.endAddr 		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
			flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_4K_ERASE;	
			
			flashMsg.flash.extflashEvent.para.result 		= FlashTask_EventSet(&flashMsg);
			
			result 											= flashMsg.flash.extflashEvent.para.result;

			if(result)
				return 0xff;

			dataClassifyInfo[dataClassify].dataCycle				= 1;

			// 跨首尾后，剩余未写地址小于数据最小单位，尾部进行舍弃
			dataClassifyInfo[dataClassify].dataWritableLength	= DM_APP_SECTOR_LENGTH;
		}
		else
		{
			// 擦除区域
			flashMsg.id 									= EXTFLASH_ID;
			flashMsg.flash.extflashEvent.para.startAddr 	= dataClassifyInfo[dataClassify].dataWritableStartAddr + dataClassifyInfo[dataClassify].dataWritableLength;
			flashMsg.flash.extflashEvent.para.length 		= DM_APP_SECTOR_LENGTH;		
			flashMsg.flash.extflashEvent.para.endAddr 		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
			flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_4K_ERASE;	
			
			flashMsg.flash.extflashEvent.para.result 		= FlashTask_EventSet(&flashMsg);
			
			result 											= flashMsg.flash.extflashEvent.para.result;

			if(result)
				return 0xff;

			dataClassifyInfo[dataClassify].dataWritableLength	+= DM_APP_SECTOR_LENGTH;
		}

		// 当循环擦除数据时需要修复目录，与该区域数据相关联的目录需被擦除
		if(dataClassifyInfo[dataClassify].dataCycle == 1)
		{
			// 数据的开始与结束地址
			eraseStartAddr			= flashMsg.flash.extflashEvent.para.startAddr - (flashMsg.flash.extflashEvent.para.startAddr % DM_APP_SECTOR_LENGTH);
			eraseEndAddr			= eraseStartAddr + DM_APP_SECTOR_LENGTH - 1;

			// 数据的有效开始位置是否在本次擦写的区域内
			if((dataClassifyInfo[dataClassify].dataValidStartAddr >= eraseStartAddr) && 
				(dataClassifyInfo[dataClassify].dataValidStartAddr <= eraseEndAddr))
			{


				readAddrTemp			= dataClassifyInfo[dataClassify].catalogValidStartAddr;
				// 读取位置与有效的结束位置不一致时，进行持续读取
				while(readAddrTemp != dataClassifyInfo[dataClassify].catalogValidEndAddr)
				{
					flashMsg.id 									= EXTFLASH_ID;
					flashMsg.flash.extflashEvent.para.startAddr 	= readAddrTemp;
					flashMsg.flash.extflashEvent.para.length 		= CATALOG_INFO_LENGTH;		
					flashMsg.flash.extflashEvent.para.endAddr 		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
					flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_READ;
					flashMsg.flash.extflashEvent.para.dataAddr 		= (uint8 *)(catalogTemp.databuf);	
					
					FlashTask_EventSet(&flashMsg);

					// 判断数据是否有效
					if((catalogTemp.u.dataClassify == dataClassify) && (catalogTemp.u.dataLength != 0x00) && (catalogTemp.u.dataLength != 0xffffffff))
					{

						dataValidStartAddrTemp		= catalogTemp.u.startAddr;
						dataValidEndAddrTemp		= dataValidStartAddrTemp + catalogTemp.u.dataLength;

						// 数据已进行循环存储
						if(dataValidEndAddrTemp > dataClassifyInfo[dataClassify].dataAreaEndAddr)
							dataValidEndAddrTemp	= dataValidEndAddrTemp - dataClassifyInfo[dataClassify].dataAreaEndAddr
														+ dataClassifyInfo[dataClassify].dataAreaStartAddr;


						// 判断该目录存放数据的开始地址是否该次擦写的地址范围内
						if(((dataValidStartAddrTemp >= eraseStartAddr)
							&& (dataValidStartAddrTemp <= eraseEndAddr)))
						{
							// 写0该目录
							flashMsg.id 									= EXTFLASH_ID;
							flashMsg.flash.extflashEvent.para.startAddr 	= readAddrTemp;
							flashMsg.flash.extflashEvent.para.length 		= CATALOG_INFO_LENGTH;		
							flashMsg.flash.extflashEvent.para.endAddr 		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
							flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_WRITE;
							flashMsg.flash.extflashEvent.para.dataAddr 		= (uint8 *)(catalogWrite0Data.databuf);
							
							FlashTask_EventSet(&flashMsg);


							// 更新有效数据开始地址，由于数据是根据目录索引故不对齐不会出现问题
							dataClassifyInfo[dataClassify].dataValidStartAddr = eraseEndAddr + 1;
							if(dataClassifyInfo[dataClassify].dataValidStartAddr > dataClassifyInfo[dataClassify].dataAreaEndAddr)
							{
								dataClassifyInfo[dataClassify].dataValidStartAddr = dataClassifyInfo[dataClassify].dataAreaStartAddr;
							}

							// 判断数据的有效地址是否也在该次擦写的范围内
							if((dataClassifyInfo[dataClassify].dataValidEndAddr >= eraseStartAddr) && 
								(dataClassifyInfo[dataClassify].dataValidEndAddr <= eraseEndAddr))
							{
								// 更新有效数据结束地址，由于数据是根据目录索引故不对齐不会出现问题
								dataClassifyInfo[dataClassify].catalogValidEndAddr = eraseEndAddr + 1;
								if(dataClassifyInfo[dataClassify].catalogValidEndAddr > dataClassifyInfo[dataClassify].catalogAreaEndAddr)
								{
									dataClassifyInfo[dataClassify].catalogValidEndAddr = dataClassifyInfo[dataClassify].catalogAreaStartAddr;
								}
							}

							if(readAddrTemp == dataClassifyInfo[dataClassify].catalogValidStartAddr)
							{
								// 更新目录有效地址开始地址
								dataClassifyInfo[dataClassify].catalogValidStartAddr		+= CATALOG_INFO_LENGTH;
								if(dataClassifyInfo[dataClassify].catalogValidStartAddr > dataClassifyInfo[dataClassify].catalogAreaEndAddr)
								{
									dataClassifyInfo[dataClassify].catalogValidStartAddr	= dataClassifyInfo[dataClassify].catalogAreaStartAddr;
								}
							}
						}
						else
						{
							// 在范围外，直接跳出，不需要再次检查
							break;
						}

					}
					else
					{
						if(readAddrTemp == dataClassifyInfo[dataClassify].catalogValidStartAddr)
						{
							// 更新目录有效地址开始地址
							dataClassifyInfo[dataClassify].catalogValidStartAddr		+= CATALOG_INFO_LENGTH;
							if(dataClassifyInfo[dataClassify].catalogValidStartAddr > dataClassifyInfo[dataClassify].catalogAreaEndAddr)
							{
								dataClassifyInfo[dataClassify].catalogValidStartAddr	= dataClassifyInfo[dataClassify].catalogAreaStartAddr;
							}
						}
					}


					readAddrTemp += CATALOG_INFO_LENGTH;
					// 超过存储区域从开始地址继续
					if(readAddrTemp > dataClassifyInfo[dataClassify].catalogAreaEndAddr)
						readAddrTemp = dataClassifyInfo[dataClassify].catalogAreaStartAddr;

					addrRecord  += CATALOG_INFO_LENGTH;
					//遍历异常，检测的目录大小超出目录分配区
					if (addrRecord > DM_APP_CATALOG_AREA_LENGTH)
					{
						break;
					}
				}
			}
		}

	}

	// 写入数据至外部flash
	flashMsg.id 									= EXTFLASH_ID;
	flashMsg.flash.extflashEvent.para.startAddr 	= dataClassifyInfo[dataClassify].dataWritableStartAddr;
	flashMsg.flash.extflashEvent.para.dataAddr 		= (uint8*)(data + lengthTemp);
	flashMsg.flash.extflashEvent.para.length 		= length;		
	flashMsg.flash.extflashEvent.para.endAddr 		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
	flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_WRITE;
	

	// 更新数据区域可写的开始地址与长度
	dataClassifyInfo[dataClassify].dataWritableLength		-= length;
	dataClassifyInfo[dataClassify].dataWritableStartAddr	+= length;

	// 更新已经存储的数据长度
	dataClassifyInfo[dataClassify].catalog.dataLength		+= length;

    FlashTask_EventSet(&flashMsg);
	return 0x00;
}

//**********************************************************************
// 函数功能:	根据分类数据进行数据存储
// 输入参数：	catalogTotal:	目录总数指针，处理结束后更新目录总数，最长目录数为16
// 				dataLength:		数据总数指针，处理结束后更新数据长度
// 				dataClassify:	分类数据
// 返回参数：	0x00: 操作成功
// 				0xff: 操作失败
//**********************************************************************
uint16 Mid_PackData_ClassifyDataInfoRead(uint16 *catalogTotal, uint32 *dataLength, uint32 dataClassify)
{
	flash_task_msg_t		flashMsg;
	catalogInfo_u			catalogTemp;
	uint16				catalogTotalNumTemp;
	uint32				dataLengthTemp;
	uint32				readAddrTemp;
	uint32				dataValidStartAddrTemp;
	uint32				dataValidEndAddrTemp;
	uint32_t 				addrRecord;
	// 检查数据是否合法
	if(dataClassify >= DM_APP_DATA_CLASSIFY_NUM)
		return 0xff;

	// 检查是否分配区域了
	if(dataClassifyInfo[dataClassify].init == 0)
		return 0xff;

	catalogTotalNumTemp		= 0;
	dataLengthTemp			= 0;
	addrRecord 				= 0;


	readAddrTemp			= dataClassifyInfo[dataClassify].catalogValidStartAddr;

	// 读取位置与有效的结束位置不一致时，进行持续读取
	while(readAddrTemp != dataClassifyInfo[dataClassify].catalogValidEndAddr)
	{
		flashMsg.id 									= EXTFLASH_ID;
		flashMsg.flash.extflashEvent.para.startAddr 	= readAddrTemp;
		flashMsg.flash.extflashEvent.para.length 		= CATALOG_INFO_LENGTH;		
		flashMsg.flash.extflashEvent.para.endAddr 		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
		flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_READ;
		flashMsg.flash.extflashEvent.para.dataAddr 		= (uint8 *)(catalogTemp.databuf);
		
		FlashTask_EventSet(&flashMsg);

		// 判断数据是否有效
		if((catalogTemp.u.dataClassify == dataClassify) && (catalogTemp.u.dataLength != 0x00) && (catalogTemp.u.dataLength != 0xffffffff))
		{

			// 判断该目录存放数据的地址是否在有效的数据范围内
			dataValidStartAddrTemp		= catalogTemp.u.startAddr;
			dataValidEndAddrTemp		= dataValidStartAddrTemp + catalogTemp.u.dataLength;

			// 数据已进行循环存储
			if(dataValidEndAddrTemp > dataClassifyInfo[dataClassify].dataAreaEndAddr)
				dataValidEndAddrTemp	= dataValidEndAddrTemp - dataClassifyInfo[dataClassify].dataAreaEndAddr - 1
											+ dataClassifyInfo[dataClassify].dataAreaStartAddr;
			

			// 写0该目录
			flashMsg.id 									= EXTFLASH_ID;
			flashMsg.flash.extflashEvent.para.startAddr 	= readAddrTemp;
			flashMsg.flash.extflashEvent.para.length 		= CATALOG_INFO_LENGTH;		
			flashMsg.flash.extflashEvent.para.endAddr 		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
			flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_WRITE;
			flashMsg.flash.extflashEvent.para.dataAddr 		= (uint8 *)(catalogWrite0Data.databuf);
			

			// 有效数据是否正向
			if(dataClassifyInfo[dataClassify].dataValidEndAddr >= dataClassifyInfo[dataClassify].dataValidStartAddr)
			{
				// 是否在有效数据范围外
				if((dataValidStartAddrTemp < dataClassifyInfo[dataClassify].dataValidStartAddr) ||
					(dataValidStartAddrTemp >= dataClassifyInfo[dataClassify].dataValidEndAddr) ||
					(dataValidEndAddrTemp <= dataClassifyInfo[dataClassify].dataValidStartAddr) ||
					(dataValidEndAddrTemp > dataClassifyInfo[dataClassify].dataValidEndAddr))
				{
					FlashTask_EventSet(&flashMsg);
					if(readAddrTemp == dataClassifyInfo[dataClassify].catalogValidStartAddr)
					{
						// 更新目录有效地址开始地址
						dataClassifyInfo[dataClassify].catalogValidStartAddr		+= CATALOG_INFO_LENGTH;
						if(dataClassifyInfo[dataClassify].catalogValidStartAddr > dataClassifyInfo[dataClassify].catalogAreaEndAddr)
						{
							dataClassifyInfo[dataClassify].catalogValidStartAddr	= dataClassifyInfo[dataClassify].catalogAreaStartAddr;
						}
					}
				}
			}
			else
			{
				// 是否在有效数据范围外
				if(!((dataValidStartAddrTemp < dataClassifyInfo[dataClassify].dataValidEndAddr) ||
					(dataValidStartAddrTemp >= dataClassifyInfo[dataClassify].dataValidStartAddr) ||
					(dataValidEndAddrTemp <= dataClassifyInfo[dataClassify].dataValidEndAddr) ||
					(dataValidEndAddrTemp > dataClassifyInfo[dataClassify].dataValidStartAddr)))
				{
					FlashTask_EventSet(&flashMsg);
					if(readAddrTemp == dataClassifyInfo[dataClassify].catalogValidStartAddr)
					{
						// 更新目录有效地址开始地址
						dataClassifyInfo[dataClassify].catalogValidStartAddr		+= CATALOG_INFO_LENGTH;
						if(dataClassifyInfo[dataClassify].catalogValidStartAddr > dataClassifyInfo[dataClassify].catalogAreaEndAddr)
						{
							dataClassifyInfo[dataClassify].catalogValidStartAddr	= dataClassifyInfo[dataClassify].catalogAreaStartAddr;
						}
					}
				}
			}

			
			catalogTotalInfo.catalogInfo[catalogTotalNumTemp]	= catalogTemp.u;
			catalogTotalInfo.catalogAddr[catalogTotalNumTemp]	= readAddrTemp;

			catalogTotalNumTemp++;
			dataLengthTemp		+= catalogTemp.u.dataLength;



			if(catalogTotalNumTemp >= CATALOG_TOTAL_MAX_NUM)
				break;
		}
		else
		{
			if(readAddrTemp == dataClassifyInfo[dataClassify].catalogValidStartAddr)
			{
				// 更新目录有效地址开始地址
				dataClassifyInfo[dataClassify].catalogValidStartAddr		+= CATALOG_INFO_LENGTH;
				if(dataClassifyInfo[dataClassify].catalogValidStartAddr > dataClassifyInfo[dataClassify].catalogAreaEndAddr)
				{
					dataClassifyInfo[dataClassify].catalogValidStartAddr	= dataClassifyInfo[dataClassify].catalogAreaStartAddr;
				}
			}
		}


		readAddrTemp += CATALOG_INFO_LENGTH;
		// 超过存储区域从开始地址继续
		if(readAddrTemp > dataClassifyInfo[dataClassify].catalogAreaEndAddr)
			readAddrTemp = dataClassifyInfo[dataClassify].catalogAreaStartAddr;

		addrRecord  += CATALOG_INFO_LENGTH;
		//遍历异常，检测的目录大小超出目录分配区
		if (addrRecord > DM_APP_CATALOG_AREA_LENGTH)
		{
			break;
		}
	}

	//目录条数及数据长度信息获取完毕
	*catalogTotal		= catalogTotalNumTemp;
	*dataLength			= dataLengthTemp;

	catalogTotalInfo.dataClassify		= dataClassify;
	catalogTotalInfo.totalCatalog		= catalogTotalNumTemp;
	if(catalogTotalNumTemp == 0)
		catalogTotalInfo.valid				= 0;
	else
		catalogTotalInfo.valid				= 1;
		
	return 0x00;
}

//**********************************************************************
// 函数功能:	根据目录序号读取目录信息
// 输入参数：	dataInfo:		目录信息指针
// 				dataClassify:	分类数据
// 				catalogNum:		目录序号，不能超过总数量，从0开始
// 返回参数：	0x00: 操作成功
// 				0xff: 操作失败
//**********************************************************************
uint16 Mid_PackData_ClassifyDataCatalogRead(catalogInfo_s *dataInfo, uint32 dataClassify, uint16 catalogNum)
{

	// 数据为加载与总信息缓存
	if(catalogTotalInfo.valid == 0)
		return 0xff;

	// 数据分类与缓存数据不同 
	if(dataClassify != catalogTotalInfo.dataClassify)
		return 0xff;

	// 请求目录序号超出范围
	if(catalogNum >= catalogTotalInfo.totalCatalog)
		return 0xff;

	*dataInfo				= catalogTotalInfo.catalogInfo[catalogNum];

	return 0;
}

//**********************************************************************
// 函数功能:	根据UTC读取数据
// 输入参数：	data:			数据指针
// 				addr:			数据开始相对位置
// 				length:			读取数据长度
// 				dataUtc:		目录UTC指定
// 				dataClassify:	数据分类指定
// 返回参数：	0x00: 操作成功
// 				0xff: 操作失败
//**********************************************************************
uint16  Mid_PackData_ClassifyDataRead(uint8 *data, uint32 addr, uint32 length, uint32 dataUtc, uint32 dataClassify)
{
	uint16	i;
	uint16	hasReadLength;
	uint32	addrTemp;
	flash_task_msg_t		flashMsg;

	// 数据为加载与总信息缓存
	if(catalogTotalInfo.valid == 0)
		return 0xff;

	// 数据分类与缓存数据不同 
	if(dataClassify != catalogTotalInfo.dataClassify)
		return 0xff;

	hasReadLength		= 0;
	// 查找目录相同的数据
	for(i = 0; i < catalogTotalInfo.totalCatalog; i++)
	{
		if(catalogTotalInfo.catalogInfo[i].utc == dataUtc)
		{
			if((length + addr) <= catalogTotalInfo.catalogInfo[i].dataLength)
			{
				// 
				addrTemp = catalogTotalInfo.catalogInfo[i].startAddr + addr;

				// 数据是否跨结束与开始地址
				if((addrTemp + length) >= dataClassifyInfo[dataClassify].dataAreaEndAddr)
				{
					// 若有数据在尾部，先把尾部的数据读取完
					if(addrTemp < dataClassifyInfo[dataClassify].dataAreaEndAddr)
					{
						flashMsg.id 									= EXTFLASH_ID;
						flashMsg.flash.extflashEvent.para.startAddr 	= addrTemp;
						flashMsg.flash.extflashEvent.para.length 		= dataClassifyInfo[dataClassify].dataAreaEndAddr - addrTemp + 1;;		
						flashMsg.flash.extflashEvent.para.endAddr 		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
						flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_READ;
						flashMsg.flash.extflashEvent.para.dataAddr 		= (uint8 *)(data);
						
						FlashTask_EventSet(&flashMsg);
						

						hasReadLength			= flashMsg.flash.extflashEvent.para.length;
					}
					// 跨首尾的数据结束地址,不包含该地址
					addrTemp				= addrTemp + length - dataClassifyInfo[dataClassify].dataAreaEndAddr 
												+ dataClassifyInfo[dataClassify].dataAreaStartAddr - 1;

					flashMsg.id 									= EXTFLASH_ID;
					flashMsg.flash.extflashEvent.para.startAddr 	= addrTemp - (length - hasReadLength);
					flashMsg.flash.extflashEvent.para.length 		= length - hasReadLength;		
					flashMsg.flash.extflashEvent.para.endAddr 		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
					flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_READ;
					flashMsg.flash.extflashEvent.para.dataAddr 		= (uint8 *)(data + hasReadLength);
					
					FlashTask_EventSet(&flashMsg);
				}
				else
				{
					flashMsg.id 									= EXTFLASH_ID;
					flashMsg.flash.extflashEvent.para.startAddr 	= addrTemp;
					flashMsg.flash.extflashEvent.para.length 		= length;		
					flashMsg.flash.extflashEvent.para.endAddr 		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
					flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_READ;
					flashMsg.flash.extflashEvent.para.dataAddr 		= (uint8 *)(data);
					
					FlashTask_EventSet(&flashMsg);
				}
				break;
			}
			else
			{
				// 读取数据超出范围
				return 0xff;
			}
		}
	}
	if(i >= catalogTotalInfo.totalCatalog)
		return 0xff;

	return 0x00;
}

//**********************************************************************
// 函数功能:	根据UTC读取数据
// 输入参数：	data:			数据指针
// 				addr:			数据开始相对位置
// 				length:			读取数据长度
// 				dataUtc:		目录UTC指定
// 				dataClassify:	数据分类指定
// 返回参数：	0x00: 操作成功
// 				0xff: 操作失败
//**********************************************************************
uint16 Mid_PackData_ReadCatalogDataLen(uint32 *length, uint32 dataUtc, uint32 dataClassify)
{
	uint16 i;

	// 数据未加载于总信息缓存
	if(catalogTotalInfo.valid == 0)
		return 0xff;

	// 数据分类与缓存数据不同 
	if(dataClassify != catalogTotalInfo.dataClassify)
		return 0xff;

	for(i = 0; i < catalogTotalInfo.totalCatalog; i++)
	{
		if(catalogTotalInfo.catalogInfo[i].utc == dataUtc)
		{
			*length			= catalogTotalInfo.catalogInfo[i].dataLength;
			return 0x00;
		}

	}
	// 未查找到相关目录
	return 0xff;
}

//**********************************************************************
// 函数功能:	删除所有数据，包括未存储的
// 输入参数：	无
// 返回参数：	0x00: 操作成功
// 				0xff: 操作失败
//**********************************************************************
uint16 Mid_PackData_DeleteTotalData(void)
{
	uint16 i;

	for(i = 0; i < DM_APP_DATA_CLASSIFY_NUM; i++)
	{
		if(dataClassifyInfo[i].init == 1)
		{
			// 目录信息初始化
			dataClassifyInfo[i].occupy						= 0;
			dataClassifyInfo[i].catalogCycle				= 0;
			dataClassifyInfo[i].dataCycle					= 0;

			dataClassifyInfo[i].catalogWritableStartAddr	= dataClassifyInfo[i].catalogAreaStartAddr;
			dataClassifyInfo[i].catalogWritableLength		= 0;
			dataClassifyInfo[i].catalogValidStartAddr		= dataClassifyInfo[i].catalogAreaStartAddr;
			dataClassifyInfo[i].catalogValidEndAddr			= dataClassifyInfo[i].catalogAreaStartAddr;


			// 数据区域分配
			dataClassifyInfo[i].dataWritableStartAddr		= dataClassifyInfo[i].dataAreaStartAddr;
			dataClassifyInfo[i].dataWritableLength			= 0;
			dataClassifyInfo[i].dataValidStartAddr			= dataClassifyInfo[i].dataAreaStartAddr;
			dataClassifyInfo[i].dataValidEndAddr			= dataClassifyInfo[i].dataAreaStartAddr;
		}
	}

	catalogTotalInfo.valid					= 0;
	return 0;
}

//**********************************************************************
// 函数功能:	删除某类数据
// 输入参数：	dataClassify：	分类数据
// 返回参数：	0x00: 操作成功
// 				0xff: 操作失败
//**********************************************************************
uint16 Mid_PackData_DeleteClassifyData(uint32 dataClassify)
{

	if(dataClassifyInfo[dataClassify].init == 1)
	{
		dataClassifyInfo[dataClassify].occupy						= 0;
		dataClassifyInfo[dataClassify].catalogCycle					= 0;
		dataClassifyInfo[dataClassify].dataCycle					= 0;

		dataClassifyInfo[dataClassify].catalogWritableStartAddr		= dataClassifyInfo[dataClassify].catalogAreaStartAddr;
		dataClassifyInfo[dataClassify].catalogWritableLength		= 0;
		dataClassifyInfo[dataClassify].catalogValidStartAddr		= dataClassifyInfo[dataClassify].catalogAreaStartAddr;
		dataClassifyInfo[dataClassify].catalogValidEndAddr			= dataClassifyInfo[dataClassify].catalogAreaStartAddr;


		// 数据区域分配
		dataClassifyInfo[dataClassify].dataWritableStartAddr		= dataClassifyInfo[dataClassify].dataAreaStartAddr;
		dataClassifyInfo[dataClassify].dataWritableLength			= 0;
		dataClassifyInfo[dataClassify].dataValidStartAddr			= dataClassifyInfo[dataClassify].dataAreaStartAddr;
		dataClassifyInfo[dataClassify].dataValidEndAddr				= dataClassifyInfo[dataClassify].dataAreaStartAddr;
	}
	else
	{
		return 0xff;
	}

	// 清除该数据缓存
	if(catalogTotalInfo.dataClassify == dataClassify)
		catalogTotalInfo.valid					= 0;

	return 0;
}

//**********************************************************************
// 函数功能:	删除指定目录
// 输入参数：	dataClassify:	分类数据
// 				utc:			目录UTC
// 返回参数：	0x00: 操作成功
// 				0xff: 操作失败
//**********************************************************************
uint16 Mid_PackData_DeleteClassifyDataUtc(uint32 dataClassify, uint32 utc)
{
	uint16 i;
	flash_task_msg_t		flashMsg;

	// 缓存是否放置了该类数据
	if((catalogTotalInfo.dataClassify == dataClassify) && (catalogTotalInfo.valid == 1))
	{
		for(i = 0; i < catalogTotalInfo.totalCatalog; i++)
		{
			if(catalogTotalInfo.catalogInfo[i].utc == utc)
			{			
				flashMsg.id 									= EXTFLASH_ID;
				flashMsg.flash.extflashEvent.para.startAddr 	= catalogTotalInfo.catalogAddr[i];
				flashMsg.flash.extflashEvent.para.length 		= CATALOG_INFO_LENGTH;		
				flashMsg.flash.extflashEvent.para.endAddr 		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
				flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_WRITE;
				flashMsg.flash.extflashEvent.para.dataAddr 		= (uint8 *)(catalogWrite0Data.databuf);
				
				FlashTask_EventSet(&flashMsg);

				return 0;
			}
		}
	}

	// 缓存中为有相同的目录
	return 0xff;
}

//**********************************************************************
// 函数功能:	由于不正常原因掉电，数据恢复前需要先分配区域，按照分配的区域进行数据恢复
// 				数据存储区域若掉电前和掉电后分配区域有不会则会造成恢复数据错误，升级可做
// 				正常的数据恢复，不调用该函数。
// 输入参数：	none
// 返回参数：	0x00: 操作成功
// 				0xff: 操作失败
//**********************************************************************
uint16 Mid_PackData_StorageDataRecover(void)
{
	uint16 i, j;
	uint32 addrTemp, addrForwardTemp;
	uint32 earlyCatalogAddr;
	uint32 lastCatalogAddr;
	uint32 earlyDataAddr;
	uint32 lastDataAddr;
	uint32 		addrRecord;
	flash_task_msg_t		flashMsg;

	catalogInfo_s	earlyCatalog;
	catalogInfo_s	lastCatalog;
	catalogInfo_u 	catalogTemp;


	addrRecord 		= 0;

	// 区域未进行分配
	if(catalogAreaManage.validLength == DM_APP_CATALOG_AREA_LENGTH)
		return 0xff;

	for(i = 0; i < DM_APP_DATA_CLASSIFY_NUM; i++)
	{
		lastCatalog.utc		= 0;
		earlyCatalog.utc	= 0xffffffff;
		if(dataClassifyInfo[i].init == 1)
		{
			addrTemp		= dataClassifyInfo[i].catalogAreaStartAddr;
			while(addrTemp < dataClassifyInfo[i].catalogAreaEndAddr)
			{
                flashMsg.id 									= EXTFLASH_ID;
				flashMsg.flash.extflashEvent.para.startAddr 	= addrTemp;
				flashMsg.flash.extflashEvent.para.length 		= CATALOG_INFO_LENGTH;		
				flashMsg.flash.extflashEvent.para.endAddr 		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
				flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_READ;
				flashMsg.flash.extflashEvent.para.dataAddr 		= (uint8 *)(catalogTemp.databuf);
				
				FlashTask_EventSet(&flashMsg);

				// 判断存储的数据是否有效,不全的目录进行丢弃
				if((catalogTemp.u.dataClassify == i) && ((catalogTemp.u.dataLength != 0) && (catalogTemp.u.dataLength != 0xffffffff)))
				{
					if(lastCatalog.utc < catalogTemp.u.utc)
					{
						lastCatalog		= catalogTemp.u;
						lastCatalogAddr	= addrTemp;
					}
					if(earlyCatalog.utc > catalogTemp.u.utc)
					{
						earlyCatalog		= catalogTemp.u;
						earlyCatalogAddr	= addrTemp;
					}
				}
				else
				{
					// 无效则判断是否有被擦除的痕迹
					for(j = 0; j < CATALOG_INFO_LENGTH; j++)
					{
						if(catalogTemp.databuf[i] != 0)
							break;
					}
					if(j < CATALOG_INFO_LENGTH)
						break;
					
				}
				addrTemp	+= CATALOG_INFO_LENGTH;
				addrForwardTemp	= addrTemp;
				addrRecord  += CATALOG_INFO_LENGTH;
				//遍历异常，检测的目录大小超出目录分配区
				if (addrRecord > DM_APP_CATALOG_AREA_LENGTH)
				{
					break;
				}
			}

			addrRecord = 0;
			// 进行反向查找目录
			addrTemp		= dataClassifyInfo[i].catalogAreaEndAddr - CATALOG_INFO_LENGTH + 1;
			while(addrTemp > addrForwardTemp)
			{
				flashMsg.id 									= EXTFLASH_ID;
				flashMsg.flash.extflashEvent.para.startAddr 	= addrTemp;
				flashMsg.flash.extflashEvent.para.length 		= CATALOG_INFO_LENGTH;		
				flashMsg.flash.extflashEvent.para.endAddr 		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
				flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_READ;
				flashMsg.flash.extflashEvent.para.dataAddr 		= (uint8 *)(catalogTemp.databuf);
				
				FlashTask_EventSet(&flashMsg);
				

				// 判断存储的数据是否有效,不全的目录进行丢弃
				if((catalogTemp.u.dataClassify == i) && ((catalogTemp.u.dataLength != 0) && (catalogTemp.u.dataLength != 0xffffffff)))
				{
					if(lastCatalog.utc < catalogTemp.u.utc)
					{
						lastCatalog		= catalogTemp.u;
						lastCatalogAddr	= addrTemp;
					}
					if(earlyCatalog.utc > catalogTemp.u.utc)
					{
						earlyCatalog		= catalogTemp.u;
						earlyCatalogAddr	= addrTemp;
					}
				}
				else
				{
					// 无效则判断是否有被擦除的痕迹
					for(j = 0; j < CATALOG_INFO_LENGTH; j++)
					{
						if(catalogTemp.databuf[i] != 0)
							break;
					}
					if(j < CATALOG_INFO_LENGTH)
						break;
				}
				addrTemp	-= CATALOG_INFO_LENGTH;

				addrRecord  += CATALOG_INFO_LENGTH;
				//遍历异常，检测的目录大小超出目录分配区
				if (addrRecord > DM_APP_CATALOG_AREA_LENGTH)
				{
					break;
				}
			}

			if(lastCatalog.utc == 0)
			{
				// 无数据可进行恢复
			}
			else
			{
				// 目录区域恢复
				if(lastCatalogAddr >= earlyCatalogAddr)
				{
					// 未跨结束与开始地址
					dataClassifyInfo[i].catalogCycle	= 0;
				}
				else
				{
					// 跨结束与开始地址
					dataClassifyInfo[i].catalogCycle	= 1;
				}

				dataClassifyInfo[i].catalogValidStartAddr	= earlyCatalogAddr;

				dataClassifyInfo[i].catalogValidEndAddr		= lastCatalogAddr + CATALOG_INFO_LENGTH;

				// 有效目录地址越界
				if(dataClassifyInfo[i].catalogValidEndAddr > dataClassifyInfo[i].catalogAreaEndAddr)
						dataClassifyInfo[i].catalogValidEndAddr		= dataClassifyInfo[i].catalogAreaStartAddr;

				// 目录结束位置是否刚好为块开始地址
				if((dataClassifyInfo[i].catalogValidEndAddr % DM_APP_SECTOR_LENGTH) == 0)
				{
					dataClassifyInfo[i].catalogWritableStartAddr	= dataClassifyInfo[i].catalogValidEndAddr;
					dataClassifyInfo[i].catalogWritableLength	= 0;
				}
				else
				{
					// 丢弃下一个目录，可能目录不完整,可进行部分恢复，根据需求做修改
					dataClassifyInfo[i].catalogWritableStartAddr	= dataClassifyInfo[i].catalogValidEndAddr + CATALOG_INFO_LENGTH;

					if(dataClassifyInfo[i].catalogWritableStartAddr > dataClassifyInfo[i].catalogAreaEndAddr)
						dataClassifyInfo[i].catalogWritableStartAddr		= dataClassifyInfo[i].catalogAreaStartAddr;

					// 是否刚好跨块
					if((dataClassifyInfo[i].catalogWritableStartAddr % DM_APP_SECTOR_LENGTH) == 0)
					{
						dataClassifyInfo[i].catalogWritableLength	= 0;
					}
					else
					{
						dataClassifyInfo[i].catalogWritableLength = 
							DM_APP_SECTOR_LENGTH - (dataClassifyInfo[i].catalogWritableStartAddr%DM_APP_SECTOR_LENGTH);
					}

				}

				// 数据区域恢复
				lastDataAddr	= lastCatalog.startAddr + lastCatalog.dataLength;
				earlyDataAddr	= earlyCatalog.startAddr;

				if(lastDataAddr > dataClassifyInfo[i].dataAreaEndAddr)
					lastDataAddr -= dataClassifyInfo[i].dataAreaEndAddr + dataClassifyInfo[i].dataAreaStartAddr;

				if(lastDataAddr >= earlyDataAddr)
				{
					// 未跨开始与结束地址
					dataClassifyInfo[i].dataCycle	= 0;
				}
				else
				{
					// 跨开始与结束地址
					dataClassifyInfo[i].dataCycle	= 1;
				}

				dataClassifyInfo[i].dataValidStartAddr	= earlyDataAddr;
				dataClassifyInfo[i].dataValidEndAddr	= lastDataAddr;

				// 该片区为未写区域均丢弃
				dataClassifyInfo[i].dataWritableStartAddr	= 
					lastDataAddr - (lastDataAddr % DM_APP_SECTOR_LENGTH) + DM_APP_SECTOR_LENGTH;

				if(dataClassifyInfo[i].dataWritableStartAddr > dataClassifyInfo[i].dataAreaEndAddr)
					dataClassifyInfo[i].dataWritableStartAddr	= dataClassifyInfo[i].dataAreaStartAddr;

				dataClassifyInfo[i].dataWritableLength		= 0;
			}
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 函数功能:	获取数据类型的总目录数及长度
// 输入参数：	catalogTotal:	目录总数指针，
// 				dataLength:		数据总数指针
// 				dataClassify:	分类数据
// 返回参数：	0x00: 操作成功
// 				0xff: 操作失败
uint16_t ClassifyTotalCatalogRead(uint16_t *catalogTotal, uint32_t *dataLength, uint32_t dataClassify,uint32_t *catalogvalidstartaddr)
{
	flash_task_msg_t		flashMsg;
	catalogInfo_u			catalogTemp;
	uint16_t				catalogTotalNumTemp;
	uint32_t				dataLengthTemp;
	uint32_t				readAddrTemp;
	uint32_t				dataValidStartAddrTemp;
	uint32_t				dataValidEndAddrTemp;

	// 检查数据是否合法
	if(dataClassify >= DM_APP_DATA_CLASSIFY_NUM)
		return 0xff;

	// 检查是否分配区域了
	if(dataClassifyInfo[dataClassify].init == 0)
		return 0xff;

	catalogTotalNumTemp		= 0;
	dataLengthTemp			= 0;


	readAddrTemp			= dataClassifyInfo[dataClassify].catalogValidStartAddr;

	// 读取位置与有效的结束位置不一致时，进行持续读取
	while(readAddrTemp != dataClassifyInfo[dataClassify].catalogValidEndAddr)
	{
		flashMsg.id 									= EXTFLASH_ID;
		flashMsg.flash.extflashEvent.para.startAddr	= readAddrTemp;
		flashMsg.flash.extflashEvent.para.length		= CATALOG_INFO_LENGTH;
		flashMsg.flash.extflashEvent.para.endAddr		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
		flashMsg.flash.extflashEvent.para.dataAddr	= (uint8_t *)(catalogTemp.databuf);
		flashMsg.flash.extflashEvent.id			= EXTFLASH_EVENT_READ;
		FlashTask_EventSet(&flashMsg);

		// 判断数据是否有效
		if((catalogTemp.u.dataClassify == dataClassify) && (catalogTemp.u.dataLength != 0x00) && (catalogTemp.u.dataLength != 0xffffffff))
		{

			// 判断该目录存放数据的地址是否在有效的数据范围内
			dataValidStartAddrTemp		= catalogTemp.u.startAddr;
			dataValidEndAddrTemp		= dataValidStartAddrTemp + catalogTemp.u.dataLength;

			// 数据已进行循环存储,超出区域部分从数据区域头部分存入
			if(dataValidEndAddrTemp > dataClassifyInfo[dataClassify].dataAreaEndAddr)
				dataValidEndAddrTemp	= dataValidEndAddrTemp - dataClassifyInfo[dataClassify].dataAreaEndAddr - 1
											+ dataClassifyInfo[dataClassify].dataAreaStartAddr;

			//目录条数及数据长度累加
			catalogTotalNumTemp++;
			dataLengthTemp		+= catalogTemp.u.dataLength;
		}
		

		//目录遍历地址后移
		readAddrTemp += CATALOG_INFO_LENGTH;
		// 超过存储区域从开始地址继续
		if(readAddrTemp > dataClassifyInfo[dataClassify].catalogAreaEndAddr)
			readAddrTemp = dataClassifyInfo[dataClassify].catalogAreaStartAddr;
	}
		
	//目录条数及数据长度信息获取完毕
	*catalogTotal				= catalogTotalNumTemp;
	*dataLength					= dataLengthTemp;
	*catalogvalidstartaddr 		= dataClassifyInfo[dataClassify].catalogValidStartAddr;
	
	return 0x00;
}

//**********************************************************************
// 函数功能:	获取目录信息
// 输入参数：	dataInfo:		目录指针，
// 				dataClassify:	分类数据
// 				catalogAddr:    目录存储地址
// 返回参数：	0x00: 操作成功
// 				0xff: 操作失败
//**********************************************************************
uint16_t ClassifyCatalogInfoRead(catalogInfo_s *dataInfo, uint32_t dataClassify,uint32_t catalogAddr)
{
	flash_task_msg_t		flashMsg;
	catalogInfo_u			catalogTemp;
	uint32_t				readAddrTemp;
	uint32_t				dataValidStartAddrTemp;
	uint32_t				dataValidEndAddrTemp;

	// 检查数据是否合法
	if(dataClassify >= DM_APP_DATA_CLASSIFY_NUM)
		return 0xff;

	// 检查是否分配区域了
	if(dataClassifyInfo[dataClassify].init == 0)
		return 0xff;



	readAddrTemp			= catalogAddr;

	// 读取位置与有效的结束位置不一致时，进行持续读取
	if(readAddrTemp != dataClassifyInfo[dataClassify].catalogValidEndAddr)
	{
		flashMsg.id 									= EXTFLASH_ID;
		flashMsg.flash.extflashEvent.para.startAddr	= readAddrTemp;
		flashMsg.flash.extflashEvent.para.length	= CATALOG_INFO_LENGTH;
		flashMsg.flash.extflashEvent.para.endAddr	= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
		flashMsg.flash.extflashEvent.id				= EXTFLASH_EVENT_READ;
		flashMsg.flash.extflashEvent.para.dataAddr	= (uint8_t *)(catalogTemp.databuf);
		FlashTask_EventSet(&flashMsg);


		// 判断数据是否有效
		if((catalogTemp.u.dataClassify == dataClassify) && (catalogTemp.u.dataLength != 0x00) && (catalogTemp.u.dataLength != 0xffffffff))
		{

			// 判断该目录存放数据的地址是否在有效的数据范围内
			dataValidStartAddrTemp		= catalogTemp.u.startAddr;
			dataValidEndAddrTemp		= dataValidStartAddrTemp + catalogTemp.u.dataLength;

			// 数据已进行循环存储,超出区域部分从数据区域头部分存入
			if(dataValidEndAddrTemp > dataClassifyInfo[dataClassify].dataAreaEndAddr)
				dataValidEndAddrTemp	= dataValidEndAddrTemp - dataClassifyInfo[dataClassify].dataAreaEndAddr - 1
											+ dataClassifyInfo[dataClassify].dataAreaStartAddr;
			*dataInfo	= catalogTemp.u;											
		}	
	}
		
	return 0x00;
}

//**********************************************************************
// 函数功能:	根据目录地址读取数据
// 输入参数：	data:			数据指针
// 				addr:			数据开始相对位置
// 				length:			读取数据长度
// 返回参数：	0x00: 操作成功
// 				0xff: 操作失败
//**********************************************************************
uint16_t  ClassifyDataPreRead(uint8_t *data, uint32_t addr, uint32_t length, uint32_t dataClassify)
{
	uint16_t	hasReadLength;
	uint32_t	addrTemp;
	flash_task_msg_t		flashMsg;


	hasReadLength		= 0;

	addrTemp = addr;//偏移地址确定

	// 数据是否跨结束与开始地址
	if((addrTemp + length) >= dataClassifyInfo[dataClassify].dataAreaEndAddr)
	{
		// 若有数据在尾部，先把尾部的数据读取完
		if(addrTemp < dataClassifyInfo[dataClassify].dataAreaEndAddr)
		{
			flashMsg.id 									= EXTFLASH_ID;
			flashMsg.flash.extflashEvent.para.startAddr	= addrTemp;
			flashMsg.flash.extflashEvent.para.length		= dataClassifyInfo[dataClassify].dataAreaEndAddr - addrTemp + 1;
			flashMsg.flash.extflashEvent.para.endAddr		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
			flashMsg.flash.extflashEvent.para.dataAddr	= (uint8_t *)(data);
			flashMsg.flash.extflashEvent.id			= EXTFLASH_EVENT_READ;
			
			FlashTask_EventSet(&flashMsg);

			hasReadLength			= flashMsg.flash.extflashEvent.para.length;
		}
		// 跨首尾的数据结束地址,不包含该地址，数据段内循环读取
		addrTemp				= addrTemp + length - dataClassifyInfo[dataClassify].dataAreaEndAddr 
									+ dataClassifyInfo[dataClassify].dataAreaStartAddr - 1;

		flashMsg.flash.extflashEvent.para.startAddr	= addrTemp - (length - hasReadLength);
		flashMsg.flash.extflashEvent.para.length		= length - hasReadLength;
		flashMsg.flash.extflashEvent.para.endAddr		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
		flashMsg.flash.extflashEvent.para.dataAddr	= (uint8_t *)(data + hasReadLength);
		flashMsg.flash.extflashEvent.id			= EXTFLASH_EVENT_READ;
		FlashTask_EventSet(&flashMsg);
	}
	else
	{
		flashMsg.id 									= EXTFLASH_ID;
		flashMsg.flash.extflashEvent.para.startAddr	= addrTemp;
		flashMsg.flash.extflashEvent.para.length		= length;
		flashMsg.flash.extflashEvent.para.endAddr		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
		flashMsg.flash.extflashEvent.para.dataAddr	= (uint8_t *)(data);
		flashMsg.flash.extflashEvent.id			= EXTFLASH_EVENT_READ;
		FlashTask_EventSet(&flashMsg);
	}
	return 0x00;
}


