#include "platform_common.h"
/* FreeRTOS includes */
#include "rtos.h"
#include "mid_interface.h"
#include "app_variable.h"
#include "app_cachedata_manage.h"
#include "app_rtc.h"
#include "drv_extflash.h"
#include "sm_wdt.h"
#include "sm_sys.h"
#include "drv_movt.h"


// ***********************************************************************
//macro define
#define 	RESET_FLAG_ADDR 		0x10000

// ********************************function define***************************************

//**********************************************************************
// 函数功能:	保存指定类型的缓存数据
// 输入参数：	无
// 返回参数：	0x00:	操作成功
// 				0xff:	操作失败
//**********************************************************************
void App_CacheDataSaveType(cache_data_e cacheType)
{
	movt_task_msg_t	movtMsg;
	uint32	  u32Temp;
	uint8 	  u8Temp;
    rtc_time_s    timeTemp;
    uint8_t       databuf[64];
	alarm_clock_t alarmTemp;
	bodyInfo_s 	  bodyinfoTemp;

	uint32 		  offsetAdressTemp;


	//先擦除区域
	Mid_CacheDataClear(cacheType);

	switch(cacheType)
	{
		case CACHE_TIME_TYPE:
		offsetAdressTemp 	= 0;
#if(SUPPORT_MOVT == 1)
		//指针信息
		movtMsg.id	= MOVT_MSG_MC_READ_CUR;
		u32Temp		= MovtTask_EventSet(movtMsg);
		databuf[0]  = (uint8)(u32Temp >> 24);
		databuf[1]  = (uint8)(u32Temp >> 16);
		databuf[2]  = (uint8)(u32Temp >> 8);
		databuf[3]  = (uint8)(u32Temp);
		Mid_WriteDataCache(databuf, offsetAdressTemp, 4, CACHE_TIME_TYPE);
		offsetAdressTemp += 4;
		Mid_Movt_PolarRead(MOVT_M_CLOCK, &u8Temp);
		databuf[0]  = u8Temp;
		Mid_WriteDataCache(databuf, offsetAdressTemp, 4, CACHE_TIME_TYPE);
		offsetAdressTemp += 4;
#endif
		//rtc
		Mid_Rtc_TimeRead(&timeTemp);
		databuf[0] = timeTemp.day;
		databuf[1] = timeTemp.hour;
		databuf[2] = timeTemp.min;
		databuf[3] = timeTemp.month;
		databuf[4] = timeTemp.sec;
		databuf[5] = timeTemp.week;
		databuf[6] = timeTemp.year;
		databuf[7] = timeTemp.zone >> 8;
		databuf[8] = timeTemp.zone;
		Mid_WriteDataCache(databuf, offsetAdressTemp, 32, CACHE_TIME_TYPE);
		offsetAdressTemp += 32;
		//word time
		databuf[0] = WorldRtc.rtc.day;
		databuf[1] = WorldRtc.rtc.hour;
		databuf[2] = WorldRtc.rtc.min;
		databuf[3] = WorldRtc.rtc.month;
		databuf[4] = WorldRtc.rtc.sec;
		databuf[5] = WorldRtc.rtc.week;
		databuf[6] = WorldRtc.rtc.year;
		databuf[7] = WorldRtc.rtc.zone >> 8;
		databuf[8] = WorldRtc.rtc.zone;
		Mid_WriteDataCache(databuf, offsetAdressTemp, 32, CACHE_TIME_TYPE);
		offsetAdressTemp += 32;
		//city code
		databuf[0] = WorldRtc.cityCode >> 8;
		databuf[1] = WorldRtc.cityCode;
		Mid_WriteDataCache(databuf, offsetAdressTemp, 2, CACHE_TIME_TYPE);
		offsetAdressTemp += 2;
		//disture
		databuf[0] = appNotDisturdTimeInfo.StartHour;
		databuf[1] = appNotDisturdTimeInfo.StartMin;
		databuf[2] = appNotDisturdTimeInfo.StopHour;
		databuf[3] = appNotDisturdTimeInfo.StopMin;
		Mid_WriteDataCache(databuf, offsetAdressTemp, 4, CACHE_TIME_TYPE);
		offsetAdressTemp += 4;
		//long sit
		databuf[0] = appLongSitInfo.StartTimeHour;
		databuf[1] = appLongSitInfo.StartTimeMin;
		databuf[2] = appLongSitInfo.StopTimeHour;
		databuf[3] = appLongSitInfo.StopTimeMin;
		databuf[4] = appLongSitInfo.DisturdStartTimehour;
		databuf[5] = appLongSitInfo.DisturdStartTimeMin;
		databuf[6] = appLongSitInfo.DisturdStopTimehour;
		databuf[7] = appLongSitInfo.DisturdStopTimeMin;
		databuf[8] = appLongSitInfo.intv_mimute >> 8;
		databuf[9] = appLongSitInfo.intv_mimute;
		Mid_WriteDataCache(databuf, offsetAdressTemp, 10, CACHE_TIME_TYPE);
		offsetAdressTemp += 10;
		break;

		case CACHE_ALARM_TYPE:
		offsetAdressTemp 	= 0;
		//alarm time
		for(uint8_t i = 0;i < ALARM_CLOCK_GROUP;i++)
		{
			Mid_AlarmClock_Read(&alarmTemp,i);
			databuf[0] = alarmTemp.alarmswitch;
			databuf[1] = alarmTemp.delayswitch;
			databuf[2] = alarmTemp.hour;
			databuf[3] = alarmTemp.min;
			databuf[4] = alarmTemp.reptswitch;
			Mid_WriteDataCache(databuf, offsetAdressTemp, 16, CACHE_ALARM_TYPE);
			offsetAdressTemp += 16;
		}
		break;

		case CACHE_SPORT_TYPE:
		offsetAdressTemp 	= 0;
		Mid_SportScene_StepAimRead(&u32Temp);
		databuf[0]  = u32Temp >> 24;
		databuf[1]  = u32Temp >> 16;
		databuf[2]  = u32Temp >> 8;
		databuf[3]  = u32Temp;
		Mid_SportScene_StepRead(&u32Temp);
		databuf[4]  = u32Temp >> 24;
		databuf[5]  = u32Temp >> 16;
		databuf[6]  = u32Temp >> 8;
		databuf[7]  = u32Temp;
		Mid_SportScene_SportDuarationRead(&u32Temp);
		databuf[8]  = u32Temp >> 24;
		databuf[9]  = u32Temp >> 16;
		databuf[10]  = u32Temp >> 8;
		databuf[11]  = u32Temp;
		Mid_SportScene_BodyInfoRead(&bodyinfoTemp);
		databuf[12]  = bodyinfoTemp.bodyHeight;
		databuf[13]  = bodyinfoTemp.bodyWeight;
		databuf[14]  = bodyinfoTemp.sex;
		databuf[15]  = bodyinfoTemp.age;
		Mid_WriteDataCache(databuf, offsetAdressTemp, 16, CACHE_SPORT_TYPE);
		offsetAdressTemp += 16;
		break;

		case CACHE_PACK_TYPE:
		//暂不处理　
		break;

		case CACHE_SYSTERM_TYPE:
		offsetAdressTemp 	= 0;
		databuf[0]  = systermConfig.systermLanguge;
		databuf[1]  = systermConfig.systermTimeType;

		databuf[2]  = systermConfig.stepCountSwitch;
		databuf[3]  = systermConfig.heartrateSwitch;
		databuf[4]  = systermConfig.weatherSwitch;
		databuf[5]  = systermConfig.stepCompleteRemindSwitch;
		databuf[6]  = systermConfig.bleDiscRemindSwitch;
		databuf[7]  = systermConfig.longSitRemindSwitch;
		databuf[8]  = systermConfig.notDisturbSwitch;
		databuf[9]  = systermConfig.gestureSwitch;

		databuf[10]  = systermConfig.appRemindSwitch >> 24;
		databuf[11]  = systermConfig.appRemindSwitch >> 16;
		databuf[12]  = systermConfig.appRemindSwitch >> 8;
		databuf[13]  = systermConfig.appRemindSwitch;

		databuf[14]  = systermConfig.appDetailRemindSwitch >> 24;
		databuf[15]  = systermConfig.appDetailRemindSwitch >> 16;
		databuf[16]  = systermConfig.appDetailRemindSwitch >> 8;
		databuf[17]  = systermConfig.appDetailRemindSwitch;

		Mid_WriteDataCache(databuf, offsetAdressTemp, 18, CACHE_SYSTERM_TYPE);
		offsetAdressTemp += 18;
		break;

		case CACHE_SCENE_TYPE:
		//暂不处理
		break;

		default:
		return;
	}
	//存储标识
	Mid_WriteDataCacheID(cacheType);
}


