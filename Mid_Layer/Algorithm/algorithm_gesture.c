/********************************************************
 * 手势识别
 ********************************************************/
#include "algorithm_gesture.h"

/* 加速度传感器在PCB上的摆放位置 */
#define ORIEN_9 0  /* 九点方向为X轴 */
#define ORIEN_12 1 /* 12点方向为Y轴 */
#define ORIEN_UP 2 /* 向上为Z轴 */

/* 以下参数为200ms采样率，+-2G采样范围的参数 */
/* -> 抬手亮屏相关参数 */
#define START_THS_12 3000 /* 12点方向进入起点状态的阈值 */
#define OLD_THS_12 6000   /* 12点方向进入起点状态前的最大阈值 */
#define STAGE_CNT_12 2	/* 12点方向进入第二状态的计数 */

#define START_THS_9 4000 /* 9点3点方向进入起点状态的阈值 */
#define OLD_THS_9 8000   /* 9点3点方向进入起点状态前的最大阈值 */
#define STAGE_CNT_9 2	/* 9点3点方向进入第二状态的计数 */

#define WAIT_STABLE_CNT 3 /* *200ms 切换到第二状态后等待到达看屏角度的时间 */
#define JUDGE_CNT 2		  /* *200ms 确定动作的稳定时间 */

#define ANGLE_9_3 6000  /* 九点到三点的轴与地面的角度范围 */
#define ANGLE_12_6 2000 /* 12点到6点的轴与地面的角度范围 */
/* <- 抬手亮屏相关参数 */

/* -> 灭屏相关参数 */
#define JUDGE_CNT_FREE 3	 /* *200ms 确定灭屏动作的稳定时间 */
#define ANGLE_9_3_FREE 14000 /* 灭屏九点到三点的轴与地面的角度范围 */
#define JUDGE_SUM_FREE 3000  /* 灭屏前的动作触发值 */
/* <- 灭屏相关参数 */

/* -> 翻腕相关参数 */
#define INC_CNT_12 2  /* *200ms 线性递增时间 */
#define DEC_CNT_12 2  /* *200ms 线性递减时间 */
#define WAIT_CNT 6	/* *200ms 等待递减时间 */
#define VARY_THS 1000 /* 每次变化的最小值 */
/* <- 翻腕相关参数 */

/* -> 甩手相关参数 */
#define SHAKE_THS 30000 /* 甩手变化阈值 */
#define SHAKE_CNT_THS 2 /* 甩手连续计数 */
/* <- 甩手相关参数 */

#define ABS(a, b) (((a) - (b) > 0) ? (a) - (b) : (b) - (a))
#define ABS0(a) (((a) - (0) > 0) ? (a) - (0) : (0) - (a))

typedef enum
{
	STAGE_NULL,
	STAGE_ONE,
	STAGE_TWO,
} stage_e;

static int16_t dataOld[3] = {0};

/* 抬手亮屏算法 */
uint8_t raise_hand_algorithm(int16_t *accData)
{
	static uint8_t WaitStab = 0, OnStableCnt = 0;
	static stage_e Stage = STAGE_NULL, StageX = STAGE_NULL;
	static uint8_t LinearDec = 0, LinearDecX = 0;

	/* 判断起点阈值 */
	if ((Stage == STAGE_NULL) && (accData[ORIEN_12] > START_THS_12))
	{
		Stage = STAGE_ONE;
	}
	if (Stage == STAGE_ONE)
	{
		/* 判断线性递减 */
		if (accData[ORIEN_12] < dataOld[ORIEN_12])
		{
			LinearDec++;
			if (LinearDec == 1)
			{
				/* 判断最大值 */
				if (dataOld[ORIEN_12] < OLD_THS_12)
				{
					LinearDec = 0;
					Stage = STAGE_NULL;
				}
			}
			/* 连续递减计数到达，切换为第二状态 */
			else if (LinearDec >= STAGE_CNT_12)
			{
				Stage = STAGE_TWO;
				LinearDec = 0;
				WaitStab = 0;
			}
		}
		else
		{
			Stage = STAGE_NULL;
			/* 还处于起点阈值内，切换为起点状态 */
			if (accData[ORIEN_12] > START_THS_12)
			{
				Stage = STAGE_ONE;
			}
		}
	}

	/* 9点3点轴方向 */
	if ((StageX == STAGE_NULL) && (ABS0(accData[ORIEN_9]) > START_THS_9))
	{
		StageX = STAGE_ONE;
	}
	if (StageX == STAGE_ONE)
	{
		/* 判断绝对值线性递减 */
		if (ABS0(accData[ORIEN_9]) < ABS0(dataOld[ORIEN_9]))
		{
			LinearDecX++;
			if (LinearDecX == 1)
			{
				/* 判断最大值 */
				if (ABS0(dataOld[ORIEN_9]) < OLD_THS_9)
				{
					LinearDecX = 0;
					StageX = STAGE_NULL;
				}
			}
			/* 绝对值线性递减到达，切换为第二状态 */
			else if (LinearDecX >= STAGE_CNT_9)
			{
				StageX = STAGE_TWO;
				LinearDecX = 0;
				WaitStab = 0;
			}
		}
		else
		{
			StageX = STAGE_NULL;
			/* 还处于起点阈值内，切换为起点状态 */
			if (ABS0(accData[ORIEN_9]) > START_THS_9)
			{
				StageX = STAGE_ONE;
			}
		}
	}

	/* 第二状态 */
	if ((Stage == STAGE_TWO) || (StageX == STAGE_TWO))
	{
		/* 当位于第二状态后，判断是否处于看屏角度的数值 */
		if ((ABS0(accData[ORIEN_9]) <= ANGLE_9_3) && (accData[ORIEN_12] <= ANGLE_12_6))
		{
			OnStableCnt++;
			/* 稳定处于看屏角度, 触发亮屏 */
			if (OnStableCnt >= JUDGE_CNT)
			{
				OnStableCnt = 0;
				WaitStab = 0;
				Stage = STAGE_NULL;
				StageX = STAGE_NULL;

				return 1;
			}
		}
		/* 还未到达看屏角度，等待一会 */
		else if (OnStableCnt == 0)
		{
			WaitStab++;
			if (WaitStab >= WAIT_STABLE_CNT)
			{
				/* 等待超时状态清零 */
				WaitStab = 0;
				Stage = STAGE_NULL;
				StageX = STAGE_NULL;
			}
		}
		/* 已到达看屏角度，但不稳定处于看屏角度，状态清零 */
		else
		{
			OnStableCnt = 0;
			WaitStab = 0;
			Stage = STAGE_NULL;
			StageX = STAGE_NULL;
		}
	}

	return 0;
}

