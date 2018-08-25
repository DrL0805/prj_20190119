/**********************************************************************
**
**模块说明: 
**   
**
**********************************************************************/

#include "io_config.h"
#include "sm_gpio.h"
#include "sm_spi.h"
#include "am_util_delay.h"

#include "drv_nandflash.h"

/*******************macro define*******************/
#define		NANDFLASH_SPI_CS_H()		am_hal_gpio_output_set(NAND_FLASH_CS_PIN)
#define		NANDFLASH_SPI_CS_L()		am_hal_gpio_output_clear(NAND_FLASH_CS_PIN)

//spi max buffer length
#define 	SPI_BUFFER_MAX_SIZE 					4095

// nandflash command define
//feature
#define 	NANDFLASH_RESET                    		 (0XFF)
#define 	NANDFLASH_GET_FEATURES                   (0X0F)
#define 	NANDFLASH_SET_FEATURES                   (0X1F)
//read
#define 	NANDFLASH_READ_ID                    	 (0X9F)
#define 	NANDFLASH_PAGE_READ                  	 (0X13)
#define 	NANDFLASH_READ_FROM_CACHE                (0X03)//(0X0B)
#define 	NANDFLASH_READ_PAGE_CACHE                (0X30)
#define 	NANDFLASH_READ_PAGE_CACHE_LAST           (0X3F)
//write
#define 	NANDFLASH_WRITE_ENABLE                   (0X06)
#define 	NANDFLASH_WRITE_DISABLE                  (0X04)
#define 	NANDFLASH_PROGRAM_EXECUTE                (0X10)
#define 	NANDFLASH_PROGRAM_LOAD                   (0X02)
#define 	NANDFLASH_LOAD_RANDOM_DATA               (0X84)//faster tha 0x02
//erase
#define 	NANDFLASH_BLOCK_ERASE                    (0XD8)
//feature->address
#define		NANDFLASH_BLOCK_LOCK					0XA0
#define		NANDFLASH_BLOCK_CONFIG					0XB0
#define		NANDFLASH_STATUS						0XC0
#define		NANDFLASH_DIE_SELECT					0XD0
//feature->address->value
//die select
#define		DIE_SELECT_DIE0							0X00
#define		DIE_SELECT_DIE1							0X40

//ID value
#define		MANUFACTURER_ID				   		 	0X2C
#define		DEVICE_ID					    		0X36
// dummy address
#define 	DUMMY(n) 								(0Xffffffff>>(32-n))	// 低n位置一，其他清零
#define 	SET(n) 									(1<<n)
#define 	CLEAR(n) 								(~(1<<n))

#define 	GET_BIT(value, n)		((value >> n) & 0x01)	// 获取变量某bit值

#define		READ_WRITE_IDELTICKS					(20)		// 1ms
#define		ERASE_IDELTICKS							(100)		// 5ms
#define		RESET_IDELTICKS							(40)		// 2ms
#define		Drv_NandFlash_DelayTick()	am_util_delay_us(50)

//reflag current die, die0 in default
static uint8_t 	CurrentDie  = 0; //0:die0; 1:die1;

//**********************************************************************
// 函数功能:	NandFlash硬件SPI打开
// 输入参数：	无
// 返回参数：	
uint8 Drv_NandFlash_Open(void)
{
#if(NAND_FLASH_CS_PIN != IO_UNKNOW)
    ret_type u8ret;
    uint32 u32font_conf = GPIO_PIN_OUTPUT; //default set pin as output

    //set flash cs pin as high, and open it
    if((u8ret = SMDrv_GPIO_BitSet(NAND_FLASH_CS_PIN)) != Ret_OK)
        return u8ret;
    if((u8ret = SMDrv_GPIO_Open(NAND_FLASH_CS_PIN,&u32font_conf,NULL)) != Ret_OK)
        return u8ret;
    
    //open flash spi
    if((u8ret = SMDrv_SPI_Open(FLASH_SPI_MODULE)) != Ret_OK)
        return u8ret;
#endif
	
	return Ret_OK;
}