//**********************************************************************
// 函数功能:	恢复指定类型的缓存数据
// 输入参数：	无
// 返回参数：	0x00:	操作成功
// 				0xff:	操作失败
//**********************************************************************
void App_CacheDataRecoverType(cache_data_e cacheType)
{
	uint16  result;
	movt_task_msg_t	movtMsg;
	uint32	  u32Temp;
    rtc_time_s    timeTemp;
    uint8_t       databuf[64];
	alarm_clock_t alarmTemp;
	bodyInfo_s 	  bodyinfoTemp;
	uint32 		  offsetAdressTemp;


	result = Mid_ReadDataCacheID(cacheType);

	//信息无用，不作恢复处理　
	if (result)
	{
		return;
	}

	switch(cacheType)
	{
		case CACHE_TIME_TYPE:
		offsetAdressTemp 	= 8;
		Mid_ReadDataCache(databuf, offsetAdressTemp, 32, CACHE_TIME_TYPE);
		offsetAdressTemp += 32;
		//rtc
		timeTemp.day  	= databuf[0];
		timeTemp.hour 	= databuf[1];
		timeTemp.min  	= databuf[2];
		timeTemp.month  = databuf[3];
		timeTemp.sec  	= databuf[4];
		timeTemp.week 	= databuf[5];
		timeTemp.year 	= databuf[6];
		timeTemp.zone 	= (uint16)databuf[7] << 8 | databuf[8];
		Mid_Rtc_TimeWrite(&timeTemp);
		
		//word time
		Mid_ReadDataCache(databuf, offsetAdressTemp, 32, CACHE_TIME_TYPE);
		offsetAdressTemp += 32;
		WorldRtc.rtc.day 	= databuf[0];
		WorldRtc.rtc.hour 	= databuf[1];
		WorldRtc.rtc.min 	= databuf[2];
		WorldRtc.rtc.month 	= databuf[3];
		WorldRtc.rtc.sec 	= databuf[4];
		WorldRtc.rtc.week 	= databuf[5];
		WorldRtc.rtc.year 	= databuf[6];
		WorldRtc.rtc.zone 	= (uint16)databuf[7] << 8 | databuf[8];

		//city code
		Mid_ReadDataCache(databuf, offsetAdressTemp, 2, CACHE_TIME_TYPE);
		offsetAdressTemp += 2;
		WorldRtc.cityCode   = (uint16)databuf[0] << 8 | databuf[1];
		
		//disture
		Mid_ReadDataCache(databuf, offsetAdressTemp, 4, CACHE_TIME_TYPE);
		offsetAdressTemp += 4;
		appNotDisturdTimeInfo.StartHour= databuf[0];
		appNotDisturdTimeInfo.StartMin = databuf[1];
		appNotDisturdTimeInfo.StopHour = databuf[2];
		appNotDisturdTimeInfo.StopMin  = databuf[3];
		
		//ling sit
		Mid_ReadDataCache(databuf, offsetAdressTemp, 10, CACHE_TIME_TYPE);
		offsetAdressTemp += 10;
		appLongSitInfo.StartTimeHour 	= databuf[0];
		appLongSitInfo.StartTimeMin   	= databuf[1];
		appLongSitInfo.StopTimeHour 	= databuf[2];
		appLongSitInfo.StopTimeMin 		= databuf[3];
		appLongSitInfo.DisturdStartTimehour = databuf[4];
		appLongSitInfo.DisturdStartTimeMin 	= databuf[5];
		appLongSitInfo.DisturdStopTimehour 	= databuf[6];
		appLongSitInfo.DisturdStopTimeMin 	= databuf[7];
		appLongSitInfo.intv_mimute 			= (uint16)databuf[8] << 8 | databuf[9];

#if(SUPPORT_MOVT == 1)
		//指针信息(放在最后，以便追针)
		offsetAdressTemp = 0;
		Mid_ReadDataCache(databuf, offsetAdressTemp, 4, CACHE_TIME_TYPE);
		offsetAdressTemp += 4;
		Mid_Movt_PolarSet(MOVT_M_CLOCK, databuf[0]);

		Mid_ReadDataCache(databuf, offsetAdressTemp, 4, CACHE_TIME_TYPE);
		offsetAdressTemp += 4;

		u32Temp 	= (uint32)databuf[0] << 24 | (uint32)databuf[1] << 16 |
					  (uint32)databuf[2] << 8  | (uint32)databuf[3];
		movtMsg.id	= MOVT_MSG_MC_SET_CUR_AIM;
		movtMsg.cur	= u32Temp;
		movtMsg.aim = App_Rtc_ExchangeTimeforCount(timeTemp.hour, timeTemp.min, timeTemp.sec)/10;
		u32Temp		= MovtTask_EventSet(movtMsg);
#endif
		break;

		case CACHE_ALARM_TYPE:
		offsetAdressTemp 	= 0;
		//alarm time
		for(uint8_t i = 0;i < ALARM_CLOCK_GROUP;i++)
		{
			Mid_ReadDataCache(databuf, offsetAdressTemp, 16, CACHE_ALARM_TYPE);
			offsetAdressTemp += 16;
			alarmTemp.alarmswitch 	= databuf[0];
			alarmTemp.delayswitch 	= databuf[1];
			alarmTemp.hour 			= databuf[2];
			alarmTemp.min 			= databuf[3];
			alarmTemp.reptswitch 	= databuf[4];
			Mid_AlarmClock_Write(&alarmTemp,i);
		}
		break;

		case CACHE_SPORT_TYPE:
		offsetAdressTemp 	= 0;
		Mid_ReadDataCache(databuf, offsetAdressTemp, 16, CACHE_SPORT_TYPE);
		offsetAdressTemp += 16;

		bodyinfoTemp.bodyHeight = databuf[12];
		bodyinfoTemp.bodyWeight = databuf[13];
		bodyinfoTemp.sex 		= databuf[14];
		bodyinfoTemp.age 		= databuf[15];
		Mid_SportScene_BodyInfoSet(&bodyinfoTemp);

		u32Temp = (uint32)databuf[8] << 24 | (uint32)databuf[9] << 16 |
				  (uint32)databuf[10] << 8 | (uint32)databuf[11] << 0; 
		Mid_SportScene_SportDuarationWrite(&u32Temp);

		u32Temp = (uint32)databuf[4] << 24 | (uint32)databuf[5] << 16 |
				  (uint32)databuf[6] << 8 | (uint32)databuf[7] << 0; 
		Mid_SportScene_StepWrite(&u32Temp);

		u32Temp = (uint32)databuf[0] << 24 | (uint32)databuf[1] << 16 |
				  (uint32)databuf[2] << 8 | (uint32)databuf[3] << 0; 
		Mid_SportScene_StepAimSet(u32Temp);
		break;

		case CACHE_PACK_TYPE:
		//暂不处理　
		break;

		case CACHE_SYSTERM_TYPE:
		offsetAdressTemp 	= 0;
		Mid_ReadDataCache(databuf, offsetAdressTemp, 18, CACHE_SYSTERM_TYPE);
		offsetAdressTemp += 18;

		systermConfig.systermLanguge 			= databuf[0];
		systermConfig.systermTimeType 			= databuf[1];
		systermConfig.stepCountSwitch 			= databuf[2];
		systermConfig.heartrateSwitch 			= databuf[3];
		systermConfig.weatherSwitch 			= databuf[4];
		systermConfig.stepCompleteRemindSwitch 	= databuf[5];
		systermConfig.bleDiscRemindSwitch 		= databuf[6];
		systermConfig.longSitRemindSwitch 		= databuf[7];
		systermConfig.notDisturbSwitch 			= databuf[8];
		systermConfig.gestureSwitch 			= databuf[9];

		systermConfig.appRemindSwitch = (uint32)databuf[10] << 24 | (uint32)databuf[11] << 16 |
										(uint32)databuf[12] << 8 | (uint32)databuf[13] << 0;

		systermConfig.appDetailRemindSwitch = (uint32)databuf[14] << 24 | (uint32)databuf[15] << 16 |
										(uint32)databuf[16] << 8 | (uint32)databuf[17] << 0;		
		break;

		case CACHE_SCENE_TYPE:
		//暂不处理
		break;

		default:
		return;
	}
}


