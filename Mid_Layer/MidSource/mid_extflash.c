/**********************************************************************
**
**模块说明: mid层flash接口
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.25  修改流程  ZSL  
**
**********************************************************************/
#include "drv_extflash.h"
#include "mid_extflash.h"

/* FreeRTOS includes */
#include "rtos.h"

/*************** func declaration ********************************/
#if 0
static uint16 ExtflashEnable(void);
static uint16 ExtflashDisable(void);
static uint16 ExtflashSelfTest(void);
static uint16 ExtflashWriteSN(extflash_para_t *para);
static uint16 ExtflashReadSN(extflash_para_t *para);
static uint16 ExtflashWriteBleMac(extflash_para_t *para);
static uint16 ExtflashReadBleMac(extflash_para_t *para);
static uint16 ExtflashReadMac(extflash_para_t *para);
static uint16 ExtflashSleep(void);
static uint16 ExtflashWakeUp(void);
static uint16 ExtflashErase_4K(extflash_para_t *para);
static uint16 ExtflashWrite(extflash_para_t *para);
static uint16 ExtflashWriteWithBuf(extflash_para_t *para);
static uint16 ExtflashRead(extflash_para_t *para);
static uint16 ExtflashWriteBleBrocastName(extflash_para_t *para);
static uint16 ExtflashReadBleBrocastName(extflash_para_t *para);
static uint16 ExtflashWriteSN_Custom(extflash_para_t *para);
static uint16 ExtflashReadSN_Custom(extflash_para_t *para);
#endif
// storage region 

#define FACTORY_SELFTEST_START_ADDR    0x0000      //工厂测试段地址(flash自检)
#define	FACTORY_SN_START_ADDR          0x1000      //工厂SN地址		
#define	BLE_MAC_START_ADDR             0x2000      //蓝牙MAC地址		
#define	CUSTOM_SN_START_ADDR           0x3000      //客户SN地址		

#define	BROCAST_NAME_ID_ADDR           0x8000      //识别码地址,字段长度为32字节
#define	BROCAST_NAME_INFO_ADDR         0x8020      //蓝牙广播信息地址，有效数据为32个字节
#define	BLE_PAIRING_PASSKEY_ADDR       0x9000      //蓝牙配对Passkey   

#define	BROCAST_NAME_ID_LEN	           12

// idle tick
#define	EXTFLASH_READ_WRITE_IDELTICKS  20
#define	EXTFLASH_ERASE_IDELTICKS       60

// cb ID max
#define	EXTFLASH_CB_ID_MAX             32

static const uint8 BROCAST_NAME_ID[BROCAST_NAME_ID_LEN] ={"brocast name"};

typedef struct 
{
	uint8   occupy;
	uint8   result;
	SemaphoreHandle_t extflashCbSemaphore;
}extflash_cb_s;

static extflash_cb_s extflashSemaphore[EXTFLASH_CB_ID_MAX];				/**< Queue handler extflash cb queue*/

static SemaphoreHandle_t extflashMutex;

//**********************************************************************
// 函数功能:	等待flash空闲
// 输入参数：	
// WaitTick:    等待flash空闲的tick个数
// 返回参数：	
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
static uint16 IdleWait(uint16 WaitTick)
{
	uint16 waitCnt, ret;
	ret 	= 0;
	waitCnt = 0;
	while(Drv_Extflash_CheckState())
	{
		waitCnt++;
		if(waitCnt > WaitTick)
		{
			ret = 0xff;
			break;
		}
		vTaskDelay(10);		// ticks
		// 防止在其他任务调用时将其关闭
		Drv_Extflash_SendCmd(FLASH_WAKEUP_CMD);
	}
	return ret;
}

//**********************************************************************
// 函数功能:	8M flash使能：硬件初始化
// 输入参数：	无
// 返回参数：	
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
static uint16 ExtflashEnable(void)
{
	return Drv_Extflash_Open();
}

//**********************************************************************
// 函数功能:	8M flash关闭：硬件初始化
// 输入参数：	无
// 返回参数：	
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
static uint16 ExtflashDisable(void)
{
	return Drv_Extflash_Close();
}

