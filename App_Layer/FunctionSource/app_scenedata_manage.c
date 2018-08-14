#include "platform_common.h"
/* FreeRTOS includes */
#include "rtos.h"
#include "mid_interface.h"
#include "app_variable.h"
#include "app_scenedata_manage.h"
#include "flash_task.h" 
#include "ble_task.h"
#include "multimodule_task.h"
#include "app_win_common.h"
#include "app_win_process.h"




// 分类数据的目录数量与数据长度返回协议
static const protocal_msg_t	PROT_SCENE_TRANS_CATA_TOTAL_RET =
{
	0x23, 0x01, 0x0B, 0x13, 0x00, 0x08, 0xf2, 0x10,
};

// 分类数据的目录内容返回协议
static const protocal_msg_t	PROT_SCENE_TRANS_CATA_RET =
{
	0x23, 0x01, 0x03, 0x13, 0x00, 0x08, 0xf2, 0x11,
};

// 分类数据的数据包请求结果反馈
static const protocal_msg_t	PROT_SCENE_TRANS_DATA_RET =
{
	0x23, 0x01, 0x04, 0x13, 0x00, 0x08, 0xf2, 0x12,
};

// 删除所有数据反馈
static const protocal_msg_t	PROT_SCENE_TRANS_DELETE_ALL_RET =
{
	0x23, 0x01, 0x04, 0x13, 0x00, 0x80, 0xf2, 0x30,
};

// 删除某类数据反馈
static const protocal_msg_t	PROT_SCENE_TRANS_DELETE_CLASS_RET =
{
	0x23, 0x01, 0x06, 0x13, 0x00, 0x80, 0xf2, 0x31,
};

// 删除目录反馈
static const protocal_msg_t	PROT_SCENE_TRANS_DELETE_CATALOG_RET =
{
	0x23, 0x01, 0x0a, 0x13, 0x00, 0x80, 0xf2, 0x32,
};

// 数据包上传协议
static const protocal_msg_t	PROT_SCENE_TRANS_DATA_UPLOAD =
{
	0x23, 0x01, 0x17, 0x12, 0x80, 0x01, 0xf0, 0x01,
};


// 场景缓存相关宏
#define			APP_SCENE_DATA_BUFF_MAX			15		//缓存数组长度：应满足各场景的公倍数
#define			APP_SCENE_UPLOAD_VALID_LEN		15		//数据上传包20字节，数据部分16字节，为不分析，每次上传15字节有效数据
#define			APP_SCENE_UPLOAD_MAX_LEN		16	
#define			APP_SCENE_CATALOG_PACK_LEN		10		//每包目录有效信息长度


//各场景存储数据最小的单元长度,与各场景数据结构体对应　
#define			APP_RUN_SCENE_DATA_UNIT_LEN			5	
#define			APP_CLIMB_SCENE_DATA_UNIT_LEN		5
#define			APP_SWING_SCENE_DATA_UNIT_LEN		3

//各场景实际上传目录信息
// #define			RUN_SCENE_CATALOG_UPLOAD_LENGTH 	47
// #define			CLIMB_SCENE_CATALOG_UPLOAD_LENGTH 	49
// #define			SWING_SCENE_CATALOG_UPLOAD_LENGTH 	43

#define	 		App_Scene_Data_ReUploadInternal 	20//50ms

#define 		App_SceneData_Max_List 				2 	//最大存储场景数据条数



static uint8 appSaveInitFlag = 0;

// 场景相关变量
static uint16 appScneBuffCnt;							//缓存数据计数
static uint8  sceneDataBuff[APP_SCENE_DATA_BUFF_MAX];	//缓存数组
static uint16 appSceneSaveState;						//场景数据存储状态

// 数据上传相关变量
static uint32 appDataUnuploadLen;					//未上传数据长度
static uint32 appDatauploadTotalLen;				//该目录的总长度
static uint16 appDataCurUploadPackNum;				//当前上传的数据包序号
static uint16 appDataCurUploadClassify;				//当前上传的数据分类
static uint32 appDataCurCatalogUtc;					//当前上传的目录UTC

static uint8  appUploadState; 						//标记当前传输状态
static uint8  flowControlTemp;
static uint16 catalogFlow;
static uint8  catalogLenTemp;
static uint32 dataClassifyTemp;
static uint8  catalogLenUploadLen;
static uint8  catalogBuff[SCENE_CATALOG_INFO_LENGTH];
static uint8  catalogBuffCnt;

static uint8  appSceneDataCacheCnt[SCENE_DM_APP_DATA_CLASSIFY_NUM];

enum
{
	DATA_SAVE_IDLE		= 0,
	DATA_SAVE_SAVING,
};

enum
{
	UPLOAD_IDLE 		= 0,
	UPLOAD_CATALOG,
	UPLOAD_DATA_PACK,
}app_upload_s;

static TimerHandle_t App_SceneData_DataReUploadTimer		= NULL;				// 更新定时器定时器


//**********************************************************************
// 函数功能:	开启延时上传定时器
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
void App_SceneData_DataReUploadTimerStart(void)
{
	if (xTimerIsTimerActive(App_SceneData_DataReUploadTimer) == pdFALSE)
    {
		xTimerReset(App_SceneData_DataReUploadTimer, 1);	
		xTimerStart(App_SceneData_DataReUploadTimer, 1);
	}
}

 //**********************************************************************
