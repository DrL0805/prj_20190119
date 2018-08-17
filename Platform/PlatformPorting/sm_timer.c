/**********************************************************************
**
**模块说明: 对接MCU ctimer驱动接口
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.18  初版  ZSL  
**
**********************************************************************/
#include "am_mcu_apollo.h"
#include "string.h"
#include "sm_timer.h"

#define CTIMER_MAX_NUM   8

//定义ctimer配置参数
typedef struct
{
    uint32 u32tm_id;   //timer number
    uint32 u32tm_seg;  //timer segment
    uint32 u32tm_irq;  //timer中断类型
    uint32 u32tm_conf; //配置Timer参数
    uint32 u32tm_delay;//delay值
}ctimer_info_s;

static comm_cb *ctimer_irq_cb[CTIMER_MAX_NUM];

//**********************************************************************
// 函数功能:    定时器中断服务函数
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
void am_ctimer_isr(void)
{
    uint32 timer_int_status;

    timer_int_status = am_hal_ctimer_int_status_get(true);
    am_hal_ctimer_int_clear(timer_int_status);

    if(timer_int_status & AM_HAL_CTIMER_INT_TIMERA0)    //（0,A）小表盘
    {
        if(ctimer_irq_cb[0] != NULL)
        {      
            (ctimer_irq_cb[0])();
        }
    }
    if(timer_int_status & AM_HAL_CTIMER_INT_TIMERB0)     //（0,B）多功能定时器
    {
        if(ctimer_irq_cb[1] != NULL)
        {      
            (ctimer_irq_cb[1])();
        }
    }
    if(timer_int_status & AM_HAL_CTIMER_INT_TIMERA1)     //（1,A）系统TICK时钟
    {
        if(ctimer_irq_cb[2] != NULL)
        {      
            (ctimer_irq_cb[2])();
        }
    }
    if(timer_int_status & AM_HAL_CTIMER_INT_TIMERB1)     //（1,B）按键
    {
        if(ctimer_irq_cb[3] != NULL)
        {      
            (ctimer_irq_cb[3])();
        }
    }
    if(timer_int_status & AM_HAL_CTIMER_INT_TIMERA2)     //（2,A）stopwatch
    {
        if(ctimer_irq_cb[4] != NULL)
        {      
            (ctimer_irq_cb[4])();
        }
    }
    if(timer_int_status & AM_HAL_CTIMER_INT_TIMERB2)     //（2,B）countdown
    {
        if(ctimer_irq_cb[5] != NULL)
        {      
            (ctimer_irq_cb[5])();
        }
    }
    if(timer_int_status & AM_HAL_CTIMER_INT_TIMERA3)     //（3,A）RTC
    {
        if(ctimer_irq_cb[6] != NULL)
        {      
            (ctimer_irq_cb[6])();
        }
    }
    if(timer_int_status & AM_HAL_CTIMER_INT_TIMERB3)     //（3,B）monitor
    {
        if(ctimer_irq_cb[7] != NULL)
        {
            (ctimer_irq_cb[7])();
        }
    }
}

