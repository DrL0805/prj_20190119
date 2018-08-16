/**********************************************************************
**
**模块说明: 对接MCU SWIIC驱动接口，并根据项目原理图作配置
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.16  初版  ZSL  
**   V2.0   2018.8.8  增加apollo3  ZSL 
**
**********************************************************************/
//在此模块中使用UART IO口，因此需定义IIC_MODULE

#include "io_config.h"

#include "am_mcu_apollo.h"
#include "sm_swi2c.h"
#include "am_util_delay.h"
#include "string.h"

//定义iic口
enum
{
    IIC0=0,
    IIC1,
    IIC2,
    IIC3,
    IIC4,
    IIC5,
    IIC_MAX,
};

typedef struct
{
    uint8  u8speed;   //SWIIC 速度
    uint8  bOpen;     //iic是否已经打开，标记若已打开，则不必重复打开，主要在多个模块使用同一IIC时
    uint32 u32module; //标记哪些模块已open iic
}swiic_info_s;

static swiic_info_s swiic_info[IIC_MAX];

//**********************************************************************
// 函数功能: iic delay,用来控制IIC 速度
// 输入参数: iic_id: IIC口，为不同的iic上配置不同的速度
// 返回参数：
//**********************************************************************
static void I2C_Delay(uint8 iic_id)
{
    //! @note Interrupts are active during execution of this function.  Therefore,
    //! any interrupt taken will affect the delay timing.
    am_util_delay_us(swiic_info[iic_id].u8speed);
}

#ifdef AM_PART_APOLLO3
//**********************************************************************
// 函数功能: 设置iic SCL脚为输出
// 输入参数: u32scl_pin: SCL pin number
// 返回参数：
//**********************************************************************
static void I2C_SCL_Out(uint32 u32scl_pin)
{
    am_hal_gpio_pincfg_t bfGpioCfg = {0};

	// 临时变量值是随机的，使用前必须清零
//	memset(&bfGpioCfg, 0x00, sizeof(am_hal_gpio_pincfg_t));	
	
    bfGpioCfg.uFuncSel       = 3;
    bfGpioCfg.eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_4MA;//AM_HAL_GPIO_PIN_DRIVESTRENGTH_2MA;
    bfGpioCfg.eGPOutcfg      = AM_HAL_GPIO_PIN_OUTCFG_PUSHPULL;
//	 bfGpioCfg.ePullup        = AM_HAL_GPIO_PIN_PULLUP_6K;//AM_HAL_GPIO_PIN_PULLUP_1_5K;
    am_hal_gpio_pinconfig(u32scl_pin, bfGpioCfg);
}

//**********************************************************************
// 函数功能: 设置iic SDA脚为输出
// 输入参数: u32sda_pin: SDA pin number
// 返回参数：
//**********************************************************************
static void I2C_SDA_Out(uint32 u32sda_pin)
{
    am_hal_gpio_pincfg_t bfGpioCfg = {0};

	// 临时变量值是随机的，使用前必须清零，否则不确定变量值导致函数调用错误
//	memset(&bfGpioCfg, 0x00, sizeof(am_hal_gpio_pincfg_t));	
	
    bfGpioCfg.uFuncSel       = 3;
    bfGpioCfg.eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_4MA;//AM_HAL_GPIO_PIN_DRIVESTRENGTH_2MA;
    bfGpioCfg.eGPOutcfg      = AM_HAL_GPIO_PIN_OUTCFG_PUSHPULL;
//	 bfGpioCfg.ePullup        = AM_HAL_GPIO_PIN_PULLUP_6K;//AM_HAL_GPIO_PIN_PULLUP_1_5K;
    am_hal_gpio_pinconfig(u32sda_pin, bfGpioCfg);
}

//**********************************************************************
// 函数功能: 设置iic SDA脚为输入
// 输入参数: 
//     iic_id: IIC口，为不同的iic上上拉电阻不同的情况作配置
//     u32sda_pin: SDA pin number
// 返回参数：
//**********************************************************************
static void I2C_SDA_In(uint8 iic_id,uint32 u32sda_pin)
{
    am_hal_gpio_pincfg_t bfGpioCfg = {0};

	// 临时变量值是随机的，使用前必须清零
//	memset(&bfGpioCfg, 0x00, sizeof(am_hal_gpio_pincfg_t));	
	
    bfGpioCfg.uFuncSel       = 3;
    bfGpioCfg.eGPOutcfg      = AM_HAL_GPIO_PIN_OUTCFG_DISABLE;
    bfGpioCfg.eGPInput       = AM_HAL_GPIO_PIN_INPUT_ENABLE;
//    bfGpioCfg.ePullup        = AM_HAL_GPIO_PIN_PULLUP_6K;//AM_HAL_GPIO_PIN_PULLUP_1_5K;
    am_hal_gpio_pinconfig(u32sda_pin, bfGpioCfg);
}

//**********************************************************************
// 函数功能: 设置iic SCL脚为输出高
// 输入参数: u32scl_pin: SCL pin number
// 返回参数：
//**********************************************************************
static void I2C_SCL_High(uint32 u32scl_pin)
{
    //note: 为了减少延时，采用宏，减少函数调用开销
    am_hal_gpio_output_set(u32scl_pin);
}

//**********************************************************************
// 函数功能: 设置iic SCL脚为输低
// 输入参数: u32scl_pin: SCL pin number
// 返回参数：
//**********************************************************************
static void I2C_SCL_Low(uint32 u32scl_pin)
{
    //note: 为了减少延时，采用宏，减少函数调用开销
    am_hal_gpio_output_clear(u32scl_pin);
}

//**********************************************************************
// 函数功能: 设置iic SDA脚为输出高
// 输入参数: u32sda_pin: SDA pin number
// 返回参数：
//**********************************************************************
static void I2C_SDA_High(uint32 u32sda_pin)
{
    //note: 为了减少延时，采用宏，减少函数调用开销
    am_hal_gpio_output_set(u32sda_pin);
}

//**********************************************************************
// 函数功能: 设置iic SDA脚为输出低
// 输入参数: u32sda_pin: SDA pin number
// 返回参数：
//**********************************************************************
static void I2C_SDA_Low(uint32 u32sda_pin)
{
    //note: 为了减少延时，采用宏，减少函数调用开销
    am_hal_gpio_output_clear(u32sda_pin);
}

//**********************************************************************
// 函数功能: 获取iic SDA脚值
// 输入参数: u32sda_pin: SDA pin number
// 返回参数：
//**********************************************************************
static uint32 I2C_SDA_Value(uint32 u32sda_pin)
{
    //note: 为了减少延时，采用宏，减少函数调用开销
    return am_hal_gpio_input_read(u32sda_pin);
}
#else
//**********************************************************************
// 函数功能: 设置iic SCL脚为输出
// 输入参数: u32scl_pin: SCL pin number
// 返回参数：
//**********************************************************************
static void I2C_SCL_Out(uint32 u32scl_pin)
{
    SMDrv_GPIO_SetIOPad(u32scl_pin, GPIO_PIN_OUTPUT);
}

