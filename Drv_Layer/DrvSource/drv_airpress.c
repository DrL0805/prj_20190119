/**********************************************************************
**
**模块说明: 气压驱动接口
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.20  修改流程  ZSL  
**
**********************************************************************/
#include "sm_swi2c.h"
#include "drv_airpress.h"
#include "sm_sys.h"

/*******************macro define*******************/
// devicc address (HPS5806)
#define	HP5806_ADDR          		(0x77 << 1)

// const define                                 
#define	HP5806_CHIP_ID				16

// register macro
#define HP5806_PSR_TMP_READ_REG		0x00
#define HP5806_PSR_READ_REG			0x00
#define HP5806_TMP_READ_REG			0x03
#define HP5806_PRS_CFG_REG			0x06
#define HP5806_TMP_CFG_REG			0x07
#define HP5806_MEAS_CFG_REG			0x08
#define HP5806_CFG_REG				0x09
#define HP5806_FIFO_STS_REG			0x0B
#define HP5806_RESET_REG			0x0C
#define HP5806_PROD_REV_ID_REG		0x0D
#define HP5806_COEF_REG				0x10
#define HP5806_TMP_COEF_SRCE_REG	0x28

//control reg value
#define HP5806_SOFT_RESET			0x09
#define TMP_EXT_ASIC				0x00
#define TMP_EXT_MEMS				0x80
#define HP5806_PRS_SHIFT_EN			0x04
#define HP5806_TMP_SHIFT_EN			0x08

/*******************variable define*******************/
#define POW_2_23_MINUS_1	0x7FFFFF   //implies 2^23-1
#define POW_2_24			0x1000000
#define POW_2_15_MINUS_1	0x7FFF
#define POW_2_16			0x10000
#define POW_2_11_MINUS_1	0x7FF
#define POW_2_12			0x1000
#define POW_2_20			0x100000
#define POW_2_19_MINUS_1	524287

//传感器校准相关
#define	HP5806_COEF_LEN		18
typedef struct
{
	int16_t	C0;
	int16_t	C1;
	int32_t	C00;
	int32_t	C10;
	int16_t	C01;
	int16_t	C11;
	int16_t	C20;
	int16_t	C21;
	int16_t	C30;
	uint32_t tmp_osr_scale_coeff;
	uint32_t prs_osr_scale_coeff;
}HP5806_calib_data;

static HP5806_calib_data HP5806_calib;
static double Temp_scaled = 0;

//Calibration  and  Measurement  Compensation
static const uint32_t CompScaleFactors[8] = {
    524288, 1572864, 3670016, 7864320, 
	253952, 516096, 1040384, 2088960,
};

//**********************************************************************
// 函数功能: 设置传感器寄存器
// 输入参数：	
// u8Reg   : 命令或偏移地址
// data    : 写入的数据
// 返回参数：无
static void AirPress_RegWrite(uint8 u8Reg, uint8 *data, uint8 dLen)
{
	SMDrv_SWI2C_Write(AP_IIC_MODULE, HP5806_ADDR, u8Reg, data, dLen);
}

//**********************************************************************
// 函数功能: 读传感器寄存器
// 输入参数：	
// cmd 	   : 命令或偏移地址
// data    : 数据指针
// dLen    : 读取的数据长度
// 返回参数：无
static void AirPress_RegRead(uint8 u8Reg, uint8 *data, uint8 dLen)
{
    SMDrv_SWI2C_Read(AP_IIC_MODULE, HP5806_ADDR, u8Reg, data, dLen);
}

//**********************************************************************
// 函数功能:	获取传感器状态
// 输入参数：	无
// 返回参数：	传感器状态
//**********************************************************************
uint8 Drv_AirPress_GetStatus(void)
{
    uint8 status;

    AirPress_RegRead(HP5806_MEAS_CFG_REG, &status, 1);
    return status;
}

//**********************************************************************
// 函数功能:    使能模块使用的IO功能
// 输入参数：	无
// 返回参数：	
// 0x00    :    设置成功
// 0x01    :    设置失败
//**********************************************************************
uint8 Drv_AirPress_EnableIO(void)
{
    return SMDrv_SWI2C_Open(AP_IIC_MODULE,IIC_SPEED_HIGH);
}

//**********************************************************************
// 函数功能:	关闭模块使用的IO功能，实现低功耗
// 输入参数：	无
// 返回参数：	
// 0x00    :    设置成功
// 0x01    :    设置失败
//**********************************************************************
uint8 Drv_AirPress_DisableIO(void)
{
    return SMDrv_SWI2C_Close(AP_IIC_MODULE); 
}

