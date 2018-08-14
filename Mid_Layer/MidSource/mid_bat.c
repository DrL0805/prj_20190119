/**********************************************************************
**
**模块说明: mid层Bat接口
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.25  修改流程  ZSL  
**
**********************************************************************/
#include "drv_bat.h"
#include "mid_bat.h"

#define APP_BAT_FULL   4300
#define APP_BAT_LOW    3600

#define BAT_CHECK_TIMES 1

static uint8  batChangeState; 

static uint8  batSoc = 100;                     //电池电量：%
static uint16 batVoltage = 4300;             //电池电压：mV
static uint8  batLevel = MID_BAT_LEVER_5;     //电量级别

/*******************function define*******************/
static midbat_cb pBatMsg_Cb = NULL;



//**********************************************************************
// 函数功能: 提供注册函数接口，给上层进行处理电池电量数据
// 输入参数：Cb：函数地址
// 返回参数：无
//**********************************************************************
void Mid_Bat_SetCallBack(midbat_cb bat_msg)
{
    if(bat_msg == NULL)
        return;
    pBatMsg_Cb = bat_msg;
}

//**********************************************************************
// 函数功能:  回调函数，当充电状态检测完成，对数据进行处理。
//      这里包括两个操作，一、保存电池充电状态，二、提供注册函数接口，给上层进行处理
// 输入参数： state：电池充电状态
// 返回参数： 无
//**********************************************************************
void Mid_Bat_ChargeState_IOIsr(bat_charge_envet evnet)
{
	batChangeState = evnet;

    if(pBatMsg_Cb != NULL)
    {
        (pBatMsg_Cb)(batChangeState);
    }
}

//**********************************************************************
// 函数功能:  电池电量测量初始化
//     电池电量测量初始化，完成基本的初始化，包括初始化ADC模块，
///    以及注册回调函数1，把ADC采集到的数据读取出来，进行处理.
//          注册回调函数2，把电池的充电状态上传
//     此函数执行完还未开始采集数据。因为要等电量稳定时，再开
//     启电池电量的数据采集
// 输入参数： 无 
// 返回参数： 无
//**********************************************************************
void Mid_Bat_Init(void)
{
	Drv_Bat_Open(Mid_Bat_ChargeState_IOIsr);
    // Drv_Bat_GoSleep(); //暂定电量计一直开启
    Drv_Bat_DisableIO();
	pBatMsg_Cb	= NULL;
}

//**********************************************************************
// 函数功能: 读取电池电量  
// 输入参数：dataTemp：传入变量，保存电池电量数据 
// 返回参数：无
//**********************************************************************
void Mid_Bat_VolRead(uint16 *dataTemp)
{
	*dataTemp = batVoltage;
}

//**********************************************************************
// 函数功能:  把ADC采集的电池电压采集值转化成电池电量百分比 
// 输入参数： dataTemp：传入变量，保存电池电量百分比
// 返回参数： 电池类型
//**********************************************************************
uint8 Mid_Bat_SocRead(uint8 *dataTemp)
{	
    *dataTemp = batSoc;

    return CHARGING_CELL;
}

//**********************************************************************
// 函数功能:  获取电量百分比并作等级评估
// 输入参数： 无
// 返回参数： 电量等级
//**********************************************************************
uint8 Mid_Bat_LevelRead(void)
{
    return batLevel;
}

//**********************************************************************
// 函数功能:  启动一次电池检测
// 输入参数： 无 
// 返回参数： 无
//**********************************************************************
static void Mid_Bat_BatCheck(void)
{   
    static uint8 checkCnt = 0;
    uint8 batleveltemp[2];
    uint16 u16Temp;

    Drv_Bat_EnableIO();

    Drv_Bat_ReadLevel(batleveltemp);
    Drv_Bat_ReadVoltage(&u16Temp);

    // if (checkCnt >= BAT_CHECK_TIMES)
    // {
        checkCnt = 0;
        batSoc = batleveltemp[0];
        batVoltage = u16Temp;

        if (batSoc >= 100)
        {
            batSoc = 100;
        }
        if (batSoc > 80)
        {
            batLevel = MID_BAT_LEVER_5;
        }else if (batSoc > 60)
        {
            batLevel =  MID_BAT_LEVER_4;
        }else if (batSoc > 40)
        {
            batLevel =  MID_BAT_LEVER_3;
        }else if (batSoc > 20)
        {
            batLevel =  MID_BAT_LEVER_2;
        }else if (batSoc > 5)
        {
            batLevel =  MID_BAT_LEVER_1;
        }
        else 
        {
            batLevel =  MID_BAT_LEVER_0;
        } 
        // Drv_Bat_GoSleep();
    // }
    // else
    // {
    //     checkCnt ++;
    // }
    Drv_Bat_DisableIO();
}

//**********************************************************************
// 函数功能:  电量计唤醒
// 输入参数： 无 
// 返回参数： 无
//**********************************************************************
static void Mid_Bat_WakeUp(void)
{
    Drv_Bat_EnableIO();
    Drv_Bat_WakeUp();
    Drv_Bat_DisableIO();
}

//**********************************************************************
// 函数功能:  电量计休眠
// 输入参数： 无 
// 返回参数： 无
//**********************************************************************
static void Mid_Bat_WakeSleep(void)
{
    Drv_Bat_EnableIO();
    Drv_Bat_GoSleep();
    Drv_Bat_DisableIO();
}

//**********************************************************************
// 函数功能:  对电池的充电状态进行一次检测
// 输入参数： 无 
// 返回参数： 无
//**********************************************************************
static void Mid_Bat_ChargeStateCheck(void)
{
	Drv_Bat_CheckChargeStatus();
}

//**********************************************************************
// 函数功能:  读取电池的充电状态  
// 输入参数： dataTemp：传入变量，保存电池的充电状态
// 返回参数： 无
//**********************************************************************
void Mid_Bat_ChargeStateRead(uint8 *dataTemp)
{
	*dataTemp = batChangeState;
}

//**********************************************************************
// 函数功能:  电池充电、电量处理事件  
// msg     :    事件信息
// 返回参数： 无
//**********************************************************************
uint16 Mid_Bat_EventProcess(bat_event_s* msg)
{
    switch(msg->id)
    {
        case BAT_WAKEUP:
        Mid_Bat_WakeUp();
        break;

        case BAT_SLEEP:
        Mid_Bat_WakeSleep();
        break;

        case BAT_CHECK:
        Mid_Bat_BatCheck();
        break;

        case CHARGE_CHECK:
        Mid_Bat_ChargeStateCheck();
        break;

        default:
        break;
    }
    return 0;
}

//**********************************************************************
// 函数功能:  电池检测 
// 输入参数： 无
// 返回参数： 电池检测结果
//**********************************************************************
uint16 Mid_Bat_SelfTest(void)
{
    uint16 result = 0;

    Drv_Bat_EnableIO();
    // Drv_Bat_WakeUp();

    if (Drv_Bat_SelfTest())
    {
        result |= 0x00ff;
    }

    Drv_Bat_ReadVoltage(&batVoltage);

    if (batVoltage < APP_BAT_LOW)
    {
        result |= 0xff00;
    }
     // Drv_Bat_GoSleep();
    Drv_Bat_DisableIO();

    return result;
}


