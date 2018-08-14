/**********************************************************************
**
**模块说明: 表针驱动接口
**   Main Movt: 大表盘；Sub Movt: 小表盘
**
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.21  修改流程  ZSL  
**
**********************************************************************/
#include "drv_movt.h"

//如果有表针，至少有主表盘
#if(SUPPORT_MOVT == 1)
#include "sm_gpio.h"
#include "sm_timer.h"
#include "sm_sys.h"

// movt state 
#define MOVT_STANDBY                   0
#define MOVT_BUSY                      1

// 3 point movt
// #define MOVE_M_CLOCK_FORWARD_MAX       32400//
// #define MOVE_M_CLOCK_REVERSE_MAX       10800//
// #define MOVE_M_CLOCK_MAX_RANGE         43200

//H001两针齿轮箱
#define         MOVE_M_CLOCK_FORWARD_MAX            3240  
#define         MOVE_M_CLOCK_REVERSE_MAX            1080
#define         MOVE_M_CLOCK_MAX_RANGE              4320

// 1 point movt
#define MOVE_3CLOCK_FORWARD_MAX         30
#define MOVE_3CLOCK_REVERSE_MAX         30
#define MOVE_3CLOCK_MAX_RANGE           60

#define MOVE_6CLOCK_FORWARD_MAX         30
#define MOVE_6CLOCK_REVERSE_MAX         30
#define MOVE_6CLOCK_MAX_RANGE           60

#define MOVE_9CLOCK_FORWARD_MAX         30
#define MOVE_9CLOCK_REVERSE_MAX         30
#define MOVE_9CLOCK_MAX_RANGE           60

movt_att_s  movtConfig[MOVT_MAX_NUM];

//定义callback
comm_cb *MovtForwardFinishIsr[MOVT_MAX_NUM];
comm_cb *MovtResverseFinishIsr[MOVT_MAX_NUM];

void MovtclockCompare(movt_att_s *movtCompare);

// 31us every cnt
uint8 tinyForwardClock_M[24] = 
{
    200,10,22,22,22,22,22,22,22,22,22,22,22,10,100,255
};

uint8 tinyReverseClock_M[24] = 
{
    255,8,20,41,133,142,15,15,15,15,15,15,15,15,255,255
};

#if((MOVT_3_A_PIN != IO_UNKNOW) && (MOVT_3_B_PIN != IO_UNKNOW))
// 31us every cnt
uint8 tinyForwardClock_3[24] = 
{
    100,4,8,8,8,8,8,8,8,8,8,8,8,4,100,255
};

// old
uint8 tinyReverseClock_3[24] = 
{
    255,6,20,18,37,6,6,6,6,6,6,6,6,6,6,6,255,255
};
#endif

#if((MOVT_6_A_PIN != IO_UNKNOW) && (MOVT_6_B_PIN != IO_UNKNOW))
// 31us every cnt
 uint8 tinyForwardClock_6[24] = 
{
    100,4,8,8,8,8,8,8,8,8,8,8,8,4,100,255
};

uint8 tinyReverseClock_6[24] = 
{
    255,6,20,18,37,6,6,6,6,6,6,6,6,6,6,6,255,255
};
#endif

#if((MOVT_9_A_PIN != IO_UNKNOW) && (MOVT_9_B_PIN != IO_UNKNOW))
// 31us every cnt
uint8 tinyForwardClock_9[24] = 
{
    100,4,8,8,8,8,8,8,8,8,8,8,8,4,100,255
};
// oled
uint8 tinyReverseClock_9[24] = 
{
    255,6,20,18,37,6,6,6,6,6,6,6,6,6,6,6,255,255
};
#endif

static void fun_null(void)
{
}

static uint8 movt_check(movt_num_t movt_id)
{
    if(MOVT_MAX_NUM == 0)
        return FALSE;
    else if(movt_id >= MOVT_MAX_NUM)
        return FALSE;

    return TRUE;
}

//**********************************************************************
// 函数功能:  启动大表盘timer
// 输入参数： cnt:timer比较值
// 返回参数： 无
//**********************************************************************
static void MainMovt_StartTimer(uint16 cnt)
{
    SMDrv_CTimer_SetCmpValue(MOVT_CTIMER_MODULE,cnt);
    SMDrv_CTimer_Start(MOVT_CTIMER_MODULE);
}

//**********************************************************************
// 函数功能:  停止大表盘timer
// 输入参数： 无
// 返回参数： 无
//**********************************************************************
static void MainMovt_StopTimer(void)
{
    SMDrv_CTimer_Stop(MOVT_CTIMER_MODULE);
    SMDrv_CTimer_Clear(MOVT_CTIMER_MODULE);
}

#ifdef SUB_MOVT
static uint8 bspSpeMovtTimerSwitch = 0;
//**********************************************************************
// 函数功能:  启动小表盘timer
// 输入参数： cnt:timer比较值
// 返回参数： 无
//**********************************************************************
static void SubMovt_StartTimer(uint16 cnt)
{
    if(bspSpeMovtTimerSwitch == 0)
    {
        bspSpeMovtTimerSwitch = 1;
        SMDrv_CTimer_SetCmpValue(SUBMOVT_CTIMER_MODULE,1);
        SMDrv_CTimer_Start(SUBMOVT_CTIMER_MODULE);
    }
}

//**********************************************************************
// 函数功能:  停止小表盘timer
// 输入参数： 无
// 返回参数： 无
//**********************************************************************
static void SubMovt_StopTimer(void)
{
    SMDrv_CTimer_Stop(SUBMOVT_CTIMER_MODULE);
    SMDrv_CTimer_Clear(SUBMOVT_CTIMER_MODULE);
}
#endif

//**********************************************************************
// 函数功能:  大表盘timer中断服务程序
// 输入参数： 无
// 返回参数： 无
//**********************************************************************
static uint8 reverseflag = 0;
void MainMovt_TimerIsr(void)
{
    if(movtConfig[MOVT_M_CLOCK].direction == MOVT_DIR_FORWARD)
    {
        switch(movtConfig[MOVT_M_CLOCK].movtStep)
        {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
            movtConfig[MOVT_M_CLOCK].timerStop();
            movtConfig[MOVT_M_CLOCK].timerStart(movtConfig[MOVT_M_CLOCK].clockForward[movtConfig[MOVT_M_CLOCK].movtStep + 1]);
            SMDrv_GPIO_BitToggle(MOVT_M_A_PIN);
            break;
        case 13:
            movtConfig[MOVT_M_CLOCK].timerStop();
            movtConfig[MOVT_M_CLOCK].timerStart(movtConfig[MOVT_M_CLOCK].clockForward[movtConfig[MOVT_M_CLOCK].movtStep + 1]);
            SMDrv_GPIO_BitToggle(MOVT_M_B_PIN);
            break;
        case 14:	
            movtConfig[MOVT_M_CLOCK].timerStop();
            movtConfig[MOVT_M_CLOCK].timerStart(movtConfig[MOVT_M_CLOCK]. clockReverse[movtConfig[MOVT_M_CLOCK].movtStep + 1]);
            break;
        case 15:
            movtConfig[MOVT_M_CLOCK].state  = MOVT_STANDBY;
            movtConfig[MOVT_M_CLOCK].movtStep = 0;
            movtConfig[MOVT_M_CLOCK].timerStop();

            //clock finish, should be process in forward
            MovtForwardFinishIsr[MOVT_M_CLOCK]();
            return;
            // break;
        }
        movtConfig[MOVT_M_CLOCK].movtStep++;
    }
    else//reverse
    {
        switch(movtConfig[MOVT_M_CLOCK].movtStep)
        {
            case 0://turn overone pin
            case 1:
            case 2:
            case 5: 
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
            case 12:
            movtConfig[MOVT_M_CLOCK].timerStop();
            movtConfig[MOVT_M_CLOCK].timerStart(movtConfig[MOVT_M_CLOCK]. clockReverse[movtConfig[MOVT_M_CLOCK].movtStep + 1]);
            am_hal_gpio_out_bit_toggle(MOVT_M_A_PIN);
            break;
            case 3:
            case 4: 
            movtConfig[MOVT_M_CLOCK].timerStop();
            movtConfig[MOVT_M_CLOCK].timerStart(movtConfig[MOVT_M_CLOCK].clockReverse[movtConfig[MOVT_M_CLOCK].movtStep + 1]);
            am_hal_gpio_out_bit_toggle(MOVT_M_A_PIN);
            am_hal_gpio_out_bit_toggle(MOVT_M_B_PIN);
            break;
            case 13:
            movtConfig[MOVT_M_CLOCK].timerStop();
            movtConfig[MOVT_M_CLOCK].timerStart(movtConfig[MOVT_M_CLOCK].clockReverse[movtConfig[MOVT_M_CLOCK].movtStep + 1]);
            am_hal_gpio_out_bit_toggle(MOVT_M_B_PIN);
            break;
                                    
            case 14:  //clock run finish 
            movtConfig[MOVT_M_CLOCK].state  = MOVT_STANDBY;
            movtConfig[MOVT_M_CLOCK].movtStep = 0;
            movtConfig[MOVT_M_CLOCK].timerStop();

/*********clock finish, should be process in forward*************/
            MovtResverseFinishIsr[MOVT_M_CLOCK]();

/*********clock finish, should be process in forward*************/
            return;
            // break;

        }

        #if 0
        if(reverseflag)
        {		
            switch(movtConfig[MOVT_M_CLOCK].movtStep)
            {
            case 0:   //turn overone pin
            //movtConfig[MOVT_M_CLOCK].movtStep = 0;
            case 1:
            case 2:
            case 5:	
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
            case 12:
            case 13:
                movtConfig[MOVT_M_CLOCK].timerStop();
                SMDrv_GPIO_BitToggle(MOVT_M_B_PIN);
                movtConfig[MOVT_M_CLOCK].timerStart(movtConfig[MOVT_M_CLOCK]. clockReverse[movtConfig[MOVT_M_CLOCK].movtStep + 1]);
                //SMDrv_GPIO_BitToggle(MOVT_M_A_PIN);
                break;
            case 3:
            case 4: 
                movtConfig[MOVT_M_CLOCK].timerStop();
                SMDrv_GPIO_BitToggle(MOVT_M_A_PIN);
                SMDrv_GPIO_BitToggle(MOVT_M_B_PIN);
                movtConfig[MOVT_M_CLOCK].timerStart(movtConfig[MOVT_M_CLOCK].clockReverse[movtConfig[MOVT_M_CLOCK].movtStep + 1]);				
                break;
            case 14:
            case 15:	
            case 16:					
                movtConfig[MOVT_M_CLOCK].timerStop();
                movtConfig[MOVT_M_CLOCK].timerStart(movtConfig[MOVT_M_CLOCK]. clockReverse[movtConfig[MOVT_M_CLOCK].movtStep + 1]);
                break;
            case 17:  //clock run finish 
                movtConfig[MOVT_M_CLOCK].state  = MOVT_STANDBY;
                movtConfig[MOVT_M_CLOCK].movtStep = 0;
                movtConfig[MOVT_M_CLOCK].timerStop();

                //clock finish, should be process in forward
                MovtResverseFinishIsr[MOVT_M_CLOCK]();

                //clock finish, should be process in forward
                SMDrv_GPIO_BitSet(MOVT_M_B_PIN);
                SMDrv_GPIO_BitSet(MOVT_M_A_PIN);
                reverseflag = 0;
                return;
                // break;
			}		
		}
		else
		{		
            switch(movtConfig[MOVT_M_CLOCK].movtStep)
            {
            case 0:  //turn overone pin
            //movtConfig[MOVT_M_CLOCK].movtStep = 0;
            case 1:
            case 2:
            case 5:	
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
            case 12:
            case 13:					
                movtConfig[MOVT_M_CLOCK].timerStop();
                SMDrv_GPIO_BitToggle(MOVT_M_A_PIN);
                movtConfig[MOVT_M_CLOCK].timerStart(movtConfig[MOVT_M_CLOCK]. clockReverse[movtConfig[MOVT_M_CLOCK].movtStep + 1]);				
                break;
            case 3:
            case 4: 
                movtConfig[MOVT_M_CLOCK].timerStop();
                SMDrv_GPIO_BitToggle(MOVT_M_B_PIN);
                SMDrv_GPIO_BitToggle(MOVT_M_A_PIN);
                movtConfig[MOVT_M_CLOCK].timerStart(movtConfig[MOVT_M_CLOCK].clockReverse[movtConfig[MOVT_M_CLOCK].movtStep + 1]);						
                break;
            case 14:
            case 15:
            case 16:
                movtConfig[MOVT_M_CLOCK].timerStop();
                movtConfig[MOVT_M_CLOCK].timerStart(movtConfig[MOVT_M_CLOCK]. clockReverse[movtConfig[MOVT_M_CLOCK].movtStep + 1]);
                break;
            case 17:  //clock run finish 
                movtConfig[MOVT_M_CLOCK].state  = MOVT_STANDBY;
                movtConfig[MOVT_M_CLOCK].movtStep = 0;
                movtConfig[MOVT_M_CLOCK].timerStop();

                //clock finish, should be process in forward
                MovtResverseFinishIsr[MOVT_M_CLOCK]();

                //clock finish, should be process in forward
                SMDrv_GPIO_BitSet(MOVT_M_B_PIN);
                SMDrv_GPIO_BitSet(MOVT_M_A_PIN);
                reverseflag = 1;
                return;
                // break;
            }		
		}
        #endif
        movtConfig[MOVT_M_CLOCK].movtStep++;
    }
}

