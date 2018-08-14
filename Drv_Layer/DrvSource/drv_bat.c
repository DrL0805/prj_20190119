/**********************************************************************
**
**模块说明: CW2015电量检测驱动接口
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.23  修改流程  ZSL  
**

**********************************************************************/
//#define BAT_MODULE
#define BAT_MODULE
#include "io_config.h"

//#include "io_config.h"
#include "sm_swi2c.h"
#include "sm_sys.h"
#include "sm_gpio.h"

#include "bsp_common.h"
#include "drv_bat.h"

#define ENABLE_PRINT 0
#if ENABLE_PRINT
#include "SEGGER_RTT.h"
#endif
/*******************macro define*******************/
//CELLWISE Fuel Gauge IC macros and func
#define		CELLWISE_SCL_PIN			8
#define		CELLWISE_SDA_PIN			9

// addr define
#define		CELLWISE_ADDR				0xC4

// const define
#define     CELLWISE_VERSION            0x6F
#define     MODE_SLEEP_MASK             (0x3<<6)
#define     MODE_SLEEP                  (0x3<<6)
#define     MODE_NORMAL                 (0x0<<6)
#define     MODE_QUICK_START            (0x3<<4)
#define     MODE_RESTART                (0xf<<0)        
#define     CONFIG_UPDATE_FLG           (0x1<<1)        
#define     ATHD                        (0x0<<3)        //ATHD = 0% 低电量中断阈值

// register macro
#define     CELLWISE_VERSION_REG        0x00 //IC版本号 默认为0x6F
#define     CELLWISE_VCELL_REG          0x02 //电池端的电压（两字节）
#define     CELLWISE_SOC_REG            0x04 //电量百分比%
#define     CELLWISE_SOC_DECIMAL_REG    0x05 //电量百分比 1/256%
#define     CELLWISE_RRT_ALRT_REG       0x06 //剩余时间、警报
#define     CELLWISE_CONFIG_REG         0x08 //低电量中断阈值设置、更新标识位
#define     CELLWISE_MODE_REG           0x0A // sleep1|0|QSTRT1|0|POR3|2|1|0
#define     CELLWISE_BATINFO_CONFIG     0x10



/*******************const define*******************/
#define BATINFO_SIZE    64

static uint8 cw_bat_modeling_info[BATINFO_SIZE] ={
0x17,0x67,0x63,0x63,0x60,0x5F,0x5F,0x5D, 
0x53,0x74,0x6A,0x4A,0x4E,0x48,0x44,0x3D, 
0x39,0x34,0x2F,0x32,0x38,0x47,0x4E,0x36, 
0x10,0x60,0x0C,0xCD,0x06,0x0D,0x5C,0x69, 
0x81,0x87,0x76,0x74,0x45,0x14,0x77,0x00, 
0x17,0x58,0x52,0x87,0x8F,0x91,0x94,0x52, 
0x82,0x8C,0x92,0x96,0x81,0x9E,0xC4,0xCB, 
0x2F,0x7D,0x72,0xA5,0xB5,0x0E,0x48,0x11, 
};



static uint8 cellwiseinitflag = 0;

static uint8 firstData = 1;
static bat_charge_envet oldBatChargeStatusOld = BAT_OFF_CHARGE;


#if((BAT_PG_PIN != IO_UNKNOW) && (BAT_CHG_PIN != IO_UNKNOW))
ChargeCheckCb Bat_ChgCheckCb = NULL;

