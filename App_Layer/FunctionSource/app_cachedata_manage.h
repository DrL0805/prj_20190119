#ifndef		APP_CACHEDATA_MANAGE_H
#define		APP_CACHEDATA_MANAGE_H


#define		APP_TIME_CACHE_VER		1
#define		APP_ALARM_CACHE_VER		1
#define		APP_SPORT_CACHE_VER		1
#define		APP_PACK_CACHE_VER		1
#define		APP_SYSTERM_CACHE_VER	1
#define		APP_SCENE_CACHE_VER		1

typedef enum 
{
    POWER_DOWN = 0x00,   //Òì³£µôµç£¬»òµçÁ¿ÓÃÍê¹Ø»ú
	POWER_OFF,           //°´¼ü¹Ø»ú£¬½øÈëstoreÄ£Ê½
	WDT_RESET,           //ÏµÍ³ÔËÐÐÒì³££¬wdt¸´Î»
	OTA_RESET,           //OTAÍê³É¸´Î»ÏµÍ³
}reset_type;


void App_CacheDataSaveType(cache_data_e cacheType);
void App_CacheDataRecoverType(cache_data_e cacheType);
void App_CacheDataSaveAll(void);
void App_CacheDataRecoverAll(void);


void App_WdtInit(void);
void App_WdtRestart(void);
void App_WdtDisable(void);
void App_ResetFlagClear(void);
reset_type App_ResetFlagCheck(void);
void App_WdtCacheRecover(void);
void App_WdtCacheSave(void);

//**********************************************************************
// º¯Êý¹¦ÄÜ:	Ð´reset flagµ½flash
// ÊäÈë²ÎÊý£º	flag: reset flag
// ·µ»Ø²ÎÊý£º	
//**********************************************************************
extern void App_WriteResetFlag(reset_type flag);

#endif		// APP_CACHE_DATA_MANAGE_H