//**********************************************************************
// 函数功能:	NandFlash硬件SPI关闭
// 输入参数：	无
// 返回参数：
uint8 Drv_NandFlash_Close(void)
{
#if(NAND_FLASH_CS_PIN != IO_UNKNOW)
    ret_type u8ret;

    if((u8ret = SMDrv_SPI_Close(FLASH_SPI_MODULE)) != Ret_OK)
        return u8ret;
#endif
	
    return Ret_OK;
}

//**********************************************************************
// 函数功能:	NandFlash SPI 写1字节
// 输入参数：	无
// 返回参数：
static void Drv_Nandflash_WriteByte(uint8_t val)
{
	SMDrv_SPI_WriteByte(FLASH_SPI_MODULE,val);
}

//**********************************************************************
// 函数功能:	NandFlash SPI 写N字节
// 输入参数：	无
// *pData  ：	数据指针
// length  ： 	写的字节数
// 返回参数：
static void Drv_Nandflash_WriteBytes(uint8_t* pData, uint16_t length)
{
	SMDrv_SPI_WriteBytes(FLASH_SPI_MODULE,pData,length);
}

//**********************************************************************
// 函数功能:	NandFlash SPI 读1字节
// 输入参数：	无
// 返回参数：	SPI读取的数据
static uint8_t Drv_Nandflash_ReadByte(void)
{
	uint8_t val=0;
	
	SMDrv_SPI_ReadBytes(FLASH_SPI_MODULE, &val, 1);
	
	return	val;
}

//**********************************************************************
// 函数功能:	SPI读N字节
// 输入参数：	
// *pData  ：	数据指针
// length  ： 	读的字节数
// 返回参数：	无
static void Drv_Nandflash_ReadBytes(uint8_t* pData, uint16_t length)
{
	SMDrv_SPI_ReadBytes(FLASH_SPI_MODULE, pData, length);
}

//**********************************************************************
// 函数功能:	nandflash写数据
// 输入参数：	
// *data   :    数据指针
// lenght  ：   数据长度
// 返回参数：	无
static void Drv_NandFlash_WriteData(uint8_t *data, uint16_t lenght)
{
	uint16_t i;
	uint16_t cnt;
	uint16_t num;
	uint16_t lenght_temp;
	
	//spi write at most 4095 bytes one time
	if(lenght > SPI_BUFFER_MAX_SIZE)
	{
		num 		= lenght / SPI_BUFFER_MAX_SIZE;
		cnt 		= 0;
		lenght_temp = lenght % SPI_BUFFER_MAX_SIZE;

		for(i = 0;i < num; i ++)
		{
			cnt = i * SPI_BUFFER_MAX_SIZE;
			Drv_Nandflash_WriteBytes(&data[cnt],SPI_BUFFER_MAX_SIZE);			
		}

		cnt 		= i * SPI_BUFFER_MAX_SIZE;
		Drv_Nandflash_WriteBytes(&data[cnt],lenght_temp);			
	}
	else
	{
		Drv_Nandflash_WriteBytes(data,lenght);
	}	
}

//**********************************************************************
// 函数功能:	nandflash读数据
// 输入参数：	
// *pData  ：	数据指针
// length  ： 	读的字节数
// 返回参数：	无
static void Drv_NandFlash_ReadData(uint8_t *data, uint32_t lenght)
{
	uint16_t i;
	uint16_t cnt;
	uint16_t num;
	uint16_t lenght_temp;
	
	//spi write at most 4095 bytes one time
	if(lenght > SPI_BUFFER_MAX_SIZE)
	{
		num 		= lenght / SPI_BUFFER_MAX_SIZE;
		cnt 		= 0;
		lenght_temp = lenght % SPI_BUFFER_MAX_SIZE;

		for(i = 0;i < num; i ++)
		{
			cnt = i * SPI_BUFFER_MAX_SIZE;
			Drv_Nandflash_ReadBytes(&data[cnt],SPI_BUFFER_MAX_SIZE);			
		}

		cnt = i * SPI_BUFFER_MAX_SIZE;
		Drv_Nandflash_ReadBytes(&data[cnt],lenght_temp);			
	}
	else
	{
		Drv_Nandflash_ReadBytes(data,lenght);
	}	
}