//**********************************************************************
// 函数功能:	8M flash自检
// 输入参数：	无
// 返回参数：	
// 0x00    :    自检成功
// 0xff    :    自检失败
//**********************************************************************
static uint16 ExtflashSelfTest(void)
{
	uint8  dataTemp[8], i;
	uint16 ret = 0;
    Drv_Extflash_SendCmd(FLASH_WAKEUP_CMD);

	for(i = 0; i < 8; i++)
	{
		dataTemp[i] = i;
	}
	Drv_Extflash_Write(dataTemp, FACTORY_SELFTEST_START_ADDR, 8);
	ret |= IdleWait(EXTFLASH_READ_WRITE_IDELTICKS);
	Drv_Extflash_Erase4K(FACTORY_SELFTEST_START_ADDR);
	ret |= IdleWait(EXTFLASH_ERASE_IDELTICKS);

	Drv_Extflash_Read(dataTemp, FACTORY_SELFTEST_START_ADDR, 8);
	for(i = 0; i < 8; i++)
	{
		if(dataTemp[i] != 0xff)
			ret |= 0xff;
		dataTemp[i] = 0x01 << i;
	}

	Drv_Extflash_Write(dataTemp, FACTORY_SELFTEST_START_ADDR, 8);
	for(i = 0; i < 8; i++)
	{
		dataTemp[i] = 0;
	}
	ret |= IdleWait(EXTFLASH_READ_WRITE_IDELTICKS);

	Drv_Extflash_Read(dataTemp, FACTORY_SELFTEST_START_ADDR, 8);
	for(i = 0; i < 8; i++)
	{
		if(dataTemp[i] != 0x01<<i)
			ret |= 0xff;
	}
	ret |= IdleWait(EXTFLASH_READ_WRITE_IDELTICKS);
	Drv_Extflash_SendCmd(FLASH_SLEEP_CMD);
	return ret;
}

//**********************************************************************
// 函数功能:	写入SN序列号
// 输入参数：	
// para 	: 	para->dataAddr存放SN信息
// 返回参数：	
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
static uint16 ExtflashWriteSN(extflash_para_t *para)
{
	Drv_Extflash_SendCmd(FLASH_WAKEUP_CMD);
	para->result |= IdleWait(EXTFLASH_READ_WRITE_IDELTICKS);
	Drv_Extflash_Erase4K(FACTORY_SN_START_ADDR);
	para->result |= IdleWait(EXTFLASH_ERASE_IDELTICKS);
	Drv_Extflash_Write(para->dataAddr, FACTORY_SN_START_ADDR, para->length);
	para->result |= IdleWait(EXTFLASH_READ_WRITE_IDELTICKS);
	Drv_Extflash_SendCmd(FLASH_SLEEP_CMD);
	return 0;
}

//**********************************************************************
// 函数功能:	读取SN序列号
// 输入参数：	
// para 	: 	para->dataAddr存放SN信息
// 返回参数：	
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
static uint16 ExtflashReadSN(extflash_para_t *para)
{
    Drv_Extflash_SendCmd(FLASH_WAKEUP_CMD);
	para->result |= IdleWait(EXTFLASH_READ_WRITE_IDELTICKS);
	Drv_Extflash_Read(para->dataAddr, FACTORY_SN_START_ADDR, para->length);
	para->result |= IdleWait(EXTFLASH_READ_WRITE_IDELTICKS);
	Drv_Extflash_SendCmd(FLASH_SLEEP_CMD);
	return 0;
}

//**********************************************************************
// 函数功能:	写用户自定义SN序列号
// 输入参数：	
// para 	: 	para->dataAddr存放SN信息
// 返回参数：	
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
static uint16 ExtflashWriteSN_Custom(extflash_para_t *para)
{
    Drv_Extflash_SendCmd(FLASH_WAKEUP_CMD);
	para->result |= IdleWait(EXTFLASH_READ_WRITE_IDELTICKS);
	Drv_Extflash_Erase4K(CUSTOM_SN_START_ADDR);
	para->result |= IdleWait(EXTFLASH_ERASE_IDELTICKS);
	Drv_Extflash_Write(para->dataAddr, CUSTOM_SN_START_ADDR, para->length);
	para->result |= IdleWait(EXTFLASH_READ_WRITE_IDELTICKS);
	Drv_Extflash_SendCmd(FLASH_SLEEP_CMD);
	return 0;
}