//**********************************************************************
// 函数功能:	电池充电开始中断，5V上电，PG拉高，开始充电
// 输入参数：	无
// 返回参数：	无
void batcharge_pg_isr(uint32 u32pin)
{
    bat_charge_envet BatChargeStatus;

    if(u32pin != BAT_PG_PIN)
        return ;
    
	if(SMDrv_GPIO_InBitRead(BAT_PG_PIN))
	{
		if(SMDrv_GPIO_InBitRead(BAT_CHG_PIN))
		{
			BatChargeStatus = BAT_FULL_CHARGE;
		}
		else
		{
			BatChargeStatus = BAT_IN_CHARGING;
		}
		
        SMDrv_GPIO_ReConfigIrq(BAT_PG_PIN,0);
	}
	else
	{
		BatChargeStatus = BAT_OFF_CHARGE;

        SMDrv_GPIO_ReConfigIrq(BAT_PG_PIN,1);
	}
    if(Bat_ChgCheckCb != NULL)
    {
        (Bat_ChgCheckCb)(BatChargeStatus);
    }
}

//**********************************************************************
// 函数功能:	电池充电满中断，充电满，CHG拉高
// 输入参数：	无
// 返回参数：	无
void batcharge_chg_isr(uint32 u32pin)
{
    if(u32pin != BAT_CHG_PIN)
        return ;

	if(SMDrv_GPIO_InBitRead(BAT_CHG_PIN))
	{
		if(SMDrv_GPIO_InBitRead(BAT_PG_PIN))
		{
			if(Bat_ChgCheckCb != NULL)
			{
			    (Bat_ChgCheckCb)(BAT_FULL_CHARGE);
			}
		}
	}
}
#endif

//**********************************************************************
// 函数功能:    寄存器写
// 输入参数：    
// 返回参数：    无
static void Bat_RegWrite(uint8 dev_address, uint8 reg_address, uint8 *regval, uint16_t length)
{
    SMDrv_SWI2C_Write(BAT_IIC_MODULE,dev_address, reg_address, regval, length);
}                                                                             

//**********************************************************************
// 函数功能:    寄存器读
// 输入参数：    无
// 返回参数：    无
static void Bat_RegRead(uint8 dev_address, uint8 reg_address, uint8 *regval, uint16_t length)
{
    SMDrv_SWI2C_Read(BAT_IIC_MODULE,dev_address, reg_address, regval, length);
}

//**********************************************************************
// 函数功能：    在0x10寄存器写入电池建模信息
// 输入参数：    无
// 返回参数：    
// 0x00    :    初始化成功
// 0xff    :    初始化失败
uint8 Drv_Bat_ModelUpdate(void)
{
    uint8 ui8loop;
    uint8 reg_val;
    
    Bat_RegRead(CELLWISE_ADDR, CELLWISE_MODE_REG, &reg_val, 1);
    if((reg_val & MODE_SLEEP_MASK) == MODE_SLEEP)
    {
    #if ENABLE_PRINT
        RTT_PRINTF(0,"cw chip is in sleep mode\n");
    #endif
    }
    
    /* update new battery info */
    Bat_RegWrite(CELLWISE_ADDR, CELLWISE_BATINFO_CONFIG, cw_bat_modeling_info, BATINFO_SIZE);
    
    /* readback & check */
    for(ui8loop=0; ui8loop<BATINFO_SIZE; ui8loop++)
    {
        Bat_RegRead(CELLWISE_ADDR, CELLWISE_BATINFO_CONFIG+ui8loop, &reg_val, 1);
        if( reg_val != cw_bat_modeling_info[ui8loop] )
        {
        #if ENABLE_PRINT
            RTT_PRINTF(0,"cw model check error!!!\n");
        #endif
            return 0xFF;
        }            
    }
    
    /* set cw2015/cw2013 to use new battery info */
    Bat_RegRead(CELLWISE_ADDR, CELLWISE_CONFIG_REG, &reg_val, 1);
    reg_val |= CONFIG_UPDATE_FLG;   /* set UPDATE_FLAG */
	reg_val &= 0x07;                /* clear ATHD低电量中断阈值 */
	reg_val |= ATHD;                /* set ATHD */
    Bat_RegWrite(CELLWISE_ADDR, CELLWISE_CONFIG_REG, &reg_val, 1);
    
    /* reset */
    reg_val = MODE_RESTART;
    Bat_RegWrite(CELLWISE_ADDR, CELLWISE_MODE_REG, &reg_val, 1);
    SMDrv_SYS_DelayUs(100);
    reg_val = MODE_NORMAL;
    Bat_RegWrite(CELLWISE_ADDR, CELLWISE_MODE_REG, &reg_val, 1);
    
    return 0x00;    
}

