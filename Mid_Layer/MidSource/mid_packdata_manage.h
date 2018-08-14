#ifndef		MID_PACKDATA_MANAGE_H
#define		MID_PACKDATA_MANAGE_H

// 外部flash存储空间分配
// 目录区域分配
#define		DM_APP_CATALOG_AREA_START_ADD			0x14000
#define		DM_APP_CATALOG_AREA_LENGTH				0x10000

// 数据区域分配
#define		DM_APP_DATA_AREA_START_ADD				0x24000
#define		DM_APP_DATA_AREA_LENGTH					0x3C000


// 数据最小片区长度，一次性写入的最大字节数据，超过会报错
#define		DM_APP_SECTOR_LENGTH					4096


// 数据分量总量
#define		DM_APP_DATA_CLASSIFY_NUM			3


// 目录数据需片区对齐，否则会出错
#define			CATALOG_INFO_LENGTH		32


// 数据分类
typedef enum
{
	STEPDATA_CLASSIFY		= 0,
	SLEEPDATA_CLASSIFY,
	HEARTDATA_CLASSIFY,
} data_classify_e;


// 采样单位
typedef enum
{
	DATASAMPLE_UNIT_1US		= 1,
	DATASAMPLE_UNIT_1MS,
	DATASAMPLE_UNIT_1S,
	DATASAMPLE_UNIT_10US,
	DATASAMPLE_UNIT_10MS,
	DATASAMPLE_UNIT_10S,
}data_sample_uint_e;


// 目录信息
typedef struct
{
	uint32		utc;				// 运动数据开始时间
	uint32		startAddr;			// 存储在外部flash开始地址
	uint32		dataLength;			// 运动数据总长度
	uint32		dataClassify;		// 数据分类
	uint16		sampleUnit;			// 数据采样单位
	uint16		sampleInterval;		// 数据采样间隔
	uint8			unitLength;			// 数据最小数据单位长度
}catalogInfo_s;

typedef union
{
	uint8			databuf[CATALOG_INFO_LENGTH];
	catalogInfo_s	u;	
}catalogInfo_u;


// 数据最长存储量，超过需另起目录
#define			DATA_MAX_LENGTH			1440 	//对应24小时（24 * 60 / 5 * 2），避免单条目录索引的数据过长，数据上传失败冗余大


// 总数据缓存目录最大数量限制
#define			CATALOG_TOTAL_MAX_NUM	16



void   Mid_PackData_Init(void);
uint16 Mid_PackData_ClassifyDataInit(uint32 dataClassify, uint32 catalogSize, uint32 dataSize);
uint16 Mid_PackData_CreateCatalog(catalogInfo_s *dataInfo);
uint16 Mid_PackData_DataSaveEnd(uint32 dataClassify);
uint16 Mid_PackData_ClassifyDataSave(uint8 *data, uint32 length, uint32 dataClassify);
uint16 Mid_PackData_ClassifyDataInfoRead(uint16 *catalogTotal, uint32 *dataLength, uint32 dataClassify);
uint16 Mid_PackData_ClassifyDataCatalogRead(catalogInfo_s *dataInfo, uint32 dataClassify, uint16 catalogNum);
uint16 Mid_PackData_ClassifyDataRead(uint8 *data, uint32 addr, uint32 length, uint32 dataUtc, uint32 dataClassify);
uint16 Mid_PackData_ReadCatalogDataLen(uint32 *length, uint32 dataUtc, uint32 dataClassify);
uint16 Mid_PackData_DeleteTotalData(void);
uint16 Mid_PackData_DeleteClassifyData(uint32 dataClassify);
uint16 Mid_PackData_DeleteClassifyDataUtc(uint32 dataClassify, uint32 utc);
uint16 Mid_PackData_StorageDataRecover(void);
uint16 Mid_PackData_ReadInfo(uint32 dataClassify, uint8 **dataPoint, uint16 *dataLength);



uint16_t ClassifyTotalCatalogRead(uint16_t *catalogTotal, uint32_t *dataLength, uint32_t dataClassify,uint32_t *catalogvalidstartaddr);

uint16_t ClassifyCatalogInfoRead(catalogInfo_s *dataInfo, uint32_t dataClassify,uint32_t catalogAddr);

uint16_t  ClassifyDataPreRead(uint8_t *data, uint32_t addr, uint32_t length, uint32_t dataClassify);
#endif		// DATA_MANAGE_APP_H
