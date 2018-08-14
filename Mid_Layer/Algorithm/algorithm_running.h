#ifndef _RUNNING_ALGORITHM_H_
#define _RUNNING_ALGORITHM_H_

#include <stdint.h>

/* 跑步算法体征信息 */
typedef struct
{
    uint8_t Weight; /* 体重：单位：kg */
    uint8_t Height; /* 身高：单位：cm */
    uint8_t Sex;    /* 性别 */
    uint8_t Age;    /* 年龄 */
} bodyInfo_A;

/* 跑步实时运动信息 */
typedef struct
{
    uint16_t Pace;     /* s/km 配速 10S内实时更新*/
    uint16_t Freq;     /* step/min 步频 10S内实时更新*/
    uint8_t HeartRate; /* 次 心率记录 */
} runningInfo_RT;

/* 跑步运动信息 */
typedef struct
{
    uint32_t StartUTC;     /* 开始UTC时间戳 */
    uint32_t Time;         /* *s 总持续时间 */
    uint16_t Distance;     /* *m 跑步总距离 */
    uint32_t Step;         /* *step 跑步总步数 */
    uint16_t Calorie;      /* *kcal 卡路里 */
    runningInfo_RT Recode; /* 跑步实时记录信息 */
} runningInfo_A;

void running_algorithm_init(uint32_t currentStep, bodyInfo_A bodyInfo, uint32_t utc); /* 跑步开始，传入当前的计步值，人体信息, utc时间 */
runningInfo_A running_algorithm(uint32_t currentStep, uint8_t heartRate);             /* 每10秒钟调用一次，传入当前的计步值,心率 */

#endif