// 函数功能:	关闭延时上传定时器
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
void App_SceneData_DataReUploadTimerStop(void)
{
	if (xTimerIsTimerActive(App_SceneData_DataReUploadTimer) != pdFALSE)
    {
		xTimerStop(App_SceneData_DataReUploadTimer, 1);
	}
}


//**********************************************************************
// 函数功能:	初始化场景数据存储
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
void App_SceneData_DataSaveInit(void (*TimerCb)(TimerHandle_t xTimer))
{
	Mid_SceneData_Init();
	Mid_SceneData_ClassifyDataInit(RUN_SCENE_DATA_CLASSIFY,   4 * DM_APP_SECTOR_LENGTH, 10 * DM_APP_SECTOR_LENGTH);
	Mid_SceneData_ClassifyDataInit(CLIMB_SCENE_DATA_CLASSIFY, 4 * DM_APP_SECTOR_LENGTH, 10 * DM_APP_SECTOR_LENGTH);
	Mid_SceneData_ClassifyDataInit(SWING_SCENE_DATA_CLASSIFY, 4 * DM_APP_SECTOR_LENGTH, 10 * DM_APP_SECTOR_LENGTH);

	appScneBuffCnt		= 0;
	appSceneSaveState	= DATA_SAVE_IDLE;
	appUploadState 		= UPLOAD_IDLE;
	catalogBuffCnt 		= 0;
	catalogLenUploadLen = 0;
	memset(catalogBuff, 0, SCENE_CATALOG_INFO_LENGTH);
	memset(appSceneDataCacheCnt, 0, SCENE_DM_APP_DATA_CLASSIFY_NUM);
	if (appSaveInitFlag == 0)
	{
		App_SceneData_DataReUploadTimer = xTimerCreate("reupload", (APP_1SEC_TICK / App_Scene_Data_ReUploadInternal), pdTRUE, 0, TimerCb);
		appSaveInitFlag 			= 1;
	}
}

//**********************************************************************
// 函数功能:	根据存储间隔调用一次，自动判定是否需要存储
// 输入参数:	
//　utc:		　该次存储的UTC时间
// sceneDateTemp:　该次需存储的数据
// 返回参数:	无
//**********************************************************************
void App_SceneData_SaveDynamic(uint32 utc, uint32 sceneDataType, scene_data_s *sceneDateTemp)
{
	scene_catalogInfo_s	catalogInfoTemp;
	uint8 			unitLength;
	uint8 			buftemp[10];
	uint16 		 	result;

	//各场景存储数据差异,//高位在前的存储格式与协议传输相符
	switch(sceneDataType)
	{
		case RUN_SCENE_DATA_CLASSIFY:
		unitLength	= APP_RUN_SCENE_DATA_UNIT_LEN;
		buftemp[0]  = (uint8)(sceneDateTemp->runScene.paceLog >> 8);
		buftemp[1]  = (uint8)(sceneDateTemp->runScene.paceLog);
		buftemp[2]  = (uint8)(sceneDateTemp->runScene.freqLog >> 8);
		buftemp[3]  = (uint8)(sceneDateTemp->runScene.freqLog);
		buftemp[4]  = (uint8)(sceneDateTemp->runScene.hrmLog);
		break;

		case CLIMB_SCENE_DATA_CLASSIFY:
		unitLength	= APP_CLIMB_SCENE_DATA_UNIT_LEN;
		buftemp[0]  = (uint8)(sceneDateTemp->climbScene.elevation >> 8);
		buftemp[1]  = (uint8)(sceneDateTemp->climbScene.elevation);
		buftemp[2]  = (uint8)(sceneDateTemp->climbScene.climbSpeed >> 8);
		buftemp[3]  = (uint8)(sceneDateTemp->climbScene.climbSpeed);
		buftemp[4]  = (uint8)(sceneDateTemp->climbScene.hrmLog);
		break;

		case SWING_SCENE_DATA_CLASSIFY:
		unitLength	= APP_SWING_SCENE_DATA_UNIT_LEN;
		buftemp[0]  = (uint8)(sceneDateTemp->swingScene.pullLog >> 8);
		buftemp[1]  = (uint8)(sceneDateTemp->swingScene.pullLog);
		buftemp[2]  = (uint8)(sceneDateTemp->swingScene.hrmLog);
		break;
	}	

	//未创建目录，需创建
	if(DATA_SAVE_IDLE == appSceneSaveState)
	{ 
		catalogInfoTemp.utc				= utc;
		catalogInfoTemp.dataClassify	= sceneDataType;
		catalogInfoTemp.sampleUnit		= DATASAMPLE_UNIT_10S;
		catalogInfoTemp.sampleInterval	= 1;
		catalogInfoTemp.unitLength		= unitLength;
		Mid_SceneData_CreateCatalog(&catalogInfoTemp);
		appSceneSaveState				= DATA_SAVE_SAVING;	
	}	

	//数据存储格式调整完放入存储存储缓存
	for (int i = 0; i < unitLength; ++i)
	{
		sceneDataBuff[appScneBuffCnt] = buftemp[i];
		appScneBuffCnt ++;
	}

	// 达到存储的长度
	if(appScneBuffCnt >= APP_SCENE_DATA_BUFF_MAX)
	{
		result = Mid_SceneData_ClassifyDataSave((uint8_t*)sceneDataBuff, appScneBuffCnt, sceneDataType);

		//存储的数据过长（暂定5小时）,结束目录并存储缓存数据
		if (result == STORAGE_OVERFLOW)
		{
			App_SceneData_SaveCache(sceneDataType);
		}
		appScneBuffCnt			= 0;
	}
}