//**********************************************************************
// 函数功能:	设置传感器standby　mode
// 输入参数：	无
// 返回参数：	
// 0x00    :    设置成功
// 0x01    :    设置失败
//**********************************************************************
uint8 Drv_AirPress_Standby(void)
{
    // uint8 ui8Regtemp[1] = {0x00};

    // ui8Regtemp[0] = STANDBY;
    // AirPress_RegWrite(HP5806_MEAS_CFG_REG, ui8Regtemp, 1);
	
	return 0;
}


//**********************************************************************
// 函数功能:	传感器软件初始化时获取校准值
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
static void Drv_AirPress_Read_Calib(void)
{
    uint8_t read_buffer[HP5806_COEF_LEN] = {0};

	AirPress_RegRead(HP5806_COEF_REG, read_buffer, HP5806_COEF_LEN);

    HP5806_calib.C0 = (read_buffer[0] << 4) + ((read_buffer[1] >>4) & 0x0F);
    if(HP5806_calib.C0 > POW_2_11_MINUS_1)
        HP5806_calib.C0 = HP5806_calib.C0 - POW_2_12;

    HP5806_calib.C1 = (read_buffer[2] + ((read_buffer[1] & 0x0F)<<8));
    if(HP5806_calib.C1 > POW_2_11_MINUS_1)
        HP5806_calib.C1 = HP5806_calib.C1 - POW_2_12;

    HP5806_calib.C00 = ((read_buffer[4]<<4) + (read_buffer[3]<<12)) + ((read_buffer[5]>>4) & 0x0F);
    if(HP5806_calib.C00 > POW_2_19_MINUS_1)
        HP5806_calib.C00 = HP5806_calib.C00 -POW_2_20;

    HP5806_calib.C10 = ((read_buffer[5] & 0x0F)<<16) + read_buffer[7] + (read_buffer[6]<<8);
    if(HP5806_calib.C10 > POW_2_19_MINUS_1)
        HP5806_calib.C10 = HP5806_calib.C10 - POW_2_20;

    HP5806_calib.C01 = (read_buffer[9] + (read_buffer[8]<<8));
    if(HP5806_calib.C01 > POW_2_15_MINUS_1)
        HP5806_calib.C01 = HP5806_calib.C01 - POW_2_16;

    HP5806_calib.C11 = (read_buffer[11] + (read_buffer[10]<<8));
    if(HP5806_calib.C11 > POW_2_15_MINUS_1)
        HP5806_calib.C11 = HP5806_calib.C11 - POW_2_16;

    HP5806_calib.C20 = (read_buffer[13] + (read_buffer[12]<<8));
    if(HP5806_calib.C20 > POW_2_15_MINUS_1)
        HP5806_calib.C20 = HP5806_calib.C20 - POW_2_16;

    HP5806_calib.C21 = (read_buffer[15] + (read_buffer[14]<<8));
    if(HP5806_calib.C21 > POW_2_15_MINUS_1)
        HP5806_calib.C21 = HP5806_calib.C21 - POW_2_16;

    HP5806_calib.C30 = (read_buffer[17] + (read_buffer[16]<<8));
    if(HP5806_calib.C30 > POW_2_15_MINUS_1)
        HP5806_calib.C30 = HP5806_calib.C30 - POW_2_16;
}

//**********************************************************************
// 函数功能:	传感器软件初始化
// 输入参数：	无
// 返回参数：	
// 0x00    :    设置成功
// 0x01    :    设置失败
//**********************************************************************
uint8 Drv_AirPress_Init(void)
{
    uint8 u8ret = 0x00;
    uint8 ui8Regtemp[1] = {0x00};
	
	u8ret = Drv_AirPress_EnableIO();
	
	if (!(COEF_RDY & Drv_AirPress_GetStatus()))
    {
        return 0x01;
    }
	
	AirPress_RegRead(HP5806_PROD_REV_ID_REG, ui8Regtemp, 1);

	/* read now the calibration coeffs, temperature coef source and store */
    Drv_AirPress_Read_Calib();
	
	//soft reset
	ui8Regtemp[0] = HP5806_SOFT_RESET;                                         
    AirPress_RegWrite(HP5806_RESET_REG, ui8Regtemp, 1);
	
	Drv_AirPress_DisableIO();

    return u8ret;
}

//**********************************************************************
// 函数功能:	硬件关闭
// 输入参数：	无
// 返回参数：	
// 0x00    :    设置成功
// 0x01    :    设置失败
//**********************************************************************
uint8 Drv_AirPress_Close(void)
{
    return SMDrv_SWI2C_Close(AP_IIC_MODULE); 
}

//**********************************************************************
// 函数功能:	传感器自校准
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
void Drv_AirPress_Calibrate(void)
{
	
}

