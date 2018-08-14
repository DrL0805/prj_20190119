#include "platform_common.h"

/* FreeRTOS includes */
#include "rtos.h"

#include "mid_extflash.h"
#include "mid_remind_manage.h"
#include "multimodule_task.h"
#include "flash_task.h"




/*************** func declaration ********************************/
//none



/*******************macro define*******************/
//none



/************** variable define *****************************/
// 数据区域分配管理
typedef struct 
{
	uint32	startAddr;			// 数据区域开始地址
	uint32	validStartAddr;		// 可进行分配开始地址
	uint32	validLength;		// 剩余可分配长度
}data_allocation_s;

static data_allocation_s	dataAreaManage;


// 数据信息
typedef struct
{
	uint8		init;						// 数据是否初始化了		1：已初始化 	0:未初始化
	uint16	dataListLenght;				// 每条信息长度
	uint16 	dataValidLenght; 			// 信息实际长度

	uint32	dataAreaStartAddr;			// 数据在外部flash分配区域的开始地址
	uint32	dataAreaEndAddr;			// 数据在外部flash分配区域的结束地址,包含该地址

	uint32	dataWritableStartAddr;		// 数据在外部flash可写开始地址
	uint32	dataWritableLength;			// 数据在外部flash可写长度

	// uint32	dataReadableStartAddr;		// 数据在外部flash读开始地址

	uint32	dataValidStartAddr;			// 数据在外部flash的有效数据开始地址
	uint32	dataValidEndAddr;			// 数据在外部flash的有效数据结束地址,不包含该地址

}remind_msg_info_s;


static remind_msg_info_s		remindMsgInfo[REMIND_CLASSIFY_NUM];




/*******************function define*******************/
//**********************************************************************
// 函数功能:	初始化数据管理模块数据
// 输入参数：	无
// 返回参数：	无
uint16  Mid_RemindMsg_ManageInit(void)
{
	uint16 i;

	dataAreaManage.startAddr					= REMIND_DATA_AREA_START_ADD;
	dataAreaManage.validStartAddr				= dataAreaManage.startAddr;
	dataAreaManage.validLength					= REMIND_DATA_AREA_LENGTH;

	for(i = 0; i < REMIND_CLASSIFY_NUM; i++)
	{
		remindMsgInfo[i].init			= 0;
	}
	
	return 0;
}


//**********************************************************************
// 函数功能:	初始化提醒类数据的数据区域
// 输入参数：	msgClassify:	提醒信息类型
// 				dataSize:		数据分配长度，为REMND_DATA_SECTOR_LENGTH整数倍
// 				listSize: 		信息存储字节数
// 返回参数：	0x00: 初始化成功
// 				0xff: 区域分配失败
uint16 Mid_RemindMsg_Init(uint32 msgClassify, uint32 dataSize, uint16 listSize)
{
	// 未定义数据
	if(msgClassify >= REMIND_CLASSIFY_NUM)
		return 0xff;

	// 至少2sector存储空间
	if(dataSize < REMIND_DATA_SECTOR_LENGTH_MIN)
		return 0xff;

	// 该类数据已初始化或该类数据分配长度超出
	if( (remindMsgInfo[msgClassify].init == 1) ||(dataSize > dataAreaManage.validLength))
		return 0xff;

	// 数据区域分配
	remindMsgInfo[msgClassify].init							= 1;
	remindMsgInfo[msgClassify].dataListLenght				= listSize;//每条信息长度固定
	remindMsgInfo[msgClassify].dataValidLenght 		    	= 0;

	remindMsgInfo[msgClassify].dataAreaStartAddr			= dataAreaManage.validStartAddr;
	remindMsgInfo[msgClassify].dataAreaEndAddr				= remindMsgInfo[msgClassify].dataAreaStartAddr + dataSize - 1;	

	remindMsgInfo[msgClassify].dataWritableStartAddr		= remindMsgInfo[msgClassify].dataAreaStartAddr;
	// remindMsgInfo[msgClassify].dataReadableStartAddr		= remindMsgInfo[msgClassify].dataAreaStartAddr;
	remindMsgInfo[msgClassify].dataWritableLength			= 0;

	remindMsgInfo[msgClassify].dataValidStartAddr			= remindMsgInfo[msgClassify].dataAreaStartAddr;
	remindMsgInfo[msgClassify].dataValidEndAddr				= remindMsgInfo[msgClassify].dataAreaStartAddr;

	// 数据分配有效区域更新
	dataAreaManage.validStartAddr							= dataAreaManage.validStartAddr + dataSize;
	dataAreaManage.validLength								= dataAreaManage.validLength - dataSize;

	return 0x00;
}

