/**********************************************************************
**
**模块说明: mid层字库接口
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.25  修改流程  ZSL  
**
**********************************************************************/
#include "mid_front.h"
#include "rtos.h"
#include "mid_scheduler.h"

// 获取SPI资源
static uint32_t Mid_Font_SpiMutexTake(void)
{
	// 获取外设访问限权
	xSemaphoreTake(SPI_I2C_M0_SemaphoreHandle, portMAX_DELAY);
	
	// 打开外设
	Drv_Font_Open();
}

// 释放SPI资源
static uint32_t Mid_Font_SpiMutexGive(void)
{
	// 关闭外设
	Drv_Font_Close();
	
	// 释放外设访问限权
	xSemaphoreGive(SPI_I2C_M0_SemaphoreHandle);
}

//**********************************************************************
// 函数功能:	front 软件初始化，队列创建
// 输入参数：	无
// 返回参数：	无
void Mid_Font_Init(void)
{
	Drv_Font_Init();
}

//**********************************************************************
// 函数功能:	根据字形大小返回宽、高、数据流长度信息
// 输入参数：	
// para 	：  字库的字体类型
// 返回参数：
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
// 函数功能: 根据汉字编码读取汉字数据流
// 输入参数：	
// para    ：字库事件参数信息，包含汉字编号及数据流指针
// 返回参数：	
// 0x00    :  设置成功
// 0xff    :  设置失败
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
// 函数功能:	根据UNICODE编码读取字（汉字或ASCII）数据流
// 输入参数：	
// para 	：  字库事件参数信息，包含字（汉字或ASCII）编号及数据流指针
// 返回参数：	
// 0x00    :    设置成功
// 0xff    :    设置失败
uint16 Mid_Font_ReadUnicode(font_para_t *para)
{
	uint16 codeTemp;
	
//	if (para->code.codeUnicode < 0x0080)//ASCII码
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
// 函数功能:	根据ASCII码读取字母数据流
// 输入参数：	
// para 	：  字库事件参数信息，包含ASCII码及数据流指针及长度
// 返回参数：	
// 0x00    :    操作成功
// 0xff    :    操作失败
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
	
	if (para->sizeKind == GB_SIZE_16X16 )//此两种字型由GB函数支持，但GB码需自行转换
	{
		#if 0	// 此代码未测试过，不可控
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

		for(i = 0; i < (para->wordSize.dataStringLengh - 2); i++)//去掉有效字宽信息(前2字节)
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
// 函数功能: 根据UI码读取UI数据流
// 输入参数：	
// para    ： 字库事件参数信息，包含UI码及数据流指针及长度
// 返回参数：	
// 0x00    :   操作成功
// 0xff    :   操作失败
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
// 函数功能:	字库芯片自检
// 输入参数：	无
// 返回参数：	
// 0x00    :    自检通过
// 0xff    :    自检失败
uint16_t Mid_Font_SelfTest(void)
{
	uint16 ret = 0;
	
	ret = Drv_Font_SelfTest();
	
	return ret;
}

//**********************************************************************
// 函数功能:	读取字的宽、高、有效宽度信息
// 输入参数：	无
// 返回参数：	
// 0x00    :    操作通过
// 0xff    :    操作失败
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








