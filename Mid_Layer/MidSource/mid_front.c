/**********************************************************************
**
**模块说明: mid层字库接口
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.25  修改流程  ZSL  
**
**********************************************************************/
#include "mid_front.h"


/* FreeRTOS includes */
#include "rtos.h"

/*************** func declaration ********************************/
static uint16 Mid_Front_Enable(void);
static uint16 Mid_Front_Disable(void);
static uint16 Mid_Front_WakeUp(void);
static uint16 Mid_Front_Sleep(void);
static uint16 Mid_Front_ReadGB(front_para_t *para);
static uint16 Mid_Front_ReadASCII(front_para_t *para);
static uint16 Mid_Front_ReadUI(front_para_t *para);
static uint16 Mid_Front_ReadUnicode(front_para_t *para);
static uint16 Mid_Front_ReadFrontSize(front_para_t *para);
static uint16 Mid_Front_SelfTest(void);

/*******************macro define*******************/

// cb ID max
#define		FRONT_CB_ID_MAX				32

/************** variable define *****************************/

typedef struct 
{
	uint8_t				occupy;
	uint8_t				result;
	SemaphoreHandle_t	frontCbSemaphore;
}extflash_cb_s;

static extflash_cb_s		frontSemaphore[FRONT_CB_ID_MAX];				/**< Queue handler extflash cb queue*/

static SemaphoreHandle_t    frontMutex;

/*******************function define*******************/

//**********************************************************************
// 函数功能:	front 软件初始化，队列创建
// 输入参数：	无
// 返回参数：	无
void Mid_Front_Init(void)
{
	static	uint8 frontInitFlag = 0;
	static	uint8 i;
	if(frontInitFlag == 0)
	{
		for(i = 0; i < FRONT_CB_ID_MAX; i++)
		{
			frontSemaphore[i].occupy					= 0;
			frontSemaphore[i].result					= 0;
			frontSemaphore[i].frontCbSemaphore		= xSemaphoreCreateBinary();
		}

		// 创建flash的二值量资源互斥保护
		frontMutex   = xSemaphoreCreateMutex();

		frontInitFlag	= 1;
	}

	Drv_Font_Open();
	Drv_Font_SendCmd(FONT_SLEEP_CMD);
	Drv_Font_Close();
}

//*******************************************************************
// 函数功能:	字库芯片使能
// 输入参数：	无
// 返回参数：	
// 0x00    :    设置成功
// 0xff    :    设置失败
static uint16 Mid_Front_Enable(void)
{
	return Drv_Font_Open();
}

//*******************************************************************
// 函数功能:	字库芯片关闭
// 输入参数：	无
// 返回参数：	
// 0x00    :    设置成功
// 0xff    :    设置失败
static uint16 Mid_Front_Disable(void)
{
	return Drv_Font_Close();
}

//**********************************************************************
// 函数功能:	根据字形大小返回宽、高、数据流长度信息
// 输入参数：	
// para 	：  字库的字体类型
// 返回参数：
//     	StreamLen：数据流长度
//      0x00    :  数据流长度读取失败
uint8 Mid_Front_GetFontSize(uint8 sizeKind,front_size_t *sizeTemp)
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

		case GB_SIZE_12X12:	
		sizeTemp->wordWidth 		= 12;
		sizeTemp->wordHeight 		= 12;
		sizeTemp->dataStringLengh 	= 24;
		break;

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
static uint16 Mid_Front_ReadGB(front_para_t *para)
{
	if (para->sizeKind != GB_SIZE_16X16 && para->sizeKind != GB_SIZE_12X12)
	{
		return 0xff;
	}
	Mid_Front_GetFontSize(para->sizeKind,para->wordSize);
	para->wordSize->validWidth 		= para->wordSize->wordWidth;
	Drv_Font_SendCmd(FONT_WAKEUP_CMD);
	Drv_Font_ReadGB2313(para->code.codeGB,para->sizeKind,para->dataAddr);
	Drv_Font_SendCmd(FONT_SLEEP_CMD);
	return 0;
}

