#ifndef		MID_SCENEDATA_MANAGE_H
#define		MID_SCENEDATA_MANAGE_H




// 外部flash存储空间分配
// 目录区域分配
#define		SCENE_DM_APP_CATALOG_AREA_START_ADD				0x6E000
#define		SCENE_DM_APP_CATALOG_AREA_LENGTH				0x14000

// 数据区域分配
#define		SCENE_DM_APP_DATA_AREA_START_ADD				0x85000
#define		SCENE_DM_APP_DATA_AREA_LENGTH					0x21000


// 数据最小片区长度，一次性写入的最大字节数据，超过会报错
#define		SCENE_DM_APP_SECTOR_LENGTH						4096


// 数据分量总量
#define		SCENE_DM_APP_DATA_CLASSIFY_NUM					3


// 目录数据需片区对齐，否则会出错
#define			SCENE_CATALOG_INFO_LENGTH					64//基础部分+详细内容部分

#define 		RUN_SCENE_CATALOG_INFO_LENGTH 				55
#define 		CLIMB_SCENE_CATALOG_INFO_LENGTH 			57
#define 		SWING_SCENE_CATALOG_INFO_LENGTH 			51

// 数据最长存储量，超过需另起目录（暂定5小时）
#define			SCENE_DATA_MAX_LENGTH						9000 	


// 总数据缓存目录最大数量限制
#define			SCENE_CATALOG_TOTAL_MAX_NUM					10





//存储状态枚举
typedef enum 
{
	STORAGE_SUCCESS 	= 0x0000,		//存储/操作成功
	STORAGE_FULL		= 0x0001,		//存储区域满，无法再存储数据
	STORAGE_OVERFLOW 	= 0x0002,		//存储长度过长，需重建目录
	STORAGE_NORMAL 		= 0x0003,	
	STORAGE_FAIL 		= 0x00ff,		//存储/操作失败
}storage_result_e;



// 场景数据分类，应与场景类型一致
typedef enum
{	
	RUN_SCENE_DATA_CLASSIFY 		= 0,
	CLIMB_SCENE_DATA_CLASSIFY		= 1,
	SWING_SCENE_DATA_CLASSIFY 		= 2,
	BASE_SCENE_DATA_CLASSIFY		= 3,
} scene_data_classify_e;


// 采样单位
typedef enum
{
	SCENE_DATASAMPLE_UNIT_1US		= 1,
	SCENE_DATASAMPLE_UNIT_1MS,
	SCENE_DATASAMPLE_UNIT_1S,
	SCENE_DATASAMPLE_UNIT_10US,
	SCENE_DATASAMPLE_UNIT_10MS,
	SCENE_DATASAMPLE_UNIT_10S,
}scene_data_sample_uint_e;
	

// 目录信息
//基础：19bytes
//run: 36bytes
//climb: 38bytes
//swing: 32bytes
typedef struct
{
	uint32			utc;				// 数据开始时间
	uint32			startAddr;			// 存储在外部flash开始地址
	uint32			dataLength;			// 数据总长度
	uint16			dataClassify;		// 数据分类
	uint16			sampleUnit;			// 数据采样单位
	uint16			sampleInterval;		// 数据采样间隔
	uint8			unitLength;			// 数据最小数据单位长度
	scene_detail_s  detail;				// 场景信息，各场景有差异
}scene_catalogInfo_s;

typedef union
{
	uint8				databuf[SCENE_CATALOG_INFO_LENGTH];
	scene_catalogInfo_s	u;	
}scene_catalogInfo_u;





void Mid_SceneData_Init(void);
uint16 Mid_SceneData_ClassifyDataInit(uint32 dataClassify, uint32 catalogSize, uint32 dataSize);
uint16 Mid_SceneData_CreateCatalog(scene_catalogInfo_s *dataInfo);
uint16 Mid_SceneData_DataSaveEnd(uint32 dataClassify,scene_detail_s *sceneDetail);
uint16 Mid_SceneData_ClassifyDataSave(uint8 *data, uint32 length, uint32 dataClassify);
uint16 Mid_SceneData_ClassifyDataInfoRead(uint16 *catalogTotal, uint32 *dataLength, uint32 dataClassify);
uint16 Mid_SceneData_ClassifyDataCatalogRead(scene_catalogInfo_s *dataInfo, uint32 dataClassify, uint16 catalogNum);
uint16 Mid_SceneData_ClassifyDataRead(uint8 *data, uint32 addr, uint32 length, uint32 dataUtc, uint32 dataClassify);
uint16 Mid_SceneData_ReadCatalogDataLen(uint32 *length, uint32 dataUtc, uint32 dataClassify);
uint16 Mid_SceneData_DeleteTotalData(void);
uint16 Mid_SceneData_DeleteClassifyData(uint32 dataClassify);
uint16 Mid_SceneData_DeleteClassifyDataUtc(uint32 dataClassify, uint32 utc);
uint16 Mid_SceneData_StorageDataRecover(void);

//开发测试用
uint16_t Mid_SceneClassifyTotalCatalogRead(uint16_t *catalogTotal, uint32_t *dataLength, uint32 dataClassify,uint32_t *catalogvalidstartaddr);
uint16_t Mid_SceneClassifyCatalogInfoRead(scene_catalogInfo_s *dataInfo, uint32 dataClassify,uint32_t catalogAddr);
uint16_t Mid_SceneClassifyDataPreRead(uint8_t *data, uint32_t addr, uint32_t length, uint32 dataClassify);



#endif		// DATA_MANAGE_APP_H