//**********************************************************************
// 函数功能:	保存所有的缓存数据
// 输入参数：	无
// 返回参数：	0x00:	操作成功
// 				0xff:	操作失败
//**********************************************************************
void App_CacheDataSaveAll(void)
{
	App_CacheDataSaveType(CACHE_TIME_TYPE);
	App_CacheDataSaveType(CACHE_ALARM_TYPE);
	App_CacheDataSaveType(CACHE_SPORT_TYPE);
	App_CacheDataSaveType(CACHE_PACK_TYPE);
	App_CacheDataSaveType(CACHE_SYSTERM_TYPE);
	App_CacheDataSaveType(CACHE_SCENE_TYPE);
}

//**********************************************************************
// 函数功能:	恢复所有的缓存数据
// 输入参数：	无
// 返回参数：	0x00:	操作成功
// 				0xff:	操作失败
//**********************************************************************
void App_CacheDataRecoverAll(void)
{
	App_CacheDataRecoverType(CACHE_TIME_TYPE);
	App_CacheDataRecoverType(CACHE_ALARM_TYPE);
	App_CacheDataRecoverType(CACHE_SPORT_TYPE);
	App_CacheDataRecoverType(CACHE_PACK_TYPE);
	App_CacheDataRecoverType(CACHE_SYSTERM_TYPE);
	App_CacheDataRecoverType(CACHE_SCENE_TYPE);
}


