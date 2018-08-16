/**********************************************************************
**
**模块说明: 
**   
**
**********************************************************************/

#include "io_config.h"
#include "sm_gpio.h"

#include "drv_touch.h"
#include "am_util_delay.h"

#if 1

#define IT7259_ADDR			(0x8C)
#define IT7259_ADDR_READ 	(IT7259_ADDR | 0x01)

#define COMMAND_BUFFER_INDEX   					0x20 
#define QUERY_BUFFER_INDEX						0x80
#define COMMAND_RESPONSE_BUFFER_INDEX 			0xA0 
#define POINT_BUFFER_INDEX    					0xE0 
#define QUERY_SUCCESS     						0x00
#define QUERY_BUSY     							0x01 
#define QUERY_ERROR     						0x02 
#define QUERY_POINT     						0x80 

#define POINT 				0x08
#define GESTURES			0x80

#define TAP 				0x20
#define FLICK 				0x22
#define DOUBLE_TAP 			0x23

#define UP					0x08
#define UPPER_RIGHT 		0x09
#define RIGHT 				0x0A
#define LOWER_RIGHT 		0x0B
#define DOWN			 	0x0C
#define LOWER_LEFT 			0x0D
#define LEFT 				0x0E
#define UPPER_LEFT 			0x0F

#define IIC_SCL_PIN 	(48)	
#define IIC_SDA_PIN		(49)


Drv_IT7259_Param_t	DrvIT7259;

void I2C_PIN_OUT(PIN)
{
//    am_hal_gpio_pincfg_t bfGpioCfg;

//    bfGpioCfg.uFuncSel       = 3;
//    bfGpioCfg.eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_2MA;//AM_HAL_GPIO_PIN_DRIVESTRENGTH_2MA;
//    bfGpioCfg.eGPOutcfg      = AM_HAL_GPIO_PIN_OUTCFG_PUSHPULL;
//	 bfGpioCfg.ePullup        = AM_HAL_GPIO_PIN_PULLUP_1_5K;//AM_HAL_GPIO_PIN_PULLUP_6K
//    am_hal_gpio_pinconfig(PIN, bfGpioCfg);	
	
    am_hal_gpio_pincfg_t bfGpioCfg;

    bfGpioCfg.uFuncSel       = 3;
    bfGpioCfg.eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_2MA;//AM_HAL_GPIO_PIN_DRIVESTRENGTH_2MA;
    bfGpioCfg.eGPOutcfg      = AM_HAL_GPIO_PIN_OUTCFG_PUSHPULL;
//	 bfGpioCfg.ePullup        = AM_HAL_GPIO_PIN_PULLUP_6K;//AM_HAL_GPIO_PIN_PULLUP_1_5K;
	
//	am_hal_gpio_state_write(PIN, AM_HAL_GPIO_OUTPUT_CLEAR);//为什么要这个才可以？
	
    am_hal_gpio_pinconfig(PIN, bfGpioCfg);	
}

void I2C_PIN_IN(PIN)
{
    am_hal_gpio_pincfg_t bfGpioCfg;

    bfGpioCfg.uFuncSel       = 3;
    bfGpioCfg.eGPOutcfg      = AM_HAL_GPIO_PIN_OUTCFG_DISABLE;
    bfGpioCfg.eGPInput       = AM_HAL_GPIO_PIN_INPUT_ENABLE;
    bfGpioCfg.ePullup        = AM_HAL_GPIO_PIN_PULLUP_1_5K;//AM_HAL_GPIO_PIN_PULLUP_6K;//
    am_hal_gpio_pinconfig(PIN, bfGpioCfg);	
}


void I2C_SCL_High(PIN)
{
	am_hal_gpio_state_write(PIN, AM_HAL_GPIO_OUTPUT_SET);
//	am_hal_gpio_output_set(PIN);
}	

void I2C_SCL_Low(PIN)
{
	am_hal_gpio_state_write(PIN, AM_HAL_GPIO_OUTPUT_CLEAR);
//	am_hal_gpio_output_clear(PIN);
}