//**********************************************************************
// 函数功能:	将未存入的buf缓存数据,存入数据中，场景退出时调用,执行完产生存储完窗口通知事件　
// 输入参数：	无
// 返回参数：	无
//**********************************************************************

void App_SceneData_SaveEnd(uint32 sceneDataType)
{
	scene_detail_s sceneDetailTemp;
	menuMessage        message;

	if(DATA_SAVE_SAVING == appSceneSaveState)
	{
		Mid_Scene_DetailRead(sceneDataType, &sceneDetailTemp);
		if(appScneBuffCnt > 0)
		{
			Mid_SceneData_ClassifyDataSave((uint8_t*)sceneDataBuff, appScneBuffCnt, sceneDataType);		
		}

		Mid_SceneData_DataSaveEnd(sceneDataType,&sceneDetailTemp);
		appScneBuffCnt			= 0;
		appSceneSaveState		= DATA_SAVE_IDLE;

		if (appSceneDataCacheCnt[sceneDataType] < 0xff)
		{
			appSceneDataCacheCnt[sceneDataType] ++;
		}
	}

	//窗口存储完通知事件
	message.state = SAVE_SUCCESS_STATE;
	message.op    = SAVE_REMIND;
    WinDowHandle  = App_Window_Process(WinDowHandle,TAG_REMIND,message);
}

//**********************************************************************
// 函数功能:	检查存储空间
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
void App_SceneData_StorageSpaceCheck(uint32 sceneDataType)
{
	menuMessage        message;

	if (appSceneDataCacheCnt[sceneDataType] >= App_SceneData_Max_List)
	{
		//窗口存储完通知事件
		message.state = SAVE_SPACE_FULL_STATE;
		message.op    = SAVE_REMIND;
	    WinDowHandle  = App_Window_Process(WinDowHandle,TAG_REMIND,message);
	}
}

//**********************************************************************
// 函数功能:	掉电后数据恢复，需先初始化数据分配部分才可进行
// 输入参数：	
// 返回参数：
//**********************************************************************
void App_SceneData_PowerdownRecover(void)
{
	Mid_SceneData_StorageDataRecover();
}

//**********************************************************************
// 函数功能:	历史数据存储，校准完成后对历史数据进行一次存储或PP请求数据时调用。
// 输入参数：	dataClassify：数据类型
// 返回参数：
//**********************************************************************
void App_SceneData_SaveCache(uint32 sceneDataType)
{
	scene_detail_s sceneDetailTemp;

	if(DATA_SAVE_SAVING == appSceneSaveState)
	{
		Mid_Scene_DetailRead(sceneDataType, &sceneDetailTemp);
		if(appScneBuffCnt > 0)
		{
			Mid_SceneData_ClassifyDataSave((uint8_t*)sceneDataBuff, appScneBuffCnt, sceneDataType);		
		}

		Mid_SceneData_DataSaveEnd(sceneDataType,&sceneDetailTemp);
		appScneBuffCnt			= 0;
		appSceneSaveState		= DATA_SAVE_IDLE;
	}
}


