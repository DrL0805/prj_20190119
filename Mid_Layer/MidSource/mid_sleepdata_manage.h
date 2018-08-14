#ifndef		MID_SLEEPDATA_MANAGE_H
#define		MID_SLEEPDATA_MANAGE_H



#if 1

// 外部flash存储空间分配
// 目录区域分配
#define		SLEEP_DM_APP_CATALOG_AREA_START_ADD				0x6A000
#define		SLEEP_DM_APP_CATALOG_AREA_LENGTH				0x4000

// 数据区域分配
#define		SLEEP_DM_APP_DATA_AREA_START_ADD				0x8200
#define		SLEEP_DM_APP_DATA_AREA_LENGTH					0x3000


// 数据最小片区长度，一次性写入的最大字节数据，超过会报错
#define		SLEEP_DM_APP_SECTOR_LENGTH					4096


// 数据分量总量
#define		SLEEP_DM_APP_DATA_CLASSIFY_NUM			1


// 目录数据需片区对齐，否则会出错
#define			SLEEP_CATALOG_INFO_LENGTH		32

//目录有效性标识
#define 		SLEEP_CATALOG_VALID_ID 			0xAA  	



// 数据分类
typedef enum
{
	SLEEP_DATA_CLASSIFY		= 0,
} sleep_data_classify_e;


//SLEEP SCENE 
typedef struct 
{
	uint32 	sleepInUtc;			//完整睡眠记录的入睡时间
	uint32 	wakeUpUtc;			//醒来时间
	uint32 	startAddr; 			//中间醒来记录存储在外部flash的开始地址
	uint32 	dataClassify;		//数据分类
	uint8 	wakeUpCnt;			//醒来次数
	uint8 	validId;			//目录有效性标识
	uint32  dataLength;			// 数据总长度
}sleep_scene_catalogInfo_s;


typedef union
{
	uint8						databuf[SLEEP_CATALOG_INFO_LENGTH];
	sleep_scene_catalogInfo_s	u;	
}sleep_scene_catalogInfo_u;


// 总数据缓存目录最大数量限制
#define			SLEEP_CATALOG_TOTAL_MAX_NUM		16



void Mid_SleepData_Init(void);
uint16 Mid_SleepData_ClassifyDataInit(uint32 dataClassify, uint32 catalogSize, uint32 dataSize);
uint16 Mid_SleepData_CreateCatalog(sleep_scene_catalogInfo_s *dataInfo);
uint16 Mid_SleepData_DataSaveEnd(uint32 dataClassify);
uint16 Mid_SleepData_ClassifyDataSave(uint8 *data, uint32 length, uint32 dataClassify);

uint16 Mid_SleepData_ClassifyDataInfoRead(uint16 *catalogTotal, uint32 *dataLength, uint32 dataClassify);
uint16 Mid_SleepData_ClassifyDataCatalogRead(sleep_scene_catalogInfo_s *dataInfo, uint32 dataClassify, uint16 catalogNum);
uint16 Mid_SleepData_ClassifyDataRead(uint8 *data, uint32 addr, uint32 length, uint32 dataUtc, uint32 dataClassify);
uint16 Mid_SleepData_ReadCatalogDataLen(uint32 *length, uint32 dataUtc, uint32 dataClassify);
uint16 Mid_SleepData_DeleteTotalData(void);
uint16 Mid_SleepData_DeleteClassifyData(uint32 dataClassify);
uint16 Mid_SleepData_DeleteClassifyDataUtc(uint32 dataClassify, uint32 utc);
uint16 Mid_SleepData_StorageDataRecover(void);

//开发测试用
uint16_t Mid_SleepClassifyTotalCatalogRead(uint16_t *catalogTotal, uint32_t *dataLength, uint32_t dataClassify,uint32_t *catalogvalidstartaddr);
uint16_t Mid_SleepClassifyCatalogInfoRead(sleep_scene_catalogInfo_s *dataInfo, uint32_t dataClassify,uint32_t catalogAddr);
uint16_t Mid_SleepClassifyDataPreRead(uint8_t *data, uint32_t addr, uint32_t length, uint32_t dataClassify);


#endif

#endif		// DATA_MANAGE_APP_H




