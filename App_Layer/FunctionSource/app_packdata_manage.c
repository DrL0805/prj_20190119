#include "platform_common.h"
/* FreeRTOS includes */
#include "rtos.h"
#include "mid_interface.h"
#include "app_variable.h"
#include "app_cachedata_manage.h"
#include "app_packdata_manage.h"
#include "flash_task.h" 
#include "ble_task.h"
#include "multimodule_task.h"



// 分类数据的目录数量与数据长度返回协议
static const protocal_msg_t	PROT_PACK_TRANS_CATA_TOTAL_RET =
{
	0x23, 0x01, 0x0B, 0x13, 0x00, 0x08, 0xf0, 0x10,
};

// 分类数据的目录内容返回协议
static const protocal_msg_t	PROT_PACK_TRANS_CATA_RET =
{
	0x23, 0x01, 0x0c, 0x13, 0x00, 0x08, 0xf0, 0x11,
};

// 分类数据的数据包请求结果反馈
static const protocal_msg_t	PROT_PACK_TRANS_DATA_RET =
{
	0x23, 0x01, 0x04, 0x13, 0x00, 0x08, 0xf0, 0x12,
};

// 删除所有数据反馈
static const protocal_msg_t	PROT_PACK_TRANS_DELETE_ALL_RET =
{
	0x23, 0x01, 0x04, 0x13, 0x00, 0x80, 0xf0, 0x30,
};

// 删除某类数据反馈
static const protocal_msg_t	PROT_PACK_TRANS_DELETE_CLASS_RET =
{
	0x23, 0x01, 0x06, 0x13, 0x00, 0x80, 0xf0, 0x31,
};

// 删除目录反馈
static const protocal_msg_t	PROT_PACK_TRANS_DELETE_CATALOG_RET =
{
	0x23, 0x01, 0x0a, 0x13, 0x00, 0x80, 0xf0, 0x32,
};

// 数据包上传协议
static const protocal_msg_t	PROT_PACK_TRANS_DATA_UPLOAD =
{
	0x23, 0x01, 0x17, 0x12, 0x80, 0x01, 0xf0, 0x01,
};


// 计步相关宏
#define			APP_STEP_NULLPACKET_MAX		12
#define			APP_STEP_PACKET_MAX			8
#define			APP_STEP_BYTE_LEN			2
#define			APP_STEP_BUFF_MAX			24
#define			APP_DATA_UPLOAD_MAX_LEN		16

#ifndef SLEEP_PACK_DUMMY
// 睡眠相关宏
#define			APP_SLEEP_PACKET_MAX		48
#define			APP_SLEEP_VALID_PACKET		12		// 默认有效数据包,不能大于APP_SLEEP_PACKET_MAX
#define			APP_SLEEP_BYTE_LEN			2
#endif

//心率相关宏
#define			APP_HEART_PACKET_MAX		16		//最大缓存数据
#define			APP_HEART_BYTE_LEN			1


#define	 		AppDataReUploadInternal 		10//100ms
#define	 		BLE_LINKINV_FAST_LIMMIT			100 //5 * 16大于　5包时

static uint16 appSaveInitFlag = 0;

// 计步相关变量
static uint16 appStepNullpacketCnt;			//空包计数
static uint16 appStepPacketCnt;				//包计数
static uint16 stepDataTemp[APP_STEP_PACKET_MAX];
static uint16 appStepSaveState;

#ifndef SLEEP_PACK_DUMMY
// 睡眠相关变量
static uint16 appSleepPacketCnt;
static uint16 appSleepVaildPacketLimit;			// 连续多少个睡眠数据做存储的限制条件,不能大于APP_SLEEP_PACKET_MAX
static uint32 appSleepStartUtc;					// 有效数据的开始时间
static uint16 appSleepSaveState;				// 当前该类数据的状态
static uint16 sleepDataTemp[APP_SLEEP_PACKET_MAX];
#endif 

//心率相关变量
static uint8 appHeartPacketCnt;
static uint8 appHeartSaveState;				// 当前该类数据的状态
static uint8 heartDataTemp[APP_HEART_PACKET_MAX];


// 数据上传相关变量
static uint32 appDataUnuploadLen;			//未上传数据长度
static uint32 appDatauploadTotalLen;		//该目录的总长度
static uint16 appDataCurUploadPackNum;		//当前上传的数据包序号
static uint16 appDataCurUploadClassify;		//当前上传的数据分类
static uint32 appDataCurCatalogUtc;			//当前上传的目录UTC


enum
{
	DATA_SAVE_IDLE		= 0,
	DATA_SAVE_SAVING,
};

static TimerHandle_t App_PackData_DataReUploadTimer		= NULL;				// 更新定时器定时器


