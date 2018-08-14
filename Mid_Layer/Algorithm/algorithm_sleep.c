
#include "algorithm_sleep.h"

typedef enum
{
	SILENCE_SLEEP, /* 安静入睡 */
	SACT_SLEEP,	/* 小动作入睡 */
	LACT_SLEEP,	/* 大动作入睡 */
	NULL_SLEEP,	/* 未入睡 */
} sleep_type;

typedef enum
{
	SLEEP_WAKE,  /* 清醒状态 */
	SLEEP_SLEEP, /* 入睡状态 */
	SLEEP_NULL,
} sleep_stage;

typedef struct
{
	uint16_t minSumMax;		/* 入睡分钟内总动作值阈值 */
	uint16_t packSumMax;	/* 大包内判断大于此值的个数要小于其CNT */
	uint8_t minFreqMax;		/* 入睡分钟内动作频率阈值 */
	uint8_t packSumMaxCnt;  /* 大包内判断大值个数要小于CNT */
	uint8_t packFreqMax;	/* 大包内判断大于此频率的个数要小于其CNT */
	uint8_t packFreqMaxCnt; /* 大包内判断大频率个数要小于CNT */
} judge_value;

typedef struct
{
	uint16_t ACTSum; /* 一分钟内动作总值 */
	uint8_t ACTCnt;  /* 一分钟内动作频次 */
	int8_t ZAvg;	 /* 一分钟内Z轴平均值 */
} minute_recode;

/* 当前睡眠状态记录 */
typedef struct
{
	uint32_t StartUTC;
	uint16_t DurationM;
	uint16_t ZStandCnt;
	uint16_t ACTFreq;
	uint8_t SlpType;
} sleep_tmp_data;

#define ABS(a, b) (((a) - (b) > 0) ? (a) - (b) : (b) - (a))
#define ABS0(a) (((a) - (0) > 0) ? (a) - (0) : (0) - (a))

#define ONE_ACT_THRESHOLD 2 /* 单轴动作阈值，小于该值判断为干扰，过滤 */
#define ACT(a, b) (ABS(a, b) < ONE_ACT_THRESHOLD ? 0 : ABS(a, b))

#define ALL_ACT_THRESHOLD 6 /* 三轴动作总阈值,小于该值判断为无动作状态 */

#define WAKE_ACT_SUM_THRE 2200 /* 单次清醒动作阈值，单次超过该值判断为醒来 */
#define WAKE_ACT_FREQ_THRE 25  /* 单次清醒动作频次阈值，单次超过该值判断为醒来 */

#define START_SLEEP_HOUR 22 /* 默认习惯睡眠开始时间 */
#define STOP_SLEEP_HOUR 10  /* 默认习惯睡眠结束时间 */
#define SILEN_TIME 60		/* 安静入睡最小记录时间 */
#define SACT_TIME 60		/* 小动入睡最小记录时间 */
#define LACT_TIME 90		/* 大动入睡最小记录时间 */
#define JUDGE_INTERVAL 27   /* 每两段睡眠的判定间隔时间 */

#define STAND_ZAVG_THRE 55  /* Z轴静置动作绝对阈值 */
#define ZSTAND_TIME 60		/* 连续疑似静置状态 */
#define ZSTAND_PERCENT 0.95 /* 当疑似静置状态大于此百分比，则判定为静置状态 */

#define SILEN_VALUE 0
#define SACT_VALUE 1
#define LACT_VALUE 2
/* 睡眠判断经验值 */
const judge_value JudgeArray[3] = {
	{1200, 800, 10, 3, 3, 5},
	{1600, 1200, 15, 3, 5, 5},
	{2200, 1600, 25, 3, 10, 5},
};

static sleep_stage SleepStage = SLEEP_WAKE; /* 当前入睡或醒来状态 */
static uint8_t SleepType = NULL_SLEEP;		/* 当前入睡的入睡状态 */
static uint8_t StartH, StopH;				/* 习惯睡眠的开始结束时间 */
static uint8_t JudgeInterval;				/* 每两段睡眠的判定间隔时间计数 */
static uint8_t SecCnt, MinCnt;				/* 分计数，秒计数 */
static int32_t ZACTSum;						/* 一分钟内Z轴总值 */
static uint8_t ZStandCnt;					/* Z轴持续静置状态计数 */
static minute_recode MinRcd[30];			/* 记录最近半小时内每分钟的动作总值，动作频次记录 */