//**********************************************************************
// 函数功能: 设置iic SDA脚为输出
// 输入参数: u32sda_pin: SDA pin number
// 返回参数：
//**********************************************************************
static void I2C_SDA_Out(uint32 u32sda_pin)
{
    SMDrv_GPIO_SetIOPad(u32sda_pin, GPIO_PIN_OUTPUT);
}

//**********************************************************************
// 函数功能: 设置iic SDA脚为输入
// 输入参数: 
//     iic_id: IIC口，为不同的iic上上拉电阻不同的情况作配置
//     u32sda_pin: SDA pin number
// 返回参数：
//**********************************************************************
static void I2C_SDA_In(uint8 iic_id,uint32 u32sda_pin)
{
    //IIC4oíIIC5ía?ó10Kμ?×è//部分无上拉电阻
    // if((iic_id == IIC4) || (iic_id == IIC5))
    //     SMDrv_GPIO_SetIOPad(u32sda_pin, GPIO_PIN_INPUT);
    // else
        SMDrv_GPIO_SetIOPad(u32sda_pin, GPIO_PIN_INPUT | GPIO_PIN_PULL1_5K);
}

//**********************************************************************
// 函数功能: 设置iic SCL脚为输出高
// 输入参数: u32scl_pin: SCL pin number
// 返回参数：
//**********************************************************************
static void I2C_SCL_High(uint32 u32scl_pin)
{
    // SMDrv_GPIO_BitSet(u32scl_pin);
    am_hal_gpio_out_bit_set(u32scl_pin);
}

//**********************************************************************
// 函数功能: 设置iic SCL脚为输低
// 输入参数: u32scl_pin: SCL pin number
// 返回参数：
//**********************************************************************
static void I2C_SCL_Low(uint32 u32scl_pin)
{
    // SMDrv_GPIO_BitClear(u32scl_pin);
    am_hal_gpio_out_bit_clear(u32scl_pin);
}

//**********************************************************************
// 函数功能: 设置iic SDA脚为输出高
// 输入参数: u32sda_pin: SDA pin number
// 返回参数：
//**********************************************************************
static void I2C_SDA_High(uint32 u32sda_pin)
{
    // SMDrv_GPIO_BitSet(u32sda_pin);
    am_hal_gpio_out_bit_set(u32sda_pin);
}

//**********************************************************************
// 函数功能: 设置iic SDA脚为输出低
// 输入参数: u32sda_pin: SDA pin number
// 返回参数：
//**********************************************************************
static void I2C_SDA_Low(uint32 u32sda_pin)
{
    // SMDrv_GPIO_BitClear(u32sda_pin);
    am_hal_gpio_out_bit_clear(u32sda_pin);
}

//**********************************************************************
// 函数功能: 获取iic SDA脚值
// 输入参数: u32sda_pin: SDA pin number
// 返回参数：
//**********************************************************************
static uint32 I2C_SDA_Value(uint32 u32sda_pin)
{
    // return SMDrv_GPIO_InBitRead(u32sda_pin);
    return am_hal_gpio_input_bit_read(u32sda_pin);
}
#endif

#if ((IIC0_SCL != IO_UNKNOW) && (IIC0_SDA != IO_UNKNOW))
//**********************************************************************
// 函数功能:   I2C起始信号
// 输入参数：    无
// 返回参数：    无
static void I2C_0_Start(void)
{
    I2C_SDA_Out(IIC0_SDA);
    I2C_SCL_High(IIC0_SCL);
    I2C_Delay(IIC0);
    I2C_SDA_High(IIC0_SDA);
    I2C_Delay(IIC0);
    I2C_SDA_Low(IIC0_SDA);
    I2C_Delay(IIC0);
    I2C_SCL_Low(IIC0_SCL);
    I2C_Delay(IIC0);
}

//**********************************************************************
// 函数功能:   I2C停止信号
// 输入参数：    无
// 返回参数：    无
static void I2C_0_Stop(void)
{
    I2C_SCL_Low(IIC0_SCL);
    I2C_SDA_Out(IIC0_SDA);
    I2C_SDA_Low(IIC0_SDA);
    I2C_Delay(IIC0);
    I2C_SCL_High(IIC0_SCL);
    I2C_Delay(IIC0);
    I2C_SDA_High(IIC0_SDA);
    I2C_Delay(IIC0);
}

//**********************************************************************
// 函数功能:   I2C无响应ACK
// 输入参数：    无
// 返回参数：    无
static void I2C_0_NoACK(void)
{
    I2C_SDA_High(IIC0_SDA);
    I2C_Delay(IIC0);
    I2C_SCL_High(IIC0_SCL);
    I2C_Delay(IIC0);
    I2C_SCL_Low(IIC0_SCL);
}

//**********************************************************************
// 函数功能:   I2C响应ACK
// 输入参数：    无
// 返回参数：    无
static void I2C_0_ACK(void)
{
    I2C_SDA_Low(IIC0_SDA);
    I2C_Delay(IIC0);
    I2C_SCL_High(IIC0_SCL);
    I2C_Delay(IIC0);
    I2C_SCL_Low(IIC0_SCL);
}

//**********************************************************************
// 函数功能:   I2C读取1字节
// 输入参数：    无
// 返回参数：    读取的字节
static unsigned char I2C_0_ReadByte(void)
{
    unsigned char i;
    unsigned char Data = 0;
    I2C_SDA_In(IIC0,IIC0_SDA);
    I2C_Delay(IIC0);
    for (i = 0; i < 8; i++)
    {
        I2C_SCL_Low(IIC0_SCL);
        I2C_Delay(IIC0);
        I2C_SCL_High(IIC0_SCL);
        I2C_Delay(IIC0);
        if (I2C_SDA_Value(IIC0_SDA) == 1)
        {
            Data = (Data << 1) + 0x01;
        }
        else
        {
            Data = (Data << 1) + 0x00;
        }
    }
    I2C_SCL_Low(IIC0_SCL);
    I2C_SDA_Out(IIC0_SDA);
    I2C_Delay(IIC0);
    return (Data);
}

//**********************************************************************
// 函数功能:   I2C写1字节
// 输入参数：    
// Data     ： 向从设备写的字节
// 返回参数：    
// 0x00    :    写成功
// 0xff    :    写失败
static unsigned short I2C_0_WriteByte(unsigned char Data)
{
    unsigned char i;
    I2C_SDA_Out(IIC0_SDA);
    for (i = 0; i < 8; i++)
    {
        if ((Data & 0x80) == 0x80)
        {
            I2C_SDA_High(IIC0_SDA);
        }
        else
        {
            I2C_SDA_Low(IIC0_SDA);
        }
        I2C_Delay(IIC0);
        I2C_SCL_High(IIC0_SCL);
        I2C_Delay(IIC0);
        I2C_SCL_Low(IIC0_SCL);
        Data <<= 0x01;
    }
    //      End of Write A Byte
    //      Wait "ACK" Signal
    I2C_SDA_In(IIC0,IIC0_SDA);
    I2C_Delay(IIC0);
    I2C_SCL_High(IIC0_SCL);
    I2C_Delay(IIC0);

    while(1)
    {
        if(I2C_SDA_Value(IIC0_SDA) == 1)
        {
            if ((--i) == 0)
            {
                //no ack indicate
                I2C_SDA_Out(IIC0_SDA);
                I2C_SDA_High(IIC0_SDA);
                I2C_Delay(IIC0);
                return (0xFF);
            }
        }
        else
        {
            break;
        }
    }
    I2C_SCL_Low(IIC0_SCL);
    I2C_SDA_Out(IIC0_SDA);
    I2C_SDA_High(IIC0_SDA);
    I2C_Delay(IIC0);
    return (0x00);   // Write OK
}

