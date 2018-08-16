/**********************************************************************
**
**模块说明: 
**   
**
**********************************************************************/

#include "mid_motor.h"

#include "rtos.h"

#define MID_MOTOR_RTT_DEBUG_ON	1
#if MID_MOTOR_RTT_DEBUG_ON
#define MID_MOTOR_RTT_printf	SEGGER_RTT_printf
#else
#define MID_MOTOR_RTT_printf(...)
#endif

TimerHandle_t 	Mid_Motor_TimerHandle;
Mid_Motor_Param_t	Mid_Motor;

void Mid_MotorCallback(TimerHandle_t xTimer)
{
//	MID_MOTOR_RTT_printf(0,"Mid_MotorCallback \n");
	
	if(eMidMotorStateShakeOn == Mid_Motor.State)
	{
		if(++Mid_Motor.CntOnTick >= Mid_Motor.TotalOnTick)
		{
			Drv_Motor_Off();
			Mid_Motor.State = eMidMotorStateShakeOff;
		}
	}
	else if(eMidMotorStateShakeOff == Mid_Motor.State)
	{
		if(++Mid_Motor.CntOffTick >= Mid_Motor.TotalOffTick)
		{
			if(++Mid_Motor.CntCycleNum >= Mid_Motor.TotalCycleNum)
			{
				Mid_Motor_ShakeStop();
			}
			else
			{
				// 复位计数，开始下一周期的震动
				Mid_Motor.CntOffTick = 0;
				Mid_Motor.CntOnTick = 0;				
				
				Drv_Motor_On();
				Mid_Motor.State = eMidMotorStateShakeOn;				
			}
		}	
	}
	else
	{
		Mid_Motor_ShakeStop();
	}
}


//**********************************************************************
// 函数功能:	马达IO口初始化
// 输入参数：	
// 返回参数：
void Mid_Motor_Init(void)
{
	// 硬件初始化
	Drv_Motor_Init();
	
	// 默认参数初始化
	Mid_Motor.State = eMidMotorStateSleep;
	Mid_Motor_ParamSet(eMidMotorShake1Hz, 3);
	
	// 创建定时器
    Mid_Motor_TimerHandle=xTimerCreate((const char*		)"Mid_Motor",
									    (TickType_t			)(MID_MOTOR_TICK_MS/portTICK_PERIOD_MS),
							            (UBaseType_t		)pdTRUE,
							            (void*				)1,
							            (TimerCallbackFunction_t)Mid_MotorCallback); //周期定时器，周期1s(1000个时钟节拍)，周期模式		
}

//**********************************************************************
// 函数功能:	马达震动
// 输入参数：	
// 返回参数：
void Mid_Motor_On(void)
{
	Drv_Motor_On();
}

//**********************************************************************
// 函数功能:	马达停止震动
// 输入参数：	
// 返回参数：
void Mid_Motor_Off(void)
{
	Drv_Motor_Off();
}

//**********************************************************************
// 函数功能:	马达震动参数设置
// 输入参数：	Level 震动等级
//				CycleNum 循环次数
// 返回参数：
uint32_t Mid_Motor_ParamSet(eMidMotorShakeLevel Level, uint16_t CycleNum)
{
	if(0 == CycleNum) return Ret_InvalidParam;
	if(eMidMotorStateSleep != Mid_Motor.State)	return Ret_DeviceBusy;

	Mid_Motor.TotalCycleNum = CycleNum;
	
	switch (Level)
	{
		case eMidMotorShake1Hz:
			Mid_Motor.TotalOnTick = MID_MOTOR_1HZ_ON_TICK;
			Mid_Motor.TotalOffTick = MID_MOTOR_1HZ_OFF_TICK;
			break;
		case eMidMotorShake2Hz:
			Mid_Motor.TotalOnTick = MID_MOTOR_2HZ_ON_TICK;
			Mid_Motor.TotalOffTick = MID_MOTOR_2HZ_OFF_TICK;			
			break;
		case eMidMotorShake4Hz:
			Mid_Motor.TotalOnTick = MID_MOTOR_4HZ_ON_TICK;
			Mid_Motor.TotalOffTick = MID_MOTOR_4HZ_OFF_TICK;			
			break;
		default :
			break;
	}

	Mid_Motor.CntCycleNum = 0;
	Mid_Motor.CntOffTick = 0;
	Mid_Motor.CntOnTick = 0;
	
	return Ret_OK;
}

uint32_t Mid_Motor_ShakeStart(void)
{
	if(eMidMotorStateSleep != Mid_Motor.State)	return Ret_DeviceBusy;
	
	// 先复位马达震动所有参数
	Mid_Motor_ShakeStop();
	
	Drv_Motor_On();
	Mid_Motor.State = eMidMotorStateShakeOn;
	
	xTimerStart(Mid_Motor_TimerHandle, 3);
	
	return Ret_OK;
}

void Mid_Motor_ShakeStop(void)
{
	xTimerStop(Mid_Motor_TimerHandle, 3);
	
	Drv_Motor_Off();
	
	Mid_Motor.State = eMidMotorStateSleep;
	Mid_Motor.CntCycleNum = 0;
	Mid_Motor.CntOffTick = 0;
	Mid_Motor.CntOnTick = 0;	
}



