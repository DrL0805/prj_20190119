

#ifndef MID_FRONT_H
#define MID_FRONT_H

#include "platform_common.h"
#include "drv_font.h"


typedef struct
{
	uint8	wordWidth;//�ֿ��洢��ȣ�
	uint8	wordHeight;//�ָߣ��洢�߶ȣ�
	uint8 	validWidth;//�ֵ���Ч��ȣ�����ʵ������С�ڻ����wordWidth��
	uint8 	dataStringLengh;//����������
}font_size_t;

typedef union
{
	uint16	codeGB;
	uint16 	codeUnicode;
	uint8 	codeASCII;
	uint8	codeIndexUI;
}code_t;

typedef struct 
{
	uint8_t*				dataAddr;	// ������Ϣ�洢��ַ
	Wordstock_size_s 		sizeKind;	// ��������
	code_t 					code;		// ����ֵ
	font_size_t 			wordSize;   // ������Ϣ
}font_para_t;

extern void Mid_Font_Init(void);
extern uint16_t Mid_Font_ReadGB(font_para_t *para);
extern uint16 Mid_Font_ReadASCII(font_para_t *para);
extern uint16_t Mid_Font_SelfTest(void);
extern uint16 Mid_Font_ReadFrontSize(font_para_t *para);

void FrontTask_Create(void);

#endif




