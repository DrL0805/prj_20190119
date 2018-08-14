#ifndef		MID_CACHEDATA_MANAGE_H
#define		MID_CACHEDATA_MANAGE_H

#include "platform_common.h"


// 缓存分配
typedef enum
{
	CACHE_TIME_TYPE			= 0x0001,
	CACHE_ALARM_TYPE,
	CACHE_SPORT_TYPE,
	CACHE_PACK_TYPE,
	CACHE_SYSTERM_TYPE,
	CACHE_SCENE_TYPE,
}cache_data_e;


uint16 Mid_CacheDataClear(cache_data_e cacheType);
uint16 Mid_WriteDataCacheVer(uint32 version, cache_data_e cacheType);
uint16 Mid_ReadDataCacheVer(uint32 *version, cache_data_e cacheType);
uint16 Mid_WriteDataCacheID(cache_data_e cacheType);
uint16 Mid_ReadDataCacheID(cache_data_e cacheType);
uint16 Mid_WriteDataCache(uint8 *dataBuf, uint32 offsetAddr, uint16 length, cache_data_e cacheType);
uint16 Mid_ReadDataCache(uint8 *dataBuf, uint32 offsetAddr, uint16 length, cache_data_e cacheType);


void Mid_WdtFlashWrite(uint8_t* pdata, uint32_t addr, uint16_t length);

#endif		//OTAFUNC_H