//**********************************************************************
// 函数功能:   I2C写N字节
// 输入参数：    
// dataAddr：  向从设备数据指针
// length  ：  数据长度
// 返回参数：    
// 0x00    :    写成功
// 0xff    :    写失败
static unsigned short I2C_0_WriteBytes(unsigned char* dataAddr, unsigned short length)
{
    unsigned short i;
    unsigned short ret = 0;
    for(i = 0; i < length; i++)
    {
        ret = I2C_0_WriteByte(dataAddr[i]);
        if(ret == 0xff)
        {
            break;
        }
    }
    return ret;
}

//**********************************************************************
// 函数功能:   I2C读N字节
// 输入参数：    
// dataAddr：  数据指针
// length  ：  数据长度
// 返回参数：   无
static void I2C_0_ReadBytes(unsigned char *dataAddr, unsigned short length)
{
    unsigned short i;
    for(i = 0; i < length; i++)
    {
        dataAddr[i] = I2C_0_ReadByte();
        if(i < (length - 1))
        {
            I2C_0_ACK();
        }
        else
        {
            I2C_0_NoACK();
        }
    }
}
#endif

#if ((IIC1_SCL != IO_UNKNOW) && (IIC1_SDA != IO_UNKNOW))
//**********************************************************************
// 函数功能:   I2C起始信号
// 输入参数：    无
// 返回参数：    无
static void I2C_1_Start(void)
{
    I2C_SDA_Out(IIC1_SDA);
    I2C_SCL_High(IIC1_SCL);
    I2C_Delay(IIC1);
    I2C_SDA_High(IIC1_SDA);
    I2C_Delay(IIC1);
    I2C_SDA_Low(IIC1_SDA);
    I2C_Delay(IIC1);
    I2C_SCL_Low(IIC1_SCL);
    I2C_Delay(IIC1);
}

//**********************************************************************
// 函数功能:   I2C停止信号
// 输入参数：    无
// 返回参数：    无
static void I2C_1_Stop(void)
{
    I2C_SCL_Low(IIC1_SCL);
    I2C_SDA_Out(IIC1_SDA);
    I2C_SDA_Low(IIC1_SDA);
    I2C_Delay(IIC1);
    I2C_SCL_High(IIC1_SCL);
    I2C_Delay(IIC1);
    I2C_SDA_High(IIC1_SDA);
    I2C_Delay(IIC1);
}

//**********************************************************************
// 函数功能:   I2C无响应ACK
// 输入参数：    无
// 返回参数：    无
static void I2C_1_NoACK(void)
{
    I2C_SDA_High(IIC1_SDA);
    I2C_Delay(IIC1);
    I2C_SCL_High(IIC1_SCL);
    I2C_Delay(IIC1);
    I2C_SCL_Low(IIC1_SCL);
}

//**********************************************************************
// 函数功能:   I2C响应ACK
// 输入参数：    无
// 返回参数：    无
static void I2C_1_ACK(void)
{
    I2C_SDA_Low(IIC1_SDA);
    I2C_Delay(IIC1);
    I2C_SCL_High(IIC1_SCL);
    I2C_Delay(IIC1);
    I2C_SCL_Low(IIC1_SCL);
}

//**********************************************************************
// 函数功能:   I2C读取1字节
// 输入参数：    无
// 返回参数：    读取的字节
static unsigned char I2C_1_ReadByte(void)
{
    unsigned char i;
    unsigned char Data = 0;
    I2C_SDA_In(IIC1,IIC1_SDA);
    I2C_Delay(IIC1);
    for (i = 0; i < 8; i++)
    {
        I2C_SCL_Low(IIC1_SCL);
        I2C_Delay(IIC1);
        I2C_SCL_High(IIC1_SCL);
        I2C_Delay(IIC1);
        if (I2C_SDA_Value(IIC1_SDA) == 1)
        {
            Data = (Data << 1) + 0x01;
        }
        else
        {
            Data = (Data << 1) + 0x00;
        }
    }
    I2C_SCL_Low(IIC1_SCL);
    I2C_SDA_Out(IIC1_SDA);
    I2C_Delay(IIC1);
    return (Data);
}

//**********************************************************************
// 函数功能:   I2C写1字节
// 输入参数：    
// Data     ： 向从设备写的字节
// 返回参数：    
// 0x00    :    写成功
// 0xff    :    写失败
static unsigned short I2C_1_WriteByte(unsigned char Data)
{
    unsigned char i;
    I2C_SDA_Out(IIC1_SDA);
    for (i = 0; i < 8; i++)
    {
        if ((Data & 0x80) == 0x80)
        {
            I2C_SDA_High(IIC1_SDA);
        }
        else
        {
            I2C_SDA_Low(IIC1_SDA);
        }
        I2C_Delay(IIC1);
        I2C_SCL_High(IIC1_SCL);
        I2C_Delay(IIC1);
        I2C_SCL_Low(IIC1_SCL);
        Data <<= 0x01;
    }
    //      End of Write A Byte
    //      Wait "ACK" Signal
    I2C_SDA_In(IIC1,IIC1_SDA);
    I2C_Delay(IIC1);
    I2C_SCL_High(IIC1_SCL);
    I2C_Delay(IIC1);

    while(1)
    {
        if(I2C_SDA_Value(IIC1_SDA) == 1)
        {
            if ((--i) == 0)
            {
                //no ack indicate
                I2C_SDA_Out(IIC1_SDA);
                I2C_SDA_High(IIC1_SDA);
                I2C_Delay(IIC1);
                return (0xFF);
            }
        }
        else
        {
            break;
        }
    }
    I2C_SCL_Low(IIC1_SCL);
    I2C_SDA_Out(IIC1_SDA);
    I2C_SDA_High(IIC1_SDA);
    I2C_Delay(IIC1);
    return (0x00);   // Write OK
}

//**********************************************************************
// 函数功能:   I2C写N字节
// 输入参数：    
// dataAddr：  向从设备数据指针
// length  ：  数据长度
// 返回参数：    
// 0x00    :    写成功
// 0xff    :    写失败
static unsigned short I2C_1_WriteBytes(unsigned char* dataAddr, unsigned short length)
{
    unsigned short i;
    unsigned short ret = 0;
    for(i = 0; i < length; i++)
    {
        ret = I2C_1_WriteByte(dataAddr[i]);
        if(ret == 0xff)
        {
            break;
        }
    }
    return ret;
}

//**********************************************************************
// 函数功能:   I2C读N字节
// 输入参数：    
// dataAddr：  数据指针
// length  ：  数据长度
// 返回参数：   无
static void I2C_1_ReadBytes(unsigned char *dataAddr, unsigned short length)
{
    unsigned short i;
    for(i = 0; i < length; i++)
    {
        dataAddr[i] = I2C_1_ReadByte();
        if(i < (length - 1))
        {
            I2C_1_ACK();
        }
        else
        {
            I2C_1_NoACK();
        }
    }
}
#endif