//**********************************************************************
// 函数功能: 将上层ctimer module转换为具体Timer配置参数。
//           此接口中的具体配置项，会根据具体项目做具体的配置和调整。
// 输入参数：
//    modul: driver module ID,值参考ctimer_module
//    u16freq: timer频率，在open Timer的时候会有使用模块传入，其他接口传入为0
//    timer_callback:上层注册的中断回调函数
// 返回参数：
//    ptm_info:ctimer定时器配置参数
//    FALSE:传入modul参数不正确
//    TRUE:正常
//**********************************************************************
static uint8 ctimer_modul2time(ctimer_module modul,uint16 u16freq,comm_cb *timer_callback,ctimer_info_s *ptm_info)
{
    uint8 u8cb_id;

    if(ptm_info == NULL)
        return FALSE;
    //防止u16freq传入0值，在计算时死机
    if(u16freq == 0)
        u16freq = 1;
    
    if(modul == MOVT_CTIMER_MODULE)         //timer:(0,A)
    {
        ptm_info->u32tm_id=0;
        ptm_info->u32tm_seg=AM_HAL_CTIMER_TIMERA;
        ptm_info->u32tm_irq=AM_HAL_CTIMER_INT_TIMERA0;
        ptm_info->u32tm_conf=AM_HAL_CTIMER_FN_REPEAT | AM_HAL_CTIMER_INT_ENABLE | AM_HAL_CTIMER_XT_32_768KHZ;
        ptm_info->u32tm_delay=1;
        u8cb_id = 0;
    }
    else if(modul == MULTI_CTIMER_MODULE)    //timer:(0,B)
    {
		#if 0	// H001
        ptm_info->u32tm_id=0;
        ptm_info->u32tm_seg=AM_HAL_CTIMER_TIMERB;
        ptm_info->u32tm_irq=AM_HAL_CTIMER_INT_TIMERB0;
        ptm_info->u32tm_conf=AM_HAL_CTIMER_FN_REPEAT | AM_HAL_CTIMER_INT_ENABLE | AM_HAL_CTIMER_XT_16_384KHZ;
        ptm_info->u32tm_delay=(uint32)(16384 / u16freq - 1);
        u8cb_id = 1;
		#endif 
		
		#if 1 // H003
        ptm_info->u32tm_id=0;
        ptm_info->u32tm_seg=AM_HAL_CTIMER_TIMERB;
        ptm_info->u32tm_irq=AM_HAL_CTIMER_INT_TIMERB0;
        ptm_info->u32tm_conf=AM_HAL_CTIMER_FN_REPEAT | AM_HAL_CTIMER_INT_ENABLE | AM_HAL_CTIMER_XT_32_768KHZ;
        ptm_info->u32tm_delay = (uint32)((32768 * u16freq / 1000) -1);	// 转为ms 
        u8cb_id = 1;
		#endif
    }
    else if(modul == SYSTICK_CTIMER_MODULE)  //timer:(1,A)
    {
        ptm_info->u32tm_id=1;
        ptm_info->u32tm_seg=AM_HAL_CTIMER_TIMERA;
        ptm_info->u32tm_irq=AM_HAL_CTIMER_INT_TIMERA1;
        ptm_info->u32tm_conf=AM_HAL_CTIMER_FN_REPEAT | AM_HAL_CTIMER_INT_ENABLE | AM_HAL_CTIMER_XT_32_768KHZ;
        ptm_info->u32tm_delay= u16freq;  //1023; //u16freq     100Hz
        u8cb_id = 2;
    }
    else if(modul == KEY_CTIMER_MODULE)      //timer:(1,B)
    {    
        ptm_info->u32tm_id=1;
        ptm_info->u32tm_seg=AM_HAL_CTIMER_TIMERB;
        ptm_info->u32tm_irq=AM_HAL_CTIMER_INT_TIMERB1;
        ptm_info->u32tm_conf=AM_HAL_CTIMER_FN_REPEAT | AM_HAL_CTIMER_INT_ENABLE | AM_HAL_CTIMER_XT_256HZ;
        ptm_info->u32tm_delay= 7;
        u8cb_id = 3;
    }
    else if(modul == STOPWATCH_CTIMER_MODULE) //timer:(2,A)
    {
        ptm_info->u32tm_id=2;
        ptm_info->u32tm_seg=AM_HAL_CTIMER_TIMERA;
        ptm_info->u32tm_irq=AM_HAL_CTIMER_INT_TIMERA2;
        ptm_info->u32tm_conf=AM_HAL_CTIMER_FN_REPEAT | AM_HAL_CTIMER_INT_ENABLE | AM_HAL_CTIMER_XT_32_768KHZ;
//        ptm_info->u32tm_delay= 328;	// 秒表精度	
		ptm_info->u32tm_delay = (uint32)((32768 * u16freq / 1000) - 1);	// 转为ms 
        u8cb_id = 4;
    }
    else if(modul == COUNTDOWN_CTIMER_MODULE) //timer:(2,B)
    {
        ptm_info->u32tm_id=2;
        ptm_info->u32tm_seg=AM_HAL_CTIMER_TIMERB;
        ptm_info->u32tm_irq=AM_HAL_CTIMER_INT_TIMERB2;
        ptm_info->u32tm_conf=AM_HAL_CTIMER_FN_REPEAT | AM_HAL_CTIMER_INT_ENABLE | AM_HAL_CTIMER_XT_256HZ;
        ptm_info->u32tm_delay= (uint32)((256 * u16freq / 1000) - 1);
        u8cb_id = 5;
    }
    else if(modul == RTC_CTIMER_MODULE)       //timer:(3,A)
    {
        ptm_info->u32tm_id=3;
        ptm_info->u32tm_seg=AM_HAL_CTIMER_TIMERA;
        ptm_info->u32tm_irq=AM_HAL_CTIMER_INT_TIMERA3;
        ptm_info->u32tm_conf=AM_HAL_CTIMER_FN_REPEAT | AM_HAL_CTIMER_INT_ENABLE | AM_HAL_CTIMER_XT_256HZ;
        ptm_info->u32tm_delay= 127;
        u8cb_id = 6;
    }
    else if(modul == MONITOR_CTIMER_MODULE)   //timer:(3,B)
    {
        ptm_info->u32tm_id=3;
        ptm_info->u32tm_seg=AM_HAL_CTIMER_TIMERB;
        ptm_info->u32tm_irq=AM_HAL_CTIMER_INT_TIMERB3;
        ptm_info->u32tm_conf=AM_HAL_CTIMER_FN_REPEAT | AM_HAL_CTIMER_INT_ENABLE | AM_HAL_CTIMER_XT_16_384KHZ;
        ptm_info->u32tm_delay=(16384/(u16freq * 2));
        u8cb_id = 7;
    }
    else if(modul == MOVT_CTIMER_MODULE) //小表盘
    {
        return FALSE;  //项目中无
    }
    else
    {
        return FALSE;
    }

    //set callback
    if(timer_callback != NULL)
    {
        ctimer_irq_cb[u8cb_id] = timer_callback;
    }
    return TRUE;
}

