#include "platform_common.h"
#include "algorithm_running.h"

static bodyInfo_A BodyInfo;       /* 人体特征参数 */
static runningInfo_A RunningInfo; /* 跑步数据 */

static uint32_t StartStep;
static uint32_t oldStep;

/* 跑步开始，数据初始化 */
void running_algorithm_init(uint32_t currentStep, bodyInfo_A bodyInfo, uint32_t utc)
{
    StartStep = currentStep;
    oldStep = currentStep;

    BodyInfo.Weight = bodyInfo.Weight;
    BodyInfo.Height = bodyInfo.Height;
    BodyInfo.Sex = bodyInfo.Sex;
    BodyInfo.Age = bodyInfo.Age;

    RunningInfo.StartUTC = utc;
    RunningInfo.Time = 0;
    RunningInfo.Distance = 0;
    RunningInfo.Step = 0;
    RunningInfo.Calorie = 0;
    RunningInfo.Recode.Pace = 0;
    RunningInfo.Recode.Freq = 0;
    RunningInfo.Recode.HeartRate = 0;
}

/* 每10秒钟调用一次，计算跑步信息 */
runningInfo_A running_algorithm(uint32_t currentStep, uint8_t heartRate)
{
    uint32_t step10S = 0;

    RunningInfo.Time += 10;                                                  /* *s 跑步总时间 */
    RunningInfo.Step = currentStep - StartStep;                              /* 跑步总步数为当前步数减初始步数 */
    RunningInfo.Distance = BodyInfo.Height * RunningInfo.Step * 0.0045;      /* 跑步总距离 = 身高*0.45/100*步数 */
    RunningInfo.Calorie = BodyInfo.Weight * RunningInfo.Distance * 0.001036; /* 跑步卡路里 = 体重 * 距离 * 1.036 */

    step10S = currentStep - oldStep; /* 10S内步数 */
    if (step10S != 0)
    {
        RunningInfo.Recode.Pace = 100000000 / (BodyInfo.Height * step10S * 45); /* 配速 = s/km */
    }
    else
    {
        RunningInfo.Recode.Pace = 0;
    }
    RunningInfo.Recode.Freq = step10S * 6;    /* 步频 = 步数/分钟 */
    RunningInfo.Recode.HeartRate = heartRate; /* 当前心率 */

    /* 记录之前步数 */
    oldStep = currentStep;

    return RunningInfo;
}
