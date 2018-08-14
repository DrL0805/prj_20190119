#ifndef			MID_COMPASS_H
#define			MID_COMPASS_H



typedef enum
{
	COMPASS_CALI_START,
	COMPASS_CALI_STOP,
	COMPASS_UPDATA_START,
	COMPASS_UPDATA_STOP,
}COMPASS_INTERFACE_T;

typedef struct 
{
	uint16	id;
}compass_event_s;



void   Mid_Compass_Init(void);
uint16 Mid_Compass_AngleRead(void);
uint16 Mid_Compass_CompensatoryAngleRead(void);
uint16 Mid_Compass_BiasSet(int8  dataTemp);
int8   Mid_Compass_BiasRead(void);
uint16 Mid_Compass_EventProcess(compass_event_s *msg);


#endif			//	COMPASS_APP_H

