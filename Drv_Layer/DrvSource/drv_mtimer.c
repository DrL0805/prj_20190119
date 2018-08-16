/**********************************************************************
**
**模块说明: 多定时器驱动接口
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.19  修改流程  ZSL  
**
**********************************************************************/
#include "string.h"
#include "sm_timer.h"
#include "drv_mtimer.h"

static Drv_MTimer_Param_t	DrvMTiemr;

// 获取两个数的最大公约数
static uint32_t Get_GCD(uint32_t a, uint32_t b)
{
    uint32_t r;
    while (b != 0)
	{
        r = a % b; 
		a = b; 
		b = r;
    }
    return a;
}

void Drv_MTimer_IsrCb(void)
{
	DRV_MTIMER_RTT_LOG(0,"Drv_MTimer_IsrCb \n");
	
	for(uint32_t i = 0;i < DrvMTiemr.CreateCnt;i++)
	{
		if(eMTimerStateRunning == DrvMTiemr.MTimer[i].State)
		{
			if(++DrvMTiemr.MTimer[i].CurrTick >= DrvMTiemr.MTimer[i].AimTick)
			{
				if(NULL != DrvMTiemr.MTimer[i].IsrCb)
				{
					DrvMTiemr.MTimer[i].CurrTick = 0;
					DrvMTiemr.MTimer[i].IsrCb();
				}
			}	
		}
	}
}

//**********************************************************************
// 函数功能：   定时器初始化，所有参数清零
// 输入参数：   void
// 返回参数:
void Drv_MTimer_Init(void)
{
	memset(&DrvMTiemr, 0x00, sizeof(Drv_MTimer_Param_t));
}

//**********************************************************************
// 函数功能：   创建定时器，并返回定时器Id
// 输入参数：   id 返回的定期id
// 				Period 周期，此参数不可少，用于计算最大公约数
//				IsrCb 定时器回调函数
// 返回参数:	
uint32_t Drv_MTimer_Create(uint32_t *Id, uint32_t Period, void (*IsrCb)(void))
{
	if(DrvMTiemr.CreateCnt >= MULTITIMER_NUM_MAX)	return Ret_NoDevice;
	
	DrvMTiemr.MTimer[DrvMTiemr.CreateCnt].IsrCb = IsrCb;
	DrvMTiemr.MTimer[DrvMTiemr.CreateCnt].Period = Period;
	DrvMTiemr.MTimer[DrvMTiemr.CreateCnt].State = eMTimerStateInited;
	
	*Id = DrvMTiemr.CreateCnt;
	DrvMTiemr.CreateCnt++;
	
	return Ret_OK;
}

//**********************************************************************
// 函数功能：   开始定时器
// 输入参数：   id 定时器Id
//				Period 定时器运行周期
// 返回参数:
uint32_t Drv_MTimer_Start(uint32_t Id, uint32_t Period)
{
	uint32_t tGCDPeriod;		// 最大公约数
	uint32_t tOldPeriod;		// 保存原来运行周期	
	
	// Id 有效性校验
	if(Id >= DrvMTiemr.CreateCnt)	return Ret_NoDevice;
	
	// 定时器周期不为0且是MULTITIMER_MIN_PERIOD的倍数
    if(Period == 0)	return Ret_InvalidParam;
	if(Period % MULTITIMER_MIN_PERIOD)	return Ret_InvalidParam;	
	
	// 因定时器周期允许改变，所以定时器开始前运行前必须保证不处于运行状态
	if(eMTimerStateRunning == DrvMTiemr.MTimer[Id].State)	return Ret_DeviceBusy;
	if(eMTimerStateUninited == DrvMTiemr.MTimer[Id].State)	return Ret_NoInit;
	
    // 停止硬件计数，防止在更改过程中进入中断
    SMDrv_CTimer_Stop(MULTI_CTIMER_MODULE);	
	
	DrvMTiemr.MTimer[Id].Period = Period;
	DrvMTiemr.MTimer[Id].State = eMTimerStateRunning;
	
	/* 获取已运行定时器周期的最大公约数，作为硬件定时器最小tick */
	tGCDPeriod = DrvMTiemr.MTimer[Id].Period;
	for(uint32_t i = 0;i < DrvMTiemr.CreateCnt;i++)
	{
		if(eMTimerStateRunning ==  DrvMTiemr.MTimer[i].State)
			tGCDPeriod = Get_GCD(tGCDPeriod, DrvMTiemr.MTimer[i].Period);
	}
	
	// 如果有定时器正在运行，保存原有周期，否则此值为当前周期
	if(DrvMTiemr.RunningCnt)
		tOldPeriod = DrvMTiemr.MinPeriod;
	else
		tOldPeriod = tGCDPeriod;
	
	DrvMTiemr.MinPeriod = tGCDPeriod;

	/* 重新计算tick数，并开始运行定时器 */
	for(uint32_t i = 0;i < DrvMTiemr.CreateCnt;i++)		
	{
		if(eMTimerStateRunning ==  DrvMTiemr.MTimer[i].State)
		{
			DrvMTiemr.MTimer[i].AimTick = DrvMTiemr.MTimer[i].Period / DrvMTiemr.MinPeriod;
			DrvMTiemr.MTimer[i].CurrTick = (DrvMTiemr.MTimer[i].CurrTick * tOldPeriod) / DrvMTiemr.MTimer[i].Period;			
		}		
	}
	
	SMDrv_CTimer_Open(MULTI_CTIMER_MODULE,DrvMTiemr.MinPeriod,Drv_MTimer_IsrCb);
	SMDrv_CTimer_Start(MULTI_CTIMER_MODULE);
	
	DrvMTiemr.RunningCnt++;
	
	DRV_MTIMER_RTT_LOG(0,"Drv_MTimer_Start MinPeriod %d, CreateCnt %d, RunningCnt %d \n",
		DrvMTiemr.MinPeriod, DrvMTiemr.CreateCnt, DrvMTiemr.RunningCnt);
}

