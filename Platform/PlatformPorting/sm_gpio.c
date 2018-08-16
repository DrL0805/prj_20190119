/**********************************************************************
**
**模块说明: 对接MCU GPIO驱动接口，并根据项目原理图作配置
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.10  初版  ZSL  
**   V2.0   2018.8.8   增加Apollo3 driver
**
**********************************************************************/
#define BAT_MODULE
//#define KEY_MODULE
#define HEART_RATE
#ifdef AM_PART_APOLLO3
#define FONT_MODULE
#define FLASH_MODULE
#else
#define BLE_MODULE
#endif

#include "string.h"
#include "io_config.h"
#include "sm_gpio.h"

#ifdef AM_PART_APOLLO3
//**********************************************************************
// 函数功能:    GPIO中断服务函数
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
void am_gpio_isr(void)
{
    uint64 u64PinIntStatus = 0x00;

    //step 1:Read and clear the GPIO interrupt status.
    am_hal_gpio_interrupt_status_get(true, &u64PinIntStatus);
    am_hal_gpio_interrupt_clear(u64PinIntStatus);
    
    //step 2: Call the individual pin interrupt handlers for any pin that triggered an interrupt.
    am_hal_gpio_interrupt_service(u64PinIntStatus);
}

//**********************************************************************
// 函数功能:  判断pin是否是SPI CS
// 输入参数： u32PinNum
// 返回参数： 
//**********************************************************************
static uint32 Smdrv_IsSpiCs(uint32 u32PinNum)
{
    if((u32PinNum == FONT_CS_PIN) || (u32PinNum == FLASH_CS_PIN))
        return TRUE;
    return FALSE;
}

//**********************************************************************
// 函数功能:    GPIO初始化
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
void SMDrv_GPIO_Init(void)
{

}

//**********************************************************************
// 函数功能: 根据GPIO module ID配置GPIO属性:输入/输出，若为中断，则设置
//           则设置中断服务程序
// 输入参数：u32PinNum: gpio num, 值参考io_config.h中IO定义
//    config_opt:设置GPIO属性，若在程序中改变GPIO属性时使用，如用GPIO模拟
//      IIC时，SDA脚一会为输入，一会为输出。
//      == NULL: 使用默认的属性配置， != NULL: 则使用传入的属性
//    g_callback:中断服务函数callback
// 返回参数: Ret_OK，Ret_InvalidParam
//**********************************************************************
ret_type SMDrv_GPIO_Open(uint32 u32PinNum,uint32 *config_opt,gpio_cb g_callback)
{
    am_hal_gpio_pincfg_t bfGpioCfg;

    if(u32PinNum >= IO_UNKNOW)
        return Ret_InvalidParam;

    //apollo3无需对spi cs脚单独配置，和spi一起配置
    if(Smdrv_IsSpiCs(u32PinNum))
        return Ret_OK;

    //step 1:如果配置为中断，设置中断服务程序
    if(g_callback != NULL)
    {
        am_hal_gpio_interrupt_register(u32PinNum,g_callback);
    }

    //step 2:配置GPIO口默认属性，根据原理图
    bfGpioCfg.uFuncSel = 3;  //IO口功能选择，=3是GPIO
    if((u32PinNum == KEY_S0_PIN) || (u32PinNum == KEY_S1_PIN) || (u32PinNum == KEY_S2_PIN))
    {
        bfGpioCfg.eIntDir    = AM_HAL_GPIO_PIN_INTDIR_HI2LO;   //下降沿触发	// AM_HAL_GPIO_PIN_INTDIR_LO2HI;//
        bfGpioCfg.eGPOutcfg  = AM_HAL_GPIO_PIN_OUTCFG_DISABLE; //输入
        bfGpioCfg.eGPInput   = AM_HAL_GPIO_PIN_INPUT_ENABLE;
		bfGpioCfg.ePullup    = AM_HAL_GPIO_PIN_PULLUP_WEAK;//AM_HAL_GPIO_PIN_PULLUP_24K;
    }
    else if(u32PinNum == HR_INT1_PIN)
    {
        bfGpioCfg.eIntDir    = AM_HAL_GPIO_PIN_INTDIR_LO2HI;   //上升沿触发
        bfGpioCfg.eGPOutcfg  = AM_HAL_GPIO_PIN_OUTCFG_DISABLE; //输入
        bfGpioCfg.eGPInput   = AM_HAL_GPIO_PIN_INPUT_ENABLE;
    }
    else if ((u32PinNum == HR_INT2_PIN) || (u32PinNum == BAT_CHG_PIN))
    {
        bfGpioCfg.eGPOutcfg  = AM_HAL_GPIO_PIN_OUTCFG_DISABLE; //输入
        bfGpioCfg.eGPInput   = AM_HAL_GPIO_PIN_INPUT_ENABLE;
        if(u32PinNum == BAT_CHG_PIN)
        {
            bfGpioCfg.ePullup    = AM_HAL_GPIO_PIN_PULLUP_6K;
        }
    }
    else             //GPIO设置为输出
    {
        bfGpioCfg.eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_2MA;
        bfGpioCfg.eGPOutcfg      = AM_HAL_GPIO_PIN_OUTCFG_PUSHPULL;
    }

    //step 3:设置pin脚输入输出，上拉属性
    if(config_opt != NULL)
    {
        //如果参数传入设置GPIO属性的参数，就使用传入的值，否则使用默认配置的值
    }    
    am_hal_gpio_pinconfig(u32PinNum, bfGpioCfg);

    //step 4:中断模式，enable中断
    if(g_callback != NULL)
    {
        am_hal_gpio_interrupt_clear(AM_HAL_GPIO_BIT(u32PinNum));
        am_hal_gpio_interrupt_enable(AM_HAL_GPIO_BIT(u32PinNum));
    }

    return Ret_OK;
}

