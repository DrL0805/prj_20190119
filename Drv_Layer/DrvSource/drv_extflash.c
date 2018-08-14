/**********************************************************************
**
**模块说明: 外接flash驱动接口
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.19  修改流程  ZSL  
**
**********************************************************************/
#define FLASH_MODULE
#include "io_config.h"

#if(FLASH_CS_PIN != IO_UNKNOW)
#include "sm_gpio.h"
#include "sm_spi.h"
#include "drv_extflash.h"

//spi max buffer length
#define SPI_BUFFER_MAX_SIZE         4095

// command
#define FLASH_WAKE_UP               (0xAB)
#define FLASH_SLEEP                 (0xB9)
#define FLASH_READ_UID              (0x4B)
#define FLASH_WRITE_ENABLE          (0x06)
#define FLASH_READ_STATU_REG1       (0x05)
#define FLASH_READ_DATA             (0x03)
#define FLASH_PAGE_PROGRAM          (0x02)
#define FLASH_4K_ERASE              (0x20)

//address
#define	FACTORY_SELFTEST_START_ADDR	 1036288				//252 section

// statue
#define	EXTFLASH_OK                   0X00
#define	EXTFLASH_ERROR                0XFF
#define	EXTFLASH_BUSY                 0X01

static uint8 flashUid[8]={0x00};

//**********************************************************************
// 函数功能: 使能flash片选
// 输入参数：	
// 返回参数：无
//**********************************************************************
static void FlashCs_Enable(void)
{
    SMDrv_GPIO_BitClear(FLASH_CS_PIN);
}

//**********************************************************************
// 函数功能: 禁止片选
// 输入参数：	
// 返回参数：无
//**********************************************************************
static void FlashCs_Disable(void)
{
    SMDrv_GPIO_BitSet(FLASH_CS_PIN);
}

//**********************************************************************
// 函数功能: flash写数据
// 输入参数：
// *data   :  数据指针
// lenght  ： 数据长度
// 返回参数： 无
//**********************************************************************
static void FlashWriteData(uint8 *data, uint16 lenght)
{
    uint16 i;
    uint16 cnt;
    uint16 num;
    uint16 lenght_temp;
	
    //spi write at most 4095 bytes one time
    if(lenght > SPI_BUFFER_MAX_SIZE)
    {
        num = lenght / SPI_BUFFER_MAX_SIZE;
        cnt = 0;
        lenght_temp = lenght % SPI_BUFFER_MAX_SIZE;

        for(i = 0;i < num; i ++)
        {
            cnt = i * SPI_BUFFER_MAX_SIZE;
            SMDrv_SPI_WriteBytes(FLASH_SPI_MODULE,&data[cnt],SPI_BUFFER_MAX_SIZE);		
        }

        cnt = i * SPI_BUFFER_MAX_SIZE;
        SMDrv_SPI_WriteBytes(FLASH_SPI_MODULE,&data[cnt],lenght_temp);		
    }
    else
    {
        SMDrv_SPI_WriteBytes(FLASH_SPI_MODULE,data,lenght);
    }
}

//**********************************************************************
// 函数功能:	flash读数据
// 输入参数：	
// *pData  ：	数据指针
// length  ： 	读的字节数
// 返回参数：	无
//**********************************************************************
static void FlashReadData(uint8 *data, uint32 lenght)
{
    uint16 i;
    uint16 cnt;
    uint16 num;
    uint16 lenght_temp;
	
    //spi write at most 4095 bytes one time
    if(lenght > SPI_BUFFER_MAX_SIZE)
    {
        num 		= lenght / SPI_BUFFER_MAX_SIZE;
        cnt 		= 0;
        lenght_temp = lenght % SPI_BUFFER_MAX_SIZE;

        for(i = 0;i < num; i ++)
        {
            cnt = i * SPI_BUFFER_MAX_SIZE;
            SMDrv_SPI_ReadBytes(FLASH_SPI_MODULE,&data[cnt],SPI_BUFFER_MAX_SIZE);			
        }

        cnt = i * SPI_BUFFER_MAX_SIZE;
        SMDrv_SPI_ReadBytes(FLASH_SPI_MODULE,&data[cnt],lenght_temp);			
    }
    else
    {
        SMDrv_SPI_ReadBytes(FLASH_SPI_MODULE,data,lenght);
    }	
}
#endif