//**********************************************************************
// 函数功能:    电量计初始化
// 输入参数：    无
// 返回参数：    
// 0x00    :    初始化成功
// 0xff    :    初始化失败
//**********************************************************************
uint8 Drv_Bat_Open(ChargeCheckCb chg_cb)
{
    uint8 ui8loop;
    uint8 i;
    uint8 reg_val = MODE_NORMAL;
    uint8 percentdata[2];

    SMDrv_SWI2C_Open(BAT_IIC_MODULE,IIC_SPEED_HIGH);
    SMDrv_GPIO_Close(BAT_INT_PIN);
    
    if(cellwiseinitflag == 0)
    {
        cellwiseinitflag = 1;
        /* wake up cw2015/13 from sleep mode */
        Bat_RegWrite(CELLWISE_ADDR, CELLWISE_MODE_REG, &reg_val, 1);
        /* check ATHD if not right */
        Bat_RegRead(CELLWISE_ADDR, CELLWISE_CONFIG_REG, &reg_val, 1);
        if((reg_val & 0xf8) != ATHD)
        {
            //"the new ATHD need set"
            reg_val &= 0x07;    /* clear ATHD */
            reg_val |= ATHD;    /* set ATHD */
            Bat_RegWrite(CELLWISE_ADDR, CELLWISE_CONFIG_REG, &reg_val, 1);
        }
        /* check config_update_flag if not right */
        Bat_RegRead(CELLWISE_ADDR, CELLWISE_CONFIG_REG, &reg_val, 1);
        if(!(reg_val & CONFIG_UPDATE_FLG))
        {
            Drv_Bat_ModelUpdate();
        }
        else
        {
            for(ui8loop=0; ui8loop<BATINFO_SIZE; ui8loop++)
            {
                Bat_RegRead(CELLWISE_ADDR, CELLWISE_BATINFO_CONFIG+ui8loop, &reg_val, 1);
                if( reg_val != cw_bat_modeling_info[ui8loop] )
                {
                #if ENABLE_PRINT
                    RTT_PRINTF(0,"cw model check error!!!\n");
                #endif
                    break;
                }            
            }
            if(ui8loop != BATINFO_SIZE)
            {
                //"update flag for new battery info need set"
                Drv_Bat_ModelUpdate();
            }
        }

        /* check SOC if not eqaul 255 */
        for (i = 0; i < 50; i++) 
        {
            SMDrv_SYS_DelayMs(10);//delay 100ms  
            Bat_RegRead(CELLWISE_ADDR, CELLWISE_SOC_REG, percentdata, 2);
            if (percentdata[0] <= 100)
            {
                break;  
            } 
        }
    }

#if((BAT_PG_PIN != IO_UNKNOW) && (BAT_CHG_PIN != IO_UNKNOW))
    //电池充电检测IO配置
    SMDrv_GPIO_Open(BAT_PG_PIN,NULL,batcharge_pg_isr);
    SMDrv_GPIO_Open(BAT_CHG_PIN,NULL,batcharge_chg_isr);
    if(chg_cb != NULL)
    {
        Bat_ChgCheckCb = chg_cb;
    }
#endif
    return 0x00;
}