/* 放手灭屏算法 */
uint8_t free_hand_algorithm(int16_t *accData)
{
	static uint8_t FreeStableCnt = 0;

	/* 垂直角度灭屏 */
	if (ABS0(accData[ORIEN_9]) >= ANGLE_9_3_FREE)
	{
		FreeStableCnt++;
		/* 稳定处于灭屏角度, 触发灭屏 */
		if (FreeStableCnt >= JUDGE_CNT_FREE)
		{
			FreeStableCnt = 0;

			uint32_t actSumFree = 0;
			for (uint8_t i = 0; i < 3; i++)
			{
				actSumFree += ABS(accData[i], dataOld[i]);
			}
			if (actSumFree >= JUDGE_SUM_FREE)
			{
				return 1;
			}
		}
	}

	return 0;
}

/* 翻腕算法 */
uint8_t turn_wrist_algorithm(int16_t *accData)
{
	static stage_e StageT = STAGE_NULL;
	static uint8_t LinearDecT, LinearInc, WaitDec;

	if (StageT == STAGE_NULL)
	{
		/* 判断线性递增且幅度满足 */
		if ((accData[ORIEN_12] > dataOld[ORIEN_12]) && (ABS(accData[ORIEN_12], dataOld[ORIEN_12]) >= VARY_THS))
		{
			LinearInc++;
			/* 连续递增计数到达，切换为第一状态 */
			if (LinearInc >= INC_CNT_12)
			{
				StageT = STAGE_ONE;
				LinearInc = 0;
				WaitDec = 0;
				LinearDecT = 0;
			}
		}
		else
		{
			LinearInc = 0;
		}
	}
	else if (StageT == STAGE_ONE)
	{
		/* 等待递减计数 */
		WaitDec++;
		/* 线性递减且幅度满足 */
		if ((accData[ORIEN_12] < dataOld[ORIEN_12]) && (ABS(dataOld[ORIEN_12], accData[ORIEN_12]) >= VARY_THS))
		{
			LinearDecT++;
			/* 连续线性递减计数到达 */
			if (LinearDecT >= DEC_CNT_12)
			{
				LinearDecT = 0;
				StageT = STAGE_NULL;
				/* 在等待时间范围内 */
				if (WaitDec <= WAIT_CNT)
				{
					WaitDec = 0;

					return 1;
				}
				WaitDec = 0;
			}
		}
		else
		{
			if (LinearDecT != 0)
			{
				StageT = STAGE_NULL;
				WaitDec = 0;
			}
			LinearDecT = 0;
		}
	}

	return 0;
}

/* 甩手算法 */
uint8_t shake_hand_algorithm(int16_t *accData)
{
	static uint8_t ShakeCnt = 0;

	/* 变化大，计数 */
	if ((ABS(accData[0], dataOld[0]) + ABS(accData[1], dataOld[1]) + ABS(accData[2], dataOld[2])) >= SHAKE_THS)
	{
		ShakeCnt++;
	}
	else
	{
		ShakeCnt = 0;
	}

	/* 连续变化大，甩手 */
	if (ShakeCnt >= SHAKE_CNT_THS)
	{
		ShakeCnt = 0;
		return 1;
	}

	return 0;
}

/* 手势处理 */
gesture_type gesture_process(int16_t *accData)
{
	gesture_type result = GESTURE_NULL;

	if (raise_hand_algorithm(accData))
	{
		result = RAISE_HAND;
	}
	else if (free_hand_algorithm(accData))
	{
		result = FREE_HAND;
	}

	if (turn_wrist_algorithm(accData))
	{
		result = TURN_WRIST;
	}
	else if (shake_hand_algorithm(accData))
	{
		result = SHAKE_HAND;
	}

	/* 记录旧数值 */
	for (uint8_t i = 0; i < 3; i++)
	{
		dataOld[i] = accData[i];
	}

	return result;
}
