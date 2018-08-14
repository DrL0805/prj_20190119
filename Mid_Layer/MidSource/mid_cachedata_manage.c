#include "platform_common.h"
/* FreeRTOS includes */
#include "rtos.h"
#include "multimodule_task.h"
#include "mid_extflash.h"
#include "flash_task.h"
#include "mid_cachedata_manage.h"


// ***********************************************************************
//macro define
#define			TIME_CACHE_BASE_ADDR			0xA000	//分配扇区的基地址
#define			TIME_CACHE_ID_ADDR				0xA000 //时间类型数据标识符地址
#define			TIME_CACHE_VER_ADDR				0xA010 //时间类型数据版本地址
#define			TIME_CACHE_START_ADDR			0xA020 //时间类型数据信息存储的起始地址

#define			ALARM_CACHE_BASE_ADDR			0xB000
#define			ALARM_CACHE_ID_ADDR				0xB000
#define			ALARM_CACHE_VER_ADDR			0xB010
#define			ALARM_CACHE_START_ADDR			0xB020	

#define			SPORT_CACHE_BASE_ADDR			0xC000
#define			SPORT_CACHE_ID_ADDR				0xC000
#define			SPORT_CACHE_VER_ADDR			0xC010
#define			SPORT_CACHE_START_ADDR			0xC020		

#define			PACK_CACHE_BASE_ADDR			0xD000
#define			PACK_CACHE_ID_ADDR				0xD000
#define			PACK_CACHE_VER_ADDR				0xD010
#define			PACK_CACHE_START_ADDR			0xD020	

#define			SYSTERM_CACHE_BASE_ADDR			0xE000
#define			SYSTERM_CACHE_ID_ADDR			0xE000
#define			SYSTERM_CACHE_VER_ADDR			0xE010
#define			SYSTERM_CACHE_START_ADDR		0xE020	

#define			SCENE_CACHE_BASE_ADDR			0xF000
#define			SCENE_CACHE_ID_ADDR				0xF000
#define			SCENE_CACHE_VER_ADDR			0xF010
#define			SCENE_CACHE_START_ADDR			0xF020		


// ***********************************************************************
static const uint8 TIME_CACHE_ID[16]		= 
{
	't', 'i', 'm', 'e',
};


static const uint8 ALARM_CACHE_ID[16]		= 
{
	'a', 'l', 'a', 'r','m',
};

static const uint8 SPORT_CACHE_ID[16]		= 
{
	's', 'p', 'o', 'r', 't',
};

static const uint8 PACK_CACHE_ID[16]		= 
{
	'p', 'a', 'c', 'k',
};

static const uint8 SYSTERM_CACHE_ID[16]		= 
{
	's', 'y', 's', 't', 'e', 'r', 'm',
};

static const uint8 SCENE_CACHE_ID[16]		= 
{
	's', 'c', 'e', 'n', 'e',
};



// ********************************function define***************************************

//**********************************************************************
// 函数功能:	擦除缓存数据块
// 输入参数：	无
// 返回参数：	0x00:	操作成功
// 				0xff:	操作失败失败
//**********************************************************************
uint16 Mid_CacheDataClear(cache_data_e cacheType)
{
	flash_task_msg_t		flashMsg;


	flashMsg.id 	= EXTFLASH_ID;
	switch(cacheType)
	{
		case CACHE_TIME_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr 	= TIME_CACHE_BASE_ADDR;
		break;

		case CACHE_ALARM_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr 	= ALARM_CACHE_BASE_ADDR;
		break;

		case CACHE_SPORT_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr 	= SPORT_CACHE_BASE_ADDR;
		break;

		case CACHE_PACK_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr 	= PACK_CACHE_BASE_ADDR;
		break;

		case CACHE_SYSTERM_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr 	= SYSTERM_CACHE_BASE_ADDR;
		break;

		case CACHE_SCENE_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr 	= SCENE_CACHE_BASE_ADDR;
		break;

		default:
		return 0xff;
	}

	flashMsg.flash.extflashEvent.para.dataAddr 		= (uint8*)(NULL);
	flashMsg.flash.extflashEvent.para.length 		= 4096;		
	flashMsg.flash.extflashEvent.para.endAddr 		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
	flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_4K_ERASE;	
	flashMsg.flash.extflashEvent.para.result		= FlashTask_EventSet(&flashMsg);

	return flashMsg.flash.extflashEvent.para.result;
}	