//**********************************************************************
// 函数功能:	读用户自定义SN序列号
// 输入参数：	
// para 	: 	para->dataAddr存放SN信息
// 返回参数：	
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
static uint16 ExtflashReadSN_Custom(extflash_para_t *para)
{
    Drv_Extflash_SendCmd(FLASH_WAKEUP_CMD);
	para->result |= IdleWait(EXTFLASH_READ_WRITE_IDELTICKS);
	Drv_Extflash_Read(para->dataAddr, CUSTOM_SN_START_ADDR, para->length);
	para->result |= IdleWait(EXTFLASH_READ_WRITE_IDELTICKS);
	Drv_Extflash_SendCmd(FLASH_SLEEP_CMD);
	return 0;
}

//**********************************************************************
// 函数功能:	写蓝牙MAC地址
// 输入参数：	
// para 	: 	para->dataAddr存放ble MAC信息
// 返回参数：	
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
static uint16 ExtflashWriteBleMac(extflash_para_t *para)
{
    Drv_Extflash_SendCmd(FLASH_WAKEUP_CMD);
	Drv_Extflash_Erase4K(BLE_MAC_START_ADDR);
	para->result |= IdleWait(EXTFLASH_ERASE_IDELTICKS);
	Drv_Extflash_Write(para->dataAddr, BLE_MAC_START_ADDR, para->length);
	para->result |= IdleWait(EXTFLASH_READ_WRITE_IDELTICKS);
	Drv_Extflash_SendCmd(FLASH_SLEEP_CMD);
	return 0;
}

//**********************************************************************
// 函数功能:	读蓝牙MAC地址
// 输入参数：	
// para 	: 	para->dataAddr存放ble MAC信息
// 返回参数：	
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
static uint16 ExtflashReadBleMac(extflash_para_t *para)
{
    Drv_Extflash_SendCmd(FLASH_WAKEUP_CMD);
	para->result |= IdleWait(EXTFLASH_READ_WRITE_IDELTICKS);
	Drv_Extflash_Read(para->dataAddr, BLE_MAC_START_ADDR, para->length);
	para->result |= IdleWait(EXTFLASH_READ_WRITE_IDELTICKS);
	Drv_Extflash_SendCmd(FLASH_SLEEP_CMD);
	return 0;
}

//**********************************************************************
// 函数功能:	写蓝牙广播名
// 输入参数：	
// para 	: 	para->dataAddr存放ble广播名信息
// 返回参数：	
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
static uint16 ExtflashWriteBleBrocastName(extflash_para_t *para)
{
    Drv_Extflash_SendCmd(FLASH_WAKEUP_CMD);

	// 擦除蓝牙广播信息块
	Drv_Extflash_Erase4K(BROCAST_NAME_ID_ADDR);
	para->result |= IdleWait(EXTFLASH_ERASE_IDELTICKS);
	// 写入识别字段
	Drv_Extflash_Write((uint8 *)BROCAST_NAME_ID, BROCAST_NAME_ID_ADDR, BROCAST_NAME_ID_LEN);
	para->result |= IdleWait(EXTFLASH_READ_WRITE_IDELTICKS);
	// 写入广播名
	Drv_Extflash_Write(para->dataAddr, BROCAST_NAME_INFO_ADDR, para->length);
	para->result |= IdleWait(EXTFLASH_READ_WRITE_IDELTICKS);
	Drv_Extflash_SendCmd(FLASH_SLEEP_CMD);

	return 0;
}

//**********************************************************************
// 函数功能:	读蓝牙广播名
// 输入参数：	
// para 	: 	para->dataAddr存放ble广播名信息
// 返回参数：	
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
static uint16 ExtflashReadBleBrocastName(extflash_para_t *para)
{
	uint8 dataTemp[BROCAST_NAME_ID_LEN];
	uint16 i;
    
    Drv_Extflash_SendCmd(FLASH_WAKEUP_CMD);
	para->result |= IdleWait(EXTFLASH_READ_WRITE_IDELTICKS);

	Drv_Extflash_Read(dataTemp, BROCAST_NAME_ID_ADDR, BROCAST_NAME_ID_LEN);
	para->result |= IdleWait(EXTFLASH_READ_WRITE_IDELTICKS);
	for(i = 0; i < BROCAST_NAME_ID_LEN; i++)
	{
		if(BROCAST_NAME_ID[i] != dataTemp[i])
		{
			para->result	|= 0x01;
			break;
		}
	}
	if(para->result == 0)
	{
		
		Drv_Extflash_Read(para->dataAddr, BROCAST_NAME_INFO_ADDR, para->length);
		para->result |= IdleWait(EXTFLASH_READ_WRITE_IDELTICKS);
	}
	Drv_Extflash_SendCmd(FLASH_SLEEP_CMD);

	return 0;
}

