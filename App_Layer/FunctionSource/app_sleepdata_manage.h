#ifndef APP_SLEEPDATA_MANAGE_H
#define APP_SLEEPDATA_MANAGE_H



#define			SLEEP_DATA_UPLOAD_MAX_LEN		8

void App_SleepData_SaveInit(void (*TimerCb)(TimerHandle_t xTimer));
void App_SleepData_Save(void);
void App_SleepData_Read(sleep_ui_info *sleepInfoTemp);
uint16 App_SleepData_DataRequestTotalInfoAck(uint8 flowControl);
uint16 App_SleepData_DataCatalogInfoRead(uint16 catalogNum, uint8 flowControl);
uint16 App_SleepData_DataUploadProcess(void);


void App_SleepData_DataSaveFakeData(void);
void App_SleepData_DataSavePrintf(uint16_t dataClassify);



#endif