//**********************************************************************
// 函数功能:	返回请求的数据分类总数据长度与数据目录数量，
// 输入参数：	
// 返回参数：	0x00: 操作成功
// 				0xff: 操作失败
//**********************************************************************
uint16 App_SceneData_DataRequestTotalInfoAck(uint32 dataClassify, uint8 flowControl)
{
	uint16 		catalogNum, result;
	uint32 		byteLen;
	ble_msg_t 	bleMsg;

	//数据类型枚举协议与存储不一致，需作调整
	dataClassifyTemp 	= dataClassify - BASE_SCENE_DATA_CLASSIFY;
	// 将缓存存入flash
	App_SceneData_SaveCache(dataClassifyTemp);
		
	//重新开始请求，把之前的数据上传关停
	App_SceneData_DataReUploadTimerStop();

	//根据数据类型获取总目录信息、目录条数、数据总长度
	result = Mid_SceneData_ClassifyDataInfoRead(&catalogNum, &byteLen, dataClassifyTemp);
	
	if(result == 0xff)
	{
		return result;
	}
	else
	{
		// 返回长度与目录数量
		bleMsg.id = BLE_MSG_SEND;
		bleMsg.packet= PROT_SCENE_TRANS_CATA_TOTAL_RET;
		bleMsg.packet.att.load.content.parameter[0]	= (uint8)(dataClassify >> 8);
		bleMsg.packet.att.load.content.parameter[1]	= (uint8)(dataClassify);
		bleMsg.packet.att.load.content.parameter[2]	= (uint8)(catalogNum >> 8);
		bleMsg.packet.att.load.content.parameter[3]	= (uint8)(catalogNum);
		bleMsg.packet.att.load.content.parameter[4]	= (uint8)(byteLen >> 24);
		bleMsg.packet.att.load.content.parameter[5]	= (uint8)(byteLen >> 16);
		bleMsg.packet.att.load.content.parameter[6]	= (uint8)(byteLen >> 8);
		bleMsg.packet.att.load.content.parameter[7]	= (uint8)(byteLen);
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
uint16 App_SceneData_DataCatalogInfoRead(uint32 dataClassify, uint16 catalogNum, uint8 flowControl)
{
	uint8 		catalogBuffIndex;
	uint8 		i;
	uint16 		result;
	ble_msg_t 	bleMsg;
	scene_catalogInfo_s dataInfo;
   
	
	dataClassifyTemp = dataClassify - BASE_SCENE_DATA_CLASSIFY;
	//获取目录信息
	result = Mid_SceneData_ClassifyDataCatalogRead(&dataInfo, dataClassifyTemp, catalogNum);

	if(result == 0xff)
	{
		return 0xff;
	}

	catalogBuffCnt 	 = 0;
	catalogBuffIndex = 0;
	memset(catalogBuff, 0xff, SCENE_CATALOG_INFO_LENGTH);

	//通用部分调整与协议传输大小端一致
	catalogBuff[catalogBuffIndex++] = (uint8)(dataClassify >> 8);
	catalogBuff[catalogBuffIndex++] = (uint8)(dataClassify);
	catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.utc >> 24);
	catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.utc >> 16);
	catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.utc >> 8);
	catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.utc);
	catalogBuff[catalogBuffIndex++]	= (uint8)(dataInfo.dataLength >> 8);
	catalogBuff[catalogBuffIndex++]	= (uint8)(dataInfo.dataLength);
	catalogBuff[catalogBuffIndex++]	= (uint8)((dataInfo.sampleUnit << 4) | (dataInfo.sampleInterval >> 12));	
	catalogBuff[catalogBuffIndex++]	= (uint8)(dataInfo.sampleInterval);
	catalogBuff[catalogBuffIndex++]	= (uint8)(dataInfo.unitLength);

	//根据不同类型数据拷贝成存储格式与协议一致
	switch(dataClassifyTemp)
	{
		case RUN_SCENE_DATA_CLASSIFY:		
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.runDetail.startUtc >> 24);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.runDetail.startUtc >> 16);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.runDetail.startUtc >> 8);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.runDetail.startUtc);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.runDetail.duarationTotal >> 24);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.runDetail.duarationTotal >> 16);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.runDetail.duarationTotal >> 8);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.runDetail.duarationTotal);
		catalogBuffIndex += 20;//分段部分暂未定义，跳过
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.runDetail.distanceTotal >> 8);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.runDetail.distanceTotal);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.runDetail.stepTotal >> 24);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.runDetail.stepTotal >> 16);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.runDetail.stepTotal >> 8);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.runDetail.stepTotal);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.runDetail.calorieTotal >> 8);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.runDetail.calorieTotal);
		break;

		case CLIMB_SCENE_DATA_CLASSIFY:
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.climbDetail.startUtc >> 24);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.climbDetail.startUtc >> 16);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.climbDetail.startUtc >> 8);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.climbDetail.startUtc);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.climbDetail.duarationTotal >> 24);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.climbDetail.duarationTotal >> 16);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.climbDetail.duarationTotal >> 8);
		catalogBuff[catalogBuffIndex++] 	= (uint8)(dataInfo.detail.climbDetail.duarationTotal);
		catalogBuffIndex += 20;
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.climbDetail.upDistance >> 8);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.climbDetail.upDistance);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.climbDetail.downDistance >> 8);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.climbDetail.downDistance);

		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.climbDetail.altitudeHighest >> 8);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.climbDetail.altitudeHighest);

		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.climbDetail.altitudeLowest >> 8);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.climbDetail.altitudeLowest);

		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.climbDetail.calorieTotal >> 8);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.climbDetail.calorieTotal);
		break;

		case SWING_SCENE_DATA_CLASSIFY:
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.swingDetail.startUtc >> 24);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.swingDetail.startUtc >> 16);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.swingDetail.startUtc >> 8);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.swingDetail.startUtc);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.swingDetail.duarationTotal >> 24);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.swingDetail.duarationTotal >> 16);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.swingDetail.duarationTotal >> 8);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.swingDetail.duarationTotal);
		catalogBuffIndex += 20;
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.swingDetail.pullTotal >> 8);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.swingDetail.pullTotal);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.swingDetail.calorieTotal >> 8);
		catalogBuff[catalogBuffIndex++] = (uint8)(dataInfo.detail.swingDetail.calorieTotal);
		break;
	}

	//需上传的目录信息总长度
	catalogLenTemp = catalogBuffIndex;
	//标记目录包号
	if (catalogLenTemp <= APP_SCENE_CATALOG_PACK_LEN)
	{
		catalogLenUploadLen = catalogLenTemp;
		catalogFlow 		= 0xff;
		catalogLenTemp 		= 0;
		appUploadState  	= UPLOAD_IDLE;
		App_SceneData_DataReUploadTimerStop();
	}
	else
	{
		catalogFlow 		= 0;
		catalogLenUploadLen = APP_SCENE_CATALOG_PACK_LEN;
		catalogLenTemp 		-= APP_SCENE_CATALOG_PACK_LEN;
	}
	// SEGGER_RTT_printf(0,"catalogBuffIndex: %d\n", catalogBuffIndex);
	// 返回对应序号目录详细信息
	bleMsg.id 	  = BLE_MSG_SEND;
	bleMsg.packet = PROT_SCENE_TRANS_CATA_RET;
	bleMsg.packet.att.loadLength += 1; 			
	bleMsg.packet.att.load.content.parameter[0] = catalogFlow; //包序号
	bleMsg.packet.att.loadLength += catalogLenUploadLen;		//目录信息
	for (i = 1; i <= catalogLenUploadLen; i++)
	{
		bleMsg.packet.att.load.content.parameter[i] = catalogBuff[catalogBuffCnt + i - 1];
	}
	// SEGGER_RTT_printf(0,"catalogFlow: %d\n", catalogFlow);
	// SEGGER_RTT_printf(0,"catalogLenUploadLen: %d\n", catalogLenUploadLen);
	// SEGGER_RTT_printf(0,"catalogBuffCnt: %d\n", catalogBuffCnt);
	bleMsg.packet.att.flowControl 				= flowControl;
    Mid_Ble_SendMsg(&bleMsg);

    //传输状态标识　
    flowControlTemp = flowControl;
    catalogBuffCnt  += catalogLenUploadLen;	//缓存计数后移
    appDataCurUploadClassify = dataClassify;

    //需分包上传目录
    if (catalogLenTemp > 0)
    {
    	appUploadState  = UPLOAD_CATALOG;
    	App_SceneData_DataReUploadTimerStart();
    }
	return 0;
}

