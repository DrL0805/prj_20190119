#include "platform_common.h"
/* FreeRTOS includes */
#include "rtos.h"
#include "mid_common.h"
#include "mid_interface.h"
#include "app_variable.h"
#include "app_protocal.h"
#include "app_sleepdata_manage.h"
#include "app_win_common.h"
#include "app_win_process.h"




/*************** func declaration ********************************/
#define START_TIME_HOUR      			18
#define STOP_TIME_HOUR 					12
#define	SleepDataReUploadInternal 		20//50ms


/*************** macro define ********************************/
#define  SLEEP_RECORD_MAX 		10//最多可记录的睡眠条数



/************** variable define *****************************/	

// 睡眠历史条数返回
static const protocal_msg_t	PROT_SLEEP_TOTAL_RET =
{
	0x23, 0x01, 0x04, 0x13, 0x00, 0x08, 0x14, 0x03,0x00,
};

// 睡眠总体记录返回
static const protocal_msg_t	PROT_SLEEP_CATALOG_RET =
{
	0x23, 0x01, 0x0D, 0x13, 0x00, 0x08, 0x14, 0x04,
};

// 睡眠醒来记录返回
static const protocal_msg_t	PROT_SLEEP_SUB_DATA_UPLOAD =
{
	0x23, 0x01, 0x09, 0x13, 0x80, 0x80, 0x14, 0x04,
};



static sleep_ui_info sleepRecode;			//保存最近一次的睡眠记录
static sleep_data sleepRecodeCache[SLEEP_RECORD_MAX];
static uint8 sleepRecodeCacheCnt;

static uint8 wakeupCntTemp;
static uint8 curWakeupCntUpload;
static uint8 curCatalogNumUpload;
static uint32 indexUtcTemp;
static uint32 datauploadTotalLen;

static uint8 appSaveInitFlag;
static TimerHandle_t App_SleepData_DataReUploadTimer		= NULL;				// 更新定时器定时器

/************** function define *****************************/
//**********************************************************************
// 函数功能:	开启延时上传定时器
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
void App_SleepData_DataReUploadTimerStart(void)
{
	if (xTimerIsTimerActive(App_SleepData_DataReUploadTimer) == pdFALSE)
    {
		xTimerReset(App_SleepData_DataReUploadTimer, 1);	
		xTimerStart(App_SleepData_DataReUploadTimer, 1);
	}
}

 //**********************************************************************
// 函数功能:	关闭延时上传定时器
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
void App_SleepData_DataReUploadTimerStop(void)
{
	if (xTimerIsTimerActive(App_SleepData_DataReUploadTimer) != pdFALSE)
    {
		xTimerStop(App_SleepData_DataReUploadTimer, 1);
	}
}

//**********************************************************************
// 函数功能：   睡眠模块初始化
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
void App_SleepData_SaveInit(void (*TimerCb)(TimerHandle_t xTimer))
{
	uint8 	i;

	sleepRecode.StartHour 	= 0;
	sleepRecode.StartMin 	= 0;
	sleepRecode.StopHour 	= 0;
	sleepRecode.StopMin 	= 0;
	sleepRecode.DurationM 	= 0;
	sleepRecode.Quality 	= 0;

	sleepRecodeCacheCnt 	= 0;

	for (i = 0; i < SLEEP_RECORD_MAX; i++)
	{
		sleepRecodeCache[i].StartUTC = 0;
		sleepRecodeCache[i].StopUTC = 0;
		sleepRecodeCache[i].DurationM = 0;
		sleepRecodeCache[i].Quality = 0;

	}

	//睡眠场景初始化
	Mid_SleepScene_Init();
	//存储区域分配
	Mid_SleepData_Init();
	Mid_SleepData_ClassifyDataInit(SLEEP_DATA_CLASSIFY, 4*SLEEP_DM_APP_SECTOR_LENGTH, 3 * SLEEP_DM_APP_SECTOR_LENGTH);
	
	if (appSaveInitFlag == 0)
	{
		App_SleepData_DataReUploadTimer = xTimerCreate("sleepreupload", (APP_1SEC_TICK / SleepDataReUploadInternal), pdTRUE, 0, TimerCb);
		appSaveInitFlag 			= 1;
	}
}

