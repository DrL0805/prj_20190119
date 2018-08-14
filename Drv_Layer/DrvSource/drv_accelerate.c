/**********************************************************************
**
**模块说明: 加速计驱动接口
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.20  修改流程  ZSL  
**
**********************************************************************/
#include "sm_swi2c.h"
#include "sm_sys.h"
#include "drv_accelerate.h"
#include "bsp_common.h"

/*******************macro define*******************/
#define BMI160_I2C_ADDR1                   (0x68 << 1)
/**< I2C Address needs to be changed */
#define BMI160_I2C_ADDR2                    (0x69<< 1)

// #define     ACCEL_BUFFER_ENABLE                 0x01


/****************************************************/
/**\name    REGISTER DEFINITIONS       */
/***************************************************/
/*******************/
/**\name CHIP ID */
/*******************/
#define BMI160_USER_CHIP_ID_ADDR                (0x00)
/*******************/
/**\name ERROR STATUS */
/*******************/
#define BMI160_USER_ERROR_ADDR                  (0X02)
/*******************/
/**\name POWER MODE STATUS */
/*******************/
#define BMI160_USER_PMU_STAT_ADDR               (0X03)
/*******************/
/**\name MAG DATA REGISTERS */
/*******************/
#define BMI160_USER_DATA_0_ADDR                 (0X04)
#define BMI160_USER_DATA_1_ADDR                 (0X05)
#define BMI160_USER_DATA_2_ADDR                 (0X06)
#define BMI160_USER_DATA_3_ADDR                 (0X07)
#define BMI160_USER_DATA_4_ADDR                 (0X08)
#define BMI160_USER_DATA_5_ADDR                 (0X09)
#define BMI160_USER_DATA_6_ADDR                 (0X0A)
#define BMI160_USER_DATA_7_ADDR                 (0X0B)
/*******************/
/**\name GYRO DATA REGISTERS */
/*******************/
#define BMI160_USER_DATA_8_ADDR                 (0X0C)
#define BMI160_USER_DATA_9_ADDR                 (0X0D)
#define BMI160_USER_DATA_10_ADDR                (0X0E)
#define BMI160_USER_DATA_11_ADDR                (0X0F)
#define BMI160_USER_DATA_12_ADDR                (0X10)
#define BMI160_USER_DATA_13_ADDR                (0X11)
#define BMI160_USER_DATA_14_ADDR                (0X12)
#define BMI160_USER_DATA_15_ADDR                (0X13)
/*******************/
/**\name ACCEL DATA REGISTERS */
/*******************/
#define BMI160_USER_DATA_16_ADDR                (0X14)
#define BMI160_USER_DATA_17_ADDR                (0X15)
#define BMI160_USER_DATA_18_ADDR                (0X16)
#define BMI160_USER_DATA_19_ADDR                (0X17)
/*******************/
/**\name SENSOR TIME REGISTERS */
/*******************/
#define BMI160_USER_SENSORTIME_0_ADDR           (0X18)
#define BMI160_USER_SENSORTIME_1_ADDR           (0X19)
#define BMI160_USER_SENSORTIME_2_ADDR           (0X1A)
/*******************/
/**\name STATUS REGISTER FOR SENSOR STATUS FLAG */
/*******************/
#define BMI160_USER_STAT_ADDR                   (0X1B)
/*******************/
/**\name INTERRUPY STATUS REGISTERS */
/*******************/
#define BMI160_USER_INTR_STAT_0_ADDR            (0X1C)
#define BMI160_USER_INTR_STAT_1_ADDR            (0X1D)
#define BMI160_USER_INTR_STAT_2_ADDR            (0X1E)
#define BMI160_USER_INTR_STAT_3_ADDR            (0X1F)
/*******************/
/**\name TEMPERATURE REGISTERS */
/*******************/
#define BMI160_USER_TEMPERATURE_0_ADDR          (0X20)
#define BMI160_USER_TEMPERATURE_1_ADDR          (0X21)
/*******************/
/**\name FIFO REGISTERS */
/*******************/
#define BMI160_USER_FIFO_LENGTH_0_ADDR          (0X22)
#define BMI160_USER_FIFO_LENGTH_1_ADDR          (0X23)
#define BMI160_USER_FIFO_DATA_ADDR              (0X24)
/***************************************************/
/**\name ACCEL CONFIG REGISTERS  FOR ODR, BANDWIDTH AND UNDERSAMPLING*/
/******************************************************/
#define BMI160_USER_ACCEL_CONFIG_ADDR           (0X40)
/*******************/
/**\name ACCEL RANGE */
/*******************/
#define BMI160_USER_ACCEL_RANGE_ADDR            (0X41)
/***************************************************/
/**\name GYRO CONFIG REGISTERS  FOR ODR AND BANDWIDTH */
/******************************************************/
#define BMI160_USER_GYRO_CONFIG_ADDR            (0X42)
/*******************/
/**\name GYRO RANGE */
/*******************/
#define BMI160_USER_GYRO_RANGE_ADDR             (0X43)
/***************************************************/
/**\name MAG CONFIG REGISTERS  FOR ODR*/
/******************************************************/
#define BMI160_USER_MAG_CONFIG_ADDR             (0X44)
/***************************************************/
/**\name REGISTER FOR GYRO AND ACCEL DOWNSAMPLING RATES FOR FIFO*/
/******************************************************/
#define BMI160_USER_FIFO_DOWN_ADDR              (0X45)
/***************************************************/
/**\name FIFO CONFIG REGISTERS*/
/******************************************************/
#define BMI160_USER_FIFO_CONFIG_0_ADDR          (0X46)
#define BMI160_USER_FIFO_CONFIG_1_ADDR          (0X47)
/***************************************************/
/**\name MAG INTERFACE REGISTERS*/
/******************************************************/
#define BMI160_USER_MAG_IF_0_ADDR               (0X4B)
#define BMI160_USER_MAG_IF_1_ADDR               (0X4C)
#define BMI160_USER_MAG_IF_2_ADDR               (0X4D)
#define BMI160_USER_MAG_IF_3_ADDR               (0X4E)
#define BMI160_USER_MAG_IF_4_ADDR               (0X4F)
/***************************************************/
/**\name INTERRUPT ENABLE REGISTERS*/
/******************************************************/
#define BMI160_USER_INTR_ENABLE_0_ADDR          (0X50)
#define BMI160_USER_INTR_ENABLE_1_ADDR          (0X51)
#define BMI160_USER_INTR_ENABLE_2_ADDR          (0X52)
#define BMI160_USER_INTR_OUT_CTRL_ADDR          (0X53)
/***************************************************/
/**\name LATCH DURATION REGISTERS*/
/******************************************************/
#define BMI160_USER_INTR_LATCH_ADDR             (0X54)
/***************************************************/
/**\name MAP INTERRUPT 1 and 2 REGISTERS*/
/******************************************************/
#define BMI160_USER_INTR_MAP_0_ADDR             (0X55)
#define BMI160_USER_INTR_MAP_1_ADDR             (0X56)
#define BMI160_USER_INTR_MAP_2_ADDR             (0X57)
/***************************************************/
/**\name DATA SOURCE REGISTERS*/
/******************************************************/
#define BMI160_USER_INTR_DATA_0_ADDR            (0X58)
#define BMI160_USER_INTR_DATA_1_ADDR            (0X59)
/***************************************************/
/**\name
INTERRUPT THRESHOLD, HYSTERESIS, DURATION, MODE CONFIGURATION REGISTERS*/
/******************************************************/
#define BMI160_USER_INTR_LOWHIGH_0_ADDR         (0X5A)
#define BMI160_USER_INTR_LOWHIGH_1_ADDR         (0X5B)
#define BMI160_USER_INTR_LOWHIGH_2_ADDR         (0X5C)
#define BMI160_USER_INTR_LOWHIGH_3_ADDR         (0X5D)
#define BMI160_USER_INTR_LOWHIGH_4_ADDR         (0X5E)
#define BMI160_USER_INTR_MOTION_0_ADDR          (0X5F)
#define BMI160_USER_INTR_MOTION_1_ADDR          (0X60)
#define BMI160_USER_INTR_MOTION_2_ADDR          (0X61)
#define BMI160_USER_INTR_MOTION_3_ADDR          (0X62)
#define BMI160_USER_INTR_TAP_0_ADDR             (0X63)
#define BMI160_USER_INTR_TAP_1_ADDR             (0X64)
#define BMI160_USER_INTR_ORIENT_0_ADDR          (0X65)
#define BMI160_USER_INTR_ORIENT_1_ADDR          (0X66)
#define BMI160_USER_INTR_FLAT_0_ADDR            (0X67)
#define BMI160_USER_INTR_FLAT_1_ADDR            (0X68)
/***************************************************/
/**\name FAST OFFSET CONFIGURATION REGISTER*/
/******************************************************/
#define BMI160_USER_FOC_CONFIG_ADDR             (0X69)
/***************************************************/
/**\name MISCELLANEOUS CONFIGURATION REGISTER*/
/******************************************************/
#define BMI160_USER_CONFIG_ADDR                 (0X6A)
/***************************************************/
/**\name SERIAL INTERFACE SETTINGS REGISTER*/
/******************************************************/
#define BMI160_USER_IF_CONFIG_ADDR              (0X6B)
/***************************************************/
/**\name GYRO POWER MODE TRIGGER REGISTER */
/******************************************************/
#define BMI160_USER_PMU_TRIGGER_ADDR            (0X6C)
/***************************************************/
/**\name SELF_TEST REGISTER*/
/******************************************************/
#define BMI160_USER_SELF_TEST_ADDR              (0X6D)
/***************************************************/
/**\name SPI,I2C SELECTION REGISTER*/
/******************************************************/
#define BMI160_USER_NV_CONFIG_ADDR              (0x70)
/***************************************************/
/**\name ACCEL AND GYRO OFFSET REGISTERS*/
/******************************************************/
#define BMI160_USER_OFFSET_0_ADDR               (0X71)
#define BMI160_USER_OFFSET_1_ADDR               (0X72)
#define BMI160_USER_OFFSET_2_ADDR               (0X73)
#define BMI160_USER_OFFSET_3_ADDR               (0X74)
#define BMI160_USER_OFFSET_4_ADDR               (0X75)
#define BMI160_USER_OFFSET_5_ADDR               (0X76)
#define BMI160_USER_OFFSET_6_ADDR               (0X77)
/***************************************************/
/**\name STEP COUNTER INTERRUPT REGISTERS*/
/******************************************************/
#define BMI160_USER_STEP_COUNT_0_ADDR           (0X78)
#define BMI160_USER_STEP_COUNT_1_ADDR           (0X79)
/***************************************************/
/**\name STEP COUNTER CONFIGURATION REGISTERS*/
/******************************************************/
#define BMI160_USER_STEP_CONFIG_0_ADDR          (0X7A)
#define BMI160_USER_STEP_CONFIG_1_ADDR          (0X7B)
/***************************************************/
/**\name COMMAND REGISTER*/
/******************************************************/
#define BMI160_CMD_COMMANDS_ADDR                (0X7E)
/***************************************************/
/**\name PAGE REGISTERS*/
/******************************************************/
#define BMI160_CMD_EXT_MODE_ADDR                (0X7F)
#define BMI160_COM_C_TRIM_FIVE_ADDR             (0X05)