//*******************************************************************
// 函数功能:	根据UNICODE编码读取字（汉字或ASCII）数据流
// 输入参数：	
// para 	：  字库事件参数信息，包含字（汉字或ASCII）编号及数据流指针
// 返回参数：	
// 0x00    :    设置成功
// 0xff    :    设置失败
static uint16 Mid_Front_ReadUnicode(front_para_t *para)
{
	uint16 codeTemp;

	if (para->code.codeUnicode < 0x0080)//ASCII码
	{
		Mid_Front_ReadASCII(para);	
	}
	else
	{
		Drv_Font_SendCmd(FONT_WAKEUP_CMD);
		codeTemp 			= Drv_Font_Unicode2GB(para->code.codeUnicode);
		para->code.codeGB 	= codeTemp;
		Mid_Front_ReadGB(para);
		//bspFront.Sleep();		
    }
    return 0;
}

//**********************************************************************
// 函数功能:	根据ASCII码读取字母数据流
// 输入参数：	
// para 	：  字库事件参数信息，包含ASCII码及数据流指针及长度
// 返回参数：	
// 0x00    :    操作成功
// 0xff    :    操作失败
static uint16 Mid_Front_ReadASCII(front_para_t *para)
{
	uint16 i;
	uint16 codeTemp;

	Mid_Front_GetFontSize(para->sizeKind,para->wordSize);
	if(para->wordSize->dataStringLengh == 0)
	{
		return 0xff;
	}
	
	if (para->sizeKind == GB_SIZE_16X16 )//此两种字型由GB函数支持，但GB码需自行转换
	{
		codeTemp 			=  0xA3A0 - 0x0020 + para->code.codeASCII;
		para->code.codeGB 	= codeTemp;
		Mid_Front_ReadGB(para);
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
		para->wordSize->validWidth 	= (uint16)para->dataAddr[0] << 8 | para->dataAddr[1];

		for(i = 0; i < (para->wordSize->dataStringLengh - 2); i++)//去掉有效字宽信息(前2字节)
		{
			para->dataAddr[i]			= para->dataAddr[i + 2];
		}
		para->wordSize->dataStringLengh -= 2;
	}
	else
	{
		para->wordSize->validWidth 	= para->wordSize->wordWidth;
	}
	return 0;
}

//**********************************************************************
// 函数功能: 根据UI码读取UI数据流
// 输入参数：	
// para    ： 字库事件参数信息，包含UI码及数据流指针及长度
// 返回参数：	
// 0x00    :   操作成功
// 0xff    :   操作失败
static uint16 Mid_Front_ReadUI(front_para_t *para)
{
	para->wordSize->wordWidth 		= 32;
	para->wordSize->wordHeight 		= 32;
	para->wordSize->validWidth 		= para->wordSize->wordWidth;
	para->wordSize->dataStringLengh = 130;
	
	Drv_Font_SendCmd(FONT_WAKEUP_CMD);
	Drv_Font_ReadUI(para->code.codeIndexUI,para->dataAddr);
	Drv_Font_SendCmd(FONT_SLEEP_CMD);
	return 0;
}

//**********************************************************************
// 函数功能:	字库芯片唤醒
// 输入参数：	无
// 返回参数：	
// 0x00    :    设置成功
// 0xff    :    设置失败
static uint16 Mid_Front_WakeUp(void)
{
	return Drv_Font_SendCmd(FONT_WAKEUP_CMD);
}

//**********************************************************************
// 函数功能:	字库芯片睡眠
// 输入参数：	无
// 返回参数：	
// 0x00    :    设置成功
// 0xff    :    设置失败
static uint16 Mid_Front_Sleep(void)
{
	return Drv_Font_SendCmd(FONT_SLEEP_CMD);
}