//**********************************************************************
// 函数功能: 初始化counter/timer
// 输入参数：
// 返回参数：
//**********************************************************************
void SMDrv_CTimer_Init(void)
{
    memset(ctimer_irq_cb,NULL,sizeof(ctimer_irq_cb));
}

//**********************************************************************
// 函数功能: 根据driver module ID打开硬件对应的ctimer
// 输入参数：	
//    modul: driver module ID,值参考ctimer_module
//    u16freq: timer频率
//    timer_callback:上层注册的中断回调函数
// 返回参数：Ret_InvalidParam或Ret_OK
//**********************************************************************
ret_type SMDrv_CTimer_Open(ctimer_module modul,uint16 u16freq,comm_cb *timer_callback)
{
    ctimer_info_s stm_info;

    if(ctimer_modul2time(modul,u16freq,timer_callback,&stm_info) == FALSE)
    {
        return Ret_InvalidParam ;//参数有误
    }

    //init ctimer
    BspTimerInitCom(stm_info.u32tm_id,stm_info.u32tm_seg,stm_info.u32tm_irq,stm_info.u32tm_conf,stm_info.u32tm_delay);
    return Ret_OK;
}

//**********************************************************************
// 函数功能:  设置CTimer中断优先级
// 输入参数： prio:中断优先级，bit0~2位值有效
// 返回参数： 无
//**********************************************************************
void SMDrv_CTimer_SetIsrPrio(uint32 prio)
{
#if AM_CMSIS_REGS
    NVIC_SetPriority(CTIMER_IRQn,prio);
#else
    am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_CTIMER, AM_HAL_INTERRUPT_PRIORITY(prio));
