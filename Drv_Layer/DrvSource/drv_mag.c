/**********************************************************************
**
**模块说明: 地磁驱动接口
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.19  修改流程  ZSL  
**
**********************************************************************/
#include "sm_swi2c.h"
#include "drv_mag.h"
#include "bsp_common.h"

// addr define
#define 	QMC7983_ADDRESS					(0x2C << 1)  //ADO=L
                                                
// const define                                 
#define		QMC7983_CHIP_ID					0x32

// register macro 
#define		QMC7983_CHIP_ID_REG				0x0D	
#define 	QMC7983_OUT_X_L_REG				0x00
#define 	QMC7983_OUT_X_M_REG				0x01
#define 	QMC7983_OUT_Y_L_REG				0x02
#define 	QMC7983_OUT_Y_M_REG				0x03
#define 	QMC7983_OUT_Z_L_REG				0x04
#define 	QMC7983_OUT_Z_M_REG				0x05
#define 	QMC7983_STA_REG					0x06	/*Status registers */
#define 	QMC7983_TEMP_L_REG 				0x07	/* Temperature registers */
#define 	QMC7983_TEMP_H_REG 				0x08
#define 	QMC7983_CTL_ONE_REG				0x09	/* Contrl register one */
#define 	QMC7983_CTL_TWO_REG				0x0A	/* Contrl register two */
#define 	QMC7983_SET_RESET_PERIOD_REG	0x0B	/* SET/RESET Period Register */

//**********************************************************************
// 函数功能:    寄存器写
// 输入参数：    
// 返回参数：    无
static void Mag_RegWrite(uint8 dev_addr, uint8 reg_addr, uint8 *regval, uint16 length)
{
    SMDrv_SWI2C_Write(MAG_IIC_MODULE,dev_addr, reg_addr, regval, length);
}

//**********************************************************************
// 函数功能:    寄存器读
// 输入参数：
// 返回参数：
static void Mag_RegRead(uint8 dev_addr, uint8 reg_addr, uint8 *regval, uint16 length)
{
    SMDrv_SWI2C_Read(MAG_IIC_MODULE,dev_addr, reg_addr, regval, length);
}

//**********************************************************************
// 函数功能:    地磁传感器初始化
// 输入参数：    无
// 返回参数：    
// 0x00    :    初始化成功
// 0xff    :    初始化失败
//**********************************************************************
uint8 Drv_Mag_Open(void)
{
    uint8 u8ret = 0x00;
    uint8 ui8Regtemp[1] = {0x00};
	
	u8ret = SMDrv_SWI2C_Open(MAG_IIC_MODULE,IIC_SPEED_HIGH);
	
	if(multisensor.magstate == uinit)
    {
		ui8Regtemp[0] = 0x00;
		Mag_RegRead(QMC7983_ADDRESS, QMC7983_CTL_ONE_REG, ui8Regtemp, 1);
		ui8Regtemp[0] &= 0x3C;
		ui8Regtemp[0] |= (MAG_CONTINUOUS_MODE << 0) | (MAG_OVER_SAMPLE_RATE_64 << 6);                                         
		Mag_RegWrite(QMC7983_ADDRESS, QMC7983_CTL_ONE_REG, ui8Regtemp, 1);
		
		ui8Regtemp[0] = 0xFF;
		Mag_RegWrite(QMC7983_ADDRESS, QMC7983_SET_RESET_PERIOD_REG, ui8Regtemp, 1);
	}
	multisensor.magstate = poweron;

    return u8ret;
}

//**********************************************************************
// 函数功能:    硬件关闭
// 输入参数：    无
// 返回参数：    
// 0x00    :    设置成功
// 0xff    :    设置失败
//**********************************************************************
uint8 Drv_Mag_Close(void)
{
    // close i2c interface.
    SMDrv_SWI2C_Close(MAG_IIC_MODULE);
    multisensor.magstate = powerdown;
    return 0;
}