//**********************************************************************
// 函数功能:   电量计自检
// 输入参数：   无
// 返回参数：    
// 0x00    :    自检成功
// 0xff    :    自检失败
//**********************************************************************
uint8 Drv_Bat_SelfTest(void)
{
   uint8  data;

    // Selftest
    Bat_RegRead(CELLWISE_ADDR, CELLWISE_VERSION_REG,&data, 1);
    if(data == CELLWISE_VERSION)
    {
    #if ENABLE_PRINT
        RTT_PRINTF(0,"CellWise test passed\n");
    #endif
        return 0x00;
    }           
    else
    {
    #if 0
        RTT_PRINTF(0,"CellWise test failed\n");
    #endif
        cellwiseinitflag = 0;
        return 0xff;
    } 
}

//**********************************************************************
// 函数功能:    读取电量剩余百分比，数据单位为%
// 输入参数：    
// percentData[0] ：   电量百分比整数位的数据
// percentData[1] ：   电量百分比小数位的数据
// 返回参数：    
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
uint8 Drv_Bat_ReadLevel(uint8 *p_i8percentData)
{
    uint8 percentdata[2];
    Bat_RegRead(CELLWISE_ADDR, CELLWISE_SOC_REG, percentdata, 2);
    p_i8percentData[0] = percentdata[0];
    p_i8percentData[1] = percentdata[1];    
    return 0x00;
}

//**********************************************************************
// 函数功能:    读取电池电压，数据单位为305uV
// 输入参数：    
// p_i8voltageData ：数据单位为mV
// 返回参数：    
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
uint8 Drv_Bat_ReadVoltage(uint16 *p_ui16voltageData)
{
    uint8 voltagedata[2];
    Bat_RegRead(CELLWISE_ADDR, CELLWISE_VCELL_REG, voltagedata, 2);
    *p_ui16voltageData = (uint16)((((uint16)voltagedata[0]<<8) + voltagedata[1])*305/1000);
    return 0x00;
}

//**********************************************************************
// 函数功能:    读取剩余时间，数据单位为：1分钟
// 输入参数：    
// p_i8voltageData ：数据单位为mV
// 返回参数：    
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
uint8 Drv_Bat_ReadRemainTime(uint16 *p_i16timeData)
{
    uint8 timedata[2];
    
    Bat_RegRead(CELLWISE_ADDR, CELLWISE_RRT_ALRT_REG, timedata, 2);
    timedata[0] &= ~(1<<8); 
    *p_i16timeData = ((uint16)timedata[0]<<8) + timedata[1];

    return 0x00;
}

//**********************************************************************
// 函数功能:    设置电量计进入睡眠状态
// 输入参数：   无
// 返回参数：    
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
uint8 Drv_Bat_GoSleep(void)
{
    uint8 ui8Regtemp[1] = {0x00};

    cellwiseinitflag = 0;
    
    Bat_RegRead(CELLWISE_ADDR, CELLWISE_MODE_REG, ui8Regtemp, 1);
    ui8Regtemp[0] |= 3<<6; 
    Bat_RegWrite(CELLWISE_ADDR, CELLWISE_MODE_REG, ui8Regtemp, 1);
    return 0x00;
}    