//**********************************************************************
// 函数功能:	写入对应数据类型的版本
// 输入参数：	version:	数据缓存版本
// 				cacheType:	数据缓存分类
// 返回参数：	0x00:	操作成功
// 				0xff:	操作失败失败
//**********************************************************************
uint16 Mid_WriteDataCacheVer(uint32 version, cache_data_e cacheType)
{
	flash_task_msg_t		flashMsg;

	switch(cacheType)
	{
		case CACHE_TIME_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr = TIME_CACHE_VER_ADDR;
		break;

		case CACHE_ALARM_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr = ALARM_CACHE_VER_ADDR;
		break;

		case CACHE_SPORT_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr = SPORT_CACHE_VER_ADDR;
		break;

		case CACHE_PACK_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr = PACK_CACHE_VER_ADDR;
		break;

		case CACHE_SYSTERM_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr = SYSTERM_CACHE_VER_ADDR;
		break;

		case CACHE_SCENE_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr = SCENE_CACHE_VER_ADDR;
		break;

		default:
		return 0xff;
	}
	flashMsg.id 	= EXTFLASH_ID;
	flashMsg.flash.extflashEvent.para.dataAddr 		= (uint8*)(version);//低位在前
	flashMsg.flash.extflashEvent.para.length 		= 16;		
	flashMsg.flash.extflashEvent.para.endAddr 		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
	flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_WRITE;	
	flashMsg.flash.extflashEvent.para.result 		= 0;
	flashMsg.flash.extflashEvent.para.result		= FlashTask_EventSet(&flashMsg);

	return flashMsg.flash.extflashEvent.para.result;
}

//**********************************************************************
// 函数功能:	读取对应数据类型的版本
// 输入参数：	version:	数据缓存版本
// 				cacheType:	数据缓存分类
// 返回参数：	0x00:	操作成功
// 				0xff:	操作失败
//**********************************************************************
uint16 Mid_ReadDataCacheVer(uint32 *version, cache_data_e cacheType)
{
	flash_task_msg_t		flashMsg;

	flashMsg.id 	= EXTFLASH_ID;
	switch(cacheType)
	{
		case CACHE_TIME_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr 	= TIME_CACHE_VER_ADDR;
		break;

		case CACHE_ALARM_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr 	= ALARM_CACHE_VER_ADDR;
		break;

		case CACHE_SPORT_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr 	= SPORT_CACHE_VER_ADDR;
		break;

		case CACHE_PACK_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr 	= PACK_CACHE_VER_ADDR;
		break;

		case CACHE_SYSTERM_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr 	= SYSTERM_CACHE_VER_ADDR;
		break;

		case CACHE_SCENE_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr 	= SCENE_CACHE_VER_ADDR;
		break;

		default:
		return 0xff;
	}
	flashMsg.flash.extflashEvent.para.dataAddr 		= (uint8*)(version);
	flashMsg.flash.extflashEvent.para.length 		= 4;		
	flashMsg.flash.extflashEvent.para.endAddr 		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
	flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_READ;	
	flashMsg.flash.extflashEvent.para.result 		= 0;
	flashMsg.flash.extflashEvent.para.result		= FlashTask_EventSet(&flashMsg);

	return flashMsg.flash.extflashEvent.para.result;
}

