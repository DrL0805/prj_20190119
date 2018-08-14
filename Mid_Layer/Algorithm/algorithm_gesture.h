
#ifndef GESTURE_APP_H
#define GESTURE_APP_H

#include <stdint.h>

typedef enum
{
    GESTURE_NULL,
    RAISE_HAND,
} gesture_type;

extern uint16_t SwitchFreq; /* 采样频率，ms */
gesture_type gesture_process(int16_t *accValue);   /* 抬手亮屏判断 */

#endif