//**********************************************************************
// 函数功能:	请求数据包，查询数据是否有效并反馈给APP，并进入发送数据的机制
// 输入参数：	
// 返回参数：	0x00: 操作成功
// 				0xff: 操作失败
//**********************************************************************
uint16 App_SceneData_DataRequestData(uint32 dataClassify, uint32 utc, uint16 packNum, uint8 flowControl)
{
	uint32 		lengthTemp;
	uint16 		result;
	ble_msg_t 	bleMsg;
   
	
	dataClassifyTemp = dataClassify - BASE_SCENE_DATA_CLASSIFY;

	//获取指定utc目录索引的数据段长度
	result = Mid_SceneData_ReadCatalogDataLen(&appDataUnuploadLen, utc, dataClassifyTemp);

	if(result == 0xff)
	{
		appUploadState = UPLOAD_IDLE;
		// 返回无效数据类型
		bleMsg.id		= BLE_MSG_SEND;
		bleMsg.packet	= PROT_SCENE_TRANS_DATA_RET;

		bleMsg.packet.att.load.content.parameter[0]	= 1;
		bleMsg.packet.att.flowControl				= flowControl;
        Mid_Ble_SendMsg(&bleMsg);
		return 0xff;
	}

	// 判断获取的数据包是否有效
	appDataCurUploadPackNum		= packNum;
	appDatauploadTotalLen		= packNum * APP_SCENE_UPLOAD_VALID_LEN;
	appDataCurUploadClassify	= dataClassify;
	appDataCurCatalogUtc		= utc;
	// 序号超过数据长度
	if(appDatauploadTotalLen >= appDataUnuploadLen)
	{
		appUploadState = UPLOAD_IDLE;

		// 返回无效数据包序号
		bleMsg.id			= BLE_MSG_SEND;
		bleMsg.packet	= PROT_SCENE_TRANS_DATA_RET;

		bleMsg.packet.att.load.content.parameter[0]	= 2;
		bleMsg.packet.att.flowControl				= flowControl;
        Mid_Ble_SendMsg(&bleMsg);

		App_SceneData_DataReUploadTimerStop();
		return 0xff;
	}

	//返回请求结果（成功）
	bleMsg.id		= BLE_MSG_SEND;
	bleMsg.packet	= PROT_SCENE_TRANS_DATA_RET;

	bleMsg.packet.att.load.content.parameter[0]	= 0;
	bleMsg.packet.att.flowControl				= flowControl;
    Mid_Ble_SendMsg(&bleMsg);
	
	appDataUnuploadLen	= appDataUnuploadLen - appDatauploadTotalLen;

	if(appDataUnuploadLen < APP_SCENE_UPLOAD_VALID_LEN)
	{
		lengthTemp	= appDataUnuploadLen;
	}
	else
	{
		lengthTemp	= APP_SCENE_UPLOAD_VALID_LEN;
	}

	bleMsg.id			= BLE_MSG_SEND;
	bleMsg.packet	= PROT_SCENE_TRANS_DATA_UPLOAD;

	bleMsg.packet.att.load.content.parameter[0]	= (uint8)(appDataCurUploadPackNum >> 8);
	bleMsg.packet.att.load.content.parameter[1]	= (uint8)(appDataCurUploadPackNum);
	bleMsg.packet.att.load.content.parameter[2]	= (uint8)(appDataCurUploadClassify >> 8);
	bleMsg.packet.att.load.content.parameter[3]	= (uint8)(appDataCurUploadClassify);

	memset((bleMsg.packet.att.load.content.parameter + 4),0xff,APP_SCENE_UPLOAD_MAX_LEN);
	result = Mid_SceneData_ClassifyDataRead((bleMsg.packet.att.load.content.parameter + 4), 
											appDatauploadTotalLen, lengthTemp, appDataCurCatalogUtc, dataClassifyTemp);

	// 无效数据
	if(result == 0xff)
	{
		appUploadState = UPLOAD_IDLE;
		// 返回无效数据包序号
		bleMsg.id			= BLE_MSG_SEND;
		bleMsg.packet	= PROT_SCENE_TRANS_DATA_RET;

		bleMsg.packet.att.load.content.parameter[0]	= 2;
		bleMsg.packet.att.flowControl				= flowControl;
        Mid_Ble_SendMsg(&bleMsg);

		App_SceneData_DataReUploadTimerStop();
		return 0xff;
	}

	// 发送数据
	bleMsg.packet.att.flowControl				= flowControl;
    Mid_Ble_SendMsg(&bleMsg);

    //启动分包数据定时发送
    App_SceneData_DataReUploadTimerStart();
	appDataCurUploadPackNum++;
	appDatauploadTotalLen += lengthTemp;
	appDataUnuploadLen	-= lengthTemp;
	appUploadState 		= UPLOAD_DATA_PACK;

	return 0x00;
}