//**********************************************************************
// 函数功能:	Flash硬件初始化
// 输入参数：	无
// 返回参数：	
// 0x00    :    初始化成功
// 0x02    :    参数错误
//**********************************************************************
uint8 Drv_Extflash_Open(void)
{
#if(FLASH_CS_PIN != IO_UNKNOW)
    ret_type u8ret;
	
	    //set flash cs pin as high, and open it
    if((u8ret = SMDrv_GPIO_BitSet(FLASH_CS_PIN)) != Ret_OK)
        return u8ret;
    if((u8ret = SMDrv_GPIO_Open(FLASH_CS_PIN,NULL,NULL)) != Ret_OK)
        return u8ret;
	
    //open flash spi
    if((u8ret = SMDrv_SPI_Open(FLASH_SPI_MODULE)) != Ret_OK)
        return u8ret;

#endif
	return Ret_OK;
}

//**********************************************************************
// 函数功能:	Flash硬件待机低功耗配置
// 输入参数：	无
// 返回参数：	
// 0x00    :    设置成功
// 0x02    :    参数错误
//**********************************************************************
uint8 Drv_Extflash_Close(void)
{
#if(FLASH_CS_PIN != IO_UNKNOW)
    ret_type u8ret;

    if((u8ret = SMDrv_SPI_Close(FLASH_SPI_MODULE)) != Ret_OK)
        return u8ret;

    #if 0
    //flash cs pin config
    if((u8ret = SMDrv_GPIO_BitSet(FLASH_CS_PIN)) != Ret_OK)
        return u8ret;
    #endif
#endif
    return Ret_OK;
}

//**********************************************************************
// 函数功能：    读取8MFlash的uid[8]，并返回uid。
// 输入参数：    void
// 返回参数：	uid数组的指针
//**********************************************************************
uint8 *Drv_Extflash_ReadUid(void)
{
#if(FLASH_CS_PIN != IO_UNKNOW)
	uint8 *uid=flashUid;
	FlashCs_Enable();
	SMDrv_SPI_WriteByte(FLASH_SPI_MODULE,FLASH_READ_UID);
	SMDrv_SPI_WriteBytes(FLASH_SPI_MODULE,uid, 4);
	SMDrv_SPI_ReadBytes(FLASH_SPI_MODULE,uid, 8);
	FlashCs_Disable();
    return uid;
#else
    return NULL;
#endif
}

//**********************************************************************
// 函数功能：向flash发送命令
// 输入参数：
//    ft_cmd = FLASH_WAKEUP_CMD: 唤醒指令
//    ft_cmd = FLASH_SLEEP_CMD:  睡眠指令
// 返回参数：	0x00:操作成功
//              0x02:参数错误
//**********************************************************************
uint8 Drv_Extflash_SendCmd(flash_cmd fs_cmd)
{
#if(FLASH_CS_PIN != IO_UNKNOW)
    uint8 u8Cmd;

    FlashCs_Enable();
    if(fs_cmd == FLASH_WAKEUP_CMD)
        u8Cmd = FLASH_WAKE_UP;
    else if(fs_cmd == FLASH_SLEEP_CMD)
        u8Cmd = FLASH_SLEEP;
    else
        return Ret_InvalidParam;
    SMDrv_SPI_WriteByte(FLASH_SPI_MODULE,u8Cmd);
    FlashCs_Disable();
#endif
    return Ret_OK;
}