//**********************************看门狗监测系统异常处理************************************//
#define 	RESET_FLAG_ADDR 			0x10000
#define 	RESET_DATA_BASE_ADDR 		0x10010
#define 	WDT_RESET_TIME 		255
#define 	WDT_INTTERUPT_TIME 	127

static const uint8_t WDT_FLAG[16] = 
{
	"WDT_Reset"
};

static const uint8_t POWER_FLAG[16] = 
{
	"POWER_OFF"
};

static const uint8_t OTA_FLAG[16] = 
{
	"Updata"
};

//**********************************************************************
// 函数功能：将数据写入外部flash中，不经过系统调用，写的地址段不能跨越256字节的倍数地址
// 输入：	pdata:	数据地址指针
// 			addr:	外部flash开始地址
// 			length:	写入长度
// 输出：	无
//**********************************************************************
void App_WdtFlashWrite(uint8_t* pdata, uint32_t addr, uint16_t length)
{
	uint8_t	k = 100;

    //step 1: open and wakeup extflash
    Drv_Extflash_Open();
	Drv_Extflash_SendCmd(FLASH_WAKEUP_CMD);
	while(k--)
	{
		if(Drv_Extflash_CheckState() == Ret_OK)
			break;
		SMDrv_SYS_DelayMs(1);
	}

    //step 3: write data to flash
	Drv_Extflash_Write(pdata, addr, length);
    //step 4: sleep and close extflash
	Drv_Extflash_SendCmd(FLASH_SLEEP_CMD);
    Drv_Extflash_Close();
}

