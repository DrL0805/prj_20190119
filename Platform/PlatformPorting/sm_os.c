/**********************************************************************
**
**模块说明: 对接OS接口
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.6.6  初版  ZSL  
**
**********************************************************************/
#include "platform_common.h"
#include "rtos.h"
#include "sm_os.h"

//**********************************************************************
// 函数功能:     获取OS Tick时钟
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
uint32 SMOS_GetTickCount(void)
{
    uint32 u32tick;
    if(xPortIsInsideInterrupt() == pdTRUE)    //在中断模式
    {
        u32tick = xTaskGetTickCountFromISR();
    }
    else
    {
        u32tick = xTaskGetTickCount();
    }
    return u32tick;
}

