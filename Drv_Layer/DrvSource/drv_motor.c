/**********************************************************************
**
**模块说明: 麦克风（硅麦）模块
**   
**
**********************************************************************/

#include "io_config.h"
#include "sm_gpio.h"

#include "drv_motor.h"


void Drv_Motor_Init(void)
{
    uint32 u32PinConfig = GPIO_PIN_OUTPUT;  //default set pin as output

    SMDrv_GPIO_BitClear(MOTOR_PIN);
    SMDrv_GPIO_Open(MOTOR_PIN,&u32PinConfig,NULL);
}

void Drv_Motor_On(void)
{
	SMDrv_GPIO_BitSet(MOTOR_PIN);
}

void Drv_Motor_Off(void)
{
	SMDrv_GPIO_BitClear(MOTOR_PIN);
}