static sleep_tmp_data SleepRecodeTmp; /* 当前睡眠记录数据 */
static sleep_data SleepRecode[5];	 /* 睡眠数据记录 */
static uint8_t RecodeNum;			  /* 睡眠数据记录有效数 */
static sleep_data SleepUI[2];		  /* 用于显示的睡眠数据，0点清零，0为显示数据，1为暂存数据 */

/* 初始化睡眠相关数据 */
void sleep_algorithm_init(void)
{
	SleepStage = SLEEP_WAKE;
	SleepType = NULL_SLEEP;
	StartH = START_SLEEP_HOUR;
	StopH = STOP_SLEEP_HOUR;
	JudgeInterval = JUDGE_INTERVAL;
	SecCnt = 0;
	MinCnt = 0;
	ZACTSum = 0;
	ZStandCnt = 0;
	for (uint8_t i = 0; i < 30; i++)
	{
		MinRcd[i].ACTSum = 0xFFFF;
		MinRcd[i].ACTCnt = 0xFF;
		MinRcd[i].ZAvg = 0xFF;
	}
	SleepRecodeTmp.StartUTC = 0;
	SleepRecodeTmp.DurationM = 0;
	SleepRecodeTmp.ZStandCnt = 0;
	SleepRecodeTmp.ACTFreq = 0;
	SleepRecodeTmp.SlpType = NULL_SLEEP;
	for (uint8_t i = 0; i < 5; i++)
	{
		SleepRecode[i].StartUTC = 0;
		SleepRecode[i].StopUTC = 0;
		SleepRecode[i].DurationM = 0;
		SleepRecode[i].Quality = 0;
	}
	for (uint8_t i = 0; i < 2; i++)
	{
		SleepUI[i].StartUTC = 0;
		SleepUI[i].StopUTC = 0;
		SleepUI[i].DurationM = 0;
		SleepUI[i].Quality = 0;
	}
	RecodeNum = 0;
}

/* 设置习惯睡眠开始结束时间 */
void set_sleep_custom(uint8_t startHour, uint8_t stopHour)
{
	StartH = startHour;
	StopH = stopHour;
}

/* 获取睡眠记录，返回有效记录数 */
uint8_t get_sleep_recode(sleep_data *recodeTmp)
{
	uint8_t result = 0;

	for (uint8_t i = 0; i <= RecodeNum; i++)
	{
		recodeTmp[i].StartUTC = SleepRecode[i].StartUTC;
		recodeTmp[i].StopUTC = SleepRecode[i].StopUTC;
		recodeTmp[i].DurationM = SleepRecode[i].DurationM;
		recodeTmp[i].Quality = SleepRecode[i].Quality;
	}
	result = RecodeNum;
	RecodeNum = 0;
	return result;
}

/* 获取系统的UTC时间 */
#include "mid_rtc.h"
static uint32_t get_utc_time(void)
{
	uint32_t utcTmp = 0;
	utcTmp = Mid_Rtc_ReadCurUtc();

	return utcTmp;
}
/* 获取系统的RTC时间 */
static rtc_time_s get_rtc_time(void)
{
	rtc_time_s rtcTime;
	Mid_Rtc_TimeRead(&rtcTime);

	return rtcTime;
}