void I2C_SDA_High(PIN)
{
	am_hal_gpio_state_write(PIN, AM_HAL_GPIO_OUTPUT_SET);
//	am_hal_gpio_output_set(PIN);
}
void I2C_SDA_Low(PIN)
{
	am_hal_gpio_state_write(PIN, AM_HAL_GPIO_OUTPUT_CLEAR);
//	am_hal_gpio_output_clear(PIN);
}
uint32_t I2C_SDA_Value(PIN)
{
	return am_hal_gpio_input_read(PIN);
}

#define I2C_Delay()			am_util_delay_ms(1) //am_util_delay_us(10)

static void I2C_PinConfig(void)
{
	I2C_PIN_OUT(IIC_SCL_PIN);
	I2C_PIN_OUT(IIC_SDA_PIN);
	I2C_SCL_High(IIC_SCL_PIN);
	I2C_SDA_High(IIC_SDA_PIN);
}

static void I2C_Start(void)
{
    I2C_PIN_OUT(IIC_SDA_PIN);
    I2C_SCL_High(IIC_SCL_PIN);
    I2C_Delay();
    I2C_SDA_High(IIC_SDA_PIN);
    I2C_Delay();
    I2C_SDA_Low(IIC_SDA_PIN);
    I2C_Delay();
    I2C_SCL_Low(IIC_SCL_PIN);
    I2C_Delay();
}

static void I2C_Stop(void)
{
    I2C_SCL_Low(IIC_SCL_PIN);
    I2C_PIN_OUT(IIC_SDA_PIN);
    I2C_SDA_Low(IIC_SDA_PIN);
    I2C_Delay();
    I2C_SCL_High(IIC_SCL_PIN);
    I2C_Delay();
    I2C_SDA_High(IIC_SDA_PIN);
    I2C_Delay();
}

static void I2C_NoACK(void)
{
    I2C_SDA_High(IIC_SDA_PIN);
    I2C_Delay();
    I2C_SCL_High(IIC_SCL_PIN);
    I2C_Delay();
    I2C_SCL_Low(IIC_SCL_PIN);
}

static void I2C_ACK(void)
{
    I2C_SDA_Low(IIC_SDA_PIN);
    I2C_Delay();
    I2C_SCL_High(IIC_SCL_PIN);
    I2C_Delay();
    I2C_SCL_Low(IIC_SCL_PIN);
}

static unsigned char I2C_ReadByte(void)
{
    unsigned char i;
    unsigned char Data = 0;
    I2C_PIN_IN(IIC_SDA_PIN);
    I2C_Delay();
    for (i = 0; i < 8; i++)
    {
        I2C_SCL_Low(IIC_SCL_PIN);
        I2C_Delay();
        I2C_SCL_High(IIC_SCL_PIN);
        I2C_Delay();
        if (I2C_SDA_Value(IIC_SDA_PIN) == 1)
        {
            Data = (Data << 1) + 0x01;
        }
        else
        {
            Data = (Data << 1) + 0x00;
        }
    }
    I2C_SCL_Low(IIC_SCL_PIN);
    I2C_PIN_OUT(IIC_SDA_PIN);
    I2C_Delay();
    return (Data);
}

static unsigned short I2C_WriteByte(unsigned char Data)
{
    unsigned char i;
    I2C_PIN_OUT(IIC_SDA_PIN);
    for (i = 0; i < 8; i++)
    {
        if ((Data & 0x80) == 0x80)
        {
            I2C_SDA_High(IIC_SDA_PIN);
        }
        else
        {
            I2C_SDA_Low(IIC_SDA_PIN);
        }
        I2C_Delay();
        I2C_SCL_High(IIC_SCL_PIN);
        I2C_Delay();
        I2C_SCL_Low(IIC_SCL_PIN);
        Data <<= 0x01;
    }

    //      Wait "ACK" Signal
    I2C_PIN_IN(IIC_SDA_PIN);
    I2C_Delay();
    I2C_SCL_High(IIC_SCL_PIN);
    I2C_Delay();

    while(1)
    {
        if(I2C_SDA_Value(IIC_SDA_PIN) == 1)
        {
            if ((--i) == 0)
            {
                I2C_PIN_OUT(IIC_SDA_PIN);
                I2C_SDA_High(IIC_SDA_PIN);
                I2C_Delay();
                return Ret_NoDevice;	//  //no ack indicate
            }
        }
        else
        {
            break;
        }
    }
    I2C_SCL_Low(IIC_SCL_PIN);
    I2C_PIN_OUT(IIC_SDA_PIN);
    I2C_SDA_High(IIC_SDA_PIN);
    I2C_Delay();
    return Ret_OK;   // Write OK
}