//**********************************************************************
// 函数功能：将数据从外部flash中读出，不经过系统调用，
// 输入：	pdata:	数据地址指针
// 			addr:	外部flash开始地址
// 			length:	写入长度
// 输出：	无
//**********************************************************************
void App_WdtFlashRead(uint8_t* pdata, uint32_t addr, uint16_t length)
{
	flash_task_msg_t		flashMsg;

	flashMsg.id 									= EXTFLASH_ID;
	flashMsg.flash.extflashEvent.para.startAddr 	= addr;
	flashMsg.flash.extflashEvent.para.dataAddr 		= pdata;
	flashMsg.flash.extflashEvent.para.length 		= length;
	flashMsg.flash.extflashEvent.para.endAddr 		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
	flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_READ;
	flashMsg.flash.extflashEvent.para.result 		= 0;
	flashMsg.flash.extflashEvent.para.result		= FlashTask_EventSet(&flashMsg);

}

//**********************************************************************
// 函数功能：缓存数据写入
// 输入：	　无
// 输出：	 无
//**********************************************************************
void App_WdtCacheSave(void)
{
	movt_task_msg_t	movtMsg;
	uint32	  u32Temp;
	uint8 	  u8Temp;
    rtc_time_s    timeTemp;
    uint8_t       databuf[64];
	alarm_clock_t alarmTemp;
	bodyInfo_s 	  bodyinfoTemp;

	uint32 		  offsetAdressTemp;


	offsetAdressTemp 	= RESET_DATA_BASE_ADDR;
#if(SUPPORT_MOVT == 1)
	//指针信息
	movtMsg.id	= MOVT_MSG_MC_READ_CUR;
	u32Temp		= MovtTask_EventSet(movtMsg);
	databuf[0]  = (uint8)(u32Temp >> 24);
	databuf[1]  = (uint8)(u32Temp >> 16);
	databuf[2]  = (uint8)(u32Temp >> 8);
	databuf[3]  = (uint8)(u32Temp);
	Mid_Movt_PolarRead(MOVT_M_CLOCK, &u8Temp);
	databuf[4]  = u8Temp;
	App_WdtFlashWrite(databuf, offsetAdressTemp, 5);
	offsetAdressTemp += 5;
#endif
	//rtc
	Mid_Rtc_TimeRead(&timeTemp);
	databuf[0] = timeTemp.day;
	databuf[1] = timeTemp.hour;
	databuf[2] = timeTemp.min;
	databuf[3] = timeTemp.month;
	databuf[4] = timeTemp.sec;
	databuf[5] = timeTemp.week;
	databuf[6] = timeTemp.year;
	databuf[7] = timeTemp.zone >> 8;
	databuf[8] = timeTemp.zone;
	App_WdtFlashWrite(databuf, offsetAdressTemp, 9);
	offsetAdressTemp += 9;

	//word time
	databuf[0] = WorldRtc.rtc.day;
	databuf[1] = WorldRtc.rtc.hour;
	databuf[2] = WorldRtc.rtc.min;
	databuf[3] = WorldRtc.rtc.month;
	databuf[4] = WorldRtc.rtc.sec;
	databuf[5] = WorldRtc.rtc.week;
	databuf[6] = WorldRtc.rtc.year;
	databuf[7] = WorldRtc.rtc.zone >> 8;
	databuf[8] = WorldRtc.rtc.zone;
	//city code
	databuf[9] = WorldRtc.cityCode >> 8;
	databuf[10] = WorldRtc.cityCode;

	App_WdtFlashWrite(databuf, offsetAdressTemp, 11);
	offsetAdressTemp += 11;

	//disture
	databuf[0] = appNotDisturdTimeInfo.StartHour;
	databuf[1] = appNotDisturdTimeInfo.StartMin;
	databuf[2] = appNotDisturdTimeInfo.StopHour;
	databuf[3] = appNotDisturdTimeInfo.StopMin;
	App_WdtFlashWrite(databuf, offsetAdressTemp, 4);
	offsetAdressTemp += 4;

	//long sit
	databuf[0] = appLongSitInfo.StartTimeHour;
	databuf[1] = appLongSitInfo.StartTimeMin;
	databuf[2] = appLongSitInfo.StopTimeHour;
	databuf[3] = appLongSitInfo.StopTimeMin;
	databuf[4] = appLongSitInfo.DisturdStartTimehour;
	databuf[5] = appLongSitInfo.DisturdStartTimeMin;
	databuf[6] = appLongSitInfo.DisturdStopTimehour;
	databuf[7] = appLongSitInfo.DisturdStopTimeMin;
	databuf[8] = appLongSitInfo.intv_mimute >> 8;
	databuf[9] = appLongSitInfo.intv_mimute;
	App_WdtFlashWrite(databuf, offsetAdressTemp, 10);
	offsetAdressTemp += 10;

	//alarm time
	for(uint8_t i = 0;i < ALARM_CLOCK_GROUP;i++)
	{
		Mid_AlarmClock_Read(&alarmTemp,i);
		databuf[0] = alarmTemp.alarmswitch;
		databuf[1] = alarmTemp.delayswitch;
		databuf[2] = alarmTemp.hour;
		databuf[3] = alarmTemp.min;
		databuf[4] = alarmTemp.reptswitch;
		App_WdtFlashWrite(databuf, offsetAdressTemp, 5);
		offsetAdressTemp += 5;
	}

	//sport
	Mid_SportScene_StepAimRead(&u32Temp);
	databuf[0]  = u32Temp >> 24;
	databuf[1]  = u32Temp >> 16;
	databuf[2]  = u32Temp >> 8;
	databuf[3]  = u32Temp;
	Mid_SportScene_StepRead(&u32Temp);
	databuf[4]  = u32Temp >> 24;
	databuf[5]  = u32Temp >> 16;
	databuf[6]  = u32Temp >> 8;
	databuf[7]  = u32Temp;
	Mid_SportScene_SportDuarationRead(&u32Temp);
	databuf[8]  = u32Temp >> 24;
	databuf[9]  = u32Temp >> 16;
	databuf[10]  = u32Temp >> 8;
	databuf[11]  = u32Temp;
	Mid_SportScene_BodyInfoRead(&bodyinfoTemp);
	databuf[12]  = bodyinfoTemp.bodyHeight;
	databuf[13]  = bodyinfoTemp.bodyWeight;
	databuf[14]  = bodyinfoTemp.sex;
	databuf[15]  = bodyinfoTemp.age;
	App_WdtFlashWrite(databuf, offsetAdressTemp, 16);
	offsetAdressTemp += 16;

	//syytern config
	databuf[0]  = systermConfig.systermLanguge;
	databuf[1]  = systermConfig.systermTimeType;

	databuf[2]  = systermConfig.stepCountSwitch;
	databuf[3]  = systermConfig.heartrateSwitch;
	databuf[4]  = systermConfig.weatherSwitch;
	databuf[5]  = systermConfig.stepCompleteRemindSwitch;
	databuf[6]  = systermConfig.bleDiscRemindSwitch;
	databuf[7]  = systermConfig.longSitRemindSwitch;
	databuf[8]  = systermConfig.notDisturbSwitch;
	databuf[9]  = systermConfig.gestureSwitch;

	databuf[10]  = systermConfig.appRemindSwitch >> 24;
	databuf[11]  = systermConfig.appRemindSwitch >> 16;
	databuf[12]  = systermConfig.appRemindSwitch >> 8;
	databuf[13]  = systermConfig.appRemindSwitch;

	databuf[14]  = systermConfig.appDetailRemindSwitch >> 24;
	databuf[15]  = systermConfig.appDetailRemindSwitch >> 16;
	databuf[16]  = systermConfig.appDetailRemindSwitch >> 8;
	databuf[17]  = systermConfig.appDetailRemindSwitch;

	App_WdtFlashWrite(databuf, offsetAdressTemp, 18);
}

