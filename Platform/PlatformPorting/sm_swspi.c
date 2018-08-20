/**********************************************************************
**
**模块说明: 软件模拟spi
**软件版本，修改日志(时间，内容),修改人:
**   V2.0   2018.8.9  增加apollo3  ZSL  
**
**********************************************************************/
#define SWSPI_MODULE

#include "io_config.h"
#include "am_mcu_apollo.h"
#include "sm_swspi.h"

#ifdef AM_PART_APOLLO3
//**********************************************************************
// 函数功能: open一组io口作模拟spi
// 输入参数：	
// 返回参数：
//**********************************************************************
ret_type SMDrv_SWSPI_Open(void)
{
    am_hal_gpio_pincfg_t bfGpioCfg = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint32 ret;

    bfGpioCfg.uFuncSel       = 3;
    bfGpioCfg.eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_2MA;
    bfGpioCfg.eGPOutcfg      = AM_HAL_GPIO_PIN_OUTCFG_PUSHPULL;
    if((ret = am_hal_gpio_pinconfig(SW_SPI_CLK, bfGpioCfg)) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - swspi sck open config failed:%d\n",ret));
    am_hal_gpio_output_clear(SW_SPI_CLK);

    //d1
    if((ret = am_hal_gpio_pinconfig(SW_SPI_MOSI, bfGpioCfg)) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - swspi mosi open config failed:%d\n",ret));
    return Ret_OK;
}

//**********************************************************************
// 函数功能: close swspi io
// 输入参数：	
// 返回参数：
//**********************************************************************
ret_type SMDrv_SWSPI_Close(void)
{
    am_hal_gpio_pincfg_t bfGpioCfg = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint32 ret;

    bfGpioCfg.uFuncSel       = 3,
    bfGpioCfg.eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_2MA;
    bfGpioCfg.eGPOutcfg      = AM_HAL_GPIO_PIN_OUTCFG_DISABLE;
    if((ret = am_hal_gpio_pinconfig(SW_SPI_CLK, bfGpioCfg)) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - swspi sck close config failed:%d\n",ret));
    if((ret = am_hal_gpio_pinconfig(SW_SPI_MOSI, bfGpioCfg)) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - swspi mosi close config failed:%d\n",ret));
    return Ret_OK;
}

//**********************************************************************
// 函数功能: 写一个byte数据
// 输入参数：byte:要写的数据	
// 返回参数：
//**********************************************************************
ret_type SMDrv_SWSPI_WriteByte(uint8 byte)
{
    uint8 mask = 0xFF;
  
    while (mask > 0) 
    {
        if ((byte & 0x80) > 0) 
        {
            am_hal_gpio_output_set(SW_SPI_MOSI);
        } 
        else 
        {
            am_hal_gpio_output_clear(SW_SPI_MOSI);
        }
        am_hal_gpio_output_clear(SW_SPI_CLK);
        am_hal_gpio_output_set(SW_SPI_CLK);
        byte <<= 1;
        mask >>= 1;
    }
    am_hal_gpio_output_clear(SW_SPI_CLK);

    return Ret_OK;
}

//**********************************************************************
// 函数功能: 读一个byte数据
// 输入参数：	
// 返回参数：
//**********************************************************************
uint8 SMDrv_SWSPI_ReadByte(void)
{
    uint8_t var=0;
	/*
    int i;
    for (i = 0; i < 8; ++i)
    {
        var<<=1;
        am_hal_gpio_out_bit_set(SW_SPI_CLK);
        if(am_hal_gpio_input_bit_read(SW_SPI_MISO)>0)
            var|=1;
        am_hal_gpio_out_bit_clear(SW_SPI_CLK);
    }
	*/
    return var;
}

#else
//**********************************************************************
// 函数功能: open一组io口作模拟spi
// 输入参数：	
// 返回参数：
//**********************************************************************
ret_type SMDrv_SWSPI_Open(void)
{
    am_hal_gpio_pin_config(SW_SPI_CLK, AM_HAL_PIN_OUTPUT);
    am_hal_gpio_out_bit_clear(SW_SPI_CLK);
    //d1
    am_hal_gpio_pin_config(SW_SPI_MOSI, AM_HAL_PIN_OUTPUT);
    return Ret_OK;
}

//**********************************************************************
// 函数功能: close swspi io
// 输入参数：	
// 返回参数：
//**********************************************************************
ret_type SMDrv_SWSPI_Close(void)
{
    am_hal_gpio_pin_config(SW_SPI_CLK, AM_HAL_PIN_DISABLE);
    am_hal_gpio_pin_config(SW_SPI_MOSI, AM_HAL_PIN_DISABLE);
    return Ret_OK;
}

//**********************************************************************
// 函数功能: 写一个byte数据
// 输入参数：byte:要写的数据	
// 返回参数：
//**********************************************************************
ret_type SMDrv_SWSPI_WriteByte(uint8 byte)
{
    uint8 mask = 0xFF;
  
    while (mask > 0) 
    {
        if ((byte & 0x80) > 0) 
        {
            am_hal_gpio_out_bit_set(SW_SPI_MOSI);
        } 
        else 
        {
            am_hal_gpio_out_bit_clear(SW_SPI_MOSI);
        }
        am_hal_gpio_out_bit_clear(SW_SPI_CLK);
        am_hal_gpio_out_bit_set(SW_SPI_CLK);
        byte <<= 1;
        mask >>= 1;
    }
    am_hal_gpio_out_bit_clear(SW_SPI_CLK);

    return Ret_OK;
}

//**********************************************************************
// 函数功能: 读一个byte数据
// 输入参数：	
// 返回参数：
//**********************************************************************
uint8_t SMDrv_SWSPI_ReadByte(void)
{
    uint8_t var=0;
	/*
    int i;
    for (i = 0; i < 8; ++i)
    {
        var<<=1;
        am_hal_gpio_out_bit_set(SW_SPI_CLK);
        if(am_hal_gpio_input_bit_read(SW_SPI_MISO)>0)
            var|=1;
        am_hal_gpio_out_bit_clear(SW_SPI_CLK);
    }
	*/
    return var;
}
#endif

//**********************************************************************
// 函数功能: 写length bytes数据
// 输入参数：pdata:要写的数据，	length:数据长度
// 返回参数：
//**********************************************************************
ret_type SMDrv_SWSPI_WriteBytes(uint8* pdata,uint16 length)
{
    uint8 cnt;

    for(cnt = 0; cnt < length; cnt++)
    {
        SMDrv_SWSPI_WriteByte(pdata[cnt]);
    }
    return Ret_OK;
}

//**********************************************************************
// 函数功能: 读length bytes数据
// 输入参数：pdata:数据buffer，	length:要读数据长度
// 返回参数：
//**********************************************************************
ret_type SMDrv_SWSPI_ReadBytes(uint8* pdata,uint16 length)
{
    uint8 cnt;
    for(cnt = 0; cnt < length; cnt++)
    {
        pdata[cnt] = SMDrv_SWSPI_ReadByte();
    }
    return Ret_OK;
}