static unsigned short I2C_WriteBytes(unsigned char* dataAddr, unsigned short length)
{
    unsigned short i;
    unsigned short ret = Ret_OK;
    for(i = 0; i < length; i++)
    {
        ret = I2C_WriteByte(dataAddr[i]);
        if(ret != Ret_OK)
        {
            break;
        }
    }
    return ret;
}

static void I2C_ReadBytes(unsigned char *dataAddr, unsigned short length)
{
    unsigned short i;
    for(i = 0; i < length; i++)
    {
        dataAddr[i] = I2C_ReadByte();
        if(i < (length - 1))
        {
            I2C_ACK();
        }
        else
        {
            I2C_NoACK();
        }
    }
}

static uint8_t Drv_IT7259_WriteBuffer(uint8_t addr, uint8_t* pdata, int len) 
{
	uint8_t RetVal = Ret_OK;
	
	I2C_Start(); 
	
	RetVal |= I2C_WriteByte(IT7259_ADDR);	// 设备地址
	
	RetVal |= I2C_WriteByte(addr);		// 寄存器地址
	
	RetVal |= I2C_WriteBytes(pdata, len);
	
	I2C_Stop(); 
	
	return RetVal;
}

static uint8_t Drv_IT7259_ReadBuffer(uint8_t addr, uint8_t* pdata, int len) 
{
	uint8_t RetVal = Ret_OK;
	
	I2C_Start(); 
	
	RetVal |= I2C_WriteByte(IT7259_ADDR);	// 设备地址
	RetVal |= I2C_WriteByte(addr);			// 寄存器地址
	
	I2C_Start(); 
	
	RetVal |= I2C_WriteByte(IT7259_ADDR_READ);	// 读地址
	
	I2C_ReadBytes(pdata, len);
	
	I2C_Stop();
	
	return Ret_OK;
}


static uint8_t Drv_IT7259_CheckBusy(void)
{
	uint8_t query, RetVal;
	uint16_t i=0;

	do
	{
		RetVal = Drv_IT7259_ReadBuffer(QUERY_BUFFER_INDEX, &query, 1);
		i++;
		if(i>500)
			return Ret_DeviceBusy;
	}while((query & QUERY_BUSY) || (Ret_OK != RetVal));
	
	return Ret_OK;
}

// 检测触摸屏ID
static uint8_t Drv_IT7259_IdCapSensor(void)
{
	uint8_t RetVal;
	uint8_t Cmd = 0;
	uint8_t data[10];
	
	RetVal = Drv_IT7259_CheckBusy();
	if( Ret_OK != RetVal)	return RetVal;
	
	RetVal = Drv_IT7259_WriteBuffer(COMMAND_BUFFER_INDEX, &Cmd, 1);
	if( Ret_OK != RetVal)	return RetVal;
	
	RetVal = Drv_IT7259_CheckBusy();
	if( Ret_OK != RetVal)	return RetVal;

	Drv_IT7259_ReadBuffer(COMMAND_RESPONSE_BUFFER_INDEX, data, 10);
	if( Ret_OK != RetVal)	return RetVal;
	
	if(data[1] != 'I' || data[2] != 'T' || data[3] != 'E' || data[4] != '7' || data[5] != '2'  || data[6] != '5'  || data[7] != '9')
		return Ret_Fail;

	return Ret_OK;
}