#define ACCEL_MODE_NORMAL   (0x11)
#define ACCEL_LOWPOWER      (0X12)
#define ACCEL_SUSPEND       (0X10)

#define GYRO_MODE_SUSPEND       (0x14)
#define GYRO_MODE_NORMAL        (0x15)
#define GYRO_MODE_FASTSTARTUP   (0x17)

/**************************************************/
/**\name    FIFO CONFIGURATIONS    */
/*************************************************/
#define FIFO_HEADER_ENABLE          (0x01)
#define FIFO_MAG_ENABLE             (0x01)
#define FIFO_ACCEL_ENABLE           (0x01)
#define FIFO_GYRO_ENABLE            (0x01)
#define FIFO_TIME_ENABLE            (0x01)
#define FIFO_STOPONFULL_ENABLE      (0x01)
#define FIFO_WM_INTERRUPT_ENABLE    (0x01)
#define BMI160_FIFO_INDEX_LENGTH    (1)
#define BMI160_FIFO_TAG_INTR_MASK   (0xFC)

/**************************************************/
/**\name	ACCEL BANDWIDTH PARAMETER         */
/*************************************************/
#define BMI160_ACCEL_UNDERSAMPLING		(0x01)

#define BMI160_ACCEL_OSR4_AVG1			(0x00)
#define BMI160_ACCEL_OSR2_AVG2			(0x01)
#define BMI160_ACCEL_NORMAL_AVG4		(0x02)
#define BMI160_ACCEL_CIC_AVG8			(0x03)
#define BMI160_ACCEL_RES_AVG16			(0x04)
#define BMI160_ACCEL_RES_AVG32			(0x05)
#define BMI160_ACCEL_RES_AVG64			(0x06)
#define BMI160_ACCEL_RES_AVG128			(0x07)