/* 判断此段睡眠是否有落在习惯睡眠区间内，不在此区间内不记录 */
static uint8_t custom_judge(void)
{
	rtc_time_s startRtc, stopRtc;
	UtcTransformTime(SleepRecode[RecodeNum].StartUTC, &startRtc);
	UtcTransformTime(SleepRecode[RecodeNum].StopUTC, &stopRtc);
	if ((startRtc.hour <= StartH) && (startRtc.hour >= StopH) && (stopRtc.hour <= StartH) && (stopRtc.hour >= StopH) && (stopRtc.hour > startRtc.hour))
	{
		return 0;
	}
	return 1;
}
/* 保存睡眠数据至UI显示 */
static void update_sleep_info(void)
{
	uint8_t uiIndex = 0;
	rtc_time_s rtcTime;
	rtcTime = get_rtc_time();
	/* 0点至开始时间的睡眠保存在今日睡眠 */
	if (rtcTime.hour < StartH)
	{
		uiIndex = 0;
	}
	/* 大于开始时间的睡眠保存在下一天 */
	else if (rtcTime.hour >= StartH)
	{
		uiIndex = 1;
	}
	/* 如果还没数据 */
	if (SleepUI[uiIndex].StartUTC == 0)
	{
		SleepUI[uiIndex].StartUTC = SleepRecode[RecodeNum].StartUTC;
		SleepUI[uiIndex].StopUTC = SleepRecode[RecodeNum].StopUTC;
		SleepUI[uiIndex].DurationM = SleepRecode[RecodeNum].DurationM;
	}
	/* 如果已有数据 */
	else
	{
		/* 如果本次睡眠的开始时间和上次睡眠的结束时间相差2小时 */
		if ((SleepRecode[RecodeNum].StartUTC - SleepUI[uiIndex].StopUTC) > 7200)
		{
			/* 如果新产生的睡眠时间大于之前的时间，则覆盖之前时间，否则不记录 */
			if (SleepRecode[RecodeNum].DurationM > SleepUI[uiIndex].DurationM)
			{
				SleepUI[uiIndex].StartUTC = SleepRecode[RecodeNum].StartUTC;
				SleepUI[uiIndex].StopUTC = SleepRecode[RecodeNum].StopUTC;
				SleepUI[uiIndex].DurationM = SleepRecode[RecodeNum].DurationM;
			}
		}
		/* 更新睡眠结束时间，睡眠时长增加 */
		else
		{
			SleepUI[uiIndex].StopUTC = SleepRecode[RecodeNum].StopUTC;
			SleepUI[uiIndex].DurationM += SleepRecode[RecodeNum].DurationM;
		}
	}
}
/* 0点时更新睡眠的显示信息，把缓存数据复制至显示数据 */
static void clear_sleep_info(void)
{
	static uint8_t clearFlag = 0;
	rtc_time_s rtcTime;
	rtcTime = get_rtc_time();
	if ((rtcTime.hour == 0) && (rtcTime.min == 0) && (clearFlag == 0))
	{
		SleepUI[0].StartUTC = SleepUI[1].StartUTC;
		SleepUI[0].StopUTC = SleepUI[1].StopUTC;
		SleepUI[0].DurationM = SleepUI[1].DurationM;

		SleepUI[1].StartUTC = 0;
		SleepUI[1].StopUTC = 0;
		SleepUI[1].DurationM = 0;

		clearFlag = 1;
	}
	if ((rtcTime.hour != 0) || (rtcTime.min != 0))
	{
		clearFlag = 0;
	}
}

/* 此次睡眠记录开始 */
static void sleep_recode_start(void)
{
	SleepRecodeTmp.StartUTC = get_utc_time();
	SleepRecodeTmp.DurationM = 0;
	SleepRecodeTmp.ZStandCnt = 0;
	SleepRecodeTmp.ACTFreq = 0;
	SleepRecodeTmp.SlpType = SleepType;
}

/* 保存睡眠数据 */
static void save_sleep_recode(void)
{
	SleepStage = SLEEP_WAKE;
	SleepType = NULL_SLEEP;
	if (((SleepRecodeTmp.SlpType == SILENCE_SLEEP) && (SleepRecodeTmp.DurationM > SILEN_TIME)) ||
		((SleepRecodeTmp.SlpType == SACT_SLEEP) && (SleepRecodeTmp.DurationM > SACT_TIME)) ||
		((SleepRecodeTmp.SlpType == LACT_SLEEP) && (SleepRecodeTmp.DurationM > LACT_TIME)))
	{
		/* 是否是静置状态 */
		if (SleepRecodeTmp.ZStandCnt < (SleepRecodeTmp.DurationM * ZSTAND_PERCENT))
		{
			/* 删除旧睡眠记录 */
			if (RecodeNum >= 5)
			{
				for (uint8_t i = 0; i < 4; i++)
				{
					SleepRecode[i].StartUTC = SleepRecode[i + 1].StartUTC;
					SleepRecode[i].StopUTC = SleepRecode[i + 1].StopUTC;
					SleepRecode[i].DurationM = SleepRecode[i + 1].DurationM;
					SleepRecode[i].Quality = SleepRecode[i + 1].Quality;
				}
				RecodeNum = 4;
			}
			uint8_t ACTAvg = 0;
			/* 入睡时间往前增加判断的25分钟 */
			SleepRecode[RecodeNum].StartUTC = SleepRecodeTmp.StartUTC - 1500;
			/* 静置状态结束则减去静置时间 */
			if (ZStandCnt > ZSTAND_TIME)
			{
				SleepRecode[RecodeNum].StopUTC = get_utc_time() - (ZSTAND_TIME * 60);
			}
			else
			{
				SleepRecode[RecodeNum].StopUTC = get_utc_time();
			}
			SleepRecode[RecodeNum].DurationM = (SleepRecode[RecodeNum].StopUTC - SleepRecode[RecodeNum].StartUTC) / 60;
			/* 如睡眠时间过短，则不记录或睡眠时间不处于习惯睡眠区间，则不记录 */
			if ((SleepRecode[RecodeNum].DurationM < SACT_TIME) || (custom_judge() == 0))
			{
				ZStandCnt = 0;
				return;
			}
			/* 根据当前睡眠段的分平均动作频次判断睡眠质量 */
			ACTAvg = SleepRecodeTmp.ACTFreq * 10 / SleepRecode[RecodeNum].DurationM;
			if (ACTAvg < 5)
			{
				SleepRecode[RecodeNum].Quality = EXCELLENT;
			}
			else if (ACTAvg < 10)
			{
				SleepRecode[RecodeNum].Quality = GOOD;
			}
			else if (ACTAvg < 15)
			{
				SleepRecode[RecodeNum].Quality = ONLY_FAIR;
			}
			else
			{
				SleepRecode[RecodeNum].Quality = POOR;
			}
			/* 更新UI显示睡眠信息 */
			update_sleep_info();
			RecodeNum++;
			JudgeInterval = 0;
			ZStandCnt = 0;
		}
	}
}