//**********************************************************************
// 函数功能:  重新设置中断触发条件
// 输入参数： u32PinNum: gpio num,
//   u32pol:  =1: 上升沿，=0: 下降沿
// 返回参数： 无
//**********************************************************************
void SMDrv_GPIO_ReConfigIrq(uint32 u32PinNum,uint32 u32pol)
{
#if 0
    uint32 ui32Polarity;
    ui32Polarity = (u32pol == 1)?AM_HAL_GPIO_RISING:AM_HAL_GPIO_FALLING;
    am_hal_interrupt_disable(AM_HAL_INTERRUPT_GPIO);
    am_hal_gpio_int_disable(AM_HAL_GPIO_BIT(u32PinNum));
    am_hal_gpio_int_polarity_bit_set(u32PinNum,ui32Polarity);
    am_hal_gpio_int_clear(AM_HAL_GPIO_BIT(u32PinNum));
    am_hal_gpio_int_enable(AM_HAL_GPIO_BIT(u32PinNum));
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_GPIO);
#endif
}

//**********************************************************************
// 函数功能:  开启/关闭pin脚中断
// 输入参数： u32PinNum: gpio num,
// u8Enable:  =1: enable中断，=0: disable中断
// 返回参数： 无
//**********************************************************************
void SMDrv_GPIO_EnableInterrupt(uint32 u32PinNum,uint8 u8Enable)
{
    if(u32PinNum >= IO_UNKNOW)
        return ;

    if(Smdrv_IsSpiCs(u32PinNum))
        return ;

    if(u8Enable == TRUE)
        am_hal_gpio_interrupt_enable(AM_HAL_GPIO_BIT(u32PinNum));
    else
        am_hal_gpio_interrupt_disable(AM_HAL_GPIO_BIT(u32PinNum));
}

//**********************************************************************
// 函数功能:  设置GPIO中断优先级
// 输入参数： prio:中断优先级，bit0~2位值有效
// 返回参数： 无
//**********************************************************************
void SMDrv_GPIO_SetIrqPrio(uint32 prio)
{
#if AM_CMSIS_REGS
    NVIC_SetPriority(GPIO_IRQn,prio);
#else
    am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_GPIO, AM_HAL_INTERRUPT_PRIORITY(prio));
#endif
}

//**********************************************************************
// 函数功能:  将GPIO disable掉，以实现低功耗
// 输入参数： u32PinNum: gpio num, 值参考io_config.h中IO定义
// 返回参数： Ret_InvalidParam,Ret_OK
//**********************************************************************
ret_type SMDrv_GPIO_Close(uint32 u32PinNum)
{
    am_hal_gpio_pincfg_t bfGpioCfg;

    if(u32PinNum >= IO_UNKNOW)
        return Ret_InvalidParam;

    if(Smdrv_IsSpiCs(u32PinNum))
        return Ret_OK;

    bfGpioCfg.uFuncSel       = 3,
    bfGpioCfg.eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_2MA;
    bfGpioCfg.eGPOutcfg      = AM_HAL_GPIO_PIN_OUTCFG_DISABLE;
    am_hal_gpio_pinconfig(u32PinNum, bfGpioCfg);
    return Ret_OK;
}