//**********************************************************************
// 函数功能:	开启延时上传定时器
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
void App_PackData_DataReUploadTimerStart(void)
{
	if (xTimerIsTimerActive(App_PackData_DataReUploadTimer) == pdFALSE)
    {
		xTimerReset(App_PackData_DataReUploadTimer, 1);	
		xTimerStart(App_PackData_DataReUploadTimer, 1);
	}
}

 //**********************************************************************
// 函数功能:	关闭延时上传定时器
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
void App_PackData_DataReUploadTimerStop(void)
{
	if (xTimerIsTimerActive(App_PackData_DataReUploadTimer) != pdFALSE)
    {
		xTimerStop(App_PackData_DataReUploadTimer, 1);
	}
}


//**********************************************************************
// 函数功能:	清空计步数据缓存
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
static void ClearStepTemp(void)
{
	uint16 i;
	for(i = 0; i < APP_STEP_PACKET_MAX; i++)
		stepDataTemp[i]	= 0;
}

//**********************************************************************
// 函数功能:	清空心率数据缓存
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
static void ClearHeartTemp(void)
{
	uint16 i;
	for(i = 0; i < APP_HEART_PACKET_MAX; i++)
		heartDataTemp[i]	= 0;
}

//**********************************************************************
// 函数功能:	初始化计步睡眠数据存储
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
void App_PackData_DataSaveInit(void (*TimerCb)(TimerHandle_t xTimer))
{
	Mid_PackData_Init();
	Mid_PackData_ClassifyDataInit(STEPDATA_CLASSIFY, 3*DM_APP_SECTOR_LENGTH, 3 * DM_APP_SECTOR_LENGTH);
	#ifndef SLEEP_PACK_DUMMY
	Mid_PackData_ClassifyDataInit(SLEEPDATA_CLASSIFY, 3*DM_APP_SECTOR_LENGTH, 3* DM_APP_SECTOR_LENGTH);
	#endif
	Mid_PackData_ClassifyDataInit(HEARTDATA_CLASSIFY, 3*DM_APP_SECTOR_LENGTH, 5 * DM_APP_SECTOR_LENGTH);

	// step
	ClearStepTemp();
	appStepNullpacketCnt		= 0;
	appStepPacketCnt			= 0;
	appStepSaveState			= DATA_SAVE_IDLE;

	#ifndef SLEEP_PACK_DUMMY
	// sleep
	appSleepPacketCnt			= 0;
	appSleepVaildPacketLimit	= APP_SLEEP_VALID_PACKET;
	appSleepStartUtc			= 0;
	appSleepSaveState			= DATA_SAVE_IDLE;
	#endif

	//heart
	appHeartPacketCnt 			= 0;
	appHeartSaveState 			= DATA_SAVE_IDLE;

	if (appSaveInitFlag == 0)
	{
		App_PackData_DataReUploadTimer = xTimerCreate("reupload", (APP_1SEC_TICK / AppDataReUploadInternal), pdTRUE, 0, TimerCb);
		appSaveInitFlag 			= 1;
	}
}

//**********************************************************************
// 函数功能:	每整5分钟调用一次，自动判定是否需要存储
// 输入参数:	utc:	该5分钟数据的UTC时间
// 				step:	该5分钟内的计步数据。连续1小时（12小包）没有数据时不会进行存储。
// 返回参数:	无
//**********************************************************************
#ifndef SLEEP_PACK_DUMMY
void App_PackData_Step5MinSave(uint32 utc, uint16 step)
{
	catalogInfo_s		catalogInfoTemp;


	if(step == 0)
	{
		if(DATA_SAVE_SAVING == appStepSaveState)
		{
			stepDataTemp[appStepPacketCnt]	= (step >> 8) + (step << 8);

			appStepNullpacketCnt++;
			appStepPacketCnt++;
			// 空包数据达到可以不存的程度
			if(APP_STEP_NULLPACKET_MAX >=appStepNullpacketCnt)
			{
				// 将剩余的缓存数据存入
				if (appStepPacketCnt > appStepNullpacketCnt)
				{
					Mid_PackData_ClassifyDataSave((uint8_t*)stepDataTemp, APP_STEP_BYTE_LEN * (appStepPacketCnt - appStepNullpacketCnt), STEPDATA_CLASSIFY);
				}
				Mid_PackData_DataSaveEnd(STEPDATA_CLASSIFY);
				appStepSaveState			= DATA_SAVE_IDLE;
				appStepNullpacketCnt		= 0;
				appStepPacketCnt			= 0;
			}
		}
	}
	else
	{
		appStepNullpacketCnt		= 0;
		if(DATA_SAVE_IDLE == appStepSaveState)
		{
			catalogInfoTemp.utc				= utc;
			catalogInfoTemp.dataClassify	= STEPDATA_CLASSIFY;
			catalogInfoTemp.sampleUnit		= DATASAMPLE_UNIT_10S;
			catalogInfoTemp.sampleInterval	= 30;
			catalogInfoTemp.unitLength		= APP_STEP_BYTE_LEN;
			Mid_PackData_CreateCatalog(&catalogInfoTemp);

			appStepSaveState				= DATA_SAVE_SAVING;	
		}

		stepDataTemp[appStepPacketCnt]	= (step >> 8) + (step << 8);
		appStepPacketCnt++;

		// 达到存储的空包数
		if(appStepPacketCnt >= APP_STEP_PACKET_MAX)//值8
		{
			Mid_PackData_ClassifyDataSave((uint8_t*)stepDataTemp, APP_STEP_BYTE_LEN * appStepPacketCnt, STEPDATA_CLASSIFY);
			appStepPacketCnt			= 0;
		}
	}
}
#else
void App_PackData_Step5MinSave(uint32 utc, uint16 step)
{
	catalogInfo_s		catalogInfoTemp;

	if(DATA_SAVE_IDLE == appStepSaveState)
	{
		catalogInfoTemp.utc				= utc;
		catalogInfoTemp.dataClassify	= STEPDATA_CLASSIFY;
		catalogInfoTemp.sampleUnit		= DATASAMPLE_UNIT_10S;
		catalogInfoTemp.sampleInterval	= 30;
		catalogInfoTemp.unitLength		= APP_STEP_BYTE_LEN;
		Mid_PackData_CreateCatalog(&catalogInfoTemp);

		appStepSaveState				= DATA_SAVE_SAVING;	
	}

	//低位在前的存储格式
	stepDataTemp[appStepPacketCnt]	= (step >> 8) + (step << 8);
	appStepPacketCnt++;

	// 达到存储的包数
	if(appStepPacketCnt >= APP_STEP_PACKET_MAX)//值8
	{
		Mid_PackData_ClassifyDataSave((uint8_t*)stepDataTemp, APP_STEP_BYTE_LEN * appStepPacketCnt, STEPDATA_CLASSIFY);
		appStepPacketCnt			= 0;
	}
}
#endif

