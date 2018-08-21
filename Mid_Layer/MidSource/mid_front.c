/**********************************************************************
**
**ģ��˵��: mid���ֿ�ӿ�
**����汾���޸���־(ʱ�䣬����),�޸���:
**   V1.0   2018.4.25  �޸�����  ZSL  
**
**********************************************************************/
#include "mid_front.h"
#include "rtos.h"
#include "mid_scheduler.h"

// ��ȡSPI��Դ
static uint32_t Mid_Font_SpiMutexTake(void)
{
	// ��ȡ���������Ȩ
	xSemaphoreTake(SPI_I2C_M0_SemaphoreHandle, portMAX_DELAY);
	
	// ������
	Drv_Font_Open();
}

// �ͷ�SPI��Դ
static uint32_t Mid_Font_SpiMutexGive(void)
{
	// �ر�����
	Drv_Font_Close();
	
	// �ͷ����������Ȩ
	xSemaphoreGive(SPI_I2C_M0_SemaphoreHandle);
}

//**********************************************************************
// ��������:	front �����ʼ�������д���
// ���������	��
// ���ز�����	��
void Mid_Font_Init(void)
{
	Drv_Font_Init();
}

//**********************************************************************
// ��������:	�������δ�С���ؿ��ߡ�������������Ϣ
// ���������	
// para 	��  �ֿ����������
// ���ز�����
uint16 Mid_Front_GetFontSize(Wordstock_size_s sizeKind,font_size_t* sizeTemp)
{
	switch(sizeKind)
	{
		case ASCII_SIZE_5X7:
			sizeTemp->wordWidth 		= 5;
			sizeTemp->wordHeight 		= 7;
			sizeTemp->dataStringLengh 	= 8;
			break;	
		case ASCII_SIZE_7X8:
			sizeTemp->wordWidth 		= 7;
			sizeTemp->wordHeight 		= 8;
			sizeTemp->dataStringLengh 	= 8;
			break;
		case ASCII_SIZE_6X12:
			sizeTemp->wordWidth 		= 6;
			sizeTemp->wordHeight 		= 12;
			sizeTemp->dataStringLengh 	= 12;
			break;
		case ASCII_SIZE_12_B_A:
		case ASCII_SIZE_12_B_T:
			sizeTemp->wordWidth 		= 12;
			sizeTemp->wordHeight 		= 12;
			sizeTemp->dataStringLengh 	= 26;
			break;
		case ASCII_SIZE_8X16:
			sizeTemp->wordWidth 		= 8;
			sizeTemp->wordHeight 		= 16;
			sizeTemp->dataStringLengh 	= 16;
			break;	

		case ASCII_SIZE_16_A:
		case ASCII_SIZE_16_T:
			sizeTemp->wordWidth 		= 16;
			sizeTemp->wordHeight 		= 16;
			sizeTemp->dataStringLengh 	= 34;
			break;	
//		case GB_SIZE_12X12:	
//			sizeTemp->wordWidth 		= 12;
//			sizeTemp->wordHeight 		= 12;
//			sizeTemp->dataStringLengh 	= 24;
//			break;
		case GB_SIZE_16X16:	
			sizeTemp->wordWidth 		= 16;
			sizeTemp->wordHeight 		= 16;
			sizeTemp->dataStringLengh 	= 32;
			break;
		default:
			break;		
	}
	
	return 0;
}

//*******************************************************************
// ��������: ���ݺ��ֱ����ȡ����������
// ���������	
// para    ���ֿ��¼�������Ϣ���������ֱ�ż�������ָ��
// ���ز�����	
// 0x00    :  ���óɹ�
// 0xff    :  ����ʧ��
uint16_t Mid_Font_ReadGB(font_para_t *para)
{
	if (para->sizeKind != GB_SIZE_16X16 && para->sizeKind != GB_SIZE_24X24)
	{
		return 0xff;
	}
	
	Mid_Font_SpiMutexTake();
	
	Mid_Front_GetFontSize(para->sizeKind,&para->wordSize);
	para->wordSize.validWidth = para->wordSize.wordWidth;
	
	Drv_Font_SendCmd(FONT_WAKEUP_CMD);
	Drv_Font_ReadGB2313(para->code.codeGB,para->sizeKind,para->dataAddr);
	Drv_Font_SendCmd(FONT_SLEEP_CMD);
	
	Mid_Font_SpiMutexGive();
	
	return 0;
}

//*******************************************************************
// ��������:	����UNICODE�����ȡ�֣����ֻ�ASCII��������
// ���������	
// para 	��  �ֿ��¼�������Ϣ�������֣����ֻ�ASCII����ż�������ָ��
// ���ز�����	
// 0x00    :    ���óɹ�
// 0xff    :    ����ʧ��
uint16 Mid_Font_ReadUnicode(font_para_t *para)
{
	uint16 codeTemp;
	
//	if (para->code.codeUnicode < 0x0080)//ASCII��
//	{
//		Mid_Front_ReadASCII(para);	
//	}
//	else
//	{
//		Drv_Font_SendCmd(FONT_WAKEUP_CMD);
//		codeTemp  = Drv_Font_Unicode2GB(para->code.codeUnicode);
//		para->code.codeGB 	= codeTemp;
//		Mid_Front_ReadGB(para);	
//    }
	
    return 0;
}

