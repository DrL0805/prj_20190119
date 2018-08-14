#ifndef 	FLASH_TASK_H
#define  	FLASH_TASK_H


#define			FLASH_TASK_QUEUE_SIZE			32



typedef enum
{
	EXTFLASH_ID			= 0x01,
	NANDFLASH_ID,
	FRONT_ID,
}flash_id;


typedef struct 
{
	uint16		id;
	union
	{
		extflash_event_t	extflashEvent;
		front_event_t		frontEvent;
	}flash;
}flash_task_msg_t;


uint16 FlashTask_EventSet(flash_task_msg_t* msg);
void   FlashTask_Create(void);

#endif
