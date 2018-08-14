#ifndef _SLEEP_ALGORITHM_H_
#define _SLEEP_ALGORITHM_H_

#include <stdint.h>

/* 睡眠质量 */
typedef enum
{
	EXCELLENT,
	GOOD,
	ONLY_FAIR,
	POOR,
} sleep_quality;

/* 睡眠UI显示信息 */
typedef struct
{
	uint8_t StartHour;
	uint8_t StartMin;
	uint8_t StopHour;
	uint8_t StopMin;
	uint16_t DurationM;
	uint8_t Quality;
} sleep_ui_info;

/* 睡眠记录信息 */
typedef struct
{
	uint32_t StartUTC;  /* 睡眠开始时间 */
	uint32_t StopUTC;   /* 睡眠结束时间 */
	uint16_t DurationM; /* 睡眠时长 */
	uint8_t Quality;	/* 睡眠质量 */
} sleep_data;

void sleep_algorithm_init(void);							/* 参数初始化,重新开始睡眠记录时调用 */
void sleep_algorithm(int16_t *accValue);					/* 睡眠算法，每秒钟调用一次 */
void set_sleep_custom(uint8_t startHour, uint8_t stopHour); /* 设置习惯睡眠开始结束时间，暂不使用 */
uint8_t get_sleep_recode(sleep_data *recodeTmp);			/* 获取每段睡眠记录，返回有效记录数，用于记录存储，上传APP */
sleep_ui_info get_sleep_info(void);							/* 获取当日的睡眠展示信息，用于本地UI显示，每次查看UI时调用 */

#endif // _SLEEP_ALGORITHM_H_