/* 获取当日的睡眠展示信息，用于本地UI显示，每次查看UI时调用, 调用时判断为醒来，保存睡眠记录*/
sleep_ui_info get_sleep_info(void)
{
	sleep_ui_info uiInfo;
	rtc_time_s rtcTime;
	if (SleepStage == SLEEP_SLEEP)
	{
		save_sleep_recode();
	}
	UtcTransformTime(SleepUI[0].StartUTC, &rtcTime);
	uiInfo.StartHour = rtcTime.hour;
	uiInfo.StartMin = rtcTime.min;
	UtcTransformTime(SleepUI[0].StopUTC, &rtcTime);
	uiInfo.StopHour = rtcTime.hour;
	uiInfo.StopMin = rtcTime.min;
	uiInfo.DurationM = SleepUI[0].DurationM;
	/* 睡眠时长永远小于等于结束时间减开始时间 */
	if (((SleepUI[0].StopUTC - SleepUI[0].StartUTC) / 60) < uiInfo.DurationM)
	{
		uiInfo.DurationM = (SleepUI[0].StopUTC - SleepUI[0].StartUTC) / 60;
	}
	/* 根据睡眠时长判断睡眠质量 */
	if (uiInfo.DurationM > 480)
	{
		uiInfo.Quality = EXCELLENT;
	}
	else if (uiInfo.DurationM > 390)
	{
		uiInfo.Quality = GOOD;
	}
	else if (uiInfo.DurationM > 300)
	{
		uiInfo.Quality = ONLY_FAIR;
	}
	else
	{
		uiInfo.Quality = POOR;
	}
	return uiInfo;
}

/* 对最近30分钟的数据判断是否入睡状态 */
static uint8_t sleep_type_judge(void)
{
	uint8_t result = NULL_SLEEP;
	uint8_t minSumCnt[3] = {0, 0, 0}, minFreqCnt[3] = {0, 0, 0}, sumCnt[3] = {0, 0, 0}, freqCnt[3] = {0, 0, 0};

	for (uint8_t i = 0; i < 30; i++)
	{
		for (uint8_t j = 0; j < 3; j++)
		{
			/* 满足一分钟内动作值小于最大阈值的个数 */
			if (MinRcd[i].ACTSum < JudgeArray[j].minSumMax)
			{
				minSumCnt[j]++;
			}
			/* 满足一分钟内动作频次小于最大阈值的个数 */
			if (MinRcd[i].ACTCnt < JudgeArray[j].minFreqMax)
			{
				minFreqCnt[j]++;
			}
			/* 满足30分钟内动作值大于阈值的个数 */
			if (MinRcd[i].ACTSum > JudgeArray[j].packSumMax)
			{
				sumCnt[j]++;
			}
			/* 满足30分钟内动作频次大于阈值的个数 */
			if (MinRcd[i].ACTCnt > JudgeArray[j].packFreqMax)
			{
				freqCnt[j]++;
			}
		}
	}

	/* 判断满足的入睡类型 */
	for (uint8_t i = 0; i < 3; i++)
	{
		if ((minSumCnt[i] >= 29) && (minFreqCnt[i] >= 29) && (sumCnt[i] < JudgeArray[i].packSumMax) && (freqCnt[i] < JudgeArray[i].packFreqMaxCnt))
		{
			result = i;
			return result;
		}
	}

	return result;
}