//**********************************************************************
// 函数功能:	读8M flash UID码
// 输入参数：	
// para 	: 	para->dataAddr存放UID码信息
// 返回参数：	
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
static uint16 ExtflashReadMac(extflash_para_t *para)
{
	uint8 i;
	uint8  *dataPoint;

    Drv_Extflash_SendCmd(FLASH_WAKEUP_CMD);
	dataPoint = Drv_Extflash_ReadUid();
	for(i = 0; i < 6; i++)
	{
		para->dataAddr[i] = dataPoint[i];
	}
	Drv_Extflash_SendCmd(FLASH_SLEEP_CMD);
	return 0;
}

//**********************************************************************
// 函数功能:	8M flash睡眠
// 输入参数：	无
// 返回参数：	
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
static uint16 ExtflashSleep(void)
{
	return Drv_Extflash_SendCmd(FLASH_SLEEP_CMD);
}

//**********************************************************************
// 函数功能:	8M flash唤醒
// 输入参数：	无
// 返回参数：	
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
static uint16 ExtflashWakeUp(void)
{
	return Drv_Extflash_SendCmd(FLASH_WAKEUP_CMD);

}

//**********************************************************************
// 函数功能:	8M flash块擦除，4KB/块
// 输入参数：	
// para 	: 	包含擦除的起始地址、结尾地址等信息
// 返回参数：	
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
static uint16 ExtflashErase_4K(extflash_para_t *para)
{
	uint32 i, startSector, endSector;
    
	Drv_Extflash_SendCmd(FLASH_WAKEUP_CMD);
	startSector = para->startAddr/4096;
	endSector   = para->endAddr/4096;	
	for(i = startSector; i <= endSector; i++)
	{
		para->result |= IdleWait(EXTFLASH_ERASE_IDELTICKS);
		Drv_Extflash_Erase4K(i << 12);
	}
	para->result |= IdleWait(EXTFLASH_ERASE_IDELTICKS);
	Drv_Extflash_SendCmd(FLASH_SLEEP_CMD);

	return 0;
}

//**********************************************************************
// 函数功能:	8M flash写dataAddr数据，一次性最大写单位为页，256B/页
// 输入参数：	
// para 	: 	写数据的起始地址、结尾地址、数据长度、数据指针等信息
// 返回参数：	
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
static uint16 ExtflashWrite(extflash_para_t *para)
{
	uint32 lengthTemp, lengthTemp2;
    
    Drv_Extflash_SendCmd(FLASH_WAKEUP_CMD);
	// 已写入长度
	lengthTemp = 0;

	// 循环写，每次最多写入256字节，不能跨页
	while(lengthTemp < para->length)
	{
		lengthTemp2 = 256 - ((lengthTemp + para->startAddr) % 256);
		// if exceed the length, 
		if((para->startAddr + lengthTemp + lengthTemp2) > para->endAddr)
		{
			lengthTemp2	= para->endAddr - (para->startAddr + lengthTemp) + 1;	//
		}
		para->result |= IdleWait(EXTFLASH_READ_WRITE_IDELTICKS);
		Drv_Extflash_Write(para->dataAddr + lengthTemp, lengthTemp + para->startAddr, lengthTemp2);
		lengthTemp += lengthTemp2;
	}
	para->result |= IdleWait(EXTFLASH_READ_WRITE_IDELTICKS);
	Drv_Extflash_SendCmd(FLASH_SLEEP_CMD);

	return 0;
}