//**********************************************************************
// 函数功能：   flash空闲状态检测，并返回状态结果。
// 输入参数：    void
// 返回参数：	状态
//              0x00:空闲
//              0x03:忙
//**********************************************************************
uint8 Drv_Extflash_CheckState(void)
{
#if(FLASH_CS_PIN != IO_UNKNOW)
    uint8 statue;

    FlashCs_Enable();
    SMDrv_SPI_WriteByte(FLASH_SPI_MODULE,FLASH_READ_STATU_REG1);
    SMDrv_SPI_ReadBytes(FLASH_SPI_MODULE,&statue, 1);
    FlashCs_Disable();

    if(statue & EXTFLASH_BUSY)
        return Ret_DeviceBusy;
#endif
    return Ret_OK;
}

//**********************************************************************
// 函数功能：    flash读，并操作结果。
// 输入参数：    
//              pdata:  存储读取到的数据buf
//              addr:   要读取的Flash地址
//              length: 读取数据的长度
// 返回参数：	0x00:操作成功
// 				0xFF:操作失败
//**********************************************************************
uint8 Drv_Extflash_Read(uint8* pdata, uint32 addr, uint16 length)
{
#if(FLASH_CS_PIN != IO_UNKNOW)
    uint8 data[3];
    data[2] = (uint8)addr;
    data[1] = (uint8)(addr >> 8);
    data[0] = (uint8)(addr >> 16);

    FlashCs_Enable();
    SMDrv_SPI_WriteByte(FLASH_SPI_MODULE,FLASH_READ_DATA);
    SMDrv_SPI_WriteBytes(FLASH_SPI_MODULE,data, 3);
    FlashReadData(pdata,length);
    FlashCs_Disable();
#endif
    return Ret_OK;
}

//**********************************************************************
// 函数功能：    flash写，并操作结果。
// 输入参数：    
//              pdata:  要写入的数据buf
//              addr:   要写入的Flash地址
//              length: 写入数据的长度
// 返回参数：	0x00:操作成功
// 				0xFF:操作失败
//**********************************************************************
uint8 Drv_Extflash_Write(uint8* pdata, uint32 addr, uint16 length)
{
#if(FLASH_CS_PIN != IO_UNKNOW)
    uint8 data[3];
    data[2] = (uint8)addr;
    data[1] = (uint8)(addr >> 8);
    data[0] = (uint8)(addr >> 16);

    FlashCs_Enable();
    SMDrv_SPI_WriteByte(FLASH_SPI_MODULE,FLASH_WRITE_ENABLE);
    FlashCs_Disable();

    FlashCs_Enable();
    SMDrv_SPI_WriteByte(FLASH_SPI_MODULE,FLASH_PAGE_PROGRAM);
    SMDrv_SPI_WriteBytes(FLASH_SPI_MODULE,data, 3);
    FlashWriteData(pdata,length);
    FlashCs_Disable();
#endif
    return Ret_OK;
}

//**********************************************************************
// 函数功能：   Flash模块擦除某个扇区，并返回操作结果。
// 输入参数：    
//     startSectorAddr:  扇区起始地址
// 返回参数：	0x00:操作成功
// 				0xFF:操作失败
//**********************************************************************
uint8 Drv_Extflash_Erase4K(uint32 startSectorAddr)
{
#if(FLASH_CS_PIN != IO_UNKNOW)
    uint8 data[3];
    data[2]	= (uint8)startSectorAddr;
    data[1]	= (uint8)(startSectorAddr >> 8);
    data[0]	= (uint8)(startSectorAddr >> 16);

    FlashCs_Enable();
    SMDrv_SPI_WriteByte(FLASH_SPI_MODULE,FLASH_WRITE_ENABLE);
    FlashCs_Disable();

    FlashCs_Enable();
    SMDrv_SPI_WriteByte(FLASH_SPI_MODULE,FLASH_4K_ERASE);
    SMDrv_SPI_WriteBytes(FLASH_SPI_MODULE,data, 3);
    FlashCs_Disable();
#endif
    return Ret_OK;
}