//**********************************************************************
// 函数功能：   睡眠信息处理(每小时处理1次)
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
void App_SleepData_Save(void)
{
	uint8 		startIndex,stopIndex;
	uint8 		startIndexCnt,stopIndexCnt;
	uint8 		sleepValidNum;
	uint8 		validCnt;
	rtc_time_s  timeTemp;
	sleep_data sleepRecodeTemp[SLEEP_RECORD_MAX];
	sleep_scene_catalogInfo_s dataInfo;
	uint8 		dataBuf[10];
	uint16 		sleepDuarationTemp;	
	uint8 		sleepNumCnt;
	uint8 		roundCnt;


	startIndex 		= 0;
	stopIndex  		= 0;
	startIndexCnt 	= 0;
	stopIndexCnt 	= 0;
	sleepDuarationTemp = 0;
	sleepNumCnt 	= 0;
	roundCnt 		= 0;
	
	Mid_SleepScene_GetRecord(sleepRecodeTemp, &sleepValidNum);

	if (sleepValidNum > 0)	//有睡眠新记录
	{
		for (validCnt = 0; validCnt < sleepValidNum; validCnt ++)
		{
			sleepRecodeCache[sleepRecodeCacheCnt] = sleepRecodeTemp[validCnt];
			sleepRecodeCacheCnt ++;
			if (sleepRecodeCacheCnt >= SLEEP_RECORD_MAX) //越界暂先不处理　
			{
				break;
			}
		}
	}
	
	//对缓存数据进行存储
	while (sleepRecodeCacheCnt > 0)	//有睡眠记录缓存
	{
		//检查完整睡眠的开始时间
		for (validCnt = 0; validCnt < sleepRecodeCacheCnt; validCnt ++)
		{
			UtcTransformTime(sleepRecodeCache[validCnt].StartUTC, &timeTemp);

			//18:00~ 次日12:00
			if (timeTemp.hour >= START_TIME_HOUR || timeTemp.hour <= STOP_TIME_HOUR)
			{
				startIndex = validCnt;
				break;
			}
			startIndexCnt ++;
		}

		//检查完整睡眠的停止时间
		for (validCnt = 0; validCnt < sleepRecodeCacheCnt; validCnt ++)
		{
			UtcTransformTime(sleepRecodeCache[validCnt].StopUTC, &timeTemp);

			if (timeTemp.hour <= STOP_TIME_HOUR)
			{
				stopIndex = validCnt;
			}
			else
			{
				//stopIndex = validCnt;//
				break;
			}
			stopIndexCnt ++;
		}
		
		//有未存储的睡眠记录
		if (startIndex > 0)
		{
			//先保存未存储的记录
			for (validCnt = 0; validCnt < startIndex; validCnt ++)
			{
				dataInfo.sleepInUtc 	= sleepRecodeCache[validCnt].StartUTC;
				dataInfo.wakeUpUtc 		= sleepRecodeCache[validCnt].StopUTC;
				dataInfo.dataClassify 	= SLEEP_DATA_CLASSIFY;
				dataInfo.wakeUpCnt 		= 0;
				dataInfo.validId 		= 0xff;
				//创建目录
				Mid_SleepData_CreateCatalog(&dataInfo);
				#if 1 //作一次数据虚存，信息无用,目的在于避免有目录无数据时，判断目录无效，进行目录清除
				dataBuf[0] 	= 0;
				dataBuf[1] 	= 0;
				dataBuf[2] 	= 0;
				dataBuf[3] 	= 0;
				dataBuf[4] 	= 0;
				dataBuf[5] 	= 0;
				dataBuf[6] 	= 0;
				dataBuf[7] 	= 0;	
				Mid_SleepData_ClassifyDataSave(dataBuf, 8, SLEEP_DATA_CLASSIFY);
				#endif

				//无醒来记录保存
				Mid_SleepData_DataSaveEnd(SLEEP_DATA_CLASSIFY);
				sleepNumCnt ++;
				if (stopIndex < startIndex)
				{
					stopIndex ++;
				}
			}

			//完整记录保存
			dataInfo.sleepInUtc 	= sleepRecodeCache[startIndex].StartUTC;
			dataInfo.wakeUpUtc 		= sleepRecodeCache[stopIndex].StopUTC;
			dataInfo.dataClassify 	= SLEEP_DATA_CLASSIFY;
			dataInfo.wakeUpCnt 		= stopIndex - startIndex;
			dataInfo.validId 		= 0xff;
			Mid_SleepData_CreateCatalog(&dataInfo);
			sleepNumCnt ++;

			//完整记录中有醒来
			if (startIndex != stopIndex)
			{
				if (startIndex < stopIndex)
				{		
					//时间顺序匹配，进行保存
					for (validCnt = startIndex + 1; validCnt < stopIndex; validCnt ++)
					{
						//
						dataBuf[0] 	= (sleepRecodeCache[validCnt - 1].StopUTC >> 24)&0xff;
						dataBuf[1] 	= (sleepRecodeCache[validCnt - 1].StopUTC >> 16)&0xff;
						dataBuf[2] 	= (sleepRecodeCache[validCnt - 1].StopUTC >> 8)&0xff;
						dataBuf[3] 	= (sleepRecodeCache[validCnt - 1].StopUTC >> 0)&0xff;//醒来时间
						dataBuf[4] 	= (sleepRecodeCache[validCnt].StartUTC >> 24)&0xff;
						dataBuf[5] 	= (sleepRecodeCache[validCnt].StartUTC >> 16)&0xff;
						dataBuf[6] 	= (sleepRecodeCache[validCnt].StartUTC >> 8)&0xff;
						dataBuf[7] 	= (sleepRecodeCache[validCnt].StartUTC >> 0)&0xff;	//入睡时间

						Mid_SleepData_ClassifyDataSave(dataBuf, 8, SLEEP_DATA_CLASSIFY);
						sleepDuarationTemp += sleepRecodeCache[validCnt].DurationM;
						sleepNumCnt ++;
					}
				}
			}
			else
			{
				#if 1 //作一次数据虚存，信息无用
				dataBuf[0] 	= 0;
				dataBuf[1] 	= 0;
				dataBuf[2] 	= 0;
				dataBuf[3] 	= 0;
				dataBuf[4] 	= 0;
				dataBuf[5] 	= 0;
				dataBuf[6] 	= 0;
				dataBuf[7] 	= 0;	
				Mid_SleepData_ClassifyDataSave(dataBuf, 8, SLEEP_DATA_CLASSIFY);
				#endif
			}
			Mid_SleepData_DataSaveEnd(SLEEP_DATA_CLASSIFY);
			//本地最近一次睡眠信息
			UtcTransformTime(sleepRecodeCache[startIndex].StartUTC, &timeTemp);
			sleepRecode.StartHour = timeTemp.hour;
			sleepRecode.StartMin  = timeTemp.min;
			UtcTransformTime(sleepRecodeCache[stopIndex].StopUTC, &timeTemp);
			sleepRecode.StopHour  = timeTemp.hour;
			sleepRecode.StopMin   = timeTemp.min;
			sleepRecode.DurationM  = sleepRecodeCache[startIndex].DurationM + sleepDuarationTemp;
			sleepRecode.Quality   = sleepRecodeCache[startIndex].Quality;//是否需要作平均　
		}
		else
		{
			//第一个sleepRecodeCache[0]为完整记录的开始
			if (startIndexCnt == 0)
			{
				//只有一个睡眠，中间无醒来
				if (startIndex == stopIndex)
				{
					//完整记录保存
					dataInfo.sleepInUtc 	= sleepRecodeCache[startIndex].StartUTC;
					dataInfo.wakeUpUtc 		= sleepRecodeCache[stopIndex].StopUTC;
					dataInfo.dataClassify 	= SLEEP_DATA_CLASSIFY;
					dataInfo.wakeUpCnt 		= 0;
					dataInfo.validId 		= 0xff;
					Mid_SleepData_CreateCatalog(&dataInfo);
					#if 1 //作一次数据虚存，信息无用
					dataBuf[0] 	= 0;
					dataBuf[1] 	= 0;
					dataBuf[2] 	= 0;
					dataBuf[3] 	= 0;
					dataBuf[4] 	= 0;
					dataBuf[5] 	= 0;
					dataBuf[6] 	= 0;
					dataBuf[7] 	= 0;	
					Mid_SleepData_ClassifyDataSave(dataBuf, 8, SLEEP_DATA_CLASSIFY);
					#endif
					Mid_SleepData_DataSaveEnd(SLEEP_DATA_CLASSIFY);
					sleepNumCnt ++;
				}
				else //有醒来		
				{
					//完整记录保存
					dataInfo.sleepInUtc 	= sleepRecodeCache[startIndex].StartUTC;
					dataInfo.wakeUpUtc 		= sleepRecodeCache[stopIndex].StopUTC;
					dataInfo.dataClassify 	= SLEEP_DATA_CLASSIFY;
					dataInfo.wakeUpCnt 		= stopIndex - startIndex;
					dataInfo.validId 		= 0xff;
					Mid_SleepData_CreateCatalog(&dataInfo);
					sleepNumCnt ++;
					if (startIndex < stopIndex)
					{		
						//时间顺序匹配，进行保存
						for (validCnt = startIndex + 1; validCnt < stopIndex; validCnt ++)
						{
							//
							dataBuf[0] 	= (sleepRecodeCache[validCnt - 1].StopUTC >> 24)& 0xff;
							dataBuf[1] 	= (sleepRecodeCache[validCnt - 1].StopUTC >> 16)& 0xff;
							dataBuf[2] 	= (sleepRecodeCache[validCnt - 1].StopUTC >> 8)& 0xff;
							dataBuf[3] 	= (sleepRecodeCache[validCnt - 1].StopUTC >> 0)& 0xff;//醒来时间
							dataBuf[4] 	= (sleepRecodeCache[validCnt].StartUTC >> 24)& 0xff;
							dataBuf[5] 	= (sleepRecodeCache[validCnt].StartUTC >> 16)& 0xff;
							dataBuf[6] 	= (sleepRecodeCache[validCnt].StartUTC >> 8)& 0xff;
							dataBuf[7] 	= (sleepRecodeCache[validCnt].StartUTC >> 0)& 0xff;	//入睡时间
							Mid_SleepData_ClassifyDataSave(dataBuf, 8, SLEEP_DATA_CLASSIFY);
							sleepDuarationTemp += sleepRecodeCache[validCnt].DurationM;
							sleepNumCnt ++;
						}
					}
					else	//不应该出现在这里
					{
						#if 1 //作一次数据虚存，信息无用
						dataBuf[0] 	= 0;
						dataBuf[1] 	= 0;
						dataBuf[2] 	= 0;
						dataBuf[3] 	= 0;
						dataBuf[4] 	= 0;
						dataBuf[5] 	= 0;
						dataBuf[6] 	= 0;
						dataBuf[7] 	= 0;	
						Mid_SleepData_ClassifyDataSave(dataBuf, 8, SLEEP_DATA_CLASSIFY);
						#endif
					}
					Mid_SleepData_DataSaveEnd(SLEEP_DATA_CLASSIFY);
				}
				//本地最近一次睡眠信息
				UtcTransformTime(sleepRecodeCache[startIndex].StartUTC, &timeTemp);
				sleepRecode.StartHour = timeTemp.hour;
				sleepRecode.StartMin  = timeTemp.min;
				UtcTransformTime(sleepRecodeCache[stopIndex].StopUTC, &timeTemp);
				sleepRecode.StopHour  = timeTemp.hour;
				sleepRecode.StopMin   =	timeTemp.min;			 
				sleepRecode.DurationM = sleepRecodeCache[startIndex].DurationM;
				sleepRecode.Quality   = sleepRecodeCache[startIndex].Quality;//是否需要作平均　
			}	
		}
		//未存储的记录前移	
		for (validCnt = 0; validCnt < sleepRecodeCacheCnt; validCnt ++)
		{
			sleepRecodeCache[validCnt] = sleepRecodeCache[validCnt + sleepNumCnt];
		}
		sleepRecodeCacheCnt -= sleepNumCnt;	 
		startIndex 		= 0;
		stopIndex  		= 0;
		startIndexCnt 	= 0;
		stopIndexCnt 	= 0;
		sleepDuarationTemp = 0;
		sleepNumCnt 	= 0;
		roundCnt ++;

		if (roundCnt >= SLEEP_RECORD_MAX)
		{
			break;
		}
	}
}