#ifdef SUB_MOVT
//**********************************************************************
// 函数功能:  小表盘timer中断服务程序
// 输入参数： 无
// 返回参数： 无
//**********************************************************************
void SubMovt_TimerIsr(void)
{
    uint8 status = TRUE;
#if((MOVT_3_A_PIN != IO_UNKNOW) && (MOVT_3_B_PIN != IO_UNKNOW))
    if(movtConfig[MOVT_3_CLOCK].state == MOVT_BUSY)
    {
        movtConfig[MOVT_3_CLOCK].tinyTimeCnt++;
        if(movtConfig[MOVT_3_CLOCK].direction == MOVT_DIR_FORWARD)
        {
            if(movtConfig[MOVT_3_CLOCK].tinyTimeCnt >= movtConfig[MOVT_3_CLOCK].clockForward[movtConfig[MOVT_3_CLOCK].movtStep])
            {
                switch(movtConfig[MOVT_3_CLOCK].movtStep)
                {
                case 0://turn overone pin
                case 1:
                case 2:
                case 3:
                case 4: 
                case 5: 
                case 6:
                case 7:
                case 8:
                case 9:
                case 10:
                case 11:
                case 12:
                    SMDrv_GPIO_BitToggle(MOVT_3_A_PIN);
                    movtConfig[MOVT_3_CLOCK].movtStep++;
                    break;
                case 13:
                    SMDrv_GPIO_BitToggle(MOVT_3_B_PIN);
                    movtConfig[MOVT_3_CLOCK].movtStep++;
                    break;
                case 14:  //clock run finish
                    movtConfig[MOVT_3_CLOCK].state      = MOVT_STANDBY;
                    movtConfig[MOVT_3_CLOCK].movtStep   = 0; 
                    MovtForwardFinishIsr[MOVT_3_CLOCK]();
                    break;
                }
                movtConfig[MOVT_3_CLOCK].tinyTimeCnt = 0;
            }
        }
        else
        {
            if(movtConfig[MOVT_3_CLOCK].tinyTimeCnt >= movtConfig[MOVT_3_CLOCK].clockReverse[movtConfig[MOVT_3_CLOCK].movtStep])
            {
                // oled
                switch(movtConfig[MOVT_3_CLOCK].movtStep)
                {
                case 0://turn overone pin
                case 1:
                case 2:
                case 5: 
                case 6:
                case 7:
                case 8:
                case 9:
                case 10:
                case 11:
                case 12:
                case 13:
                case 14:
                    SMDrv_GPIO_BitToggle(MOVT_3_A_PIN);
                    movtConfig[MOVT_3_CLOCK].movtStep++;
                    break;
                case 3:
                case 4: 
                    SMDrv_GPIO_BitToggle(MOVT_3_A_PIN);
                    SMDrv_GPIO_BitToggle(MOVT_3_B_PIN);
                    movtConfig[MOVT_3_CLOCK].movtStep++;
                    break;
                case 15:
                    SMDrv_GPIO_BitToggle(MOVT_3_B_PIN);
                    movtConfig[MOVT_3_CLOCK].movtStep++;
                    break;
                case 16:  //clock run finish
                    movtConfig[MOVT_3_CLOCK].state      = MOVT_STANDBY;
                    movtConfig[MOVT_3_CLOCK].movtStep   = 0;
                    MovtResverseFinishIsr[MOVT_3_CLOCK](); 
                    break;
                }
                movtConfig[MOVT_3_CLOCK].tinyTimeCnt = 0;
            } 
        }
    }
#endif
#if((MOVT_6_A_PIN != IO_UNKNOW) && (MOVT_6_B_PIN != IO_UNKNOW))
    if(movtConfig[MOVT_6_CLOCK].state == MOVT_BUSY)
	{
        movtConfig[MOVT_6_CLOCK].tinyTimeCnt++;
        if(movtConfig[MOVT_6_CLOCK].direction == MOVT_DIR_FORWARD)
        {
            if(movtConfig[MOVT_6_CLOCK].tinyTimeCnt >= movtConfig[MOVT_6_CLOCK].clockForward[movtConfig[MOVT_6_CLOCK].movtStep])
            {
                switch(movtConfig[MOVT_6_CLOCK].movtStep)
                {
                case 0://turn overone pin
                case 1:
                case 2:
                case 3:
                case 4: 
                case 5: 
                case 6:
                case 7:
                case 8:
                case 9:
                case 10:
                case 11:
                case 12:
					SMDrv_GPIO_BitToggle(MOVT_6_A_PIN);
					movtConfig[MOVT_6_CLOCK].movtStep++;
					break;
                case 13:
					SMDrv_GPIO_BitToggle(MOVT_6_B_PIN);
					movtConfig[MOVT_6_CLOCK].movtStep++;
					break;
                case 14:  //clock run finish
					movtConfig[MOVT_6_CLOCK].state      = MOVT_STANDBY;
					movtConfig[MOVT_6_CLOCK].movtStep   = 0; 
					MovtForwardFinishIsr[MOVT_6_CLOCK]();
					break;
                }
                movtConfig[MOVT_6_CLOCK].tinyTimeCnt = 0;
            }
        }
        else
        {
            if(movtConfig[MOVT_6_CLOCK].tinyTimeCnt >= movtConfig[MOVT_6_CLOCK].clockReverse[movtConfig[MOVT_6_CLOCK].movtStep])
            {
                switch(movtConfig[MOVT_6_CLOCK].movtStep)
                {
                case 0://turn overone pin
                case 1:
                case 2:
                case 5: 
                case 6:
                case 7:
                case 8:
                case 9:
                case 10:
                case 11:
                case 12:
                case 13:
                case 14:
                    SMDrv_GPIO_BitToggle(MOVT_6_A_PIN);
                    movtConfig[MOVT_6_CLOCK].movtStep++;
                    break;
                case 3:
                case 4: 
                    SMDrv_GPIO_BitToggle(MOVT_6_A_PIN);
                    SMDrv_GPIO_BitToggle(MOVT_6_B_PIN);
                    movtConfig[MOVT_6_CLOCK].movtStep++;
                    break;
                case 15:
                    SMDrv_GPIO_BitToggle(MOVT_6_B_PIN);
                    movtConfig[MOVT_6_CLOCK].movtStep++;
                    break;
                case 16:  //clock run finish
                    movtConfig[MOVT_6_CLOCK].state      = MOVT_STANDBY;
                    movtConfig[MOVT_6_CLOCK].movtStep   = 0;
                    MovtResverseFinishIsr[MOVT_6_CLOCK](); 
                    break;
                }
                movtConfig[MOVT_6_CLOCK].tinyTimeCnt = 0;
            } 
        }
    }
#endif
#if((MOVT_9_A_PIN != IO_UNKNOW) && (MOVT_9_B_PIN != IO_UNKNOW))
    if(movtConfig[MOVT_9_CLOCK].state == MOVT_BUSY)
    {
        movtConfig[MOVT_9_CLOCK].tinyTimeCnt++;
        if(movtConfig[MOVT_9_CLOCK].direction == MOVT_DIR_FORWARD)
        {
            if(movtConfig[MOVT_9_CLOCK].tinyTimeCnt >= movtConfig[MOVT_9_CLOCK].clockForward[movtConfig[MOVT_9_CLOCK].movtStep])
            {
                switch(movtConfig[MOVT_9_CLOCK].movtStep)
                {
                case 0://turn overone pin
                case 1:
                case 2:
                case 3:
                case 4: 
                case 5: 
                case 6:
                case 7:
                case 8:
                case 9:
                case 10:
                case 11:
                case 12:
                    SMDrv_GPIO_BitToggle(MOVT_9_A_PIN);
                    movtConfig[MOVT_9_CLOCK].movtStep++;
                    break;
                case 13:
                    SMDrv_GPIO_BitToggle(MOVT_9_B_PIN);
                    movtConfig[MOVT_9_CLOCK].movtStep++;
                    break;
                case 14:  //clock run finish
                    movtConfig[MOVT_9_CLOCK].state      = MOVT_STANDBY;
                    movtConfig[MOVT_9_CLOCK].movtStep   = 0; 
                    MovtForwardFinishIsr[MOVT_9_CLOCK]();
                    break;
                }
                movtConfig[MOVT_9_CLOCK].tinyTimeCnt = 0;
            }
        }
        else
        {
            if(movtConfig[MOVT_9_CLOCK].tinyTimeCnt >= movtConfig[MOVT_9_CLOCK].clockReverse[movtConfig[MOVT_9_CLOCK].movtStep])
            {
                // oled
                switch(movtConfig[MOVT_9_CLOCK].movtStep)
                {
                case 0://turn overone pin
                case 1:
                case 2:
                case 5: 
                case 6:
                case 7:
                case 8:
                case 9:
                case 10:
                case 11:
                case 12:
                case 13:
                case 14:
                    SMDrv_GPIO_BitToggle(MOVT_9_A_PIN);
                    movtConfig[MOVT_9_CLOCK].movtStep++;
                    break;
                case 3:
                case 4: 
                    SMDrv_GPIO_BitToggle(MOVT_9_A_PIN);
                    SMDrv_GPIO_BitToggle(MOVT_9_B_PIN);
                    movtConfig[MOVT_9_CLOCK].movtStep++;
                    break;
                case 15:
                    SMDrv_GPIO_BitToggle(MOVT_9_B_PIN);
                    movtConfig[MOVT_9_CLOCK].movtStep++;
                    break;
                case 16:  //clock run finish
                    movtConfig[MOVT_9_CLOCK].state      = MOVT_STANDBY;
                    movtConfig[MOVT_9_CLOCK].movtStep   = 0;
                    MovtResverseFinishIsr[MOVT_9_CLOCK](); 
                    break;
                }
                movtConfig[MOVT_9_CLOCK].tinyTimeCnt = 0;
            }
        }
    }
#endif

    status = TRUE;
#if((MOVT_3_A_PIN != IO_UNKNOW) && (MOVT_3_B_PIN != IO_UNKNOW))
    if(movtConfig[MOVT_3_CLOCK].state != MOVT_STANDBY)
        status &= FALSE;
#endif
#if((MOVT_6_A_PIN != IO_UNKNOW) && (MOVT_6_B_PIN != IO_UNKNOW))
    if(movtConfig[MOVT_6_CLOCK].state != MOVT_STANDBY)
        status &= FALSE;
#endif
#if((MOVT_9_A_PIN != IO_UNKNOW) && (MOVT_9_B_PIN != IO_UNKNOW))
    if(movtConfig[MOVT_9_CLOCK].state != MOVT_STANDBY)
        status &= FALSE;
#endif

#if 0
    if((movtConfig[MOVT_3_CLOCK].state == MOVT_STANDBY)
        &&(movtConfig[MOVT_6_CLOCK].state == MOVT_STANDBY)
        &&(movtConfig[MOVT_9_CLOCK].state == MOVT_STANDBY))
#else
    if(status == TRUE)
#endif
    {
        bspSpeMovtTimerSwitch   = 0;
        SubMovt_StopTimer();
    }
}
#endif