//**********************************************************************
// 函数功能:	8M flash写data[16]数组数据，一次性最大写单位为页，256B/页
// 输入参数：	
// para 	: 	写数据的起始地址、结尾地址、数据长度、数据指针等信息
// 返回参数：	
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
static uint16 ExtflashWriteWithBuf(extflash_para_t *para)
{
	uint32 lengthTemp, lengthTemp2;
    
    Drv_Extflash_SendCmd(FLASH_WAKEUP_CMD);
	// 已写入长度
	lengthTemp = 0;

	// 循环写，每次最多写入256字节，不能跨页
	while(lengthTemp < para->length)
	{
		lengthTemp2 = 256 - ((lengthTemp + para->startAddr) % 256);
		// if exceed the length, 
		if((para->startAddr + lengthTemp + lengthTemp2) > para->endAddr)
		{
			lengthTemp2	= para->endAddr - (para->startAddr + lengthTemp) + 1;	//
		}
		para->result |= IdleWait(EXTFLASH_READ_WRITE_IDELTICKS);
		Drv_Extflash_Write(para->data + lengthTemp, lengthTemp + para->startAddr, lengthTemp2);
		lengthTemp += lengthTemp2;
	}
	para->result |= IdleWait(EXTFLASH_READ_WRITE_IDELTICKS);
	Drv_Extflash_SendCmd(FLASH_SLEEP_CMD);

	return 0;
}

//**********************************************************************
// 函数功能:	8M flash读，每次最大读2048byts（硬件SPI每次写最长4095B）,
// 输入参数：	
// para 	: 	读数据的起始地址、结尾地址、数据长度、数据指针等信息
// 返回参数：	
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
static uint16 ExtflashRead(extflash_para_t *para)
{
	uint32 lengthTemp, lengthTemp2;
    
    Drv_Extflash_SendCmd(FLASH_WAKEUP_CMD);
	lengthTemp = 0;
	while(lengthTemp < para->length)
	{
		lengthTemp2 = 2048 - ((lengthTemp + para->startAddr) % 2048);
		// if exceed the length, 
		if((para->startAddr + lengthTemp + lengthTemp2) > para->endAddr)
		{
			lengthTemp2	= para->endAddr - (para->startAddr + lengthTemp) + 1;	//
		}
		para->result |= IdleWait(EXTFLASH_READ_WRITE_IDELTICKS);
		Drv_Extflash_Read(para->dataAddr + lengthTemp, lengthTemp + para->startAddr, lengthTemp2);
		lengthTemp += lengthTemp2;
	}
	Drv_Extflash_SendCmd(FLASH_SLEEP_CMD);

	return 0;
}

//**********************************************************************
// 函数功能:	写蓝牙配对passkey
// 输入参数：	
// para 	: 	para->dataAddr存放ble passkey信息
// 返回参数：	
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
static uint16 ExtflashWriteBlePassKey(extflash_para_t *para)
{   
    //step 1: erase 4k flash
    Drv_Extflash_SendCmd(FLASH_WAKEUP_CMD);
	Drv_Extflash_Erase4K(BLE_PAIRING_PASSKEY_ADDR);
    Drv_Extflash_SendCmd(FLASH_SLEEP_CMD);

    //step 2: write data
    para->startAddr = BLE_PAIRING_PASSKEY_ADDR;
    para->endAddr = para->startAddr + para->length;
    ExtflashWrite(para);
	return 0;
}

//**********************************************************************
// 函数功能:	读蓝牙配对passkey
// 输入参数：	
// para 	: 	para->dataAddr存放ble passkey信息
// 返回参数：	
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
static uint16 ExtflashReadBlePassKey(extflash_para_t *para)
{
return 0;
    para->startAddr = BLE_PAIRING_PASSKEY_ADDR;
    para->endAddr = para->startAddr + para->length;
    ExtflashRead(para);
	return 0;
}

//**********************************************************************
// 函数功能:	flash函数回调,同步二值量
// 输入参数：	
// queueId 	:	指定二值同步信号id	
// 返回参数：	无
//**********************************************************************
static void ExtFlashCb(uint8* queueId, uint8 result)
{
	if(*queueId < EXTFLASH_CB_ID_MAX)
	{
		extflashSemaphore[*queueId].result		= result;
		xSemaphoreGive(extflashSemaphore[*queueId].extflashCbSemaphore);//释放二值信号量，解锁任务（解除queueId对exflash任务的阻塞）
	}
}