//**********************************************************************
// 函数功能:	将未存入的buf缓存数据,存入数据中，可在APP请求数据时调用。
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
#ifndef SLEEP_PACK_DUMMY
void App_PackData_StepSaveCache(void)
{
	if(DATA_SAVE_SAVING == appStepSaveState)
	{
		if(appStepPacketCnt > 0)
		{
			Mid_PackData_ClassifyDataSave((uint8_t*)stepDataTemp, APP_STEP_BYTE_LEN * appStepPacketCnt, STEPDATA_CLASSIFY);			
		}
		Mid_PackData_DataSaveEnd(STEPDATA_CLASSIFY);
		appStepPacketCnt			= 0;
		appStepNullpacketCnt		= 0;
		appSleepPacketCnt			= 0;//考虑到把空包也存入了计步包中，需把睡眠空清空，避免时间重叠
		appStepSaveState			= DATA_SAVE_IDLE;
	}
}
#else
void App_PackData_StepSaveCache(void)
{
	if(DATA_SAVE_SAVING == appStepSaveState)
	{
		if(appStepPacketCnt > 0)
		{
			Mid_PackData_ClassifyDataSave((uint8_t*)stepDataTemp, APP_STEP_BYTE_LEN * appStepPacketCnt, STEPDATA_CLASSIFY);			
		}
		Mid_PackData_DataSaveEnd(STEPDATA_CLASSIFY);
		appStepPacketCnt			= 0;
		appStepSaveState			= DATA_SAVE_IDLE;
	}
}
#endif

//**********************************************************************
// 函数功能:	设置最短存储的睡眠数据长度，每个长度为5分钟的睡眠数据动作值，当睡眠数据超过该长度
// 				才会放置至
// 输入参数:	minLength:	最小睡眠数据存储长度
// 返回参数:	无
//**********************************************************************
#ifndef SLEEP_PACK_DUMMY
uint16 App_PackData_SleepMinLengthSet(uint16 minLength)
{
	if(minLength > APP_SLEEP_PACKET_MAX)
		return 0xff;	
	appSleepVaildPacketLimit		= minLength;

	return 0;
}
#endif

//**********************************************************************
// 函数功能:	将未存入外部flash的睡眠缓存数据,存入flash中，可在APP请求数据时调用与产生步数时进行调用。
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
#ifndef SLEEP_PACK_DUMMY
void App_PackData_SleepSaveCache(void)
{
	if(DATA_SAVE_SAVING == appSleepSaveState)
	{
		if(appSleepPacketCnt > 0)
		{
			Mid_PackData_ClassifyDataSave((uint8_t*)sleepDataTemp, APP_SLEEP_BYTE_LEN * appSleepPacketCnt, SLEEPDATA_CLASSIFY);
		}
		Mid_PackData_DataSaveEnd(SLEEPDATA_CLASSIFY);

		appSleepPacketCnt		= 0;
		appSleepSaveState		= DATA_SAVE_IDLE;
	}
}
#endif