//**********************************************************************
// 函数功能:  设置FrowardFinish callback
// 输入参数： movt_id:表针ID
//      fwd_finish_cb:callback
// 返回参数： 无
//**********************************************************************
uint8 Drv_Movt_SetFrowardFinishCb(movt_num_t movt_id, comm_cb fwd_finish_cb)
{
    if(movt_check(movt_id) == FALSE)
        return Ret_InvalidParam;

    if(fwd_finish_cb != NULL)
    {
        MovtForwardFinishIsr[movt_id] = fwd_finish_cb;
    }
    return Ret_OK;
}

//**********************************************************************
// 函数功能:  设置ReverseFinish callback
// 输入参数： movt_id:表针ID
//      rvs_finish_cb:callback
// 返回参数： 无
//**********************************************************************
uint8 Drv_Movt_SetReverseFinishCb(movt_num_t movt_id, comm_cb rvs_finish_cb)
{
    if(movt_check(movt_id) == FALSE)
        return Ret_InvalidParam;

    if(rvs_finish_cb)
    {
        MovtResverseFinishIsr[movt_id] = rvs_finish_cb;
    }
    return Ret_OK;
}

//**********************************************************************
// 函数功能:  比较表针当前位置和目标位置
// 输入参数： movtCompare:表针ID
// 返回参数： 无
//**********************************************************************
void Drv_Movt_ClockCompare(movt_att_s *movtCompare)
{
    // movt is stop state or busy state return
    if(movtCompare->stop || movtCompare->state == MOVT_BUSY) 
        return;
    if(movtCompare->repeat)
    {
        if(movtCompare->state == MOVT_STANDBY)
        {
            movtCompare->state      = MOVT_BUSY;
            movtCompare->direction  = movtCompare->tempDirection;
            if(movtCompare->direction == MOVT_DIR_FORWARD)
            {
                movtCompare->timerStart(movtCompare->clockForward[0]);
            }
            else
            {
                movtCompare->timerStart(movtCompare->clockReverse[0]);
            }
        }
    }
    else
    {
        //movt is reach the aim position will return
        if(movtCompare->currentPosition  == movtCompare->aimPosition)
            return;
        if(movtCompare->directionSet == MOVT_DIR_AUTO)
        {
            #if 0 //暂不支持反转
            if(movtCompare->currentPosition > movtCompare->aimPosition)
            {
                if((movtCompare->currentPosition - movtCompare->aimPosition) > movtCompare->movtReverseMax)
                { 
                    movtCompare->direction = MOVT_DIR_FORWARD;  // forward run
                }
                else 
                {
                    movtCompare->direction = MOVT_DIR_REVERSE;  //reserse run
                }
            }
            else
            {
                if((movtCompare->aimPosition - movtCompare->currentPosition) > movtCompare->movtForwardMax)
                {
                    movtCompare->direction = MOVT_DIR_REVERSE;  //reserse run
                }
                else
                {
                    movtCompare->direction = MOVT_DIR_FORWARD;   //forward run
                }
            }
            #endif
            movtCompare->direction = MOVT_DIR_FORWARD;  // forward run  
        }
        else
        {
            movtCompare->direction  = movtCompare->tempDirection;
        }

        if(movtCompare->state == MOVT_STANDBY)
        {
            movtCompare->state      = MOVT_BUSY;
            if(movtCompare->directionSet == MOVT_DIR_SETTING)
            {
                movtCompare->direction  = movtCompare->tempDirection;
            }
            
            if(movtCompare->direction == MOVT_DIR_FORWARD)
            {
                movtCompare->timerStart(movtCompare->clockForward[0]);
            }
            else
            {
                movtCompare->timerStart(movtCompare->clockReverse[0]);
            }
        }
    }
}

//**********************************************************************
// 函数功能:  判断正转或反转
// 输入参数： movtCompare
// 返回参数： 无
//**********************************************************************
uint16 Drv_Movt_RunDiretion(movt_att_s *movtCompare)
{
    if(movtCompare->repeat)
    {
        if(movtCompare->state == MOVT_STANDBY)
        {
            return movtCompare->direction;			
        }
    }
    else
    {
        //movt is reach the aim position will return
        if(movtCompare->currentPosition  == movtCompare->aimPosition)
            return 0xff;
        if(movtCompare->directionSet == MOVT_DIR_AUTO)
        {
            if(movtCompare->currentPosition > movtCompare->aimPosition)
            {
                if((movtCompare->currentPosition - movtCompare->aimPosition) > movtCompare->movtReverseMax)
                {
                    return MOVT_DIR_FORWARD;  // forward run
                }
                else
                {
                    return MOVT_DIR_REVERSE;  //reserse run
                }
            }
            else
            {
                if((movtCompare->aimPosition - movtCompare->currentPosition) > movtCompare->movtForwardMax)
                {
                    return  MOVT_DIR_REVERSE;  //reserse run
                }
                else
                {
                    return  MOVT_DIR_FORWARD;  //forward run
                }
            }
        }
        else
        {
            return  movtCompare->tempDirection;
        }
    }
    return movtCompare->direction;
}

//**********************************************************************
// 函数功能:  初始化表针
// 输入参数： 无
// 返回参数： 无
//**********************************************************************
void Drv_Movt_Init(void)
{
    uint8     i;
    SMDrv_GPIO_BitSet(MOVT_M_A_PIN);
    SMDrv_GPIO_BitSet(MOVT_M_B_PIN);    
    SMDrv_GPIO_Open(MOVT_M_A_PIN, NULL,NULL);
    SMDrv_GPIO_Open(MOVT_M_B_PIN, NULL,NULL);
#if((MOVT_3_A_PIN != IO_UNKNOW) && (MOVT_3_B_PIN != IO_UNKNOW))
    SMDrv_GPIO_BitSet(MOVT_3_A_PIN);
    SMDrv_GPIO_BitSet(MOVT_3_B_PIN);
    SMDrv_GPIO_Open(MOVT_3_A_PIN, NULL,NULL);
    SMDrv_GPIO_Open(MOVT_3_B_PIN, NULL,NULL);
#endif
#if((MOVT_6_A_PIN != IO_UNKNOW) && (MOVT_6_B_PIN != IO_UNKNOW))
    SMDrv_GPIO_BitSet(MOVT_6_A_PIN);
    SMDrv_GPIO_BitSet(MOVT_6_B_PIN);
    SMDrv_GPIO_Open(MOVT_6_A_PIN, NULL,NULL);
    SMDrv_GPIO_Open(MOVT_6_B_PIN, NULL,NULL);
#endif
#if((MOVT_9_A_PIN != IO_UNKNOW) && (MOVT_9_B_PIN != IO_UNKNOW))
    SMDrv_GPIO_BitSet(MOVT_9_A_PIN);
    SMDrv_GPIO_BitSet(MOVT_9_B_PIN);
    SMDrv_GPIO_Open(MOVT_9_A_PIN, NULL,NULL);
    SMDrv_GPIO_Open(MOVT_9_B_PIN, NULL,NULL);
#endif

    //为大表盘创建Timer，并注册callback
    SMDrv_CTimer_Open(MOVT_CTIMER_MODULE, 1, MainMovt_TimerIsr);
#ifdef SUB_MOVT
    //为小表盘创建Timer，并注册callback
    SMDrv_CTimer_Open(SUBMOVT_CTIMER_MODULE, 1, SubMovt_TimerIsr);
#endif
    SMDrv_CTimer_SetIsrPrio(1);  //the timer should set high

#ifdef SUB_MOVT
    bspSpeMovtTimerSwitch                       = 0;
#endif
    movtConfig[MOVT_M_CLOCK].currentPosition    = 0;
    movtConfig[MOVT_M_CLOCK].aimPosition        = 0;
    movtConfig[MOVT_M_CLOCK].movtStep           = 0;
    movtConfig[MOVT_M_CLOCK].movtForwardMax     = MOVE_M_CLOCK_FORWARD_MAX;
    movtConfig[MOVT_M_CLOCK].movtReverseMax     = MOVE_M_CLOCK_REVERSE_MAX;
    movtConfig[MOVT_M_CLOCK].movtRangeMax       = MOVE_M_CLOCK_MAX_RANGE;
    movtConfig[MOVT_M_CLOCK].tinyTimeCnt        = 0;
    movtConfig[MOVT_M_CLOCK].direction          = MOVT_DIR_FORWARD;
    movtConfig[MOVT_M_CLOCK].tempDirection      = MOVT_DIR_FORWARD;
    movtConfig[MOVT_M_CLOCK].directionSet       = MOVT_DIR_AUTO;
    movtConfig[MOVT_M_CLOCK].state              = MOVT_STANDBY;
    movtConfig[MOVT_M_CLOCK].repeat             = 0;
    movtConfig[MOVT_M_CLOCK].stop               = 0;
    movtConfig[MOVT_M_CLOCK].clockForward       = (uint8 *)tinyForwardClock_M;
    movtConfig[MOVT_M_CLOCK].clockReverse       = (uint8 *)tinyReverseClock_M;
    movtConfig[MOVT_M_CLOCK].timerStart         = MainMovt_StartTimer;
    movtConfig[MOVT_M_CLOCK].timerStop          = MainMovt_StopTimer;

#if((MOVT_3_A_PIN != IO_UNKNOW) && (MOVT_3_B_PIN != IO_UNKNOW))
    movtConfig[MOVT_3_CLOCK].currentPosition    = 0;
    movtConfig[MOVT_3_CLOCK].aimPosition        = 0;
    movtConfig[MOVT_3_CLOCK].movtStep           = 0;
    movtConfig[MOVT_3_CLOCK].movtForwardMax     = MOVE_3CLOCK_FORWARD_MAX;
    movtConfig[MOVT_3_CLOCK].movtReverseMax     = MOVE_3CLOCK_REVERSE_MAX;
    movtConfig[MOVT_3_CLOCK].movtRangeMax       = MOVE_3CLOCK_MAX_RANGE;
    movtConfig[MOVT_3_CLOCK].tinyTimeCnt        = 0;
    movtConfig[MOVT_3_CLOCK].direction          = MOVT_DIR_FORWARD;
    movtConfig[MOVT_3_CLOCK].tempDirection      = MOVT_DIR_FORWARD;
    movtConfig[MOVT_3_CLOCK].directionSet       = MOVT_DIR_AUTO;
    movtConfig[MOVT_3_CLOCK].state              = MOVT_STANDBY;
    movtConfig[MOVT_3_CLOCK].repeat             = 0;
    movtConfig[MOVT_3_CLOCK].stop               = 0;
    movtConfig[MOVT_3_CLOCK].clockForward       = (uint8 *)tinyForwardClock_3;
    movtConfig[MOVT_3_CLOCK].clockReverse       = (uint8 *)tinyReverseClock_3;
    movtConfig[MOVT_3_CLOCK].timerStart         = SubMovt_StartTimer;
    movtConfig[MOVT_3_CLOCK].timerStop          = SubMovt_StopTimer;
#endif
#if((MOVT_6_A_PIN != IO_UNKNOW) && (MOVT_6_B_PIN != IO_UNKNOW))
    movtConfig[MOVT_6_CLOCK].currentPosition    = 0;
    movtConfig[MOVT_6_CLOCK].aimPosition        = 0;
    movtConfig[MOVT_6_CLOCK].movtStep           = 0;
    movtConfig[MOVT_6_CLOCK].movtForwardMax     = MOVE_6CLOCK_FORWARD_MAX;
    movtConfig[MOVT_6_CLOCK].movtReverseMax     = MOVE_6CLOCK_REVERSE_MAX;
    movtConfig[MOVT_6_CLOCK].movtRangeMax       = MOVE_6CLOCK_MAX_RANGE;
    movtConfig[MOVT_6_CLOCK].tinyTimeCnt        = 0;
    movtConfig[MOVT_6_CLOCK].direction          = MOVT_DIR_FORWARD;
    movtConfig[MOVT_6_CLOCK].tempDirection      = MOVT_DIR_FORWARD;
    movtConfig[MOVT_6_CLOCK].directionSet       = MOVT_DIR_AUTO;
    movtConfig[MOVT_6_CLOCK].state              = MOVT_STANDBY;
    movtConfig[MOVT_6_CLOCK].repeat             = 0;
    movtConfig[MOVT_6_CLOCK].stop               = 0;
    movtConfig[MOVT_6_CLOCK].clockForward       = (uint8 *)tinyForwardClock_6;
    movtConfig[MOVT_6_CLOCK].clockReverse       = (uint8 *)tinyReverseClock_6;
    movtConfig[MOVT_6_CLOCK].timerStart         = SubMovt_StartTimer;
    movtConfig[MOVT_6_CLOCK].timerStop          = SubMovt_StopTimer;
#endif
#if((MOVT_9_A_PIN != IO_UNKNOW) && (MOVT_9_B_PIN != IO_UNKNOW))
    movtConfig[MOVT_9_CLOCK].currentPosition    = 0;
    movtConfig[MOVT_9_CLOCK].aimPosition        = 0;
    movtConfig[MOVT_9_CLOCK].movtStep           = 0;
    movtConfig[MOVT_9_CLOCK].movtForwardMax     = MOVE_9CLOCK_FORWARD_MAX;
    movtConfig[MOVT_9_CLOCK].movtReverseMax     = MOVE_9CLOCK_REVERSE_MAX;
    movtConfig[MOVT_9_CLOCK].movtRangeMax       = MOVE_9CLOCK_MAX_RANGE;
    movtConfig[MOVT_9_CLOCK].tinyTimeCnt        = 0;
    movtConfig[MOVT_9_CLOCK].direction          = MOVT_DIR_FORWARD;
    movtConfig[MOVT_9_CLOCK].tempDirection      = MOVT_DIR_FORWARD;
    movtConfig[MOVT_9_CLOCK].directionSet       = MOVT_DIR_AUTO;
    movtConfig[MOVT_9_CLOCK].state              = MOVT_STANDBY;
    movtConfig[MOVT_9_CLOCK].repeat             = 0;
    movtConfig[MOVT_9_CLOCK].stop               = 0;
    movtConfig[MOVT_9_CLOCK].clockForward       = (uint8 *)tinyForwardClock_9;
    movtConfig[MOVT_9_CLOCK].clockReverse       = (uint8 *)tinyReverseClock_9;
    movtConfig[MOVT_9_CLOCK].timerStart         = SubMovt_StartTimer;
    movtConfig[MOVT_9_CLOCK].timerStop          = SubMovt_StopTimer;
#endif

    for(i = 0; i < MOVT_MAX_NUM; i++)
    {
        MovtForwardFinishIsr[i]     = fun_null;
        MovtResverseFinishIsr[i]    = fun_null;
    }
}

