/**********************************************************************
**
**模块说明: 水压驱动接口
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.20  修改流程  ZSL  
**
**********************************************************************/
#include "sm_swi2c.h"
#include "drv_waterpress.h"

/*******************macro define*******************/
// devicc address (MS5832-30BA)
#define DEV_ADDR           0xEC

// command (MS5832-30BA)
#define RESET_CMD          0x1E
#define ADC_READ_CMD       0x00
#define PROM_READ_CMD      0xA0

/*******************variable define*******************/
//标准大气压,单位为Pa
static WaterPressSensorState SensorState;

// calibrated coefficients 
//static uint16_t Prom_C0 =	0;  //C0	
static uint16 Prom_C1 =	0;  //C1	
static uint16 Prom_C2 =	0;  //C2	
static uint16 Prom_C3 =	0;  //C3	
static uint16 Prom_C4 =	0;  //C4	
static uint16 Prom_C5 =	0;  //C5	
static uint16 Prom_C6 =	0;  //C6	

//**********************************************************************
// 函数功能: 设置传感器寄存器
// 输入参数：	
// u8Reg   : 命令或偏移地址
// data    : 写入的数据
// dLen    : 写入的数据长度，长度为0时，只写命令
// 返回参数：无
//**********************************************************************
static void WaterPress_RegWrite(uint8 u8Reg,uint8 *data,uint8 dLen)
{
    if (dLen == 0)
    {
        SMDrv_SWI2C_WriteCmd(WP_IIC_MODULE,DEV_ADDR,u8Reg);
    }
    else
    {
        SMDrv_SWI2C_Write(WP_IIC_MODULE,DEV_ADDR,u8Reg, data, dLen);
    }
}

//**********************************************************************
// 函数功能: 读传感器寄存器
// 输入参数：	
// cmd 	   :  命令或偏移地址
// data    :  数据指针
// dLen    :  读取的数据长度
// 返回参数： 无
//**********************************************************************
static void WaterPress_RegRead(uint8 u8Reg,uint8 *data,uint8 dLen)
{
    SMDrv_SWI2C_Read(WP_IIC_MODULE,DEV_ADDR, u8Reg, data, dLen);
}

//**********************************************************************
// 函数功能: 读取校准参数
// 输入参数：无
// 返回参数：无
static void WaterPress_ReadProm(void)
{
    uint8 i;
    uint8 promcmd = PROM_READ_CMD;
    uint16 prombuf[7];
    uint8 promregbuf[2];

    for(i =	0;i	< 7;i++)
    {
        WaterPress_RegRead(promcmd,promregbuf,2);
        promcmd	+= 0x02;
        prombuf[i] = (uint16)promregbuf[0]<<8;
        prombuf[i] |= (uint16)promregbuf[1];
    }

    //Prom_C0 =	prombuf[0];
    Prom_C1	= prombuf[1];
    Prom_C2	= prombuf[2];
    Prom_C3	= prombuf[3];
    Prom_C4	= prombuf[4];
    Prom_C5	= prombuf[5];
    Prom_C6	= prombuf[6];
}

//**********************************************************************
// 函数功能:    使能模块使用的IO功能
// 输入参数：	无
// 返回参数：	
// 0x00    :    设置成功
// 0x01    :    设置失败
//**********************************************************************
uint8 Drv_WaterPress_EnableIO(void)
{
    return SMDrv_SWI2C_Open(WP_IIC_MODULE,IIC_SPEED_HIGH);
}

//**********************************************************************
// 函数功能:	关闭模块使用的IO功能，实现低功耗
// 输入参数：	无
// 返回参数：	
// 0x00    :    设置成功
// 0x01    :    设置失败
//**********************************************************************
uint8 Drv_WaterPress_DisableIO(void)
{
    return SMDrv_SWI2C_Close(WP_IIC_MODULE); 
}

//**********************************************************************
// 函数功能: 传感器软件初始化
// 输入参数：无
// 返回参数：	
// 0x00    : 设置成功
// 0x01    : 设置失败
//**********************************************************************
uint8 Drv_WaterPress_Init(void)
{
    uint8 data = 0;

    //open iic
    if(SMDrv_SWI2C_Open(WP_IIC_MODULE,IIC_SPEED_HIGH) != Ret_OK)
        return Ret_Fail;

    //soft reset WaterPress
    WaterPress_RegWrite(RESET_CMD,&data,0);
    WaterPress_ReadProm();

    SensorState = SensorIdle;
    return Ret_OK;
}