//**********************************************************************
// 函数功能:	强制清除睡眠缓存数据，有效存储的数据会想存入外部flash中。
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
#ifndef SLEEP_PACK_DUMMY
void App_PackData_SleepCacheClear(void)
{
	App_PackData_SleepSaveCache();
	appSleepPacketCnt		= 0;
}
#endif

//**********************************************************************
// 函数功能:	每整5分钟调用一次，自动判定是否需要存储
// 输入参数:	utc:	该5分钟数据的UTC时间
// 				step:	该5分钟内的计步数据,
// 				sleepMotion:	该5分钟的动作累积值，只进行连续的数据存储，数据长度可设置
// 返回参数:	无
//**********************************************************************
#ifndef SLEEP_PACK_DUMMY
void App_PackData_Sleep5MinSave(uint32 utc, uint16 step, uint16 sleepMotion)
{
	catalogInfo_s		catalogInfoTemp;
	
	if(step)
	{
		// 若有已经有目录创建，则将剩下的睡眠数据存入外部flash中
		App_PackData_SleepSaveCache();
		// 清空数据状态
		appSleepSaveState		= DATA_SAVE_IDLE;
		appSleepPacketCnt		= 0;
	}
	else
	{
		if(appSleepSaveState == DATA_SAVE_IDLE)
		{
			// 判断是否未创建目录且且缓存区未放置数据
			if(appSleepPacketCnt == 0)
			{
				appSleepStartUtc	= utc;
			}
			sleepDataTemp[appSleepPacketCnt]		= (sleepMotion >> 8) + (sleepMotion << 8);
			appSleepPacketCnt++;
			
			if(appSleepPacketCnt >= appSleepVaildPacketLimit)
			{
				catalogInfoTemp.utc				= appSleepStartUtc;
				catalogInfoTemp.dataClassify	= SLEEPDATA_CLASSIFY;
				catalogInfoTemp.sampleUnit		= DATASAMPLE_UNIT_10S;
				catalogInfoTemp.sampleInterval	= 30;
				catalogInfoTemp.unitLength		= APP_SLEEP_BYTE_LEN;
				Mid_PackData_CreateCatalog(&catalogInfoTemp);

				appSleepSaveState				= DATA_SAVE_SAVING;	

				Mid_PackData_ClassifyDataSave((uint8*)sleepDataTemp, APP_SLEEP_BYTE_LEN * appSleepPacketCnt, SLEEPDATA_CLASSIFY);
				appSleepPacketCnt		= 0;
			}
		}
		else
		{
			sleepDataTemp[appSleepPacketCnt]		= (sleepMotion >> 8) + (sleepMotion << 8);
			appSleepPacketCnt++;
			
			if(appSleepPacketCnt >= appSleepVaildPacketLimit)
			{
				Mid_PackData_ClassifyDataSave((uint8*)sleepDataTemp, APP_SLEEP_BYTE_LEN * appSleepPacketCnt, SLEEPDATA_CLASSIFY);
				appSleepPacketCnt		= 0;
			}
		}
	}
}
#endif

//**********************************************************************
// 函数功能:	将未存入外部flash的缓存数据,存入flash中，可在APP请求数据时调用与产生步数时进行调用。
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
void App_PackData_HrmSaveCache(void)
{
	if(DATA_SAVE_SAVING == appHeartSaveState)
	{
		if(appHeartPacketCnt > 0)
		{
			Mid_PackData_ClassifyDataSave((uint8_t*)heartDataTemp, APP_HEART_BYTE_LEN * appHeartPacketCnt, HEARTDATA_CLASSIFY);
		}
		Mid_PackData_DataSaveEnd(HEARTDATA_CLASSIFY);
		appHeartPacketCnt		= 0;
		appHeartSaveState		= DATA_SAVE_IDLE;
	}
}

//**********************************************************************
// 函数功能:	每整1分钟调用一次，自动判定是否需要存储
// 输入参数:	utc:	该1分钟数据的UTC时间
// 				hrmval:	该1分钟内的心率数据,
// 返回参数:	无
//**********************************************************************
void App_PackData_HrmSave(uint32 utc,uint8 hrmval)
{
	catalogInfo_s		catalogInfoTemp;

	if(DATA_SAVE_IDLE == appHeartSaveState)
	{
		catalogInfoTemp.utc				= utc;
		catalogInfoTemp.dataClassify	= HEARTDATA_CLASSIFY;
		catalogInfoTemp.sampleUnit		= DATASAMPLE_UNIT_10S;
		catalogInfoTemp.sampleInterval	= 12;
		catalogInfoTemp.unitLength		= APP_HEART_BYTE_LEN;
		Mid_PackData_CreateCatalog(&catalogInfoTemp);

		appHeartSaveState				= DATA_SAVE_SAVING;	
	}

	heartDataTemp[appHeartPacketCnt]	    = hrmval;
	appHeartPacketCnt++;

	// 达到存储的空包数
	if(appHeartPacketCnt >= APP_HEART_PACKET_MAX)//值10
	{
		Mid_PackData_ClassifyDataSave((uint8_t*)heartDataTemp, APP_HEART_BYTE_LEN * appHeartPacketCnt, HEARTDATA_CLASSIFY);
		appHeartPacketCnt			= 0;
	}
}