//**********************************************************************
// 函数功能:	NandFlash状态寄存器获取
// 输入参数：	无
// 返回参数：	
// status  :    状态寄存器值
static uint8_t Drv_Nandflash_GetStatus(void)
{
	uint8_t status;
	uint8_t cmd[2];

	cmd[0]	= NANDFLASH_GET_FEATURES;
	cmd[1]	= NANDFLASH_STATUS;
	
	NANDFLASH_SPI_CS_L();
	Drv_NandFlash_WriteData(cmd,2);
	status = Drv_Nandflash_ReadByte();
    NANDFLASH_SPI_CS_H();

	return status;
}

//**********************************************************************
// 函数功能:	NandFlash写使能
// 输入参数：	无
// 返回参数：	无
static void Drv_Nandflash_WriteEnable(void)
{
	NANDFLASH_SPI_CS_L();
	Drv_Nandflash_WriteByte(NANDFLASH_WRITE_ENABLE);
	NANDFLASH_SPI_CS_H();
}

//**********************************************************************
// 函数功能:	NandFlash写禁止
// 输入参数：	无
// 返回参数：	无
static void Drv_Nandflash_WriteDisable(void)
{
	NANDFLASH_SPI_CS_L();
	Drv_Nandflash_WriteByte(NANDFLASH_WRITE_DISABLE);
	NANDFLASH_SPI_CS_H();
}

//**********************************************************************
// 函数功能:	NandFlash写禁止
// 输入参数：	
// dienew  :	die 编号，0/1
// 返回参数：	
// 0x00    :    操作成功
// 0xff    :    操作失败
static uint16_t Drv_Nandflash_DieSet(uint8_t dienew)
{
	uint8_t 	cmd[3];

	cmd[0] 		=  NANDFLASH_SET_FEATURES;
	cmd[1] 		= NANDFLASH_DIE_SELECT;
	if (dienew)
	{
		cmd[2] 		= DIE_SELECT_DIE1;//set to die1
	}
	else
	{
		cmd[2] 		= DIE_SELECT_DIE0;//set to die0
	}
	
	NANDFLASH_SPI_CS_L();
	Drv_NandFlash_WriteData(cmd, 3);
	NANDFLASH_SPI_CS_H();
	
	am_util_delay_us(5);

	return 0;
}

//**********************************************************************
// 函数功能:	等待nandflash空闲
// 输入参数：	
// WaitTick:    等待nandflash空闲的tick个数
// 返回参数：	
// 0x00    :    操作成功
static uint16_t Drv_Nandflash_IdleWait(uint16_t WaitTick)
{
	uint16_t waitCnt, ret = Ret_OK;
	waitCnt = 0;
	while(NANDFLASH_OIP_BUSY & Drv_Nandflash_GetStatus())
	{
		waitCnt++;
		if(waitCnt > WaitTick)
		{
			ret = Ret_DeviceBusy;
			break;
		}
		Drv_NandFlash_DelayTick();		// ticks
	}
	return ret;
}