//**********************************************************************
// 函数功能:	设置齿轮箱极性
// 输入参数：	无
// 返回参数：	0x00:操作成功
// 				0xff:操作失败
//**********************************************************************
uint16 Drv_Movt_PolarSet(movt_num_t clock, uint8 polar)
{
    switch(clock)
    {
		case MOVT_M_CLOCK:
        if(polar)
        {
            SMDrv_GPIO_BitSet(MOVT_M_A_PIN);
            SMDrv_GPIO_BitSet(MOVT_M_B_PIN);
        }
        else
        {
            SMDrv_GPIO_BitClear(MOVT_M_A_PIN);
            SMDrv_GPIO_BitClear(MOVT_M_B_PIN);
        }
        break;
#if((MOVT_3_A_PIN != IO_UNKNOW) && (MOVT_3_B_PIN != IO_UNKNOW))
    case MOVT_3_CLOCK:
        if(polar)
        {
            SMDrv_GPIO_BitSet(MOVT_3_A_PIN);
            SMDrv_GPIO_BitSet(MOVT_3_B_PIN);
        }
        else
        {
            SMDrv_GPIO_BitClear(MOVT_3_A_PIN);
            SMDrv_GPIO_BitClear(MOVT_3_B_PIN);
        }
        break;
#endif
#if((MOVT_6_A_PIN != IO_UNKNOW) && (MOVT_6_B_PIN != IO_UNKNOW))
        case MOVT_6_CLOCK:
        if(polar)
        {
            SMDrv_GPIO_BitSet(MOVT_6_A_PIN);
            SMDrv_GPIO_BitSet(MOVT_6_B_PIN);
        }
        else
        {
            SMDrv_GPIO_BitClear(MOVT_6_A_PIN);
            SMDrv_GPIO_BitClear(MOVT_6_B_PIN);
        }
        break;
#endif
#if((MOVT_9_A_PIN != IO_UNKNOW) && (MOVT_9_B_PIN != IO_UNKNOW))
        case MOVT_9_CLOCK:
        if(polar)
        {
            SMDrv_GPIO_BitSet(MOVT_9_A_PIN);
            SMDrv_GPIO_BitSet(MOVT_9_B_PIN);
        }
        else
        {
            SMDrv_GPIO_BitClear(MOVT_9_A_PIN);
            SMDrv_GPIO_BitClear(MOVT_9_B_PIN);
        }
        break;
#endif
    default:
        return 0xff;
    }
    return 0;
}

//**********************************************************************
// 函数功能:	读取齿轮箱极性
// 输入参数：	clock：指定指针
// 				polar：极性返回指针
// 返回参数：	0x00:操作成功
// 				0xff:操作失败
//**********************************************************************
uint16 Drv_Movt_PolarRead(movt_num_t clock, uint8 *polar)
{
    switch(clock)
    {
    case MOVT_M_CLOCK:
        *polar	= SMDrv_GPIO_OutBitRead(MOVT_M_A_PIN);
        break;
#if((MOVT_3_A_PIN != IO_UNKNOW) && (MOVT_3_B_PIN != IO_UNKNOW))
    case MOVT_3_CLOCK:
        *polar	= SMDrv_GPIO_OutBitRead(MOVT_3_A_PIN);
        break;
#endif
#if((MOVT_6_A_PIN != IO_UNKNOW) && (MOVT_6_B_PIN != IO_UNKNOW))
    case MOVT_6_CLOCK:
        *polar	= SMDrv_GPIO_OutBitRead(MOVT_6_A_PIN);
        break;
#endif
#if((MOVT_9_A_PIN != IO_UNKNOW) && (MOVT_9_B_PIN != IO_UNKNOW))
    case MOVT_9_CLOCK:
        *polar	= SMDrv_GPIO_OutBitRead(MOVT_9_A_PIN);
        break;
#endif
    default:
        return 0xff;
    }
    return 0;
}

//**********************************************************************
// 函数功能:	设置正转波形宽度，中间表盘为每1个计数30.05uS，其他每个计数为61uS
// 输入参数：	clock：指定指针
// 				polar：波形数据指针，长度为24个字节
// 返回参数：	0x00:操作成功
// 				0xff:操作失败
//**********************************************************************
uint16 Drv_Movt_ForwardWaveformSet(movt_num_t clock, uint8 *waveform)
{
    uint8	i;
    uint8 *temp = NULL;
    switch(clock)
    {
    case MOVT_M_CLOCK:
        temp = tinyForwardClock_M;
        break;
#if((MOVT_3_A_PIN != IO_UNKNOW) && (MOVT_3_B_PIN != IO_UNKNOW))
    case MOVT_3_CLOCK:
        temp = tinyForwardClock_3;
        break;
#endif
#if((MOVT_6_A_PIN != IO_UNKNOW) && (MOVT_6_B_PIN != IO_UNKNOW))
    case MOVT_6_CLOCK:
        temp = tinyForwardClock_6;
        break;
#endif
#if((MOVT_9_A_PIN != IO_UNKNOW) && (MOVT_9_B_PIN != IO_UNKNOW))
    case MOVT_9_CLOCK:
        temp = tinyForwardClock_9;
        break;
#endif
    default:
        return 0xff;
    }

    if(temp != NULL)
    {
        for(i = 0; i < 24; i++)
        {
            temp[i]	= waveform[i];
        }
    }
    return 0;
}

//**********************************************************************
// 函数功能: 设置反转波形宽度，中间表盘为每1个计数30.05uS，其他每个计数为61uS
// 输入参数：clock：指定指针
// 			 polar：波形数据指针，长度为24个字节
// 返回参数：0x00:操作成功
// 			 0xff:操作失败
//**********************************************************************
uint16 Drv_Movt_ReverseWaveformSet(movt_num_t clock, uint8 *waveform)
{
    uint8	i;
    uint8 *temp = NULL;
    switch(clock)
    {
    case MOVT_M_CLOCK:
        temp = tinyReverseClock_M;
        break;
#if((MOVT_3_A_PIN != IO_UNKNOW) && (MOVT_3_B_PIN != IO_UNKNOW))
    case MOVT_3_CLOCK:
        temp = tinyReverseClock_3;
        break;
#endif
#if((MOVT_6_A_PIN != IO_UNKNOW) && (MOVT_6_B_PIN != IO_UNKNOW))
    case MOVT_6_CLOCK:
        temp = tinyReverseClock_6;
        break;
#endif
#if((MOVT_9_A_PIN != IO_UNKNOW) && (MOVT_9_B_PIN != IO_UNKNOW))
    case MOVT_9_CLOCK:
        temp = tinyReverseClock_9;
        break;
#endif
    default:
        return 0xff;
    }

    if(temp != NULL)
    {
        for(i = 0; i < 24; i++)
        {
            temp[i]	= waveform[i];
        }
    }
    return 0;
}

uint16 Drv_Movt_ReadPositionInfo(movt_num_t movtId,uint16 actionId)
{
    uint16 positionVa;

    switch(actionId)
    {
        case MOVT_SET_MC_READ_CUR:
        case MOVT_SET_3C_READ_CUR:
        case MOVT_SET_6C_READ_CUR:
        case MOVT_SET_9C_READ_CUR:
        positionVa = movtConfig[movtId].currentPosition;
        break;

        case MOVT_SET_MC_READ_AIM:
        case MOVT_SET_3C_READ_AIM:
        case MOVT_SET_6C_READ_AIM:
        case MOVT_SET_9C_READ_AIM:
        positionVa = movtConfig[movtId].aimPosition;
        break;

        default:break;
    }

    return positionVa;
}

void Drv_Movt_MClockForwardFinish(void)
{
    movtConfig[MOVT_M_CLOCK].currentPosition ++;
    if(movtConfig[MOVT_M_CLOCK].currentPosition == movtConfig[MOVT_M_CLOCK].movtRangeMax)
    {
        movtConfig[MOVT_M_CLOCK].currentPosition = 0;
    }

    Drv_Movt_ClockCompare(&movtConfig[MOVT_M_CLOCK]);
}


void Drv_Movt_MClockReverseFinish(void)
{
    if(movtConfig[MOVT_M_CLOCK].currentPosition > 0)
    {
        movtConfig[MOVT_M_CLOCK].currentPosition--;
    }
    else
    {
        movtConfig[MOVT_M_CLOCK].currentPosition = movtConfig[MOVT_M_CLOCK].movtRangeMax - 1;
    }

    Drv_Movt_ClockCompare(&movtConfig[MOVT_M_CLOCK]);
}