//**********************************************************************
// 函数功能：   睡眠信息获取-UI交互获取
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
void App_SleepData_Read(sleep_ui_info *sleepInfoTemp)
{
	Mid_SleepScene_CurSleepGet(sleepInfoTemp);
}

//**********************************************************************
// 函数功能:	返回睡眠目录条数
// 输入参数：	
// 返回参数：	0x00: 操作成功
// 				0xff: 操作失败
//**********************************************************************
uint16 App_SleepData_DataRequestTotalInfoAck(uint8 flowControl)
{
	uint16 catalogNum, result;
	uint32 byteLen;
	ble_msg_t bleMsg;

	//进行一次数据处理　
	App_SleepData_Save();

	//根据数据类型获取总目录信息、目录条数、数据总长度
	result = Mid_SleepData_ClassifyDataInfoRead(&catalogNum, &byteLen, SLEEP_DATA_CLASSIFY);			
	
	if(result == 0xff)
	{
		return result;
	}
	else
	{
		// 返回长度与目录数量
		bleMsg.id = BLE_MSG_SEND;
		bleMsg.packet = PROT_SLEEP_TOTAL_RET;
		bleMsg.packet.att.load.content.parameter[0]	= (uint8)(catalogNum);
		bleMsg.packet.att.flowControl = flowControl;
        Mid_Ble_SendMsg(&bleMsg);
	}
	return 0;
}

