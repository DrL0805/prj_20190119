#ifndef APP_SCENEDATA_MANAGE_H
#define APP_SCENEDATA_MANAGE_H




void App_SceneData_DataSaveInit(void (*TimerCb)(TimerHandle_t xTimer));
void App_SceneData_SaveDynamic(uint32 utc, uint32 sceneDataType, scene_data_s *sceneDateTemp);
void App_SceneData_SaveEnd(uint32 sceneDataType);
void App_SceneData_SaveCache(uint32 sceneDataType);
void App_SceneData_StorageSpaceCheck(uint32 sceneDataType);
void   App_SceneData_ReadList(uint32 sceneDataType);

uint16 App_SceneData_DataRequestTotalInfoAck(uint32 dataClassify, uint8 flowControl);
uint16 App_SceneData_DataCatalogInfoRead(uint32 dataClassify, uint16 catalogNum, uint8 flowControl);
uint16 App_SceneData_DataRequestData(uint32 dataClassify, uint32 utc, uint16 packNum, uint8 flowControl);
uint16 App_SceneData_DataDeleteCatalog(uint32 dataClassify, uint32 utc, uint8 flowControl);
uint16 App_SceneData_DataUploadProcess(void);


void App_SceneData_DataSaveFakeData(void);
void App_SceneData_DataSavePrintf(uint16_t dataClassify);

#endif