//**********************************************************************
// 函数功能:	掉电后数据恢复，需先初始化数据分配部分才可进行
// 输入参数：	
// 返回参数：
//**********************************************************************
void App_PackData_DataSavePowerdownRecover(void)
{
	Mid_PackData_StorageDataRecover();
}


//**********************************************************************
// 函数功能:	历史数据存储，校准完成后对历史数据进行一次存储
// 输入参数：	dataClassify：数据类型
// 返回参数：
//**********************************************************************
void App_PackData_HistoryDataSave(void)
{
	App_PackData_StepSaveCache();
	#ifndef SLEEP_PACK_DUMMY
	App_PackData_SleepCacheClear();//无效的目录丢弃
	#endif
	App_PackData_HrmSaveCache();
}


//**********************************************************************
// 函数功能:	返回请求的数据分类总数据长度与数据目录数量，
// 输入参数：	
// 返回参数：	0x00: 操作成功
// 				0xff: 操作失败
//**********************************************************************
uint16 App_PackData_DataRequestTotalInfoAck(uint32 dataClassify, uint8 flowControl)
{
	uint16 catalogNum, result;
	uint32 byteLen;
	ble_msg_t bleMsg;

	// 将缓存存入flash
	switch(dataClassify)
	{
		case STEPDATA_CLASSIFY:
		App_PackData_StepSaveCache();
		break;
		
		#ifndef SLEEP_PACK_DUMMY
		case SLEEPDATA_CLASSIFY:
		App_PackData_SleepSaveCache();
		break;
		#endif

		case HEARTDATA_CLASSIFY:
		App_PackData_HrmSaveCache();
		break;
	}
	//重新开始请求，把之前的数据上传关停
	App_PackData_DataReUploadTimerStop();

	//根据数据类型获取总目录信息、目录条数、数据总长度
	result = Mid_PackData_ClassifyDataInfoRead(&catalogNum, &byteLen, dataClassify);
	
	if(result == 0xff)
	{
		//修改蓝牙连接间隔变慢(不作改变处理)
		// if (BLE_LINKINV_SLOW != bleLinkinv.StateRead() && BLE_LINKINV_FAST_TO_SLOW != bleLinkinv.StateRead())
		// {
		// 	bleLinkinv.SwitchSlow();
		// }
		
		return result;
	}
	else
	{
		#if 0
		//修改蓝牙连接间隔变快,大于10包时
		if ((BLE_LINKINV_SLOW == Mid_BleLinkInv_StateRead()) && (byteLen >= BLE_LINKINV_FAST_LIMMIT))
		{
			Mid_BleLinkInv_SwitchToFast(BLE_LINKINV_SWITCH_CONNECT);
		}
		// else(不作改变处理）
		// {
		// 	if ((catalogNum == 0) || (byteLen == 0))
		// 	{
		// 		//修改蓝牙连接间隔变慢
		// 		bleLinkinv.SwitchSlow();
		// 	}
		// }
		#endif

		// 返回长度与目录数量
		bleMsg.id = BLE_MSG_SEND;
		bleMsg.packet= PROT_PACK_TRANS_CATA_TOTAL_RET;
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
uint16 App_PackData_DataCatalogInfoRead(uint32 dataClassify, uint16 catalogNum, uint8 flowControl)
{
	uint16 result;
	ble_msg_t bleMsg;
	catalogInfo_s dataInfo;

	result = Mid_PackData_ClassifyDataCatalogRead(&dataInfo, dataClassify, catalogNum);

	if(result == 0xff)
	{
		return 0xff;
	}
	// 返回对应序号目录详细信息
	bleMsg.id = BLE_MSG_SEND;
	bleMsg.packet = PROT_PACK_TRANS_CATA_RET;
	bleMsg.packet.att.load.content.parameter[0]	= (uint8)(dataInfo.utc >> 24);
	bleMsg.packet.att.load.content.parameter[1]	= (uint8)(dataInfo.utc >> 16);
	bleMsg.packet.att.load.content.parameter[2]	= (uint8)(dataInfo.utc >> 8);
	bleMsg.packet.att.load.content.parameter[3]	= (uint8)(dataInfo.utc);
	bleMsg.packet.att.load.content.parameter[4]	= (uint8)(dataInfo.dataLength >> 8);
	bleMsg.packet.att.load.content.parameter[5]	= (uint8)(dataInfo.dataLength);
	bleMsg.packet.att.load.content.parameter[6]	= (uint8)((dataInfo.sampleUnit << 4) | (dataInfo.sampleInterval >> 12));
	bleMsg.packet.att.load.content.parameter[7]	= (uint8)(dataInfo.sampleInterval);
	bleMsg.packet.att.load.content.parameter[8]	= (uint8)(dataInfo.unitLength);
	bleMsg.packet.att.flowControl = flowControl;
    Mid_Ble_SendMsg(&bleMsg);
	return 0;
}

//**********************************************************************
// 函数功能:	请求数据包，查询数据是否有效并反馈给APP，并进入发送数据的机制
// 输入参数：	
// 返回参数：	0x00: 操作成功
// 				0xff: 操作失败
//**********************************************************************
uint16 App_PackData_DataRequestData(uint32 dataClassify, uint32 utc, uint16 packNum, uint8 flowControl)
{
	uint32 lengthTemp;
	uint16 result;
	ble_msg_t bleMsg;


	//获取指定utc目录索引的数据段长度
	result = Mid_PackData_ReadCatalogDataLen(&appDataUnuploadLen, utc, dataClassify);

	if(result == 0xff)
	{
		// 返回无效数据类型
		bleMsg.id			= BLE_MSG_SEND;
		bleMsg.packet	= PROT_PACK_TRANS_DATA_RET;

		bleMsg.packet.att.load.content.parameter[0]	= 1;
		bleMsg.packet.att.flowControl				= flowControl;
        Mid_Ble_SendMsg(&bleMsg);
		return 0xff;
	}

	// 判断获取的数据包是否有效
	appDataCurUploadPackNum	= packNum;
	appDatauploadTotalLen		= packNum * APP_DATA_UPLOAD_MAX_LEN;
	appDataCurUploadClassify	= dataClassify;
	appDataCurCatalogUtc		= utc;
	// 序号超过数据长度
	if(appDatauploadTotalLen >= appDataUnuploadLen)
	{
		// 返回无效数据包序号
		bleMsg.id			= BLE_MSG_SEND;
		bleMsg.packet	= PROT_PACK_TRANS_DATA_RET;

		bleMsg.packet.att.load.content.parameter[0]	= 2;
		bleMsg.packet.att.flowControl				= flowControl;
        Mid_Ble_SendMsg(&bleMsg);

		App_PackData_DataReUploadTimerStop();
		return 0xff;
	}

	//返回请求结果（成功）
	bleMsg.id			= BLE_MSG_SEND;
	bleMsg.packet	= PROT_PACK_TRANS_DATA_RET;

	bleMsg.packet.att.load.content.parameter[0]	= 0;
	bleMsg.packet.att.flowControl				= flowControl;
    Mid_Ble_SendMsg(&bleMsg);
	
	appDataUnuploadLen	= appDataUnuploadLen - appDatauploadTotalLen;

	if(appDataUnuploadLen < APP_DATA_UPLOAD_MAX_LEN)
	{
		lengthTemp	= appDataUnuploadLen;
	}
	else
	{
		lengthTemp	= APP_DATA_UPLOAD_MAX_LEN;
	}

	bleMsg.id			= BLE_MSG_SEND;
	bleMsg.packet	= PROT_PACK_TRANS_DATA_UPLOAD;

	bleMsg.packet.att.load.content.parameter[0]	= (uint8)(appDataCurUploadPackNum >> 8);
	bleMsg.packet.att.load.content.parameter[1]	= (uint8)(appDataCurUploadPackNum);
	bleMsg.packet.att.load.content.parameter[2]	= (uint8)(appDataCurUploadClassify >> 8);
	bleMsg.packet.att.load.content.parameter[3]	= (uint8)(appDataCurUploadClassify);

	memset((bleMsg.packet.att.load.content.parameter + 4),0xff,APP_DATA_UPLOAD_MAX_LEN);
	result = Mid_PackData_ClassifyDataRead((bleMsg.packet.att.load.content.parameter + 4), 
											appDatauploadTotalLen, lengthTemp, appDataCurCatalogUtc, appDataCurUploadClassify);

	// 无效数据
	if(result == 0xff)
	{
		// 返回无效数据包序号
		bleMsg.id			= BLE_MSG_SEND;
		bleMsg.packet	= PROT_PACK_TRANS_DATA_RET;

		bleMsg.packet.att.load.content.parameter[0]	= 2;
		bleMsg.packet.att.flowControl				= flowControl;
        Mid_Ble_SendMsg(&bleMsg);

		App_PackData_DataReUploadTimerStop();
		return 0xff;
	}

	// 发送数据
	bleMsg.packet.att.flowControl				= flowControl;
    Mid_Ble_SendMsg(&bleMsg);

    //启动分包数据定时发送
    App_PackData_DataReUploadTimerStart();
	appDataCurUploadPackNum++;
	appDatauploadTotalLen += lengthTemp;
	appDataUnuploadLen	-= lengthTemp;

	return 0x00;
}

//**********************************************************************
// 函数功能:	数据上传处理，当MCU接收到BLE数据上传送达成功时调用该函数(指定数据包后，从指定包号开始上传)
// 输入参数：	
// 返回参数：	0x00: 操作成功
// 				0xff: 操作失败
//**********************************************************************
uint16 App_PackData_DataUploadProcess(void)
{
	uint16 result;
	ble_msg_t bleMsg;
	uint32 lengthTemp;


	// 数据上传完毕
	if(appDataUnuploadLen == 0)
	{
		App_PackData_DataReUploadTimerStop();
		return 0;
	}

	//检查协议栈空闲状态
	if (BLE_Stack_CheckSendStatus() == 1)
	{
	// SEGGER_RTT_printf(0,"App_PackData_DataUploadProcess\n");		 
		if(appDataUnuploadLen < APP_DATA_UPLOAD_MAX_LEN)
		{
			lengthTemp	= appDataUnuploadLen;
		}
		else
		{
			lengthTemp	= APP_DATA_UPLOAD_MAX_LEN;
		}

		bleMsg.id			= BLE_MSG_SEND;
		bleMsg.packet	= PROT_PACK_TRANS_DATA_UPLOAD;

		bleMsg.packet.att.load.content.parameter[0]	= (uint8)(appDataCurUploadPackNum >> 8);
		bleMsg.packet.att.load.content.parameter[1]	= (uint8)(appDataCurUploadPackNum);
		bleMsg.packet.att.load.content.parameter[2]	= (uint8)(appDataCurUploadClassify >> 8);
		bleMsg.packet.att.load.content.parameter[3]	= (uint8)(appDataCurUploadClassify);

		memset((bleMsg.packet.att.load.content.parameter + 4),0xff,APP_DATA_UPLOAD_MAX_LEN);
		result = Mid_PackData_ClassifyDataRead((bleMsg.packet.att.load.content.parameter + 4), 
												appDatauploadTotalLen, lengthTemp, appDataCurCatalogUtc, appDataCurUploadClassify);

		// 无效数据
		if(result == 0xff)
		{
			// 返回无效数据包序号
			bleMsg.id			= BLE_MSG_SEND;
			bleMsg.packet	= PROT_PACK_TRANS_DATA_RET;
			bleMsg.packet.att.load.content.parameter[0]	= 2;
	        Mid_Ble_SendMsg(&bleMsg);
			App_PackData_DataReUploadTimerStop();
			return 0xff;
		}
		// 发送数据
	    Mid_Ble_SendMsg(&bleMsg);

		//包号递增、数据长度改变
		appDataCurUploadPackNum++;
		appDatauploadTotalLen += lengthTemp;
		appDataUnuploadLen	-= lengthTemp;
	}

	return 0x00;
}


//**********************************************************************
// 函数功能:	删除所有数据,并返回结果
// 输入参数：	
// 返回参数：	0x00: 操作成功
// 				0xff: 操作失败
//**********************************************************************
uint16 App_PackData_DataTotalDelete(uint8 flowControl)
{
	uint16 result;
	ble_msg_t bleMsg;

	result = Mid_PackData_DeleteTotalData();

	bleMsg.id			= BLE_MSG_SEND;
	bleMsg.packet	= PROT_PACK_TRANS_DELETE_ALL_RET;
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
uint16 App_PackData_DataDeleteClassify(uint32 dataClassify, uint8 flowControl)
{
	uint16 result;
	ble_msg_t bleMsg;

	result = Mid_PackData_DeleteClassifyData(dataClassify);

	bleMsg.id			= BLE_MSG_SEND;
	bleMsg.packet	= PROT_PACK_TRANS_DELETE_CLASS_RET;
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
uint16 App_PackData_DataDeleteCatalog(uint32 dataClassify, uint32 utc, uint8 flowControl)
{
	uint16 result;
	ble_msg_t bleMsg;

	result = Mid_PackData_DeleteClassifyDataUtc(dataClassify, utc);

	bleMsg.id			= BLE_MSG_SEND;
	bleMsg.packet	= PROT_PACK_TRANS_DELETE_CATALOG_RET;
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


static uint32_t utc = BASE_UTC;//2018/5/1  18:0:0对应2018/5/1 10:00 
static uint32_t utchr = BASE_UTC;
static uint32_t test = 0;

//**********************************************************************
// 函数功能:	制造假数据计步或睡眠(4天的数据)
// 输入参数：	
// 返回参数：
//**********************************************************************
#ifndef SLEEP_PACK_DUMMY
void App_PackData_DataSaveFakeData(void)
{
	uint16_t i, j;
	
	if (test > 100)
	{
		utc			= BASE_UTC;
		test = 0;
	}
	test ++;
    vTaskDelay(10);
	
	// 创建假计步数据
	for(i = 0; i < DATA_DAYS; i++)
	{
		//2h计步
		for(j = 0; j < 24; j++)
		{
			App_PackData_Step5MinSave(utc, i+1);
			App_PackData_Sleep5MinSave(utc, (i+1)*10, i*10);
			utc	+=	300;
		}
		
		//10小时非计步
		for(j = 0; j < 120; j++)
		{
			App_PackData_Step5MinSave(utc, 0);
			App_PackData_Sleep5MinSave(utc, 0, i*10);
			utc	+=	300;
		}

		//2Ｈ计步
		for(j = 0; j < 24; j++)
		{
			App_PackData_Step5MinSave(utc, i+1);
			App_PackData_Sleep5MinSave(utc, (i+1)*10, i*10);
			utc	+=	300;
		}

		//6H非计步
		for(j = 0; j < 72; j++)
		{
			App_PackData_Step5MinSave(utc, 0);
			App_PackData_Sleep5MinSave(utc, 0, i*10);
			utc	+=	300;
		}

		//2Ｈ计步
		for(j = 0; j < 24; j++)
		{
			App_PackData_Step5MinSave(utc, i+1);
			App_PackData_Sleep5MinSave(utc, (i+1)*10, i*10);
			utc	+=	300;
		}

		//2H非计步
		for(j = 0; j < 24; j++)
		{
			App_PackData_Step5MinSave(utc, 0);
			App_PackData_Sleep5MinSave(utc, 0, i*10);
			utc	+=	300;
		}
	}
	// MultiModuleTask_EventSet(ACTIION_MOTO);
}
#else
void App_PackData_DataSaveFakeData(void)
{
	uint16_t i, j;
	uint32    utcTemp = 0;
	
	if (test > 100)
	{
		utc			= BASE_UTC;
		test = 0;
	}
	test ++;
    vTaskDelay(10);
	
	// 创建假计步数据
	for(i = 0; i < DATA_DAYS; i++)
	{
		//2h计步
		for(j = 0; j < 24; j++)
		{
			App_PackData_Step5MinSave(utc, i+1);
			utc	+=	300;
			utcTemp += 300;
		}
		
		//2小时非计步
		for(j = 0; j < 24; j++)
		{
			App_PackData_Step5MinSave(utc, 0);
			utc	+=	300;
			utcTemp += 300;
		}

		if (utcTemp % 1440 == 0)
		{
			App_PackData_StepSaveCache();
		}
	}
	// MultiModuleTask_EventSet(ACTIION_MOTO);
	vTaskDelay(10);
}
#endif

//**********************************************************************
// 函数功能:	制造假数据心率
// 输入参数：	
// 返回参数：
//**********************************************************************
void App_PackData_DataSaveFakeDataHeart(void)
{
	uint16_t i, j;
	uint8    hrmTemp1 = 75;
	uint8    hrmTemp2 = 80;
	uint32    utcTemp = 0;
	
	utchr			= BASE_UTC;

    vTaskDelay(10);
	
	// 创建假计步数据
	for(i = 0; i < DATA_DAYS; i++)
	{
		//6h
		for(j = 0; j < 360; j++)
		{
			App_PackData_HrmSave(utchr, hrmTemp1);
			utchr	+=	60;
			utcTemp += 60;
		}
		
		//6h
		for(j = 0; j < 360; j++)
		{
			App_PackData_HrmSave(utchr, hrmTemp2);
			utchr	+=	60;
			utcTemp += 60;
		}

		if (utcTemp % 1440 == 0)
		{
			App_PackData_HrmSaveCache();
		}
	}
	MultiModuleTask_EventSet(ACTIION_MOTO);
	vTaskDelay(10);
}

// 函数功能:	存储数据打印（在APP获取前打印出来）
// 输入参数：	
// 返回参数：

void App_PackData_DataSavePrintf(uint16_t dataClassify)
{
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

	switch(dataClassify)
	{
		case STEPDATA_CLASSIFY:
		SEGGER_RTT_printf(0,"STEPDATA_CLASSIFY\n");
		break;

		case SLEEPDATA_CLASSIFY:
		SEGGER_RTT_printf(0,"SLEEPDATA_CLASSIFY\n");
		break;

		case HEARTDATA_CLASSIFY:
		SEGGER_RTT_printf(0,"HEARTDATA_CLASSIFY\n");
		break;

		default:
		break;
	}

	//获取总目录
	result = ClassifyTotalCatalogRead(&catalogNumTotal, &dataLenTotal, dataClassify,&catalogStartAddr);
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
		SEGGER_RTT_printf(0,"catalog address: 	%x\n", catalogStartAddr);
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
		SEGGER_RTT_printf(0,"data address: 	%x\n", dataStartAddr);
		vTaskDelay(1);
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
			vTaskDelay(1);
			for (i = 0; i < APP_DATA_UPLOAD_MAX_LEN / 2; i++)
			{
				SEGGER_RTT_printf(0,"%4x%4x",databuf[2*i],databuf[2*i+1]);												
			}
			SEGGER_RTT_printf(0,"\n");	
			vTaskDelay(5);
			utcToltalLen 	-= lengthTemp;	
		}
		vTaskDelay(20);
	}
}