//**********************************************************************
// 函数功能:	返回对应请求的目录详细信息给APP
// 输入参数：	
// 返回参数：	0x00: 操作成功
// 				0xff: 操作失败
//**********************************************************************
uint16 App_SleepData_DataCatalogInfoRead(uint16 catalogNum, uint8 flowControl)
{
	uint16 result;
	ble_msg_t bleMsg;
	sleep_scene_catalogInfo_s dataInfo;

	result = Mid_SleepData_ClassifyDataCatalogRead(&dataInfo, SLEEP_DATA_CLASSIFY, catalogNum);

	if(result == 0xff)
	{
		return 0xff;
	}

	wakeupCntTemp 		= dataInfo.wakeUpCnt;
	datauploadTotalLen  = 0;
	curCatalogNumUpload = catalogNum;
	indexUtcTemp 		= dataInfo.sleepInUtc;
	// 返回对应序号目录详细信息
	bleMsg.id = BLE_MSG_SEND;
	bleMsg.packet = PROT_SLEEP_CATALOG_RET;
	bleMsg.packet.att.load.content.parameter[0]	= (uint8)(catalogNum);
	bleMsg.packet.att.load.content.parameter[1]	= (uint8)(dataInfo.wakeUpCnt);

	bleMsg.packet.att.load.content.parameter[2]	= (uint8)(dataInfo.sleepInUtc >> 24);
	bleMsg.packet.att.load.content.parameter[3]	= (uint8)(dataInfo.sleepInUtc >> 16);
	bleMsg.packet.att.load.content.parameter[4]	= (uint8)(dataInfo.sleepInUtc >> 8);
	bleMsg.packet.att.load.content.parameter[5]	= (uint8)(dataInfo.sleepInUtc);

	bleMsg.packet.att.load.content.parameter[6]	= (uint8)(dataInfo.wakeUpUtc >> 24);
	bleMsg.packet.att.load.content.parameter[7]	= (uint8)(dataInfo.wakeUpUtc >> 16);
	bleMsg.packet.att.load.content.parameter[8]	= (uint8)(dataInfo.wakeUpUtc >> 8);
	bleMsg.packet.att.load.content.parameter[9]	= (uint8)(dataInfo.wakeUpUtc);

	bleMsg.packet.att.flowControl = flowControl;
    Mid_Ble_SendMsg(&bleMsg);

    if (wakeupCntTemp > 0)
    {
    	App_SleepData_DataReUploadTimerStart();
    }
    else
    {
    	Mid_SleepData_DeleteClassifyDataUtc(SLEEP_DATA_CLASSIFY, indexUtcTemp);
    }

	return 0;
}