#endif
}

//**********************************************************************
// 函数功能 : 设置timer比较值
// 输入参数 ：
//    modul : driver module ID,值参考ctimer_module
//u32cmp_cnt: 比较值
// 返回参数：Ret_InvalidParam或Ret_OK
//**********************************************************************
ret_type SMDrv_CTimer_SetCmpValue(ctimer_module modul,uint32 u32cmp_cnt)
{
    ctimer_info_s stm_info;

    if(ctimer_modul2time(modul,0,NULL,&stm_info) == FALSE)
    {
        return Ret_InvalidParam ;//参数有误
    }

	am_hal_ctimer_compare_set(stm_info.u32tm_id,stm_info.u32tm_seg,0,u32cmp_cnt);	
    return Ret_OK;
}

//**********************************************************************
// 函数功能: 启动timer
// 输入参数：modul: driver module ID,值参考ctimer_module
// 返回参数：Ret_InvalidParam或Ret_OK
//**********************************************************************
ret_type SMDrv_CTimer_Start(ctimer_module modul)
{
    ctimer_info_s stm_info;

    if(ctimer_modul2time(modul,0,NULL,&stm_info) == FALSE)
    {
        return Ret_InvalidParam ;//参数有误
    }

	am_hal_ctimer_int_enable(stm_info.u32tm_irq);
	am_hal_ctimer_start(stm_info.u32tm_id, stm_info.u32tm_seg);	
    return Ret_OK;
}

//**********************************************************************
// 函数功能: 停止timer
// 输入参数: modul: driver module ID,值参考ctimer_module
// 返回参数：Ret_InvalidParam或Ret_OK
//**********************************************************************
ret_type SMDrv_CTimer_Stop(ctimer_module modul)
{
    ctimer_info_s stm_info;

    if(ctimer_modul2time(modul,0,NULL,&stm_info) == FALSE)
    {
        return Ret_InvalidParam ;//参数有误
    }

	am_hal_ctimer_stop(stm_info.u32tm_id, stm_info.u32tm_seg);
	am_hal_ctimer_int_disable(stm_info.u32tm_irq);
    return Ret_OK;
}

//**********************************************************************
// 函数功能: 清零timer
// 输入参数：modul: driver module ID,值参考ctimer_module
// 返回参数：Ret_InvalidParam或Ret_OK
//**********************************************************************
ret_type SMDrv_CTimer_Clear(ctimer_module modul)
{
    ctimer_info_s stm_info;

    if(ctimer_modul2time(modul,0,NULL,&stm_info) == FALSE)
    {
        return Ret_InvalidParam ;//参数有误
    }

	am_hal_ctimer_clear(stm_info.u32tm_id, stm_info.u32tm_seg);
    return Ret_OK;
}

//**********************************************************************
// 函数功能: 读取timer计数值
// 输入参数: modul: driver module ID,值参考ctimer_module
// 返回参数: 返回timer计数值 
//**********************************************************************
uint16 SMDrv_CTimer_ReadCount(ctimer_module modul)
{
    ctimer_info_s stm_info;

    if(ctimer_modul2time(modul,0,NULL,&stm_info) == FALSE)
    {
        return Ret_InvalidParam ;//参数有误
    }

	return am_hal_ctimer_read(stm_info.u32tm_id, stm_info.u32tm_seg);
}

//**********************************************************************
// 函数功能: 获取模块使用的ctimer的number
// 输入参数: modul: driver module ID,值参考ctimer_module
// 返回参数: ctimer的number
//**********************************************************************
uint32 SMDrv_CTimer_GetTimerNumber(ctimer_module modul)
{
    ctimer_info_s stm_info;

    if(ctimer_modul2time(modul,0,NULL,&stm_info) == FALSE)
    {
        return Ret_InvalidParam ;//参数有误
    }

	return stm_info.u32tm_id;
}