//**********************************************************************
// 函数功能:	8Mflash 软件初始化，队列创建
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
void Mid_ExtFlash_Init(void)
{
	static	uint8 extflashInitFlag = 0;
	static	uint8 i;
	if(extflashInitFlag == 0)
	{
		for(i = 0; i < EXTFLASH_CB_ID_MAX; i++)
		{
			extflashSemaphore[i].occupy					= 0;
			//创建二值信号量,the semaphore must first be 'given' before it can be 'taken'
			extflashSemaphore[i].extflashCbSemaphore	= xSemaphoreCreateBinary();
		}

		// 创建flash的二值量资源互斥保护
		extflashMutex   = xSemaphoreCreateMutex();//追寻互斥信号量

		extflashInitFlag	= 1;
	}

	ExtflashEnable();
	Drv_Extflash_SendCmd(FLASH_SLEEP_CMD);
	ExtflashDisable();
}

//**********************************************************************
// 函数功能:  extflash事件放入队列前预处理
// 输入参数：	
// msg 	:	  事件指针	
// 返回参数：  0x00:操作成功
// 			   0xff:操作失败
//**********************************************************************
uint16 Mid_ExtFlash_EventSetPre(extflash_event_t *msg)
{
	uint8 i;

    switch(msg->id)
    {
    case EXTFLASH_EVENT_ENABLE:
    case EXTFLASH_EVENT_DISABLE:
    case EXTFLASH_EVENT_SELFTEST:
    case EXTFLASH_EVENT_WRITE_SN:
    case EXTFLASH_EVENT_READ_SN:
    case EXTFLASH_EVENT_WRITE_BLEMAC:
    case EXTFLASH_EVENT_READ_BLEMAC:
    case EXTFLASH_EVENT_READ_MAC:
    case EXTFLASH_EVENT_SLEEP:
    case EXTFLASH_EVENT_WAKEUP:
    case EXTFLASH_EVENT_4K_ERASE:
    case EXTFLASH_EVENT_READ:
    case EXTFLASH_EVENT_WRITE:
    case EXTFLASH_EVENT_WRITE_BLE_BROCAST:
    case EXTFLASH_EVENT_READ_BLE_BROCAST:
    case EXTFLASH_EVENT_WRITE_SN_CUSTOM:
    case EXTFLASH_EVENT_READ_SN_CUSTOM:
    case EXTFLASH_EVENT_WRITE_BLE_PASSKEY:   //ble passkey
	case EXTFLASH_EVENT_READ_BLE_PASSKEY:
		xSemaphoreTake(extflashMutex, portMAX_DELAY);//阻塞获取互斥信号量，如互斥信号量有效，则无阻塞等待，获取后其它任务或事件不可见
		for(i = 0; i < EXTFLASH_CB_ID_MAX; i++)
		{
			if(extflashSemaphore[i].occupy == 0)
			{
				extflashSemaphore[i].occupy = 1;	
				break;
			}
		}

		if(i < EXTFLASH_CB_ID_MAX)
		{
			msg->para.cbId		= i;//Id记录
			xSemaphoreGive(extflashMutex);//释放互斥信号量（归还）
			msg->para.Cb		= ExtFlashCb;
			return 0x00;
		}

		msg->para.Cb		= NULL;
		msg->para.cbId		= 0xff;
		xSemaphoreGive(extflashMutex);
		break;
    case EXTFLASH_EVENT_WRITE_WITH_BUF:   // 该事件不需要等待，在任务中执行
        msg->para.Cb		= NULL;
        return 0x00;
    default:
		return 0xff;
	}
	// 检测是否有空闲的二值量
	return 0xff;
}

