/**********************************************************************
**
**模块说明: mid层stop watch接口
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.24  修改流程  ZSL  
**
**********************************************************************/
#include "sm_timer.h"
#include "mid_stopwatch.h"

static stopwatch_time_s     stopwatchTime;
static stopwatch_time_s     stopwatchLapTemp1, stopwatchLapTemp2;
static stopwatch_time_s     stopwatchLapTime[5];
static stopwatch_time_s     stopwatchLapAvg;
static uint16 stopwatchCnt;         // stopwatchCnt++ per 31250 us
static uint8  stopwatchLapCnt = 0;  // range : 0-5
static uint8  maxLapPos, minLapPos; // range : 0-4

static mid_stopwatch_cb pStopWatch_Msg = NULL; 

//**********************************************************************
// 函数功能: 中断函数。31.25ms进入该函数一次
//       此函数对秒表工作时长进行计数
// 输入参数：无
// 返回参数：无
//**********************************************************************
void Mid_StopWatch_Isr(void)
{
	stopwatchCnt++;
	stopwatchTime.us += 31250;
	if(stopwatchTime.hour == 0)
	{
		if(pStopWatch_Msg != NULL)
        {
            (pStopWatch_Msg)(STOPWATCH_32Hz_MSG);  //31.25ms计时完成
        }
	}
	if(stopwatchCnt == 16)
	{
		if(pStopWatch_Msg != NULL)
        {
            (pStopWatch_Msg)(STOPWATCH_HALFSEC_MSG); //计时0.5s计时完成
        }
	}

	if(stopwatchCnt == 32)
	{
		if(pStopWatch_Msg != NULL)
        {
            (pStopWatch_Msg)(STOPWATCH_SEC_MSG);  //计时一秒计时完成
        }
		stopwatchTime.us	= 0;
		stopwatchCnt        = 0;
		stopwatchTime.sec++;
		if(stopwatchTime.sec == 60)
		{
			stopwatchTime.sec = 0;
			stopwatchTime.min++;
			if(stopwatchTime.min == 60)
			{
				stopwatchTime.min = 0;
				stopwatchTime.hour++;
				if(stopwatchTime.hour > 99)
				{
					stopwatchTime.hour = 0;
				}
			}
		}
	}
}

//**********************************************************************
// 函数功能: 秒表复位函数
//       对秒表参数全部复位为0，对秒表Timer进行硬件服务
// 输入参数：无
// 返回参数：无
//**********************************************************************
void Mid_StopWatch_Reset(void)
{
	stopwatchCnt		   = 0;
	stopwatchLapCnt		   = 0;
	maxLapPos              = 0;
	minLapPos              = 0;
	stopwatchTime.hour	   = 0;
	stopwatchTime.min	   = 0;
	stopwatchTime.sec	   = 0;
	stopwatchTime.us	   = 0;
	stopwatchLapTemp1.hour = 0;
	stopwatchLapTemp1.min  = 0;
	stopwatchLapTemp1.sec  = 0;
	stopwatchLapTemp1.us   = 0;
    SMDrv_CTimer_Clear(STOPWATCH_CTIMER_MODULE);
}

//**********************************************************************
// 函数功能: 秒表功能初始化函数。
//       对秒表Timer进行硬件进行初始化，并注册中断回调函数，对秒表参数复位
// 输入参数：无
// 返回参数：无
//**********************************************************************
void Mid_StopWatch_Init(void)
{
    SMDrv_CTimer_Open(STOPWATCH_CTIMER_MODULE,7,Mid_StopWatch_Isr);
	Mid_StopWatch_Reset();
};

//**********************************************************************
// 函数功能: 设置callback，倒计时消息，倒计时完成后，上层需要进行什么操作
// 输入参数：countdown_cb
// 返回参数：无   
//**********************************************************************
void Mid_StopWatch_SetCallBack(mid_stopwatch_cb stopwatch_cb)
{
    if(stopwatch_cb == NULL)
        return;
    pStopWatch_Msg = stopwatch_cb;
}