//**********************************************************************
// 函数功能:  设置pin脚输出高电平
// 输入参数： u32PinNum: gpio num, 值参考io_config.h中IO定义
// 返回参数： Ret_InvalidParam,Ret_OK
//**********************************************************************
ret_type SMDrv_GPIO_BitSet(uint32 u32PinNum)
{
    if(u32PinNum >= IO_UNKNOW)
        return Ret_InvalidParam;

    if(Smdrv_IsSpiCs(u32PinNum))
        return Ret_OK;

    am_hal_gpio_state_write(u32PinNum, AM_HAL_GPIO_OUTPUT_SET);
    return Ret_OK;
}

//**********************************************************************
// 函数功能:  设置pin脚输出低电平
// 输入参数： u32PinNum: gpio num, 值参考io_config.h中IO定义
// 返回参数： Ret_InvalidParam,Ret_OK
//**********************************************************************
ret_type SMDrv_GPIO_BitClear(uint32 u32PinNum)
{
    if(u32PinNum >= IO_UNKNOW)
        return Ret_InvalidParam;

    if(Smdrv_IsSpiCs(u32PinNum))
        return Ret_OK;

    am_hal_gpio_state_write(u32PinNum, AM_HAL_GPIO_OUTPUT_CLEAR);
    return Ret_OK;
}

//**********************************************************************
// 函数功能:  将pin脚输出电平反转
// 输入参数： u32PinNum: gpio num, 值参考io_config.h中IO定义
// 返回参数： Ret_InvalidParam,Ret_OK
//**********************************************************************
ret_type SMDrv_GPIO_BitToggle(uint32 u32PinNum)
{
    if(u32PinNum >= IO_UNKNOW)
        return Ret_InvalidParam;

    if(Smdrv_IsSpiCs(u32PinNum))
        return Ret_OK;

    am_hal_gpio_state_write(u32PinNum, AM_HAL_GPIO_OUTPUT_TOGGLE);
    return Ret_OK;
}

//**********************************************************************
// 函数功能:  从输入读取寄存器中获取pin脚电平值
// 输入参数： u32PinNum: gpio num, 值参考io_config.h中IO定义
// 返回参数： 0xFFFFFFFF:参数错误，其他值为读取到的值
//**********************************************************************
uint32 SMDrv_GPIO_InBitRead(uint32 u32PinNum)
{
    uint32_t u32State;

    if(u32PinNum >= IO_UNKNOW)
        return 0xFFFFFFFF;

    if(Smdrv_IsSpiCs(u32PinNum))
        return 0xFFFFFFFF;

    am_hal_gpio_state_read(u32PinNum,AM_HAL_GPIO_INPUT_READ, &u32State);
    return u32State;
}

//**********************************************************************
// 函数功能:  从数据输出寄存器中获取pin脚电平值
// 输入参数： u32PinNum: gpio num, 值参考io_config.h中IO定义
// 返回参数： 0xFFFFFFFF:参数错误，其他值为读取到的值
//**********************************************************************
uint32 SMDrv_GPIO_OutBitRead(uint32 u32PinNum)
{
    uint32_t u32State;

    if(u32PinNum >= IO_UNKNOW)
        return 0xFFFFFFFF;

    if(Smdrv_IsSpiCs(u32PinNum))
        return 0xFFFFFFFF;

    am_hal_gpio_state_read(u32PinNum,AM_HAL_GPIO_OUTPUT_READ, &u32State);
    return u32State;
}

#else

typedef struct
{
   uint32  pin;           //gpio pin脚
   gpio_cb gpio_callback; //gpio中断回调函数
}gpio_irq_s;

//定义GPIO中断数
#define GPIO_INT_MAX 10

//记录注册的GPIO中断的pin，callback，以及个数
static gpio_irq_s gpio_int[GPIO_INT_MAX];
uint8 u8IntCount =0;