//**********************************************************************
// 函数功能:    寄存器写
// 输入参数：    
// 返回参数：    无
static void Accel_RegisterWrite(uint8 dev_address, uint8 reg_address, uint8 *regval, uint16 length)
{
     SMDrv_SWI2C_Write(ACC_IIC_MODULE,dev_address, reg_address, regval, length);
}

//**********************************************************************
// 函数功能:    寄存器读
// 输入参数：    无
// 返回参数：    无
static void Accel_RegisterRead(uint8 dev_address, uint8 reg_address, uint8 *regval, uint16 length)
{
    SMDrv_SWI2C_Read(ACC_IIC_MODULE,dev_address, reg_address, regval, length);
}

//**********************************************************************
// 函数功能:  重力传感器初始化
// 输入参数： 无
// 返回参数：    
// 0x00    :  初始化成功
// 0xff    :  初始化失败
//**********************************************************************
uint8 Drv_Accel_Open(void)
{
	uint8 ui8Regtemp[1] = {0x00};
    uint16 ui16ret;

    SMDrv_SWI2C_Open(ACC_IIC_MODULE,IIC_SPEED_HIGHEST);
   
    if(multisensor.gyrostate == uinit && multisensor.accelstate == uinit)
    {
        //FIFO使能
        #ifdef ACCEL_BUFFER_ENABLE
        Drv_Accel_SetBuffer(1);
        #endif

        /* Set the accel bandwidth as OSR4 */
        ui8Regtemp[0] = BMI160_ACCEL_OSR4_AVG1 << 4; //
		    Accel_RegisterWrite(BMI160_I2C_ADDR1, BMI160_USER_ACCEL_CONFIG_ADDR, ui8Regtemp, 1);		     
    }
	 multisensor.accelstate = poweron;
	
    return ui16ret;
}

