/**********************************************************************
**
**模块说明: mid层KEY接口
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.24  修改流程  ZSL  
**
**********************************************************************/
#include "sm_timer.h"
#include "mid_countdown.h"

//countdown 回调函数
static mid_countdown_cb pCountDown_Msg = NULL; 
static countdown_time_s  countdownTime;
static uint16 countdownCnt = 0;  // countdownCnt++ per 31250 us

//**********************************************************************
// 函数功能: 回调函数，当倒计时31.25ms完成时，对时间进行计数操作，并对阶段时间进行处理
//       倒计时31.25ms完成后，对时间进行计数操作，并对阶段性时间进行处理
// 输入参数：无
// 返回参数：无
//**********************************************************************
void Mid_Countdown_Isr(void)
{
    countdownCnt++;

    if(countdownCnt == 16)
    {
        if(pCountDown_Msg != NULL)
        {
            (pCountDown_Msg)(COUNTDOWN_HALFSEC_MSG);
        }
    }

    if(countdownCnt == 32)
    {
		countdownCnt = 0;

        if(pCountDown_Msg != NULL)
        {
            (pCountDown_Msg)(COUNTDOWN_SEC_MSG);
        }
        
        if(countdownTime.sec == 0)
        {
           if(countdownTime.min == 0)
           {
              if(countdownTime.hour == 0)
              {
                  if(pCountDown_Msg != NULL)
                  {
                      (pCountDown_Msg)(COUNTDOWN_DONE_MSG); //count down finish
                  }
              }
              else
              {
                  countdownTime.hour--;
                  countdownTime.min = 59;
                  countdownTime.sec = 59;
              }
           }
           else
           {
              countdownTime.min--;
              countdownTime.sec = 59;
           }
        }
        else
        {
          countdownTime.sec --;
        }
    }
}

//**********************************************************************
// 函数功能: 倒计时复位，复位后为10小时倒计时
// 输入参数：无 
// 返回参数：无 
//**********************************************************************
void Mid_Countdown_Reset(void)
{
    countdownCnt 		   = 0;
	countdownTime.hour     = 0;
    countdownTime.min      = 0;
    countdownTime.sec      = 0;
    countdownTime.us       = 0;
    SMDrv_CTimer_Clear(COUNTDOWN_CTIMER_MODULE);
}

//**********************************************************************
// 函数功能: 倒计时初始化,硬件初始化，并注册中断回调函数
// 输入参数：无
// 返回参数：无
//**********************************************************************
void Mid_Countdown_Init(void)
{
    pCountDown_Msg = NULL; 

    //open a ctimer for RTC
    SMDrv_CTimer_Open(COUNTDOWN_CTIMER_MODULE,7,Mid_Countdown_Isr);
    Mid_Countdown_Reset();
}

//**********************************************************************
// 函数功能: 设置callback，倒计时消息，倒计时完成后，上层需要进行什么操作
// 输入参数：countdown_cb
// 返回参数：无   
//**********************************************************************
void Mid_Countdown_SetCallBack(mid_countdown_cb countdown_cb)
{
    if(countdown_cb == NULL)
        return;
    pCountDown_Msg = countdown_cb;
}

//**********************************************************************
// 函数功能: 倒计时开始
// 输入参数：无  
// 返回参数：无
//**********************************************************************
void Mid_Countdown_Start(void)
{
    SMDrv_CTimer_Start(COUNTDOWN_CTIMER_MODULE);
}

//**********************************************************************
// 函数功能: 倒计时停止
// 输入参数：无
// 返回参数：无
//**********************************************************************
void Mid_Countdown_Stop(void)
{
    SMDrv_CTimer_Stop(COUNTDOWN_CTIMER_MODULE);
}

//**********************************************************************
// 函数功能: 读取倒计时剩余时间
// 输入参数：timeTemp：传入变量，保存倒计时剩余时间
// 返回参数：无 
//**********************************************************************
void Mid_Countdown_TimeRead(countdown_time_s *timeTemp)
{
    timeTemp->hour  = countdownTime.hour;
    timeTemp->min   = countdownTime.min;
    timeTemp->sec   = countdownTime.sec;
    timeTemp->us    = countdownTime.us;
}

//**********************************************************************
// 函数功能: 设置/写入倒计时剩余时间
// 输入参数：timeTemp：传入倒计时剩余时间参数
//        
// 返回参数：无  
//**********************************************************************
void Mid_Countdown_TimeWrite(countdown_time_s *timeTemp)
{
   countdownTime.hour = timeTemp->hour;
   countdownTime.min  = timeTemp->min;
   countdownTime.sec  = timeTemp->sec;
   countdownTime.us   = timeTemp->us;
}