//**********************************************************************
// 函数功能:  启动秒表函数
// 输入参数：无
// 返回参数：无
//**********************************************************************
void Mid_StopWatch_Start(void)
{
    SMDrv_CTimer_Start(STOPWATCH_CTIMER_MODULE);
}

//**********************************************************************
// 函数功能:  暂停秒表函数
// 输入参数：无
// 返回参数：无
//**********************************************************************
void Mid_StopWatch_Stop(void)
{
    SMDrv_CTimer_Stop(STOPWATCH_CTIMER_MODULE);
}

//**********************************************************************
// 函数功能: 读取秒表时间函数，带us补偿
//           把秒表时间读取出来，带us补偿
// 输入参数：timeTemp：传入变量，保存秒表的时间
// 返回参数：    
//**********************************************************************
void Mid_StopWatch_CountRead(stopwatch_time_s *timeTemp)
{
	uint32 cntTemp;
	cntTemp			= SMDrv_CTimer_ReadCount(STOPWATCH_CTIMER_MODULE) * 3906;
	timeTemp->hour	= stopwatchTime.hour;
	timeTemp->min	= stopwatchTime.min;
	timeTemp->sec	= stopwatchTime.sec;
	timeTemp->us	= stopwatchTime.us + cntTemp;
}

//**********************************************************************
// 函数功能: 读取秒表时间，不带us补偿
//       把秒表时间读取出来
// 输入参数：timeTemp：传入变量，保存秒表的时间   
// 返回参数：无
//**********************************************************************
void Mid_StopWatch_TimeRead(stopwatch_time_s *timeTemp)
{
	timeTemp->hour	= stopwatchTime.hour;
	timeTemp->min	= stopwatchTime.min;
	timeTemp->sec	= stopwatchTime.sec;
	timeTemp->us	= stopwatchTime.us;
}

//**********************************************************************
// 函数功能: 计算（timeTemp2-timeTemp1）的时差，精确到us
// 输入参数：   
//        timeTemp1:单圈时间1
//        timeTemp2:单圈时间2
// 返回参数：difftime：（timeTemp2-timeTemp1）的时差
//**********************************************************************
stopwatch_time_s Mid_StopWatch_DiffLapTime(stopwatch_time_s *timeTemp1, stopwatch_time_s *timeTemp2)
{
	stopwatch_time_s difftime;
	stopwatch_time_s timeTemp;
	uint32 secTemp1, secTemp2;

	timeTemp.hour = timeTemp2->hour;
	timeTemp.min  = timeTemp2->min;
	timeTemp.sec  = timeTemp2->sec;
	timeTemp.us   = timeTemp2->us;
	
	if(timeTemp.us >= timeTemp1->us)
	{
		difftime.us = timeTemp.us - timeTemp1->us;
	}
	else
	{
		difftime.us = 1000000 + timeTemp.us - timeTemp1->us;
		if(timeTemp.sec)  
		{
			timeTemp.sec--;
		}
		else
		{
			timeTemp.sec = 59;
			if(timeTemp.min)
			{
				timeTemp.min--;
			}
			else
			{
				timeTemp.min = 59;
				timeTemp.hour--;
			}
		}
	}

	secTemp1 = timeTemp1->hour*3600 + timeTemp1->min*60 + timeTemp1->sec;
	secTemp2 = timeTemp.hour*3600 + timeTemp.min*60 + timeTemp.sec;

	secTemp2 -= secTemp1;
	
	difftime.hour = secTemp2 / 3600;
	difftime.min  = secTemp2 % 3600 / 60;
	difftime.sec  = secTemp2 % 60;

	return difftime;
}