//**********************************************************************
// 函数功能:    硬件关闭
// 输入参数：    无
// 返回参数：    
// 0x00    :    设置成功
// 0xff    :    设置失败
//**********************************************************************
uint8 Drv_Accel_Close(void)
{
    // close i2c interface.
    SMDrv_SWI2C_Close(ACC_IIC_MODULE);
    multisensor.accelstate = powerdown;
    return 0;
}

//**********************************************************************
// 函数功能:    重力传感器的采样率和量程设置
// 输入参数：    
// sampleRate    采样率，默认ACCEL_SAMPLERATE_31_25HZ
//               Valid Values for uint16_t sampleRate are:
//               ACCEL_SAMPLERATE_0_5HZ   ACCEL_SAMPLERATE_1HZ
//               ACCEL_SAMPLERATE_2HZ     ACCEL_SAMPLERATE_31_25HZ
//               ACCEL_SAMPLERATE_62_5HZ  ACCEL_SAMPLERATE_250HZ
//               ACCEL_SAMPLERATE_500HZ      
// scaleRange    测量量程设置，默认ACCEL_SCALERANGE_2G
//               Accel fullscale selection : ?à2g (00), ?à4g (01), ?à8g (10), ?à16g (11)
//               Valid Values for uint8 scaleRange are:
//               ACCEL_SCALERANGE_2G   ACCEL_SCALERANGE_4G
//               ACCEL_SCALERANGE_8G   ACCEL_SCALERANGE_16G
// 返回参数：    
// 0x00    :    初始化成功
// 0xff    :    初始化失败
//**********************************************************************
uint8 Drv_Accel_Set(uint16 sampleRate, uint8 scaleRange)
{
    uint8 ui8Regtemp[1] = {0x00};
    
    if (Drv_Accel_WakeUp())
    {
        return 0xff;
    }

    //Disable accel xyz's selftest, set SacleRange.
	ui8Regtemp[0] = 0x00;
    if(scaleRange != ACCEL_SCALERANGE_2G &&
       scaleRange != ACCEL_SCALERANGE_4G &&
       scaleRange != ACCEL_SCALERANGE_8G &&
       scaleRange != ACCEL_SCALERANGE_16G)
    {return 0xFF;}
	//设置量程
    ui8Regtemp[0] |= scaleRange; 
    Accel_RegisterWrite(BMI160_I2C_ADDR1, BMI160_USER_ACCEL_RANGE_ADDR, ui8Regtemp, 1);//0x41;
    
    //Set Sample Rate
    if(sampleRate != ACCEL_SAMPLERATE_0_78HZ   &&
       sampleRate != ACCEL_SAMPLERATE_1_56HZ   &&
       sampleRate != ACCEL_SAMPLERATE_3_12HZ   &&
       sampleRate != ACCEL_SAMPLERATE_6_25HZ   &&
       sampleRate != ACCEL_SAMPLERATE_12_5HZ  &&
       sampleRate != ACCEL_SAMPLERATE_25HZ  &&
       sampleRate != ACCEL_SAMPLERATE_50HZ  &&
       sampleRate != ACCEL_SAMPLERATE_100HZ &&
       sampleRate != ACCEL_SAMPLERATE_200HZ &&
       sampleRate != ACCEL_SAMPLERATE_400HZ &&
       sampleRate != ACCEL_SAMPLERATE_800HZ &&
       sampleRate != ACCEL_SAMPLERATE_1600HZ)
    {return 0xFF;}
	
	//设置采样率
    ui8Regtemp[0] = 0x00;
    Accel_RegisterRead(BMI160_I2C_ADDR1, BMI160_USER_ACCEL_CONFIG_ADDR, ui8Regtemp, 1);
    ui8Regtemp[0] = (ui8Regtemp[0] & 0xF0) | sampleRate;                                          
    Accel_RegisterWrite(BMI160_I2C_ADDR1, BMI160_USER_ACCEL_CONFIG_ADDR, ui8Regtemp, 1);
    
    //陀螺仪传感器未使用，则设置重力计LPB模式
    if (multisensor.gyrostate == uinit || multisensor.gyrostate != poweron)
    {
        /*Set the accel mode as Normal write in the register 0x7E*/
        ui8Regtemp[0] = ACCEL_LOWPOWER; 
        Accel_RegisterWrite(BMI160_I2C_ADDR1, BMI160_CMD_COMMANDS_ADDR, ui8Regtemp, 1);
		
		/*Set the accel undersampling in the register 0x40*/
		ui8Regtemp[0] = 0x00;
		Accel_RegisterRead(BMI160_I2C_ADDR1, BMI160_USER_ACCEL_CONFIG_ADDR, ui8Regtemp, 1);
		ui8Regtemp[0] |= BMI160_ACCEL_UNDERSAMPLING << 7;
		Accel_RegisterWrite(BMI160_I2C_ADDR1, BMI160_USER_ACCEL_CONFIG_ADDR, ui8Regtemp, 1);

        /*Set the gyro mode as SUSPEND write in the register 0x7E*/
        ui8Regtemp[0] = GYRO_MODE_SUSPEND; 
        Accel_RegisterWrite(BMI160_I2C_ADDR1, BMI160_CMD_COMMANDS_ADDR, ui8Regtemp, 1);
    }
    return 0x00;
}