//**********************************************************************
// 函数功能:    GPIO中断服务函数
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
void am_gpio_isr(void)
{
    uint64 int_pin_value;
    uint32 gpio_id; 

    int_pin_value = am_hal_gpio_int_status_get(true);
    am_hal_gpio_int_clear(0xFFFFFFFFFFFFFFFF);

    for(gpio_id = 0; gpio_id < u8IntCount; gpio_id++)
    {
        if(AM_HAL_GPIO_BIT(gpio_int[gpio_id].pin) & int_pin_value)
        {
            if(gpio_int[gpio_id].gpio_callback != NULL)
            {
                gpio_int[gpio_id].gpio_callback(gpio_int[gpio_id].pin);
            }
        }
    }
}

//**********************************************************************
// 函数功能:    GPIO初始化
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
void SMDrv_GPIO_Init(void)
{
     memset(gpio_int,0x00,sizeof(gpio_int));
     u8IntCount =0;
}

//**********************************************************************
// 函数功能: 根据GPIO module ID配置GPIO属性:输入/输出，若为中断，则设置
//           则设置中断服务程序
// 输入参数：u32PinNum: gpio num, 值参考io_config.h中IO定义
//    config_opt:设置GPIO属性，若在程序中改变GPIO属性时使用，如用GPIO模拟
//      IIC时，SDA脚一会为输入，一会为输出。
//      == NULL: 使用默认的属性配置， != NULL: 则使用传入的属性
//    g_callback:中断服务函数callback
// 返回参数: Ret_OK，Ret_InvalidParam
//**********************************************************************
ret_type SMDrv_GPIO_Open(uint32 u32PinNum,uint32 *config_opt,gpio_cb g_callback)
{
    uint32 u32Pin,bgpio_irq = 0;
    uint32 gpio_conf = AM_HAL_PIN_OUTPUT, gpio_pol = AM_HAL_GPIO_FALLING;

    if(u32PinNum >= IO_UNKNOW)
        return Ret_InvalidParam; 

    //step 1:如果配置为中断，设置中断服务程序，并获取对应GPIO值
    if(u8IntCount < GPIO_INT_MAX)
    {
        if(g_callback != NULL)
        {
            for(u32Pin = 0; u32Pin < GPIO_INT_MAX; u32Pin++)
            {
                //要注册的中断gpio已经注册过
                if(gpio_int[u32Pin].pin == u32PinNum)
                    break;
                if(gpio_int[u32Pin].gpio_callback == NULL)
                    break;
            }
            //注册的中断GPIO超过，预设最大值
            if(u32Pin >= GPIO_INT_MAX)
                return Ret_InvalidParam; 

            //没有注册过，或重启注册callback
            //if((u32Pin < GPIO_INT_MAX) && (gpio_int[u32Pin].gpio_callback != g_callback))
            if(gpio_int[u32Pin].gpio_callback != g_callback)
            {
                //如果没有注册过，则将u8IntCount +=1
                if(gpio_int[u32Pin].gpio_callback == NULL)
                    u8IntCount++;
                gpio_int[u32Pin].gpio_callback = g_callback;
                gpio_int[u32Pin].pin = u32PinNum;
            }
        }
    }

    //step 2:配置GPIO口默认属性，根据原理图
    if((u32PinNum == KEY_S0_PIN) || (u32PinNum == KEY_S1_PIN) ||(u32PinNum == KEY_S2_PIN))
    {
        bgpio_irq = 1;
        gpio_pol = AM_HAL_GPIO_FALLING;    //下降沿触发
        gpio_conf = AM_HAL_PIN_INPUT | AM_HAL_GPIO_PULL24K;
    }
    else if((u32PinNum == HR_INT1_PIN) ||(u32PinNum == BLE_RDY_PIN))
    {
        bgpio_irq = 1;
        gpio_pol = AM_HAL_GPIO_RISING;    //上升沿触发
        gpio_conf = AM_HAL_PIN_INPUT;
        
    }else if ((u32PinNum == HR_INT2_PIN) || (u32PinNum == BAT_CHG_PIN) || (u32PinNum == BAT_PG_PIN))
    {
        gpio_conf = AM_HAL_PIN_INPUT;
        if(u32PinNum == BAT_CHG_PIN)
        {
            gpio_conf |= AM_HAL_GPIO_PULL6K;
        }
    }
    else
    {
        gpio_conf = AM_HAL_PIN_OUTPUT;   //GPIO设置为输出
    }
            
    //step 3:设置pin脚输入输出，上拉属性
    if(config_opt != NULL)
    {
        //如果参数传入设置GPIO属性的参数，就使用传入的值，否则使用默认配置的值
        gpio_conf = *config_opt;
    }
    am_hal_gpio_pin_config(u32PinNum, gpio_conf);

    //step 4:中断模式，则将GPIO口配置为中断
    if(bgpio_irq == 1)
    {
        //am_hal_gpio_int_set(AM_HAL_GPIO_BIT(u32PinNum));
        if(u32PinNum != HR_INT2_PIN)
        {
            am_hal_gpio_int_polarity_bit_set(u32PinNum,gpio_pol);  
            am_hal_gpio_int_clear(AM_HAL_GPIO_BIT(u32PinNum));
            if(u32PinNum != BLE_RDY_PIN) //BLE EM9304 open的时候暂不需要enable
            {
                am_hal_gpio_int_enable(AM_HAL_GPIO_BIT(u32PinNum));
            }
        }
    }

    return Ret_OK;
}