//**********************************************************************
// 函数功能:	启动温度转换
// 输入参数：	
// OSR_Val ： 	使用的下采集率
// 返回参数：	
// 0x00    :  设置成功
// 0x01    :  设置失败
// 0x02    :  参数非法
//**********************************************************************
uint8 Drv_AirPress_TempCvt(uint8 OSR_Val)
{
    uint8 ui8Regtemp[1] = {0x00};

    if(	OSR_Val != OSR_1 && OSR_Val != OSR_2 && 
		OSR_Val != OSR_4 && OSR_Val != OSR_8 &&
		OSR_Val != OSR_16 && OSR_Val != OSR_32 &&
		OSR_Val != OSR_64 && OSR_Val != OSR_128 )
    {
		return Ret_InvalidParam;
	}
	
	//check sensor ready
    if (!(SENSOR_RDY & Drv_AirPress_GetStatus()))
    {
        return Ret_Fail;
    }
	
	//set temperature osr & update temperature Compensation Scale Factors
	ui8Regtemp[0] = TMP_EXT_MEMS | (OSR_Val << 4) | OSR_Val;                                         
    AirPress_RegWrite(HP5806_TMP_CFG_REG, ui8Regtemp, 1);
	HP5806_calib.tmp_osr_scale_coeff = CompScaleFactors[OSR_Val];
	
	/*If oversampling rate is greater than 8 times, then set SHIFT bit in CFG_REG */
	ui8Regtemp[0] = 0;
	AirPress_RegRead(HP5806_CFG_REG, ui8Regtemp, 1);
    if (OSR_Val > OSR_8)
	{
		ui8Regtemp[0] |= HP5806_TMP_SHIFT_EN;
    }
	else
	{
		ui8Regtemp[0] &= ~HP5806_TMP_SHIFT_EN;
	}
	AirPress_RegWrite(HP5806_CFG_REG, ui8Regtemp, 1);
	
	//start Temperature measurement
	ui8Regtemp[0] = TMP_MEAS;
    AirPress_RegWrite(HP5806_MEAS_CFG_REG, ui8Regtemp, 1);
	
    return Ret_OK;
}

//**********************************************************************
// 函数功能:  启动温度和压力转换
// 输入参数：	
// OSR_Val ： 使用的下采集率
// 返回参数：	
// 0x00    :  设置成功
// 0x01    :  设置失败
// 0x02    :  参数非法
//**********************************************************************
uint8 Drv_AirPress_TemPressCvt(uint8 OSR_Val)
{
    uint8 ui8Regtemp[1] = {0x00};

    if(	OSR_Val != OSR_1 && OSR_Val != OSR_2 && 
		OSR_Val != OSR_4 && OSR_Val != OSR_8 &&
		OSR_Val != OSR_16 && OSR_Val != OSR_32 &&
		OSR_Val != OSR_64 && OSR_Val != OSR_128 )
    {
		return Ret_InvalidParam;
	}
	
	//check ready
    if (!(SENSOR_RDY & Drv_AirPress_GetStatus()))
    {
        return Ret_Fail;
    }
	
	//set temperature osr & update temperature Compensation Scale Factors
	ui8Regtemp[0] = TMP_EXT_MEMS | (OSR_Val << 4) | OSR_Val;                                         
    AirPress_RegWrite(HP5806_TMP_CFG_REG, ui8Regtemp, 1);
	HP5806_calib.tmp_osr_scale_coeff = CompScaleFactors[OSR_Val];
	
	//set Pressure osr & update Pressure Compensation Scale Factors
	ui8Regtemp[0] = (OSR_Val << 4) | OSR_Val;                                         
    AirPress_RegWrite(HP5806_PRS_CFG_REG, ui8Regtemp, 1);
	HP5806_calib.prs_osr_scale_coeff = CompScaleFactors[OSR_Val];
	
	/*If oversampling rate is greater than 8 times, then set SHIFT bit in CFG_REG */
	ui8Regtemp[0] = 0;
	AirPress_RegRead(HP5806_CFG_REG, ui8Regtemp, 1);
    if (OSR_Val > OSR_8)
	{
		ui8Regtemp[0] |= HP5806_TMP_SHIFT_EN | HP5806_PRS_SHIFT_EN;
    }
	else
	{
		ui8Regtemp[0] &= ~(HP5806_TMP_SHIFT_EN | HP5806_PRS_SHIFT_EN);
	}
	AirPress_RegWrite(HP5806_CFG_REG, ui8Regtemp, 1);
	
	//start Pressure measurement
	ui8Regtemp[0] = BG_ALL_MEAS;
    AirPress_RegWrite(HP5806_MEAS_CFG_REG, ui8Regtemp, 1);	
	
    return Ret_OK;
}