#if ((IIC2_SCL != IO_UNKNOW) && (IIC2_SDA != IO_UNKNOW))
//**********************************************************************
// 函数功能:   I2C起始信号
// 输入参数：    无
// 返回参数：    无
static void I2C_2_Start(void)
{
    I2C_SDA_Out(IIC2_SDA);
    I2C_SCL_High(IIC2_SCL);
    I2C_Delay(IIC2);
    I2C_SDA_High(IIC2_SDA);
    I2C_Delay(IIC2);
    I2C_SDA_Low(IIC2_SDA);
    I2C_Delay(IIC2);
    I2C_SCL_Low(IIC2_SCL);
    I2C_Delay(IIC2);
}

//**********************************************************************
// 函数功能:   I2C停止信号
// 输入参数：    无
// 返回参数：    无
static void I2C_2_Stop(void)
{
    I2C_SCL_Low(IIC2_SCL);
    I2C_SDA_Out(IIC2_SDA);
    I2C_SDA_Low(IIC2_SDA);
    I2C_Delay(IIC2);
    I2C_SCL_High(IIC2_SCL);
    I2C_Delay(IIC2);
    I2C_SDA_High(IIC2_SDA);
    I2C_Delay(IIC2);
}

//**********************************************************************
// 函数功能:   I2C无响应ACK
// 输入参数：    无
// 返回参数：    无
static void I2C_2_NoACK(void)
{
    I2C_SDA_High(IIC2_SDA);
    I2C_Delay(IIC2);
    I2C_SCL_High(IIC2_SCL);
    I2C_Delay(IIC2);
    I2C_SCL_Low(IIC2_SCL);
}

//**********************************************************************
// 函数功能:   I2C响应ACK
// 输入参数：    无
// 返回参数：    无
static void I2C_2_ACK(void)
{
    I2C_SDA_Low(IIC2_SDA);
    I2C_Delay(IIC2);
    I2C_SCL_High(IIC2_SCL);
    I2C_Delay(IIC2);
    I2C_SCL_Low(IIC2_SCL);
}

//**********************************************************************
// 函数功能:   I2C读取1字节
// 输入参数：    无
// 返回参数：    读取的字节
static unsigned char I2C_2_ReadByte(void)
{
    unsigned char i;
    unsigned char Data = 0;
    I2C_SDA_In(IIC2,IIC2_SDA);
    I2C_Delay(IIC2);
    for (i = 0; i < 8; i++)
    {
        I2C_SCL_Low(IIC2_SCL);
        I2C_Delay(IIC2);
        I2C_SCL_High(IIC2_SCL);
        I2C_Delay(IIC2);
        if (I2C_SDA_Value(IIC2_SDA) == 1)
        {
            Data = (Data << 1) + 0x01;
        }
        else
        {
            Data = (Data << 1) + 0x00;
        }
    }
    I2C_SCL_Low(IIC2_SCL);
    I2C_SDA_Out(IIC2_SDA);
    I2C_Delay(IIC2);
    return (Data);
}

//**********************************************************************
// 函数功能:   I2C写1字节
// 输入参数：    
// Data     ： 向从设备写的字节
// 返回参数：    
// 0x00    :    写成功
// 0xff    :    写失败
static unsigned short I2C_2_WriteByte(unsigned char Data)
{
    unsigned char i;
    I2C_SDA_Out(IIC2_SDA);
    for (i = 0; i < 8; i++)
    {
        if ((Data & 0x80) == 0x80)
        {
            I2C_SDA_High(IIC2_SDA);
        }
        else
        {
            I2C_SDA_Low(IIC2_SDA);
        }
        I2C_Delay(IIC2);
        I2C_SCL_High(IIC2_SCL);
        I2C_Delay(IIC2);
        I2C_SCL_Low(IIC2_SCL);
        Data <<= 0x01;
    }
    //      End of Write A Byte
    //      Wait "ACK" Signal
    I2C_SDA_In(IIC2,IIC2_SDA);
    I2C_Delay(IIC2);
    I2C_SCL_High(IIC2_SCL);
    I2C_Delay(IIC2);

    while(1)
    {
        if(I2C_SDA_Value(IIC2_SDA) == 1)
        {
            if ((--i) == 0)
            {
                       //no ack indicate
                I2C_SDA_Out(IIC2_SDA);
                I2C_SDA_High(IIC2_SDA);
                I2C_Delay(IIC2);
                return (0xFF);
            }
        }
        else
        {
            break;
        }
    }
    I2C_SCL_Low(IIC2_SCL);
    I2C_SDA_Out(IIC2_SDA);
    I2C_SDA_High(IIC2_SDA);
    I2C_Delay(IIC2);
    return (0x00);   // Write OK
}

//**********************************************************************
// 函数功能:   I2C写N字节
// 输入参数：    
// dataAddr：  向从设备数据指针
// length  ：  数据长度
// 返回参数：    
// 0x00    :    写成功
// 0xff    :    写失败
static unsigned short I2C_2_WriteBytes(unsigned char* dataAddr, unsigned short length)
{
    unsigned short i;
    unsigned short ret = 0;
    for(i = 0; i < length; i++)
    {
        ret = I2C_2_WriteByte(dataAddr[i]);
        if(ret == 0xff)
        {
            break;
        }
    }
    return ret;
}

//**********************************************************************
// 函数功能:   I2C读N字节
// 输入参数：    
// dataAddr：  数据指针
// length  ：  数据长度
// 返回参数：   无
static void I2C_2_ReadBytes(unsigned char *dataAddr, unsigned short length)
{
    unsigned short i;
    for(i = 0; i < length; i++)
    {
        dataAddr[i] = I2C_2_ReadByte();
        if(i < (length - 1))
        {
            I2C_2_ACK();
        }
        else
        {
            I2C_2_NoACK();
        }
    }
}
#endif

#if ((IIC3_SCL != IO_UNKNOW) && (IIC3_SDA != IO_UNKNOW))
//**********************************************************************
// 函数功能:   I2C起始信号
// 输入参数：    无
// 返回参数：    无
static void I2C_3_Start(void)
{
    I2C_SDA_Out(IIC3_SDA);
    I2C_SCL_High(IIC3_SCL);
    I2C_Delay(IIC3);
    I2C_SDA_High(IIC3_SDA);
    I2C_Delay(IIC3);
    I2C_SDA_Low(IIC3_SDA);
    I2C_Delay(IIC3);
    I2C_SCL_Low(IIC3_SCL);
    I2C_Delay(IIC3);
}

//**********************************************************************
// 函数功能:   I2C停止信号
// 输入参数：    无
// 返回参数：    无
static void I2C_3_Stop(void)
{
    I2C_SCL_Low(IIC3_SCL);
    I2C_SDA_Out(IIC3_SDA);
    I2C_SDA_Low(IIC3_SDA);
    I2C_Delay(IIC3);
    I2C_SCL_High(IIC3_SCL);
    I2C_Delay(IIC3);
    I2C_SDA_High(IIC3_SDA);
    I2C_Delay(IIC3);
}