//**********************************************************************
// 函数功能:    地磁传感器采样率及量程设置
// 输入参数：    
// sampleRate    采样率
//               Valid Values for uint16 sampleRate are:
//               MAG_DATA_RATE_10HZ
//               MAG_DATA_RATE_50HZ
//               MAG_DATA_RATE_100HZ
//               MAG_DATA_RATE_200HZ
// scaleRange    测量量程
//               fullscale selection 
//               Valid Values for uint8 scaleRange are:
//               MAG_SCALE_RANGE_2GS
//               MAG_SCALE_RANGE_8GS
//               MAG_SCALE_RANGE_12GS
//               MAG_SCALE_RANGE_20GS
// 返回参数：    
// 0x00    :    初始化成功
// 0xff    :    初始化失败
//**********************************************************************
uint8 Drv_Mag_Set(uint16 sampleRate, uint8 scaleRange)
{
    uint8 ui8Regtemp[1] = {0x00};
    
	if (Drv_Mag_WakeUp())
    {
        return 0xff;
    }
	
    //Set Sample Rate
    if(	sampleRate != MAG_DATA_RATE_10HZ && 
		sampleRate != MAG_DATA_RATE_50HZ &&
		sampleRate != MAG_DATA_RATE_100HZ &&
		sampleRate != MAG_DATA_RATE_200HZ )
    {return 0xFF;}
	
    ui8Regtemp[0] = 0x00;
    Mag_RegRead(QMC7983_ADDRESS, QMC7983_CTL_ONE_REG, ui8Regtemp, 1);
	ui8Regtemp[0] &= 0xF3;
	ui8Regtemp[0] |= (sampleRate << 2);                                         
    Mag_RegWrite(QMC7983_ADDRESS, QMC7983_CTL_ONE_REG, ui8Regtemp, 1);
	
    //Set Scale Range
    if(	scaleRange != MAG_SCALE_RANGE_2GS && 
		scaleRange != MAG_SCALE_RANGE_8GS &&
		scaleRange != MAG_SCALE_RANGE_12GS &&
		scaleRange != MAG_SCALE_RANGE_20GS )
    {return 0xFF;}
	
    ui8Regtemp[0] = 0x00;
    Mag_RegRead(QMC7983_ADDRESS, QMC7983_CTL_ONE_REG, ui8Regtemp, 1);
	ui8Regtemp[0] &= 0xCF;
	ui8Regtemp[0] |= (scaleRange << 4);                                         
    Mag_RegWrite(QMC7983_ADDRESS, QMC7983_CTL_ONE_REG, ui8Regtemp, 1);
	
    return Ret_OK;
}

//**********************************************************************
// 函数功能:  读取地磁传感器3轴数据，，数据为二进补码形式
// 输入参数：    
// axisData ：三轴数据指针
// 返回参数：    
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
uint8 Drv_Mag_Read(int16 xyzData[3])
{
    uint8 axisdata[6];
    Mag_RegRead(QMC7983_ADDRESS, QMC7983_OUT_X_L_REG, axisdata, 6);
    xyzData[0] = ((int16)axisdata[1]<<8) + axisdata[0];
    xyzData[1] = ((int16)axisdata[3]<<8) + axisdata[2];
    xyzData[2] = ((int16)axisdata[5]<<8) + axisdata[4];
    return Ret_OK;
}

//**********************************************************************
// 函数功能:    地磁传感器唤醒
// 输入参数：   无
// 返回参数：    
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
uint8 Drv_Mag_WakeUp(void)
{
    uint8 ui8Regtemp[1] = {0x00};

    if (multisensor.magstate != poweron)
    {
        // Setup i2c interface.    
        SMDrv_SWI2C_Open(MAG_IIC_MODULE,IIC_SPEED_HIGH);

        //Set CONTROL Into Continuous Mode
		ui8Regtemp[0] = 0x00;
		Mag_RegRead(QMC7983_ADDRESS, QMC7983_CTL_ONE_REG, ui8Regtemp, 1);
		ui8Regtemp[0] &= 0xFC;
		ui8Regtemp[0] |= (MAG_CONTINUOUS_MODE << 0);                                         
		Mag_RegWrite(QMC7983_ADDRESS, QMC7983_CTL_ONE_REG, ui8Regtemp, 1);
    }
    multisensor.magstate = poweron;
    
    return 0x00;
}

//**********************************************************************
// 函数功能:    设置地磁传感器进入睡眠状态
// 输入参数：   无
// 返回参数：    
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
uint8 Drv_Mag_GotoSleep(void)
{
	multisensor.magstate = sleep;
	
    uint8 ui8Regtemp[1];
	
    //Set CONTROL Into Standby Mode
    ui8Regtemp[0] = 0x00;
    Mag_RegRead(QMC7983_ADDRESS, QMC7983_CTL_ONE_REG, ui8Regtemp, 1);
	ui8Regtemp[0] &= 0xFC;
	ui8Regtemp[0] |= (MAG_STANDBY_MODE << 0);                                         
    Mag_RegWrite(QMC7983_ADDRESS, QMC7983_CTL_ONE_REG, ui8Regtemp, 1);

    Drv_Mag_Close();
    return Ret_OK;
}

//**********************************************************************
// 函数功能:    地磁传感器自检
// 输入参数：    无
// 返回参数：    
// 0x00    :    自检成功
// 0x01    :    自检失败
//**********************************************************************
uint8 Drv_Mag_SelfTest(void)
{
    uint8 ui8Regtemp[1] = {0x00};
 
    Drv_Mag_Open();
	
	//read CHIP ID
	Mag_RegRead(QMC7983_ADDRESS, QMC7983_CHIP_ID_REG, ui8Regtemp, 1);
	if(ui8Regtemp[0] != QMC7983_CHIP_ID)
	{
		return 0x01;
	}
	
    //Set CONTROL Into Self test Mode
    ui8Regtemp[0] = 0x00;
    Mag_RegRead(QMC7983_ADDRESS, QMC7983_CTL_ONE_REG, ui8Regtemp, 1);
	ui8Regtemp[0] &= 0xFC;
	ui8Regtemp[0] |= (MAG_SELFTEST_MODE << 0);                                         
    Mag_RegWrite(QMC7983_ADDRESS, QMC7983_CTL_ONE_REG, ui8Regtemp, 1);

	Drv_Mag_Close();
    return Ret_OK;
}