//**********************************************************************
// 函数功能:	数据上传处理，当MCU接收到BLE数据上传送达成功时调用该函数(指定数据包后，从指定包号开始上传)
// 输入参数：	
// 返回参数：	0x00: 操作成功
// 				0xff: 操作失败
//**********************************************************************
uint16 App_SleepData_DataUploadProcess(void)
{
	uint16 result;
	ble_msg_t bleMsg;

	 // SEGGER_RTT_printf(0,"App_PackData_DataUploadProcess\n");
	// 数据上传完毕
	if(wakeupCntTemp == 0)
	{
		App_SleepData_DataReUploadTimerStop();
		Mid_SleepData_DeleteClassifyDataUtc(SLEEP_DATA_CLASSIFY, indexUtcTemp);
		return 0;
	}

	bleMsg.id			= BLE_MSG_SEND;
	bleMsg.packet	= PROT_SLEEP_SUB_DATA_UPLOAD;

	bleMsg.packet.att.load.content.parameter[0]	= (uint8)(curCatalogNumUpload);
	bleMsg.packet.att.load.content.parameter[1]	= (uint8)(curWakeupCntUpload);

	memset((bleMsg.packet.att.load.content.parameter + 2),0xff,SLEEP_DATA_UPLOAD_MAX_LEN);
	result = Mid_PackData_ClassifyDataRead((bleMsg.packet.att.load.content.parameter + 2), 
											datauploadTotalLen, SLEEP_DATA_UPLOAD_MAX_LEN, indexUtcTemp, SLEEP_DATA_CLASSIFY);

	// 无效数据
	if(result == 0xff)
	{
		App_SleepData_DataReUploadTimerStop();
		return 0xff;
	}
	// 发送数据
    Mid_Ble_SendMsg(&bleMsg);

	//包号递增、数据长度改变
	wakeupCntTemp --;
	curWakeupCntUpload ++;
	datauploadTotalLen += SLEEP_DATA_UPLOAD_MAX_LEN;

	return 0x00;
}