//**********************************************************************
// 函数功能:   I2C无响应ACK
// 输入参数：    无
// 返回参数：    无
static void I2C_3_NoACK(void)
{
    I2C_SDA_High(IIC3_SDA);
    I2C_Delay(IIC3);
    I2C_SCL_High(IIC3_SCL);
    I2C_Delay(IIC3);
    I2C_SCL_Low(IIC3_SCL);
}

//**********************************************************************
// 函数功能:   I2C响应ACK
// 输入参数：    无
// 返回参数：    无
static void I2C_3_ACK(void)
{
    I2C_SDA_Low(IIC3_SDA);
    I2C_Delay(IIC3);
    I2C_SCL_High(IIC3_SCL);
    I2C_Delay(IIC3);
    I2C_SCL_Low(IIC3_SCL);
}

//**********************************************************************
// 函数功能:   I2C读取1字节
// 输入参数：    无
// 返回参数：    读取的字节
static unsigned char I2C_3_ReadByte(void)
{
    unsigned char i;
    unsigned char Data = 0;
    I2C_SDA_In(IIC3,IIC3_SDA);
    I2C_Delay(IIC3);
    for (i = 0; i < 8; i++)
    {
        I2C_SCL_Low(IIC3_SCL);
        I2C_Delay(IIC3);
        I2C_SCL_High(IIC3_SCL);
        I2C_Delay(IIC3);
        if (I2C_SDA_Value(IIC3_SDA) == 1)
        {
            Data = (Data << 1) + 0x01;
        }
        else
        {
            Data = (Data << 1) + 0x00;
        }
    }
    I2C_SCL_Low(IIC3_SCL);
    I2C_SDA_Out(IIC3_SDA);
    I2C_Delay(IIC3);
    return (Data);
}

//**********************************************************************
// 函数功能:   I2C写1字节
// 输入参数：    
// Data     ： 向从设备写的字节
// 返回参数：    
// 0x00    :    写成功
// 0xff    :    写失败
static unsigned short I2C_3_WriteByte(unsigned char Data)
{
    unsigned char i;
    I2C_SDA_Out(IIC3_SDA);
    for (i = 0; i < 8; i++)
    {
        if ((Data & 0x80) == 0x80)
        {
            I2C_SDA_High(IIC3_SDA);
        }
        else
        {
            I2C_SDA_Low(IIC3_SDA);
        }
        I2C_Delay(IIC3);
        I2C_SCL_High(IIC3_SCL);
        I2C_Delay(IIC3);
        I2C_SCL_Low(IIC3_SCL);
        Data <<= 0x01;
    }
    //      End of Write A Byte
    //      Wait "ACK" Signal
    I2C_SDA_In(IIC3,IIC3_SDA);
    I2C_Delay(IIC3);
    I2C_SCL_High(IIC3_SCL);
    I2C_Delay(IIC3);

    while(1)
    {
        if(I2C_SDA_Value(IIC3_SDA) == 1)
        {
            if ((--i) == 0)
            {
                //no ack indicate
                I2C_SDA_Out(IIC3_SDA);
                I2C_SDA_High(IIC3_SDA);
                I2C_Delay(IIC3);
                return (0xFF);
            }
        }
        else
        {
            break;
        }
    }
    I2C_SCL_Low(IIC3_SCL);
    I2C_SDA_Out(IIC3_SDA);
    I2C_SDA_High(IIC3_SDA);
    I2C_Delay(IIC3);
    return (0x00);   // Write OK
}

//**********************************************************************
// 函数功能:   I2C写N字节
// 输入参数：    
// dataAddr：  向从设备数据指针
// length  ：  数据长度
// 返回参数：    
// 0x00    :    写成功
// 0xff    :    写失败
static unsigned short I2C_3_WriteBytes(unsigned char* dataAddr, unsigned short length)
{
    unsigned short i;
    unsigned short ret = 0;
    for(i = 0; i < length; i++)
    {
        ret = I2C_3_WriteByte(dataAddr[i]);
        if(ret == 0xff)
        {
            break;
        }
    }
    return ret;
}

//**********************************************************************
// 函数功能:   I2C读N字节
// 输入参数：    
// dataAddr：  数据指针
// length  ：  数据长度
// 返回参数：   无
static void I2C_3_ReadBytes(unsigned char *dataAddr, unsigned short length)
{
    unsigned short i;
    for(i = 0; i < length; i++)
    {
        dataAddr[i] = I2C_3_ReadByte();
        if(i < (length - 1))
        {
            I2C_3_ACK();
        }
        else
        {
            I2C_3_NoACK();
        }
    }
}
#endif

#if ((IIC4_SCL != IO_UNKNOW) && (IIC4_SDA != IO_UNKNOW))
//**********************************************************************
// 函数功能:   I2C起始信号
// 输入参数：    无
// 返回参数：    无
static void I2C_4_Start(void)
{
    I2C_SDA_Out(IIC4_SDA);
    I2C_SCL_High(IIC4_SCL);
    I2C_Delay(IIC4);
    I2C_SDA_High(IIC4_SDA);
    I2C_Delay(IIC4);
    I2C_SDA_Low(IIC4_SDA);
    I2C_Delay(IIC4);
    I2C_SCL_Low(IIC4_SCL);
    I2C_Delay(IIC4);
}

//**********************************************************************
// 函数功能:   I2C停止信号
// 输入参数：    无
// 返回参数：    无
static void I2C_4_Stop(void)
{
    I2C_SCL_Low(IIC4_SCL);
    I2C_SDA_Out(IIC4_SDA);
    I2C_SDA_Low(IIC4_SDA);
    I2C_Delay(IIC4);
    I2C_SCL_High(IIC4_SCL);
    I2C_Delay(IIC4);
    I2C_SDA_High(IIC4_SDA);
    I2C_Delay(IIC4);
}

//**********************************************************************
// 函数功能:   I2C无响应ACK
// 输入参数：    无
// 返回参数：    无
static void I2C_4_NoACK(void)
{
    I2C_SDA_High(IIC4_SDA);
    I2C_Delay(IIC4);
    I2C_SCL_High(IIC4_SCL);
    I2C_Delay(IIC4);
    I2C_SCL_Low(IIC4_SCL);
}

//**********************************************************************
// 函数功能:   I2C响应ACK
// 输入参数：    无
// 返回参数：    无
static void I2C_4_ACK(void)
{
    I2C_SDA_Low(IIC4_SDA);
    I2C_Delay(IIC4);
    I2C_SCL_High(IIC4_SCL);
    I2C_Delay(IIC4);
    I2C_SCL_Low(IIC4_SCL);
}

//**********************************************************************
// 函数功能:   I2C读取1字节
// 输入参数：    无
// 返回参数：    读取的字节
static unsigned char I2C_4_ReadByte(void)
{
    unsigned char i;
    unsigned char Data = 0;
    I2C_SDA_In(IIC4,IIC4_SDA);
    I2C_Delay(IIC4);
    for (i = 0; i < 8; i++)
    {
        I2C_SCL_Low(IIC4_SCL);
        I2C_Delay(IIC4);
        I2C_SCL_High(IIC4_SCL);
        I2C_Delay(IIC4);
        if (I2C_SDA_Value(IIC4_SDA) == 1)
        {
            Data = (Data << 1) + 0x01;
        }
        else
        {
            Data = (Data << 1) + 0x00;
        }
    }
    I2C_SCL_Low(IIC4_SCL);
    I2C_SDA_Out(IIC4_SDA);
    I2C_Delay(IIC4);
    return (Data);
}