//**********************************************************************
// 函数功能：缓存数据恢复
// 输入：	　无
// 输出：	 无
//**********************************************************************
void App_WdtCacheRecover(void)
{
	movt_task_msg_t	movtMsg;
	uint32	  u32Temp;
    rtc_time_s    timeTemp;
    uint8_t       databuf[64];
	alarm_clock_t alarmTemp;
	bodyInfo_s 	  bodyinfoTemp;
	uint32 		  offsetAdressTemp;

	offsetAdressTemp 	= RESET_DATA_BASE_ADDR;

	offsetAdressTemp += 5;
	App_WdtFlashRead(databuf, offsetAdressTemp, 9);
	offsetAdressTemp += 9;

	//rtc
	timeTemp.day  	= databuf[0];
	timeTemp.hour 	= databuf[1];
	timeTemp.min  	= databuf[2];
	timeTemp.month  = databuf[3];
	timeTemp.sec  	= databuf[4];
	timeTemp.week 	= databuf[5];
	timeTemp.year 	= databuf[6];
	timeTemp.zone 	= (uint16)databuf[7] << 8 | databuf[8];
	Mid_Rtc_TimeWrite(&timeTemp);
	
	//word time
	App_WdtFlashRead(databuf, offsetAdressTemp, 11);
	offsetAdressTemp += 11;
	WorldRtc.rtc.day 	= databuf[0];
	WorldRtc.rtc.hour 	= databuf[1];
	WorldRtc.rtc.min 	= databuf[2];
	WorldRtc.rtc.month 	= databuf[3];
	WorldRtc.rtc.sec 	= databuf[4];
	WorldRtc.rtc.week 	= databuf[5];
	WorldRtc.rtc.year 	= databuf[6];
	WorldRtc.rtc.zone 	= (uint16)databuf[7] << 8 | databuf[8];
	//city code
	WorldRtc.cityCode   = (uint16)databuf[9] << 8 | databuf[10];
	
	//disture
	App_WdtFlashRead(databuf, offsetAdressTemp, 4);
	offsetAdressTemp += 4;
	appNotDisturdTimeInfo.StartHour= databuf[0];
	appNotDisturdTimeInfo.StartMin = databuf[1];
	appNotDisturdTimeInfo.StopHour = databuf[2];
	appNotDisturdTimeInfo.StopMin  = databuf[3];
	
	//ling sit
	App_WdtFlashRead(databuf, offsetAdressTemp, 10);
	offsetAdressTemp += 10;
	appLongSitInfo.StartTimeHour 	= databuf[0];
	appLongSitInfo.StopTimeMin   	= databuf[1];
	appLongSitInfo.StopTimeHour 	= databuf[2];
	appLongSitInfo.StopTimeMin 		= databuf[3];
	appLongSitInfo.DisturdStartTimehour = databuf[4];
	appLongSitInfo.DisturdStartTimeMin 	= databuf[5];
	appLongSitInfo.DisturdStopTimehour 	= databuf[6];
	appLongSitInfo.DisturdStopTimeMin 	= databuf[7];
	appLongSitInfo.intv_mimute 			= (uint16)databuf[8] << 8 | databuf[9];

#if(SUPPORT_MOVT == 1)
	//指针信息(放在最后，以便追针)
	offsetAdressTemp 	= RESET_DATA_BASE_ADDR;
	App_WdtFlashRead(databuf, offsetAdressTemp, 5);
	Mid_Movt_PolarSet(MOVT_M_CLOCK, databuf[0]);

	u32Temp 	= (uint32)databuf[1] << 24 | (uint32)databuf[2] << 16 |
				  (uint32)databuf[3] << 8  | (uint32)databuf[4];
	movtMsg.id	= MOVT_MSG_MC_SET_CUR_AIM;
	movtMsg.cur	= u32Temp;
	movtMsg.aim = App_Rtc_ExchangeTimeforCount(timeTemp.hour, timeTemp.min, timeTemp.sec)/10;
	u32Temp		= MovtTask_EventSet(movtMsg);
#endif

	offsetAdressTemp 	= RESET_DATA_BASE_ADDR + 39;
	//alarm time
	for(uint8_t i = 0;i < ALARM_CLOCK_GROUP;i++)
	{
		App_WdtFlashRead(databuf, offsetAdressTemp, 5);
		offsetAdressTemp += 5;
		alarmTemp.alarmswitch 	= databuf[0];
		alarmTemp.delayswitch 	= databuf[1];
		alarmTemp.hour 			= databuf[2];
		alarmTemp.min 			= databuf[3];
		alarmTemp.reptswitch 	= databuf[4];
		Mid_AlarmClock_Write(&alarmTemp,i);
	}

	App_WdtFlashRead(databuf, offsetAdressTemp, 16);
	offsetAdressTemp += 16;

	bodyinfoTemp.bodyHeight = databuf[12];
	bodyinfoTemp.bodyWeight = databuf[13];
	bodyinfoTemp.sex 		= databuf[14];
	bodyinfoTemp.age 		= databuf[15];
	Mid_SportScene_BodyInfoSet(&bodyinfoTemp);

	u32Temp = (uint32)databuf[8] << 24 | (uint32)databuf[9] << 16 |
			  (uint32)databuf[10] << 8 | (uint32)databuf[11] << 0; 
	Mid_SportScene_SportDuarationWrite(&u32Temp);

	u32Temp = (uint32)databuf[4] << 24 | (uint32)databuf[5] << 16 |
			  (uint32)databuf[6] << 8 | (uint32)databuf[7] << 0; 
	Mid_SportScene_StepWrite(&u32Temp);

	u32Temp = (uint32)databuf[0] << 24 | (uint32)databuf[1] << 16 |
			  (uint32)databuf[2] << 8 | (uint32)databuf[3] << 0; 
	Mid_SportScene_StepAimSet(u32Temp);


	App_WdtFlashRead(databuf, offsetAdressTemp, 18);
	offsetAdressTemp += 18;
	systermConfig.systermLanguge 			= databuf[0];
	systermConfig.systermTimeType 			= databuf[1];
	systermConfig.stepCountSwitch 			= databuf[2];
	systermConfig.heartrateSwitch 			= databuf[3];
	systermConfig.weatherSwitch 			= databuf[4];
	systermConfig.stepCompleteRemindSwitch 	= databuf[5];
	systermConfig.bleDiscRemindSwitch 		= databuf[6];
	systermConfig.longSitRemindSwitch 		= databuf[7];
	systermConfig.notDisturbSwitch 			= databuf[8];
	systermConfig.gestureSwitch 			= databuf[9];
	systermConfig.appRemindSwitch = (uint32)databuf[10] << 24 | (uint32)databuf[11] << 16 |
									(uint32)databuf[12] << 8 | (uint32)databuf[13] << 0;

	systermConfig.appDetailRemindSwitch = (uint32)databuf[14] << 24 | (uint32)databuf[15] << 16 |
										(uint32)databuf[16] << 8 | (uint32)databuf[17] << 0;
}