// 获取触摸屏分辨率
static uint8_t Drv_IT7259_2DResolutionsGet(uint16_t *pwXResolution, uint16_t *pwYResolution, uint8_t *pucStep)
{
	uint8_t RetVal= Ret_OK;
	uint8_t cmd[2] = {0x01, 0x02};
	uint8_t data[14];
	
	RetVal = Drv_IT7259_CheckBusy();
	if( Ret_OK != RetVal)	return RetVal;
	RetVal = Drv_IT7259_WriteBuffer(COMMAND_BUFFER_INDEX, cmd, 2);
	if( Ret_OK != RetVal)	return RetVal;	

	RetVal = Drv_IT7259_CheckBusy();
	if( Ret_OK != RetVal)	return RetVal;
	RetVal = Drv_IT7259_ReadBuffer(COMMAND_RESPONSE_BUFFER_INDEX, data, 14);
	if( Ret_OK != RetVal)	return RetVal;
	
	if(pwXResolution != NULL)
		*pwXResolution = data[2] + (data[3] << 8);

	if(pwYResolution != NULL)
		*pwYResolution = data[4] + (data[5] << 8);

	if(pucStep != NULL)
		*pucStep = data[6];
	
	return Ret_OK;	
}

// 检测是否有新触摸产生
static uint8_t Drv_IT7259_NewPointCheck(void)
{
	uint8_t query, RetVal = Ret_OK;
	
	RetVal = Drv_IT7259_ReadBuffer(QUERY_BUFFER_INDEX, &query, 1);
	if( Ret_OK != RetVal)	return RetVal;	
		
	if (query & QUERY_POINT)  
		return 1;	
	else
		return 0;
}

// 获取触摸屏点阵信息
static uint8_t Drv_IT7259PointDataRead(uint8_t* DataBuf)
{
	uint8_t RetVal = Ret_OK;
	
	RetVal = Drv_IT7259_ReadBuffer(POINT_BUFFER_INDEX, DataBuf, 14);
	
	return RetVal;
}

// 触摸中断回调函数
static void Drv_Touch_Isr(uint32 u32PinNum)
{
//	DRV_TOUCH_RTT_LOG(0,"Drv_Touch_Isr \n");
	
	if(Drv_IT7259_NewPointCheck())
	{
		if(Ret_OK == Drv_IT7259PointDataRead(DrvIT7259.PointData))
		{
			if(DrvIT7259.PointData[0] & POINT)		// 报点															
			{
				DRV_TOUCH_RTT_LOG(0,"POINT \n");
			}

			if(DrvIT7259.PointData[0] & GESTURES)	// 手势													
			{
				DRV_TOUCH_RTT_LOG(0,"GID %02X \n", DrvIT7259.PointData[1]);				
			}			
		}
	}
}

void delay(void)
{
	uint32_t i = 0xFFFFFF;
	do
	{
		i--;
	}while(i);
}

void Drv_IT7259_Init(void)
{
	 SMDrv_GPIO_BitClear(48);	//为什么要这个才可以？,经测试可以是任务IO口
	I2C_PinConfig();

//	while(1)
//	{
//		I2C_SCL_High(IIC_SCL_PIN);
//		I2C_SDA_High(IIC_SDA_PIN);
//		I2C_Delay();
//		I2C_SCL_Low(IIC_SCL_PIN);
//		I2C_SDA_Low(IIC_SDA_PIN);
//		I2C_Delay();
//	}
	
//	SMDrv_GPIO_Open(TOUCH_INT_PIN,NULL,Drv_Touch_Isr);
	
	if(Ret_OK != Drv_IT7259_IdCapSensor())
	{
		DRV_TOUCH_RTT_ERR(0,"Drv_IT7259_IdCapSensor Err \n");
		
	}
	
	if(Ret_OK != Drv_IT7259_2DResolutionsGet(&DrvIT7259.XResolution, &DrvIT7259.YResolution, &DrvIT7259.Scale))
	{
		DRV_TOUCH_RTT_ERR(0,"Drv_IT7259_2DResolutionsGet Err \n");
	}
}


 

#endif