//**********************************************************************
// 函数功能：   停止定时器
// 输入参数：   id 定时器Id
// 返回参数:
// 注意事项：尽量别在Drv_MTimer_IsrCb中断中调用Drv_MTimer_Stop，可能导致剩余定时器有1次中断不准确
uint32_t Drv_MTimer_Stop(uint32_t Id)
{	
	uint32_t tGCDPeriod;		// 最大公约数	
	uint32_t tOldPeriod;		// 保存原来运行周期	
	bool	tFirstFlg = true;	
	
	if(eMTimerStateRunning != DrvMTiemr.MTimer[Id].State)	return Ret_NoDevice;
	
	DrvMTiemr.MTimer[Id].State = eMTimerStateInited;
	DrvMTiemr.MTimer[Id].AimTick = 0;
	DrvMTiemr.MTimer[Id].CurrTick = 0;
	DrvMTiemr.RunningCnt--;
	
	SMDrv_CTimer_Stop(MULTI_CTIMER_MODULE);
	
	// 如果还有定时器，重新计算最小周期
	if(DrvMTiemr.RunningCnt)
	{
		for(uint32_t i = 0;i < DrvMTiemr.CreateCnt;i++)
		{
			if(eMTimerStateRunning ==  DrvMTiemr.MTimer[i].State)
			{
				if(tFirstFlg)
				{
					tFirstFlg = false;
					tGCDPeriod = DrvMTiemr.MTimer[i].Period;
				}
				else
				{
					tGCDPeriod = Get_GCD(tGCDPeriod, DrvMTiemr.MTimer[i].Period);
				}
			}				
		}

		// 如果最小周期有改变，需重新计算tick值
		if(tGCDPeriod != DrvMTiemr.MinPeriod)
		{
			tOldPeriod = DrvMTiemr.MinPeriod;
			DrvMTiemr.MinPeriod = tGCDPeriod;
			
			/* 重新计算tick数，并开始运行定时器 */
			for(uint32_t i = 0;i < DrvMTiemr.CreateCnt;i++)		
			{
				if(eMTimerStateRunning ==  DrvMTiemr.MTimer[i].State)
				{
					DrvMTiemr.MTimer[i].AimTick = DrvMTiemr.MTimer[i].Period / DrvMTiemr.MinPeriod;
					DrvMTiemr.MTimer[i].CurrTick = (DrvMTiemr.MTimer[i].CurrTick * tOldPeriod) / DrvMTiemr.MTimer[i].Period;					
				}	
			}
			
			SMDrv_CTimer_Open(MULTI_CTIMER_MODULE,DrvMTiemr.MinPeriod,Drv_MTimer_IsrCb);
			SMDrv_CTimer_Start(MULTI_CTIMER_MODULE);			
		}
		else
		{
			SMDrv_CTimer_Start(MULTI_CTIMER_MODULE);
		}
	}
	
	DRV_MTIMER_RTT_LOG(0,"Drv_MTimer_Stop MinPeriod %d, CreateCnt %d, RunningCnt %d \n",
		DrvMTiemr.MinPeriod, DrvMTiemr.CreateCnt, DrvMTiemr.RunningCnt);	
}
 
/****************** 以下为定时器测试代码 ******************************/ 
#if DRV_MTIMER_RTT_DEBUG
static uint32_t TimerId0, TimerId1;
static void TimerId0IsrCb(void)
{	
	static uint32_t tCnt = 0;
	
	if(5 == ++tCnt)
		Drv_MTimer_Stop(TimerId0);	
	
	DRV_MTIMER_RTT_LOG(0,"TimerId0IsrCb \n");	
}

static void TimerId1IsrCb(void)
{
	static uint32_t tCnt = 0;
	
	if(2 == ++tCnt)
		Drv_MTimer_Stop(TimerId1);
	
	DRV_MTIMER_RTT_LOG(0,"TimerId1IsrCb \n");
}


void Drv_MTimer_Test(void)
{
	Drv_MTimer_Init();
	Drv_MTimer_Create(&TimerId0, 1000, TimerId0IsrCb);
	Drv_MTimer_Create(&TimerId1, 1000, TimerId1IsrCb);
	
	Drv_MTimer_Start(TimerId0, 5);
	Drv_MTimer_Start(TimerId1, 200);
}
#endif


