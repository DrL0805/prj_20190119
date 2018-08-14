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

typedef struct
{
    uint16	  freq;       // 设置的频率
    uint16	  cnt;        // 计数值
    uint16	  aimCnt;     // 目标计数值
    comm_cb   *IsrCb;      //回调函数
}multitimer_para_s;

//用于记录最大的频率值。
static uint16 maxFreqHz;

static multitimer_para_s multiTimerPara[MULTITIMER_NUM_MAX];

//定时器中断函数,放在am_ctimer_isr的B0中断里使用。
void mutil_timer_isr(void)
{
    uint16 paraCnt;
    for(paraCnt = 0;paraCnt < MULTITIMER_NUM_MAX; paraCnt++)//每个有效的MultiTimer的Cnt都增加1
    {
        if(multiTimerPara[paraCnt].freq != 0)
        {
            multiTimerPara[paraCnt].cnt++;
            if(multiTimerPara[paraCnt].cnt >= multiTimerPara[paraCnt].aimCnt)
            {
                if(multiTimerPara[paraCnt].IsrCb != NULL)
                {
                    multiTimerPara[paraCnt].IsrCb();
                }
                multiTimerPara[paraCnt].cnt = 0;
            }
        }
    }
}

//**********************************************************************
// 函数功能：   配置硬件定时器频率。
// 输入参数：   freq:	设置硬件定时器频率,最高频率不超过1000
// 返回参数：	0x00:操作成功
// 				0x2:操作失败
//**********************************************************************
static uint8 MultiTimer_Open(uint16 freq)
{
    if(freq > 1000)
    {   
        return Ret_InvalidParam;
    }
 
    return SMDrv_CTimer_Open(MULTI_CTIMER_MODULE,freq,mutil_timer_isr);
}

//**********************************************************************
// 函数功能：   定时器初始化，并返回操作结果。
// 输入参数：   void
// 返回参数:
//**********************************************************************
void Drv_MultiTimer_Init(void)
{
    memset(multiTimerPara,0x00,sizeof(multiTimerPara));
    maxFreqHz = 0;
}

//**********************************************************************
// 函数功能：	设置一个timer，并返回操作结果。
// 输入参数：	id:		返回定时器ID
// 				freq:	定时器设置频率
// 				IsrCb:	回调函数
// 返回参数：	0x00: 设置成功
// 				0x01: 设置失败
//              0x02: 参数非法
//**********************************************************************
uint8 Drv_MultiTimer_Set(uint16 *id, uint16 freq, void (*IsrCb)(void))
{
    uint16 i;
    uint16	idTemp = 0, maxFreqHzTemp = 0;

    if((freq == 0) || (freq > 1000)) //如果频率为0Hz或大于1000HZ，则设置失败。
    {
        return Ret_InvalidParam;
    }

    for(i = 0; i < MULTITIMER_NUM_MAX; i++)
    {
        if(multiTimerPara[i].freq == 0)
        {
            idTemp	= i;
            *id	= idTemp;
            break;
        }
    }

    // 所有定时器均已分配完了
    if(i == MULTITIMER_NUM_MAX)
        return Ret_Fail;

    // 停止硬件计数，防止在更改过程中进入中断
    SMDrv_CTimer_Stop(MULTI_CTIMER_MODULE);
    
    multiTimerPara[idTemp].freq  = freq;
    multiTimerPara[idTemp].IsrCb = IsrCb;
    for(i = 0; i < MULTITIMER_NUM_MAX; i++)
    {
        if(multiTimerPara[i].freq == 0)
            continue;
        switch(multiTimerPara[i].freq)
        {
        case FREQ_1HZ:
        case FREQ_2HZ:
        case FREQ_25HZ:
        case FREQ_50HZ:
        case FREQ_100HZ:
        case FREQ_200HZ:
            if(maxFreqHzTemp < multiTimerPara[i].freq)
                maxFreqHzTemp = multiTimerPara[i].freq;
            break;
        default:
            maxFreqHzTemp = 1000;
            break;
        }
    }

    if(maxFreqHzTemp > maxFreqHz)
    {
        // 硬件定时器频率改变，计数需重新调整
        maxFreqHz = maxFreqHzTemp;
        for(i = 0; i < MULTITIMER_NUM_MAX; i++)
        {
            if(multiTimerPara[i].freq != 0)
            {
                multiTimerPara[i].cnt    = 0;
                multiTimerPara[i].aimCnt = maxFreqHz / multiTimerPara[i].freq;
            }
        }
        MultiTimer_Open(maxFreqHz);
        SMDrv_CTimer_Start(MULTI_CTIMER_MODULE);
    }
    else
    {
        // 最大频率未变更，直接重新开启硬件定时器
        SMDrv_CTimer_Start(MULTI_CTIMER_MODULE);
    }
    return Ret_OK;
}

//**********************************************************************
// 函数功能：   删除一个timer，并返回操作结果。
// 输入参数：   id:		定时器ID
// 返回参数：	0x00:	设置成功
// 				0x02:   参数非法
//**********************************************************************
uint8 Drv_MultiTimer_Delete(uint16 id)
{
    uint16 i;
    uint16	maxFreqHzTemp = 0;

    if(id >= MULTITIMER_NUM_MAX)
        return Ret_InvalidParam;

    // 停止硬件计数，防止在更改过程中进入中断
    SMDrv_CTimer_Stop(MULTI_CTIMER_MODULE);

    multiTimerPara[id].freq	  = 0;
    multiTimerPara[id].cnt    = 0;
    multiTimerPara[id].aimCnt = 0;
    multiTimerPara[id].IsrCb  = NULL;
    for(i = 0; i < MULTITIMER_NUM_MAX; i++)
    {
        if(multiTimerPara[i].freq == 0)
            continue;
        switch(multiTimerPara[i].freq)
        {
        case FREQ_1HZ:
        case FREQ_2HZ:
        case FREQ_25HZ:
        case FREQ_50HZ:
        case FREQ_100HZ:
        case FREQ_200HZ:
            if(maxFreqHzTemp < multiTimerPara[i].freq)
                maxFreqHzTemp = multiTimerPara[i].freq;
            break;
        default:
            maxFreqHzTemp = 1000;
            break;
        }
    }

    // 硬件定时器频率改变，计数需重新调整
    if(maxFreqHzTemp != maxFreqHz)
    {
        // 所有定时器均已关闭
        maxFreqHz = maxFreqHzTemp;
        if(maxFreqHzTemp == 0)
            return Ret_OK;

        for(i = 0; i < MULTITIMER_NUM_MAX; i++)
        {
            if(multiTimerPara[i].freq != 0)
            {
                multiTimerPara[i].cnt    = 0;
                multiTimerPara[i].aimCnt = maxFreqHz / multiTimerPara[i].freq;
            }
        }

        MultiTimer_Open(maxFreqHz);   
        SMDrv_CTimer_Start(MULTI_CTIMER_MODULE);
    }
    else
    {
        // 所有定时器均已关闭
        if(maxFreqHzTemp == 0)
            return Ret_OK;
        else
            SMDrv_CTimer_Start(MULTI_CTIMER_MODULE);
    }

    return Ret_OK;
}