//**********************************************************************
// 函数功能:    电量计唤醒
// 输入参数：   无
// 返回参数：    
// 0x00    :    操作成功
// 0xff    :    操作失败
//**********************************************************************
uint8 Drv_Bat_WakeUp(void)
{
    uint8 ui8loop;
    uint8 reg_val = MODE_NORMAL;
    
    if(cellwiseinitflag == 0)
    {
        cellwiseinitflag = 1;
        /* wake up cw2015/13 from sleep mode */
        Bat_RegWrite(CELLWISE_ADDR, CELLWISE_MODE_REG, &reg_val, 1);
        /* check ATHD if not right */
        Bat_RegRead(CELLWISE_ADDR, CELLWISE_CONFIG_REG, &reg_val, 1);
        if((reg_val & 0xf8) != ATHD)
        {
            //"the new ATHD need set"
            reg_val &= 0x07;    /* clear ATHD */
            reg_val |= ATHD;    /* set ATHD */
            Bat_RegWrite(CELLWISE_ADDR, CELLWISE_CONFIG_REG, &reg_val, 1);
        }
        /* check config_update_flag if not right */
        Bat_RegRead(CELLWISE_ADDR, CELLWISE_CONFIG_REG, &reg_val, 1);
        if(!(reg_val & CONFIG_UPDATE_FLG))
        {
            Drv_Bat_ModelUpdate();
        }
        else
        {
            for(ui8loop=0; ui8loop<BATINFO_SIZE; ui8loop++)
            {
                Bat_RegRead(CELLWISE_ADDR, CELLWISE_BATINFO_CONFIG+ui8loop, &reg_val, 1);
                if( reg_val != cw_bat_modeling_info[ui8loop] )
                {
                #if ENABLE_PRINT
                    RTT_PRINTF(0,"cw model check error!!!\n");
                #endif
                    break;
                }            
            }
            if(ui8loop != BATINFO_SIZE)
            {
                //"update flag for new battery info need set"
                Drv_Bat_ModelUpdate();
            }
        }
    }

    return 0x00;
}

//**********************************************************************
// 函数功能:   设置充电检查回调
// 输入参数：   无
// 返回参数：    
// 0x00    :    自检成功
// 0x02    :    参数非法
//**********************************************************************
uint8 Drv_Bat_SetChargeCallBack(ChargeCheckCb chg_cb)
{
#if((BAT_PG_PIN != IO_UNKNOW) && (BAT_CHG_PIN != IO_UNKNOW))
    if(chg_cb == NULL)
        return Ret_InvalidParam;

    Bat_ChgCheckCb = chg_cb;
#endif
    return Ret_OK;
}

//**********************************************************************
// 函数功能:	电池充电状态检测
// 输入参数：	无
// 返回参数：	无
//**********************************************************************
void Drv_Bat_CheckChargeStatus(void)
{
#if((BAT_PG_PIN != IO_UNKNOW) && (BAT_CHG_PIN != IO_UNKNOW))
    bat_charge_envet BatChargeStatus;

    if (SMDrv_GPIO_InBitRead(BAT_PG_PIN))
	{
		if (SMDrv_GPIO_InBitRead(BAT_CHG_PIN))//full charge
		{
			BatChargeStatus = BAT_FULL_CHARGE;
		}
		else									//in charging
		{
			BatChargeStatus = BAT_IN_CHARGING;
		}
	}
	else										//off charging
	{
		BatChargeStatus = BAT_OFF_CHARGE;
	}
    
    if (firstData)
    {
        firstData = 0;
        //充电状态时返回
        if (oldBatChargeStatusOld != BatChargeStatus)
        {
            (Bat_ChgCheckCb)(BatChargeStatus);
        }
         oldBatChargeStatusOld = BatChargeStatus;
		
    }
	else
	{
		 if(Bat_ChgCheckCb != NULL && (oldBatChargeStatusOld != BatChargeStatus))
		{
            oldBatChargeStatusOld = BatChargeStatus;
			(Bat_ChgCheckCb)(BatChargeStatus);
		}
	}
#endif
}

//**********************************************************************
// 函数功能:    使能模块使用的IO功能
// 输入参数：    无
// 返回参数：    
// 0x00    :    设置成功
// 0x01    :    设置失败
//**********************************************************************
uint8 Drv_Bat_EnableIO(void)
{
    return SMDrv_SWI2C_Open(BAT_IIC_MODULE,IIC_SPEED_HIGH);
}

//**********************************************************************
// 函数功能:    关闭模块使用的IO功能，实现低功耗
// 输入参数：    无
// 返回参数：    
// 0x00    :    设置成功
// 0x01    :    设置失败
//**********************************************************************
uint8 Drv_Bat_DisableIO(void)
{
    return SMDrv_SWI2C_Close(BAT_IIC_MODULE); 
}