//**********************************************************************
// 函数功能:	创建新信息记录，并存储utc（新信息时先执行一次创建）
// 输入参数：	utc： 			utc时间
//				msgClassify:	提醒信息类型
// 返回参数：	0x00: 操作成功
// 				0xff: 操作失败
uint16 Mid_RemindMsg_CreatNewList(uint32 utc, uint32 msgClassify,uint32 *catalogStartAddr)
{
	flash_task_msg_t		flashMsg;
	uint16				result;
	uint8 				utcbuf[4];
	uint32 				lengthTemp = 0;

	// 检查数据是否合法
	if(msgClassify >= REMIND_CLASSIFY_NUM)
		return 0xff;

	// 检查是否分配区域了
	if(remindMsgInfo[msgClassify].init == 0)
		return 0xff;

	// 检查存储空间是否足够存储下一条信息
	if(remindMsgInfo[msgClassify].dataWritableLength < remindMsgInfo[msgClassify].dataListLenght)//空间无法存储一条完整的信息
	{
		//当前为尾区域
		if((remindMsgInfo[msgClassify].dataWritableStartAddr + remindMsgInfo[msgClassify].dataListLenght)
			> remindMsgInfo[msgClassify].dataAreaEndAddr)
		{
			// 擦除区域（第一区域）
			flashMsg.id 									= EXTFLASH_ID;
			flashMsg.flash.extflashEvent.para.startAddr 	= remindMsgInfo[msgClassify].dataAreaStartAddr;//切换到第一区域
			flashMsg.flash.extflashEvent.para.length 		= REMIND_DATA_SECTOR_LENGTH;		
			flashMsg.flash.extflashEvent.para.endAddr 		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
			flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_4K_ERASE;				
			result 											= FlashTask_EventSet(&flashMsg);

			if(result)
				return 0xff;

			remindMsgInfo[msgClassify].dataValidStartAddr		= remindMsgInfo[msgClassify].dataAreaStartAddr + REMIND_DATA_SECTOR_LENGTH;
			remindMsgInfo[msgClassify].dataValidEndAddr			= remindMsgInfo[msgClassify].dataAreaEndAddr;

			remindMsgInfo[msgClassify].dataWritableStartAddr  	= remindMsgInfo[msgClassify].dataAreaStartAddr;
			remindMsgInfo[msgClassify].dataWritableLength		= REMIND_DATA_SECTOR_LENGTH;		
		}
		else 
		{
			if (remindMsgInfo[msgClassify].dataValidStartAddr == remindMsgInfo[msgClassify].dataValidEndAddr)//首次存储，需擦除
			{
				// 擦除首块
				flashMsg.id 										= EXTFLASH_ID;
				flashMsg.flash.extflashEvent.para.startAddr 		= remindMsgInfo[msgClassify].dataAreaStartAddr;
				flashMsg.flash.extflashEvent.para.length 			= REMIND_DATA_SECTOR_LENGTH;		
				flashMsg.flash.extflashEvent.para.endAddr 			= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
				flashMsg.flash.extflashEvent.id 					= EXTFLASH_EVENT_4K_ERASE;				
				result 												= FlashTask_EventSet(&flashMsg);

				if(result)
					return 0xff;

				remindMsgInfo[msgClassify].dataValidStartAddr		= remindMsgInfo[msgClassify].dataAreaStartAddr;
				remindMsgInfo[msgClassify].dataValidEndAddr 		= remindMsgInfo[msgClassify].dataAreaStartAddr;
				remindMsgInfo[msgClassify].dataWritableStartAddr 	= remindMsgInfo[msgClassify].dataAreaStartAddr;
			}
			else
			{
				// 擦除区域(下一区域)
				flashMsg.id 										= EXTFLASH_ID;
				flashMsg.flash.extflashEvent.para.startAddr 		= remindMsgInfo[msgClassify].dataWritableStartAddr + remindMsgInfo[msgClassify].dataWritableLength;
				flashMsg.flash.extflashEvent.para.length 			= REMIND_DATA_SECTOR_LENGTH;		
				flashMsg.flash.extflashEvent.para.endAddr 			= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
				flashMsg.flash.extflashEvent.id 					= EXTFLASH_EVENT_4K_ERASE;				
				result 												= FlashTask_EventSet(&flashMsg);

				if(result)
					return 0xff;

				//目前只分配2个区
				//remindMsgInfo[msgClassify].dataValidStartAddr		= remindMsgInfo[msgClassify].dataAreaStartAddr;
				remindMsgInfo[msgClassify].dataValidEndAddr			= remindMsgInfo[msgClassify].dataAreaStartAddr + REMIND_DATA_SECTOR_LENGTH;
				remindMsgInfo[msgClassify].dataWritableStartAddr 	= remindMsgInfo[msgClassify].dataAreaStartAddr + REMIND_DATA_SECTOR_LENGTH;
			}
			
			remindMsgInfo[msgClassify].dataWritableLength			+= REMIND_DATA_SECTOR_LENGTH;			
		}
		// remindMsgInfo[msgClassify].dataReadableStartAddr 	= remindMsgInfo[msgClassify].dataValidStartAddr;	
	}

	//每条信息固定长度remindMsgInfo[msgClassify].dataListLenght，未使用到部分也跳过
	lengthTemp 											= remindMsgInfo[msgClassify].dataValidLenght % remindMsgInfo[msgClassify].dataListLenght;//跳过上一条未使用到空间
	remindMsgInfo[msgClassify].dataWritableLength  		-= lengthTemp;
	remindMsgInfo[msgClassify].dataWritableStartAddr 	+= lengthTemp;

	//新列表开始，有效长度重新清零
	remindMsgInfo[msgClassify].dataValidLenght 				= 0;


	utcbuf[3] 					= (utc >> 24) & 0xff;
	utcbuf[2] 					= (utc >> 16) & 0xff;
	utcbuf[1] 					= (utc >> 8) & 0xff;
	utcbuf[0] 					= (utc >> 0) & 0xff;

	*catalogStartAddr 			= remindMsgInfo[msgClassify].dataWritableStartAddr;		

	// 写入数据至外部flash
	flashMsg.id 										= EXTFLASH_ID;
	flashMsg.flash.extflashEvent.para.startAddr 		= remindMsgInfo[msgClassify].dataWritableStartAddr;
	flashMsg.flash.extflashEvent.para.dataAddr 			= utcbuf;
	flashMsg.flash.extflashEvent.para.length 			= 4;		
	flashMsg.flash.extflashEvent.para.endAddr 			= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
	flashMsg.flash.extflashEvent.id 					= EXTFLASH_EVENT_WRITE;	

	// 更新数据区域可写的开始地址与长度
	remindMsgInfo[msgClassify].dataWritableLength		-= 6;//越过2byte长度存储地址
	remindMsgInfo[msgClassify].dataWritableStartAddr	+= 6;
	remindMsgInfo[msgClassify].dataValidEndAddr 		= remindMsgInfo[msgClassify].dataWritableStartAddr - 1;
	remindMsgInfo[msgClassify].dataValidLenght 			+= 6;

    FlashTask_EventSet(&flashMsg);

	return 0x00;
}