///////////////////////////////////////////////////////////////////////////////////////////////
		/******************************** 开发测试******************************************/
//////////////////////////////////////////////////////////////////////////////////////////////



//**********************************************************************
// 函数功能:	制造假数据心率
// 输入参数：	
// 返回参数：
//**********************************************************************
void App_SleepData_DataSaveFakeData(void)
{
	
}

// 函数功能:	存储数据打印（在APP获取前打印出来）
// 输入参数：	
// 返回参数：

void App_SleepData_DataSavePrintf(uint16_t dataClassify)
{
	#if 0
	uint32_t 		i;
	uint16_t 		result;
	uint16_t 		catalogNumTotal,catalogIndex;
	uint32_t 		dataLenTotal;
	uint32_t 		utcPackNum,utcPackIndex;
	uint32_t 		lengthTemp,addrOffset,utcToltalLen;
	uint32_t 		curUtc;
	catalogInfo_s 	dataInfo;
	uint8_t 		databuf[APP_DATA_UPLOAD_MAX_LEN];
	uint32_t 		catalogStartAddr;
	uint32_t 		dataStartAddr;

	catalogNumTotal 			= 0;
	dataLenTotal 				= 0;
	addrOffset 					= 0;
	utcToltalLen 				= 0;
	curUtc 						= 0;
	lengthTemp 					= 0;
	catalogStartAddr 			= 0;
	dataStartAddr 				= 0;

	//获取总目录
	result = ClassifyTotalCatalogRead(&catalogNumTotal, &dataLenTotal, dataClassify,&catalogStartAddr);
	SEGGER_RTT_printf(0,"TotalcatalogNum: 	%d\n", catalogNumTotal);
	SEGGER_RTT_printf(0,"TotaldataLen: 		%d\n", dataLenTotal);

	//无目录
	if (catalogNumTotal == 0 && dataLenTotal == 0)
	{
		return;
	}
	//目录遍历
	for (catalogIndex = 0; catalogIndex < catalogNumTotal; catalogIndex++)
	{		
		//获取目录信息
		SEGGER_RTT_printf(0,"address: 	%x\n", catalogStartAddr);
		result 			  = ClassifyCatalogInfoRead(&dataInfo, dataClassify, catalogStartAddr);
		catalogStartAddr += CATALOG_INFO_LENGTH;

		//请求指定目录的数据
		dataStartAddr 			= dataInfo.startAddr;
		utcToltalLen 			= dataInfo.dataLength;
		utcPackNum 				= utcToltalLen / APP_DATA_UPLOAD_MAX_LEN; //包数据统计  /16
		if (utcToltalLen % APP_DATA_UPLOAD_MAX_LEN)
		{
			utcPackNum ++;
		}
		curUtc 					= dataInfo.utc;

		SEGGER_RTT_printf(0,"catalog:   %d\n", catalogIndex);
		SEGGER_RTT_printf(0,"totalLen:  %d\n", utcToltalLen);
		SEGGER_RTT_printf(0,"totalPack: %d\n", utcPackNum);
		//数据包遍历
		for(utcPackIndex = 0; utcPackIndex < utcPackNum; utcPackIndex++)
		{	
			addrOffset 		= utcPackIndex * APP_DATA_UPLOAD_MAX_LEN;

			if(utcToltalLen< APP_DATA_UPLOAD_MAX_LEN)
			{
				lengthTemp	= utcToltalLen;
			}
			else
			{
				lengthTemp	= APP_DATA_UPLOAD_MAX_LEN;
			}
			memset(databuf,0xff,APP_DATA_UPLOAD_MAX_LEN);
			//读取一包数据
			result = ClassifyDataPreRead(databuf, (dataStartAddr + addrOffset), lengthTemp,dataClassify);

			//数据打印
			SEGGER_RTT_printf(0,"%8x %4x %4x ",curUtc,utcPackIndex,dataClassify);

			for (i = 0; i < APP_DATA_UPLOAD_MAX_LEN / 2; i++)
			{
				SEGGER_RTT_printf(0,"%4x%4x",databuf[2*i],databuf[2*i+1]);												
			}
			SEGGER_RTT_printf(0,"\n");	

			utcToltalLen 	-= lengthTemp;	
		}
		vTaskDelay(15);
	}
	#endif
}