//**********************************************************************
// 函数功能:	看门狗复位中断处理
// 输入参数：	无
// 返回参数：	0x00:	操作成功
// 				0xff:	操作失败
//**********************************************************************
void App_WdtIsrCb(void)
{
	//step 2: save system,setting info to flash
    App_WdtCacheSave();
    //step 1: 写标识,write wdt flag
    App_WdtFlashWrite((uint8_t *)WDT_FLAG, RESET_FLAG_ADDR, 16);

}

//**********************************************************************
// 函数功能:	看门狗初始化
// 输入参数：	无
// 返回参数：	0x00:	操作成功
// 				0xff:	操作失败
//**********************************************************************
void App_WdtInit(void)
{
	SMDrv_WDT_Open(WDT_RESET_TIME,WDT_INTTERUPT_TIME,App_WdtIsrCb);
}

//**********************************************************************
// 函数功能:	看门狗清零重启
// 输入参数：	无
// 返回参数：	0x00:	操作成功
// 				0xff:	操作失败
//**********************************************************************
void App_WdtRestart(void)
{
	SMDrv_WDT_Open(WDT_RESET_TIME,WDT_INTTERUPT_TIME,App_WdtIsrCb);
	SMDrv_WDT_ReStart();
}

//**********************************************************************
// 函数功能:	看门狗关闭
// 输入参数：	无
// 返回参数：	0x00:	操作成功
// 				0xff:	操作失败
//**********************************************************************
void App_WdtDisable(void)
{
	SMDrv_WDT_Close();
}