//**********************************************************************
// 函数功能:	初始化分类数据的目录与数据区域
// 输入参数：	data:			原始数据指针
// 				length:			数据长度，不能超过DM_APP_SECTOR_LENGTH
// 				msgClassify:	分类数据
// 返回参数：	0x00: 操作成功
// 				0xff: 操作失败
uint16 Mid_RemindMsg_SaveList(uint8 *data, uint32 length, uint32 msgClassify)
{
	flash_task_msg_t		flashMsg;

	uint32 				lengthTemp;


	lengthTemp 		= length;

	// 检查数据是否合法
	if(msgClassify >= REMIND_CLASSIFY_NUM)
		return 0xff;

	// 单次写入的数据不能超过DM_APP_SECTOR_LENGTH
	if(lengthTemp > REMIND_DATA_SECTOR_LENGTH)
		return 0xff;

	// 检查是否分配区域了
	if(remindMsgInfo[msgClassify].init == 0)
		return 0xff;

	//检查是否可有效写入
	if(remindMsgInfo[msgClassify].dataWritableLength < lengthTemp)
	{
		lengthTemp 		= remindMsgInfo[msgClassify].dataWritableLength;
	}

	//信息长度越界，截取部分存储
	if ((remindMsgInfo[msgClassify].dataValidLenght + lengthTemp)> remindMsgInfo[msgClassify].dataListLenght)
	{
		lengthTemp 		= remindMsgInfo[msgClassify].dataListLenght - remindMsgInfo[msgClassify].dataValidLenght;
	}

	// 写入数据至外部flash
	flashMsg.id 									= EXTFLASH_ID;
	flashMsg.flash.extflashEvent.para.startAddr 	= remindMsgInfo[msgClassify].dataWritableStartAddr;
	flashMsg.flash.extflashEvent.para.dataAddr 		= (uint8*)data;
	flashMsg.flash.extflashEvent.para.length 		= lengthTemp;	
	flashMsg.flash.extflashEvent.para.endAddr 		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
	flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_WRITE;
	
	// 更新数据区域可写的开始地址与长度
	remindMsgInfo[msgClassify].dataWritableLength		-= lengthTemp;
	remindMsgInfo[msgClassify].dataWritableStartAddr	+= lengthTemp;
	remindMsgInfo[msgClassify].dataValidEndAddr 		+= lengthTemp;
	remindMsgInfo[msgClassify].dataValidLenght 			+= lengthTemp;

    FlashTask_EventSet(&flashMsg);

	return 0x00;
}

