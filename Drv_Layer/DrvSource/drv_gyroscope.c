/**********************************************************************
**
**模块说明: 陀螺仪驱动接口
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.20  修改流程  ZSL  
**
**********************************************************************/
#include "sm_swi2c.h"
#include "sm_sys.h"

#include "drv_gyroscope.h"
#include "bsp_common.h"


/*******************macro define*******************/
#define BMI160_I2C_ADDR2                   (0x68 << 1)	// SDO脚接GND
/**< I2C Address needs to be changed */
#define BMI160_I2C_ADDR1                   (0x69<< 1) 	// SDO脚接VDD，梁杰根据H003硬件原理图调换了下

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

#define BMI160_GYRO_OSR4_MODE       (0x00)
#define BMI160_GYRO_OSR2_MODE       (0x01)
#define BMI160_GYRO_NORMAL_MODE     (0x02)
#define BMI160_GYRO_CIC_MODE        (0x03)


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



//**********************************************************************
// 函数功能:    寄存器写
// 输入参数：    
// 返回参数：    无
static void Gyro_RegisterWrite(uint8 dev_address, uint8 reg_address, uint8 *regval, uint16 length)
{
     SMDrv_SWI2C_Write(GYR_IIC_MODULE,dev_address, reg_address, regval, length);
}

//**********************************************************************
// 函数功能:    寄存器读
// 输入参数：    无
// 返回参数：    无
static void Gyro_RegisterRead(uint8 dev_address, uint8 reg_address, uint8 *regval, uint16 length)
{
    SMDrv_SWI2C_Read(GYR_IIC_MODULE,dev_address, reg_address, regval, length);
}

//**********************************************************************
// 函数功能:    陀磥仪传感器初始化
// 输入参数：    无
// 返回参数：    
// 0x00    :    初始化成功
// 0xff    :    初始化失败
//**********************************************************************
uint8 Drv_Gyro_Open(void)
{
    uint8 ui8Regtemp[1] = {0x00};
    uint16 ui16ret;
   
    SMDrv_SWI2C_Open(GYR_IIC_MODULE,IIC_SPEED_HIGH);
    
    if(multisensor.gyrostate == uinit &&  multisensor.accelstate == uinit)
    {
        //FIFO使能
        #ifdef ACCEL_BUFFER_ENABLE
        Drv_Gyro_SetBuffer(1);
        #endif     

        ui8Regtemp[0] = (BMI160_GYRO_NORMAL_MODE << 5);
        Gyro_RegisterWrite(BMI160_I2C_ADDR1, BMI160_USER_GYRO_CONFIG_ADDR, ui8Regtemp, 1); 
    }
	multisensor.gyrostate = poweron;
 
    return ui16ret;
}

//**********************************************************************
// 函数功能:    硬件关闭
// 输入参数：    无
// 返回参数：    
// 0x00    :    设置成功
// 0xff    :    设置失败
//**********************************************************************
uint8 Drv_Gyro_Close(void)
{
   // close i2c interface.
   SMDrv_SWI2C_Close(GYR_IIC_MODULE);

    multisensor.gyrostate = powerdown;

    return 0;
}