//**********************************************************************
// 函数功能:	字库芯片自检
// 输入参数：	无
// 返回参数：	
// 0x00    :    自检通过
// 0xff    :    自检失败
static uint16 Mid_Front_SelfTest(void)
{
	uint16 ret = 0;
	
	Drv_Font_SendCmd(FONT_WAKEUP_CMD);
	ret = Drv_Font_SelfTest();
	Drv_Font_SendCmd(FONT_SLEEP_CMD);
	
	return ret;
}

//**********************************************************************
// 函数功能:	读取字的宽、高、有效宽度信息
// 输入参数：	无
// 返回参数：	
// 0x00    :    操作通过
// 0xff    :    操作失败
static uint16 Mid_Front_ReadFrontSize(front_para_t *para)
{
	Mid_Front_GetFontSize(para->sizeKind,para->wordSize);
	if(para->wordSize->dataStringLengh == 0)
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

		para->wordSize->validWidth 	= (uint16)para->dataAddr[0] << 8 | para->dataAddr[1];
	}
	else
	{
		para->wordSize->validWidth 	= para->wordSize->wordWidth;
	}
	return 0;
}


//**********************************************************************
// 函数功能:	flash函数回调,同步二值量
// 输入参数：	
// queueId 	:	指定二值同步信号id	
// 返回参数：	无
static void FrontCb(uint8_t* queueId, uint8_t result)
{
	if(*queueId < FRONT_CB_ID_MAX)
	{
		frontSemaphore[*queueId].result		= result;
		xSemaphoreGive(frontSemaphore[*queueId].frontCbSemaphore);
	}
}


//**********************************************************************
// 函数功能:	front事件放入队列前预处理
// 输入参数：	
// msg 	:		事件指针	
// 返回参数：	0x00:操作成功
// 				0xff:操作失败
uint16_t Mid_Front_EventSetPre(front_event_t *msg)
{
	uint8_t i;

	switch(msg->id)
	{
		case FRONT_EVENT_ENABLE:
		case FRONT_EVENT_DISABLE:
		case FRONT_EVENT_READ_GB:
		case FRONT_EVENT_READ_ASCII:
		case FRONT_EVENT_WAKEUP:
		case FRONT_EVENT_SLEEP:
		case FRONT_EVENT_SELFTEST:
		case FRONT_EVENT_READ_UNICODE:
		case FRONT_EVENT_READ_FRONT_SIZE:

		xSemaphoreTake(frontMutex, portMAX_DELAY);
		for(i = 0; i < FRONT_CB_ID_MAX; i++)
		{
			if(frontSemaphore[i].occupy == 0)
			{
				frontSemaphore[i].occupy = 1;	
				break;
			}
		}

		if(i < FRONT_CB_ID_MAX)
		{
			msg->para.cbId		= i;
			xSemaphoreGive(frontMutex);
			msg->para.Cb		= FrontCb;
			return 0x00;
		}

		msg->para.Cb		= NULL;
		msg->para.cbId		= 0xff;
		xSemaphoreGive(frontMutex);
		break;

		default:
		return 0xff;
	}
	// 检测是否有空闲的二值量
		
	return 0xff;
}

//**********************************************************************
// 函数功能:	任务队列放置不进时，需要进行清除二值资源，对其解除占用
// 输入参数：	
// msg 	:		事件指针	
// 返回参数：	0x00:操作成功
// 				0xff:操作失败
uint16_t Mid_Front_EventSetFail(front_event_t *msg)
{
	switch(msg->id)
	{
		case FRONT_EVENT_ENABLE:
		case FRONT_EVENT_DISABLE:
		case FRONT_EVENT_READ_GB:
		case FRONT_EVENT_READ_ASCII:
		case FRONT_EVENT_WAKEUP:
		case FRONT_EVENT_SLEEP:
		case FRONT_EVENT_SELFTEST:
		case FRONT_EVENT_READ_UNICODE:
		case FRONT_EVENT_READ_FRONT_SIZE:
		
		xSemaphoreTake(frontMutex, portMAX_DELAY);
		frontSemaphore[msg->para.cbId].occupy = 0;	
		xSemaphoreGive(frontMutex);
		break;

		default:
		return 0xff;
	}
	return 0x00;
}