//**********************************************************************
// 函数功能:	读取压力值（单位Pa），需要注意无符号数值到有符号数值的转换
// 输入参数：	
// Press ： 	压力值指针
// 返回参数：	
// 0x00    :    设置成功
// 0x01    :    设置失败
//**********************************************************************
uint8 Drv_AirPress_ReadPress(int32 *Press)
{
	uint8 ui8Regtemp[1] = {0x00};
	uint8_t read_buffer[3] = {0};
	double	 press_raw;	
	double	 press_scaled;
	double	 press_final;
	
    if (!(RPS_RDY & Drv_AirPress_GetStatus()))
    {
        return Ret_Fail;
    }
	
	//read Pressure data
	AirPress_RegRead(HP5806_PSR_READ_REG, read_buffer, 3);
	
	//calculate Pressure
	press_raw = (read_buffer[0] << 16) + (read_buffer[1] << 8) + (read_buffer[2]);
	if(press_raw > POW_2_23_MINUS_1)
	{
		press_raw = press_raw - POW_2_24;
	}
	press_scaled = (double)press_raw / HP5806_calib.prs_osr_scale_coeff;
	press_final = HP5806_calib.C00
					+ press_scaled * (HP5806_calib.C10 + press_scaled * (HP5806_calib.C20 + press_scaled * HP5806_calib.C30))
					+ Temp_scaled * HP5806_calib.C01
					+ Temp_scaled * press_scaled * (HP5806_calib.C11 + press_scaled * HP5806_calib.C21);
//	press_final = press_final * 0.01f;  //to convert it into mBar
	
    *Press = (int32)press_final; //Pa
	
	//sensor standby
	ui8Regtemp[0] = STANDBY;
    AirPress_RegWrite(HP5806_MEAS_CFG_REG, ui8Regtemp, 1);

    return Ret_OK;
}

//**********************************************************************
// 函数功能:  读取温度值（单位C*100），需要注意无符号数值到有符号数值的转换
// 输入参数：	
// Temp    ： 温度值指针
// 返回参数：	
// 0x00    :  设置成功
// 0x01    :  设置失败
//**********************************************************************
uint8 Drv_AirPress_ReadTemp(int16 *Temp)
{
	uint8 ui8Regtemp[1] = {0x00};
	uint8_t read_buffer[3] = {0};
	double	 temp_raw;	
	double	 temp_scaled;
	double	 temp_final;
	
    if (!(TMP_RDY & Drv_AirPress_GetStatus()))
    {
        return Ret_Fail;
    }
	
	//read Temperature data
	AirPress_RegRead(HP5806_TMP_READ_REG, read_buffer, 3);
	
	//calculate Temperature
	temp_raw = (read_buffer[0] << 16) + (read_buffer[1] << 8) + (read_buffer[2]);
	if(temp_raw > POW_2_23_MINUS_1)
	{
		temp_raw = temp_raw - POW_2_24;
	}
	temp_scaled = (double)temp_raw / (double)(HP5806_calib.tmp_osr_scale_coeff);
	temp_final =  (HP5806_calib.C0 / 2.0f) + HP5806_calib.C1 * temp_scaled;
			
    *Temp = (int16)(temp_final * 100);	//to convert it into C*100
	
	//Temp_scaled for calculate Pressure
	Temp_scaled = temp_scaled;
	
	//sensor standby
	ui8Regtemp[0] = STANDBY;
    AirPress_RegWrite(HP5806_MEAS_CFG_REG, ui8Regtemp, 1);
	
    return Ret_OK;
}

//**********************************************************************
// 函数功能:      设置海拔高度计算值的补偿量
// 输入参数：	
// LocalAvgPress：当前的标准大气压力值（单位Pa）
// 返回参数：	
// 0x00    :      设置成功
// 0xff    :      设置失败
//**********************************************************************
uint8 Drv_AirPress_SetAltitudeCmps(int32 LocalAvgPress)
{
    
    return Ret_OK;
}

//**********************************************************************
// 函数功能: 传感器自检
// 输入参数：无
// 返回参数：	
// 0x00    : 自检通过
// 0x01    : 自检失败
//**********************************************************************
uint8 Drv_AirPress_SelfTest(void)
{
    uint8 ret = Ret_OK;
	uint8_t ui8Regtemp[1] = {0x00};

    //open iic
    if(SMDrv_SWI2C_Open(AP_IIC_MODULE,IIC_SPEED_HIGH) != Ret_OK)
        return Ret_Fail;

    AirPress_RegRead(HP5806_PROD_REV_ID_REG, ui8Regtemp, 1);

    SMDrv_SWI2C_Close(AP_IIC_MODULE); 
    return ret;
}

