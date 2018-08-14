
#include "algorithm_sleep.h"

typedef enum
{
	SILENCE_SLEEP, /* ������˯ */
	SACT_SLEEP,	/* С������˯ */
	LACT_SLEEP,	/* ������˯ */
	NULL_SLEEP,	/* δ��˯ */
} sleep_type;

typedef enum
{
	SLEEP_WAKE,  /* ����״̬ */
	SLEEP_SLEEP, /* ��˯״̬ */
	SLEEP_NULL,
} sleep_stage;

typedef struct
{
	uint16_t minSumMax;		/* ��˯�������ܶ���ֵ��ֵ */
	uint16_t packSumMax;	/* ������жϴ��ڴ�ֵ�ĸ���ҪС����CNT */
	uint8_t minFreqMax;		/* ��˯�����ڶ���Ƶ����ֵ */
	uint8_t packSumMaxCnt;  /* ������жϴ�ֵ����ҪС��CNT */
	uint8_t packFreqMax;	/* ������жϴ��ڴ�Ƶ�ʵĸ���ҪС����CNT */
	uint8_t packFreqMaxCnt; /* ������жϴ�Ƶ�ʸ���ҪС��CNT */
} judge_value;

typedef struct
{
	uint16_t ACTSum; /* һ�����ڶ�����ֵ */
	uint8_t ACTCnt;  /* һ�����ڶ���Ƶ�� */
	int8_t ZAvg;	 /* һ������Z��ƽ��ֵ */
} minute_recode;

/* ��ǰ˯��״̬��¼ */
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

#define ONE_ACT_THRESHOLD 2 /* ���ᶯ����ֵ��С�ڸ�ֵ�ж�Ϊ���ţ����� */
#define ACT(a, b) (ABS(a, b) < ONE_ACT_THRESHOLD ? 0 : ABS(a, b))

#define ALL_ACT_THRESHOLD 6 /* ���ᶯ������ֵ,С�ڸ�ֵ�ж�Ϊ�޶���״̬ */

#define WAKE_ACT_SUM_THRE 2200 /* �������Ѷ�����ֵ�����γ�����ֵ�ж�Ϊ���� */
#define WAKE_ACT_FREQ_THRE 25  /* �������Ѷ���Ƶ����ֵ�����γ�����ֵ�ж�Ϊ���� */

#define START_SLEEP_HOUR 22 /* Ĭ��ϰ��˯�߿�ʼʱ�� */
#define STOP_SLEEP_HOUR 10  /* Ĭ��ϰ��˯�߽���ʱ�� */
#define SILEN_TIME 60		/* ������˯��С��¼ʱ�� */
#define SACT_TIME 60		/* С����˯��С��¼ʱ�� */
#define LACT_TIME 90		/* ����˯��С��¼ʱ�� */
#define JUDGE_INTERVAL 27   /* ÿ����˯�ߵ��ж����ʱ�� */

#define STAND_ZAVG_THRE 55  /* Z�ᾲ�ö���������ֵ */
#define ZSTAND_TIME 60		/* �������ƾ���״̬ */
#define ZSTAND_PERCENT 0.95 /* �����ƾ���״̬���ڴ˰ٷֱȣ����ж�Ϊ����״̬ */

#define SILEN_VALUE 0
#define SACT_VALUE 1
#define LACT_VALUE 2
/* ˯���жϾ���ֵ */
const judge_value JudgeArray[3] = {
	{1200, 800, 10, 3, 3, 5},
	{1600, 1200, 15, 3, 5, 5},
	{2200, 1600, 25, 3, 10, 5},
};

static sleep_stage SleepStage = SLEEP_WAKE; /* ��ǰ��˯������״̬ */
static uint8_t SleepType = NULL_SLEEP;		/* ��ǰ��˯����˯״̬ */
static uint8_t StartH, StopH;				/* ϰ��˯�ߵĿ�ʼ����ʱ�� */
static uint8_t JudgeInterval;				/* ÿ����˯�ߵ��ж����ʱ����� */
static uint8_t SecCnt, MinCnt;				/* �ּ���������� */
static int32_t ZACTSum;						/* һ������Z����ֵ */
static uint8_t ZStandCnt;					/* Z���������״̬���� */
static minute_recode MinRcd[30];			/* ��¼�����Сʱ��ÿ���ӵĶ�����ֵ������Ƶ�μ�¼ */

