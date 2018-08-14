#ifndef __SM_TIMER_H
#define __SM_TIMER_H

#include "platform_common.h"

//定义使用CTimer的模块类型
//timer 0A~3B，在中断服务程序中，处理顺序:0A->0B->1A->1B->2A->2B->3A->3B (0~7)
//因此优先处理的Timer，要安排在前面的
typedef enum
{
    MOVT_CTIMER_MODULE,      //主表盘
    MULTI_CTIMER_MODULE,     //multi-timer
    SYSTICK_CTIMER_MODULE,   //系统tick时钟
    KEY_CTIMER_MODULE,       //按键
    STOPWATCH_CTIMER_MODULE, //秒表 stop watch
    COUNTDOWN_CTIMER_MODULE, //倒计时 count down
    RTC_CTIMER_MODULE,       //RTC
    MONITOR_CTIMER_MODULE,   //monitor LCD屏
    //SUBMOVT_CTIMER_MODULE,   //小表盘
}ctimer_module;

//**********************************************************************
// 函数功能: 初始化counter/timer
// 输入参数：
// 返回参数：
//**********************************************************************
extern void SMDrv_CTimer_Init(void);

//**********************************************************************
// 函数功能: 根据driver module ID打开硬件对应的ctimer
// 输入参数：	
//    modul: driver module ID,值参考ctimer_module
//    u16freq: timer频率
//    timer_callback:上层注册的中断回调函数
// 返回参数：Ret_InvalidParam或Ret_OK
//**********************************************************************
extern ret_type SMDrv_CTimer_Open(ctimer_module modul,uint16 u16freq,comm_cb *timer_callback);

//**********************************************************************
// 函数功能:  设置CTimer中断优先级
// 输入参数： prio:中断优先级，bit0~2位值有效
// 返回参数： 无
//**********************************************************************
extern void SMDrv_CTimer_SetIsrPrio(uint32 prio);

//**********************************************************************
// 函数功能 : 设置timer比较值
// 输入参数 ：
//    modul : driver module ID,值参考ctimer_module
//u32cmp_cnt: 比较值
// 返回参数：Ret_InvalidParam或Ret_OK
//**********************************************************************
ret_type SMDrv_CTimer_SetCmpValue(ctimer_module modul,uint32 u32cmp_cnt);

//**********************************************************************
// 函数功能: 启动timer
// 输入参数：modul: driver module ID,值参考ctimer_module
// 返回参数：Ret_InvalidParam或Ret_OK
//**********************************************************************
extern ret_type SMDrv_CTimer_Start(ctimer_module modul);

//**********************************************************************
// 函数功能: 停止timer
// 输入参数: modul: driver module ID,值参考ctimer_module
// 返回参数：Ret_InvalidParam或Ret_OK
//**********************************************************************
extern ret_type SMDrv_CTimer_Stop(ctimer_module modul);

//**********************************************************************
// 函数功能: 清零timer
// 输入参数：modul: driver module ID,值参考ctimer_module
// 返回参数：Ret_InvalidParam或Ret_OK
//**********************************************************************
extern ret_type SMDrv_CTimer_Clear(ctimer_module modul);

//**********************************************************************
// 函数功能: 读取timer计数值
// 输入参数: modul: driver module ID,值参考ctimer_module
// 返回参数: 返回timer计数值 
//**********************************************************************
extern uint16 SMDrv_CTimer_ReadCount(ctimer_module modul);

//**********************************************************************
// 函数功能: 获取模块使用的ctimer的number
// 输入参数: modul: driver module ID,值参考ctimer_module
// 返回参数: ctimer的number
//**********************************************************************
extern uint32 SMDrv_CTimer_GetTimerNumber(ctimer_module modul);

#endif