//**********************************************************************
// 函数功能:	向NandFlash数据缓冲区写数据
// 输入参数：	
// *pdata  : 	准备写入的数据指针
// addr    :   	数据写入的开始地址
// length 	: 	写入的数据长度
// 返回参数：	
// 0x00    :    操作成功
// 0xff    :    操作失败
static uint16_t Drv_NandFlash_CacheLoad(uint8_t* pdata, uint32_t addr, uint32_t length)
{
	uint16_t 	columnaddr,blockaddr;
	uint8_t 	data[3];
	
	blockaddr 	= PHY_BLOCK_ADDR_GET(addr);
	columnaddr 	= PHY_BYTE_ADDR_GET(addr);
	
	if ((blockaddr % 2)== 0)//even
	{
		columnaddr &= CLEAR(12);//plane for even block
	}
	else
	{
		columnaddr |= SET(12);//plane for odd block
	}
	
	data[0] 	= NANDFLASH_PROGRAM_LOAD;
	data[1] 	= (uint8_t)(columnaddr >> 8);	//MSB first	
	data[2] 	= (uint8_t)columnaddr;

	NANDFLASH_SPI_CS_L();
	Drv_NandFlash_WriteData(data, 3);
	Drv_NandFlash_WriteData(pdata, length);
    NANDFLASH_SPI_CS_H();

    return 0;
}


//**********************************************************************
// 函数功能:	启动NandFlash数据缓冲区数据转移到逻辑单元
// 输入参数：	
// addr    :    数据写入的开始地址
// 返回参数：	
// 0x00    :    操作成功
// 0xff    :    操作失败
static uint16_t Drv_NandFlash_Program(uint32_t addr)
{
	uint32_t 	addrtemp;
	uint8_t		data[4];
	
	if(GET_BIT(addr, 29) != CurrentDie)
	{
		CurrentDie = GET_BIT(addr, 29);
		Drv_Nandflash_DieSet(CurrentDie);
	}	
	
	/* 以page为单位，bit0~11忽略*/
	addrtemp = addr >> 12;
	data[0] 	= NANDFLASH_PROGRAM_EXECUTE;
	data[1] 	= (uint8_t)(addrtemp >> 16);
	data[2] 	= (uint8_t)(addrtemp >> 8);
	data[3] 	= (uint8_t)(addrtemp >> 0);	// 仅最高位为有效地址，低7bit无效	

    NANDFLASH_SPI_CS_L();
	Drv_NandFlash_WriteData(data, 4);
	NANDFLASH_SPI_CS_H();

	return 0;
}

//**********************************************************************
// 函数功能:	启动Nandflash内部数据转移到缓冲区
// 输入参数：
// addr    :    addr NandFlash实际的29bit物理地址
// 返回参数：	
// 0x00    :    操作成功
// 0xff    :    操作失败
static uint16_t Drv_NandFlash_Read(uint32_t addr)
{
	uint32_t 	addrtemp;
	uint8_t		data[4];
	
	if(GET_BIT(addr, 29) != CurrentDie)
	{
		CurrentDie = GET_BIT(addr, 29);
		Drv_Nandflash_DieSet(CurrentDie);
	}	
	
	/* 缓存cache数据以page为单位，bit0~11忽略*/
	addrtemp = addr >> 12;
	data[0] 	= NANDFLASH_PAGE_READ;
	data[1] 	= (uint8_t)(addrtemp >> 16);
	data[2] 	= (uint8_t)(addrtemp >> 8);
	data[3] 	= (uint8_t)(addrtemp >> 0);	// 仅最高位为有效地址，低7bit无效	

    NANDFLASH_SPI_CS_L();
	Drv_NandFlash_WriteData(data, 4);
	NANDFLASH_SPI_CS_H();

	return 0;
}

//**********************************************************************
// 函数功能:	读取Nandflash数据缓冲区的数据
// 输入参数：	
// *pdata  : 	存储数据的指针
// addr    :   	数据读取的开始地址
// length 	: 	读取的数据长度
// 返回参数：	
// 0x00    :    设置成功
// 0xff    :    设置失败
static uint16_t Drv_NandFlash_CacheRead(uint8_t* pdata,uint32_t addr, uint32_t length)
{
	uint16_t 	columnaddr,blockaddr;
	uint8_t 	data[3];

	if(GET_BIT(addr, 29) != CurrentDie)
	{
		CurrentDie = GET_BIT(addr, 29);
		Drv_Nandflash_DieSet(CurrentDie);
	}
	
	blockaddr 	= PHY_BLOCK_ADDR_GET(addr);
	columnaddr 	= PHY_BYTE_ADDR_GET(addr);
	
	if ((blockaddr % 2)== 0)//even
	{
		columnaddr &= CLEAR(12);//plane for even block
	}
	else
	{
		columnaddr |= SET(12);//plane for odd block
	}	
	
	data[0] 	= NANDFLASH_READ_FROM_CACHE;
	data[1] 	= (uint8_t)(columnaddr >> 8);
	data[2] 	= (uint8_t)(columnaddr);

	NANDFLASH_SPI_CS_L();
	Drv_NandFlash_WriteData(data, 3);
	Drv_Nandflash_ReadByte();			//one dummy byte
	Drv_NandFlash_ReadData(pdata, length);
    NANDFLASH_SPI_CS_H();

    return 0;
}