void Drv_Movt_3ClockForwardFinish(void)
{
	#if((MOVT_3_A_PIN != IO_UNKNOW) && (MOVT_3_B_PIN != IO_UNKNOW))
    movtConfig[MOVT_3_CLOCK].currentPosition ++;
    if(movtConfig[MOVT_3_CLOCK].currentPosition == movtConfig[MOVT_3_CLOCK].movtRangeMax)
    {
        movtConfig[MOVT_3_CLOCK].currentPosition = 0;
    }

    Drv_Movt_ClockCompare(&movtConfig[MOVT_3_CLOCK]);
	#endif
}


void Drv_Movt_3ClockReverseFinish(void)
{
	#if((MOVT_3_A_PIN != IO_UNKNOW) && (MOVT_3_B_PIN != IO_UNKNOW))
    if(movtConfig[MOVT_3_CLOCK].currentPosition > 0)
    {
        movtConfig[MOVT_3_CLOCK].currentPosition--;
    }
    else
    {
        movtConfig[MOVT_3_CLOCK].currentPosition = movtConfig[MOVT_3_CLOCK].movtRangeMax - 1;
    }

    Drv_Movt_ClockCompare(&movtConfig[MOVT_3_CLOCK]);
	#endif
}

void Drv_Movt_6ClockForwardFinish(void)
{
	#if((MOVT_6_A_PIN != IO_UNKNOW) && (MOVT_6_B_PIN != IO_UNKNOW))
    movtConfig[MOVT_6_CLOCK].currentPosition ++;
    if(movtConfig[MOVT_6_CLOCK].currentPosition == movtConfig[MOVT_6_CLOCK].movtRangeMax)
    {
        movtConfig[MOVT_6_CLOCK].currentPosition = 0;
    }

    Drv_Movt_ClockCompare(&movtConfig[MOVT_6_CLOCK]);
	#endif
}


void Drv_Movt_6ClockReverseFinish(void)
{
	#if((MOVT_6_A_PIN != IO_UNKNOW) && (MOVT_6_B_PIN != IO_UNKNOW))
    if(movtConfig[MOVT_6_CLOCK].currentPosition > 0)
    {
        movtConfig[MOVT_6_CLOCK].currentPosition--;
    }
    else
    {
        movtConfig[MOVT_6_CLOCK].currentPosition = movtConfig[MOVT_6_CLOCK].movtRangeMax - 1;
    }

    Drv_Movt_ClockCompare(&movtConfig[MOVT_6_CLOCK]);
	#endif
}


void Drv_Movt_9ClockForwardFinish(void)
{
	#if((MOVT_9_A_PIN != IO_UNKNOW) && (MOVT_9_B_PIN != IO_UNKNOW))
    movtConfig[MOVT_9_CLOCK].currentPosition ++;
    if(movtConfig[MOVT_9_CLOCK].currentPosition == movtConfig[MOVT_9_CLOCK].movtRangeMax)
    {
        movtConfig[MOVT_9_CLOCK].currentPosition = 0;
    }

    Drv_Movt_ClockCompare(&movtConfig[MOVT_9_CLOCK]);
	#endif
}


void Drv_Movt_9ClockReverseFinish(void)
{
	#if((MOVT_9_A_PIN != IO_UNKNOW) && (MOVT_9_B_PIN != IO_UNKNOW))
    if(movtConfig[MOVT_9_CLOCK].currentPosition > 0)
    {
        movtConfig[MOVT_9_CLOCK].currentPosition--;
    }
    else
    {
        movtConfig[MOVT_9_CLOCK].currentPosition = movtConfig[MOVT_9_CLOCK].movtRangeMax - 1;
    }

    Drv_Movt_ClockCompare(&movtConfig[MOVT_9_CLOCK]);
	#endif
}



void Drv_Movt_Forward(movt_num_t movtId)
{
    switch(movtId)
    {
        case MOVT_M_CLOCK:
        movtConfig[MOVT_M_CLOCK].aimPosition++;

        if(movtConfig[MOVT_M_CLOCK].aimPosition >= movtConfig[MOVT_M_CLOCK].movtRangeMax)
            movtConfig[MOVT_M_CLOCK].aimPosition    = 0;

        movtConfig[MOVT_M_CLOCK].directionSet       = MOVT_DIR_AUTO;
        movtConfig[MOVT_M_CLOCK].repeat             = 0;
    //                movtConfig[MOVT_M_CLOCK].stop               = 0;
        Drv_Movt_ClockCompare(&movtConfig[MOVT_M_CLOCK]);
        break;
#if((MOVT_3_A_PIN != IO_UNKNOW) && (MOVT_3_B_PIN != IO_UNKNOW))	
        case MOVT_3_CLOCK:
        movtConfig[MOVT_3_CLOCK].aimPosition++;

        if(movtConfig[MOVT_3_CLOCK].aimPosition >= movtConfig[MOVT_3_CLOCK].movtRangeMax)
            movtConfig[MOVT_3_CLOCK].aimPosition    = 0;

        movtConfig[MOVT_3_CLOCK].directionSet       = MOVT_DIR_AUTO;
        movtConfig[MOVT_3_CLOCK].repeat         = 0;
//                movtConfig[MOVT_3_CLOCK].stop               = 0;
        Drv_Movt_ClockCompare(&movtConfig[MOVT_3_CLOCK]);

        break;
#endif
		
#if((MOVT_6_A_PIN != IO_UNKNOW) && (MOVT_6_B_PIN != IO_UNKNOW))
        case MOVT_6_CLOCK:
        movtConfig[MOVT_6_CLOCK].aimPosition++;

        if(movtConfig[MOVT_6_CLOCK].aimPosition >= movtConfig[MOVT_6_CLOCK].movtRangeMax)
            movtConfig[MOVT_6_CLOCK].aimPosition    = 0;

        movtConfig[MOVT_6_CLOCK].directionSet       = MOVT_DIR_AUTO;
        movtConfig[MOVT_6_CLOCK].repeat         = 0;
    //                movtConfig[MOVT_6_CLOCK].stop               = 0;
        Drv_Movt_ClockCompare(&movtConfig[MOVT_6_CLOCK]);

        break;
#endif
		
#if((MOVT_9_A_PIN != IO_UNKNOW) && (MOVT_9_B_PIN != IO_UNKNOW))		
        case MOVT_9_CLOCK:
        movtConfig[MOVT_9_CLOCK].aimPosition++;

        if(movtConfig[MOVT_9_CLOCK].aimPosition >= movtConfig[MOVT_9_CLOCK].movtRangeMax)
            movtConfig[MOVT_9_CLOCK].aimPosition    = 0;

        movtConfig[MOVT_9_CLOCK].directionSet       = MOVT_DIR_AUTO;
        movtConfig[MOVT_9_CLOCK].repeat         = 0;
//                movtConfig[MOVT_9_CLOCK].stop               = 0;
        Drv_Movt_ClockCompare(bsp_movt.clock9);

        break;
#endif
        default:break;
    }  
}

void Drv_Movt_Reverse(movt_num_t movtId)
{
    switch(movtId)
    {
        case MOVT_M_CLOCK:
          if(movtConfig[MOVT_M_CLOCK].aimPosition == 0)
            movtConfig[MOVT_M_CLOCK].aimPosition = movtConfig[MOVT_M_CLOCK].movtRangeMax - 1;
        else
            movtConfig[MOVT_M_CLOCK].aimPosition--;

        movtConfig[MOVT_M_CLOCK].directionSet       = MOVT_DIR_AUTO;
        movtConfig[MOVT_M_CLOCK].repeat             = 0;
       //movtConfig[MOVT_M_CLOCK].stop              = 0;
        Drv_Movt_ClockCompare(&movtConfig[MOVT_M_CLOCK]);
        break;

#if((MOVT_3_A_PIN != IO_UNKNOW) && (MOVT_3_B_PIN != IO_UNKNOW))
        case MOVT_3_CLOCK:
        if(movtConfig[MOVT_3_CLOCK].aimPosition == 0)
            movtConfig[MOVT_3_CLOCK].aimPosition = movtConfig[MOVT_3_CLOCK].movtRangeMax - 1;
        else
            movtConfig[MOVT_3_CLOCK].aimPosition--;

        movtConfig[MOVT_3_CLOCK].directionSet       = MOVT_DIR_AUTO;
        movtConfig[MOVT_3_CLOCK].repeat         = 0;
//                movtConfig[MOVT_3_CLOCK].stop               = 0;
        Drv_Movt_ClockCompare(&movtConfig[MOVT_3_CLOCK]);
        break;
#endif

#if((MOVT_6_A_PIN != IO_UNKNOW) && (MOVT_6_B_PIN != IO_UNKNOW))		
        case MOVT_6_CLOCK:
        if(movtConfig[MOVT_6_CLOCK].aimPosition == 0)
            movtConfig[MOVT_6_CLOCK].aimPosition = movtConfig[MOVT_6_CLOCK].movtRangeMax - 1;
        else
            movtConfig[MOVT_6_CLOCK].aimPosition--;

        movtConfig[MOVT_6_CLOCK].directionSet       = MOVT_DIR_AUTO;
        movtConfig[MOVT_6_CLOCK].repeat         = 0;
//                movtConfig[MOVT_6_CLOCK].stop               = 0;
        Drv_Movt_ClockCompare(&movtConfig[MOVT_6_CLOCK]);
        break;
#endif

#if((MOVT_9_A_PIN != IO_UNKNOW) && (MOVT_9_B_PIN != IO_UNKNOW))		
        case MOVT_9_CLOCK:
        if(movtConfig[MOVT_9_CLOCK].aimPosition == 0)
            movtConfig[MOVT_9_CLOCK].aimPosition = movtConfig[MOVT_9_CLOCK].movtRangeMax - 1;
        else
            movtConfig[MOVT_9_CLOCK].aimPosition--;

        movtConfig[MOVT_9_CLOCK].directionSet       = MOVT_DIR_AUTO;
        movtConfig[MOVT_9_CLOCK].repeat         = 0;
//                movtConfig[MOVT_9_CLOCK].stop               = 0;
        Drv_Movt_ClockCompare(bsp_movt.clock9);
        break;
#endif
		
        default:break;
    }
  
}

void Drv_Movt_Forwarding(movt_num_t movtId)
{
    switch(movtId)
    {
        case MOVT_M_CLOCK:
        movtConfig[MOVT_M_CLOCK].directionSet       = MOVT_DIR_SETTING;
        movtConfig[MOVT_M_CLOCK].tempDirection      = MOVT_DIR_FORWARD;
        movtConfig[MOVT_M_CLOCK].repeat             = 1;
        // movtConfig[MOVT_M_CLOCK].stop               = 0;
        Drv_Movt_ClockCompare(&movtConfig[MOVT_M_CLOCK]);

        break;

#if((MOVT_3_A_PIN != IO_UNKNOW) && (MOVT_3_B_PIN != IO_UNKNOW))
        case MOVT_3_CLOCK:
        movtConfig[MOVT_3_CLOCK].directionSet       = MOVT_DIR_SETTING;
        movtConfig[MOVT_3_CLOCK].tempDirection      = MOVT_DIR_FORWARD;
        movtConfig[MOVT_3_CLOCK].repeat         = 1;
//      movtConfig[MOVT_3_CLOCK].stop               = 0;
        Drv_Movt_ClockCompare(&movtConfig[MOVT_3_CLOCK]);
        break;
#endif

#if((MOVT_6_A_PIN != IO_UNKNOW) && (MOVT_6_B_PIN != IO_UNKNOW))		
        case MOVT_6_CLOCK:
        movtConfig[MOVT_6_CLOCK].directionSet       = MOVT_DIR_SETTING;
        movtConfig[MOVT_6_CLOCK].tempDirection      = MOVT_DIR_FORWARD;
        movtConfig[MOVT_6_CLOCK].repeat         = 1;
//      movtConfig[MOVT_6_CLOCK].stop               = 0;
        Drv_Movt_ClockCompare(&movtConfig[MOVT_6_CLOCK]);
        break;
#endif
		
#if((MOVT_9_A_PIN != IO_UNKNOW) && (MOVT_9_B_PIN != IO_UNKNOW))		
        case MOVT_9_CLOCK:
        movtConfig[MOVT_9_CLOCK].directionSet       = MOVT_DIR_SETTING;
        movtConfig[MOVT_9_CLOCK].tempDirection      = MOVT_DIR_FORWARD;
        movtConfig[MOVT_9_CLOCK].repeat         = 1;
//                movtConfig[MOVT_9_CLOCK].stop               = 0;
        Drv_Movt_ClockCompare(bsp_movt.clock9);
        break;
#endif
        default:break;

    }
}

