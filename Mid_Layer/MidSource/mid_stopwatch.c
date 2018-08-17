/**********************************************************************
**
**模块说明: mid层stop watch接口
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.24  修改流程  ZSL  
**
**********************************************************************/
#include "sm_timer.h"
#include "mid_stopwatch.h"

Mid_StopWatch_Param_t	MidStopWatch;

//**********************************************************************
// 函数功能: 中断函数 
// 输入参数：无
// 返回参数：无
void Mid_StopWatch_Isr(void)
{
	MidStopWatch.TotalMs += STOPWATCH_PERIOD_MS;
}

//**********************************************************************
// 函数功能: 秒表复位函数
//       对秒表参数全部复位为0，对秒表Timer进行硬件复位
// 输入参数：无
// 返回参数：无
//**********************************************************************
void Mid_StopWatch_Reset(void)
{
	MidStopWatch.MeasureCnt = 0;
	MidStopWatch.TotalMs = 0;
	MidStopWatch.RuningFlg = false;
	
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
    SMDrv_CTimer_Open(STOPWATCH_CTIMER_MODULE,STOPWATCH_PERIOD_MS,Mid_StopWatch_Isr);
	
	Mid_StopWatch_Reset();
	
	MidStopWatch.InitedFlg = true;
};

//**********************************************************************
// 函数功能:  启动秒表函数
// 输入参数：无
// 返回参数：无
//**********************************************************************
void Mid_StopWatch_Start(void)
{
	MidStopWatch.RuningFlg = true;
	
    SMDrv_CTimer_Start(STOPWATCH_CTIMER_MODULE);
}

//**********************************************************************
// 函数功能:  暂停秒表函数
// 输入参数：无
// 返回参数：无
//**********************************************************************
void Mid_StopWatch_Stop(void)
{
	MidStopWatch.RuningFlg = false;
	
    SMDrv_CTimer_Stop(STOPWATCH_CTIMER_MODULE);
}

//**********************************************************************
// 函数功能: 设置秒表测量点，把当前秒表时间保存起来
//           把秒表时间读取出来，带补偿
// 输入参数：
// 返回参数：  0 成功，非0 失败
//**********************************************************************
uint32_t Mid_StopWatch_MeasurePoint(void)
{
	uint32 cntTemp;
	
	if(MidStopWatch.MeasureCnt >= STOPWATCH_MAX_STORE)
		return 0xFF;	
	
	// 获取当前tick值，并转为ms。时钟使用的是32768Hz
	cntTemp = (uint32_t)SMDrv_CTimer_ReadCount(STOPWATCH_CTIMER_MODULE) * 0.0305;	// (1000 / 32768)
	
	// 保存当前带补偿的时间值
	MidStopWatch.MeasurePoint[MidStopWatch.MeasureCnt] = MidStopWatch.TotalMs + cntTemp;
	MidStopWatch.MeasureCnt++;
	
	return 0x00;
}

//**********************************************************************
// 函数功能: 获取当前秒表计数值
// 输入参数：Methon 获取方法 0 不带补偿，1 带补偿
// 返回参数：    
uint32_t Mid_StopWatch_TotalMsGet(uint8_t Methon)
{	
	if(Methon)
	{
		uint32 cntTemp;
		
		// 获取当前tick值，并转为ms。时钟使用的是32768Hz
		cntTemp = (uint32_t)SMDrv_CTimer_ReadCount(STOPWATCH_CTIMER_MODULE) * 0.0305;	// (1000 / 32768)
		
		return (MidStopWatch.TotalMs + cntTemp);
	}
	else
	{
		return MidStopWatch.TotalMs;
	}
}

//**********************************************************************
// 函数功能: 获取秒表内部参数
// 输入参数：
// 返回参数：
void Mid_StopWatch_ParamGet(Mid_StopWatch_Param_t* Mid_StopWatch_Param)
{
	memcpy(Mid_StopWatch_Param, &MidStopWatch, sizeof(Mid_StopWatch_Param_t));
}

//**********************************************************************
// 函数功能: ms值转为 h:m:s:ms模式
// 输入参数：   Ms 需转换的Ms值
//				Mid_StopWatch_Format 保存转换结果
// 返回参数：
//**********************************************************************
void Mid_StopWatch_FormatSwitch(uint32_t Ms, Mid_StopWatch_Format_t* Mid_StopWatch_Format)
{
	uint32_t tMs = Ms;
	
	Mid_StopWatch_Format->hour = tMs / 3600000;
	
	tMs %= 3600000;
	Mid_StopWatch_Format->min = tMs / 60000;
	
	tMs %= 60000;
	Mid_StopWatch_Format->sec = tMs / 1000;	
	
	tMs %= 1000;
	Mid_StopWatch_Format->ms = tMs;	
}