//**********************************************************************
// 函数功能:	数据上传处理，当MCU接收到BLE数据上传送达成功时调用该函数(指定数据包后，从指定包号开始上传)
// 输入参数：	
// 返回参数：	0x00: 操作成功
// 				0xff: 操作失败
//**********************************************************************
uint16 App_SceneData_DataUploadProcess(void)
{
	uint16 		result;
	uint8 		i;
	ble_msg_t 	bleMsg;
	uint32		lengthTemp;


	switch(appUploadState)
	{
		case UPLOAD_CATALOG:
		if (catalogBuffCnt >= SCENE_CATALOG_INFO_LENGTH)
		{
			App_SceneData_DataReUploadTimerStop();
			appUploadState = UPLOAD_IDLE;
			return 0;
		}
		//检查协议栈空闲状态
		if (BLE_Stack_CheckSendStatus() == 1)
		{
			//标记目录包号
			if (catalogLenTemp <= APP_SCENE_CATALOG_PACK_LEN)
			{
				catalogLenUploadLen = catalogLenTemp;
				catalogFlow 		= 0xff;
				catalogLenTemp 		= 0;
				App_SceneData_DataReUploadTimerStop();
				appUploadState = UPLOAD_IDLE;
			}
			else
			{
				catalogFlow 		+= 1;
				catalogLenUploadLen = APP_SCENE_CATALOG_PACK_LEN;
				catalogLenTemp 		-= APP_SCENE_CATALOG_PACK_LEN;
			}
			//防止越界
			if ((catalogBuffCnt + catalogLenUploadLen) > SCENE_CATALOG_INFO_LENGTH)
			{
				catalogLenUploadLen = SCENE_CATALOG_INFO_LENGTH -catalogBuffCnt;
				App_SceneData_DataReUploadTimerStop();
				appUploadState = UPLOAD_IDLE;
			}

			// 返回对应序号目录详细信息
			bleMsg.id 	  = BLE_MSG_SEND;
			bleMsg.packet = PROT_SCENE_TRANS_CATA_RET;
			bleMsg.packet.att.loadLength += 1; 			
			bleMsg.packet.att.load.content.parameter[0] = catalogFlow; //包序号
			bleMsg.packet.att.loadLength += catalogLenUploadLen;		//目录信息
			for (i = 1; i <= catalogLenUploadLen; i++)
			{
				bleMsg.packet.att.load.content.parameter[i] = catalogBuff[catalogBuffCnt + i - 1];
			}
			// SEGGER_RTT_printf(0,"catalogFlow: %d\n", catalogFlow);
			// SEGGER_RTT_printf(0,"catalogLenUploadLen: %d\n", catalogLenUploadLen);
			// SEGGER_RTT_printf(0,"catalogBuffCnt: %d\n", catalogBuffCnt);
			bleMsg.packet.att.flowControl 				= flowControlTemp;
		    Mid_Ble_SendMsg(&bleMsg);

		    //传输状态标识　
		    catalogBuffCnt  += catalogLenUploadLen;	//缓存计数后移
		}
		break;

		case UPLOAD_DATA_PACK:
		// 数据上传完毕
		if(appDataUnuploadLen == 0)
		{
			appUploadState 		= UPLOAD_IDLE;
			App_SceneData_DataReUploadTimerStop();
			return 0;
		}

		//检查协议栈空闲状态
		if (BLE_Stack_CheckSendStatus() == 1)
		{	 
			if(appDataUnuploadLen < APP_SCENE_UPLOAD_VALID_LEN)
			{
				lengthTemp	= appDataUnuploadLen;
			}
			else
			{
				lengthTemp	= APP_SCENE_UPLOAD_VALID_LEN;
			}

			bleMsg.id			= BLE_MSG_SEND;
			bleMsg.packet	= PROT_SCENE_TRANS_DATA_UPLOAD;

			bleMsg.packet.att.load.content.parameter[0]	= (uint8)(appDataCurUploadPackNum >> 8);
			bleMsg.packet.att.load.content.parameter[1]	= (uint8)(appDataCurUploadPackNum);
			bleMsg.packet.att.load.content.parameter[2]	= (uint8)(appDataCurUploadClassify >> 8);
			bleMsg.packet.att.load.content.parameter[3]	= (uint8)(appDataCurUploadClassify);

			memset((bleMsg.packet.att.load.content.parameter + 4),0xff,APP_SCENE_UPLOAD_MAX_LEN);
			result = Mid_SceneData_ClassifyDataRead((bleMsg.packet.att.load.content.parameter + 4), 
													appDatauploadTotalLen, lengthTemp, appDataCurCatalogUtc, dataClassifyTemp);

			// 无效数据
			if(result == 0xff)
			{
				appUploadState 		= UPLOAD_IDLE;
				// 返回无效数据包序号
				bleMsg.id			= BLE_MSG_SEND;
				bleMsg.packet	= PROT_SCENE_TRANS_DATA_RET;
				bleMsg.packet.att.load.content.parameter[0]	= 2;
		        Mid_Ble_SendMsg(&bleMsg);
				App_SceneData_DataReUploadTimerStop();
				return 0xff;
			}
			// 发送数据
		    Mid_Ble_SendMsg(&bleMsg);

			//包号递增、数据长度改变
			appDataCurUploadPackNum++;
			appDatauploadTotalLen += lengthTemp;
			appDataUnuploadLen	-= lengthTemp;
		}
		break;

		default:
		appUploadState 		= UPLOAD_IDLE;
		App_SceneData_DataReUploadTimerStop();
		break;

	}
	return 0x00;
}