//**********************************************************************
// 函数功能:	写入对应数据类型的标识
// 输入参数：	version:	数据缓存版本
// 				cacheType:	数据缓存分类
// 返回参数：	0x00:	操作成功
// 				0xff:	操作失败失败
//**********************************************************************
uint16 Mid_WriteDataCacheID(cache_data_e cacheType)
{
	flash_task_msg_t		flashMsg;

	flashMsg.id 	= EXTFLASH_ID;
	switch(cacheType)
	{
		case CACHE_TIME_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr 	= TIME_CACHE_ID_ADDR;
		flashMsg.flash.extflashEvent.para.dataAddr 		= (uint8*)(TIME_CACHE_ID);
		break;

		case CACHE_ALARM_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr 	= ALARM_CACHE_ID_ADDR;
		flashMsg.flash.extflashEvent.para.dataAddr 		= (uint8*)(ALARM_CACHE_ID);
		break;

		case CACHE_SPORT_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr 	= SPORT_CACHE_ID_ADDR;
		flashMsg.flash.extflashEvent.para.dataAddr 		= (uint8*)(SPORT_CACHE_ID);
		break;

		case CACHE_PACK_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr 	= PACK_CACHE_ID_ADDR;
		flashMsg.flash.extflashEvent.para.dataAddr 		= (uint8*)(PACK_CACHE_ID);
		break;

		case CACHE_SYSTERM_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr 	= SYSTERM_CACHE_ID_ADDR;
		flashMsg.flash.extflashEvent.para.dataAddr 		= (uint8*)(SYSTERM_CACHE_ID);
		break;

		case CACHE_SCENE_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr 	= SCENE_CACHE_ID_ADDR;
		flashMsg.flash.extflashEvent.para.dataAddr 		= (uint8*)(SCENE_CACHE_ID);
		break;

		default:
		return 0xff;
	}
	flashMsg.flash.extflashEvent.para.length 		= 16;		
	flashMsg.flash.extflashEvent.para.endAddr 		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
	flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_WRITE;	
	flashMsg.flash.extflashEvent.para.result 		= 0;
	flashMsg.flash.extflashEvent.para.result		= FlashTask_EventSet(&flashMsg);

	return flashMsg.flash.extflashEvent.para.result;
}

//**********************************************************************
// 函数功能:	读取对应数据类型的标识并判断
// 输入参数：	version:	数据缓存版本
// 				cacheType:	数据缓存分类
// 返回参数：	0x00:	操作成功
// 				0xff:	操作失败失败
//**********************************************************************
uint16 Mid_ReadDataCacheID(cache_data_e cacheType)
{
	uint8				idTemp[16];
	flash_task_msg_t	flashMsg;
	uint8 				i;

	flashMsg.id 	= EXTFLASH_ID;
	switch(cacheType)
	{
		case CACHE_TIME_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr 	= TIME_CACHE_ID_ADDR;
		break;

		case CACHE_ALARM_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr 	= ALARM_CACHE_ID_ADDR;
		break;

		case CACHE_SPORT_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr 	= SPORT_CACHE_ID_ADDR;
		break;

		case CACHE_PACK_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr 	= PACK_CACHE_ID_ADDR;
		break;

		case CACHE_SYSTERM_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr 	= SYSTERM_CACHE_ID_ADDR;
		break;

		case CACHE_SCENE_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr 	= SCENE_CACHE_ID_ADDR;
		break;

		default:
		return 0xff;
	}

	flashMsg.id 									= EXTFLASH_ID;
	flashMsg.flash.extflashEvent.para.dataAddr 		= idTemp;
	flashMsg.flash.extflashEvent.para.length 		= 16;		
	flashMsg.flash.extflashEvent.para.endAddr 		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
	flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_READ;	
	flashMsg.flash.extflashEvent.para.result 		= 0;
	flashMsg.flash.extflashEvent.para.result		= FlashTask_EventSet(&flashMsg);
	
	if (flashMsg.flash.extflashEvent.para.result)
	{
		return 0xff;
	}

	switch(cacheType)
	{
		case CACHE_TIME_TYPE:
		for(i = 0; i < 4; i++)
		{
			if(idTemp[i] != TIME_CACHE_ID[i])
				return 0xff;
		}
		break;

		case CACHE_ALARM_TYPE:
		for(i = 0; i < 5; i++)
		{
			if(idTemp[i] != ALARM_CACHE_ID[i])
				return 0xff;
		}
		break;

		case CACHE_SPORT_TYPE:
		for(i = 0; i < 5; i++)
		{
			if(idTemp[i] != SPORT_CACHE_ID[i])
				return 0xff;
		}
		break;

		case CACHE_PACK_TYPE:
		for(i = 0; i < 4; i++)
		{
			if(idTemp[i] != PACK_CACHE_ID[i])
				return 0xff;
		}
		break;

		case CACHE_SYSTERM_TYPE:
		for(i = 0; i < 7; i++)
		{
			if(idTemp[i] != SYSTERM_CACHE_ID[i])
				return 0xff;
		}
		break;

		case CACHE_SCENE_TYPE:
		for(i = 0; i < 5; i++)
		{
			if(idTemp[i] != SCENE_CACHE_ID[i])
				return 0xff;
		}
		break;
	}

	return flashMsg.flash.extflashEvent.para.result;
}