static sleep_tmp_data SleepRecodeTmp; /* ��ǰ˯�߼�¼���� */
static sleep_data SleepRecode[5];	 /* ˯�����ݼ�¼ */
static uint8_t RecodeNum;			  /* ˯�����ݼ�¼��Ч�� */
static sleep_data SleepUI[2];		  /* ������ʾ��˯�����ݣ�0�����㣬0Ϊ��ʾ���ݣ�1Ϊ�ݴ����� */

/* ��ʼ��˯��������� */
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

/* ����ϰ��˯�߿�ʼ����ʱ�� */
void set_sleep_custom(uint8_t startHour, uint8_t stopHour)
{
	StartH = startHour;
	StopH = stopHour;
}

/* ��ȡ˯�߼�¼��������Ч��¼�� */
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

/* ��ȡϵͳ��UTCʱ�� */
#include "mid_rtc.h"
static uint32_t get_utc_time(void)
{
	uint32_t utcTmp = 0;
	utcTmp = Mid_Rtc_ReadCurUtc();

	return utcTmp;
}
/* ��ȡϵͳ��RTCʱ�� */
static rtc_time_s get_rtc_time(void)
{
	rtc_time_s rtcTime;
	Mid_Rtc_TimeRead(&rtcTime);

	return rtcTime;
}

/* �жϴ˶�˯���Ƿ�������ϰ��˯�������ڣ����ڴ������ڲ���¼ */
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
/* ����˯��������UI��ʾ */
static void update_sleep_info(void)
{
	uint8_t uiIndex = 0;
	rtc_time_s rtcTime;
	rtcTime = get_rtc_time();
	/* 0������ʼʱ���˯�߱����ڽ���˯�� */
	if (rtcTime.hour < StartH)
	{
		uiIndex = 0;
	}
	/* ���ڿ�ʼʱ���˯�߱�������һ�� */
	else if (rtcTime.hour >= StartH)
	{
		uiIndex = 1;
	}
	/* �����û���� */
	if (SleepUI[uiIndex].StartUTC == 0)
	{
		SleepUI[uiIndex].StartUTC = SleepRecode[RecodeNum].StartUTC;
		SleepUI[uiIndex].StopUTC = SleepRecode[RecodeNum].StopUTC;
		SleepUI[uiIndex].DurationM = SleepRecode[RecodeNum].DurationM;
	}
	/* ����������� */
	else
	{
		/* �������˯�ߵĿ�ʼʱ����ϴ�˯�ߵĽ���ʱ�����2Сʱ */
		if ((SleepRecode[RecodeNum].StartUTC - SleepUI[uiIndex].StopUTC) > 7200)
		{
			/* ����²�����˯��ʱ�����֮ǰ��ʱ�䣬�򸲸�֮ǰʱ�䣬���򲻼�¼ */
			if (SleepRecode[RecodeNum].DurationM > SleepUI[uiIndex].DurationM)
			{
				SleepUI[uiIndex].StartUTC = SleepRecode[RecodeNum].StartUTC;
				SleepUI[uiIndex].StopUTC = SleepRecode[RecodeNum].StopUTC;
				SleepUI[uiIndex].DurationM = SleepRecode[RecodeNum].DurationM;
			}
		}
		/* ����˯�߽���ʱ�䣬˯��ʱ������ */
		else
		{
			SleepUI[uiIndex].StopUTC = SleepRecode[RecodeNum].StopUTC;
			SleepUI[uiIndex].DurationM += SleepRecode[RecodeNum].DurationM;
		}
	}
}
/* 0��ʱ����˯�ߵ���ʾ��Ϣ���ѻ������ݸ�������ʾ���� */
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