//**********************************************************************
// 函数功能:  重新设置中断触发条件
// 输入参数： u32PinNum: gpio num,
//   u32pol:  =1: 上升沿，=0: 下降沿
// 返回参数： 无
//**********************************************************************
void SMDrv_GPIO_ReConfigIrq(uint32 u32PinNum,uint32 u32pol)
{
    uint32 ui32Polarity;

    ui32Polarity = (u32pol == 1)?AM_HAL_GPIO_RISING:AM_HAL_GPIO_FALLING;
    am_hal_interrupt_disable(AM_HAL_INTERRUPT_GPIO);
    am_hal_gpio_int_disable(AM_HAL_GPIO_BIT(u32PinNum));
    am_hal_gpio_int_polarity_bit_set(u32PinNum,ui32Polarity);
    am_hal_gpio_int_clear(AM_HAL_GPIO_BIT(u32PinNum));
    am_hal_gpio_int_enable(AM_HAL_GPIO_BIT(u32PinNum));
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_GPIO);
}

//**********************************************************************
// 函数功能:  开启/关闭pin脚中断
// 输入参数： u32PinNum: gpio num,
// u8Enable:  =1: enable中断，=0: disable中断
// 返回参数： 无
//**********************************************************************
void SMDrv_GPIO_EnableInterrupt(uint32 u32PinNum,uint8 u8Enable)
{
    if(u8Enable == TRUE)
        am_hal_gpio_int_enable(AM_HAL_GPIO_BIT(u32PinNum));
    else
        am_hal_gpio_int_disable(AM_HAL_GPIO_BIT(u32PinNum));
}

//**********************************************************************
// 函数功能:  设置GPIO中断优先级
// 输入参数： prio:中断优先级，bit0~2位值有效
// 返回参数： 无
//**********************************************************************
void SMDrv_GPIO_SetIrqPrio(uint32 prio)
{
    am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_GPIO, AM_HAL_INTERRUPT_PRIORITY(prio));
}

//**********************************************************************
// 函数功能:  设置GPIO为其他模式，如uart，iic，spi，pdm等
// 输入参数： u32PinNum: gpio num, 值参考io_config.h中IO定义
//            u32config: gpio pad
// 返回参数： 无
//**********************************************************************
void SMDrv_GPIO_SetIOPad(uint32 u32PinNum,uint32 u32config)
{
    am_hal_gpio_pin_config(u32PinNum, u32config);
}

//**********************************************************************
// 函数功能:  将GPIO disable掉，以实现低功耗
// 输入参数： u32PinNum: gpio num, 值参考io_config.h中IO定义
// 返回参数： Ret_InvalidParam,Ret_OK
//**********************************************************************
ret_type SMDrv_GPIO_Close(uint32 u32PinNum)
{
    uint32 u32Pin;

    if(u32PinNum >= IO_UNKNOW)
        return Ret_InvalidParam;

    for(u32Pin = 0; u32Pin < GPIO_INT_MAX; u32Pin++)
    {
        //中断gpio已经注册过,则清除callback，但pin还是保持原来的值
        if(gpio_int[u32Pin].pin == u32PinNum)
        {
            gpio_int[u32Pin].gpio_callback = NULL;
            u8IntCount--;

            am_hal_gpio_int_disable(AM_HAL_GPIO_BIT(u32PinNum)); //disable 中断
            am_hal_gpio_int_clear(AM_HAL_GPIO_BIT(u32PinNum));
            break;
        }
    }

    am_hal_gpio_pin_config(u32PinNum, AM_HAL_PIN_DISABLE);
    return Ret_OK;
}