//**********************************************************************
// 函数功能:    重力传感器FIFO功能开关设置
// 输入参数：    
// setState ：   FIFO开关，0x00关闭，0x01使能
// 返回参数：    
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
uint8  Drv_Accel_SetBuffer(uint16 setState)
{
    uint8 ui8Regtemp[1] = {0x00};
    uint8 ui8Ret;
    
    if(setState == 0x01)//ê1?üFIFO
    {
        Accel_RegisterRead(BMI160_I2C_ADDR1,  BMI160_USER_FIFO_CONFIG_1_ADDR, ui8Regtemp, 1);
        ui8Regtemp[0] |= (FIFO_HEADER_ENABLE << 4);
        Accel_RegisterWrite(BMI160_I2C_ADDR1, BMI160_USER_FIFO_CONFIG_1_ADDR, ui8Regtemp, 1);	 

        Accel_RegisterRead(BMI160_I2C_ADDR1,  BMI160_USER_FIFO_CONFIG_1_ADDR, ui8Regtemp, 1);
        ui8Regtemp[0] |= (FIFO_ACCEL_ENABLE << 6);
        Accel_RegisterWrite(BMI160_I2C_ADDR1, BMI160_USER_FIFO_CONFIG_1_ADDR, ui8Regtemp, 1);
    }
    else if(setState == 0x00)//
    {       
        Accel_RegisterRead(BMI160_I2C_ADDR1,  BMI160_USER_FIFO_CONFIG_1_ADDR, ui8Regtemp, 1);
        ui8Regtemp[0] &= ~(FIFO_ACCEL_ENABLE << 6);
        Accel_RegisterWrite(BMI160_I2C_ADDR1, BMI160_USER_FIFO_CONFIG_1_ADDR, ui8Regtemp, 1);	
    }
    return ui8Ret;
}
                            