//**********************************************************************
// 函数功能:   I2C写1字节
// 输入参数：    
// Data     ： 向从设备写的字节
// 返回参数：    
// 0x00    :    写成功
// 0xff    :    写失败
static unsigned short I2C_4_WriteByte(unsigned char Data)
{
    unsigned char i;
    I2C_SDA_Out(IIC4_SDA);
    for (i = 0; i < 8; i++)
    {
        if ((Data & 0x80) == 0x80)
        {
            I2C_SDA_High(IIC4_SDA);
        }
        else
        {
            I2C_SDA_Low(IIC4_SDA);
        }
        I2C_Delay(IIC4);
        I2C_SCL_High(IIC4_SCL);
        I2C_Delay(IIC4);
        I2C_SCL_Low(IIC4_SCL);
        Data <<= 0x01;
    }
    //      End of Write A Byte
    //      Wait "ACK" Signal
    I2C_SDA_In(IIC4,IIC4_SDA);
    I2C_Delay(IIC4);
    I2C_SCL_High(IIC4_SCL);
    I2C_Delay(IIC4);

    while(1)
    {
        if(I2C_SDA_Value(IIC4_SDA) == 1)
        {
            if ((--i) == 0)
            {
                       //no ack indicate
                I2C_SDA_Out(IIC4_SDA);
                I2C_SDA_High(IIC4_SDA);
                I2C_Delay(IIC4);
                return (0xFF);
            }
        }
        else
        {
            break;
        }
    }
    I2C_SCL_Low(IIC4_SCL);
    I2C_SDA_Out(IIC4_SDA);
    I2C_SDA_High(IIC4_SDA);
    I2C_Delay(IIC4);
    return (0x00);   // Write OK
}

//**********************************************************************
// 函数功能:   I2C写N字节
// 输入参数：    
// dataAddr：  向从设备数据指针
// length  ：  数据长度
// 返回参数：    
// 0x00    :    写成功
// 0xff    :    写失败
static unsigned short I2C_4_WriteBytes(unsigned char* dataAddr, unsigned short length)
{
    unsigned short i;
    unsigned short ret = 0;
    for(i = 0; i < length; i++)
    {
        ret = I2C_4_WriteByte(dataAddr[i]);
        if(ret == 0xff)
        {
            break;
        }
    }
    return ret;
}

//**********************************************************************
// 函数功能:   I2C读N字节
// 输入参数：    
// dataAddr：  数据指针
// length  ：  数据长度
// 返回参数：   无
static void I2C_4_ReadBytes(unsigned char *dataAddr, unsigned short length)
{
    unsigned short i;
    for(i = 0; i < length; i++)
    {
        dataAddr[i] = I2C_4_ReadByte();
        if(i < (length - 1))
        {
            I2C_4_ACK();
        }
        else
        {
            I2C_4_NoACK();
        }
    }
}
#endif

#if ((IIC5_SCL != IO_UNKNOW) && (IIC5_SDA != IO_UNKNOW))
//**********************************************************************
// 函数功能:   I2C起始信号
// 输入参数：    无
// 返回参数：    无
static void I2C_5_Start(void)
{
    I2C_SDA_Out(IIC5_SDA);
    I2C_SCL_High(IIC5_SCL);
    I2C_Delay(IIC5);
    I2C_SDA_High(IIC5_SDA);
    I2C_Delay(IIC5);
    I2C_SDA_Low(IIC5_SDA);
    I2C_Delay(IIC5);
    I2C_SCL_Low(IIC5_SCL);
    I2C_Delay(IIC5);
}

//**********************************************************************
// 函数功能:   I2C停止信号
// 输入参数：    无
// 返回参数：    无
static void I2C_5_Stop(void)
{
    I2C_SCL_Low(IIC5_SCL);
    I2C_SDA_Out(IIC5_SDA);
    I2C_SDA_Low(IIC5_SDA);
    I2C_Delay(IIC5);
    I2C_SCL_High(IIC5_SCL);
    I2C_Delay(IIC5);
    I2C_SDA_High(IIC5_SDA);
    I2C_Delay(IIC5);
}

//**********************************************************************
// 函数功能:   I2C无响应ACK
// 输入参数：    无
// 返回参数：    无
static void I2C_5_NoACK(void)
{
    I2C_SDA_High(IIC5_SDA);
    I2C_Delay(IIC5);
    I2C_SCL_High(IIC5_SCL);
    I2C_Delay(IIC5);
    I2C_SCL_Low(IIC5_SCL);
}

//**********************************************************************
// 函数功能:   I2C响应ACK
// 输入参数：    无
// 返回参数：    无
static void I2C_5_ACK(void)
{
    I2C_SDA_Low(IIC5_SDA);
    I2C_Delay(IIC5);
    I2C_SCL_High(IIC5_SCL);
    I2C_Delay(IIC5);
    I2C_SCL_Low(IIC5_SCL);
}

//**********************************************************************
// 函数功能:   I2C读取1字节
// 输入参数：    无
// 返回参数：    读取的字节
static unsigned char I2C_5_ReadByte(void)
{
    unsigned char i;
    unsigned char Data = 0;
    I2C_SDA_In(IIC5,IIC5_SDA);
    I2C_Delay(IIC5);
    for (i = 0; i < 8; i++)
    {
        I2C_SCL_Low(IIC5_SCL);
        I2C_Delay(IIC5);
        I2C_SCL_High(IIC5_SCL);
        I2C_Delay(IIC5);
        if (I2C_SDA_Value(IIC5_SDA) == 1)
        {
            Data = (Data << 1) + 0x01;
        }
        else
        {
            Data = (Data << 1) + 0x00;
        }
    }
    I2C_SCL_Low(IIC5_SCL);
    I2C_SDA_Out(IIC5_SDA);
    I2C_Delay(IIC5);
    return (Data);
}

//**********************************************************************
// 函数功能:   I2C写1字节
// 输入参数：    
// Data     ： 向从设备写的字节
// 返回参数：    
// 0x00    :    写成功
// 0xff    :    写失败
static unsigned short I2C_5_WriteByte(unsigned char Data)
{
    unsigned char i;
    I2C_SDA_Out(IIC5_SDA);
    for (i = 0; i < 8; i++)
    {
        if ((Data & 0x80) == 0x80)
        {
            I2C_SDA_High(IIC5_SDA);
        }
        else
        {
            I2C_SDA_Low(IIC5_SDA);
        }
        I2C_Delay(IIC5);
        I2C_SCL_High(IIC5_SCL);
        I2C_Delay(IIC5);
        I2C_SCL_Low(IIC5_SCL);
        Data <<= 0x01;
    }
    //      End of Write A Byte
    //      Wait "ACK" Signal
    I2C_SDA_In(IIC5,IIC5_SDA);
    I2C_Delay(IIC5);
    I2C_SCL_High(IIC5_SCL);
    I2C_Delay(IIC5);

    while(1)
    {
        if(I2C_SDA_Value(IIC5_SDA) == 1)
        {
            if ((--i) == 0)
            {
                       //no ack indicate
                I2C_SDA_Out(IIC5_SDA);
                I2C_SDA_High(IIC5_SDA);
                I2C_Delay(IIC5);
                return (0xFF);
            }
        }
        else
        {
            break;
        }
    }
    I2C_SCL_Low(IIC5_SCL);
    I2C_SDA_Out(IIC5_SDA);
    I2C_SDA_High(IIC5_SDA);
    I2C_Delay(IIC5);
    return (0x00);   // Write OK
}