//**********************************************************************
// 函数功能:	任务队列放置不进时，需要进行清除二值资源，对其解除占用
// 输入参数：	
// msg 	:		事件指针	
// 返回参数：	0x00:操作成功
// 				0xff:操作失败
//**********************************************************************
uint16 Mid_ExtFlash_EventSetFail(extflash_event_t *msg)
{
    switch(msg->id)
    {
    case EXTFLASH_EVENT_ENABLE:
    case EXTFLASH_EVENT_DISABLE:
    case EXTFLASH_EVENT_SELFTEST:
    case EXTFLASH_EVENT_WRITE_SN:
    case EXTFLASH_EVENT_READ_SN:
    case EXTFLASH_EVENT_WRITE_BLEMAC:
    case EXTFLASH_EVENT_READ_BLEMAC:
    case EXTFLASH_EVENT_READ_MAC:
    case EXTFLASH_EVENT_SLEEP:
    case EXTFLASH_EVENT_WAKEUP:
    case EXTFLASH_EVENT_4K_ERASE:
    case EXTFLASH_EVENT_READ:
    case EXTFLASH_EVENT_WRITE:
    case EXTFLASH_EVENT_WRITE_BLE_BROCAST:
    case EXTFLASH_EVENT_READ_BLE_BROCAST:
    case EXTFLASH_EVENT_WRITE_SN_CUSTOM:
    case EXTFLASH_EVENT_READ_SN_CUSTOM:
    case EXTFLASH_EVENT_WRITE_BLE_PASSKEY:   //ble passkey
    case EXTFLASH_EVENT_READ_BLE_PASSKEY:
        xSemaphoreTake(extflashMutex, portMAX_DELAY);
        extflashSemaphore[msg->para.cbId].occupy = 0;	
        xSemaphoreGive(extflashMutex);
        break;
    // 该事件不需要等待，在任务中执行
    case EXTFLASH_EVENT_WRITE_WITH_BUF:
		break;
    default:
		return 0xff;
	}
	return 0x00;
}

//**********************************************************************
// 函数功能:  等待时间在flash任务中执行结束
// 输入参数：	
// queueId :  指定二值同步信号id	
// 返回参数： 0x00:操作成功
// 			  0xff:操作失败
//**********************************************************************
uint16 Mid_ExtFlash_WaitComplete(extflash_event_t *msg)
{
	if(msg->para.cbId >= EXTFLASH_CB_ID_MAX)
		return 0xff;

    switch(msg->id)
	{
    case EXTFLASH_EVENT_ENABLE:
    case EXTFLASH_EVENT_DISABLE:
    case EXTFLASH_EVENT_SELFTEST:
    case EXTFLASH_EVENT_WRITE_SN:
    case EXTFLASH_EVENT_READ_SN:
    case EXTFLASH_EVENT_WRITE_BLEMAC:
    case EXTFLASH_EVENT_READ_BLEMAC:
    case EXTFLASH_EVENT_READ_MAC:
    case EXTFLASH_EVENT_SLEEP:
    case EXTFLASH_EVENT_WAKEUP:
    case EXTFLASH_EVENT_4K_ERASE:
    case EXTFLASH_EVENT_READ:
    case EXTFLASH_EVENT_WRITE:
    case EXTFLASH_EVENT_WRITE_BLE_BROCAST:
    case EXTFLASH_EVENT_READ_BLE_BROCAST:
    case EXTFLASH_EVENT_WRITE_SN_CUSTOM:
    case EXTFLASH_EVENT_READ_SN_CUSTOM:
    case EXTFLASH_EVENT_WRITE_BLE_PASSKEY:   //ble passkey
    case EXTFLASH_EVENT_READ_BLE_PASSKEY:
		// 等待事件放入队列处理完，在回调中释放该Id的二值信号量（即事件处理完），以解除该Id事件对任务的阻塞
		xSemaphoreTake(extflashSemaphore[msg->para.cbId].extflashCbSemaphore, portMAX_DELAY);
		xSemaphoreTake(extflashMutex, portMAX_DELAY);
		extflashSemaphore[msg->para.cbId].occupy = 0;
		xSemaphoreGive(extflashMutex);
		break;		
    case EXTFLASH_EVENT_WRITE_WITH_BUF:   // 该事件不需要等待，在任务中执行
		return 0x00;
    default:
		return 0xff;
	}

	return extflashSemaphore[msg->para.cbId].result;
}

//**********************************************************************
// 函数功能:  获取BLE 信息，如获取BLE MAC ADDR,ADV name
// 输入参数：	
// 返回参数： 0x00:操作成功
// 			  0xff:操作失败
//**********************************************************************
uint16 Mid_ExtFlash_ReadBleInfo(extflash_event_id event,extflash_para_t *para)
{
    return 0;

    ExtflashEnable();

    if(event == EXTFLASH_EVENT_READ_BLEMAC)
    {
		ExtflashReadBleMac(para);
    }
    else if(event == EXTFLASH_EVENT_READ_MAC)
    {
		ExtflashReadMac(para);
    }
    else if(event == EXTFLASH_EVENT_READ_BLE_BROCAST)
    {
		ExtflashReadBleBrocastName(para);
    }
    else if(event == EXTFLASH_EVENT_READ_BLE_PASSKEY)
    {
        ExtflashReadBlePassKey(para);
    }
    else
        ;
    ExtflashDisable();
    return 0;
}