//**********************************************************************
// 函数功能:    陀磥仪传感器采样率和量程设置
// 输入参数：    
// sampleRate    采样率
//               Valid Values for uint16_t sampleRate are:
//               GYO_SAMPLERATE_5HZ_1KBW        GYO_SAMPLERATE_10HZ_1KBW
//               GYO_SAMPLERATE_20HZ_1KBW       GYO_SAMPLERATE_41HZ_1KBW
//               GYO_SAMPLERATE_92HZ_1KBW       GYO_SAMPLERATE_184HZ_1KBW
//               GYO_SAMPLERATE_250HZ_8KBW      GYO_SAMPLERATE_3K6HZ_8KBW
//               GYO_SAMPLERATE_3K6HZ_32KBW     GYO_SAMPLERATE_8K8HZ_32KBW
// scaleRange    测量量程
//               Gyro fullscale selection : +250dps (00), +500dps (01), +1000dps (10), +2000dps (11)
//               Valid Values for uint8_t scaleRange are:
//               GYOR_SCALE_RANGE_250    GYOR_SCALE_RANGE_500
//               GYOR_SCALE_RANGE_1000   GYOR_SCALE_RANGE_2000
// 返回参数：    
// 0x00    :    初始化成功
// 0xff    :    初始化失败
//**********************************************************************
uint8 Drv_Gyro_Set(uint16 sampleRate, uint8 scaleRange)
{
    uint8 ui8Regtemp[1] = {0x00};
    
    if (Drv_Gyro_WakeUp())
    {
        return 0xff;
    }
    //Set Sample Rate
    if(sampleRate != GYO_SAMPLERATE_25HZ   && 
       sampleRate != GYO_SAMPLERATE_50HZ   && 
       sampleRate != GYO_SAMPLERATE_100HZ  && 
       sampleRate != GYO_SAMPLERATE_200HZ  && 
       sampleRate != GYO_SAMPLERATE_400HZ  &&
       sampleRate != GYO_SAMPLERATE_800HZ  &&
       sampleRate != GYO_SAMPLERATE_1600HZ  &&
       sampleRate != GYO_SAMPLERATE_3200HZ)
    {return 0xFF;}

    ui8Regtemp[0] = 0x00;
    Gyro_RegisterRead(BMI160_I2C_ADDR1, BMI160_USER_GYRO_CONFIG_ADDR, ui8Regtemp, 1);
    ui8Regtemp[0] = (ui8Regtemp[0] & 0xF0) | sampleRate;                                          
    Gyro_RegisterWrite(BMI160_I2C_ADDR1, BMI160_USER_GYRO_CONFIG_ADDR, ui8Regtemp, 1);

    // Disable gyro xyz's selftest, set SacleRange
    if(scaleRange != GYRO_SCALE_RANGE_2000_DEG_SEC &&
       scaleRange != GYRO_SCALE_RANGE_1000_DEG_SEC && 
       scaleRange != GYRO_SCALE_RANGE_500_DEG_SEC &&
       scaleRange != GYRO_SCALE_RANGE_250_DEG_SEC &&
       scaleRange != GYRO_SCALE_RANGE_125_DEG_SEC)
    {return 0xFF;}
                                       
    ui8Regtemp[0] = 0x00;
    ui8Regtemp[0] |= scaleRange ;
    Gyro_RegisterWrite(BMI160_I2C_ADDR1, BMI160_USER_GYRO_RANGE_ADDR, ui8Regtemp, 1);

    return 0x00;
}

//**********************************************************************
// 函数功能:    陀螺仪传感器FIFO功能开关设置
// 输入参数：    
// setState ：   FIFO开关，0x00关闭，0x01使能
// 返回参数：    
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
uint8 Drv_Gyro_SetBuffer(uint16 setState)
{
    uint8 ui8Regtemp[1] = {0x00};
    uint8 ui8Ret;
    
    if(setState == 0x01)//使能FIFO
    {
        Gyro_RegisterRead(BMI160_I2C_ADDR1,  BMI160_USER_FIFO_CONFIG_1_ADDR, ui8Regtemp, 1);
        ui8Regtemp[0] |= (FIFO_HEADER_ENABLE << 4);
        Gyro_RegisterWrite(BMI160_I2C_ADDR1, BMI160_USER_FIFO_CONFIG_1_ADDR, ui8Regtemp, 1);    

        Gyro_RegisterRead(BMI160_I2C_ADDR1,  BMI160_USER_FIFO_CONFIG_1_ADDR, ui8Regtemp, 1);
        ui8Regtemp[0] |= (FIFO_GYRO_ENABLE << 7);
        Gyro_RegisterWrite(BMI160_I2C_ADDR1, BMI160_USER_FIFO_CONFIG_1_ADDR, ui8Regtemp, 1);           
    }
    else if(setState == 0x00)//关闭FIFO
    {
         Gyro_RegisterRead(BMI160_I2C_ADDR1,  BMI160_USER_FIFO_CONFIG_1_ADDR, ui8Regtemp, 1);
        ui8Regtemp[0] &= ~(FIFO_GYRO_ENABLE << 7);
        Gyro_RegisterWrite(BMI160_I2C_ADDR1, BMI160_USER_FIFO_CONFIG_1_ADDR, ui8Regtemp, 1); 
    }
    return ui8Ret;
}