/* �˴�˯�߼�¼��ʼ */
static void sleep_recode_start(void)
{
	SleepRecodeTmp.StartUTC = get_utc_time();
	SleepRecodeTmp.DurationM = 0;
	SleepRecodeTmp.ZStandCnt = 0;
	SleepRecodeTmp.ACTFreq = 0;
	SleepRecodeTmp.SlpType = SleepType;
}

/* ����˯������ */
static void save_sleep_recode(void)
{
	SleepStage = SLEEP_WAKE;
	SleepType = NULL_SLEEP;
	if (((SleepRecodeTmp.SlpType == SILENCE_SLEEP) && (SleepRecodeTmp.DurationM > SILEN_TIME)) ||
		((SleepRecodeTmp.SlpType == SACT_SLEEP) && (SleepRecodeTmp.DurationM > SACT_TIME)) ||
		((SleepRecodeTmp.SlpType == LACT_SLEEP) && (SleepRecodeTmp.DurationM > LACT_TIME)))
	{
		/* �Ƿ��Ǿ���״̬ */
		if (SleepRecodeTmp.ZStandCnt < (SleepRecodeTmp.DurationM * ZSTAND_PERCENT))
		{
			/* ɾ����˯�߼�¼ */
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
			/* ��˯ʱ����ǰ�����жϵ�25���� */
			SleepRecode[RecodeNum].StartUTC = SleepRecodeTmp.StartUTC - 1500;
			/* ����״̬�������ȥ����ʱ�� */
			if (ZStandCnt > ZSTAND_TIME)
			{
				SleepRecode[RecodeNum].StopUTC = get_utc_time() - (ZSTAND_TIME * 60);
			}
			else
			{
				SleepRecode[RecodeNum].StopUTC = get_utc_time();
			}
			SleepRecode[RecodeNum].DurationM = (SleepRecode[RecodeNum].StopUTC - SleepRecode[RecodeNum].StartUTC) / 60;
			/* ��˯��ʱ����̣��򲻼�¼��˯��ʱ�䲻����ϰ��˯�����䣬�򲻼�¼ */
			if ((SleepRecode[RecodeNum].DurationM < SACT_TIME) || (custom_judge() == 0))
			{
				ZStandCnt = 0;
				return;
			}
			/* ���ݵ�ǰ˯�߶εķ�ƽ������Ƶ���ж�˯������ */
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
			/* ����UI��ʾ˯����Ϣ */
			update_sleep_info();
			RecodeNum++;
			JudgeInterval = 0;
			ZStandCnt = 0;
		}
	}
}

/* ��ȡ���յ�˯��չʾ��Ϣ�����ڱ���UI��ʾ��ÿ�β鿴UIʱ����, ����ʱ�ж�Ϊ����������˯�߼�¼*/
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
	/* ˯��ʱ����ԶС�ڵ��ڽ���ʱ�����ʼʱ�� */
	if (((SleepUI[0].StopUTC - SleepUI[0].StartUTC) / 60) < uiInfo.DurationM)
	{
		uiInfo.DurationM = (SleepUI[0].StopUTC - SleepUI[0].StartUTC) / 60;
	}
	/* ����˯��ʱ���ж�˯������ */
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

