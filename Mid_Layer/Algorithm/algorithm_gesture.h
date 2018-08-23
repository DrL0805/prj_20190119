
#ifndef GESTURE_APP_H
#define GESTURE_APP_H

#include <stdint.h>

typedef enum
{
    GESTURE_NULL,
    RAISE_HAND,     /* 抬手 */
    FREE_HAND,      /* 放手 */
    TURN_WRIST,     /* 翻腕 */
    SHAKE_HAND,     /* 甩手 */
} gesture_type;

gesture_type gesture_process(int16_t *accValue);   /* 手势判断，200ms调用 */
#endif