//**********************************************************************
// 函数功能:    读取陀螺仪传感器3轴数据，数据为二进补码形式
// 输入参数：    
// axisData ：   三轴数据指针
// 返回参数：    
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
uint8 Drv_Gyro_Read(int16 xyzData[3])
{
    uint8 axisdata[6];
    Gyro_RegisterRead(BMI160_I2C_ADDR1, BMI160_USER_DATA_8_ADDR, axisdata, 6);
    xyzData[0] = ((uint16)axisdata[1]<<8) + axisdata[0];
    xyzData[1] = ((uint16)axisdata[3]<<8) + axisdata[2];
    xyzData[2] = ((uint16)axisdata[5]<<8) + axisdata[4];

    return 0x00;
}

//**********************************************************************
// 函数功能:    从陀螺仪传感器FIFO读取3轴数据
// 输入参数：    
// CacheArry ：          三轴数据指针
// CacheMaxWriteLenght:  读取的数据长度
//FIFOReadNumber：       FIFO有效数据长度
// 返回参数：    
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
uint8 Drv_Gyro_ReadBuffer(int16 *CacheArry, uint16 CacheMaxWriteLenght, uint16 *FIFOReadNumber)
{
       
    return 0x00;
}

//**********************************************************************
// 函数功能:    陀螺仪传感器唤醒
// 输入参数：   无
// 返回参数：    
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
uint8 Drv_Gyro_WakeUp(void)
{
    uint8 ui8Regtemp[1] = {0x00};

    if (multisensor.gyrostate != poweron)
    {
        // Setup i2c interface.    
        SMDrv_SWI2C_Open(GYR_IIC_MODULE,IIC_SPEED_HIGH);

        //唤醒芯片 
        ui8Regtemp[0] = GYRO_MODE_NORMAL;
        Gyro_RegisterWrite(BMI160_I2C_ADDR1, BMI160_CMD_COMMANDS_ADDR, ui8Regtemp, 1);
    }

    multisensor.gyrostate = poweron;
    return 0x00;       
}

//**********************************************************************
// 函数功能:    设置陀螺仪传感器进入睡眠状态
// 输入参数：   无
// 返回参数：    
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
uint8 Drv_Gyro_GoSleep(void)
{
    multisensor.gyrostate = sleep;
    uint8 ui8Regtemp[1] = {0x00};

	Drv_Gyro_Open();
	
    ui8Regtemp[0] = GYRO_MODE_SUSPEND;
    Gyro_RegisterWrite(BMI160_I2C_ADDR1, BMI160_CMD_COMMANDS_ADDR, ui8Regtemp, 1);

    if (multisensor.accelstate == uinit || multisensor.accelstate != poweron)//重力传感器未使用
    {
        ui8Regtemp[0] = ACCEL_SUSPEND;
        Gyro_RegisterWrite(BMI160_I2C_ADDR1, BMI160_CMD_COMMANDS_ADDR, ui8Regtemp, 1);
    }
    else
    {
        ui8Regtemp[0] = ACCEL_LOWPOWER;
        Gyro_RegisterWrite(BMI160_I2C_ADDR1, BMI160_CMD_COMMANDS_ADDR, ui8Regtemp, 1);
    }

	if(multisensor.accelstate != poweron)
	{
		Drv_Gyro_Close();
	}
	return 0x00;       
}

//**********************************************************************
// 函数功能:   陀螺仪传感器自检测
// 输入参数：   无
// 返回参数：    
// 0x00    :    自检成功
// 0xff    :    自检失败
//**********************************************************************
uint8 Drv_Gyro_SelfTest(void)
{
   uint8  data;

   Drv_Gyro_Open();
    // Selftest
    Gyro_RegisterRead(BMI160_I2C_ADDR1, BMI160_USER_CHIP_ID_ADDR,&data, 1);
	
	Drv_Gyro_GoSleep();
	
    Drv_Gyro_Close();
    if(data == 0xD1)
    {
        return 0x00;
    }           
    else
    {
        return 0xff;
    } 
}