//**********************************************************************
// 函数功能:	写reset flag到flash
// 输入参数：	flag: reset flag
// 返回参数：	
//**********************************************************************
void App_WriteResetFlag(reset_type flag)
{	
	//写标识
	if(flag == OTA_RESET)
    {
        App_WdtFlashWrite((uint8_t *)OTA_FLAG, RESET_FLAG_ADDR, 16);
    }
    else if(flag == WDT_RESET)
    {
        App_WdtFlashWrite((uint8_t *)WDT_FLAG, RESET_FLAG_ADDR, 16);
    }
    else if(flag == POWER_OFF)
    {
        App_WdtFlashWrite((uint8_t *)POWER_FLAG, RESET_FLAG_ADDR, 16);
    }
    else
        ; //掉电 
}

//**********************************************************************
// 函数功能：复位信息区域擦除
// 输入：	无
// 输出：	无
//**********************************************************************
void App_ResetFlagClear(void)
{
	flash_task_msg_t		flashMsg;

	flashMsg.id 	= EXTFLASH_ID;
	flashMsg.flash.extflashEvent.para.startAddr 	= RESET_FLAG_ADDR;
	flashMsg.flash.extflashEvent.para.dataAddr 		= (uint8*)(NULL);
	flashMsg.flash.extflashEvent.para.length 		= 4096;		
	flashMsg.flash.extflashEvent.para.endAddr 		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
	flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_4K_ERASE;	
	flashMsg.flash.extflashEvent.para.result		= FlashTask_EventSet(&flashMsg);
}

//**********************************************************************
// 函数功能：复位信息检索
// 输入：	无
// 输出：	无
//**********************************************************************
reset_type App_ResetFlagCheck(void)
{
    uint8_t	tempData[16], i;

    //step 1: read reset flag
    App_WdtFlashRead(tempData, RESET_FLAG_ADDR, 16);
#if 0
    for(i = 0; i < 9; i++)
    {
        SEGGER_RTT_printf(0,"0x%x,",tempData[i]);
    }
    SEGGER_RTT_printf(0,"\n");
#endif
    
    //step 2: check Power off reset
    for(i = 0; i < 9; i++)
    {
        if(tempData[i] != POWER_FLAG[i])
            break;
    }
    if(i >= 9)   //关机复位
    {
        //SEGGER_RTT_printf(0,"power off reset\n");
        return POWER_OFF;
    }

    //step 3: check WDT reset
    for(i = 0; i < 9; i++)
    {
        if(tempData[i] != WDT_FLAG[i])
            break;
    }
    if(i >= 9)   //看门狗复位
    {
        //SEGGER_RTT_printf(0,"WDT reset\n");
        return WDT_RESET;
    }

    //step 4: check OTA rest
    for(i = 0; i < 6; i++)
    {
        if(tempData[i] != OTA_FLAG[i])
            break;
    }
    if(i >= 6)   //OTA复位
    {
        //SEGGER_RTT_printf(0,"OTA reset\n");
        return OTA_RESET;
    }

    //掉电复位
    return POWER_DOWN;
}

