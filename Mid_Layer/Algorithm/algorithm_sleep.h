#ifndef _SLEEP_ALGORITHM_H_
#define _SLEEP_ALGORITHM_H_

#include <stdint.h>

/* ˯������ */
typedef enum
{
	EXCELLENT,
	GOOD,
	ONLY_FAIR,
	POOR,
} sleep_quality;

/* ˯��UI��ʾ��Ϣ */
typedef struct
{
	uint8_t StartHour;
	uint8_t StartMin;
	uint8_t StopHour;
	uint8_t StopMin;
	uint16_t DurationM;
	uint8_t Quality;
} sleep_ui_info;

/* ˯�߼�¼��Ϣ */
typedef struct
{
	uint32_t StartUTC;  /* ˯�߿�ʼʱ�� */
	uint32_t StopUTC;   /* ˯�߽���ʱ�� */
	uint16_t DurationM; /* ˯��ʱ�� */
	uint8_t Quality;	/* ˯������ */
} sleep_data;

void sleep_algorithm_init(void);							/* ������ʼ��,���¿�ʼ˯�߼�¼ʱ���� */
void sleep_algorithm(int16_t *accValue);					/* ˯���㷨��ÿ���ӵ���һ�� */
void set_sleep_custom(uint8_t startHour, uint8_t stopHour); /* ����ϰ��˯�߿�ʼ����ʱ�䣬�ݲ�ʹ�� */
uint8_t get_sleep_recode(sleep_data *recodeTmp);			/* ��ȡÿ��˯�߼�¼��������Ч��¼�������ڼ�¼�洢���ϴ�APP */
sleep_ui_info get_sleep_info(void);							/* ��ȡ���յ�˯��չʾ��Ϣ�����ڱ���UI��ʾ��ÿ�β鿴UIʱ���� */

#endif // _SLEEP_ALGORITHM_H_