//**********************************************************************
// 函数功能:  根据事件ID进行处理
// 输入参数：	
// queueId :  指定二值同步信号id	
// 返回参数： 0x00:操作成功
// 		      0xff:操作失败
//**********************************************************************
uint16 Mid_ExtFlash_EventProcess(extflash_event_t *msg)
{
	msg->para.result		= 0x00;
    switch(msg->id)
	{
    case EXTFLASH_EVENT_ENABLE:
		ExtflashEnable();
		break;
    case EXTFLASH_EVENT_DISABLE:
		ExtflashDisable();
		break;
    case EXTFLASH_EVENT_SELFTEST:
		ExtflashEnable();
		msg->para.result = ExtflashSelfTest();
		ExtflashDisable();
		break;
    case EXTFLASH_EVENT_WRITE_SN:
		ExtflashEnable();
		ExtflashWriteSN(&msg->para);
		ExtflashDisable();
		break;
    case EXTFLASH_EVENT_READ_SN:
		ExtflashEnable();
		ExtflashReadSN(&msg->para);
		ExtflashDisable();
		break;
    case EXTFLASH_EVENT_WRITE_BLEMAC:
		ExtflashEnable();
		ExtflashWriteBleMac(&msg->para);
		ExtflashDisable();
		break;
    case EXTFLASH_EVENT_READ_BLEMAC:
		ExtflashEnable();
		ExtflashReadBleMac(&msg->para);
		ExtflashDisable();
		break;
    case EXTFLASH_EVENT_READ_MAC:
		ExtflashEnable();
		ExtflashReadMac(&msg->para);
		ExtflashDisable();
		break;
    case EXTFLASH_EVENT_SLEEP:
		ExtflashEnable();
		ExtflashSleep();
		ExtflashDisable();
		break;				
    case EXTFLASH_EVENT_WAKEUP:
		ExtflashEnable();
		ExtflashWakeUp();
		ExtflashDisable();
		break;
    case EXTFLASH_EVENT_4K_ERASE:
		ExtflashEnable();
		ExtflashErase_4K(&msg->para);
		ExtflashDisable();
		break;			
    case EXTFLASH_EVENT_READ:
		ExtflashEnable();
		ExtflashRead(&msg->para);
		ExtflashDisable();
		break;				
    case EXTFLASH_EVENT_WRITE:
		ExtflashEnable();
		ExtflashWrite(&msg->para);
		ExtflashDisable();
		break;
    case EXTFLASH_EVENT_WRITE_WITH_BUF:
		ExtflashEnable();
		ExtflashWriteWithBuf(&msg->para);
		ExtflashDisable();
		break;
    case EXTFLASH_EVENT_WRITE_BLE_BROCAST:
		ExtflashEnable();
		ExtflashWriteBleBrocastName(&msg->para);
		ExtflashDisable();
		break;
    case EXTFLASH_EVENT_READ_BLE_BROCAST:
		ExtflashEnable();
		ExtflashReadBleBrocastName(&msg->para);
		ExtflashDisable();
		break;
    case EXTFLASH_EVENT_WRITE_SN_CUSTOM:
		ExtflashEnable();
		ExtflashWriteSN_Custom(&msg->para);
		ExtflashDisable();
		break;
    case EXTFLASH_EVENT_READ_SN_CUSTOM:
		ExtflashEnable();
		ExtflashReadSN_Custom(&msg->para);
		ExtflashDisable();
		break;
    case EXTFLASH_EVENT_WRITE_BLE_PASSKEY:   //ble passkey
		ExtflashEnable();
        ExtflashWriteBlePassKey(&msg->para);
		ExtflashDisable();
        break;
    case EXTFLASH_EVENT_READ_BLE_PASSKEY:
		ExtflashEnable();
        ExtflashReadBlePassKey(&msg->para);
		ExtflashDisable();
        break;
    default:
		msg->para.result = 0xFF;
		break;
	}
	if(msg->para.Cb != NULL)
	{
		msg->para.Cb(&msg->para.cbId, msg->para.result);
	}
	return 0;
}