//**********************************************************************
// 函数功能:	删除所有数据,并返回结果
// 输入参数：	
// 返回参数：	0x00: 操作成功
// 				0xff: 操作失败
//**********************************************************************
uint16 App_SceneData_DataTotalDelete(uint8 flowControl)
{
	uint16 result;
	ble_msg_t bleMsg;

	result = Mid_SceneData_DeleteTotalData();

	bleMsg.id			= BLE_MSG_SEND;
	bleMsg.packet	= PROT_SCENE_TRANS_DELETE_ALL_RET;
	if(result == 0)
	{
		bleMsg.packet.att.load.content.parameter[0]	= 0;
	}
	else
	{
		bleMsg.packet.att.load.content.parameter[0]	= 1;
	}
	bleMsg.packet.att.flowControl = flowControl;
    Mid_Ble_SendMsg(&bleMsg);
	
	return result;
}

//**********************************************************************
// 函数功能:	删除某类分类数据的所有数据,并返回结果
// 输入参数：	
// 返回参数：	0x00: 操作成功
// 				0xff: 操作失败
//**********************************************************************
uint16 App_SceneData_DataDeleteClassify(uint32 dataClassify, uint8 flowControl)
{
	uint16 result;
	ble_msg_t bleMsg;

	dataClassifyTemp = dataClassify - BASE_SCENE_DATA_CLASSIFY;

	result = Mid_SceneData_DeleteClassifyData(dataClassifyTemp);

	bleMsg.id			= BLE_MSG_SEND;
	bleMsg.packet	= PROT_SCENE_TRANS_DELETE_CLASS_RET;
	bleMsg.packet.att.load.content.parameter[0]	= (uint8)(dataClassify >> 8);
	bleMsg.packet.att.load.content.parameter[1]	= (uint8)(dataClassify);

	if(result == 0)
	{
		bleMsg.packet.att.load.content.parameter[2]	= 0;
	}
	else
	{
		bleMsg.packet.att.load.content.parameter[2]	= 1;
	}
	bleMsg.packet.att.flowControl		= flowControl;
    Mid_Ble_SendMsg(&bleMsg);
	return result;
}

//**********************************************************************
// 函数功能:	删除指定目录,并返回结果
// 输入参数：	
// 返回参数：	0x00: 操作成功
// 				0xff: 操作失败
//**********************************************************************
uint16 App_SceneData_DataDeleteCatalog(uint32 dataClassify, uint32 utc, uint8 flowControl)
{
	uint16 result;
	ble_msg_t bleMsg;

	dataClassifyTemp = dataClassify - BASE_SCENE_DATA_CLASSIFY;
	result = Mid_SceneData_DeleteClassifyDataUtc(dataClassifyTemp, utc);

	bleMsg.id			= BLE_MSG_SEND;
	bleMsg.packet	= PROT_SCENE_TRANS_DELETE_CATALOG_RET;
	bleMsg.packet.att.load.content.parameter[0]	= (uint8)(dataClassify >> 8);
	bleMsg.packet.att.load.content.parameter[1]	= (uint8)(dataClassify);
	bleMsg.packet.att.load.content.parameter[2]	= (uint8)(utc >> 24);
	bleMsg.packet.att.load.content.parameter[3]	= (uint8)(utc >> 16);
	bleMsg.packet.att.load.content.parameter[4]	= (uint8)(utc >> 8);
	bleMsg.packet.att.load.content.parameter[5]	= (uint8)(utc);

	if(result == 0)
	{
		bleMsg.packet.att.load.content.parameter[6]	= 0;
	}
	else
	{
		bleMsg.packet.att.load.content.parameter[6]	= 1;
	}
	bleMsg.packet.att.flowControl		= flowControl;
    Mid_Ble_SendMsg(&bleMsg);
    if (appSceneDataCacheCnt[dataClassifyTemp] > 0)
    {
    	appSceneDataCacheCnt[dataClassifyTemp] --;
    }
    
	return result;
}


///////////////////////////////////////////////////////////////////////////////////////////////
		/******************************** 开发测试******************************************/
//////////////////////////////////////////////////////////////////////////////////////////////


#define 	STEP_1_DAYS 		1
#define 	STEP_2_DAYS 		2
#define 	STEP_3_DAYS 		3
#define 	STEP_4_DAYS 		4

#define 	SLEEP_1_DAYS 		1
#define 	SLEEP_2_DAYS 		2
#define 	SLEEP_3_DAYS 		3
#define 	SLEEP_4_DAYS 		4