//**********************************************************************
// 函数功能:	信息存储完成，保存信息长度
// 				msgClassify:	分类数据
// 返回参数：	0x00: 操作成功
// 				0xff: 操作失败
uint16 Mid_RemindMsg_SaveListEnd(uint32 msgClassify)
{
	flash_task_msg_t		flashMsg;
	uint8 				data[2];

	// 检查数据是否合法
	if(msgClassify >= DM_APP_DATA_CLASSIFY_NUM)
		return 0xff;

	// 检查是否分配区域了
	if(remindMsgInfo[msgClassify].init == 0)
		return 0xff;

	data[1] 		= (remindMsgInfo[msgClassify].dataValidLenght >> 8) & 0xff;
	data[0] 		= (remindMsgInfo[msgClassify].dataValidLenght >> 0) & 0xff;


	// 信息长度（UTC+LEN+MSG）写入至外部flash
	flashMsg.id 									= EXTFLASH_ID;
	flashMsg.flash.extflashEvent.para.startAddr 	= remindMsgInfo[msgClassify].dataWritableStartAddr - remindMsgInfo[msgClassify].dataValidLenght + 4;
	flashMsg.flash.extflashEvent.para.dataAddr 		= (uint8*)data;
	flashMsg.flash.extflashEvent.para.length 		= 2;	
	flashMsg.flash.extflashEvent.para.endAddr 		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
	flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_WRITE;

    FlashTask_EventSet(&flashMsg);

	return 0x00;
}

//**********************************************************************
// 函数功能:	从当前地址逐条读取提醒信息
// 输入参数：	data:			数据指针:UTC+LEN+MSG
// 				catalogDataStartAddr:目录记录的数据起始地址
// 				length:			数据长度
// 				msgClassify:	数据分类指定
// 返回参数：	0x00: 操作成功
// 				0xff: 操作失败
uint16 Mid_RemindMsg_Read(uint8 *data,uint32 catalogDataStartAddr, uint32 msgClassify)
{
	flash_task_msg_t		flashMsg;
	uint8 				lengthTemp;

	// 检查数据是否合法
	if(msgClassify >= REMIND_CLASSIFY_NUM)
		return 0xff;

	// 检查是否分配区域了
	if(remindMsgInfo[msgClassify].init == 0)
		return 0xff;
	
	if (catalogDataStartAddr > remindMsgInfo[msgClassify].dataAreaEndAddr || catalogDataStartAddr < remindMsgInfo[msgClassify].dataAreaStartAddr)
	{
		return 0xff;
	}

	// 获取（UTC+LEN）信息
	flashMsg.id 									= EXTFLASH_ID;
	flashMsg.flash.extflashEvent.para.startAddr 	= catalogDataStartAddr;
	flashMsg.flash.extflashEvent.para.dataAddr 		= (uint8*)data;
	flashMsg.flash.extflashEvent.para.length 		= 6;	
	flashMsg.flash.extflashEvent.para.endAddr 		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
	flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_READ;
    FlashTask_EventSet(&flashMsg);

    lengthTemp  									= (uint16)data[4] << 8 | data[5];

    // 获取信息内容
	flashMsg.id 									= EXTFLASH_ID;
	flashMsg.flash.extflashEvent.para.startAddr 	= catalogDataStartAddr + 6;
	flashMsg.flash.extflashEvent.para.dataAddr 		= (uint8*)(data + 6);
	flashMsg.flash.extflashEvent.para.length 		= lengthTemp - 6;	
	flashMsg.flash.extflashEvent.para.endAddr 		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
	flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_READ;
    FlashTask_EventSet(&flashMsg);

    return 0;
}