void Drv_Movt_Reversing(movt_num_t movtId)
{
    switch(movtId)
    {
        case MOVT_M_CLOCK:
         movtConfig[MOVT_M_CLOCK].directionSet       = MOVT_DIR_SETTING;
        movtConfig[MOVT_M_CLOCK].tempDirection      = MOVT_DIR_REVERSE;
        movtConfig[MOVT_M_CLOCK].repeat             = 1;
    //  movtConfig[MOVT_M_CLOCK].stop               = 0;
        Drv_Movt_ClockCompare(&movtConfig[MOVT_M_CLOCK]);
        break;
		
#if((MOVT_3_A_PIN != IO_UNKNOW) && (MOVT_3_B_PIN != IO_UNKNOW))
        case MOVT_3_CLOCK:
        movtConfig[MOVT_3_CLOCK].directionSet       = MOVT_DIR_SETTING;
        movtConfig[MOVT_3_CLOCK].tempDirection      = MOVT_DIR_REVERSE;
        movtConfig[MOVT_3_CLOCK].repeat         = 1;
//                movtConfig[MOVT_3_CLOCK].stop               = 0;
        Drv_Movt_ClockCompare(&movtConfig[MOVT_3_CLOCK]);
        break;
#endif

#if((MOVT_6_A_PIN != IO_UNKNOW) && (MOVT_6_B_PIN != IO_UNKNOW))
        case MOVT_6_CLOCK:
        movtConfig[MOVT_6_CLOCK].directionSet       = MOVT_DIR_SETTING;
        movtConfig[MOVT_6_CLOCK].tempDirection      = MOVT_DIR_REVERSE;
        movtConfig[MOVT_6_CLOCK].repeat         = 1;
//                movtConfig[MOVT_6_CLOCK].stop               = 0;
        Drv_Movt_ClockCompare(&movtConfig[MOVT_6_CLOCK]);
        break;
#endif

#if((MOVT_9_A_PIN != IO_UNKNOW) && (MOVT_9_B_PIN != IO_UNKNOW))		
        case MOVT_9_CLOCK:
        movtConfig[MOVT_9_CLOCK].directionSet       = MOVT_DIR_SETTING;
        movtConfig[MOVT_9_CLOCK].tempDirection      = MOVT_DIR_REVERSE;
        movtConfig[MOVT_9_CLOCK].repeat         = 1;
//                movtConfig[MOVT_9_CLOCK].stop               = 0;
        Drv_Movt_ClockCompare(bsp_movt.clock9);
        break;
#endif
        default:break;
    }  
}

void Drv_Movt_SetCurAction(movt_num_t movtId,uint16 actionId,uint16 curval)
{  
    switch(movtId)
    {
        case MOVT_M_CLOCK:
        if(actionId == MOVT_SET_MC_SET_CUR)
        {
            movtConfig[MOVT_M_CLOCK].directionSet       = MOVT_DIR_AUTO;
        }
        else
        {
            movtConfig[MOVT_M_CLOCK].directionSet       = MOVT_DIR_SETTING;
            if(actionId == MOVT_SET_MC_SET_CUR_FORWARD)
                movtConfig[MOVT_M_CLOCK].tempDirection      = MOVT_DIR_FORWARD;
            else
                movtConfig[MOVT_M_CLOCK].tempDirection      = MOVT_DIR_REVERSE;
        }

        movtConfig[MOVT_M_CLOCK].currentPosition    = curval;
        if(movtConfig[MOVT_M_CLOCK].currentPosition >= movtConfig[MOVT_M_CLOCK].movtRangeMax)
            movtConfig[MOVT_M_CLOCK].currentPosition    = movtConfig[MOVT_M_CLOCK].currentPosition%movtConfig[MOVT_M_CLOCK].movtRangeMax;
        movtConfig[MOVT_M_CLOCK].repeat             = 0;
        //                movtConfig[MOVT_M_CLOCK].stop               = 0;
        Drv_Movt_ClockCompare(&movtConfig[MOVT_M_CLOCK]);
        break;

#if((MOVT_3_A_PIN != IO_UNKNOW) && (MOVT_3_B_PIN != IO_UNKNOW))
        case MOVT_3_CLOCK:
        if(actionId == MOVT_SET_MC_SET_CUR)
        {
            movtConfig[MOVT_3_CLOCK].directionSet       = MOVT_DIR_AUTO;
        }
        else
        {
            movtConfig[MOVT_3_CLOCK].directionSet       = MOVT_DIR_SETTING;
            if(actionId == MOVT_SET_3C_SET_CUR_FORWARD)
                movtConfig[MOVT_3_CLOCK].tempDirection      = MOVT_DIR_FORWARD;
            else
                movtConfig[MOVT_3_CLOCK].tempDirection      = MOVT_DIR_REVERSE;
        }


        movtConfig[MOVT_3_CLOCK].currentPosition        = curval;
        if(movtConfig[MOVT_3_CLOCK].currentPosition >= movtConfig[MOVT_3_CLOCK].movtRangeMax)
            movtConfig[MOVT_3_CLOCK].currentPosition    = movtConfig[MOVT_3_CLOCK].currentPosition%movtConfig[MOVT_3_CLOCK].movtRangeMax;
        movtConfig[MOVT_3_CLOCK].repeat             = 0;
//                movtConfig[MOVT_3_CLOCK].stop               = 0;
        Drv_Movt_ClockCompare(&movtConfig[MOVT_3_CLOCK]);
        break;
#endif
		
#if((MOVT_6_A_PIN != IO_UNKNOW) && (MOVT_6_B_PIN != IO_UNKNOW))		
        case MOVT_6_CLOCK:
        if(actionId == MOVT_SET_6C_SET_CUR)
        {
            movtConfig[MOVT_6_CLOCK].directionSet       = MOVT_DIR_AUTO;
        }
        else
        {
            movtConfig[MOVT_6_CLOCK].directionSet       = MOVT_DIR_SETTING;
            if(actionId == MOVT_SET_6C_SET_CUR_FORWARD)
                movtConfig[MOVT_6_CLOCK].tempDirection      = MOVT_DIR_FORWARD;
            else
                movtConfig[MOVT_6_CLOCK].tempDirection      = MOVT_DIR_REVERSE;
        }

        movtConfig[MOVT_6_CLOCK].currentPosition        = curval;
        if(movtConfig[MOVT_6_CLOCK].currentPosition >= movtConfig[MOVT_6_CLOCK].movtRangeMax)
            movtConfig[MOVT_6_CLOCK].currentPosition    = movtConfig[MOVT_6_CLOCK].currentPosition%movtConfig[MOVT_6_CLOCK].movtRangeMax;
        movtConfig[MOVT_6_CLOCK].repeat         = 0;
        //                movtConfig[MOVT_6_CLOCK].stop               = 0;
        Drv_Movt_ClockCompare(&movtConfig[MOVT_6_CLOCK]);
        break;
#endif

#if((MOVT_9_A_PIN != IO_UNKNOW) && (MOVT_9_B_PIN != IO_UNKNOW))		
        case MOVT_9_CLOCK:
        if(actionId == MOVT_SET_9C_SET_CUR)
        {
            movtConfig[MOVT_9_CLOCK].directionSet       = MOVT_DIR_AUTO;
        }
        else
        {
            movtConfig[MOVT_9_CLOCK].directionSet       = MOVT_DIR_SETTING;
            if(actionId == MOVT_SET_9C_SET_CUR_FORWARD)
                movtConfig[MOVT_9_CLOCK].tempDirection      = MOVT_DIR_FORWARD;
            else
                movtConfig[MOVT_9_CLOCK].tempDirection      = MOVT_DIR_REVERSE;
        }
        
        movtConfig[MOVT_9_CLOCK].currentPosition        = curval;
        if(movtConfig[MOVT_9_CLOCK].currentPosition >= movtConfig[MOVT_9_CLOCK].movtRangeMax)
            movtConfig[MOVT_9_CLOCK].currentPosition    = movtConfig[MOVT_9_CLOCK].currentPosition%movtConfig[MOVT_9_CLOCK].movtRangeMax;
        movtConfig[MOVT_9_CLOCK].repeat         = 0;
//                movtConfig[MOVT_9_CLOCK].stop               = 0;
        Drv_Movt_ClockCompare(bsp_movt.clock9);
        break;
#endif
        default:break;
    }
}

void Drv_Movt_SetAimAction(movt_num_t movtId,uint16 actionId,uint16 aimval)
{
    switch(movtId)
    {
        case MOVT_M_CLOCK:
        if(actionId == MOVT_SET_MC_SET_AIM)
        {
            movtConfig[movtId].directionSet       = MOVT_DIR_AUTO;
        }
        else
        {
            movtConfig[movtId].directionSet       = MOVT_DIR_SETTING;
            if(actionId == MOVT_SET_MC_SET_AIM_FORWARD)
                movtConfig[movtId].tempDirection      = MOVT_DIR_FORWARD;
            else
                movtConfig[movtId].tempDirection      = MOVT_DIR_REVERSE;
        }

        movtConfig[movtId].aimPosition        = aimval;
        if(movtConfig[movtId].aimPosition >= movtConfig[movtId].movtRangeMax)
            movtConfig[movtId].aimPosition    = movtConfig[movtId].aimPosition % movtConfig[movtId].movtRangeMax;

        movtConfig[movtId].repeat             = 0;
        //                movtConfig[MOVT_M_CLOCK].stop               = 0;
        Drv_Movt_ClockCompare(&movtConfig[movtId]);
        break;

#if((MOVT_3_A_PIN != IO_UNKNOW) && (MOVT_3_B_PIN != IO_UNKNOW))
        case MOVT_3_CLOCK:
        if(actionId == MOVT_SET_3C_SET_AIM)
        {
            movtConfig[MOVT_3_CLOCK].directionSet       = MOVT_DIR_AUTO;
        }
        else
        {
            movtConfig[MOVT_3_CLOCK].directionSet       = MOVT_DIR_SETTING;
            if(actionId == MOVT_SET_3C_SET_AIM_FORWARD)
                movtConfig[MOVT_3_CLOCK].tempDirection      = MOVT_DIR_FORWARD;
            else
                movtConfig[MOVT_3_CLOCK].tempDirection      = MOVT_DIR_REVERSE;
        }

        movtConfig[MOVT_3_CLOCK].aimPosition        = aimval;
        if(movtConfig[MOVT_3_CLOCK].aimPosition >= movtConfig[MOVT_3_CLOCK].movtRangeMax)
            movtConfig[MOVT_3_CLOCK].aimPosition    = movtConfig[MOVT_3_CLOCK].aimPosition%movtConfig[MOVT_3_CLOCK].movtRangeMax;
        movtConfig[MOVT_3_CLOCK].repeat             = 0;
//                movtConfig[MOVT_3_CLOCK].stop               = 0;
        Drv_Movt_ClockCompare(&movtConfig[MOVT_3_CLOCK]);
        break;
#endif

#if((MOVT_6_A_PIN != IO_UNKNOW) && (MOVT_6_B_PIN != IO_UNKNOW))		
        case MOVT_6_CLOCK:
        if(actionId == MOVT_SET_6C_SET_AIM)
        {
            movtConfig[MOVT_6_CLOCK].directionSet       = MOVT_DIR_AUTO;
        }
        else
        {
            movtConfig[MOVT_6_CLOCK].directionSet       = MOVT_DIR_SETTING;
            if(actionId == MOVT_SET_6C_SET_AIM_FORWARD)
                movtConfig[MOVT_6_CLOCK].tempDirection      = MOVT_DIR_FORWARD;
            else
                movtConfig[MOVT_6_CLOCK].tempDirection      = MOVT_DIR_REVERSE;
        }


        movtConfig[MOVT_6_CLOCK].aimPosition        = aimval;
        if(movtConfig[MOVT_6_CLOCK].aimPosition >= movtConfig[MOVT_6_CLOCK].movtRangeMax)
            movtConfig[MOVT_6_CLOCK].aimPosition    = movtConfig[MOVT_6_CLOCK].aimPosition%movtConfig[MOVT_6_CLOCK].movtRangeMax;
        movtConfig[MOVT_6_CLOCK].repeat             = 0;
//                movtConfig[MOVT_6_CLOCK].stop               = 0;
        Drv_Movt_ClockCompare(&movtConfig[MOVT_6_CLOCK]);
        break;
#endif
		
#if((MOVT_9_A_PIN != IO_UNKNOW) && (MOVT_9_B_PIN != IO_UNKNOW))
        case MOVT_9_CLOCK:
        if(actionId == MOVT_SET_9C_SET_AIM)
        {
            movtConfig[MOVT_9_CLOCK].directionSet       = MOVT_DIR_AUTO;
        }
        else
        {
            movtConfig[MOVT_9_CLOCK].directionSet       = MOVT_DIR_SETTING;
            if(actionId == MOVT_SET_9C_SET_AIM_FORWARD)
                movtConfig[MOVT_9_CLOCK].tempDirection      = MOVT_DIR_FORWARD;
            else
                movtConfig[MOVT_9_CLOCK].tempDirection      = MOVT_DIR_REVERSE;
        }

        movtConfig[MOVT_9_CLOCK].aimPosition        = aimval;
        if(movtConfig[MOVT_9_CLOCK].aimPosition >= movtConfig[MOVT_9_CLOCK].movtRangeMax)
            movtConfig[MOVT_9_CLOCK].aimPosition    = movtConfig[MOVT_9_CLOCK].aimPosition%movtConfig[MOVT_9_CLOCK].movtRangeMax;
        movtConfig[MOVT_9_CLOCK].repeat         = 0;
//      movtConfig[MOVT_9_CLOCK].stop               = 0;
        Drv_Movt_ClockCompare(bsp_movt.clock9);
        break;
#endif
		
        default:break;

    }

    
}