/* �����30���ӵ������ж��Ƿ���˯״̬ */
static uint8_t sleep_type_judge(void)
{
	uint8_t result = NULL_SLEEP;
	uint8_t minSumCnt[3] = {0, 0, 0}, minFreqCnt[3] = {0, 0, 0}, sumCnt[3] = {0, 0, 0}, freqCnt[3] = {0, 0, 0};

	for (uint8_t i = 0; i < 30; i++)
	{
		for (uint8_t j = 0; j < 3; j++)
		{
			/* ����һ�����ڶ���ֵС�������ֵ�ĸ��� */
			if (MinRcd[i].ACTSum < JudgeArray[j].minSumMax)
			{
				minSumCnt[j]++;
			}
			/* ����һ�����ڶ���Ƶ��С�������ֵ�ĸ��� */
			if (MinRcd[i].ACTCnt < JudgeArray[j].minFreqMax)
			{
				minFreqCnt[j]++;
			}
			/* ����30�����ڶ���ֵ������ֵ�ĸ��� */
			if (MinRcd[i].ACTSum > JudgeArray[j].packSumMax)
			{
				sumCnt[j]++;
			}
			/* ����30�����ڶ���Ƶ�δ�����ֵ�ĸ��� */
			if (MinRcd[i].ACTCnt > JudgeArray[j].packFreqMax)
			{
				freqCnt[j]++;
			}
		}
	}

	/* �ж��������˯���� */
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

/* ÿ���ӵ���˯���㷨�����뵱ǰ�ļ��ٶ�����ֵ */
void sleep_algorithm(int16_t *accelValue)
{
	static int8_t accOld[3]; /* �ϴε�����ֵ */

	int8_t accValue[3];
	uint16_t actSum = 0;
	for (uint8_t i = 0; i < 3; i++)
	{
		/* תΪ8bit����,ͬʱ��С���� */
		accValue[i] = accelValue[i] / 256;
		/* �����ܶ���ֵ */
		actSum += ACT(accValue[i], accOld[i]);
	}

	/* һ�������ۼӶ���ֵ�Ͷ���Ƶ�� */
	if (SecCnt < 60)
	{
		/* һ�������ܶ���ֵ */
		MinRcd[MinCnt].ACTSum += actSum;
		/* һ������Z����ֵ */
		ZACTSum += accValue[2];
		/* һ�����ڶ���Ƶ�� */
		if (actSum > ALL_ACT_THRESHOLD)
		{
			MinRcd[MinCnt].ACTCnt++;
		}
	}

	/* ʱ����� */
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

	/* ����1���������жϴ��� */
	if (SecCnt == 59)
	{
		/* �жϼ���������� */
		if (JudgeInterval <= JUDGE_INTERVAL)
		{
			JudgeInterval++;
		}
		/* �ж����30���ӵ�˯��״̬ */
		SleepType = sleep_type_judge();
		/* �Ƿ�0������Ϣ */
		clear_sleep_info();
		/* ����״̬���� */
		if (SleepStage == SLEEP_WAKE)
		{
			/* ����״̬�л�Ϊ��˯״̬����¼��˯���ͺ͵�ǰ��˯ʱ�� */
			if ((SleepType != NULL_SLEEP) && (JudgeInterval >= JUDGE_INTERVAL))
			{
				SleepStage = SLEEP_SLEEP;
				sleep_recode_start();
			}
		}
		/* ��˯״̬���� */
		else if (SleepStage == SLEEP_SLEEP)
		{
			if (SleepRecodeTmp.SlpType != NULL_SLEEP)
			{
				/* ��ǰ��˯ʱ���1 */
				SleepRecodeTmp.DurationM++;
				/* ��ǰ��˯������������ */
				SleepRecodeTmp.ACTFreq += MinRcd[MinCnt].ACTCnt;
				/* ��ǰ���ü�����һ */
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

			/* ��⵽��˯��״̬����������˯�� */
			if (SleepType == NULL_SLEEP)
			{
				save_sleep_recode();
			}
			/* ����⵽�����������ƾ���״̬����������˯�� */
			else if (ZStandCnt > ZSTAND_TIME)
			{
				save_sleep_recode();
			}
			/* ��ǰΪ��˯״̬���жϵ��ζ�����Ƶ�γ������л�Ϊ��״̬���ﵽ�������¼�˴�˯�� */
			else if ((MinRcd[MinCnt].ACTSum > WAKE_ACT_SUM_THRE) || (MinRcd[MinCnt].ACTCnt > WAKE_ACT_FREQ_THRE))
			{
				save_sleep_recode();
			}

			/* �統ǰΪ����˯��֮���⵽ΪС����˯�򰲾���˯�����������˯�ļ�¼��תΪ�µ���˯ */
			if (SleepRecodeTmp.SlpType == LACT_SLEEP)
			{
				if ((SleepType == SACT_SLEEP) || (SleepType == SILENCE_SLEEP))
				{
					sleep_recode_start();
				}
			}
			/* �統ǰΪ������˯��֮���⵽תΪС����˯�����˯���ж��Ƿ���״̬ */
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