//**********************************************************************
// 函数功能:	NandFlash软件初始化
// 输入参数：	
// blockLockState: 块锁保护设置
// 返回参数：	
// 0x00    :    初始化成功
// 0xff    :    初始化失败
uint16_t Drv_NandFlash_SoftInit(uint8_t blockLockState)
{
	uint8_t cmd[2];
	
	cmd[0] 		= NANDFLASH_BLOCK_LOCK;
	cmd[1] 		= blockLockState;		//unlock all 
	
	Drv_NandFlash_Open();
	NANDFLASH_SPI_CS_L();
	Drv_Nandflash_WriteByte(NANDFLASH_SET_FEATURES);
	Drv_NandFlash_WriteData(cmd, 2);
	NANDFLASH_SPI_CS_H();
	Drv_NandFlash_Close();
	
	return 0;
}

//**********************************************************************
// 函数功能:	NandFlash软件复位操作,需要等待1.2ms才能对芯片操作
// 输入参数：	无
// 返回参数：	
// 0x00    :    设置成功
// 0xff    :    设置失败
uint16_t Drv_NandFlash_SoftReset(void)
{
	Drv_NandFlash_Open();
	NANDFLASH_SPI_CS_L();
	Drv_Nandflash_WriteByte(NANDFLASH_RESET);
	NANDFLASH_SPI_CS_H();
	Drv_NandFlash_Close();
	
	/* 必须延时大于1.2ms */
	am_util_delay_ms(2);

	return 0;
}

//**********************************************************************
// 函数功能:	NandFlash块擦除,128KB
// 输入参数：	
// startBlockAddr: 待擦除的块起始地址
// 返回参数：	
// 0x00    :    操作成功
// 0xff    :    操作失败
uint16_t Drv_NandFlash_BlockErase(uint32_t addr)
{
	uint32_t addrtemp;
    uint8_t  	data[4];

	Drv_NandFlash_Open();
	
	/* 每次操作前对die进行校验 */ 
	if(GET_BIT(addr, 29) != CurrentDie)
	{
		CurrentDie = GET_BIT(addr, 29);
		Drv_Nandflash_DieSet(CurrentDie);
	}
	
	/* WRITE ENABLE */
	Drv_Nandflash_WriteEnable();
	
	/* block擦除指令 */
	addrtemp = addr >> 12;
	data[0] 	= NANDFLASH_BLOCK_ERASE;
	data[1] 	= (uint8_t)(addrtemp >> 16);
	data[2] 	= (uint8_t)(addrtemp >> 8);
	data[3] 	= (uint8_t)(addrtemp >> 0);	// 仅最高位为有效地址，低7bit无效	
    NANDFLASH_SPI_CS_L();
    Drv_NandFlash_WriteData(data, 4);
    NANDFLASH_SPI_CS_H();  

	/* 等待快擦除完成 */
	Drv_Nandflash_IdleWait(ERASE_IDELTICKS);
	
	/* WRITE DISABLE */
	Drv_Nandflash_WriteDisable();
	
	Drv_NandFlash_Close();
	
    return 0;      
}