//**********************************************************************
// 函数功能: 计算秒表单圈 时间的平均值
//           calculate average value of all lap time
// 输入参数：*lapAvg：传入变量指针，保存单圈时间的平均值
// 返回参数：无 
//**********************************************************************
void Mid_StopWatch_LapGetAvg(stopwatch_time_s *lapAvg)
{
	uint8  i, lapCnt;
	uint32 us_Sum, ms_Sum;
	us_Sum = 0;
	ms_Sum = 0;
	
	lapCnt = stopwatchLapCnt;
	for(i = 0; i < lapCnt; i++)
	{
		us_Sum += stopwatchLapTime[i].us;
		ms_Sum += stopwatchLapTime[i].hour * 3600000 + stopwatchLapTime[i].min * 60000 + stopwatchLapTime[i].sec * 1000;
	}
	us_Sum /= lapCnt;			// get avg of us
	us_Sum /= 1000;				// us->ms
	ms_Sum /= lapCnt;			// get avg of ms
	ms_Sum += us_Sum;
	lapAvg->hour = ms_Sum / 3600000;
	lapAvg->min  = ms_Sum % 3600000 / 60000;
	lapAvg->sec  = ms_Sum % 60000 / 1000;
	lapAvg->us   = ms_Sum % 1000 * 1000;
}

//**********************************************************************
// 函数功能: 单圈时间数组里面，算出单圈时间数组里的最值位置
//           find the pos of max and min lap
// 输入参数：无
// 返回参数：无
//**********************************************************************
void Mid_StopWatch_LapGetMaxMin(void)
{
	uint8 i;
	i = stopwatchLapCnt - 1;
	
	if(stopwatchLapTime[i].hour > stopwatchLapTime[maxLapPos].hour)
	{
		maxLapPos = i;
	}
	else if(stopwatchLapTime[i].hour < stopwatchLapTime[maxLapPos].hour)
	{
		if(stopwatchLapTime[i].hour < stopwatchLapTime[minLapPos].hour)
		{
			minLapPos = i;
		}
		else if(stopwatchLapTime[i].hour == stopwatchLapTime[minLapPos].hour)
		{
			if(stopwatchLapTime[i].min < stopwatchLapTime[minLapPos].min)
			{
				minLapPos = i;
			}
			else if(stopwatchLapTime[i].min == stopwatchLapTime[minLapPos].min)
			{
				if(stopwatchLapTime[i].sec < stopwatchLapTime[minLapPos].sec)
				{
					minLapPos = i;
				}
				else if(stopwatchLapTime[i].sec == stopwatchLapTime[minLapPos].sec)
				{
					if(stopwatchLapTime[i].us < stopwatchLapTime[minLapPos].us)
					{
						minLapPos = i;
					}
				}
			}
		}
	}	
	else
	{
		if(stopwatchLapTime[i].min > stopwatchLapTime[maxLapPos].min)
		{
			maxLapPos = i;
		}
		else if(stopwatchLapTime[i].min < stopwatchLapTime[maxLapPos].min)
		{
			if(stopwatchLapTime[i].min < stopwatchLapTime[minLapPos].min)
			{
				minLapPos = i;
			}
			else if(stopwatchLapTime[i].min == stopwatchLapTime[minLapPos].min)
			{
				if(stopwatchLapTime[i].sec < stopwatchLapTime[minLapPos].sec)
				{
					minLapPos = i;	
				}
				else if(stopwatchLapTime[i].sec == stopwatchLapTime[minLapPos].sec)
				{
					if(stopwatchLapTime[i].us < stopwatchLapTime[minLapPos].us)
					{
						minLapPos = i;
					}
				}
			}
		}
		else
		{
			if(stopwatchLapTime[i].sec > stopwatchLapTime[maxLapPos].sec)
			{
				maxLapPos = i;
			}
			else if(stopwatchLapTime[i].sec < stopwatchLapTime[maxLapPos].sec)
			{
				if(stopwatchLapTime[i].sec < stopwatchLapTime[minLapPos].sec)
				{
					minLapPos = i;
				}
				else if(stopwatchLapTime[i].sec == stopwatchLapTime[minLapPos].sec)
				{
					if(stopwatchLapTime[i].us < stopwatchLapTime[minLapPos].us)
					{
						minLapPos = i;
					}
				}
			}
			else 
			{
				if(stopwatchLapTime[i].us > stopwatchLapTime[maxLapPos].us)
				{
					maxLapPos = i;
				}
				else if(stopwatchLapTime[i].us < stopwatchLapTime[maxLapPos].us)
				{
					if(stopwatchLapTime[i].us < stopwatchLapTime[minLapPos].us)
					{
						minLapPos = i;
					}
				}
			}
		}
	}
}