//**********************************************************************
// ��������:	����ASCII���ȡ��ĸ������
// ���������	
// para 	��  �ֿ��¼�������Ϣ������ASCII�뼰������ָ�뼰����
// ���ز�����	
// 0x00    :    �����ɹ�
// 0xff    :    ����ʧ��
uint16 Mid_Font_ReadASCII(font_para_t *para)
{
	uint16 i;
	uint16 codeTemp;

	Mid_Front_GetFontSize(para->sizeKind,&para->wordSize);
	if(para->wordSize.dataStringLengh == 0)
	{
		return 0xff;
	}

	Mid_Font_SpiMutexTake();
	
	if (para->sizeKind == GB_SIZE_16X16 )//������������GB����֧�֣���GB��������ת��
	{
		#if 0	// �˴���δ���Թ������ɿ�
		codeTemp 			=  0xA3A0 - 0x0020 + para->code.codeASCII;
		para->code.codeGB 	= codeTemp;
		Mid_Font_ReadGB(para);
		#endif
	}
	else
	{
		Drv_Font_SendCmd(FONT_WAKEUP_CMD);
		Drv_Font_ReadASCII(para->code.codeASCII,para->sizeKind,para->dataAddr);
		Drv_Font_SendCmd(FONT_SLEEP_CMD);
	}

	if( (para->sizeKind == ASCII_SIZE_12_B_A)||
		(para->sizeKind == ASCII_SIZE_12_B_T)||
		(para->sizeKind == ASCII_SIZE_16_A)||
		(para->sizeKind == ASCII_SIZE_16_T)
	)
	{
		para->wordSize.validWidth 	= (uint16)para->dataAddr[0] << 8 | para->dataAddr[1];

		for(i = 0; i < (para->wordSize.dataStringLengh - 2); i++)//ȥ����Ч�ֿ���Ϣ(ǰ2�ֽ�)
		{
			para->dataAddr[i]			= para->dataAddr[i + 2];
		}
		para->wordSize.dataStringLengh -= 2;
	}
	else
	{
		para->wordSize.validWidth 	= para->wordSize.wordWidth;
	}
	
	Mid_Font_SpiMutexGive();
	
	return 0;
}

//**********************************************************************
// ��������: ����UI���ȡUI������
// ���������	
// para    �� �ֿ��¼�������Ϣ������UI�뼰������ָ�뼰����
// ���ز�����	
// 0x00    :   �����ɹ�
// 0xff    :   ����ʧ��
uint16 Mid_Font_ReadUI(font_para_t *para)
{
	#if 0
	para->wordSize->wordWidth 		= 32;
	para->wordSize->wordHeight 		= 32;
	para->wordSize->validWidth 		= para->wordSize->wordWidth;
	para->wordSize->dataStringLengh = 130;
	
	Drv_Font_SendCmd(FONT_WAKEUP_CMD);
	Drv_Font_ReadUI(para->code.codeIndexUI,para->dataAddr);
	Drv_Font_SendCmd(FONT_SLEEP_CMD);
	#endif
	return 0;
}


//**********************************************************************
// ��������:	�ֿ�оƬ�Լ�
// ���������	��
// ���ز�����	
// 0x00    :    �Լ�ͨ��
// 0xff    :    �Լ�ʧ��
uint16_t Mid_Font_SelfTest(void)
{
	uint16 ret = 0;
	
	ret = Drv_Font_SelfTest();
	
	return ret;
}

//**********************************************************************
// ��������:	��ȡ�ֵĿ��ߡ���Ч�����Ϣ
// ���������	��
// ���ز�����	
// 0x00    :    ����ͨ��
// 0xff    :    ����ʧ��
uint16 Mid_Font_ReadFrontSize(font_para_t *para)
{
	Mid_Front_GetFontSize(para->sizeKind,&para->wordSize);
	if(para->wordSize.dataStringLengh == 0)
	{
		return 0xff;
	}

	if( (para->sizeKind == ASCII_SIZE_12_B_A)||
		(para->sizeKind == ASCII_SIZE_12_B_T)||
		(para->sizeKind == ASCII_SIZE_16_A)||
		(para->sizeKind == ASCII_SIZE_16_T)
	)
	{	
		Drv_Font_SendCmd(FONT_WAKEUP_CMD);
		Drv_Font_ReadASCII(para->code.codeASCII,para->sizeKind,para->dataAddr);
		Drv_Font_SendCmd(FONT_SLEEP_CMD);

		para->wordSize.validWidth 	= (uint16)para->dataAddr[0] << 8 | para->dataAddr[1];
	}
	else
	{
		para->wordSize.validWidth 	= para->wordSize.wordWidth;
	}
	return 0;
}