//**********************************************************************
// 函数功能: 硬件关闭
// 输入参数：无
// 返回参数：	
// 0x00    : 设置成功
// 0xff    : 设置失败
//**********************************************************************
uint8 Drv_WaterPress_Close(void)
{
    SMDrv_SWI2C_Close(WP_IIC_MODULE); 
    SensorState = SensorIdle;
    return Ret_OK;
}

//**********************************************************************
// 函数功能: 启动传感器压力ADC转换
// 输入参数：无
// 返回参数：	
// 0x00    : 设置成功
// 0x01    : 设置失败
// 0x03    : 设备忙
//**********************************************************************
uint8 Drv_WaterPress_PressCvt(uint8 PressOSR) 
{
    uint8 data  = 0;

    if(SensorState != SensorIdle)
    {
        return Ret_DeviceBusy;
    }

    if((PressOSR != PRESS_D1_OSR_256) && (PressOSR != PRESS_D1_OSR_512) && \
       (PressOSR != PRESS_D1_OSR_1024) && (PressOSR != PRESS_D1_OSR_2048) && \
       (PressOSR != PRESS_D1_OSR_4096) && (PressOSR != PRESS_D1_OSR_8192))
    {
        return Ret_InvalidParam;
    }

    WaterPress_RegWrite(PressOSR,&data,0);	
    SensorState = PressCVT;
    return Ret_OK;
}

//**********************************************************************
// 函数功能: 启动传感器温度ADC转换
// 输入参数：无
// 返回参数：	
// 0x00    : 设置成功
// 0x01    : 设置失败
// 0x03    : 设备忙
//**********************************************************************
uint8 Drv_WaterPress_TempCvt(uint8 TempOSR)
{
    uint8 data = 0;

    if(SensorState != SensorIdle)
    {
        return Ret_DeviceBusy;
    }

    if((TempOSR != TEMP_D2_OSR_256) && (TempOSR != TEMP_D2_OSR_512) &&  \
       (TempOSR != TEMP_D2_OSR_1024) && (TempOSR != TEMP_D2_OSR_2048) && \
       (TempOSR != TEMP_D2_OSR_4096) && (TempOSR != TEMP_D2_OSR_8192))
    {
        return Ret_InvalidParam;
    }
    WaterPress_RegWrite(TempOSR,&data,0);
    SensorState = TempCVT;

    return Ret_OK;
}

//**********************************************************************
// 函数功能:  读取传感器AD值（温度/压力AD值）
// 输入参数： 无
// 返回参数：	
// 0x00    :  设置成功
// 0x01    :  设置失败
//**********************************************************************
uint8 Drv_WaterPress_ReadADC(uint32 *adcval)
{	
    uint8 RegVal[3] = {0};
    uint32 Dval = 0;

    if((SensorState != PressCVT) && (SensorState != TempCVT))
    {
        return Ret_Fail;
    }
    
    WaterPress_RegRead(ADC_READ_CMD,RegVal,3);
    Dval = (uint32)RegVal[0]<<16;
    Dval |=	(uint32)RegVal[1]<<8;
    Dval |=	(uint32)RegVal[2];
    *adcval = Dval;
    SensorState = SensorIdle;
    return Ret_OK;
}

//**********************************************************************
// 函数功能: 根据温度AD值计算温度值（单位C*100）
// 输入参数：无
// 返回参数：	
// 0x00    : 设置成功
// 0x01    : 设置失败
//**********************************************************************
uint8 Drv_WaterPress_CalTemp(uint32 tempDval,int32 *Temp)
{
    int64 TEMP = 0;
    int64 dT = 0;
    int64 T2 = 0;
    int64 OFF2 = 0;
    int64 SENS2 = 0;

    dT = tempDval - (Prom_C5 << 8);
    TEMP = 2000 + ((dT * Prom_C6) >> 23);

    if (TEMP / 100.0 < 20)// temperature compensation appllied here
    {
        T2 = (3 * dT * dT) >> 33;
        OFF2 = (3 * (TEMP - 2000) * (TEMP - 2000)) >> 1;
        SENS2 = (5 * (TEMP - 2000)* (TEMP - 2000)) >> 3;
        if (TEMP / 100.0 < -15) 
        {
            OFF2 = OFF2 + 7 * (TEMP + 1500) * (TEMP + 1500);
            SENS2 = SENS2 + 4 * (TEMP + 1500) * (TEMP + 1500);
        }
    }
    else
    {	
        T2 = (2 * dT * dT) >> 37;
        OFF2 = (1 * (TEMP - 2000) * (TEMP - 2000)) >> 4;
        SENS2 = 0;
    }	

    TEMP = TEMP - T2;
    // *Temp = TEMP / 100.0;//Unit C
    *Temp =	TEMP;// Unit C*100
    return Ret_OK;
}

