#ifndef			APP_PACKDATA_MANAGE_H
#define			APP_PACKDATA_MANAGE_H


//如ＡＰＰ需要睡眠分包数据，需要屏蔽该宏定义
// #define 		SLEEP_PACK_DUMMY 			1

// step pack 
void App_PackData_Step5MinSave(uint32 utc, uint16 step);
void App_PackData_StepSaveCache(void);

// sleep pack
uint16 App_PackData_SleepMinLengthSet(uint16 minLength);
void App_PackData_SleepSaveCache(void);
void App_PackData_Sleep5MinSave(uint32 utc, uint16 step, uint16 sleepMotion);

//heart rate pack
void App_PackData_HrmSave(uint32 utc, uint8 hrval);
void App_PackData_HrmSaveCache(void);


void App_PackData_DataSavePowerdownRecover(void);
void App_PackData_HistoryDataSave(void);


// pack data common
void App_PackData_DataSaveInit(void (*TimerCb)(TimerHandle_t xTimer));

uint16 App_PackData_DataRequestTotalInfoAck(uint32 dataClassify, uint8 flowControl);

uint16 App_PackData_DataCatalogInfoRead(uint32 dataClassify, uint16 catalogNum, uint8 flowControl);

uint16 App_PackData_DataRequestData(uint32 dataClassify, uint32 utc, uint16 packNum, uint8 flowControl);

uint16 App_PackData_DataUploadProcess(void);

uint16 App_PackData_DataTotalDelete(uint8 flowControl);

uint16 App_PackData_DataDeleteClassify(uint32 dataClassify, uint8 flowControl);

uint16 App_PackData_DataDeleteCatalog(uint32 dataClassify, uint32 utc, uint8 flowControl);



void App_PackData_DataSaveFakeData(void);
void App_PackData_DataSaveFakeDataHeart(void);
void App_PackData_DataSavePrintf(uint16_t dataClassify);

#endif			//APP_DATASAVE_H