void Drv_Movt_SetCurAimAction(movt_num_t movtId,uint16 actionId,uint16 curval,uint16 aimval)
{
    switch(movtId)
    {
        case MOVT_M_CLOCK:
         if(actionId == MOVT_SET_MC_SET_CUR_AIM)
        {
            movtConfig[MOVT_M_CLOCK].directionSet       = MOVT_DIR_AUTO;
        }
        else
        {
            movtConfig[MOVT_M_CLOCK].directionSet       = MOVT_DIR_SETTING;
            if(actionId == MOVT_SET_MC_SET_CUR_AIM_FORWARD)
                movtConfig[MOVT_M_CLOCK].tempDirection      = MOVT_DIR_FORWARD;
            else
                movtConfig[MOVT_M_CLOCK].tempDirection      = MOVT_DIR_REVERSE;
        }

        movtConfig[MOVT_M_CLOCK].currentPosition        = curval;
        movtConfig[MOVT_M_CLOCK].aimPosition            = aimval;
        if(movtConfig[MOVT_M_CLOCK].currentPosition >= movtConfig[MOVT_M_CLOCK].movtRangeMax)
            movtConfig[MOVT_M_CLOCK].currentPosition    = movtConfig[MOVT_M_CLOCK].currentPosition%movtConfig[MOVT_M_CLOCK].movtRangeMax;

        if(movtConfig[MOVT_M_CLOCK].aimPosition >= movtConfig[MOVT_M_CLOCK].movtRangeMax)
            movtConfig[MOVT_M_CLOCK].aimPosition    = movtConfig[MOVT_M_CLOCK].aimPosition%movtConfig[MOVT_M_CLOCK].movtRangeMax;


        movtConfig[MOVT_M_CLOCK].repeat             = 0;
    //                movtConfig[MOVT_M_CLOCK].stop               = 0;
        Drv_Movt_ClockCompare(&movtConfig[MOVT_M_CLOCK]);
        break;

#if((MOVT_3_A_PIN != IO_UNKNOW) && (MOVT_3_B_PIN != IO_UNKNOW))
        case MOVT_3_CLOCK:
        if(actionId == MOVT_SET_3C_SET_CUR_AIM)
        {
            movtConfig[MOVT_3_CLOCK].directionSet       = MOVT_DIR_AUTO;
        }
        else
        {
            movtConfig[MOVT_3_CLOCK].directionSet       = MOVT_DIR_SETTING;
            if(actionId == MOVT_SET_3C_SET_CUR_AIM_FORWARD)
                movtConfig[MOVT_3_CLOCK].tempDirection      = MOVT_DIR_FORWARD;
            else
                movtConfig[MOVT_3_CLOCK].tempDirection      = MOVT_DIR_REVERSE;
        }

        movtConfig[MOVT_3_CLOCK].currentPosition        = curval;
        movtConfig[MOVT_3_CLOCK].aimPosition            = aimval;

        if(movtConfig[MOVT_3_CLOCK].aimPosition >= movtConfig[MOVT_3_CLOCK].movtRangeMax)
            movtConfig[MOVT_3_CLOCK].aimPosition    = movtConfig[MOVT_3_CLOCK].aimPosition%movtConfig[MOVT_3_CLOCK].movtRangeMax;

        if(movtConfig[MOVT_3_CLOCK].currentPosition >= movtConfig[MOVT_3_CLOCK].movtRangeMax)
            movtConfig[MOVT_3_CLOCK].currentPosition    = movtConfig[MOVT_3_CLOCK].currentPosition%movtConfig[MOVT_3_CLOCK].movtRangeMax;

        movtConfig[MOVT_3_CLOCK].repeat         = 0;
//                movtConfig[MOVT_3_CLOCK].stop               = 0;
        Drv_Movt_ClockCompare(&movtConfig[MOVT_3_CLOCK]);
        break;
#endif

#if((MOVT_6_A_PIN != IO_UNKNOW) && (MOVT_6_B_PIN != IO_UNKNOW))		
         case MOVT_6_CLOCK:
         if(actionId == MOVT_SET_6C_SET_CUR_AIM)
                {
                    movtConfig[MOVT_6_CLOCK].directionSet       = MOVT_DIR_AUTO;
                }
                else
                {
                    movtConfig[MOVT_6_CLOCK].directionSet       = MOVT_DIR_SETTING;
                    if(actionId == MOVT_SET_6C_SET_CUR_AIM_FORWARD)
                        movtConfig[MOVT_6_CLOCK].tempDirection      = MOVT_DIR_FORWARD;
                    else
                        movtConfig[MOVT_6_CLOCK].tempDirection      = MOVT_DIR_REVERSE;
                }

                movtConfig[MOVT_6_CLOCK].currentPosition        = curval;
                movtConfig[MOVT_6_CLOCK].aimPosition            = aimval;

                if(movtConfig[MOVT_6_CLOCK].aimPosition >= movtConfig[MOVT_6_CLOCK].movtRangeMax)
                    movtConfig[MOVT_6_CLOCK].aimPosition    = movtConfig[MOVT_6_CLOCK].aimPosition%movtConfig[MOVT_6_CLOCK].movtRangeMax;

                if(movtConfig[MOVT_6_CLOCK].currentPosition >= movtConfig[MOVT_6_CLOCK].movtRangeMax)
                    movtConfig[MOVT_6_CLOCK].currentPosition    = movtConfig[MOVT_6_CLOCK].currentPosition%movtConfig[MOVT_6_CLOCK].movtRangeMax;

                movtConfig[MOVT_6_CLOCK].repeat         = 0;
//                movtConfig[MOVT_6_CLOCK].stop               = 0;
                Drv_Movt_ClockCompare(&movtConfig[MOVT_6_CLOCK]);
        break;
#endif

#if((MOVT_9_A_PIN != IO_UNKNOW) && (MOVT_9_B_PIN != IO_UNKNOW))
        case MOVT_9_CLOCK:
        if(actionId == MOVT_SET_9C_SET_CUR_AIM)
        {
            movtConfig[MOVT_9_CLOCK].directionSet       = MOVT_DIR_AUTO;
        }
        else
        {
            movtConfig[MOVT_9_CLOCK].directionSet       = MOVT_DIR_SETTING;
            if(actionId == MOVT_SET_9C_SET_CUR_AIM_FORWARD)
                movtConfig[MOVT_9_CLOCK].tempDirection      = MOVT_DIR_FORWARD;
            else
                movtConfig[MOVT_9_CLOCK].tempDirection      = MOVT_DIR_REVERSE;
        }

        movtConfig[MOVT_9_CLOCK].currentPosition        = curval;
        movtConfig[MOVT_9_CLOCK].aimPosition            = aimval;

        if(movtConfig[MOVT_9_CLOCK].aimPosition >= movtConfig[MOVT_9_CLOCK].movtRangeMax)
            movtConfig[MOVT_9_CLOCK].aimPosition    = movtConfig[MOVT_9_CLOCK].aimPosition%movtConfig[MOVT_9_CLOCK].movtRangeMax;

        if(movtConfig[MOVT_9_CLOCK].currentPosition >= movtConfig[MOVT_9_CLOCK].movtRangeMax)
            movtConfig[MOVT_9_CLOCK].currentPosition    = movtConfig[MOVT_9_CLOCK].currentPosition%movtConfig[MOVT_9_CLOCK].movtRangeMax;

        movtConfig[MOVT_9_CLOCK].repeat         = 0;
//                movtConfig[MOVT_9_CLOCK].stop               = 0;
        Drv_Movt_ClockCompare(bsp_movt.clock9);
        break;
#endif
		
        default:break;
    }

   

}

void Drv_Movt_SetStopAction(movt_num_t movtId)
{
    switch(movtId)
    {
        case MOVT_M_CLOCK:
        movtConfig[MOVT_M_CLOCK].stop               = 1;
        break;
#if((MOVT_3_A_PIN != IO_UNKNOW) && (MOVT_3_B_PIN != IO_UNKNOW))
         case MOVT_3_CLOCK:
         movtConfig[MOVT_3_CLOCK].stop              = 1;
        break;
#endif

#if((MOVT_6_A_PIN != IO_UNKNOW) && (MOVT_6_B_PIN != IO_UNKNOW))
        case MOVT_6_CLOCK:
        movtConfig[MOVT_6_CLOCK].stop               = 1;
        break;
#endif
		
#if((MOVT_9_A_PIN != IO_UNKNOW) && (MOVT_9_B_PIN != IO_UNKNOW))
        case MOVT_9_CLOCK:
        movtConfig[MOVT_9_CLOCK].stop               = 1;
        break;
#endif
        default:break;
    }
    
}