//**********************************************************************
// 函数功能:   I2C写N字节
// 输入参数：    
// dataAddr：  向从设备数据指针
// length  ：  数据长度
// 返回参数：    
// 0x00    :    写成功
// 0xff    :    写失败
static unsigned short I2C_5_WriteBytes(unsigned char* dataAddr, unsigned short length)
{
    unsigned short i;
    unsigned short ret = 0;
    for(i = 0; i < length; i++)
    {
        ret = I2C_5_WriteByte(dataAddr[i]);
        if(ret == 0xff)
        {
            break;
        }
    }
    return ret;
}

//**********************************************************************
// 函数功能:   I2C读N字节
// 输入参数：    
// dataAddr：  数据指针
// length  ：  数据长度
// 返回参数：   无
static void I2C_5_ReadBytes(unsigned char *dataAddr, unsigned short length)
{
    unsigned short i;
    for(i = 0; i < length; i++)
    {
        dataAddr[i] = I2C_5_ReadByte();
        if(i < (length - 1))
        {
            I2C_5_ACK();
        }
        else
        {
            I2C_5_NoACK();
        }
    }
}
#endif

//**********************************************************************
// 函数功能: 初始化iic
// 输入参数：	
// 返回参数：
//**********************************************************************
void SMDrv_SWI2C_Init(void)
{    
    memset(swiic_info,0x00,sizeof(swiic_info));
}

//**********************************************************************
// 函数功能: 根据driver module ID打开硬件对应的iic
// 输入参数：	
//    modul: driver module ID,值参考iic_module
//    u32speed: IIC速度设置，值参考IIC_SPEED_XXX
// 返回参数：Ret_InvalidParam或Ret_OK
//**********************************************************************
ret_type SMDrv_SWI2C_Open(iic_module modul,uint32 u32speed)
{
    uint8 u8iic_id = IIC_MAX;
    uint32 u32scl_pin,u32sda_pin;

    switch(modul)
    {
    case HR_IIC_MODULE:     //心率 iic4
        u8iic_id = IIC4;
        u32scl_pin=IIC4_SCL;
        u32sda_pin=IIC4_SDA;
        break;
    case AP_IIC_MODULE:     //气压和bat共用 iic1
    case BAT_IIC_MODULE:
        u8iic_id = IIC1;
        u32scl_pin=IIC1_SCL;
        u32sda_pin=IIC1_SDA;
        break;
    case MAG_IIC_MODULE:    //地磁 iic0
        u8iic_id = IIC0;
        u32scl_pin=IIC0_SCL;
        u32sda_pin=IIC0_SDA;
        break;
    case ACC_IIC_MODULE:   
    case GYR_IIC_MODULE:
        u8iic_id = IIC2;
        u32scl_pin=IIC2_SCL;
        u32sda_pin=IIC2_SDA;
        break;
    default:
        return Ret_InvalidParam;
    }

    if(swiic_info[u8iic_id].bOpen == FALSE)  //只有u8iic_id IIC口未open过的情况下，才做open
    {
        //设置iic速度
        swiic_info[u8iic_id].u8speed = (uint8)u32speed;
    
        //设置SCL和SDA pin
        I2C_SCL_Out(u32scl_pin);
        I2C_SDA_In(u8iic_id,u32sda_pin);
        //SCL设置输出高
        I2C_SCL_High(u32scl_pin);
        
        swiic_info[u8iic_id].bOpen = TRUE;   //标记u8iic_id iic口已经open
    }
    swiic_info[u8iic_id].u32module = (1 << modul); //标记模块已经open
    return Ret_OK;
}

//**********************************************************************
// 函数功能: 关闭driver module ID硬件对应的IIC,以实现低功耗
// 输入参数：	
//    modul: driver module ID,值参考iic_module
// 返回参数：Ret_InvalidParam或Ret_OK
//**********************************************************************
ret_type SMDrv_SWI2C_Close(iic_module modul)
{
    uint8 u8iic_id = IIC_MAX;
    uint32 u32scl_pin,u32sda_pin;
    
    switch(modul)
    {
    case HR_IIC_MODULE:     //心率 iic4
        u8iic_id = IIC4;
        u32scl_pin=IIC4_SCL;
        u32sda_pin=IIC4_SDA;
        break;
    case AP_IIC_MODULE:     //气压和bat共用 iic1
    case BAT_IIC_MODULE:
        u8iic_id = IIC1;
        u32scl_pin=IIC1_SCL;
        u32sda_pin=IIC1_SDA;
        break;
    case MAG_IIC_MODULE:   //地磁 iic0
        u8iic_id = IIC0;
        u32scl_pin=IIC0_SCL;
        u32sda_pin=IIC0_SDA;
    break;
    case ACC_IIC_MODULE:    //加速计和陀螺仪共用 iic5
    case GYR_IIC_MODULE:
        u8iic_id = IIC2;
        u32scl_pin=IIC2_SCL;
        u32sda_pin=IIC2_SDA;
        break;
    default:
        return Ret_InvalidParam;
    }

    swiic_info[u8iic_id].u32module &= (~(1 << modul)); //清除模块使用iic口标记
    if(swiic_info[u8iic_id].u32module == 0x00)         //已经没有模块使用iic口，则将iic口IO关闭，进入低功耗模式
    {
        I2C_SCL_Out(u32scl_pin);
        I2C_SDA_Out(u32sda_pin);
        
        //SCL设置输出高
        I2C_SCL_High(u32scl_pin);
        I2C_SDA_High(u32sda_pin);
        swiic_info[u8iic_id].bOpen = FALSE;    //清除iic口已open标记，表明已无模块使用此iic口
    }
    return Ret_OK;
}

