/********************************************************
 * 手势亮屏判断
 ********************************************************/
#include "algorithm_gesture.h"

#define ORIEN_9 0  /* 九点方向为X轴 */
#define ORIEN_12 1 /* 12点方向为Y轴 */
#define ORIEN_UP 2 /* 向上为Z轴 */

/* 以下参数为200ms采样率，+-2G采样范围的参数 */
#define START_THS_12 3000 /* 12点方向进入起点状态的阈值 */
#define OLD_THS_12 6000   /* 12点方向进入起点状态前的最大阈值 */
#define STAGE_CNT_12 3	/* 12点方向进入第二状态的计数 */

#define START_THS_9 4000 /* 9点3点方向进入起点状态的阈值 */
#define OLD_THS_9 8000  /* 9点3点方向进入起点状态前的最大阈值 */
#define STAGE_CNT_9 3	/* 9点3点方向进入第二状态的计数 */

#define WAIT_STABLE_CNT 3 /* *200ms 切换到第二状态后等待到达看屏角度的时间 */
#define JUDGE_CNT 4		  /* *200ms 确定动作的稳定时间 */

#define ANGLE_9_3 6000  /* 九点到三点的轴与地面的角度范围 */
#define ANGLE_12_6 2000 /* 12点到6点的轴与地面的角度范围 */

#define ABS(a, b) (((a) - (b) > 0) ? (a) - (b) : (b) - (a))
#define ABS0(a) (((a) - (0) > 0) ? (a) - (0) : (0) - (a))

typedef enum
{
	STAGE_NULL,
	STAGE_ONE,
	STAGE_TWO,
} stage_e;

uint16_t SwitchFreq = 500; /* 调用频率 */

gesture_type gesture_process(int16_t *accData)
{
	static int16_t dataOld[3] = {0};
	static uint8_t WaitStab = 0;
	static uint8_t OnStableCnt = 0;
	static stage_e Stage = STAGE_NULL;
	static uint8_t LinearDec = 0;
	static stage_e StageX = STAGE_NULL;
	static uint8_t LinearDecX = 0;

	gesture_type result = GESTURE_NULL;

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

				result = RAISE_HAND;
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

	/* 记录旧数值 */
	for (uint8_t i = 0; i < 3; i++)
	{
		dataOld[i] = accData[i];
	}

	/* 根据状态切换调用频率 */
	if ((Stage != STAGE_NULL) || (StageX != STAGE_NULL))
	{
		SwitchFreq = 200;
	}
	else
	{
		SwitchFreq = 500;
	}

	return result;
}