void Drv_Movt_SetRecoverAction(movt_num_t movtId,uint16 actionId)
{
    switch(movtId)
    {
        case MOVT_M_CLOCK:
        if(actionId == MOVT_SET_MC_RECOVER)
        {
			movtConfig[MOVT_M_CLOCK].directionSet       = MOVT_DIR_AUTO;
        }
        else
        {
            movtConfig[MOVT_M_CLOCK].directionSet       = MOVT_DIR_SETTING;
            if(actionId == MOVT_SET_MC_RECOVER_FORWARD)
                movtConfig[MOVT_M_CLOCK].tempDirection      = MOVT_DIR_FORWARD;
            else
                movtConfig[MOVT_M_CLOCK].tempDirection      = MOVT_DIR_REVERSE;
        }

        movtConfig[MOVT_M_CLOCK].stop               = 0;
        movtConfig[MOVT_M_CLOCK].repeat             = 0;
        Drv_Movt_ClockCompare(&movtConfig[MOVT_M_CLOCK]);
        break;
#if((MOVT_3_A_PIN != IO_UNKNOW) && (MOVT_3_B_PIN != IO_UNKNOW))
         case MOVT_3_CLOCK:
         if(actionId == MOVT_SET_3C_RECOVER)
                {
                    movtConfig[MOVT_3_CLOCK].directionSet       = MOVT_DIR_AUTO;
                }
                else
                {
                    movtConfig[MOVT_3_CLOCK].directionSet       = MOVT_DIR_SETTING;
                    if(actionId == MOVT_SET_3C_RECOVER_FORWARD)
                        movtConfig[MOVT_3_CLOCK].tempDirection      = MOVT_DIR_FORWARD;
                    else
                        movtConfig[MOVT_3_CLOCK].tempDirection      = MOVT_DIR_REVERSE;
                }

                movtConfig[MOVT_3_CLOCK].stop               = 0;
                movtConfig[MOVT_3_CLOCK].repeat             = 0;
                Drv_Movt_ClockCompare(&movtConfig[MOVT_3_CLOCK]);
        break;
#endif
				
#if((MOVT_6_A_PIN != IO_UNKNOW) && (MOVT_6_B_PIN != IO_UNKNOW))				
        case MOVT_6_CLOCK:
        if(actionId == MOVT_SET_6C_RECOVER)
        {
            movtConfig[MOVT_6_CLOCK].directionSet       = MOVT_DIR_AUTO;
        }
        else
        {
            movtConfig[MOVT_6_CLOCK].directionSet       = MOVT_DIR_SETTING;
            if(actionId == MOVT_SET_6C_RECOVER_FORWARD)
                movtConfig[MOVT_6_CLOCK].tempDirection      = MOVT_DIR_FORWARD;
            else
                movtConfig[MOVT_6_CLOCK].tempDirection      = MOVT_DIR_REVERSE;
        }

        movtConfig[MOVT_6_CLOCK].stop               = 0;
        movtConfig[MOVT_6_CLOCK].repeat             = 0;
        Drv_Movt_ClockCompare(&movtConfig[MOVT_6_CLOCK]);
        break;

#endif
		
#if((MOVT_9_A_PIN != IO_UNKNOW) && (MOVT_9_B_PIN != IO_UNKNOW))
        case MOVT_9_CLOCK:
        if(actionId == MOVT_SET_9C_RECOVER)
        {
            movtConfig[MOVT_9_CLOCK].directionSet       = MOVT_DIR_AUTO;
        }
        else
        {
            movtConfig[MOVT_9_CLOCK].directionSet       = MOVT_DIR_SETTING;
            if(actionId == MOVT_SET_9C_RECOVER_FORWARD)
                movtConfig[MOVT_9_CLOCK].tempDirection      = MOVT_DIR_FORWARD;
            else
                movtConfig[MOVT_9_CLOCK].tempDirection      = MOVT_DIR_REVERSE;
        }

        movtConfig[MOVT_9_CLOCK].stop               = 0;
        movtConfig[MOVT_9_CLOCK].repeat             = 0;
        Drv_Movt_ClockCompare(&movtConfig[MOVT_9_CLOCK]);
        break;

#endif
        default:break;
    }  
}

//**********************************************************************
// 函数功能: 表针自测
// 输入参数：无
// 返回参数：	
//  0x00: 操作成功
// 	0x01: 大表盘测试失败
// 	0x02: 3点位表盘测试失败
// 	0x04: 6点位表盘测试失败
// 	0x08: 9点位表盘测试失败
//**********************************************************************
uint8 Drv_Movt_SelfTest(void)
{
    uint16 ret = 0;
    uint16 pinLevel = 0;
    uint32 u32config;
 
    if(SMDrv_GPIO_InBitRead(MOVT_M_A_PIN) == SMDrv_GPIO_InBitRead(MOVT_M_B_PIN))
        if(SMDrv_GPIO_InBitRead(MOVT_M_A_PIN) == 1)
            pinLevel |= 0x01;

    if(SMDrv_GPIO_InBitRead(MOVT_3_A_PIN) == SMDrv_GPIO_InBitRead(MOVT_3_B_PIN))
        if(SMDrv_GPIO_InBitRead(MOVT_3_A_PIN) == 1)
            pinLevel |= 0x02;

    if(SMDrv_GPIO_InBitRead(MOVT_6_A_PIN) == SMDrv_GPIO_InBitRead(MOVT_6_B_PIN))
        if(SMDrv_GPIO_InBitRead(MOVT_6_A_PIN) == 1)
            pinLevel |= 0x04;

    if(SMDrv_GPIO_InBitRead(MOVT_9_A_PIN) == SMDrv_GPIO_InBitRead(MOVT_9_B_PIN))
        if(SMDrv_GPIO_InBitRead(MOVT_9_A_PIN) == 1)
            pinLevel |= 0x08;

    u32config = GPIO_PIN_INPUT | GPIO_PIN_PULL24K;
    SMDrv_GPIO_Open(MOVT_M_A_PIN, &u32config,NULL);
    SMDrv_GPIO_Open(MOVT_M_B_PIN, NULL,NULL);  //output
    SMDrv_GPIO_BitClear(MOVT_M_B_PIN);
#if((MOVT_3_A_PIN != IO_UNKNOW) && (MOVT_3_B_PIN != IO_UNKNOW))
    SMDrv_GPIO_Open(MOVT_3_A_PIN, &u32config,NULL);
    SMDrv_GPIO_Open(MOVT_3_B_PIN, NULL,NULL);
    SMDrv_GPIO_BitClear(MOVT_3_B_PIN);
#endif
#if((MOVT_6_A_PIN != IO_UNKNOW) && (MOVT_6_B_PIN != IO_UNKNOW))
    SMDrv_GPIO_Open(MOVT_6_A_PIN, &u32config,NULL);
    SMDrv_GPIO_Open(MOVT_6_B_PIN, NULL,NULL);
    SMDrv_GPIO_BitClear(MOVT_6_B_PIN);
#endif
#if((MOVT_9_A_PIN != IO_UNKNOW) && (MOVT_9_B_PIN != IO_UNKNOW))
    SMDrv_GPIO_Open(MOVT_9_A_PIN, AM_HAL_PIN_INPUT | AM_HAL_GPIO_PULL24K,NULL);
    SMDrv_GPIO_Open(MOVT_9_B_PIN, NULL,NULL);
    SMDrv_GPIO_BitClear(MOVT_9_B_PIN);
#endif

    // wait to read the pin
    SMDrv_SYS_DelayMs(1);

    if(SMDrv_GPIO_InBitRead(MOVT_M_A_PIN) != 0)
    {
        ret |= MOVT_M_ERROR;
    }
    SMDrv_GPIO_BitSet(MOVT_M_B_PIN);

#if((MOVT_3_A_PIN != IO_UNKNOW) && (MOVT_3_B_PIN != IO_UNKNOW))
    if(SMDrv_GPIO_InBitRead(MOVT_3_A_PIN) != 0)
    {
        ret |= MOVT_3_ERROR;
    }
    SMDrv_GPIO_BitSet(MOVT_3_B_PIN);
#endif

#if((MOVT_6_A_PIN != IO_UNKNOW) && (MOVT_6_B_PIN != IO_UNKNOW))
    if(SMDrv_GPIO_InBitRead(MOVT_6_A_PIN) != 0)
    {
        ret |= MOVT_6_ERROR;
    }
    SMDrv_GPIO_BitSet(MOVT_6_B_PIN);
#endif

#if((MOVT_9_A_PIN != IO_UNKNOW) && (MOVT_9_B_PIN != IO_UNKNOW))
    if(SMDrv_GPIO_InBitRead(MOVT_9_A_PIN) != 0)
    {
        ret |= MOVT_9_ERROR;
    }
    SMDrv_GPIO_BitSet(MOVT_9_B_PIN);
#endif    

    // wait to read the pin
    SMDrv_SYS_DelayMs(1);

    if(SMDrv_GPIO_InBitRead(MOVT_M_A_PIN) != 1)
    {
        ret |= MOVT_M_ERROR;
    }
 
#if((MOVT_3_A_PIN != IO_UNKNOW) && (MOVT_3_B_PIN != IO_UNKNOW))
    if(SMDrv_GPIO_InBitRead(MOVT_3_A_PIN) != 1)
    {
        ret |= MOVT_3_ERROR;
    }
#endif
#if((MOVT_6_A_PIN != IO_UNKNOW) && (MOVT_6_B_PIN != IO_UNKNOW))
    if(SMDrv_GPIO_InBitRead(MOVT_6_A_PIN) != 1)
    {
        ret |= MOVT_6_ERROR;
    }
#endif
#if((MOVT_9_A_PIN != IO_UNKNOW) && (MOVT_9_B_PIN != IO_UNKNOW))
    if(SMDrv_GPIO_InBitRead(MOVT_9_A_PIN) != 1)
    {
        ret |= MOVT_9_ERROR;
    }
#endif

    if(pinLevel & 0x01)
    {
        SMDrv_GPIO_BitSet(MOVT_M_A_PIN);
        SMDrv_GPIO_BitSet(MOVT_M_B_PIN);
    }
    else
    {
        SMDrv_GPIO_BitClear(MOVT_M_A_PIN);
        SMDrv_GPIO_BitClear(MOVT_M_B_PIN);
    }
#if((MOVT_3_A_PIN != IO_UNKNOW) && (MOVT_3_B_PIN != IO_UNKNOW))
    if(pinLevel & 0x02)
    {
        SMDrv_GPIO_BitSet(MOVT_3_A_PIN);
        SMDrv_GPIO_BitSet(MOVT_3_B_PIN);
    }
    else
    {
        SMDrv_GPIO_BitClear(MOVT_3_A_PIN);
        SMDrv_GPIO_BitClear(MOVT_3_B_PIN);
    }
#endif
#if((MOVT_6_A_PIN != IO_UNKNOW) && (MOVT_6_B_PIN != IO_UNKNOW))
    if(pinLevel & 0x04)
    {
        SMDrv_GPIO_BitSet(MOVT_6_A_PIN);
        SMDrv_GPIO_BitSet(MOVT_6_B_PIN);
    }
    else
    {
        SMDrv_GPIO_BitClear(MOVT_6_A_PIN);
        SMDrv_GPIO_BitClear(MOVT_6_B_PIN);
	}
#endif
#if((MOVT_9_A_PIN != IO_UNKNOW) && (MOVT_9_B_PIN != IO_UNKNOW))
    if(pinLevel & 0x08)
    {
        SMDrv_GPIO_BitSet(MOVT_9_A_PIN);
        SMDrv_GPIO_BitSet(MOVT_9_B_PIN);
    }
    else
    {
        SMDrv_GPIO_BitClear(MOVT_9_A_PIN);
        SMDrv_GPIO_BitClear(MOVT_9_B_PIN);
    }
#endif

    SMDrv_GPIO_Open(MOVT_M_A_PIN, NULL,NULL);
    SMDrv_GPIO_Open(MOVT_M_B_PIN, NULL,NULL);
#if((MOVT_3_A_PIN != IO_UNKNOW) && (MOVT_3_B_PIN != IO_UNKNOW))
    SMDrv_GPIO_Open(MOVT_3_A_PIN, NULL,NULL);
    SMDrv_GPIO_Open(MOVT_3_B_PIN, NULL,NULL);
#endif
#if((MOVT_6_A_PIN != IO_UNKNOW) && (MOVT_6_B_PIN != IO_UNKNOW))
    SMDrv_GPIO_Open(MOVT_6_A_PIN, NULL,NULL);
    SMDrv_GPIO_Open(MOVT_6_B_PIN, NULL,NULL);
#endif
#if((MOVT_9_A_PIN != IO_UNKNOW) && (MOVT_9_B_PIN != IO_UNKNOW))
    SMDrv_GPIO_Open(MOVT_9_A_PIN, NULL,NULL);
    SMDrv_GPIO_Open(MOVT_9_B_PIN, NULL,NULL);
#endif
    return ret;
}

#endif