//**********************************************************************
// 函数功能: I2C写命令
// 输入参数：	
//    modul: driver module ID,值参考iic_module
//    deviceAddr：  从设备地址
//    regAddr  ：  命令或偏移地址
// 返回参数：Ret_InvalidParam或Ret_OK
//**********************************************************************
ret_type SMDrv_SWI2C_WriteCmd(iic_module modul,uint8 deviceAddr, uint8 regAddr)
{
    switch(modul)
    {
    case HR_IIC_MODULE:     //心率 iic4
        if(swiic_info[IIC4].bOpen == FALSE)  //IIC已经关闭，不能做读写操作
            return Ret_NoInit;
        I2C_4_Start();
        I2C_4_WriteByte(deviceAddr);
        I2C_4_WriteByte(regAddr);
        I2C_4_Stop();
        break;
	
    case AP_IIC_MODULE:      //气压和bat共用 iic1
    case BAT_IIC_MODULE:
        if(swiic_info[IIC1].bOpen == FALSE)  //IIC已经关闭，不能做读写操作
            return Ret_NoInit;
        I2C_1_Start();
        I2C_1_WriteByte(deviceAddr);
        I2C_1_WriteByte(regAddr);
        I2C_1_Stop();
        break;
    #if 0
    case MAG_IIC_MODULE:    //地磁 iic0
        if(swiic_info[IIC0].bOpen == FALSE)  //IIC已经关闭，不能做读写操作
            return Ret_NoInit;
        I2C_0_Start();
        I2C_0_WriteByte(deviceAddr);
        I2C_0_WriteByte(regAddr);
        I2C_0_Stop();
        break;
    #endif
    case ACC_IIC_MODULE:   //加速计和陀螺仪共用 iic5
    case GYR_IIC_MODULE:
        if(swiic_info[IIC2].bOpen == FALSE)  //IIC已经关闭，不能做读写操作
            return Ret_NoInit;
        I2C_2_Start();
        I2C_2_WriteByte(deviceAddr);
        I2C_2_WriteByte(regAddr);
        I2C_2_Stop();
        break;
    default:
        return Ret_InvalidParam;
    }
    return Ret_OK;
}

//**********************************************************************
// 函数功能:   I2C写操作
// 输入参数：
// modul: driver module ID,值参考iic_module
// deviceAddr：  从设备地址
// regAddr  ：  命令或偏移地址
// data_Point： 数据指针
// length   ：  数据长度
// 返回参数：   Ret_InvalidParam或Ret_OK
ret_type SMDrv_SWI2C_Write(iic_module modul,uint8 deviceAddr, uint8 regAddr, uint8* data_Point, uint16 length)
{
    switch(modul)
    {
    case HR_IIC_MODULE:     //心率 iic4
        if(swiic_info[IIC4].bOpen == FALSE)  //IIC已经关闭，不能做读写操作
            return Ret_NoInit;
        I2C_4_Start();
        I2C_4_WriteByte(deviceAddr);
        I2C_4_WriteByte(regAddr);
        I2C_4_WriteBytes(data_Point, length);
        I2C_4_Stop();
        break;
    case AP_IIC_MODULE:		   //气压和bat共用 iic1
    case BAT_IIC_MODULE:
        if(swiic_info[IIC1].bOpen == FALSE)  //IIC已经关闭，不能做读写操作
            return Ret_NoInit;
        I2C_1_Start();
        I2C_1_WriteByte(deviceAddr);
        I2C_1_WriteByte(regAddr);
        I2C_1_WriteBytes(data_Point, length);
        I2C_1_Stop();
        break;
        #if 0
    case MAG_IIC_MODULE:    //地磁 iic0
        if(swiic_info[IIC0].bOpen == FALSE)  //IIC已经关闭，不能做读写操作
            return Ret_NoInit;
        I2C_0_Start();
        I2C_0_WriteByte(deviceAddr);
        I2C_0_WriteByte(regAddr);
        I2C_0_WriteBytes(data_Point, length);
        I2C_0_Stop();
        break;
        #endif
    case ACC_IIC_MODULE:   //加速计和陀螺仪共用 iic5
    case GYR_IIC_MODULE:
        if(swiic_info[IIC2].bOpen == FALSE)  //IIC已经关闭，不能做读写操作
            return Ret_NoInit;
        I2C_2_Start();
        I2C_2_WriteByte(deviceAddr);
        I2C_2_WriteByte(regAddr);
        I2C_2_WriteBytes(data_Point, length);
        I2C_2_Stop();
        break;
    default:
        return Ret_InvalidParam;
    }
    return Ret_OK;
}

//**********************************************************************
// 函数功能:   I2C读操作
// 输入参数：
// modul: driver module ID,值参考iic_module
// deviceAddr：  从设备地址
// regAddr  ：  命令或偏移地址
// data_Point： 数据指针
// length   ：  数据长度
// 返回参数：   Ret_InvalidParam或Ret_OK
//**********************************************************************
ret_type SMDrv_SWI2C_Read(iic_module modul,uint8 deviceAddr, uint8 regAddr, uint8* data_Point, uint16 length)
{
    switch(modul)
    {
    case HR_IIC_MODULE:     //心率 iic4
        if(swiic_info[IIC4].bOpen == FALSE)  //IIC已经关闭，不能做读写操作
            return Ret_NoInit;
        I2C_4_Start();
        I2C_4_WriteByte(deviceAddr);
        I2C_4_WriteByte(regAddr);
        I2C_4_Start();
        I2C_4_WriteByte(deviceAddr | 0x01);
        I2C_4_ReadBytes(data_Point, length);
        I2C_4_Stop();
        break;
    case AP_IIC_MODULE:    //气压和bat共用 iic1
	case BAT_IIC_MODULE:
		if(swiic_info[IIC1].bOpen == FALSE)  //IIC已经关闭，不能做读写操作
            return Ret_NoInit;
        I2C_1_Start();
        I2C_1_WriteByte(deviceAddr);
        I2C_1_WriteByte(regAddr);
        I2C_1_Start();
        I2C_1_WriteByte(deviceAddr | 0x01);
        I2C_1_ReadBytes(data_Point, length);
        I2C_1_Stop();
		break;
        #if 0
    case MAG_IIC_MODULE:    //地磁 iic0
        if(swiic_info[IIC0].bOpen == FALSE)  //IIC已经关闭，不能做读写操作
            return Ret_NoInit;
        I2C_0_Start();
        I2C_0_WriteByte(deviceAddr);
        I2C_0_WriteByte(regAddr);
        I2C_0_Start();
        I2C_0_WriteByte(deviceAddr | 0x01);
        I2C_0_ReadBytes(data_Point, length);
        I2C_0_Stop();
        break;
        #endif
    case ACC_IIC_MODULE:    //加速计和陀螺仪共用 iic5
    case GYR_IIC_MODULE:
        if(swiic_info[IIC2].bOpen == FALSE)  //IIC已经关闭，不能做读写操作
            return Ret_NoInit;
        I2C_2_Start();
        I2C_2_WriteByte(deviceAddr);
        I2C_2_WriteByte(regAddr);
        I2C_2_Start();
        I2C_2_WriteByte(deviceAddr | 0x01);
        I2C_2_ReadBytes(data_Point, length);
        I2C_2_Stop();
        break;
    default:
        return Ret_InvalidParam;
    }
    return Ret_OK;
}

//**********************************************************************
// 函数功能: 获取IIC资源，如果多个driver模块使用同一个iic时需要加锁保护
// 输入参数：	
//    modul: driver module ID,值参考iic_module
//    u32timeout:等待超时时间
// 返回参数：
//**********************************************************************
ret_type SMDrv_SWI2C_LockMutex(iic_module modul,uint32 u32timeout)
{
    return Ret_OK;
}

//**********************************************************************
// 函数功能: 释放IIC资源
// 输入参数：	
//    modul: driver module ID,值参考iic_module
// 返回参数：
//**********************************************************************
ret_type SMDrv_SWI2C_UnLockMutex(iic_module modul)
{
    return Ret_OK;
}

