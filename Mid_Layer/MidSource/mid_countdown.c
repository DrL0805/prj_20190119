/**********************************************************************
**
**模块说明: mid层KEY接口
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.24  修改流程  ZSL  
**
**********************************************************************/
#include "sm_timer.h"
#include "mid_countdown.h"

static Mid_CountDown_Format_t  countdownTime;

static Mid_CountDown_Param_t	MidCountDown;

//**********************************************************************
// 函数功能: 
// 输入参数：无
// 返回参数：无
//**********************************************************************
void Mid_Countdown_Isr(void)
{
	if(0 == --MidCountDown.RemainSec)
	{
		Mid_Countdown_Stop();
	}
	
//	SEGGER_RTT_printf(0,"MidCountDown.RemainSec %d \n", Mid_Countdown_RemainRead());
}

//**********************************************************************
// 函数功能: 倒计时复位
// 输入参数：无 
// 返回参数：无 
//**********************************************************************
void Mid_Countdown_Reset(void)
{
	MidCountDown.RemainSec = MidCountDown.TotalSec;

    SMDrv_CTimer_Clear(COUNTDOWN_CTIMER_MODULE);
}

//**********************************************************************
// 函数功能: 倒计时初始化,硬件初始化，并注册中断回调函数
// 输入参数：无
// 返回参数：无
//**********************************************************************
void Mid_Countdown_Init(void)
{
	MidCountDown.InitedFlg = true;

    SMDrv_CTimer_Open(COUNTDOWN_CTIMER_MODULE, COUNTDOWN_PERIOD_MS, Mid_Countdown_Isr);
    Mid_Countdown_Reset();
}

//**********************************************************************
// 函数功能: 倒计时开始
// 输入参数：无  
// 返回参数：无
//**********************************************************************
void Mid_Countdown_Start(void)
{
	MidCountDown.RuningFlg = true;
	
    SMDrv_CTimer_Start(COUNTDOWN_CTIMER_MODULE);
}

//**********************************************************************
// 函数功能: 倒计时停止
// 输入参数：无
// 返回参数：无
//**********************************************************************
void Mid_Countdown_Stop(void)
{
	MidCountDown.RuningFlg = false;
	
    SMDrv_CTimer_Stop(COUNTDOWN_CTIMER_MODULE);
}

//**********************************************************************
// 函数功能: 读取倒计时剩余时间
// 输入参数：timeTemp：传入变量，保存倒计时剩余时间
// 返回参数：无 
//**********************************************************************
 uint32_t Mid_Countdown_RemainRead(void)
{
	return MidCountDown.RemainSec;
}

//**********************************************************************
// 函数功能: 设置/写入倒计时剩余时间
// 输入参数：timeTemp：传入倒计时剩余时间参数
//        
// 返回参数：无  
//**********************************************************************
void Mid_Countdown_TimeWrite(uint32_t Sec)
{
	MidCountDown.TotalSec = Sec;
	MidCountDown.RemainSec = Sec;
}