//**********************************************************************
// 函数功能:	等待时间在flash任务中执行结束
// 输入参数：	
// queueId 	:	指定二值同步信号id	
// 返回参数：	0x00:操作成功
// 				0xff:操作失败
uint16_t Mid_Front_WaitComplete(front_event_t *msg)
{

	if(msg->para.cbId >= FRONT_CB_ID_MAX)
		return 0xff;

	switch(msg->id)
	{
		case FRONT_EVENT_ENABLE:
		case FRONT_EVENT_DISABLE:
		case FRONT_EVENT_READ_GB:
		case FRONT_EVENT_READ_ASCII:
		case FRONT_EVENT_WAKEUP:
		case FRONT_EVENT_SLEEP:
		case FRONT_EVENT_SELFTEST:
		case FRONT_EVENT_READ_UNICODE:
		case FRONT_EVENT_READ_FRONT_SIZE:
		// 等待事件放入，
		xSemaphoreTake(frontSemaphore[msg->para.cbId].frontCbSemaphore, portMAX_DELAY);
		xSemaphoreTake(frontMutex, portMAX_DELAY);
		frontSemaphore[msg->para.cbId].occupy = 0;
		xSemaphoreGive(frontMutex);
		break;

		default:
		return 0xff;
	}

	return frontSemaphore[msg->para.cbId].result;
}





//**********************************************************************
// 函数功能:	根据事件ID进行处理
// 输入参数：	
// queueId 	:	指定二值同步信号id	
// 返回参数：	0x00:操作成功
// 				0xff:操作失败
uint16_t Mid_Front_EventProcess(front_event_t *msg)
{
	msg->para.result		= 0x00;
	switch(msg->id)
	{
		case FRONT_EVENT_ENABLE:
		Mid_Front_Enable();
		break;
				
		case FRONT_EVENT_DISABLE:
		Mid_Front_Disable();
		break;
				
		case FRONT_EVENT_READ_GB:
		Mid_Front_Enable();
		Mid_Front_ReadGB(&msg->para);
		Mid_Front_Disable();
		break;

		case FRONT_EVENT_READ_UNICODE:
		Mid_Front_Enable();
		Mid_Front_ReadUnicode(&msg->para);
		Mid_Front_Disable();
		break;
		
		case FRONT_EVENT_SELFTEST:
		Mid_Front_Enable();
		msg->para.result = Mid_Front_SelfTest();
		Mid_Front_Disable();
		break;

		case FRONT_EVENT_READ_ASCII:
		Mid_Front_Enable();
		Mid_Front_ReadASCII(&msg->para);
		Mid_Front_Disable();
		break;

		case FRONT_EVENT_READ_UI:
		Mid_Front_Enable();
		Mid_Front_ReadUI(&msg->para);
		Mid_Front_Disable();
		break;	
				
		case FRONT_EVENT_WAKEUP:
		Mid_Front_Enable();
		Mid_Front_WakeUp();
		Mid_Front_Disable();
		break;
				
		case FRONT_EVENT_SLEEP:
		Mid_Front_Enable();
		Mid_Front_Sleep();
		Mid_Front_Disable();
		break;

		case FRONT_EVENT_READ_FRONT_SIZE:
		Mid_Front_Enable();
		Mid_Front_ReadFrontSize(&msg->para);
		Mid_Front_Disable();
		break;

		default:
		msg->para.result		= 0x00;
		break;
	}
	if(msg->para.Cb != NULL)
	{
		msg->para.Cb(&msg->para.cbId,  msg->para.result);
	}

	return 0;
}