//**********************************************************************
// 函数功能: 根据温度及压力AD值计算压力值（单位Pa）
// 输入参数：无
// 返回参数：	
// 0x00    : 设置成功
// 0x01    : 设置失败
//**********************************************************************
uint8 Drv_WaterPress_CalPress(uint32 tempDval, uint32 pressDval, int32 *Press)
{	
    int64 TEMP	= 0;
    int64 P = 0;
    int64 dT = 0;
    int64 OFF = 0;
    int64 SENS = 0;
    //int64_t T2 = 0;
    int64 OFF2 = 0;
    int64 SENS2 = 0;

    dT = tempDval - (Prom_C5 << 8);
    TEMP = 2000 + ((dT * Prom_C6) >> 23);
    OFF = (Prom_C2 << 16) + ((Prom_C4 * dT) >> 7);
    SENS = (Prom_C1 << 15) + ((Prom_C3 * dT) >> 8);

    if(TEMP / 100.0 < 20)// temperature compensation appllied here
    {
        //T2 = (3 * dT * dT) >> 33;
        OFF2 = (3 * (TEMP - 2000) * (TEMP - 2000)) >> 1;
        SENS2 = (5 * (TEMP - 2000)* (TEMP - 2000)) >> 3;
        if (TEMP / 100.0 < -15) 
        {
            OFF2 = OFF2 + 7 * (TEMP + 1500) * (TEMP + 1500);
            SENS2 = SENS2 + 4 * (TEMP + 1500) * (TEMP + 1500);
        }
    }
    else
    {	
        //T2 = (2 * dT * dT) >> 37;
        OFF2 = (1 * (TEMP - 2000) * (TEMP - 2000)) >> 4;
        SENS2 = 0;
    }		
    //TEMP = TEMP - T2;
    OFF = OFF - OFF2;
    SENS = SENS - SENS2;
    //*Temp = TEMP / 100.0;//Unit C
    P = ((((pressDval * SENS) >> 21) - OFF) >> 13);
    *Press = (int32) P * 10.0;// Unit Pa

    return Ret_OK;
}

//**********************************************************************
// 函数功能: 设置传感器空闲状态
// 输入参数：无
// 返回参数：	
// 0x00    : 设置成功
// 0x01    : 设置失败
//**********************************************************************
uint8 Drv_WaterPress_SetIdle(void)
{	
    SensorState = SensorIdle;
    return Ret_OK;
}

//**********************************************************************
// 函数功能: 传感器自检
// 输入参数：无
// 返回参数：	
// 0x00    : 自检通过
// 0x01    : 自检失败
//**********************************************************************
uint8 Drv_WaterPress_SelfTest(void)
{
    uint8 i;
    uint8 promcmd = PROM_READ_CMD;
    uint16 prombuf[7];
    uint8 promregbuf[2];
    uint8 retcnt1 = 0;
    uint8 retcnt2 = 0;

    if(Drv_WaterPress_EnableIO() != Ret_OK)
        return Ret_Fail;

    if(Drv_WaterPress_Init() != Ret_OK)
        return Ret_Fail;

    for(i =	0;i	< 7;i++)
    {
        WaterPress_RegRead(promcmd,promregbuf,2);
        promcmd	+=	0x02;
        prombuf[i] = (uint16)promregbuf[0]<<8;
        prombuf[i] |= (uint16)promregbuf[1];
        if (prombuf[i] == 0xffff)
        {
            retcnt1++;
        }
        if (prombuf[i] == 0)
        {
            retcnt2++;
        }
    }

    Drv_WaterPress_DisableIO(); 
    if((retcnt1 == 7) || (retcnt2 == 7))
    {
        return Ret_Fail;
    }
    return Ret_OK;
}

