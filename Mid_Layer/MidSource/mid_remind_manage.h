#ifndef MID_REMIND_MANAGE_H
#define MID_REMIND_MANAGE_H



// 外部flash存储空间分配
// 简短数据区域分配

#define		REMIND_DATA_AREA_START_ADD				0xF2000
#define		REMIND_DATA_AREA_LENGTH					0x06000//24kb


// 数据最小片区长度，一次性写入的最大字节数据，超过会报错
#define		REMIND_DATA_SECTOR_LENGTH						4096

#define		REMIND_DATA_SECTOR_LENGTH_MIN					8192

// 数据分量总量
#define		REMIND_CLASSIFY_NUM					3



// 数据分类
typedef enum
{
	REMIND_CALL_PHONE		= 0,
	REMIND_MSG_BRIEF,
	REMIND_MSG_WEIXIN,	
}remind_classify_e;


uint16 Mid_RemindMsg_ManageInit(void);
uint16 Mid_RemindMsg_Init(uint32 msgClassify, uint32 dataSize, uint16 listSize);
uint16 Mid_RemindMsg_CreatNewList(uint32 utc, uint32 dataClassify,uint32 *catalogStartAddr);
uint16 Mid_RemindMsg_SaveList(uint8 *data, uint32 length, uint32 msgClassify);
uint16 Mid_RemindMsg_SaveListEnd(uint32 msgClassify);
uint16 Mid_RemindMsg_Read(uint8 *data,uint32 catalogDataStartAddr, uint32 msgClassify);

#endif