//**********************************************************************
// 函数功能:  设置pin脚输出高电平
// 输入参数： u32PinNum: gpio num, 值参考io_config.h中IO定义
// 返回参数： Ret_InvalidParam,Ret_OK
//**********************************************************************
ret_type SMDrv_GPIO_BitSet(uint32 u32PinNum)
{
#if 0    //在加for判断中断pin，在用IO模拟iic/spi时会影响速度
    uint32 u32Pin;

    if(u32PinNum >= IO_UNKNOW)
        return Ret_InvalidParam;

    //作为中断使用GPIO，不宜手动设置输出高低电平
    for(u32Pin = 0; u32Pin < GPIO_INT_MAX; u32Pin++)
    {
        if((gpio_int[u32Pin].pin == u32PinNum) && (gpio_int[u32Pin].gpio_callback != NULL))
            return Ret_OK;
    }        
#endif

    am_hal_gpio_out_bit_set(u32PinNum);
    return Ret_OK;
}

//**********************************************************************
// 函数功能:  设置pin脚输出低电平
// 输入参数： u32PinNum: gpio num, 值参考io_config.h中IO定义
// 返回参数： Ret_InvalidParam,Ret_OK
//**********************************************************************
ret_type SMDrv_GPIO_BitClear(uint32 u32PinNum)
{
#if 0    //在加for判断中断pin，在用IO模拟iic/spi时会影响速度
    uint32 u32Pin;

    if(u32PinNum >= IO_UNKNOW)
        return Ret_InvalidParam;

    //作为中断使用GPIO，不宜手动设置输出高低电平
    for(u32Pin = 0; u32Pin < GPIO_INT_MAX; u32Pin++)
    {
        if((gpio_int[u32Pin].pin == u32PinNum) && (gpio_int[u32Pin].gpio_callback != NULL))
            return Ret_OK;
    }
#endif

    am_hal_gpio_out_bit_clear(u32PinNum);
    return Ret_OK;
}

//**********************************************************************
// 函数功能:  将pin脚输出电平反转
// 输入参数： u32PinNum: gpio num, 值参考io_config.h中IO定义
// 返回参数： Ret_InvalidParam,Ret_OK
//**********************************************************************
ret_type SMDrv_GPIO_BitToggle(uint32 u32PinNum)
{
#if 0    //在加for判断中断pin，在用IO模拟iic/spi时会影响速度
    uint32 u32Pin;

    if(u32PinNum >= IO_UNKNOW)
        return Ret_InvalidParam;

    //中断GPIO，不宜手动设置输出高低电平
    for(u32Pin = 0; u32Pin < GPIO_INT_MAX; u32Pin++)
    {
        if((gpio_int[u32Pin].pin == u32PinNum) && (gpio_int[u32Pin].gpio_callback != NULL))
            return Ret_OK;
    }
#endif

    am_hal_gpio_out_bit_toggle(u32PinNum);
    return Ret_OK;
}

//**********************************************************************
// 函数功能:  从输入读取寄存器中获取pin脚电平值
// 输入参数： u32PinNum: gpio num, 值参考io_config.h中IO定义
// 返回参数： 0xFFFFFFFF:参数错误，其他值为读取到的值
//**********************************************************************
uint32 SMDrv_GPIO_InBitRead(uint32 u32PinNum)
{
    if(u32PinNum >= IO_UNKNOW)
        return 0xFFFFFFFF;

    return am_hal_gpio_input_bit_read(u32PinNum);
}

//**********************************************************************
// 函数功能:  从数据输出寄存器中获取pin脚电平值
// 输入参数： u32PinNum: gpio num, 值参考io_config.h中IO定义
// 返回参数： 0xFFFFFFFF:参数错误，其他值为读取到的值
//**********************************************************************
uint32 SMDrv_GPIO_OutBitRead(uint32 u32PinNum)
{
    if(u32PinNum >= IO_UNKNOW)
        return 0xFFFFFFFF;

    return am_hal_gpio_out_bit_read(u32PinNum);
}
#endif