#define 	STEP_DAYS 			STEP_4_DAYS
#define 	SLEEP_DAYS 			SLEEP_4_DAYS

#define 	DATA_DAYS 			4
#define 	BASE_UTC 			1525168800


static uint32_t utc = BASE_UTC;//2018/5/20 0:0:0对应2018/5/21 0:0:0
static uint32_t test;

//**********************************************************************
// 函数功能:	制造假数据计步或睡眠(4天的数据)
// 输入参数：	
// 返回参数：
//**********************************************************************

void App_SceneData_DataSaveFakeData(void)
{
	uint16_t i, j;
	scene_data_s sceneDateTemp;

	sceneDateTemp.runScene.paceLog  = 0x15;
	sceneDateTemp.runScene.freqLog  = 0x16;
	sceneDateTemp.runScene.hrmLog 	= 0x42;
	
	if (test > 100)
	{
		utc			= BASE_UTC;
	}
    vTaskDelay(10);
	
	// 创建假计步数据
	for(i = 0; i < DATA_DAYS; i++)
	{
		//2h计步
		for(j = 0; j < 360; j++)
		{
			 App_SceneData_SaveDynamic(utc, RUN_SCENE_TYPE, &sceneDateTemp);
			utc	+=	10;
		}
		App_SceneData_SaveCache(RUN_SCENE_TYPE);
	}
	MultiModuleTask_EventSet(ACTIION_MOTO);
}


void App_SceneData_DataSavePrintf(uint16_t dataClassify)
{
	uint32_t 		i;
	uint16 			result;
	uint16_t 		catalogNumTotal,catalogIndex;
	uint32_t 		dataLenTotal;
	uint32_t 		utcPackNum,utcPackIndex;
	uint32_t 		lengthTemp,addrOffset,utcToltalLen;
	uint32_t 		curUtc;
	scene_catalogInfo_s  dataInfoprintf;
	uint8_t 		databuf[APP_SCENE_UPLOAD_MAX_LEN];
	uint32_t 		catalogStartAddr;
	uint32_t 		dataStartAddr;

	uint32 			dataClassifyPrintf;

	catalogNumTotal 			= 0;
	dataLenTotal 				= 0;
	addrOffset 					= 0;
	utcToltalLen 				= 0;
	curUtc 						= 0;
	lengthTemp 					= 0;
	catalogStartAddr 			= 0;
	dataStartAddr 				= 0;

	dataClassifyPrintf = dataClassify - BASE_SCENE_DATA_CLASSIFY;

	//获取总目录
	result = Mid_SceneClassifyTotalCatalogRead(&catalogNumTotal, &dataLenTotal, dataClassifyPrintf,&catalogStartAddr);
	SEGGER_RTT_printf(0,"TotalcatalogNum: 	%d\n", catalogNumTotal);
	SEGGER_RTT_printf(0,"TotaldataLen: 		%d\n", dataLenTotal);
	vTaskDelay(1);
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
		result 			  = Mid_SceneClassifyCatalogInfoRead(&dataInfoprintf, dataClassifyPrintf, catalogStartAddr);
		catalogStartAddr += SCENE_CATALOG_INFO_LENGTH;

		//请求指定目录的数据
		dataStartAddr 			= dataInfoprintf.startAddr;
		utcToltalLen 			= dataInfoprintf.dataLength;
		utcPackNum 				= utcToltalLen / APP_SCENE_UPLOAD_VALID_LEN; //包数据统计  /16
		if (utcToltalLen % APP_SCENE_UPLOAD_VALID_LEN)
		{
			utcPackNum ++;
		}
		curUtc 					= dataInfoprintf.utc;

		SEGGER_RTT_printf(0,"catalog:   %d\n", catalogIndex);
		SEGGER_RTT_printf(0,"totalLen:  %d\n", utcToltalLen);
		SEGGER_RTT_printf(0,"totalPack: %d\n", utcPackNum);
		SEGGER_RTT_printf(0,"data address: 	%x\n", dataStartAddr);
		vTaskDelay(1);
		//数据包遍历
		for(utcPackIndex = 0; utcPackIndex < utcPackNum; utcPackIndex++)
		{	
			addrOffset 		= utcPackIndex * APP_SCENE_UPLOAD_VALID_LEN;

			if(utcToltalLen< APP_SCENE_UPLOAD_VALID_LEN)
			{
				lengthTemp	= utcToltalLen;
			}
			else
			{
				lengthTemp	= APP_SCENE_UPLOAD_VALID_LEN;
			}
			memset(databuf,0xff,APP_SCENE_UPLOAD_MAX_LEN);
			//读取一包数据
			result = Mid_SceneClassifyDataPreRead(databuf, (dataStartAddr + addrOffset), lengthTemp,dataClassifyPrintf);

			//数据打印
			SEGGER_RTT_printf(0,"%8x %4x %4x ",curUtc,utcPackIndex,dataClassify);

			for (i = 0; i < APP_SCENE_UPLOAD_MAX_LEN; i++)
			{
				SEGGER_RTT_printf(0,"%4x",databuf[i]);												
			}
			SEGGER_RTT_printf(0,"\n");	
			vTaskDelay(5);
			utcToltalLen 	-= lengthTemp;	
		}
		vTaskDelay(15);
	}
}
