//**********************************************************************
// 函数功能:	写入对应数据类型的数据缓存
// 输入参数：	dataBuf:	写入的数据指针
// 				offsetAddr:	偏移地址(相对于信息存储开始地址的偏移)
// 				length:		buf长度
// 				cacheType:	缓存数据类型
// 返回参数：	0x00:	操作成功
// 				0xff:	操作失败
//**********************************************************************
uint16 Mid_WriteDataCache(uint8 *dataBuf, uint32 offsetAddr, uint16 length, cache_data_e cacheType)
{
	flash_task_msg_t		flashMsg;

	switch(cacheType)
	{
		case CACHE_TIME_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr 	= TIME_CACHE_START_ADDR  + offsetAddr;
		break;

		case CACHE_ALARM_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr 	= ALARM_CACHE_START_ADDR  + offsetAddr;
		break;

		case CACHE_SPORT_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr 	= SPORT_CACHE_START_ADDR  + offsetAddr;
		break;

		case CACHE_PACK_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr 	= PACK_CACHE_START_ADDR  + offsetAddr;
		break;

		case CACHE_SYSTERM_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr 	= SYSTERM_CACHE_START_ADDR  + offsetAddr;
		break;

		case CACHE_SCENE_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr 	= SCENE_CACHE_START_ADDR  + offsetAddr;
		break;

		default:
		return 0xff;
	}

	flashMsg.id 									= EXTFLASH_ID;
	flashMsg.flash.extflashEvent.para.dataAddr 		= dataBuf;
	flashMsg.flash.extflashEvent.para.length 		= length;		
	flashMsg.flash.extflashEvent.para.endAddr 		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
	flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_WRITE;	
	flashMsg.flash.extflashEvent.para.result 		= 0;
	flashMsg.flash.extflashEvent.para.result		= FlashTask_EventSet(&flashMsg);
	
	return flashMsg.flash.extflashEvent.para.result;
}

//**********************************************************************
// 函数功能:	读取对应数据类型的数据缓存
// 输入参数：	dataBuf:	缓存数据指针
// 				offsetAddr:	偏移地址
// 				length:		buf长度
// 				cacheType:	缓存数据类型
// 返回参数：	0x00:	操作成功
// 				0xff:	操作失败
//**********************************************************************
uint16 Mid_ReadDataCache(uint8 *dataBuf, uint32 offsetAddr, uint16 length, cache_data_e cacheType)
{
	flash_task_msg_t		flashMsg;

	switch(cacheType)
	{
		case CACHE_TIME_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr = TIME_CACHE_START_ADDR  + offsetAddr;
		break;

		case CACHE_ALARM_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr = ALARM_CACHE_START_ADDR  + offsetAddr;
		break;

		case CACHE_SPORT_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr 	= SPORT_CACHE_START_ADDR  + offsetAddr;
		break;

		case CACHE_PACK_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr 	= PACK_CACHE_START_ADDR  + offsetAddr;
		break;

		case CACHE_SYSTERM_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr 	= SYSTERM_CACHE_START_ADDR  + offsetAddr;
		break;

		case CACHE_SCENE_TYPE:
		flashMsg.flash.extflashEvent.para.startAddr 	= SCENE_CACHE_START_ADDR  + offsetAddr;
		break;

		default:
		return 0xff;
	}

	flashMsg.id 									= EXTFLASH_ID;
	flashMsg.flash.extflashEvent.para.dataAddr 		= dataBuf;
	flashMsg.flash.extflashEvent.para.length 		= length;
	flashMsg.flash.extflashEvent.para.endAddr 		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
	flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_READ;
	flashMsg.flash.extflashEvent.para.result 		= 0;
	flashMsg.flash.extflashEvent.para.result		= FlashTask_EventSet(&flashMsg);
	

	return flashMsg.flash.extflashEvent.para.result;
}