//**********************************************************************
// 函数功能:  将此次单圈时间存入单圈时间结构体数组，并算出单圈时间的最值
//  set a new lap, calculate average value, find the pos of max and min lap
// 输入参数：无
// 返回参数：无
//**********************************************************************
void Mid_StopWatch_SetLap(void)
{
	uint8 i;
	if(stopwatchLapCnt < 5)
	{
		Mid_StopWatch_CountRead(&stopwatchLapTemp2);
		stopwatchLapTime[stopwatchLapCnt] = Mid_StopWatch_DiffLapTime(&stopwatchLapTemp1, &stopwatchLapTemp2);
		stopwatchLapTemp1 = stopwatchLapTemp2;
		stopwatchLapCnt++;
	}
	else
	{
		Mid_StopWatch_CountRead(&stopwatchLapTemp2);
		for(i = 0; i < 4; i++)
		{
			stopwatchLapTime[i] = stopwatchLapTime[i+1];
		}

		if(maxLapPos)
			maxLapPos--;
		if(minLapPos)
			minLapPos--;
		
		stopwatchLapTime[4] = Mid_StopWatch_DiffLapTime(&stopwatchLapTemp1, &stopwatchLapTemp2);
		stopwatchLapTemp1 = stopwatchLapTemp2;
	}
	
	if(stopwatchLapCnt > 1)
	{
		Mid_StopWatch_LapGetAvg(&stopwatchLapAvg);
		Mid_StopWatch_LapGetMaxMin();
	}
	else
	{
		maxLapPos = 0;
		minLapPos = 0;
		stopwatchLapAvg = stopwatchLapTime[0];
	}
}

//**********************************************************************
// 函数功能:  读取最大单圈时间的位置
// 输入参数： 无
// 返回参数： maxLapPos：最大单圈时间的位置
//**********************************************************************
uint8 Mid_StopWatch_ReadLapMax(void)
{
	return maxLapPos;
}

//**********************************************************************
// 函数功能:  读取最小单圈时间的位置
// 输入参数： 无   
// 返回参数：minLapPos ：最小单圈时间的位置
//**********************************************************************
uint8 Mid_StopWatch_ReadLapMin(void)
{
	return minLapPos;
}

//**********************************************************************
// 函数功能:  读取平均单圈时间函数
// 输入参数： 无 
// 返回参数：stopwatchLapAvg：平均单圈时间的结构体变量。
//**********************************************************************
stopwatch_time_s *Mid_StopWatch_ReadLapAvg(void)
{
	return &stopwatchLapAvg;
}

//**********************************************************************
// 函数功能: 读取所取单圈时间次数函数
// 输入参数：无
// 返回参数：stopwatchLapCnt：单圈计数
//**********************************************************************
uint8 Mid_StopWatch_ReadLapCnt(void)
{
	return stopwatchLapCnt;
}

//**********************************************************************
// 函数功能:  读取单圈时间函数
// 输入参数： 无
// 返回参数：stopwatchLapTime：单圈时间
//**********************************************************************
stopwatch_time_s *Mid_StopWatch_ReadLapTime(void)
{
	return stopwatchLapTime;
}