//**********************************************************************
// 函数功能: 循环设置倒计时小时部分的十位数加
// 输入参数：无
// 返回参数：无
//**********************************************************************
void Mid_Countdown_HighHourAdd(void)
{
    countdownTime.hour += 10;
    if(countdownTime.hour >= 30)
    {
        countdownTime.hour %= 10;
    }
}

//**********************************************************************
// 函数功能: 循环设置倒计时小时部分的十位数减
// 输入参数：无
// 返回参数：无
//**********************************************************************
void Mid_Countdown_HighHourLess(void)
{
    if(countdownTime.hour < 10)
    {
        countdownTime.hour += 20;
    }
    else
    {
        countdownTime.hour -= 10;
    }
}

//**********************************************************************
// 函数功能: 循环设置倒计时小时部分的个位数加
// 输入参数：无
// 返回参数：无 
//**********************************************************************
void Mid_Countdown_LowHourAdd(void)
{
    if(countdownTime.hour >= 23)
    {
        countdownTime.hour = 20;
    }
    else if(countdownTime.hour == 19)
    {
        countdownTime.hour = 10;
    }
    else if(countdownTime.hour == 9)
    {
        countdownTime.hour = 0;
    }
    else 
    {
        countdownTime.hour++;
    }
}

//**********************************************************************
// 函数功能: 循环设置倒计时小时部分的个位数减
// 输入参数：无
// 返回参数：无
//**********************************************************************
void Mid_Countdown_LowHourLess(void)
{
    if(countdownTime.hour == 0)
    {
        countdownTime.hour = 9;
    }
    else if(countdownTime.hour == 10)
    {
        countdownTime.hour = 19;
    }
    else if(countdownTime.hour == 20)
    {
        countdownTime.hour = 23;
    }
    else
    {
        countdownTime.hour--;
    }
}

//**********************************************************************
// 函数功能: 循环设置倒计时分钟部分的十位数加
// 输入参数：无
// 返回参数：无
//**********************************************************************
void Mid_Countdown_HighMinAdd(void)
{
    countdownTime.min += 10;
    if(countdownTime.min >= 60)
    {
        countdownTime.min %= 10;
    }
}

//**********************************************************************
// 函数功能: 循环设置倒计时分钟部分的十位数减
// 输入参数：无   
// 返回参数：无
//**********************************************************************
void Mid_Countdown_HighMinLess(void)
{
    if(countdownTime.min < 10)
    {
        countdownTime.min += 50;
    }
    else
    {
        countdownTime.min -= 10;
    }
}

//**********************************************************************
// 函数功能: 循环设置倒计时分钟部分的个位数加
// 输入参数：无   
// 返回参数：无
//**********************************************************************
void Mid_Countdown_LowMinAdd(void)
{
    uint8 temp;

    temp = countdownTime.min + 1;
    if(temp%10 == 0)
    {
        temp -= 10;
    }
    countdownTime.min = temp;
}

//**********************************************************************
// 函数功能: 循环设置倒计时分钟部分的个位数减
// 输入参数：无   
// 返回参数：无
//**********************************************************************
void Mid_Countdown_LowMinLess(void)
{
    uint8 temp;

    temp = countdownTime.min;
    if(temp%10 == 0)
    {
        temp += 9;
    }
    else
    {
        temp -= 1;
    }
    countdownTime.min = temp;
}

//**********************************************************************
// 函数功能: 循环设置倒计时秒钟部分的十位数加
// 输入参数：无   
// 返回参数：无
//**********************************************************************
void Mid_Countdown_HighSecAdd(void)
{
    countdownTime.sec += 10;
    if(countdownTime.sec >= 60)
    {
        countdownTime.sec %= 10;
    }
}

//**********************************************************************
// 函数功能: 循环设置倒计时秒钟部分的十位数减
// 输入参数：无   
// 返回参数：无
//**********************************************************************
void Mid_Countdown_HighSecLess(void)
{
    if(countdownTime.sec < 10)
    {
        countdownTime.sec += 50;
    }
    else
    {
        countdownTime.sec -= 10;
    }
}

//**********************************************************************
// 函数功能: 循环设置倒计时秒钟部分的个位数加
// 输入参数：无   
// 返回参数：无
//**********************************************************************
void Mid_Countdown_LowSecAdd(void)
{
    uint8 temp;

    temp = countdownTime.sec + 1;
    if(temp%10 == 0)
    {
        temp -= 10;
    }
    countdownTime.sec = temp;
}

//**********************************************************************
// 函数功能: 循环设置倒计时秒钟部分的个位数减
// 输入参数：无   
// 返回参数：无
//**********************************************************************
void Mid_Countdown_LowSecLess(void)
{
    uint8 temp;

    temp = countdownTime.sec;
    if(temp%10 == 0)
    {
        temp += 9;
    }
    else
    {
        temp -= 1;
    }
    countdownTime.sec = temp;
}