/* 每秒钟调用睡眠算法，传入当前的加速度三轴值 */
void sleep_algorithm(int16_t *accelValue)
{
	static int8_t accOld[3]; /* 上次的三轴值 */

	int8_t accValue[3];
	uint16_t actSum = 0;
	for (uint8_t i = 0; i < 3; i++)
	{
		/* 转为8bit数据,同时减小干扰 */
		accValue[i] = accelValue[i] / 256;
		/* 三轴总动作值 */
		actSum += ACT(accValue[i], accOld[i]);
	}

	/* 一分钟内累加动作值和动作频次 */
	if (SecCnt < 60)
	{
		/* 一分钟内总动作值 */
		MinRcd[MinCnt].ACTSum += actSum;
		/* 一分钟内Z轴总值 */
		ZACTSum += accValue[2];
		/* 一分钟内动作频次 */
		if (actSum > ALL_ACT_THRESHOLD)
		{
			MinRcd[MinCnt].ACTCnt++;
		}
	}

	/* 时间计数 */
	SecCnt++;
	if (SecCnt == 60)
	{
		SecCnt = 0;
		MinRcd[MinCnt].ZAvg = ZACTSum / 60;
		MinCnt++;
		if (MinCnt == 30)
		{
			MinCnt = 0;
		}
		MinRcd[MinCnt].ACTSum = 0;
		MinRcd[MinCnt].ACTCnt = 0;
		ZACTSum = 0;
	}

	/* 到达1分钟数据判断处理 */
	if (SecCnt == 59)
	{
		/* 判断间隔计数递增 */
		if (JudgeInterval <= JUDGE_INTERVAL)
		{
			JudgeInterval++;
		}
		/* 判断最近30分钟的睡眠状态 */
		SleepType = sleep_type_judge();
		/* 是否到0点清信息 */
		clear_sleep_info();
		/* 醒来状态处理 */
		if (SleepStage == SLEEP_WAKE)
		{
			/* 醒来状态切换为入睡状态，记录入睡类型和当前入睡时间 */
			if ((SleepType != NULL_SLEEP) && (JudgeInterval >= JUDGE_INTERVAL))
			{
				SleepStage = SLEEP_SLEEP;
				sleep_recode_start();
			}
		}
		/* 入睡状态处理 */
		else if (SleepStage == SLEEP_SLEEP)
		{
			if (SleepRecodeTmp.SlpType != NULL_SLEEP)
			{
				/* 当前入睡时间加1 */
				SleepRecodeTmp.DurationM++;
				/* 当前入睡动作次数增加 */
				SleepRecodeTmp.ACTFreq += MinRcd[MinCnt].ACTCnt;
				/* 当前静置计数加一 */
				if (ABS0(MinRcd[MinCnt].ZAvg) > STAND_ZAVG_THRE)
				{
					SleepRecodeTmp.ZStandCnt++;
					ZStandCnt++;
				}
				else
				{
					ZStandCnt = 0;
				}
			}

			/* 检测到无睡眠状态，结束本次睡眠 */
			if (SleepType == NULL_SLEEP)
			{
				save_sleep_recode();
			}
			/* 当检测到连续处于疑似静置状态，结束本次睡眠 */
			else if (ZStandCnt > ZSTAND_TIME)
			{
				save_sleep_recode();
			}
			/* 当前为入睡状态，判断单次动作和频次超出，切换为醒状态。达到条件则记录此次睡眠 */
			else if ((MinRcd[MinCnt].ACTSum > WAKE_ACT_SUM_THRE) || (MinRcd[MinCnt].ACTCnt > WAKE_ACT_FREQ_THRE))
			{
				save_sleep_recode();
			}

			/* 如当前为大动入睡，之后检测到为小动入睡或安静入睡，则清除大动入睡的记录，转为新的入睡 */
			if (SleepRecodeTmp.SlpType == LACT_SLEEP)
			{
				if ((SleepType == SACT_SLEEP) || (SleepType == SILENCE_SLEEP))
				{
					sleep_recode_start();
				}
			}
			/* 如当前为安静入睡，之后检测到转为小动入睡或大动入睡，判断是否静置状态 */
			else if (SleepRecodeTmp.SlpType == SILENCE_SLEEP)
			{
				if ((SleepType == SACT_SLEEP) || (SleepType == LACT_SLEEP))
				{
					if (SleepRecodeTmp.ZStandCnt > (SleepRecodeTmp.DurationM * ZSTAND_PERCENT))
					{
						sleep_recode_start();
					}
				}
			}
		}
	}

	for (uint8_t i = 0; i < 3; i++)
	{
		accOld[i] = accValue[i];
	}
}
