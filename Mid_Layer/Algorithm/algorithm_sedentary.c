
#include "algorithm_sedentary.h"

static uint16_t SedentaryTime;

/*******************************
**久坐算法初始化
**input: total step
**output: null
********************************/
void alg_sedentary_init(uint32_t totalStep)
{
    SedentaryTime = 0;
}

/*******************************
**久坐算法处理, 每秒钟调用一次
**input: accelValue，totalStep
**output: null
********************************/
void alg_sedentary_process(int16_t *accelValue, uint32_t totalStep)
{

}

/*******************************
**获取久坐时间
**input: null
**output: sit time
********************************/
uint16_t alg_sedentary_get_time(void)
{

    return SedentaryTime;
}