//**********************************************************************
// 函数功能:    读取重力传感器3轴数据，数据为二进补码形式
// 输入参数：    
// axisData ：   三轴数据指针
// 返回参数：    
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
uint8 Drv_Accel_Read(int16 *axisData)
{
    uint8 axisdata[6];
    Accel_RegisterRead(BMI160_I2C_ADDR1, BMI160_USER_DATA_14_ADDR, axisdata, 6);
    axisData[0] = ((uint16)axisdata[1]<<8) + axisdata[0];
    axisData[1] = ((uint16)axisdata[3]<<8) + axisdata[2];
    axisData[2] = ((uint16)axisdata[5]<<8) + axisdata[4];
    return 0x00;
}

//**********************************************************************
// 函数功能:    从重力传感器FIFO读取3轴数据
// 输入参数：    
// CacheArry ：          三轴数据指针
// CacheMaxWriteLenght:  读取的数据长度
//FIFOReadNumber：       FIFO有效数据长度
// 返回参数：    
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
uint8 Drv_Accel_BufferRead(int16 *CacheArry, uint16 CacheMaxWriteLenght, uint16 *FIFOReadNumber)
{
 //    uint8 fifoCount[2];
 //    uint16 fifolength;
         
 //    Accel_RegisterRead(BMI160_I2C_ADDR1, MPU_9250_FIFO_COUNTH_REG, fifoCount, 2);
 //    fifolength = (uint16)fifoCount[0]<<8 | fifoCount[1];

	// CacheMaxWriteLenght = CacheMaxWriteLenght <<1;
	// if(fifolength > CacheMaxWriteLenght)
	// {		
	// 	fifolength = CacheMaxWriteLenght;
	// }	
	// Accel_RegisterRead(BMI160_I2C_ADDR1, MPU_9250_FIFO_RW_REG, (uint8 *)CacheArry, fifolength);
 //    *FIFOReadNumber = fifolength >>1;   
	
    return 0x00;
}

//**********************************************************************
// 函数功能:    重力传感器唤醒
// 输入参数：   无
// 返回参数：    
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
uint8 Drv_Accel_WakeUp(void)
{
    uint8 ui8Regtemp[1] = {0x00};

    if (multisensor.accelstate != poweron)
    {
        // Setup i2c interface.    
        SMDrv_SWI2C_Open(ACC_IIC_MODULE,IIC_SPEED_HIGH);

        //唤醒芯片 
        ui8Regtemp[0] = ACCEL_MODE_NORMAL;
        Accel_RegisterWrite(BMI160_I2C_ADDR1, BMI160_CMD_COMMANDS_ADDR, ui8Regtemp, 1);
    }
    multisensor.accelstate = poweron;
    
    return 0x00;
}

//**********************************************************************
// 函数功能:    设置重力传感器进入睡眠状态
// 输入参数：   无
// 返回参数：    
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
uint8 Drv_Accel_GoSleep(void)
{
    multisensor.accelstate = sleep;
    uint8 ui8Regtemp[1]= {0X00};

    ui8Regtemp[0] = ACCEL_SUSPEND;
    Accel_RegisterWrite(BMI160_I2C_ADDR1, BMI160_CMD_COMMANDS_ADDR, ui8Regtemp, 1);

	Drv_Accel_Close();

    return 0x00;
}

//**********************************************************************
// 函数功能:   重力传感器复位
// 输入参数：   无
// 返回参数：    
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
uint8 Drv_Accel_Reset(void)
{
	
    return 0;
}

//**********************************************************************
// 函数功能:   重力传感器自检测
// 输入参数：   无
// 返回参数：    
// 0x00    :    自检成功
// 0xff    :    自检失败
//**********************************************************************
uint8 Drv_Accel_SelfTest(void)
{
   uint8  data;

    Drv_Accel_Open();
    // Selftest
    Accel_RegisterRead(BMI160_I2C_ADDR1, BMI160_USER_CHIP_ID_ADDR,&data, 1);
    Drv_Accel_Close();
    if(data == 0x01)
    {
        return 0x00;
    }           
    else
    {
        return 0xff;
    } 
}