//**********************************************************************
// 函数功能:	写某地址数据，不越page
// 输入参数：	pdata 代写入数据指针
//				addr NandFlash实际的29bit物理地址
// 				Length 读数据长度
// 返回参数：
uint16_t Drv_NandFlash_PageWrite(uint8_t* pdata,uint32_t addr, uint32_t length)
{
	uint16_t RetVal = Ret_OK;
	
	if((PHY_BYTE_ADDR_GET(addr) + length) > PHY_BYTE_OF_PAGE)
		return Ret_InvalidParam;	
	
	Drv_NandFlash_Open();
	
	// 在切换die时，必须写入2次才能写入成功，暂时不知道什么原因
//	for(uint32_t i = 0;i < 2;i++)
	{
		/* 每次操作前对die进行校验 */ 
		if(GET_BIT(addr, 29) != CurrentDie)
		{
			CurrentDie = GET_BIT(addr, 29);
			Drv_Nandflash_DieSet(CurrentDie);
		}
		
		/* WRITE ENABLE */
		Drv_Nandflash_WriteEnable();
		
		/* 待写数据加载到cache */
		RetVal |= Drv_NandFlash_CacheLoad(pdata, addr, length);

		/* 执行写入操作，并等待完成 */
		RetVal |= Drv_NandFlash_Program(addr);
		RetVal |= Drv_Nandflash_IdleWait(READ_WRITE_IDELTICKS);	
		
		/* WRITE DISABLE */
		Drv_Nandflash_WriteDisable();	
	}

	Drv_NandFlash_Close();
	
	if(RetVal)
	{
		DRV_NANDFLASH_RTT_LOG(0,"Drv_NandFlash_PageWrite Err %d \n", RetVal);
	}	
	
	return RetVal;
}

//**********************************************************************
// 函数功能:	读取某地址数据，不越page
// 输入参数：	pdata 读取结果参数指针
//				addr NandFlash实际的29bit物理地址
// 				Length 读数据长度
// 返回参数：	
uint16_t Drv_NandFlash_PageRead(uint8_t* pdata,uint32_t addr, uint32_t length)
{
	uint16_t RetVal = Ret_OK;
	
	if((PHY_BYTE_ADDR_GET(addr) + length) > PHY_BYTE_OF_PAGE)
		return Ret_InvalidParam;
	
	Drv_NandFlash_Open();
	
	/* 每次操作前对die进行校验 */ 
	if(GET_BIT(addr, 29) != CurrentDie)
	{
		CurrentDie = GET_BIT(addr, 29);
		Drv_Nandflash_DieSet(CurrentDie);
	}
	
	/* 读取数据到cache，并等待完成 */
	RetVal |= Drv_NandFlash_Read(addr);
	RetVal |= Drv_Nandflash_IdleWait(READ_WRITE_IDELTICKS);
	
	/* 从cache中读取数据到ram */ 
	RetVal |= Drv_NandFlash_CacheRead(pdata, addr, length);
	
	Drv_NandFlash_Close();
	
	if(RetVal)
	{
		DRV_NANDFLASH_RTT_LOG(0,"Drv_NandFlash_PageRead Err %d \n", RetVal);
	}
	
	return RetVal;
}


uint16_t Drv_NandFlashIdCHeck(void)
{
	uint8_t TmpBuf[3];
	
	Drv_NandFlash_Open();
	NANDFLASH_SPI_CS_L();
	SMDrv_SPI_WriteByte(FLASH_SPI_MODULE,NANDFLASH_READ_ID);
	SMDrv_SPI_ReadBytes(FLASH_SPI_MODULE, TmpBuf, 1);	//one dummy byte
	SMDrv_SPI_ReadBytes(FLASH_SPI_MODULE, TmpBuf+1, 2);
	NANDFLASH_SPI_CS_H();
	Drv_NandFlash_Close();
	
    if (TmpBuf[1] != MANUFACTURER_ID )
    {
    	return TmpBuf[1];
    }
    if (TmpBuf[2] != DEVICE_ID)
    {
    	return TmpBuf[2];
    }
	
	return 0x00;
}






