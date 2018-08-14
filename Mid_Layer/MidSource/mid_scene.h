#ifndef MID_SCENE_H
#define MID_SCENE_H



#define 	SCENE_DATA_LEN 		10

//场景类型，应与场景数据分类一致
typedef enum 
{
	RUN_SCENE_TYPE		= 0,
	CLIMB_SCENE_TYPE,
	SWING_SCENE_TYPE,
}SCENE_DATA_TYPE;

// 事件定义
typedef enum
{
	RUN_SCENE_OPEN 		= 0,
	RUN_SCENE_CLOSE,
	RUN_SCENE_PROCESS,
	
	CLIMB_SCENE_OPEN,
	CLIMB_SCENE_CLOSE,
	CLIMB_SCENE_PROCESS,
	
	SWING_SCENE_OPEN,
	SWING_SCENE_CLOSE,
	SWING_SCENE_PROCESS,
}SCENE_INTERFACE_E;


typedef struct 
{
	uint8 id;
}scene_event_s;


typedef union 
{
	uint8 databuf[SCENE_DATA_LEN];
	struct
	{
		uint16   paceLog;      		/* s/km 配速 10S内实时更新*/
	    uint16  freqLog;     		/* step/min 步频 10S内实时更新*/
	    uint8   hrmLog; 			/* 次 心率记录 */
	}runScene;
	struct
	{
		int16   elevation;      		/* m 海拔 */
	    int16   climbSpeed;     		/* m/h 当前垂直速度 */
	    uint8   hrmLog; 				/* 次 心率记录 */
	}climbScene;
	struct
	{
	    uint16  pullLog;     		/* 次/min 划水率 10S内实时更新*/
	    uint8   hrmLog; 			/* 次 心率记录 */
	}swingScene;
}scene_data_s;

//跑步信息结构体
typedef struct 
{
	uint32 		startUtc;
	uint32 		duarationTotal;
	uint32 		duratonT1;
	uint32 		duratonT2;
	uint32 		duratonT3;		
	uint32 		duratonT4;
	uint32 		duratonT5;
	uint16 		distanceTotal;
	uint32 		stepTotal;
	uint16 		calorieTotal;
}run_detail_s;


//登山信息结构体
typedef struct 
{
	uint32 		startUtc;
	uint32 		duarationTotal;
	uint32 		duratonT1;
	uint32 		duratonT2;
	uint32 		duratonT3;		
	uint32 		duratonT4;
	uint32 		duratonT5;
	uint16 		upDistance;
	uint16 		downDistance;
	uint16 		altitudeHighest;
	uint16 		altitudeLowest;
	uint16 		calorieTotal;
}climb_detail_s;

//游泳信息结构体
typedef struct 
{
	uint32 		startUtc;
	uint32 		duarationTotal;
	uint32 		duratonT1;
	uint32 		duratonT2;
	uint32 		duratonT3;		
	uint32 		duratonT4;
	uint32 		duratonT5;
	uint16 		pullTotal;
	uint16 		calorieTotal;
}swing_detail_s;

//场景信息联合体
typedef union
{
	run_detail_s   runDetail;
	climb_detail_s climbDetail;
	swing_detail_s swingDetail;
}scene_detail_s;



uint16 Mid_Scene_Init(void);
uint16 Mid_Scene_EventProcess(scene_event_s* msg);
uint16 Mid_Scene_DetailRead(uint32 sceneType, scene_detail_s *sceneDetailTemp);
uint16 Mid_Scene_LogDataRead(uint32 sceneType, scene_data_s *sceneDataTemp);


#endif


