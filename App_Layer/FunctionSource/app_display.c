#include "platform_common.h"
#include "mid_interface.h"
#include "app_picture_source.h"
#include "app_display.h"
#include "app_ascii_source.h"



/*************** func declaration ********************************/
void App_PicRoundTimerStart(void);
void App_PicRoundTimerStop(void);

/*************** macro define ********************************/
#define  PIC_ROUND_PERIOD      20 //窗口切换周期：单位：APP_1SEC_TICK


/************** variable define *****************************/
TimerHandle_t picRoundTimer;        //滚动显示定时器
SemaphoreHandle_t picRoundSemaphore; //滚动显示同步信息量

uint8           screenState = 0;    //显示屏亮灭状态
uint16          pixelCnt;           //显示滚动像素计数
uint16          pixelDelta;         //滚动偏移量累加
uint16          pixelRoundMax;      //滚动最大偏移量
uint16          pixelRoundUnit;     //滚动像素单位
round_dirct_t   picRoundDirt;       //滚动方向


//**********************************************************************
// 函数功能：   显示关闭
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
void App_DispOff(void)
{
    if (screenState)
    {
        screenState = 0;
        Mid_Screen_LightOff();
    }  
}

//**********************************************************************
// 函数功能：   显示开启
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
void App_DispOn(void)
{
    if (!screenState)
    {
        screenState = 1;
        Mid_Screen_LightOn();
    }
}

//**********************************************************************
// 函数功能：   显示更新
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
void App_DispFresh(pic_info_s  *imageInfo)
{
    Mid_Screen_UpdataAreaToDispRam(imageInfo); //复制图像缓存数据到显示缓存[目前固定全部复制]
    Mid_Screen_DispArea(imageInfo);            //显示区域到屏
    App_DispOn();                       //屏亮
}

//**********************************************************************
// 函数功能：    清除图像缓存
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
void App_PicRamClear(void)
{
    App_PicRoundTimerStop();
    xSemaphoreTake(picRoundSemaphore, 1);
    Mid_Screen_InitPicRam(0);
}

//**********************************************************************
// 函数功能：    显示滚动定时器启动
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
void App_PicRoundTimerStart(void)
{
    if (xTimerIsTimerActive(picRoundTimer) == pdFALSE)
    {
        xTimerReset(picRoundTimer, 1);
        xTimerStart(picRoundTimer, 1);
    }
}

//**********************************************************************
// 函数功能：    显示滚动定时器停止
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
void App_PicRoundTimerStop(void)
{
    // if (xTimerIsTimerActive(picRoundTimer) != pdFALSE)
    // {    
        xTimerStop(picRoundTimer, 2);
    // }
}

//**********************************************************************
// 函数功能：    滚动事件处理
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
void App_EventRoundProcess(void)
{
    pic_info_s  imageInfo;

    imageInfo.axis_X    = 0;
    imageInfo.axis_Y    = 0;
    imageInfo.height    = 32;
    imageInfo.length    = 96;
    imageInfo.dispmode  = EXRULE;

    pixelCnt ++;
    pixelDelta += pixelRoundUnit;
    if (pixelCnt >= pixelRoundMax)
    {   
         App_PicRoundTimerStop();
         App_DispFresh(&imageInfo);
         xSemaphoreGive(picRoundSemaphore);

    } 
    else
    {  
        Mid_Screen_RoundPicRamToDisRam(false, picRoundDirt, pixelDelta);
        Mid_Screen_DispArea(&imageInfo);            //显示区域到屏
        App_DispOn();                               //屏亮
    }
}

//**********************************************************************
// 函数功能：    显示模块初始化
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
void App_DispInit(void (*TimerCb)(TimerHandle_t xTimer))
{
    picRoundTimer     = xTimerCreate("picRound_T", PIC_ROUND_PERIOD, pdTRUE, 0, TimerCb);
    picRoundSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(picRoundSemaphore);
}

//**********************************************************************
// 函数功能：    显示刷新
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
void App_RoundDisp(round_dirct_t dect,uint8 pixel)
{  
	pic_info_s  imageInfo;

    App_PicRoundTimerStop();
    if (dect == ROUND_NONE)
    {
        picRoundDirt        = dect;

        imageInfo.axis_X    = 0;
        imageInfo.axis_Y    = 0;
        imageInfo.height    = 32;
        imageInfo.length    = 96;
        imageInfo.dispmode  = EXRULE;

        App_DispFresh(&imageInfo);
        xSemaphoreGive(picRoundSemaphore);
    }
    else
    {       
        picRoundDirt   = dect;
        pixelRoundUnit = pixel;
        Mid_Screen_RoundPicRamToDisRam(true, picRoundDirt, pixelRoundUnit);
        if (dect == ROUND_UP || dect == ROUND_DOWN)
        {
            pixelRoundMax = 32 / pixelRoundUnit;
        }else if (dect == ROUND_LEFT || dect == ROUND_RIGHT)
        {
            pixelRoundMax = 96 / pixelRoundUnit;
        }
        else
        {
            pixelRoundMax = 0;
        }
        pixelCnt    = 0;
        pixelDelta  = 0;    
        App_PicRoundTimerStart();       
    }
}

//**********************************************************************
// 函数功能： 返回居中的坐标原点
// 输入参数： leterInfoTemp 需修改的结构体参数
//    		  height 区域高度
//    		  length 区域长度 
// 返回参数： 
static void App_Disp_AlignOffset(letter_info_s  *leterInfoTemp, uint8_t height, uint8_t length)
{
    uint8_t axisX,axisY;
    
    switch(leterInfoTemp->alic)
    {
        case ALIGN_CENTER:
            axisX = (leterInfoTemp->letterAxisInfo.length - length) / 2 + leterInfoTemp->letterAxisInfo.axis_X;
            axisY = (leterInfoTemp->letterAxisInfo.height - height) / 2 + leterInfoTemp->letterAxisInfo.axis_Y;
            break;
        default:
            axisX = (leterInfoTemp->letterAxisInfo.length - length) / 2 + leterInfoTemp->letterAxisInfo.axis_X;
            axisY = (leterInfoTemp->letterAxisInfo.height - height) / 2 + leterInfoTemp->letterAxisInfo.axis_Y;     
            break;
    }
    
    leterInfoTemp->letterAxisInfo.axis_X = axisX;
    leterInfoTemp->letterAxisInfo.axis_Y = axisY;
}

//**********************************************************************
// 函数功能： 居中显示自定义字体，
// 输入参数： 调用方式如下：
//    uint8_t numLen = 8;
//    uint8_t numString[8] = {2, 3, 10, 3, 1, 10, 4, 5};
//    Mid_Screen_FontTgl24AlignDisp(&imageInfo, numString, numLen);   
// 返回参数： 
void Mid_Screen_FontTgl24AlignDisp(letter_info_s *leterInfoTemp, uint8_t *numString, uint8_t numLen)
{
    uint8_t dispLength = 0;
    for (uint8_t i = 0; i < numLen; i++)
    {
		// 防越界处理
		if(numString[i] >= (FONT_TGL_24_NUM-1))
			numString[i] = 16;	// 16是空的占位符
		
        dispLength += FONT_TGL_24[numString[i]][0];
    }
    App_Disp_AlignOffset(leterInfoTemp, 20, dispLength);
    leterInfoTemp->letterAxisInfo.dispmode  = INCRULE;          // 自定义字体全是INCRULE
    for (uint8_t i = 0; i < numLen; i++)
    {
        Mid_Screen_UpdataToPicRam(&leterInfoTemp->letterAxisInfo, FONT_TGL_24[numString[i]]);
        leterInfoTemp->letterAxisInfo.axis_X += FONT_TGL_24[numString[i]][0];
    }
}

//**********************************************************************
// 函数功能： 显示字符串
// 输入参数： 
// 返回参数： 
void App_Disp_StrDisp(letter_source_s *leterInfoSourceTemp)
{
    letter_info_s letterInfo;
    uint16 strCnt;
    uint8 strLenth = 0;

    uint8 frontStreamBuf[130] = {0};
    flash_task_msg_t flashMsg;
    front_size_t frontsizeinfo;

    uint16 codeTemp;

    letterInfo.letterAxisInfo.axis_X = leterInfoSourceTemp->letterInfo.letterAxisInfo.axis_X;
    letterInfo.letterAxisInfo.axis_Y = leterInfoSourceTemp->letterInfo.letterAxisInfo.axis_Y;
    letterInfo.letterAxisInfo.height = leterInfoSourceTemp->letterInfo.letterAxisInfo.height;
    letterInfo.letterAxisInfo.length = leterInfoSourceTemp->letterInfo.letterAxisInfo.length;
    letterInfo.letterAxisInfo.dispmode = leterInfoSourceTemp->letterInfo.letterAxisInfo.dispmode;

    switch (leterInfoSourceTemp->letterInfo.language)
    {
    case ENGLISH_TYPE:
        strLenth = 0;
        flashMsg.id = FRONT_ID;
        flashMsg.flash.frontEvent.id = FRONT_EVENT_READ_ASCII;
        flashMsg.flash.frontEvent.para.dataAddr = frontStreamBuf;
        flashMsg.flash.frontEvent.para.sizeKind = leterInfoSourceTemp->letterInfo.asciiLetterSize;

        for (strCnt = 0; strCnt < leterInfoSourceTemp->letterStrLen; strCnt++)
        {
            codeTemp = (uint16)leterInfoSourceTemp->letterStrAddr[strCnt];
            flashMsg.flash.frontEvent.para.code.codeASCII = codeTemp;
            flashMsg.flash.frontEvent.para.wordSize = &frontsizeinfo;
            FlashTask_EventSet(&flashMsg);

            strLenth += frontsizeinfo.validWidth;
        }

        App_Disp_AlignOffset(&letterInfo, frontsizeinfo.wordHeight, strLenth);

        for (strCnt = 0; strCnt < leterInfoSourceTemp->letterStrLen; strCnt++)
        {
            codeTemp = (uint16)leterInfoSourceTemp->letterStrAddr[strCnt];
            flashMsg.flash.frontEvent.para.code.codeASCII = codeTemp;
            flashMsg.flash.frontEvent.para.wordSize = &frontsizeinfo;
            FlashTask_EventSet(&flashMsg);

            letterInfo.letterAxisInfo.height = frontsizeinfo.wordHeight;
            letterInfo.letterAxisInfo.length = frontsizeinfo.wordWidth;
            Mid_Screen_UpdataToPicRam(&letterInfo.letterAxisInfo, frontStreamBuf);
            letterInfo.letterAxisInfo.axis_X += frontsizeinfo.validWidth;
        }
        break;

    case CHINESE_TYPE:
        strLenth = 0;
        flashMsg.id = FRONT_ID;
        flashMsg.flash.frontEvent.id = FRONT_EVENT_READ_GB;
        flashMsg.flash.frontEvent.para.dataAddr = frontStreamBuf;
        flashMsg.flash.frontEvent.para.sizeKind = leterInfoSourceTemp->letterInfo.gbLetterSize;

        for (strCnt = 0; strCnt < leterInfoSourceTemp->letterStrLen; strCnt += 2)
        {
            codeTemp = (uint16)leterInfoSourceTemp->letterStrAddr[strCnt] << 8 | (uint16)leterInfoSourceTemp->letterStrAddr[strCnt + 1];
            flashMsg.flash.frontEvent.para.code.codeGB = codeTemp;
            flashMsg.flash.frontEvent.para.wordSize = &frontsizeinfo;
            FlashTask_EventSet(&flashMsg);

            strLenth += frontsizeinfo.validWidth;
        }

        App_Disp_AlignOffset(&letterInfo, frontsizeinfo.wordHeight, strLenth);

        for (strCnt = 0; strCnt < leterInfoSourceTemp->letterStrLen; strCnt += 2)
        {
            codeTemp = (uint16)leterInfoSourceTemp->letterStrAddr[strCnt] << 8 | (uint16)leterInfoSourceTemp->letterStrAddr[strCnt + 1];
            flashMsg.flash.frontEvent.para.code.codeGB = codeTemp;
            flashMsg.flash.frontEvent.para.wordSize = &frontsizeinfo;
            FlashTask_EventSet(&flashMsg);

            letterInfo.letterAxisInfo.height = frontsizeinfo.wordHeight;
            letterInfo.letterAxisInfo.length = frontsizeinfo.wordWidth;
            Mid_Screen_UpdataToPicRam(&letterInfo.letterAxisInfo, frontStreamBuf);
            letterInfo.letterAxisInfo.axis_X += frontsizeinfo.validWidth;
        }
        break;

    default:
        break;
    }
}

/**************************************************
            显示电池图标
**************************************************/
void BatDisp(letter_info_s  *leterInfoTemp, uint8 batVol)
{
    pic_info_s  imageInfo;

    imageInfo.axis_X    = leterInfoTemp->letterAxisInfo.axis_X;
    imageInfo.axis_Y    = leterInfoTemp->letterAxisInfo.axis_Y;
    imageInfo.height    = leterInfoTemp->letterAxisInfo.height;
    imageInfo.length    = leterInfoTemp->letterAxisInfo.length;
    imageInfo.dispmode  = leterInfoTemp->letterAxisInfo.dispmode;
    
    switch(batVol)
    {
        case BAT_LEVER_0:
            Mid_Screen_UpdataToPicRam(&imageInfo, gImage_ic_battery_0);
            break;
        case BAT_LEVER_1:
            Mid_Screen_UpdataToPicRam(&imageInfo, gImage_ic_battery_1);
            break;
        case BAT_LEVER_2:
            Mid_Screen_UpdataToPicRam(&imageInfo, gImage_ic_battery_2);
            break;
        case BAT_LEVER_3:
            Mid_Screen_UpdataToPicRam(&imageInfo, gImage_ic_battery_3);
            break;
        case BAT_LEVER_4:
            Mid_Screen_UpdataToPicRam(&imageInfo, gImage_ic_battery_4);
            break;
        case BAT_LEVER_5:
            Mid_Screen_UpdataToPicRam(&imageInfo, gImage_ic_battery_5);
            break;
        default:
            Mid_Screen_UpdataToPicRam(&imageInfo, gImage_ic_battery_0);
            break;
    }
}

/***************************************************************
	以下是梁杰定义的函数以及结构体类型
***************************************************************/
//**********************************************************************
// 函数功能： 在任意给定区域按给定格式显示图片
// 输入参数： pic_disp_info_t 图片参数结构体
// 返回参数： 此字符的有效宽度
void AppScreenPictureDisp(pic_disp_info_t* PicDispInfo)
{
	 pic_info_s  imageInfo;
	
	switch(PicDispInfo->Alic)	// 根据对齐方式，修改坐标原点
	{
		case ALIC_MIDDLE:
			imageInfo.axis_X = PicDispInfo->AxisX + (PicDispInfo->AreaLength - PicDispInfo->PictureLength) / 2;
			imageInfo.axis_Y = PicDispInfo->AxisY + (PicDispInfo->AreaHeight - PicDispInfo->PictureHeight) / 2;			
			break;
		case ALIC_LEFT:
			break;
		case ALIC_RIGHT:
			break;
		default:	// 默认居中显示
			imageInfo.axis_X = PicDispInfo->AxisX + (PicDispInfo->AreaLength - PicDispInfo->PictureLength) / 2;
			imageInfo.axis_Y = PicDispInfo->AxisY + (PicDispInfo->AreaHeight - PicDispInfo->PictureHeight) / 2;
			break;
	}
	
	imageInfo.height    = PicDispInfo->PictureHeight;
	imageInfo.length    = PicDispInfo->PictureLength;
	imageInfo.dispmode  = EXRULE;
	Mid_Screen_UpdataToPicRam(&imageInfo, PicDispInfo->DotBuf);		
}

//**********************************************************************
// 函数功能： 存在本地的ASCII值显示
// 输入参数： AxisX X坐标
//			  AxisY Y坐标
//			  CodeValue 
//			  LetterSize 字体类型（大小）
// 返回参数： 此字符的有效宽度
uint8_t AppScreenLocalAsciiDisp(uint8_t AxisX, uint8_t AxisY, uint8_t AsciiCode, letter_size_t LetterSize)
{
	pic_info_s PicInfo;
	uint8_t TmpValidWidth = 0;

	PicInfo.axis_X    = AxisX;
	PicInfo.axis_Y    = AxisY;
	PicInfo.dispmode  = EXRULE;  
	
	if(AsciiCode >= ASCII_LETTER_NUM)
		return TmpValidWidth;
	
	switch (LetterSize)
    {
    	case APP_ASCII_SIZE_8X16:
			PicInfo.length    = AsciiInfo8x16_array[AsciiCode][0];
			PicInfo.height    = AsciiInfo8x16_array[AsciiCode][1];	
			Mid_Screen_UpdataToPicRam(&PicInfo, &AsciiInfo8x16_array[AsciiCode][4]); 
			TmpValidWidth = AsciiInfo8x16_array[AsciiCode][2];
    		break;
    	case APP_ASCII_SIZE_16_A:
			PicInfo.length    = AsciiInfo16A_array[AsciiCode][0];
			PicInfo.height    = AsciiInfo16A_array[AsciiCode][1];	
			Mid_Screen_UpdataToPicRam(&PicInfo, &AsciiInfo16A_array[AsciiCode][4]); 
			TmpValidWidth = AsciiInfo16A_array[AsciiCode][2];
    		break;
		case APP_ASCII_SIZE_12_B_A:
			PicInfo.length    = AsciiInfo12BA_array[AsciiCode][0];
			PicInfo.height    = AsciiInfo12BA_array[AsciiCode][1];	
			Mid_Screen_UpdataToPicRam(&PicInfo, &AsciiInfo12BA_array[AsciiCode][4]);  
			TmpValidWidth = AsciiInfo12BA_array[AsciiCode][2];
			break;
    	default:
    		break;
    }
	
	return TmpValidWidth;
}

//**********************************************************************
// 函数功能： 在任意坐标位置显示一个ASCII字符，仅支持GBK编码
// 输入参数： AxisX X坐标
//			  AxisY Y坐标
//			  CodeValue ASCII编码值
//			  LetterSize 字体类型（大小）
//			  CodeType  编码类型
// 返回参数： 此字符的有效宽度
uint8_t AppScreenGBKAsciiDisp(
	uint8_t AxisX, 					// X坐标		
	uint8_t AxisY, 					// Y坐标
	uint8_t* CodeValue,				// ASCII编码值
	letter_size_t LetterSize,		// 字体大小（类型）
	letter_code_type_t CodeType		// 编码类型
	)
{
	#if 1	// 调用本地FLASH方式
	uint8_t RetVal = 0;
	
	(void)CodeType;
	
	AppScreenLocalAsciiDisp(AxisX, AxisY, *CodeValue, LetterSize);
	
	switch (LetterSize)
    {
    	case APP_ASCII_SIZE_8X16:
			RetVal = AsciiInfo8x16_array[*CodeValue][2];
    		break;
    	case APP_ASCII_SIZE_16_A:
			RetVal = AsciiInfo16A_array[*CodeValue][2];
    		break;
		case APP_ASCII_SIZE_12_B_A:
			RetVal = AsciiInfo12BA_array[*CodeValue][2];
			break;
    	default:
    		break;
    }

	return RetVal;
	#endif
	
	
	#if 0	// 读取字库方式
	pic_info_s PicInfo;
	
    uint8 frontStreamBuf[130] = {0};	// 字体点阵存储buf
    flash_task_msg_t flashMsg;			// 传入的FLASH参数
    front_size_t frontsizeinfo;			// 从FLASH读出的字体参数

	switch(CodeType)
	{
		case LETTER_TYPTE_UTF8: break;
		case LETTER_TYPTE_UNICODE:
			flashMsg.id                                      = FRONT_ID;
			flashMsg.flash.frontEvent.id                     = FRONT_EVENT_READ_UNICODE; 
			flashMsg.flash.frontEvent.para.dataAddr          = frontStreamBuf;
			flashMsg.flash.frontEvent.para.sizeKind          = LetterSize;
			flashMsg.flash.frontEvent.para.code.codeASCII    = *CodeValue;
			flashMsg.flash.frontEvent.para.wordSize          = &frontsizeinfo;
			FlashTask_EventSet(&flashMsg);					
			break;
		case LETTER_TYPTE_GB:
			flashMsg.id                                      = FRONT_ID;
			flashMsg.flash.frontEvent.id                     = FRONT_EVENT_READ_ASCII; 
			flashMsg.flash.frontEvent.para.dataAddr          = frontStreamBuf;
			flashMsg.flash.frontEvent.para.sizeKind          = LetterSize;
			flashMsg.flash.frontEvent.para.code.codeASCII    = *CodeValue;
			flashMsg.flash.frontEvent.para.wordSize          = &frontsizeinfo;
			FlashTask_EventSet(&flashMsg);			
			break;
		default:
			break;
	}
	
	PicInfo.axis_X    = AxisX;
	PicInfo.axis_Y    = AxisY;
	PicInfo.height    = frontsizeinfo.wordHeight;
	PicInfo.length    = frontsizeinfo.wordWidth;
	PicInfo.dispmode  = EXRULE;  
	
	Mid_Screen_UpdataToPicRam(&PicInfo, frontStreamBuf);   	
	
	return frontsizeinfo.validWidth;
	#endif
}

//**********************************************************************
// 函数功能： 在任意坐标位置显示一个中文字符，仅支持GBK编码
// 输入参数： AxisX X坐标
//			  AxisY Y坐标
//			  CodeValue 中文GB2312编码值
//			  LetterSize 字体类型（大小）
//			  CodeType 编码类型
// 返回参数： 此字符的有效宽度
uint8_t AppScreenGBKChineseDisp(
	uint8_t AxisX, 					// X坐标		
	uint8_t AxisY, 					// Y坐标
	uint8_t* CodeValue,				// 中文GB2312编码值
	letter_size_t LetterSize,		// 字体大小（类型）
	letter_code_type_t CodeType		// 编码类型
	)
{
	pic_info_s PicInfo;
	
    uint8 frontStreamBuf[130] = {0};	// 字体点阵存储buf
    flash_task_msg_t flashMsg;			// 传入的FLASH参数
    front_size_t frontsizeinfo;			// 从FLASH读出的字体参数

	flashMsg.id                                      = FRONT_ID;
	flashMsg.flash.frontEvent.id                     = FRONT_EVENT_READ_GB; 
	flashMsg.flash.frontEvent.para.dataAddr          = frontStreamBuf;
	flashMsg.flash.frontEvent.para.sizeKind          = LetterSize;
	flashMsg.flash.frontEvent.para.code.codeGB       = (uint16)(CodeValue[0] << 8 | CodeValue[1]);	 // 中文编码，高位在前
	flashMsg.flash.frontEvent.para.wordSize          = &frontsizeinfo;
	FlashTask_EventSet(&flashMsg);			
	
	PicInfo.axis_X    = AxisX;
	PicInfo.axis_Y    = AxisY;
	PicInfo.height    = frontsizeinfo.wordHeight;
	PicInfo.length    = frontsizeinfo.wordWidth;
	PicInfo.dispmode  = EXRULE;  
	
	Mid_Screen_UpdataToPicRam(&PicInfo, frontStreamBuf);   	
	
	return frontsizeinfo.validWidth;
}

//**********************************************************************
// 函数功能： 在任意坐标位置显示一个字符，仅支持UNICODE编码
// 输入参数： AxisX X坐标
//			  AxisY Y坐标
//			  CodeValue 编码值
//			  LetterSize 字体类型（大小）
//			  CodeType 编码类型
// 返回参数： 此字符的有效宽度
uint8_t AppScreenUnicodeCharDisp(
	uint8_t AxisX, 					// X坐标		
	uint8_t AxisY, 					// Y坐标
	uint8_t* CodeValue,				// 编码值
	letter_size_t LetterSize,		// 字体大小（类型）
	letter_code_type_t CodeType		// 编码类型
	)
{
	pic_info_s PicInfo;
	
    uint8 frontStreamBuf[130] = {0};	// 字体点阵存储buf
    flash_task_msg_t flashMsg;			// 传入的FLASH参数
    front_size_t frontsizeinfo;			// 从FLASH读出的字体参数
	
	if(0 == CodeValue[1])	// Ascii 字符，调用本地FLASH字库
	{
		(void)CodeType;
		
		return AppScreenLocalAsciiDisp(AxisX, AxisY, *CodeValue, LetterSize);
	}
	else
	{
		flashMsg.id                                      = FRONT_ID;
		flashMsg.flash.frontEvent.id                     = FRONT_EVENT_READ_UNICODE; 
		flashMsg.flash.frontEvent.para.dataAddr          = frontStreamBuf;
		flashMsg.flash.frontEvent.para.sizeKind          = LetterSize;
		flashMsg.flash.frontEvent.para.code.codeUnicode = (uint16)(CodeValue[1] << 8 | CodeValue[0]);
		flashMsg.flash.frontEvent.para.wordSize          = &frontsizeinfo;
		FlashTask_EventSet(&flashMsg);						
		
		PicInfo.axis_X    = AxisX;
		PicInfo.axis_Y    = AxisY;
		PicInfo.height    = frontsizeinfo.wordHeight;
		PicInfo.length    = frontsizeinfo.wordWidth;
		PicInfo.dispmode  = EXRULE;  
		
		Mid_Screen_UpdataToPicRam(&PicInfo, frontStreamBuf);   	
		
		return frontsizeinfo.validWidth;	
	}
}

//**********************************************************************
// 函数功能： 获取字符的有效宽度
// 输入参数： CodeValue 编码值
//			  LetterSize 字体类型
//			  CodeType 编码格式
// 返回参数： 此字符的有效宽度
uint8_t AppScreenFrontValidWidthGet(uint8_t* CodeValue, letter_size_t LetterSize, letter_code_type_t CodeType)
{
//	pic_info_s PicInfo;
	
    uint8 frontStreamBuf[130] = {0};	// 字体点阵存储buf
    flash_task_msg_t flashMsg;			// 传入的FLASH参数
    front_size_t frontsizeinfo;			// 从FLASH读出的字体参数
	uint8_t RetVal = 0;
	
	switch(CodeType)
	{
		case LETTER_TYPTE_UTF8: break;
		case LETTER_TYPTE_UNICODE: 
			flashMsg.id                                      = FRONT_ID;
			flashMsg.flash.frontEvent.id                     = FRONT_EVENT_READ_UNICODE; 
			flashMsg.flash.frontEvent.para.dataAddr          = frontStreamBuf;
			flashMsg.flash.frontEvent.para.sizeKind          = LetterSize;
			flashMsg.flash.frontEvent.para.code.codeUnicode = (uint16)(CodeValue[1] << 8 | CodeValue[0]);
			flashMsg.flash.frontEvent.para.wordSize          = &frontsizeinfo;
			FlashTask_EventSet(&flashMsg);		

			RetVal = frontsizeinfo.validWidth;
			break;
		case LETTER_TYPTE_GB:
			if(*CodeValue >= 0x81)	// 中文字符
			{
				flashMsg.id                                      = FRONT_ID;
				flashMsg.flash.frontEvent.id                     = FRONT_EVENT_READ_GB; 
				flashMsg.flash.frontEvent.para.dataAddr          = frontStreamBuf;
				flashMsg.flash.frontEvent.para.sizeKind          = LetterSize;
				flashMsg.flash.frontEvent.para.code.codeGB    = (uint16)(CodeValue[0] << 8 | CodeValue[1]);	 // 中文编码，高位在前
				flashMsg.flash.frontEvent.para.wordSize          = &frontsizeinfo;
				FlashTask_EventSet(&flashMsg);	

				RetVal = frontsizeinfo.validWidth;
			}
			else					// 英文字符
			{
				switch (LetterSize)
				{
					case APP_ASCII_SIZE_8X16:
						RetVal = AsciiInfo8x16_array[*CodeValue][2];				
						break;
					case APP_ASCII_SIZE_16_A:
						RetVal = AsciiInfo16A_array[*CodeValue][2];
						break;
					case APP_ASCII_SIZE_12_B_A:
						RetVal = AsciiInfo12BA_array[*CodeValue][2];
						break;
					default:
						break;
				}
			}
			break;
		default:
			break;
	}

	return RetVal ; //frontsizeinfo.validWidth;
}

//**********************************************************************
// 函数功能： 获取字符的宽度，字符宽度和有效宽度
// 输入参数： CodeValue 编码值
//			  LetterSize 字体类型
//			  CodeType 编码格式
//			  uint8_t* WordWidth 字符宽度指针
//			  uint8_t* ValidWidth 有效宽度指针
// 返回参数： 此字符的有效宽度
void AppScreenFrontWidthGet(uint8_t* CodeValue, letter_size_t LetterSize, letter_code_type_t CodeType, uint8_t* WordWidth, uint8_t* ValidWidth)
{
//	pic_info_s PicInfo;
	
    uint8 frontStreamBuf[130] = {0};	// 字体点阵存储buf
    flash_task_msg_t flashMsg;			// 传入的FLASH参数
    front_size_t frontsizeinfo;			// 从FLASH读出的字体参数	
	
	if(0 == CodeValue[1])	// ASCII值直接从本地FLASH获取
	{
		switch (LetterSize)
		{
			case APP_ASCII_SIZE_8X16:
				*WordWidth = AsciiInfo8x16_array[*CodeValue][0];
				*ValidWidth = AsciiInfo8x16_array[*CodeValue][2];				
				break;
			case APP_ASCII_SIZE_16_A:
				*WordWidth = AsciiInfo16A_array[*CodeValue][0];
				*ValidWidth = AsciiInfo16A_array[*CodeValue][2];
				break;
			case APP_ASCII_SIZE_12_B_A:
				*WordWidth = AsciiInfo12BA_array[*CodeValue][0];
				*ValidWidth = AsciiInfo12BA_array[*CodeValue][2];
				break;
			default:
				break;
		}		
	}
	else	// 从字库读取
	{
		switch(CodeType)
		{
			case LETTER_TYPTE_UTF8: break;
			case LETTER_TYPTE_UNICODE: 
				flashMsg.id                                      = FRONT_ID;
				flashMsg.flash.frontEvent.id                     = FRONT_EVENT_READ_UNICODE; 
				flashMsg.flash.frontEvent.para.dataAddr          = frontStreamBuf;
				flashMsg.flash.frontEvent.para.sizeKind          = LetterSize;
				flashMsg.flash.frontEvent.para.code.codeUnicode = (uint16)(CodeValue[1] << 8 | CodeValue[0]);
				flashMsg.flash.frontEvent.para.wordSize          = &frontsizeinfo;
				FlashTask_EventSet(&flashMsg);					
				break;
			case LETTER_TYPTE_GB:
				if(*CodeValue >= 0x81)	// 中文字符
				{
					flashMsg.id                                      = FRONT_ID;
					flashMsg.flash.frontEvent.id                     = FRONT_EVENT_READ_GB; 
					flashMsg.flash.frontEvent.para.dataAddr          = frontStreamBuf;
					flashMsg.flash.frontEvent.para.sizeKind          = LetterSize;
					flashMsg.flash.frontEvent.para.code.codeGB    = (uint16)(CodeValue[0] << 8 | CodeValue[1]);	 // 中文编码，高位在前
					flashMsg.flash.frontEvent.para.wordSize          = &frontsizeinfo;
					FlashTask_EventSet(&flashMsg);				
				}
				else					// 英文字符
				{
					flashMsg.id                                      = FRONT_ID;
					flashMsg.flash.frontEvent.id                     = FRONT_EVENT_READ_ASCII; 
					flashMsg.flash.frontEvent.para.dataAddr          = frontStreamBuf;
					flashMsg.flash.frontEvent.para.sizeKind          = LetterSize;
					flashMsg.flash.frontEvent.para.code.codeASCII    = *CodeValue;
					flashMsg.flash.frontEvent.para.wordSize          = &frontsizeinfo;
					FlashTask_EventSet(&flashMsg);					
				}
				break;
			default:
				break;
		}

		*WordWidth = frontsizeinfo.wordWidth;
		*ValidWidth = frontsizeinfo.validWidth;		
	}
}

//**********************************************************************
// 函数功能： 在任意位置显示任意Unicode编码字体
// 输入参数： LetterSource letter_source_s结构体
// 返回参数： 无
void AppScreenTextUnicodeDisp(letter_source_s *LetterSource)
{
	uint8_t TmpStrPos = 0;	// 正在处理的字符串位置
	uint8_t TmpAxisX, TmpAxisY; 
	
	TmpAxisX = LetterSource->letterInfo.letterAxisInfo.axis_X;
	TmpAxisY = LetterSource->letterInfo.letterAxisInfo.axis_Y;
	
	if(0 == LetterSource->letterStrLen)
		return;
	
	do
	{
		if(LetterSource->letterStrAddr[TmpStrPos+1] == 0)	// 字符型字体，暂时只把ASCII码值作为字符型字体
		{
			TmpAxisX += AppScreenUnicodeCharDisp(   TmpAxisX, 
												TmpAxisY, 
												&LetterSource->letterStrAddr[TmpStrPos], 
												LetterSource->letterInfo.asciiLetterSize, 
												LetterSource->letterInfo.letterType);			
		}	
		else										// GB中文型字体
		{
			TmpAxisX += AppScreenUnicodeCharDisp(   TmpAxisX, 
												TmpAxisY, 
												&LetterSource->letterStrAddr[TmpStrPos], 
												LetterSource->letterInfo.gbLetterSize, 
												LetterSource->letterInfo.letterType);			
		}
		TmpStrPos += 2;	
	}while(TmpStrPos < LetterSource->letterStrLen);
}

//**********************************************************************
// 函数功能： 在任意位置显示任意GBK编码字体（中英文混合显示）
// 输入参数： LetterSource letter_source_s结构体
// 返回参数： 无
void AppScreenTextGBKDisp(letter_source_s *LetterSource)
{
	uint8_t TmpStrPos = 0;	// 正在处理的字符串位置
	uint8_t TmpAxisX, TmpAxisY; 
	
	TmpAxisX = LetterSource->letterInfo.letterAxisInfo.axis_X;
	TmpAxisY = LetterSource->letterInfo.letterAxisInfo.axis_Y;
	
	if(0 == LetterSource->letterStrLen)
		return;
	
	do
	{
		if(LetterSource->letterStrAddr[TmpStrPos] >= 0x81)		// 中文字符
		{
			TmpAxisX += AppScreenGBKChineseDisp(   TmpAxisX, 
												TmpAxisY, 
												&LetterSource->letterStrAddr[TmpStrPos], 
												LetterSource->letterInfo.gbLetterSize, 
												LetterSource->letterInfo.letterType);
			TmpStrPos += 2;
		}
		else						// 英文字符	
		{
			TmpAxisX += AppScreenGBKAsciiDisp( TmpAxisX, 
											TmpAxisY, 
											&LetterSource->letterStrAddr[TmpStrPos], 
											LetterSource->letterInfo.asciiLetterSize, 
											LetterSource->letterInfo.letterType);
			TmpStrPos++;
		}		
	}while(TmpStrPos < LetterSource->letterStrLen);
}

//**********************************************************************
// 函数功能： 在任意位置显示Unicode编码的字体，并且能自动排版显示（最大两行）
// 输入参数： LetterSource letter_source_s结构体
//			  注意：Unicode所有字符，均占2字节
// 返回参数： 无
void AppScreenTextLayoutUnicodeDisp(letter_source_s *LetterSource)
{
	uint8_t TmpWordWidth[255];				// 每个字符的宽度
	uint8_t TmpValidWidth[255];				// 每个字符的有效宽度
	uint32_t i;	
	uint32_t TmpTotalValidWidth = 0;		// 总字符的有效宽度
	uint32_t TmpDealtWidth = 0;				// 已处理的字符串点阵宽度
	uint32_t TmpStrPos;
	
	// 参数初始化
	memset(TmpWordWidth, 0x00, 255);
	memset(TmpValidWidth, 0x00, 255);

	// 获取需显示字符串的点阵宽度信息
	TmpStrPos = 0;
	TmpTotalValidWidth = 0;
	i = 0;
	do
	{
		if(0 == LetterSource->letterStrAddr[TmpStrPos+1])	// ASCII
		{
			AppScreenFrontWidthGet(&LetterSource->letterStrAddr[TmpStrPos], 
													LetterSource->letterInfo.asciiLetterSize, 
													LetterSource->letterInfo.letterType,
													&TmpWordWidth[i],
													&TmpValidWidth[i]);			
		}
		else
		{
			AppScreenFrontWidthGet(&LetterSource->letterStrAddr[TmpStrPos], 
													LetterSource->letterInfo.gbLetterSize, 
													LetterSource->letterInfo.letterType,
													&TmpWordWidth[i],
													&TmpValidWidth[i]);					
		}

		TmpTotalValidWidth += TmpValidWidth[i];
		i++;
		TmpStrPos += 2;		
	}while(TmpStrPos < LetterSource->letterStrLen);
    
	// 若字符串总长度小于给定的显示范围，则不需要换行，否则需要换行显示
	// 最后一个字符的位置必须大于等于字宽，否则可能显示越界
	if((TmpTotalValidWidth+TmpWordWidth[i]-TmpValidWidth[i]) <= LetterSource->letterInfo.letterAxisInfo.length)	// 一行显示
	{
		switch(LetterSource->letterInfo.alic)
		{
			case ALIC_MIDDLE:	
				LetterSource->letterInfo.letterAxisInfo.axis_X = LetterSource->letterInfo.letterAxisInfo.axis_X + 
					(LetterSource->letterInfo.letterAxisInfo.length - TmpTotalValidWidth) / 2;		// 计算开始显示的X坐标
			LetterSource->letterInfo.letterAxisInfo.axis_Y = LetterSource->letterInfo.letterAxisInfo.axis_Y + 
					(LetterSource->letterInfo.letterAxisInfo.height - LetterSource->letterInfo.frontHeight) / 2;	// 计算开始显示的Y坐标	

				AppScreenTextUnicodeDisp(LetterSource);				
				break;
			case ALIC_LEFT:
			LetterSource->letterInfo.letterAxisInfo.axis_Y = LetterSource->letterInfo.letterAxisInfo.axis_Y + 
					(LetterSource->letterInfo.letterAxisInfo.height - LetterSource->letterInfo.frontHeight) / 2;	// 计算开始显示的Y坐标	
			
				AppScreenTextUnicodeDisp(LetterSource);
				break;
			case ALIC_RIGHT: break;
			default:
				break;
		}		
	}
	else	// 两行显示，多余部分用...代替
	{
		letter_source_s TmpLetterSource;

		memcpy(&TmpLetterSource, LetterSource, sizeof(struct __letter_source_s)); // 复制结构体
		
        TmpLetterSource.letterStrLen = 1;									      // 一次显示一个字节
		
		/* 显示第一行 */
        i = 0;                      // 已处理的字符串计数
        TmpDealtWidth = 0;      	 // 已处理的字符串总宽度        
        TmpLetterSource.letterInfo.letterAxisInfo.axis_Y = 0; 
		do
		{
			TmpLetterSource.letterInfo.letterAxisInfo.axis_X = TmpDealtWidth + LetterSource->letterInfo.letterAxisInfo.axis_X;	
			TmpLetterSource.letterStrAddr = &LetterSource->letterStrAddr[2*i];			
			TmpDealtWidth += TmpValidWidth[i++];
			AppScreenTextUnicodeDisp(&TmpLetterSource);	
		}while((TmpDealtWidth + TmpWordWidth[i]) < LetterSource->letterInfo.letterAxisInfo.length);   // 是否越界以字符刷屏宽度为准，而不是有效宽度
		
		// 第二行能否全部显示
		 TmpLetterSource.letterInfo.letterAxisInfo.axis_Y = 16; 
		if((TmpTotalValidWidth - TmpDealtWidth + TmpWordWidth[LetterSource->letterStrLen / 2]-TmpValidWidth[LetterSource->letterStrLen / 2]) 
			<= LetterSource->letterInfo.letterAxisInfo.length)
		{
			TmpDealtWidth = 0;  // 开始显示第二行，已处理点阵重新计数
			do
			{
				TmpLetterSource.letterInfo.letterAxisInfo.axis_X = TmpDealtWidth + LetterSource->letterInfo.letterAxisInfo.axis_X;	
				TmpLetterSource.letterStrAddr = &LetterSource->letterStrAddr[2*i];			
				TmpDealtWidth += TmpValidWidth[i++];
				AppScreenTextUnicodeDisp(&TmpLetterSource);					
			}while(i < (LetterSource->letterStrLen / 2));
		}
		else	// 不能全部显示，多余部分显示...
		{
			TmpDealtWidth = 0;  // 开始显示第二行，已处理点阵重新计数
			do
			{
				TmpLetterSource.letterInfo.letterAxisInfo.axis_X = TmpDealtWidth + LetterSource->letterInfo.letterAxisInfo.axis_X;	
				TmpLetterSource.letterStrAddr = &LetterSource->letterStrAddr[2*i];			
				TmpDealtWidth += TmpValidWidth[i++];
				AppScreenTextUnicodeDisp(&TmpLetterSource);	
			}while((TmpDealtWidth + TmpValidWidth[i]) < LetterSource->letterInfo.letterAxisInfo.length - 6 - 12);   	// 一个字符'.'实际宽度为12，有效宽度为3							

			/* 显示... */
			uint8_t TmpBuf[255] = {0x2e, 0x00};	// unicode编码字符"."
			uint8_t TmpCnt = 0;
			do
			{
				TmpLetterSource.letterInfo.letterAxisInfo.axis_X = TmpDealtWidth + LetterSource->letterInfo.letterAxisInfo.axis_X;	
				TmpLetterSource.letterStrAddr = TmpBuf;			
				TmpDealtWidth += 3;	// 一个字符'.'占用3个点阵。
				AppScreenTextUnicodeDisp(&TmpLetterSource);	
			}while(++TmpCnt < 3);   	// 结尾显示三个字符'.'
		}
	}	
}

//**********************************************************************
// 函数功能： 在任意位置显示GBK编码的字体（中英文混合显示），并且能自动排版显示（最大两行）
// 输入参数： LetterSource letter_source_s结构体
//			  注意：GBK编码中，英文占1字节，中文占2字节，
// 返回参数： 无
void AppScreenTextLayoutGBKDisp(letter_source_s *LetterSource)
{
	uint8_t TmpStrPos;
	uint8_t TmpStrEachLen[32][2];	// 第一维度保存每个字符串的宽度，第二维度表示此字符串占用的字节数（ASCII占1字节，中文字符占2字节）
	uint8_t TmpStrTotalLen;		// 保存字符串总宽度
	uint8_t	TmpStrDealtdLen;	// 保存已处理的字符串宽度
	uint8_t TmpStrByteLen;		// 保存已处理的字符串字节数
	uint8_t i;
	uint8_t TmpLetterStrLen;
	
	// 获取需显示字符串的点阵总宽度
	TmpStrPos = 0;
	TmpStrTotalLen = 0;
	i = 0;
	do
	{
		if(LetterSource->letterStrAddr[TmpStrPos] >= 0x81)		// 中文字符
		{
			TmpStrEachLen[i][0] = AppScreenFrontValidWidthGet(&LetterSource->letterStrAddr[TmpStrPos], 
													LetterSource->letterInfo.gbLetterSize, 
													LetterSource->letterInfo.letterType);
			TmpStrTotalLen += TmpStrEachLen[i][0];
			TmpStrEachLen[i][1] = 2;
			i++;
			TmpStrPos += 2;
		}
		else						// 英文字符	
		{
			TmpStrEachLen[i][0] = AppScreenFrontValidWidthGet(&LetterSource->letterStrAddr[TmpStrPos], 
												LetterSource->letterInfo.asciiLetterSize, 
												LetterSource->letterInfo.letterType);
			TmpStrTotalLen += TmpStrEachLen[i][0];
			TmpStrEachLen[i][1] = 1;
			i++;
			TmpStrPos++;
		}		
	}while(TmpStrPos < LetterSource->letterStrLen);

	// 若字符串总长度小于给定的显示范围，则不需要换行，否则需要换行显示
	if(TmpStrTotalLen <= LetterSource->letterInfo.letterAxisInfo.length)	// 无需换行显示
	{
		switch(LetterSource->letterInfo.alic)
		{
			case ALIC_MIDDLE:	
				LetterSource->letterInfo.letterAxisInfo.axis_X = LetterSource->letterInfo.letterAxisInfo.axis_X + 
					(LetterSource->letterInfo.letterAxisInfo.length - TmpStrTotalLen) / 2;		// 计算开始显示的X坐标
			LetterSource->letterInfo.letterAxisInfo.axis_Y = LetterSource->letterInfo.letterAxisInfo.axis_Y + 
					(LetterSource->letterInfo.letterAxisInfo.height - LetterSource->letterInfo.frontHeight) / 2;	// 计算开始显示的Y坐标	

				AppScreenTextGBKDisp(LetterSource);				
				break;
			case ALIC_LEFT:
				LetterSource->letterInfo.letterAxisInfo.axis_Y = LetterSource->letterInfo.letterAxisInfo.axis_Y + 
					(LetterSource->letterInfo.letterAxisInfo.height - LetterSource->letterInfo.frontHeight) / 2;	// 计算开始显示的Y坐标		
				AppScreenTextGBKDisp(LetterSource);
				break;
			case ALIC_RIGHT: break;
			default:
				break;
		}		
	}
	else	// 需换行显示
	{
		TmpStrPos = 0;
		TmpStrDealtdLen = 0;
		TmpStrByteLen = 0;
		// 计算第一行需要显示几个字符串，这种"复杂的"处理方式是为了减少不必要的字库flash读写次数，节约功耗
		// 第一行占用的字符数为:i-1
		for(i = 0; TmpStrDealtdLen < LetterSource->letterInfo.letterAxisInfo.length; i++)
		{
			TmpStrDealtdLen += TmpStrEachLen[i][0];
			TmpStrByteLen += TmpStrEachLen[i][1];
		}
		TmpStrDealtdLen -= TmpStrEachLen[i-1][0];	// 第一行显示字符串总宽度（点阵宽度）
		TmpStrByteLen -= TmpStrEachLen[i-1][1];		// 第一行显示字符串编码占用字节数（字节长度）
		
		switch(LetterSource->letterInfo.alic)
		{
			case ALIC_MIDDLE:
				// 显示第一行内容
				TmpLetterStrLen = LetterSource->letterStrLen;
				LetterSource->letterStrLen = TmpStrByteLen;
				AppScreenTextGBKDisp(LetterSource);
			
				// 显示第二行内容
				LetterSource->letterStrLen = TmpLetterStrLen - TmpStrByteLen;									// 剩余需显示的字节数
				LetterSource->letterStrAddr = &LetterSource->letterStrAddr[TmpStrByteLen];			// 显示起始地址
				LetterSource->letterInfo.letterAxisInfo.axis_X = LetterSource->letterInfo.letterAxisInfo.axis_X + 
				(LetterSource->letterInfo.letterAxisInfo.length - (TmpStrTotalLen - TmpStrDealtdLen)) / 2;		// 计算开始显示的X坐标
			    LetterSource->letterInfo.letterAxisInfo.axis_Y = 16;	// 根据H001场景，第二行显示固定位置16

				AppScreenTextGBKDisp(LetterSource);			
				break;				
			case ALIC_LEFT:
				// 显示第一行内容
				TmpLetterStrLen = LetterSource->letterStrLen;
				LetterSource->letterStrLen = TmpStrByteLen;
				AppScreenTextGBKDisp(LetterSource);
			
				// 显示第二行内容
				LetterSource->letterStrLen = TmpLetterStrLen - TmpStrByteLen;									// 剩余需显示的字节数
				LetterSource->letterStrAddr = &LetterSource->letterStrAddr[TmpStrByteLen];			// 显示起始地址
//				LetterSource->letterInfo.letterAxisInfo.axis_X = LetterSource->letterInfo.letterAxisInfo.axis_X + 
//					(LetterSource->letterInfo.letterAxisInfo.length - (TmpStrTotalLen - TmpStrDealtdLen)) / 2;		// 计算开始显示的X坐标
			LetterSource->letterInfo.letterAxisInfo.axis_Y = 16;	// 根据H001场景，第二行显示固定位置16

				AppScreenTextGBKDisp(LetterSource);				
				break;
			case ALIC_RIGHT: break;

			default:
				break;
		}
	}
}

//**********************************************************************
// 函数功能： 时间界面日期显示
// 输入参数： RtcTime RtcTime结构体
// 返回参数： 无
void AppTimeDateDisp(rtc_time_s RtcTime)
{
    letter_source_s LetterSource;
    uint8_t       tempBuf[10]; 

	// 防越界处理
	if(RtcTime.month > 12)
		RtcTime.month = 0;	// 越界后显示0，提示开发者日期显示不正常
	
	if(RtcTime.day > 31)
		RtcTime.day = 0;

	tempBuf[0]     = RtcTime.month/10 + '0';
	tempBuf[1]     = RtcTime.month%10 + '0';  
	tempBuf[2]     = 0x2F;                     // 斜杠
	tempBuf[3]     = RtcTime.day/10 + '0';
	tempBuf[4]     = RtcTime.day%10 + '0';	

	LetterSource.letterInfo.letterAxisInfo.axis_X = 0;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
	LetterSource.letterInfo.letterAxisInfo.height = 16;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 16;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		

	LetterSource.letterStrAddr = tempBuf;
	LetterSource.letterStrLen = 5;			
	AppScreenTextLayoutGBKDisp(&LetterSource);	
	
	#if 0
    uint8       tempBuf[10];
    letter_source_s letterSourceInfo;   

	// 防越界处理
	if(RtcTime.month > 12)
		RtcTime.month = 0;	// 越界后显示0，提示开发者日期显示不正常
	
	if(RtcTime.day > 31)
		RtcTime.day = 0;

	tempBuf[0]     = RtcTime.month/10 + '0';
	tempBuf[1]     = RtcTime.month%10 + '0';  
	tempBuf[2]     = 0x2F;                     // 斜杠
	tempBuf[3]     = RtcTime.day/10 + '0';
	tempBuf[4]     = RtcTime.day%10 + '0';
	
	letterSourceInfo.letterInfo.letterAxisInfo.axis_X    = 0;
	letterSourceInfo.letterInfo.letterAxisInfo.axis_Y    = 0;
	letterSourceInfo.letterInfo.letterAxisInfo.height    = 16;
	letterSourceInfo.letterInfo.letterAxisInfo.length    = 64;
	
	letterSourceInfo.letterInfo.alic = ALIC_MIDDLE;         
	letterSourceInfo.letterInfo.letterType = LETTER_TYPTE_GB;
	letterSourceInfo.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;
	letterSourceInfo.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;
	letterSourceInfo.letterInfo.letterAxisInfo.dispmode = EXRULE;
	letterSourceInfo.letterInfo.language = ENGLISH_TYPE; //CHINESE_TYPE         

	letterSourceInfo.letterStrLen = 5;
	letterSourceInfo.letterStrAddr = tempBuf;       

	App_Disp_StrDisp(&letterSourceInfo);	
	#endif
}

//**********************************************************************
// 函数功能： 时间界面星期显示
// 输入参数： RtcTime RtcTime结构体
// 返回参数： 无
void AppTimeWeekDisp(rtc_time_s RtcTime)
{
    uint8       tempBuf[10];
    letter_source_s LetterSource;

	switch(RtcTime.week)
	{
		case SUN:
			tempBuf[0]    = 'S';
			tempBuf[1]    = 'U';
			tempBuf[2]    = 'N';
			break;
		case MON:
			tempBuf[0]    = 'M';
			tempBuf[1]    = 'O';
			tempBuf[2]    = 'N';
			break;
		case TUE:
			tempBuf[0]    = 'T';
			tempBuf[1]    = 'U';
			tempBuf[2]    = 'E';
			break;
		case WED:
			tempBuf[0]    = 'W';
			tempBuf[1]    = 'E';
			tempBuf[2]    = 'D';
			break;
		case THU:
			tempBuf[0]    = 'T';
			tempBuf[1]    = 'H';
			tempBuf[2]    = 'U';
			break;
		case FRI:
			tempBuf[0]    = 'F';
			tempBuf[1]    = 'R';
			tempBuf[2]    = 'I';
			break;
		case SAT:
			tempBuf[0]    = 'S';
			tempBuf[1]    = 'A';
			tempBuf[2]    = 'T';
			break;
		default:		// 星期错误显示ERR
			tempBuf[0]    = 'E';
			tempBuf[1]    = 'R';
			tempBuf[2]    = 'R';			
			break;
	} 	

	LetterSource.letterInfo.letterAxisInfo.axis_X = 0;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 16;
	LetterSource.letterInfo.letterAxisInfo.height = 16;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 16;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		

	LetterSource.letterStrAddr = tempBuf;
	LetterSource.letterStrLen = 3;			
	AppScreenTextLayoutGBKDisp(&LetterSource);		
	
	#if 0
    uint8       tempBuf[10];
    
    letter_source_s letterSourceInfo;   

	switch(RtcTime.week)
	{
		case SUN:
			tempBuf[0]    = 'S';
			tempBuf[1]    = 'U';
			tempBuf[2]    = 'N';
			break;
		case MON:
			tempBuf[0]    = 'M';
			tempBuf[1]    = 'O';
			tempBuf[2]    = 'N';
			break;
		case TUE:
			tempBuf[0]    = 'T';
			tempBuf[1]    = 'U';
			tempBuf[2]    = 'E';
			break;
		case WED:
			tempBuf[0]    = 'W';
			tempBuf[1]    = 'E';
			tempBuf[2]    = 'D';
			break;
		case THU:
			tempBuf[0]    = 'T';
			tempBuf[1]    = 'H';
			tempBuf[2]    = 'U';
			break;
		case FRI:
			tempBuf[0]    = 'F';
			tempBuf[1]    = 'R';
			tempBuf[2]    = 'I';
			break;
		case SAT:
			tempBuf[0]    = 'S';
			tempBuf[1]    = 'A';
			tempBuf[2]    = 'T';
			break;
		default:		// 星期错误显示ERR
			tempBuf[0]    = 'E';
			tempBuf[1]    = 'R';
			tempBuf[2]    = 'R';			
			break;
	}   
			
    letterSourceInfo.letterInfo.letterAxisInfo.axis_X = 0;
    letterSourceInfo.letterInfo.letterAxisInfo.axis_Y = 16;
    letterSourceInfo.letterInfo.letterAxisInfo.height = 16;
    letterSourceInfo.letterInfo.letterAxisInfo.length = 64;   
    letterSourceInfo.letterInfo.letterAxisInfo.dispmode = EXRULE;  			
			
	letterSourceInfo.letterInfo.alic = ALIC_MIDDLE;         
	letterSourceInfo.letterInfo.letterType = LETTER_TYPTE_GB;
	letterSourceInfo.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;
	letterSourceInfo.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;
	letterSourceInfo.letterInfo.letterAxisInfo.dispmode = EXRULE;
	letterSourceInfo.letterInfo.language = ENGLISH_TYPE;    
            
	letterSourceInfo.letterStrLen = 3;              // 星期显示占3个字符
	letterSourceInfo.letterStrAddr = tempBuf;       

	App_Disp_StrDisp(&letterSourceInfo);
	#endif
}

//**********************************************************************
// 函数功能： 时间界面RTC时间显示
// 输入参数： RtcTime RTC时间，时：分：秒
//			  TimeFormat 时间制式，12小时制/24小时制
// 返回参数： 无
void AppTimeTimeDisp(rtc_time_s RtcTime, time_format_t TimeFormat)
{
    uint8       tempBuf[10], tempLen;
    
    letter_source_s letterSourceInfo;   
    
    letterSourceInfo.letterInfo.letterAxisInfo.axis_X = 0;
    letterSourceInfo.letterInfo.letterAxisInfo.axis_Y = 0;
    letterSourceInfo.letterInfo.letterAxisInfo.height = 32;
    letterSourceInfo.letterInfo.letterAxisInfo.length = 96;   
    letterSourceInfo.letterInfo.letterAxisInfo.dispmode = EXRULE;   
    
	// 数据越界处理
	if(RtcTime.hour > 24)
		RtcTime.hour = 0;
	
	if(RtcTime.min > 60)
		RtcTime.min = 0;

	if(RtcTime.sec > 60)
		RtcTime.sec = 0;	
	
    switch(TimeFormat)
    {
        case TIME_FORMAT_24:                 
            tempLen        = 8;
            tempBuf[0]     = RtcTime.hour / 10;
            tempBuf[1]     = RtcTime.hour % 10;
            tempBuf[2]     = 10;                        // 冒号
            tempBuf[3]     = RtcTime.min  / 10;
            tempBuf[4]     = RtcTime.min  % 10;
            tempBuf[5]     = 10;                        // 冒号
            tempBuf[6]     = RtcTime.sec  / 10;
            tempBuf[7]     = RtcTime.sec  % 10;
            Mid_Screen_FontTgl24AlignDisp(&letterSourceInfo.letterInfo, tempBuf, tempLen);  
            break;
			
        case TIME_FORMAT_12: 
			if(RtcTime.hour >=12)
			{
				tempBuf[0] = 'P';
				AppScreenLocalAsciiDisp(6, 6, *tempBuf, APP_ASCII_SIZE_12_B_A);
			}
			else
			{
				tempBuf[0] = 'A';
				AppScreenLocalAsciiDisp(6, 6, *tempBuf, APP_ASCII_SIZE_12_B_A);
			}

			if(RtcTime.hour >=12)
				RtcTime.hour -= 12;
		
            tempLen        = 8;
            tempBuf[0]     = RtcTime.hour / 10;
            tempBuf[1]     = RtcTime.hour % 10;
            tempBuf[2]     = 10;                        // 冒号
            tempBuf[3]     = RtcTime.min  / 10;
            tempBuf[4]     = RtcTime.min  % 10;
            tempBuf[5]     = 10;                        // 冒号
            tempBuf[6]     = RtcTime.sec  / 10;
            tempBuf[7]     = RtcTime.sec  % 10;        
            Mid_Screen_FontTgl24AlignDisp(&letterSourceInfo.letterInfo, tempBuf, tempLen); 

            break;  
        default:
            break;
        
    }
}

//**********************************************************************
// 函数功能： 时间界面电量状态图标显示
// 输入参数： BatLevel 电量状态，BAT_LEVER_0~5共6个状态
// 返回参数： 无
void AppTimeBatDisp(bat_level_t BatLevel)
{
	pic_disp_info_t PicDispInfo;
	
    PicDispInfo.AxisX = 63;
    PicDispInfo.AxisY = 16;
	PicDispInfo.AreaLength = 32;
	PicDispInfo.AreaHeight = 16;
    PicDispInfo.PictureLength = gImage_ic_battery_0[0];
    PicDispInfo.PictureHeight = gImage_ic_battery_0[1];
	PicDispInfo.Alic = ALIC_MIDDLE;
	
    switch(BatLevel)
    {
        case BAT_LEVER_0:
			PicDispInfo.DotBuf = &gImage_ic_battery_0[2];
			AppScreenPictureDisp(&PicDispInfo);
            break;
        case BAT_LEVER_1:
			PicDispInfo.DotBuf = &gImage_ic_battery_1[2];
			AppScreenPictureDisp(&PicDispInfo);
            break;
        case BAT_LEVER_2:
			PicDispInfo.DotBuf = &gImage_ic_battery_2[2];
			AppScreenPictureDisp(&PicDispInfo);
            break;
        case BAT_LEVER_3:
			PicDispInfo.DotBuf = &gImage_ic_battery_3[2];
			AppScreenPictureDisp(&PicDispInfo);
            break;
        case BAT_LEVER_4:
			PicDispInfo.DotBuf = &gImage_ic_battery_4[2];
			AppScreenPictureDisp(&PicDispInfo);
            break;
        case BAT_LEVER_5:
			PicDispInfo.DotBuf = &gImage_ic_battery_5[2];
			AppScreenPictureDisp(&PicDispInfo);
            break;
        default:
			PicDispInfo.DotBuf = &gImage_ic_battery_0[2];
			AppScreenPictureDisp(&PicDispInfo);
            break;
    }	
}

//**********************************************************************
// 函数功能： 空电池电量图标的显示
// 输入参数： PicDispState 图标状态，开/关
// 返回参数： 无
void AppTimeBatFlick(pic_disp_state_t PicDispState)
{
    pic_disp_info_t PicDispInfo;
    uint8_t TmpBuf[64];
    
    PicDispInfo.AxisX = 63;
    PicDispInfo.AxisY = 16;
    PicDispInfo.AreaLength = 32;
    PicDispInfo.AreaHeight = 16;
    PicDispInfo.PictureLength = gImage_ic_battery_0[0];
    PicDispInfo.PictureHeight = gImage_ic_battery_0[1];
    PicDispInfo.Alic = ALIC_MIDDLE;
    
    switch(PicDispState)
    {
        case PIC_DISP_ON:
            PicDispInfo.DotBuf = &gImage_ic_battery_0[2];
            AppScreenPictureDisp(&PicDispInfo);
            break;

        case PIC_DISP_OFF: 
        memset(TmpBuf, 0x00, 64);
            PicDispInfo.DotBuf = TmpBuf;
            AppScreenPictureDisp(&PicDispInfo);
        break;
        
    }   
}

//**********************************************************************
// 函数功能： 时间界面未接来电图标显示
// 输入参数： PicDispState 图标状态，开/关
// 返回参数： 无
void AppTimeMisscallDisp(pic_disp_state_t PicDispState)
{
	pic_disp_info_t PicDispInfo;
	uint8_t TmpBuf[32];
	
    PicDispInfo.AxisX = 63;
    PicDispInfo.AxisY = 0;
	PicDispInfo.AreaLength = 16;
	PicDispInfo.AreaHeight = 16;
    PicDispInfo.PictureLength = gImage_ic_call_missed_16[0];
    PicDispInfo.PictureHeight = gImage_ic_call_missed_16[1];
	PicDispInfo.Alic = ALIC_MIDDLE;
	
	switch(PicDispState)
	{
		case PIC_DISP_ON:
			PicDispInfo.DotBuf = &gImage_ic_call_missed_16[2];
			AppScreenPictureDisp(&PicDispInfo);
			break;
		case PIC_DISP_OFF:
			memset(TmpBuf, 0x00, 32);
			PicDispInfo.DotBuf = TmpBuf;
			AppScreenPictureDisp(&PicDispInfo);
			break;
	}	
}

//**********************************************************************
// 函数功能： 时间界面蓝牙图标显示
// 输入参数： PicDispState 图标状态，开/关
// 返回参数： 无
void AppTimeBleDisp(pic_disp_state_t PicDispState)
{
	pic_disp_info_t PicDispInfo;
	uint8_t TmpBuf[32];
	
    PicDispInfo.AxisX = 79;
    PicDispInfo.AxisY = 0;
	PicDispInfo.AreaLength = 16;
	PicDispInfo.AreaHeight = 16;
    PicDispInfo.PictureLength = gImage_ic_bt_16[0];
    PicDispInfo.PictureHeight = gImage_ic_bt_16[1];
	PicDispInfo.Alic = ALIC_MIDDLE;
	
	switch(PicDispState)
	{
		case PIC_DISP_ON:
			PicDispInfo.DotBuf = &gImage_ic_bt_16[2];
			AppScreenPictureDisp(&PicDispInfo);
			break;
		case PIC_DISP_OFF:
			memset(TmpBuf, 0x00, 32);
			PicDispInfo.DotBuf = TmpBuf;
			AppScreenPictureDisp(&PicDispInfo);
			break;
        default:
            memset(TmpBuf, 0x00, 32);
            PicDispInfo.DotBuf = TmpBuf;
            AppScreenPictureDisp(&PicDispInfo);           
            break;
	}	
}

//**********************************************************************
// 函数功能： 天气图标及温度显示
// 输入参数： WeatherStatus 天气类型
//			  Temperature 温度值，有符号数
// 返回参数： 无
void AppTimeWeatherDisp(weather_status_t WeatherStatus, int8_t Temperature)
{
	pic_disp_info_t PicDispInfo;
	
    PicDispInfo.AxisX = 0;
    PicDispInfo.AxisY = 0;
	PicDispInfo.AreaLength = 32;
	PicDispInfo.AreaHeight = 32;
    PicDispInfo.PictureLength = 32;
    PicDispInfo.PictureHeight = 32;
	PicDispInfo.Alic = ALIC_MIDDLE;	
	
	// 显示天气状态图标
    switch(WeatherStatus)
    {
        case WEATHER_SUNNY:
			PicDispInfo.DotBuf = &gImage_ic_weather_0000_sunny[2];
			AppScreenPictureDisp(&PicDispInfo);			
            break;
        case WEATHER_CLOUDY:
			PicDispInfo.DotBuf = &gImage_ic_weather_0001_cloudy[2];
			AppScreenPictureDisp(&PicDispInfo);	
            break;
        case WEATHER_OVERCAST:
			PicDispInfo.DotBuf = &gImage_ic_weather_0002_overcast[2];
			AppScreenPictureDisp(&PicDispInfo);	
            break;
        case WEATHER_HAZE:
			PicDispInfo.DotBuf = &gImage_ic_weather_0003_haze[2];
			AppScreenPictureDisp(&PicDispInfo);	
            break;
        case WEATHER_ICERAIN:
			PicDispInfo.DotBuf = &gImage_ic_weather_0004_icerain[2];
			AppScreenPictureDisp(&PicDispInfo);	
            break;
        case WEATHER_FOGGY:
			PicDispInfo.DotBuf = &gImage_ic_weather_0005_foggy[2];
			AppScreenPictureDisp(&PicDispInfo);	
            break;
        case WEATHER_LIGHT_RAIN:
			PicDispInfo.DotBuf = &gImage_ic_weather_0100_light_rain[2];
			AppScreenPictureDisp(&PicDispInfo);	
            break;
        case WEATHER_MODERATE_RAIN:
			PicDispInfo.DotBuf = &gImage_ic_weather_0101_moderate_rain[2];
			AppScreenPictureDisp(&PicDispInfo);	
            break;
        case WEATHER_HEAVY_RAIN:
			PicDispInfo.DotBuf = &gImage_ic_weather_0102_heavy_rain[2];
			AppScreenPictureDisp(&PicDispInfo);	
            break;
        case WEATHER_RAINSTORM:
			PicDispInfo.DotBuf = &gImage_ic_weather_0103_rainstorm[2];
			AppScreenPictureDisp(&PicDispInfo);	
            break;
        case WEATHER_BIG_RAINSTORM:
			PicDispInfo.DotBuf = &gImage_ic_weather_0104_big_rainstorm[2];
			AppScreenPictureDisp(&PicDispInfo);	
            break;
        case WEATHER_SUPER_RAINSTORM:
			PicDispInfo.DotBuf = &gImage_ic_weather_0105_super_rainstorm[2];
			AppScreenPictureDisp(&PicDispInfo);	
            break;
        case WEATHER_SNOW_SHOWER:
			PicDispInfo.DotBuf = &gImage_ic_weather_0200_snow_shower[2];
			AppScreenPictureDisp(&PicDispInfo);	
            break;
        case WEATHER_LIGHT_SNOW:
			PicDispInfo.DotBuf = &gImage_ic_weather_0201_light_snow[2];
			AppScreenPictureDisp(&PicDispInfo);	
            break;
        case WEATHER_MODERATE_SNOW:
			PicDispInfo.DotBuf = &gImage_ic_weather_0202_moderate_snow[2];
			AppScreenPictureDisp(&PicDispInfo);	
            break;
        case WEATHER_HEAVY_SNOW:
			PicDispInfo.DotBuf = &gImage_ic_weather_0203_heavy_snow[2];
			AppScreenPictureDisp(&PicDispInfo);	
            break;
        case WEATHER_BLIZZARD:
			PicDispInfo.DotBuf = &gImage_ic_weather_0204_blizzard[2];
			AppScreenPictureDisp(&PicDispInfo);	
            break;
        case WEATHER_DUST:
			PicDispInfo.DotBuf = &gImage_ic_weather_0300_dust[2];
			AppScreenPictureDisp(&PicDispInfo);	
            break;
        case WEATHER_DUSTBLOW:
			PicDispInfo.DotBuf = &gImage_ic_weather_0301_dustblow[2];
			AppScreenPictureDisp(&PicDispInfo);	
            break;
        case WEATHER_SANDSTORM:
			PicDispInfo.DotBuf = &gImage_ic_weather_0302_sandstorm[2];
			AppScreenPictureDisp(&PicDispInfo);	
            break;
        case WEATHER_STRONG_SANDSTORM:
			PicDispInfo.DotBuf = &gImage_ic_weather_0303_strong_sandstorm[2];
			AppScreenPictureDisp(&PicDispInfo);	
            break;
        case WEATHER_SHOWER:
			PicDispInfo.DotBuf = &gImage_ic_weather_0400_shower[2];
			AppScreenPictureDisp(&PicDispInfo);	
            break;
        case WEATHER_THUNDER_RAIN:
			PicDispInfo.DotBuf = &gImage_ic_weather_0401_thunder_rain[2];
			AppScreenPictureDisp(&PicDispInfo);	
            break;
        case WEATHER_SLEETY:
			PicDispInfo.DotBuf = &gImage_ic_weather_0402_sleety[2];
			AppScreenPictureDisp(&PicDispInfo);	
            break;
        case WEATHER_HAIL:
			PicDispInfo.DotBuf = &gImage_ic_weather_0403_hail[2];
			AppScreenPictureDisp(&PicDispInfo);	
            break;
        default:
            PicDispInfo.DotBuf = &gImage_ic_weather_0001_cloudy[2];
            AppScreenPictureDisp(&PicDispInfo);
            break;
    }	
	
    uint8       tempBuf[10], tempLen;
    letter_source_s letterSourceInfo; 
//	letter_source_s LetterSource;	
	if(Temperature >= 0)
	{
		// 防越界处理
		if(Temperature > 99)
			Temperature = 99;
		
		tempBuf[0] = 16;	// 占位符
		tempBuf[1] = Temperature / 10;   // 十位
		tempBuf[2] = Temperature % 10;   // 个位
	}
	else
	{
		// 防越界处理
		if(Temperature < -99)
			Temperature = -99;
		
		tempBuf[0] = 12;   	// 负号
		tempBuf[1] = (~Temperature + 1) / 10;   // 十位
		tempBuf[2] = (~Temperature + 1) % 10;   // 个位		
	}
	
	tempLen        = 3;
	letterSourceInfo.letterInfo.letterAxisInfo.axis_X = 44;
	letterSourceInfo.letterInfo.letterAxisInfo.axis_Y = 0;
	letterSourceInfo.letterInfo.letterAxisInfo.height = 32;
	letterSourceInfo.letterInfo.letterAxisInfo.length = 26;   
	letterSourceInfo.letterInfo.letterAxisInfo.dispmode = EXRULE;
	Mid_Screen_FontTgl24AlignDisp(&letterSourceInfo.letterInfo, tempBuf, tempLen); 		
	
	tempBuf[0] = 0xA1, tempBuf[1] = 0xE6;		// 显示温度图标 ℃
	AppScreenGBKChineseDisp(72, 6, tempBuf, APP_GB_SIZE_12X12, LETTER_TYPTE_GB);		
}

//**********************************************************************
// 函数功能： 计步主界面显示
// 输入参数： LanguageType 语言类型
// 返回参数： 无
void AppStepMainInterfaceDisp(systerm_languge_t LanguageType)
{
    letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	uint8_t sportTitleEn[] = "Activity";

	switch(LanguageType)
	{
		case SYS_ENGLISH_TYPE:
			/* 显示走路图标 */
		   PicDispInfo.AxisX = 0;
			PicDispInfo.AxisY = 0;
			PicDispInfo.AreaLength = 32;
			PicDispInfo.AreaHeight = 32;
			PicDispInfo.PictureLength = gImage_ic_walk_24[0];
			PicDispInfo.PictureHeight = gImage_ic_walk_24[1];
			PicDispInfo.Alic = ALIC_MIDDLE;	

			PicDispInfo.DotBuf = &gImage_ic_walk_24[2];
			AppScreenPictureDisp(&PicDispInfo);	
			
			/* 显示文字 */
			LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
			LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
			LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
			LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
			LetterSource.letterInfo.frontHeight = 16;			// 字符高度
		//	LetterSource.letterInfo.interval = 0;
			LetterSource.letterInfo.alic = ALIC_MIDDLE;
		//	LetterSource.letterInfo.language = 0;
			LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
			LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
			LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		

			LetterSource.letterStrAddr = sportTitleEn;
			LetterSource.letterStrLen = strlen("Activity");			
			AppScreenTextLayoutGBKDisp(&LetterSource);				
			
			break;
		case SYS_CHINESE_TYPE:
		    PicDispInfo.AxisX = 0;
			PicDispInfo.AxisY = 0;
			PicDispInfo.AreaLength = 96;
			PicDispInfo.AreaHeight = 32;
			PicDispInfo.PictureLength = gImage_ActivityCn[0];
			PicDispInfo.PictureHeight = gImage_ActivityCn[1];
			PicDispInfo.Alic = ALIC_MIDDLE;	
			PicDispInfo.DotBuf = &gImage_ActivityCn[2];
			
			AppScreenPictureDisp(&PicDispInfo);	
			break;
		default:
            /* 显示走路图标 */
           PicDispInfo.AxisX = 0;
            PicDispInfo.AxisY = 0;
            PicDispInfo.AreaLength = 32;
            PicDispInfo.AreaHeight = 32;
            PicDispInfo.PictureLength = gImage_ic_walk_24[0];
            PicDispInfo.PictureHeight = gImage_ic_walk_24[1];
            PicDispInfo.Alic = ALIC_MIDDLE; 

            PicDispInfo.DotBuf = &gImage_ic_walk_24[2];
            AppScreenPictureDisp(&PicDispInfo); 
            
            /* 显示文字 */
            LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
            LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
            LetterSource.letterInfo.letterAxisInfo.height = 32; // 显示区域高度
            LetterSource.letterInfo.letterAxisInfo.length = 64; // 显示区域长度
            LetterSource.letterInfo.frontHeight = 16;           // 字符高度
        //  LetterSource.letterInfo.interval = 0;
            LetterSource.letterInfo.alic = ALIC_MIDDLE;
        //  LetterSource.letterInfo.language = 0;
            LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
            LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;  // 英文字符字体
            LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;       // 中文字符字体       

            LetterSource.letterStrAddr = sportTitleEn;
            LetterSource.letterStrLen = strlen("Activity");         
            AppScreenTextLayoutGBKDisp(&LetterSource);          
			break;
	}
	
	#if 0
    letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	uint8_t sportTitleEn[] = "Activity";
	uint8_t sportTitleCn[] = {		// 活动量
		6,
		0xBB, 0xEE, 0xB6, 0xAF, 0xC1, 0xBF};
	
	/* 显示走路图标 */
   PicDispInfo.AxisX = 0;
    PicDispInfo.AxisY = 0;
	PicDispInfo.AreaLength = 32;
	PicDispInfo.AreaHeight = 32;
    PicDispInfo.PictureLength = gImage_ic_walk_24[0];
    PicDispInfo.PictureHeight = gImage_ic_walk_24[1];
	PicDispInfo.Alic = ALIC_MIDDLE;	

	PicDispInfo.DotBuf = &gImage_ic_walk_24[2];
	AppScreenPictureDisp(&PicDispInfo);	
	
	/* 显示文字 */
	LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
	LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 16;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		
	switch(LanguageType)
	{
		case SYS_ENGLISH_TYPE:
			LetterSource.letterStrAddr = sportTitleEn;
			LetterSource.letterStrLen = strlen("Activity");			
			AppScreenTextLayoutGBKDisp(&LetterSource);			
			break;
		case SYS_CHINESE_TYPE:
			LetterSource.letterStrAddr = &sportTitleCn[1];
			LetterSource.letterStrLen = sportTitleCn[0];		
			AppScreenTextLayoutGBKDisp(&LetterSource);	
			break;
		default:
            LetterSource.letterStrAddr = sportTitleEn;
            LetterSource.letterStrLen = strlen("Activity");         
            AppScreenTextLayoutGBKDisp(&LetterSource);
			break;
	}
	#endif
}

//**********************************************************************
// 函数功能： 计步完成度显示
// 输入参数： StepNum 步数
//			  Percent 完成度百分比
// 返回参数： 无
void AppStepCompleteDisp(uint32_t StepNum, uint32_t Percent)
{
	letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	uint8       tempBuf[10], tempExchangeBuf[10], tempLen;
	uint32_t TmpNum;
	
	/* 显示完成度图标 */
    PicDispInfo.AxisX = 0;
    PicDispInfo.AxisY = 0;
	PicDispInfo.AreaLength = 32;
	PicDispInfo.AreaHeight = 32;
    PicDispInfo.PictureLength = gImage_ic_step_24[0];
    PicDispInfo.PictureHeight = gImage_ic_step_24[1];
	PicDispInfo.Alic = ALIC_MIDDLE;	

	PicDispInfo.DotBuf = &gImage_ic_step_24[2];
	AppScreenPictureDisp(&PicDispInfo);	
	
	/* 显示计步数量 */
	// 计算计步值共有几位
	if(StepNum > 999999)
		StepNum = 999999;
	tempLen = 0;
	TmpNum = StepNum;
	do
	{
		tempBuf[tempLen] = TmpNum % 10;
		TmpNum /= 10;
		tempLen++;
	}while(TmpNum > 0);

	// 交换数组值
	for(uint32_t i = 0; i < tempLen; i++)
	{
		tempExchangeBuf[i] = tempBuf[tempLen-1-i];
	}
	
	// 显示计步值
    LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
    LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
    LetterSource.letterInfo.letterAxisInfo.height = 20;
    LetterSource.letterInfo.letterAxisInfo.length = 64;   
    LetterSource.letterInfo.letterAxisInfo.dispmode = EXRULE;   
	
	Mid_Screen_FontTgl24AlignDisp(&LetterSource.letterInfo, tempExchangeBuf, tempLen);  	
	
	/* 显示完成百分比 */ 
	// 计算百分比值共有几位
	if(Percent > 9999)
		Percent = 9999;	
	tempLen = 0;
	TmpNum = Percent;
	do
	{
		tempBuf[tempLen] = TmpNum % 10;
		TmpNum /= 10;
		tempLen++;
	}while(TmpNum > 0);
	
	// 交换数组值，并转换为ASCII码
	for(uint32_t i = 0; i < tempLen; i++)
	{
		tempExchangeBuf[i] = tempBuf[tempLen-1-i] + '0';	
	}
	
	tempExchangeBuf[tempLen] = '%';	// 百分比符号％
	tempLen++;
	
	// 显示百分比值.
	LetterSource.letterStrAddr = tempExchangeBuf;
	LetterSource.letterStrLen = tempLen;
	LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 20;
	LetterSource.letterInfo.letterAxisInfo.height = 12;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 12;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_12_B_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体
	
	AppScreenTextLayoutGBKDisp(&LetterSource);
}

//**********************************************************************
// 函数功能： 计步卡路里显示
// 输入参数： Kcal 卡路里
// 返回参数： 无
void AppStepKcalDisp(uint32_t Kcal)
{
	letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	uint8       tempBuf[10], tempExchangeBuf[10], tempLen;
	uint32_t TmpNum;
	
	/* 显示图标 */
    PicDispInfo.AxisX = 0;
    PicDispInfo.AxisY = 0;
	PicDispInfo.AreaLength = 32;
	PicDispInfo.AreaHeight = 32;
    PicDispInfo.PictureLength = gImage_ic_calorie_24[0];
    PicDispInfo.PictureHeight = gImage_ic_calorie_24[1];
	PicDispInfo.Alic = ALIC_MIDDLE;	

	PicDispInfo.DotBuf = &gImage_ic_calorie_24[2];
	AppScreenPictureDisp(&PicDispInfo);	
	
	/* 显示卡路里值 */
	// 计算计步值共有几位
	if(Kcal > 999999)
		Kcal = 999999;	
	tempLen = 0;
	TmpNum = Kcal;
	do
	{
		tempBuf[tempLen] = TmpNum % 10;
		TmpNum /= 10;
		tempLen++;
	}while(TmpNum > 0);

	// 交换数组值
	for(uint32_t i = 0; i < tempLen; i++)
	{
		tempExchangeBuf[i] = tempBuf[tempLen-1-i];
	}
	
    LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
    LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
    LetterSource.letterInfo.letterAxisInfo.height = 20;
    LetterSource.letterInfo.letterAxisInfo.length = 64;   
    LetterSource.letterInfo.letterAxisInfo.dispmode = EXRULE;   
	
	Mid_Screen_FontTgl24AlignDisp(&LetterSource.letterInfo, tempExchangeBuf, tempLen);  	
	
	/* 显示字符串:kcal */ 
	uint8_t KcalString[] = "kcal";
	
	LetterSource.letterStrAddr = KcalString;
	LetterSource.letterStrLen = strlen("kcal");
	LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 20;
	LetterSource.letterInfo.letterAxisInfo.height = 12;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 12;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_12_B_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体
	
	AppScreenTextLayoutGBKDisp(&LetterSource);
}

//**********************************************************************
// 函数功能： 计步活动时长显示
// 输入参数： Duration 时间，单位分钟
// 返回参数： 无
void AppStepDurationDisp(uint32_t Duration)
{
	letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	uint8       tempBuf[10], tempExchangeBuf[10], tempLen;
	uint32_t TmpNum;
	
	/* 显示图标 */
    PicDispInfo.AxisX = 0;
    PicDispInfo.AxisY = 0;
	PicDispInfo.AreaLength = 32;
	PicDispInfo.AreaHeight = 32;
    PicDispInfo.PictureLength = gImage_ic_timer_outline_24[0];
    PicDispInfo.PictureHeight = gImage_ic_timer_outline_24[1];
	PicDispInfo.Alic = ALIC_MIDDLE;	

	PicDispInfo.DotBuf = &gImage_ic_timer_outline_24[2];
	AppScreenPictureDisp(&PicDispInfo);	
	
	/* 显示卡路里值 */
	// 计算计步值共有几位
	if(Duration > 999999)
		Duration = 999999;		
	tempLen = 0;
	TmpNum = Duration;
	do
	{
		tempBuf[tempLen] = TmpNum % 10;
		TmpNum /= 10;
		tempLen++;
	}while(TmpNum > 0);

	// 交换数组值
	for(uint32_t i = 0; i < tempLen; i++)
	{
		tempExchangeBuf[i] = tempBuf[tempLen-1-i];
	}
	
    LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
    LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
    LetterSource.letterInfo.letterAxisInfo.height = 20;
    LetterSource.letterInfo.letterAxisInfo.length = 64;   
    LetterSource.letterInfo.letterAxisInfo.dispmode = EXRULE;   
	
	Mid_Screen_FontTgl24AlignDisp(&LetterSource.letterInfo, tempExchangeBuf, tempLen);  	
	
	/* 显示字符串 */ 
	uint8_t KcalString[] = "min";
	
	LetterSource.letterStrAddr = KcalString;
	LetterSource.letterStrLen = strlen("min");
	LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 20;
	LetterSource.letterInfo.letterAxisInfo.height = 12;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 12;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_12_B_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体
	
	AppScreenTextLayoutGBKDisp(&LetterSource);
}

//**********************************************************************
// 函数功能： 计步运动距离显示
// 输入参数： Distance 距离，单位米
// 返回参数： 无
void AppStepDistanceDisp(uint32_t Distance)
{
	letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	uint8       tempBuf[10], tempExchangeBuf[10], tempLen;
	uint32_t TmpNum;
	
	/* 显示图标 */
    PicDispInfo.AxisX = 0;
    PicDispInfo.AxisY = 0;
	PicDispInfo.AreaLength = 32;
	PicDispInfo.AreaHeight = 32;
    PicDispInfo.PictureLength = gImage_ic_distance_24[0];
    PicDispInfo.PictureHeight = gImage_ic_distance_24[1];
	PicDispInfo.Alic = ALIC_MIDDLE;	

	PicDispInfo.DotBuf = &gImage_ic_distance_24[2];
	AppScreenPictureDisp(&PicDispInfo);	
	
	/* 显示数值 */
	/*
	e:计算结果  a：被除数  b：除数:
	1(四舍五入) ： e=(a+(b/2))/b   
	2(进一法)    ： e=(a+(b-1))/b	
		*/
	if(Distance > 999999)
		Distance = 999999;	
	tempLen = 0;
	TmpNum = (Distance + 5) / 10;	// 4舍5入保留2位小数方法（个位和百位为小数）
	do
	{
		tempBuf[tempLen] = TmpNum % 10;
		TmpNum /= 10;
		tempLen++;
	}while(TmpNum > 0);

	// 若小于位数小于3，用0补齐，因为最小显示0.00
	if(tempLen < 3)
	{
		for(;tempLen < 3;tempLen++)
		{
			tempBuf[tempLen] = 0;
		}
	}	
	
	// 交换数组值
	for(uint32_t i = 0; i < tempLen; i++)
	{
		tempExchangeBuf[i] = tempBuf[tempLen-1-i];
	}	
	
	// 插入小数点
	tempLen++;
	tempExchangeBuf[tempLen - 1] = tempExchangeBuf[tempLen - 2];	// 后移1位
	tempExchangeBuf[tempLen - 2] = tempExchangeBuf[tempLen - 3];
	tempExchangeBuf[tempLen - 3] = 11;		// 小数点

    LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
    LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
    LetterSource.letterInfo.letterAxisInfo.height = 20;
    LetterSource.letterInfo.letterAxisInfo.length = 64;   
    LetterSource.letterInfo.letterAxisInfo.dispmode = EXRULE;   
	
	Mid_Screen_FontTgl24AlignDisp(&LetterSource.letterInfo, tempExchangeBuf, tempLen);  	
	
	/* 显示字符串 */ 
	uint8_t KcalString[] = "km";
	
	LetterSource.letterStrAddr = KcalString;
	LetterSource.letterStrLen = strlen("km");
	LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 20;
	LetterSource.letterInfo.letterAxisInfo.height = 12;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 12;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_12_B_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体
	
	AppScreenTextLayoutGBKDisp(&LetterSource);
}

//**********************************************************************
// 函数功能： 心率主界面显示
// 输入参数： LanguageType 语言类型
// 返回参数： 无
void AppHeartMainInterfaceDisp(systerm_languge_t LanguageType)
{
    letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	uint8_t TitleEn1[] = "Heart";
	uint8_t TitleEn2[] = "Rate";

	switch(LanguageType)
	{
		case SYS_ENGLISH_TYPE:
			/* 显示走路图标 */
		   PicDispInfo.AxisX = 0;
			PicDispInfo.AxisY = 0;
			PicDispInfo.AreaLength = 32;
			PicDispInfo.AreaHeight = 32;
			PicDispInfo.PictureLength = gImage_ic_hr_24[0];
			PicDispInfo.PictureHeight = gImage_ic_hr_24[1];
			PicDispInfo.Alic = ALIC_MIDDLE;	

			PicDispInfo.DotBuf = &gImage_ic_hr_24[2];
			AppScreenPictureDisp(&PicDispInfo);	
			
			/* 显示文字 */
			LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
			LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
			LetterSource.letterInfo.letterAxisInfo.height = 16;	// 显示区域高度
			LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
			LetterSource.letterInfo.frontHeight = 16;			// 字符高度
		//	LetterSource.letterInfo.interval = 0;
			LetterSource.letterInfo.alic = ALIC_MIDDLE;
		//	LetterSource.letterInfo.language = 0;
			LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
			LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
			LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		
			LetterSource.letterStrAddr = TitleEn1;
			LetterSource.letterStrLen = strlen("Heart");			
			AppScreenTextLayoutGBKDisp(&LetterSource);		

			LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
			LetterSource.letterInfo.letterAxisInfo.axis_Y = 16;
			LetterSource.letterInfo.letterAxisInfo.height = 16;	// 显示区域高度
			LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
			LetterSource.letterStrAddr = TitleEn2;
			LetterSource.letterStrLen = strlen("Rate");			
			AppScreenTextLayoutGBKDisp(&LetterSource);			
			break;
		case SYS_CHINESE_TYPE:
		    PicDispInfo.AxisX = 0;
			PicDispInfo.AxisY = 0;
			PicDispInfo.AreaLength = 96;
			PicDispInfo.AreaHeight = 32;
			PicDispInfo.PictureLength = 96;
			PicDispInfo.PictureHeight = 32;
			PicDispInfo.Alic = ALIC_MIDDLE;	
			PicDispInfo.DotBuf = &gImage_HeartCn[2];
			
			AppScreenPictureDisp(&PicDispInfo);	
			break;
		default:
            /* 显示走路图标 */
           PicDispInfo.AxisX = 0;
            PicDispInfo.AxisY = 0;
            PicDispInfo.AreaLength = 32;
            PicDispInfo.AreaHeight = 32;
            PicDispInfo.PictureLength = gImage_ic_hr_24[0];
            PicDispInfo.PictureHeight = gImage_ic_hr_24[1];
            PicDispInfo.Alic = ALIC_MIDDLE; 

            PicDispInfo.DotBuf = &gImage_ic_hr_24[2];
            AppScreenPictureDisp(&PicDispInfo); 
            
            /* 显示文字 */
            LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
            LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
            LetterSource.letterInfo.letterAxisInfo.height = 16; // 显示区域高度
            LetterSource.letterInfo.letterAxisInfo.length = 64; // 显示区域长度
            LetterSource.letterInfo.frontHeight = 16;           // 字符高度
        //  LetterSource.letterInfo.interval = 0;
            LetterSource.letterInfo.alic = ALIC_MIDDLE;
        //  LetterSource.letterInfo.language = 0;
            LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
            LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;  // 英文字符字体
            LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;       // 中文字符字体       
            LetterSource.letterStrAddr = TitleEn1;
            LetterSource.letterStrLen = strlen("Heart");            
            AppScreenTextLayoutGBKDisp(&LetterSource);      

            LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
            LetterSource.letterInfo.letterAxisInfo.axis_Y = 16;
            LetterSource.letterInfo.letterAxisInfo.height = 16; // 显示区域高度
            LetterSource.letterInfo.letterAxisInfo.length = 64; // 显示区域长度
            LetterSource.letterStrAddr = TitleEn2;
            LetterSource.letterStrLen = strlen("Rate");         
            AppScreenTextLayoutGBKDisp(&LetterSource);        
			break;
	}	
	
	#if 0
	letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	uint8_t TitleEn[] = "HR";
	uint8_t TitleCn[] = {		
		4,
		0xD0, 0xC4, 0xC2, 0xCA};

	/* 显示图标 */
    PicDispInfo.AxisX = 0;
    PicDispInfo.AxisY = 0;
	PicDispInfo.AreaLength = 32;
	PicDispInfo.AreaHeight = 32;
    PicDispInfo.PictureLength = gImage_ic_hr_24[0];
    PicDispInfo.PictureHeight = gImage_ic_hr_24[1];
	PicDispInfo.Alic = ALIC_MIDDLE;	

	PicDispInfo.DotBuf = &gImage_ic_hr_24[2];
	AppScreenPictureDisp(&PicDispInfo);	
   
	/* 显示文字 */
	LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
	LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 16;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		
	switch(LanguageType)
	{
		case SYS_ENGLISH_TYPE:
			LetterSource.letterStrAddr = TitleEn;
			LetterSource.letterStrLen = strlen("HR");			
			AppScreenTextLayoutGBKDisp(&LetterSource);			
			break;
		case SYS_CHINESE_TYPE:
			LetterSource.letterStrAddr = &TitleCn[1];
			LetterSource.letterStrLen = TitleCn[0];		
			AppScreenTextLayoutGBKDisp(&LetterSource);	
			break;
		default:
            LetterSource.letterStrAddr = TitleEn;
            LetterSource.letterStrLen = strlen("HR");           
            AppScreenTextLayoutGBKDisp(&LetterSource);
			break;
	}
	#endif
}

//**********************************************************************
// 函数功能： 未检测到心率，请重戴显示
// 输入参数： LanguageType 语言类型
// 返回参数： 无
void AppHeartNoWearDisp(systerm_languge_t LanguageType)
{
    letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	uint8_t TitleEn[] = "Rewear";

	switch(LanguageType)
	{
		case SYS_ENGLISH_TYPE:
			/* 显示走路图标 */
		   PicDispInfo.AxisX = 0;
			PicDispInfo.AxisY = 0;
			PicDispInfo.AreaLength = 32;
			PicDispInfo.AreaHeight = 32;
			PicDispInfo.PictureLength = 24;
			PicDispInfo.PictureHeight = 24;
			PicDispInfo.Alic = ALIC_MIDDLE;	

			PicDispInfo.DotBuf = &gImage_ic_hr_outline_24[2];
			AppScreenPictureDisp(&PicDispInfo);	
			
			/* 显示文字 */
			LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
			LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
			LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
			LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
			LetterSource.letterInfo.frontHeight = 16;			// 字符高度
		//	LetterSource.letterInfo.interval = 0;
			LetterSource.letterInfo.alic = ALIC_MIDDLE;
		//	LetterSource.letterInfo.language = 0;
			LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
			LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
			LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		

			LetterSource.letterStrAddr = TitleEn;
			LetterSource.letterStrLen = strlen("Rewear");			
			AppScreenTextLayoutGBKDisp(&LetterSource);				
			
			break;
		case SYS_CHINESE_TYPE:
		    PicDispInfo.AxisX = 0;
			PicDispInfo.AxisY = 0;
			PicDispInfo.AreaLength = 96;
			PicDispInfo.AreaHeight = 32;
			PicDispInfo.PictureLength = 96;
			PicDispInfo.PictureHeight = 32;
			PicDispInfo.Alic = ALIC_MIDDLE;	
			PicDispInfo.DotBuf = &gImage_HeartRewearCn[2];
			
			AppScreenPictureDisp(&PicDispInfo);	
			break;
		default:
            /* 显示走路图标 */
           PicDispInfo.AxisX = 0;
            PicDispInfo.AxisY = 0;
            PicDispInfo.AreaLength = 32;
            PicDispInfo.AreaHeight = 32;
            PicDispInfo.PictureLength = 24;
            PicDispInfo.PictureHeight = 24;
            PicDispInfo.Alic = ALIC_MIDDLE; 

            PicDispInfo.DotBuf = &gImage_ic_hr_outline_24[2];
            AppScreenPictureDisp(&PicDispInfo); 
            
            /* 显示文字 */
            LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
            LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
            LetterSource.letterInfo.letterAxisInfo.height = 32; // 显示区域高度
            LetterSource.letterInfo.letterAxisInfo.length = 64; // 显示区域长度
            LetterSource.letterInfo.frontHeight = 16;           // 字符高度
        //  LetterSource.letterInfo.interval = 0;
            LetterSource.letterInfo.alic = ALIC_MIDDLE;
        //  LetterSource.letterInfo.language = 0;
            LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
            LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;  // 英文字符字体
            LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;       // 中文字符字体       

            LetterSource.letterStrAddr = TitleEn;
            LetterSource.letterStrLen = strlen("Re-Wear");          
            AppScreenTextLayoutGBKDisp(&LetterSource);          
			break;
	}
	
	#if 0
	letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	uint8_t TitleEn[] = "Re-Wear";
	uint8_t TitleCn[] = {		
		6,
		0xC7, 0xEB, 0xD6, 0xD8, 0xB4, 0xF7};
	
	/* 显示图标 */
    PicDispInfo.AxisX = 0;
    PicDispInfo.AxisY = 0;
	PicDispInfo.AreaLength = 32;
	PicDispInfo.AreaHeight = 32;
    PicDispInfo.PictureLength = gImage_ic_hr_outline_24[0];
    PicDispInfo.PictureHeight = gImage_ic_hr_outline_24[1];
	PicDispInfo.Alic = ALIC_MIDDLE;	

	PicDispInfo.DotBuf = &gImage_ic_hr_outline_24[2];
	AppScreenPictureDisp(&PicDispInfo);	

	/* 显示文字 */
	LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
	LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 16;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		
	switch(LanguageType)
	{
		case SYS_ENGLISH_TYPE:
			LetterSource.letterStrAddr = TitleEn;
			LetterSource.letterStrLen = strlen("Re-Wear");			
			AppScreenTextLayoutGBKDisp(&LetterSource);			
			break;
		case SYS_CHINESE_TYPE:
			LetterSource.letterStrAddr = &TitleCn[1];
			LetterSource.letterStrLen = TitleCn[0];		
			AppScreenTextLayoutGBKDisp(&LetterSource);	
			break;
		default:
            LetterSource.letterStrAddr = TitleEn;
            LetterSource.letterStrLen = strlen("Re-Wear");          
            AppScreenTextLayoutGBKDisp(&LetterSource);
			break;
	}	
	#endif
}

//**********************************************************************
// 函数功能： 心率显示函数
// 输入参数： status 图标显示开关
//			  RestHeart 静息心率值，为0xFF时显示“--”
//			  DynamicHeart 	动态心率值，为0xFF时显示“--”
// 返回参数： 无
void AppHeartDataDisp(binary_type_t status, uint8_t RestHeart, uint8_t DynamicHeart)
{
	letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	uint8       tempBuf[72], tempExchangeBuf[32], tempLen;
	uint32_t TmpNum;

	/* 显示心率图标 */
	PicDispInfo.AxisX = 0;
	PicDispInfo.AxisY = 0;
	PicDispInfo.AreaLength = 32;
	PicDispInfo.AreaHeight = 32;	
	PicDispInfo.Alic = ALIC_MIDDLE;	
	PicDispInfo.PictureLength = gImage_ic_hr_24[0];
	PicDispInfo.PictureHeight = gImage_ic_hr_24[1];	

    switch (status)
	{
		case BINARY_TYPE_ON:		
			PicDispInfo.DotBuf = &gImage_ic_hr_24[2];
			AppScreenPictureDisp(&PicDispInfo);				
			break;
		case BINARY_TYPE_OFF:
			memset(tempBuf, 0x00, 72);
			PicDispInfo.DotBuf = tempBuf;
			AppScreenPictureDisp(&PicDispInfo);			
			break;
		default:
            PicDispInfo.DotBuf = &gImage_ic_hr_24[2];
            AppScreenPictureDisp(&PicDispInfo);         
			break;
	}

	/* 显示静态心率 */
	if(0xFF != RestHeart)
	{
		tempLen = 0;
		TmpNum = RestHeart;
		do
		{
			tempBuf[tempLen] = TmpNum % 10;
			TmpNum /= 10;
			tempLen++;
		}while(TmpNum > 0);

		// 交换数组值，并转换为ASCII码
		for(uint32_t i = 0; i < tempLen; i++)
		{
			tempExchangeBuf[i+4] = tempBuf[tempLen-1-i] + '0';	
		}
		tempLen += 4;
	}
	else	// 无心率值，显示--
	{
		tempExchangeBuf[4] = '-';	
		tempExchangeBuf[5] = '-';	
		tempLen = 6;
	}
	tempExchangeBuf[0] = 'R';
	tempExchangeBuf[1] = 'H';
	tempExchangeBuf[2] = 'R';
	tempExchangeBuf[3] = ' ';	
	
	LetterSource.letterStrAddr = tempExchangeBuf;
	LetterSource.letterStrLen = tempLen;
	LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 20;
	LetterSource.letterInfo.letterAxisInfo.height = 12;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 12;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_12_B_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体
	
	AppScreenTextLayoutGBKDisp(&LetterSource);		
	
	/* 显示动态心率 */
	if(0xFF != DynamicHeart)
	{
		tempLen = 0;
		TmpNum = DynamicHeart;
		do
		{
			tempBuf[tempLen] = TmpNum % 10;
			TmpNum /= 10;
			tempLen++;
		}while(TmpNum > 0);

		// 交换数组值
		for(uint32_t i = 0; i < tempLen; i++)
		{
			tempExchangeBuf[i] = tempBuf[tempLen-1-i];
		}		
	}
	else
	{
		tempLen = 2;
		tempExchangeBuf[0] = 12;	// 连字符‘-’
		tempExchangeBuf[1] = 12;
	}

    LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
    LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
    LetterSource.letterInfo.letterAxisInfo.height = 20;
    LetterSource.letterInfo.letterAxisInfo.length = 64;   
    LetterSource.letterInfo.letterAxisInfo.dispmode = EXRULE;   
	
	Mid_Screen_FontTgl24AlignDisp(&LetterSource.letterInfo, tempExchangeBuf, tempLen);  	
}

//**********************************************************************
// 函数功能： 心率显示函数
// 输入参数： status 图标显示开关
//			  RestHeart 静息心率值
// 返回参数： 无
void AppHeartCheckFlick(binary_type_t status,uint8_t RestHeart)
{
    pic_info_s  imageInfo;
    letter_source_s LetterSource;
    uint8       tempBuf[72], tempExchangeBuf[32], tempLen;
    uint32_t TmpNum;

    imageInfo.axis_X = 0;
    imageInfo.axis_Y = 0;
    imageInfo.height = NULL;
    imageInfo.length = NULL;
    imageInfo.dispmode = INCRULE;

    switch(status)
    {
        case BINARY_TYPE_ON:
        Mid_Screen_UpdataToPicRam(&imageInfo, gImage_heartflickSet);
        break;

        case BINARY_TYPE_OFF:
        Mid_Screen_UpdataToPicRam(&imageInfo, gImage_heartflickClear);
        break;

        default:
        Mid_Screen_UpdataToPicRam(&imageInfo, gImage_heartflickSet);
        break;
    } 

    /* 显示静态心率 */
    if(0xFF != RestHeart)
    {
        tempLen = 0;
        TmpNum = RestHeart;
        do
        {
            tempBuf[tempLen] = TmpNum % 10;
            TmpNum /= 10;
            tempLen++;
        }while(TmpNum > 0);

        // 交换数组值，并转换为ASCII码
        for(uint32_t i = 0; i < tempLen; i++)
        {
            tempExchangeBuf[i+4] = tempBuf[tempLen-1-i] + '0';  
        }
        tempLen += 4;
    }
    else    // 无心率值，显示--
    {
        tempExchangeBuf[4] = '-';   
        tempExchangeBuf[5] = '-';   
        tempLen = 6;
    }
    tempExchangeBuf[0] = 'R';
    tempExchangeBuf[1] = 'H';
    tempExchangeBuf[2] = 'R';
    tempExchangeBuf[3] = ' ';   
    
    LetterSource.letterStrAddr = tempExchangeBuf;
    LetterSource.letterStrLen = tempLen;
    LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
    LetterSource.letterInfo.letterAxisInfo.axis_Y = 20;
    LetterSource.letterInfo.letterAxisInfo.height = 12; // 显示区域高度
    LetterSource.letterInfo.letterAxisInfo.length = 64; // 显示区域长度
    LetterSource.letterInfo.frontHeight = 12;           // 字符高度
//  LetterSource.letterInfo.interval = 0;
    LetterSource.letterInfo.alic = ALIC_MIDDLE;
//  LetterSource.letterInfo.language = 0;
    LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
    LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_12_B_A;    // 英文字符字体
    LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;       // 中文字符字体
    
    AppScreenTextLayoutGBKDisp(&LetterSource);  
}

//**********************************************************************
// 函数功能：   心率图标显示
// 输入参数： status 图标显示开关
// 返回参数： 无
void AppHeartPicDisp(binary_type_t status)
{
	pic_disp_info_t PicDispInfo;
	uint8       tempBuf[72];

	/* 显示心率图标 */
	PicDispInfo.AxisX = 0;
	PicDispInfo.AxisY = 0;
	PicDispInfo.AreaLength = 32;
	PicDispInfo.AreaHeight = 32;	
	PicDispInfo.Alic = ALIC_MIDDLE;	
	PicDispInfo.PictureLength = gImage_ic_hr_24[0];
	PicDispInfo.PictureHeight = gImage_ic_hr_24[1];	

    switch (status)
	{
		case BINARY_TYPE_ON:		
			PicDispInfo.DotBuf = &gImage_ic_hr_24[2];
			AppScreenPictureDisp(&PicDispInfo);				
			break;
		case BINARY_TYPE_OFF:
			memset(tempBuf, 0x00, 72);
			PicDispInfo.DotBuf = tempBuf;
			AppScreenPictureDisp(&PicDispInfo);			
			break;
		default:
            PicDispInfo.DotBuf = &gImage_ic_hr_24[2];
            AppScreenPictureDisp(&PicDispInfo); 
			break;
	}
}

//**********************************************************************
// 函数功能：   静息心率显示	
// 输入参数： RestHeart 心率值，值为0xFF时显示“--”
// 返回参数： 无
void AppRestHeartDisp(uint8_t RestHeart)
{
	letter_source_s LetterSource;
	uint8       tempBuf[72], tempExchangeBuf[32], tempLen;
	uint32_t TmpNum;

	/* 显示静态心率 */
	if(0xFF != RestHeart)
	{
		tempLen = 0;
		TmpNum = RestHeart;
		do
		{
			tempBuf[tempLen] = TmpNum % 10;
			TmpNum /= 10;
			tempLen++;
		}while(TmpNum > 0);

		// 交换数组值，并转换为ASCII码
		for(uint32_t i = 0; i < tempLen; i++)
		{
			tempExchangeBuf[i+4] = tempBuf[tempLen-1-i] + '0';	
		}
		tempLen += 4;
	}
	else	// 无心率值，显示--
	{
		tempExchangeBuf[4] = '-';	
		tempExchangeBuf[5] = '-';	
		tempLen = 6;
	}
	tempExchangeBuf[0] = 'R';
	tempExchangeBuf[1] = 'H';
	tempExchangeBuf[2] = 'R';
	tempExchangeBuf[3] = ' ';	
	
	LetterSource.letterStrAddr = tempExchangeBuf;
	LetterSource.letterStrLen = tempLen;
	LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 20;
	LetterSource.letterInfo.letterAxisInfo.height = 12;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 12;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_12_B_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体
	
	AppScreenTextLayoutGBKDisp(&LetterSource);			
}

//**********************************************************************
// 函数功能：   动态心率显示	
// 输入参数： DynamicHeart 心率值，值为0xFF时显示“--”
// 返回参数： 无
void AppDynamicHeartDisp(uint8_t DynamicHeart)
{
	letter_source_s LetterSource;
	uint8       tempBuf[72], tempExchangeBuf[32], tempLen;
	uint32_t TmpNum;	
	
	/* 显示动态心率 */
	if(0xFF != DynamicHeart)
	{
		tempLen = 0;
		TmpNum = DynamicHeart;
		do
		{
			tempBuf[tempLen] = TmpNum % 10;
			TmpNum /= 10;
			tempLen++;
		}while(TmpNum > 0);

		// 交换数组值
		for(uint32_t i = 0; i < tempLen; i++)
		{
			tempExchangeBuf[i] = tempBuf[tempLen-1-i];
		}		
	}
	else
	{
		tempLen = 2;
		tempExchangeBuf[0] = 12;	// 连字符‘-’
		tempExchangeBuf[1] = 12;
	}

    LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
    LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
    LetterSource.letterInfo.letterAxisInfo.height = 20;
    LetterSource.letterInfo.letterAxisInfo.length = 64;   
    LetterSource.letterInfo.letterAxisInfo.dispmode = EXRULE;   
	
	Mid_Screen_FontTgl24AlignDisp(&LetterSource.letterInfo, tempExchangeBuf, tempLen);  		
}

//**********************************************************************
// 函数功能：   睡眠主界面显示
// 输入参数： LanguageType 语言类型
// 返回参数： 无
void AppSleepMainInterfaceDisp(systerm_languge_t LanguageType)
{
    letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	uint8_t TitleEn[] = "Sleep";

	switch(LanguageType)
	{
		case SYS_ENGLISH_TYPE:
			/* 显示走路图标 */
		   PicDispInfo.AxisX = 0;
			PicDispInfo.AxisY = 0;
			PicDispInfo.AreaLength = 32;
			PicDispInfo.AreaHeight = 32;
			PicDispInfo.PictureLength = 24;
			PicDispInfo.PictureHeight = 24;
			PicDispInfo.Alic = ALIC_MIDDLE;	

			PicDispInfo.DotBuf = &gImage_ic_sleep_24[2];
			AppScreenPictureDisp(&PicDispInfo);	
			
			/* 显示文字 */
			LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
			LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
			LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
			LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
			LetterSource.letterInfo.frontHeight = 16;			// 字符高度
		//	LetterSource.letterInfo.interval = 0;
			LetterSource.letterInfo.alic = ALIC_MIDDLE;
		//	LetterSource.letterInfo.language = 0;
			LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
			LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
			LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		

			LetterSource.letterStrAddr = TitleEn;
			LetterSource.letterStrLen = strlen("Sleep");			
			AppScreenTextLayoutGBKDisp(&LetterSource);				
			
			break;
		case SYS_CHINESE_TYPE:
		    PicDispInfo.AxisX = 0;
			PicDispInfo.AxisY = 0;
			PicDispInfo.AreaLength = 96;
			PicDispInfo.AreaHeight = 32;
			PicDispInfo.PictureLength = 96;
			PicDispInfo.PictureHeight = 32;
			PicDispInfo.Alic = ALIC_MIDDLE;	
			PicDispInfo.DotBuf = &gImage_SleepCn[2];
			
			AppScreenPictureDisp(&PicDispInfo);	
			break;
		default:
            /* 显示走路图标 */
            PicDispInfo.AxisX = 0;
            PicDispInfo.AxisY = 0;
            PicDispInfo.AreaLength = 32;
            PicDispInfo.AreaHeight = 32;
            PicDispInfo.PictureLength = 24;
            PicDispInfo.PictureHeight = 24;
            PicDispInfo.Alic = ALIC_MIDDLE; 

            PicDispInfo.DotBuf = &gImage_ic_sleep_24[2];
            AppScreenPictureDisp(&PicDispInfo); 
            
            /* 显示文字 */
            LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
            LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
            LetterSource.letterInfo.letterAxisInfo.height = 32; // 显示区域高度
            LetterSource.letterInfo.letterAxisInfo.length = 64; // 显示区域长度
            LetterSource.letterInfo.frontHeight = 16;           // 字符高度
        //  LetterSource.letterInfo.interval = 0;
            LetterSource.letterInfo.alic = ALIC_MIDDLE;
        //  LetterSource.letterInfo.language = 0;
            LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
            LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;  // 英文字符字体
            LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;       // 中文字符字体       

            LetterSource.letterStrAddr = TitleEn;
            LetterSource.letterStrLen = strlen("Sleep");            
            AppScreenTextLayoutGBKDisp(&LetterSource);  
			break;
	}	
	
	
	#if 0
    letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	uint8_t TitleEn[] = "Sleep";
	uint8_t TitleCn[] = {		
		4,
		0xCB, 0xAF, 0xC3, 0xDF};
	
	/* 显示图标 */
   PicDispInfo.AxisX = 0;
    PicDispInfo.AxisY = 0;
	PicDispInfo.AreaLength = 32;
	PicDispInfo.AreaHeight = 32;
    PicDispInfo.PictureLength = gImage_ic_sleep_24[0];
    PicDispInfo.PictureHeight = gImage_ic_sleep_24[1];
	PicDispInfo.Alic = ALIC_MIDDLE;	

	PicDispInfo.DotBuf = &gImage_ic_sleep_24[2];
	AppScreenPictureDisp(&PicDispInfo);	
    
		
	/* 显示文字 */
	LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
	LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 16;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		
	switch(LanguageType)
	{
		case SYS_ENGLISH_TYPE:
			LetterSource.letterStrAddr = TitleEn;
			LetterSource.letterStrLen = strlen("Sleep");			
			AppScreenTextLayoutGBKDisp(&LetterSource);			
			break;
		case SYS_CHINESE_TYPE:
			LetterSource.letterStrAddr = &TitleCn[1];
			LetterSource.letterStrLen = TitleCn[0];		
			AppScreenTextLayoutGBKDisp(&LetterSource);	
			break;
		default:
            LetterSource.letterStrAddr = TitleEn;
            LetterSource.letterStrLen = strlen("Sleep");            
            AppScreenTextLayoutGBKDisp(&LetterSource);
			break;
	}
	#endif
}

//**********************************************************************
// 函数功能：   无睡眠数据显示
// 输入参数： LanguageType 语言类型
// 返回参数： 无
void AppSleepNodataDisp(systerm_languge_t LanguageType)
{
    letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	uint8_t TitleEn1[] = "No Sleep";
	uint8_t TitleEn2[] = "Data";

	switch(LanguageType)
	{
		case SYS_ENGLISH_TYPE:
			/* 显示 "No Sleep"*/
			LetterSource.letterInfo.letterAxisInfo.axis_X = 0;
			LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
			LetterSource.letterInfo.letterAxisInfo.height = 16;	// 显示区域高度
			LetterSource.letterInfo.letterAxisInfo.length = 96;	// 显示区域长度
			LetterSource.letterInfo.frontHeight = 16;			// 字符高度
		//	LetterSource.letterInfo.interval = 0;
			LetterSource.letterInfo.alic = ALIC_MIDDLE;
		//	LetterSource.letterInfo.language = 0;
			LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
			LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
			LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		
			LetterSource.letterStrAddr = TitleEn1;
			LetterSource.letterStrLen = strlen((const char *)TitleEn1);//("No Sleep Data");			
			AppScreenTextLayoutGBKDisp(&LetterSource);	

			/* 显示 "Data"*/
			LetterSource.letterInfo.letterAxisInfo.axis_X = 0;
			LetterSource.letterInfo.letterAxisInfo.axis_Y = 16;
			LetterSource.letterInfo.letterAxisInfo.height = 16;	// 显示区域高度
			LetterSource.letterInfo.letterAxisInfo.length = 96;	// 显示区域长度	
			LetterSource.letterStrAddr = TitleEn2;
			LetterSource.letterStrLen = strlen((const char *)TitleEn2);//("No Sleep Data");			
			AppScreenTextLayoutGBKDisp(&LetterSource);				
			break;
		case SYS_CHINESE_TYPE:
		    PicDispInfo.AxisX = 0;
			PicDispInfo.AxisY = 0;
			PicDispInfo.AreaLength = 96;
			PicDispInfo.AreaHeight = 32;
			PicDispInfo.PictureLength = 96;
			PicDispInfo.PictureHeight = 32;
			PicDispInfo.Alic = ALIC_MIDDLE;	
			PicDispInfo.DotBuf = &gImage_NoSleepDataCn[2];
			
			AppScreenPictureDisp(&PicDispInfo);	
			break;
		default:
        /* 显示 "No Sleep"*/
            LetterSource.letterInfo.letterAxisInfo.axis_X = 0;
            LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
            LetterSource.letterInfo.letterAxisInfo.height = 16; // 显示区域高度
            LetterSource.letterInfo.letterAxisInfo.length = 96; // 显示区域长度
            LetterSource.letterInfo.frontHeight = 16;           // 字符高度
        //  LetterSource.letterInfo.interval = 0;
            LetterSource.letterInfo.alic = ALIC_MIDDLE;
        //  LetterSource.letterInfo.language = 0;
            LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
            LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;  // 英文字符字体
            LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;       // 中文字符字体       
            LetterSource.letterStrAddr = TitleEn1;
            LetterSource.letterStrLen = strlen((const char *)TitleEn1);//("No Sleep Data");         
            AppScreenTextLayoutGBKDisp(&LetterSource);  

            /* 显示 "Data"*/
            LetterSource.letterInfo.letterAxisInfo.axis_X = 0;
            LetterSource.letterInfo.letterAxisInfo.axis_Y = 16;
            LetterSource.letterInfo.letterAxisInfo.height = 16; // 显示区域高度
            LetterSource.letterInfo.letterAxisInfo.length = 96; // 显示区域长度   
            LetterSource.letterStrAddr = TitleEn2;
            LetterSource.letterStrLen = strlen((const char *)TitleEn2);//("No Sleep Data");         
            AppScreenTextLayoutGBKDisp(&LetterSource);
			break;
	}		
	
	#if 0
	letter_source_s LetterSource;
	uint8_t TitleEn[] = " No Sleep Data";
	uint8_t TitleCn[] = {		
		10,
		0xCE, 0xDE, 0xCB, 0xAF, 0xC3, 0xDF, 0XCA, 0XFD, 0XBE, 0XDD};
	
	/* 显示文字 */
	LetterSource.letterInfo.letterAxisInfo.axis_X = 0;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
	LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 96;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 16;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		
	switch(LanguageType)
	{
		case SYS_ENGLISH_TYPE:
			LetterSource.letterStrAddr = TitleEn;
			LetterSource.letterStrLen = strlen((const char *)TitleEn);//("No Sleep Data");			
			AppScreenTextLayoutGBKDisp(&LetterSource);			
			break;
		case SYS_CHINESE_TYPE:
			LetterSource.letterStrAddr = &TitleCn[1];
			LetterSource.letterStrLen = TitleCn[0];		
			AppScreenTextLayoutGBKDisp(&LetterSource);	
			break;
		default:
            LetterSource.letterStrAddr = TitleEn;
            LetterSource.letterStrLen = strlen((const char *)TitleEn);//("No Sleep Data");          
            AppScreenTextLayoutGBKDisp(&LetterSource);
			break;
	}
	#endif
}

//**********************************************************************
// 函数功能：   睡眠时长显示
// 输入参数： Minute 分钟
//			  SleepGrade 睡眠质量
// 返回参数： 无
void AppSleepDurationDisp(uint32_t Minute, sleep_grade_type_t SleepGrade)
{
	letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	uint8_t TmpBuf[32], TmpLen;
    
	/* 显示睡眠图标 */
    PicDispInfo.AxisX = 0;
    PicDispInfo.AxisY = 0;
	PicDispInfo.AreaLength = 32;
	PicDispInfo.AreaHeight = 32;
    PicDispInfo.PictureLength = gImage_ic_sleep_24[0];
    PicDispInfo.PictureHeight = gImage_ic_sleep_24[1];
	PicDispInfo.Alic = ALIC_MIDDLE;	

	PicDispInfo.DotBuf = &gImage_ic_sleep_24[2];
	AppScreenPictureDisp(&PicDispInfo);		
	
	/* 显示睡眠时间 */	
    LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
    LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
    LetterSource.letterInfo.letterAxisInfo.height = 20;
    LetterSource.letterInfo.letterAxisInfo.length = 64;   
    LetterSource.letterInfo.letterAxisInfo.dispmode = EXRULE;   

	// 越界处理，睡眠最大显示99小时
	if(Minute > 5940)	
		Minute = 5940;
			 
	uint8_t TmpSleepHour = Minute / 60;
	uint8_t TmpSleepMin = Minute % 60;
	TmpLen = 0;
	if(TmpSleepHour >= 10)
	{
		TmpBuf[TmpLen++] = TmpSleepHour / 10;
		TmpBuf[TmpLen++] = TmpSleepHour % 10;
	}
	else
	{
		TmpBuf[TmpLen++] = TmpSleepHour;
	}
	
	TmpBuf[TmpLen++] = 13;	// 符号“h”
	
	if(TmpSleepMin >= 10)
	{
		TmpBuf[TmpLen++] = TmpSleepMin / 10;
		TmpBuf[TmpLen++] = TmpSleepMin % 10;
	}
	else
	{
		TmpBuf[TmpLen++] = TmpSleepMin;
	}
	
	TmpBuf[TmpLen++] = 14;	// 符号“m”
	
	Mid_Screen_FontTgl24AlignDisp(&LetterSource.letterInfo, TmpBuf, TmpLen);  	
	
	/* 显示睡眠状态字符 */
	uint8_t TmpString_1[] = "Excellent";
	uint8_t TmpString_2[] = "Good";
	uint8_t TmpString_3[] = "Only Fair";
	uint8_t TmpString_4[] = "Poor";
	
	switch(SleepGrade)
	{
		case SLEEP_GRADE_EXCELLENT:
			LetterSource.letterStrLen = strlen("Excellent");
			LetterSource.letterStrAddr = TmpString_1;
			break;
		case SLEEP_GRADE_GOOD:
			LetterSource.letterStrLen = strlen("Good");
			LetterSource.letterStrAddr = TmpString_2;
			break;
		case SLEEP_GRADE_ONLY_FAIR:
			LetterSource.letterStrLen = strlen("Only Fair");
			LetterSource.letterStrAddr = TmpString_3;
			break;
		case SLEEP_GRADE_POOR:
			LetterSource.letterStrLen = strlen("Poor");
			LetterSource.letterStrAddr = TmpString_4;
			break;
		default:
            LetterSource.letterStrLen = strlen("Excellent");
            LetterSource.letterStrAddr = TmpString_1;
			break;
	}

	LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 20;
	LetterSource.letterInfo.letterAxisInfo.height = 12;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 12;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_12_B_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体
	
	AppScreenTextLayoutGBKDisp(&LetterSource);	
}

//**********************************************************************
// 函数功能：   睡眠时间点显示
// 输入参数： StartPoint 睡眠开始时间点
//			  StopPoint 睡眠结束时间点
//			  TimeFormat 时间制式，12小时制/24小时制
// 返回参数： 无
void AppSleepPointDisp(time_data_t* StartPoint, time_data_t* StopPoint, time_format_t TimeFormat)
{
	letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	uint8_t TmpBuf[32];
    
	/* 显示睡眠图标 */
    PicDispInfo.AxisX = 0;
    PicDispInfo.AxisY = 0;
	PicDispInfo.AreaLength = 32;
	PicDispInfo.AreaHeight = 32;
    PicDispInfo.PictureLength = gImage_ic_sleep_24[0];
    PicDispInfo.PictureHeight = gImage_ic_sleep_24[1];
	PicDispInfo.Alic = ALIC_MIDDLE;	

	PicDispInfo.DotBuf = &gImage_ic_sleep_24[2];
	AppScreenPictureDisp(&PicDispInfo);		
	
	// 数据越界处理
	if(StartPoint->Hour > 24) StartPoint->Hour = 24;
	if(StartPoint->Min > 60) StartPoint->Min = 60;	
	if(StopPoint->Hour > 24) StopPoint->Hour = 24;
	if(StopPoint->Min > 60) StopPoint->Min = 60;	
	
	/* 入睡时间 */
	switch (TimeFormat)
    {
    	case TIME_FORMAT_12:
			if(StartPoint->Hour >= 12)
			{
				TmpBuf[0] = 'P';
				AppScreenLocalAsciiDisp(32, 0, *TmpBuf, APP_ASCII_SIZE_12_B_A);
				TmpBuf[0] = (StartPoint->Hour - 12) / 10 + '0';
				TmpBuf[1] = (StartPoint->Hour - 12) % 10 + '0';				
			}
			else
			{
				TmpBuf[0] = 'A';
				AppScreenLocalAsciiDisp(32, 0, *TmpBuf, APP_ASCII_SIZE_12_B_A);
				TmpBuf[0] = StartPoint->Hour / 10 + '0';
				TmpBuf[1] = StartPoint->Hour % 10 + '0';					
			}
    		break;
    	case TIME_FORMAT_24:
			TmpBuf[0] = StartPoint->Hour / 10 + '0';
			TmpBuf[1] = StartPoint->Hour % 10 + '0';			
    		break;
    	default:
    		break;
    }

	TmpBuf[2] = ':';
	TmpBuf[3] = StartPoint->Min / 10 + '0';
	TmpBuf[4] = StartPoint->Min % 10 + '0';	
	
	LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
	LetterSource.letterInfo.letterAxisInfo.height = 16;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 16;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		

	LetterSource.letterStrAddr = TmpBuf;
	LetterSource.letterStrLen = 5;			
	AppScreenTextLayoutGBKDisp(&LetterSource);	
	
	/* 醒来时间 */
	switch (TimeFormat)
    {
    	case TIME_FORMAT_12:
			if(StopPoint->Hour >= 12)
			{
				TmpBuf[0] = 'P';
				AppScreenLocalAsciiDisp(32, 16, *TmpBuf, APP_ASCII_SIZE_12_B_A);
				TmpBuf[0] = (StopPoint->Hour - 12) / 10 + '0';
				TmpBuf[1] = (StopPoint->Hour - 12) % 10 + '0';					
			}
			else
			{
				TmpBuf[0] = 'A';
				AppScreenLocalAsciiDisp(32, 16, *TmpBuf, APP_ASCII_SIZE_12_B_A);
				TmpBuf[0] = StopPoint->Hour / 10 + '0';
				TmpBuf[1] = StopPoint->Hour % 10 + '0';			
			}
		
    		break;
    	case TIME_FORMAT_24:
			TmpBuf[0] = StopPoint->Hour / 10 + '0';
			TmpBuf[1] = StopPoint->Hour % 10 + '0';			
    		break;
    	default:
    		break;
    }

	TmpBuf[2] = ':';
	TmpBuf[3] = StopPoint->Min / 10 + '0';
	TmpBuf[4] = StopPoint->Min % 10 + '0';	
	
	LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 16;
	LetterSource.letterInfo.letterAxisInfo.height = 16;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 16;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		

	LetterSource.letterStrAddr = TmpBuf;
	LetterSource.letterStrLen = 5;			
	AppScreenTextLayoutGBKDisp(&LetterSource);		
}

//**********************************************************************
// 函数功能：   闹钟主界面显示
// 输入参数： LanguageType 语言类型
// 返回参数： 无
void AppClockMainInterfaceDisp(systerm_languge_t LanguageType)
{
    letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	uint8_t TitleEn[] = "Alarm";

	switch(LanguageType)
	{
		case SYS_ENGLISH_TYPE:
			/* 显示走路图标 */
		   PicDispInfo.AxisX = 0;
			PicDispInfo.AxisY = 0;
			PicDispInfo.AreaLength = 32;
			PicDispInfo.AreaHeight = 32;
			PicDispInfo.PictureLength = 24;
			PicDispInfo.PictureHeight = 24;
			PicDispInfo.Alic = ALIC_MIDDLE;	

			PicDispInfo.DotBuf = &gImage_ic_notifications_active_24[2];
			AppScreenPictureDisp(&PicDispInfo);	
			
			/* 显示文字 */
			LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
			LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
			LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
			LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
			LetterSource.letterInfo.frontHeight = 16;			// 字符高度
		//	LetterSource.letterInfo.interval = 0;
			LetterSource.letterInfo.alic = ALIC_MIDDLE;
		//	LetterSource.letterInfo.language = 0;
			LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
			LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
			LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		

			LetterSource.letterStrAddr = TitleEn;
			LetterSource.letterStrLen = strlen("Alarm");			
			AppScreenTextLayoutGBKDisp(&LetterSource);				
			
			break;
		case SYS_CHINESE_TYPE:
		    PicDispInfo.AxisX = 0;
			PicDispInfo.AxisY = 0;
			PicDispInfo.AreaLength = 96;
			PicDispInfo.AreaHeight = 32;
			PicDispInfo.PictureLength = 96;
			PicDispInfo.PictureHeight = 32;
			PicDispInfo.Alic = ALIC_MIDDLE;	
			PicDispInfo.DotBuf = &gImage_AlarmCn[2];
			
			AppScreenPictureDisp(&PicDispInfo);	
			break;
		default:
            /* 显示走路图标 */
           PicDispInfo.AxisX = 0;
            PicDispInfo.AxisY = 0;
            PicDispInfo.AreaLength = 32;
            PicDispInfo.AreaHeight = 32;
            PicDispInfo.PictureLength = 24;
            PicDispInfo.PictureHeight = 24;
            PicDispInfo.Alic = ALIC_MIDDLE; 

            PicDispInfo.DotBuf = &gImage_ic_notifications_active_24[2];
            AppScreenPictureDisp(&PicDispInfo); 
            
            /* 显示文字 */
            LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
            LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
            LetterSource.letterInfo.letterAxisInfo.height = 32; // 显示区域高度
            LetterSource.letterInfo.letterAxisInfo.length = 64; // 显示区域长度
            LetterSource.letterInfo.frontHeight = 16;           // 字符高度
        //  LetterSource.letterInfo.interval = 0;
            LetterSource.letterInfo.alic = ALIC_MIDDLE;
        //  LetterSource.letterInfo.language = 0;
            LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
            LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;  // 英文字符字体
            LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;       // 中文字符字体       

            LetterSource.letterStrAddr = TitleEn;
            LetterSource.letterStrLen = strlen("Alarm");            
            AppScreenTextLayoutGBKDisp(&LetterSource);
			break;
	}	
	
	#if 0
	letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	uint8_t TitleEn[] = "Alarm";
	uint8_t TitleCn[] = {		
		4,
		0xC4, 0xD6, 0xD6, 0xD3};
	
	/* 显示图标 */
   PicDispInfo.AxisX = 0;
    PicDispInfo.AxisY = 0;
	PicDispInfo.AreaLength = 32;
	PicDispInfo.AreaHeight = 32;
    PicDispInfo.PictureLength = gImage_ic_notifications_active_24[0];
    PicDispInfo.PictureHeight = gImage_ic_notifications_active_24[1];
	PicDispInfo.Alic = ALIC_MIDDLE;	

	PicDispInfo.DotBuf = &gImage_ic_notifications_active_24[2];
	AppScreenPictureDisp(&PicDispInfo);	
    
	/* 显示文字 */
	LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
	LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 16;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		
	switch(LanguageType)
	{
		case SYS_ENGLISH_TYPE:
			LetterSource.letterStrAddr = TitleEn;
			LetterSource.letterStrLen = strlen("Alarm");			
			AppScreenTextLayoutGBKDisp(&LetterSource);			
			break;
		case SYS_CHINESE_TYPE:
			LetterSource.letterStrAddr = &TitleCn[1];
			LetterSource.letterStrLen = TitleCn[0];		
			AppScreenTextLayoutGBKDisp(&LetterSource);	
			break;
		default:
            LetterSource.letterStrAddr = TitleEn;
            LetterSource.letterStrLen = strlen("Alarm");            
            AppScreenTextLayoutGBKDisp(&LetterSource);
			break;
	}
	#endif

}

//**********************************************************************
// 函数功能：   无闹钟数据显示
// 输入参数： LanguageType 语言类型
// 返回参数： 无
void AppClockNoDataDisp(systerm_languge_t LanguageType)
{
	
    letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	uint8_t TitleEn[] = "No Alarm";

	switch(LanguageType)
	{
		case SYS_ENGLISH_TYPE:
			LetterSource.letterInfo.letterAxisInfo.axis_X = 0;
			LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
			LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
			LetterSource.letterInfo.letterAxisInfo.length = 96;	// 显示区域长度
			LetterSource.letterInfo.frontHeight = 16;			// 字符高度
		//	LetterSource.letterInfo.interval = 0;
			LetterSource.letterInfo.alic = ALIC_MIDDLE;
		//	LetterSource.letterInfo.language = 0;
			LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
			LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
			LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		

            LetterSource.letterStrAddr = TitleEn;
            LetterSource.letterStrLen = strlen("No Alarm");         
            AppScreenTextLayoutGBKDisp(&LetterSource);
			break;
		case SYS_CHINESE_TYPE:
		    PicDispInfo.AxisX = 0;
			PicDispInfo.AxisY = 0;
			PicDispInfo.AreaLength = 96;
			PicDispInfo.AreaHeight = 32;
			PicDispInfo.PictureLength = 96;
			PicDispInfo.PictureHeight = 32;
			PicDispInfo.Alic = ALIC_MIDDLE;	
			PicDispInfo.DotBuf = &gImage_NoAlarmCn[2];
			
			AppScreenPictureDisp(&PicDispInfo);	
			break;
		default:
            LetterSource.letterInfo.letterAxisInfo.axis_X = 0;
            LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
            LetterSource.letterInfo.letterAxisInfo.height = 32; // 显示区域高度
            LetterSource.letterInfo.letterAxisInfo.length = 96; // 显示区域长度
            LetterSource.letterInfo.frontHeight = 16;           // 字符高度
        //  LetterSource.letterInfo.interval = 0;
            LetterSource.letterInfo.alic = ALIC_MIDDLE;
        //  LetterSource.letterInfo.language = 0;
            LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
            LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;  // 英文字符字体
            LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;       // 中文字符字体       

            LetterSource.letterStrAddr = TitleEn;
            LetterSource.letterStrLen = strlen("No Alarm");         
            AppScreenTextLayoutGBKDisp(&LetterSource);
			break;
	}	
	
	#if 0
	letter_source_s LetterSource;
	uint8_t TitleEn[] = "No Alarm";
	uint8_t TitleCn[] = {		
		6,
		0xCE, 0XDE, 0xC4, 0xD6, 0xD6, 0xD3};
	
	/* 显示文字 */
	LetterSource.letterInfo.letterAxisInfo.axis_X = 0;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
	LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 96;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 16;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		
	switch(LanguageType)
	{
		case SYS_ENGLISH_TYPE:
			LetterSource.letterStrAddr = TitleEn;
			LetterSource.letterStrLen = strlen("No Alarm");			
			AppScreenTextLayoutGBKDisp(&LetterSource);			
			break;
		case SYS_CHINESE_TYPE:
			LetterSource.letterStrAddr = &TitleCn[1];
			LetterSource.letterStrLen = TitleCn[0];		
			AppScreenTextLayoutGBKDisp(&LetterSource);	
			break;
		default:
            LetterSource.letterStrAddr = TitleEn;
            LetterSource.letterStrLen = strlen("No Alarm");         
            AppScreenTextLayoutGBKDisp(&LetterSource);
			break;
	}	
	#endif
}

//**********************************************************************
// 函数功能：   闹钟状态显示
// 输入参数： ClockName	闹钟名字
//			  ClockState 闹钟状态，开/关
//			  TimeDate 闹钟时间，时：分
//			  TimeFormat 时间制式，12小时制/24小时制
// 返回参数： 无
void AppClockStateDisp(clock_name_t ClockName, binary_type_t ClockState, time_data_t* TimeDate, time_format_t TimeFormat)
{
	letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	uint8_t TmpBuf[32], TmpLen;

	/* 显示闹钟图标 */
    PicDispInfo.AxisX = 0;
    PicDispInfo.AxisY = 0;
	PicDispInfo.AreaLength = 32;
	PicDispInfo.AreaHeight = 32;
    PicDispInfo.PictureLength = gImage_ic_alarm1_off_24[0];
    PicDispInfo.PictureHeight = gImage_ic_alarm1_off_24[1];
	PicDispInfo.Alic = ALIC_MIDDLE;	
	switch (ClockName)
    {
    	case CLOCK_NAME_1:
			if(BINARY_TYPE_ON == ClockState)
				PicDispInfo.DotBuf = &gImage_ic_alarm1_on_24[2];	
			else
				PicDispInfo.DotBuf = &gImage_ic_alarm1_off_24[2];	
    		break;
    	case CLOCK_NAME_2:
			if(BINARY_TYPE_ON == ClockState)
				PicDispInfo.DotBuf = &gImage_ic_alarm2_on_24[2];	
			else
				PicDispInfo.DotBuf = &gImage_ic_alarm2_off_24[2];			
    		break;
		case CLOCK_NAME_3:
			if(BINARY_TYPE_ON == ClockState)
				PicDispInfo.DotBuf = &gImage_ic_alarm3_on_24[2];	
			else
				PicDispInfo.DotBuf = &gImage_ic_alarm3_off_24[2];			
    		break;
    	default:
    		break;
    }
	AppScreenPictureDisp(&PicDispInfo);	

	/* 显示内容 */
    LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
    LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
    LetterSource.letterInfo.letterAxisInfo.height = 32;
    LetterSource.letterInfo.letterAxisInfo.length = 64;   
    LetterSource.letterInfo.letterAxisInfo.dispmode = EXRULE;   
               
	if(TimeDate->Hour > 24) TimeDate->Hour = 24;
	if(TimeDate->Min > 60) TimeDate->Min = 60;
	
    switch(TimeFormat)
    {
        case TIME_FORMAT_24:                 
            TmpLen        = 5;
            TmpBuf[0]     = TimeDate->Hour / 10;
            TmpBuf[1]     = TimeDate->Hour % 10;
            TmpBuf[2]     = 10;                        // 冒号
            TmpBuf[3]     = TimeDate->Min  / 10;
            TmpBuf[4]     = TimeDate->Min  % 10;
            Mid_Screen_FontTgl24AlignDisp(&LetterSource.letterInfo, TmpBuf, TmpLen);  
            break;
        case TIME_FORMAT_12:   
			if(TimeDate->Hour >=12)
			{
				TmpBuf[0] = 'P';
				AppScreenLocalAsciiDisp(34, 6, *TmpBuf, APP_ASCII_SIZE_12_B_A);
			}
			else
			{
				TmpBuf[0] = 'A';
				AppScreenLocalAsciiDisp(34, 6, *TmpBuf, APP_ASCII_SIZE_12_B_A);
			}

			if(TimeDate->Hour >=12)
				TimeDate->Hour -= 12;
			
            TmpLen        = 5;
            TmpBuf[0]     = TimeDate->Hour / 10;
            TmpBuf[1]     = TimeDate->Hour % 10;
            TmpBuf[2]     = 10;                        // 冒号
            TmpBuf[3]     = TimeDate->Min  / 10;
            TmpBuf[4]     = TimeDate->Min  % 10;
            Mid_Screen_FontTgl24AlignDisp(&LetterSource.letterInfo, TmpBuf, TmpLen); 			
            break;  
        default:
            break;
    }
}

//**********************************************************************
// 函数功能：   设置主界面显示
// 输入参数： Language	语言类型
// 返回参数： 无
void AppSetMainInterfaceDisp(systerm_languge_t LanguageType)
{
    letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	uint8_t TitleEn[] = "Settings";

	switch(LanguageType)
	{
		case SYS_ENGLISH_TYPE:
			/* 显示走路图标 */
		   PicDispInfo.AxisX = 0;
			PicDispInfo.AxisY = 0;
			PicDispInfo.AreaLength = 32;
			PicDispInfo.AreaHeight = 32;
			PicDispInfo.PictureLength = 24;
			PicDispInfo.PictureHeight = 24;
			PicDispInfo.Alic = ALIC_MIDDLE;	

			PicDispInfo.DotBuf = &gImage_ic_settings_24[2];
			AppScreenPictureDisp(&PicDispInfo);	
			
			/* 显示文字 */
			LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
			LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
			LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
			LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
			LetterSource.letterInfo.frontHeight = 16;			// 字符高度
		//	LetterSource.letterInfo.interval = 0;
			LetterSource.letterInfo.alic = ALIC_MIDDLE;
		//	LetterSource.letterInfo.language = 0;
			LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
			LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
			LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		

			LetterSource.letterStrAddr = TitleEn;
			LetterSource.letterStrLen = strlen("Settings");			
			AppScreenTextLayoutGBKDisp(&LetterSource);				
			
			break;
		case SYS_CHINESE_TYPE:
		    PicDispInfo.AxisX = 0;
			PicDispInfo.AxisY = 0;
			PicDispInfo.AreaLength = 96;
			PicDispInfo.AreaHeight = 32;
			PicDispInfo.PictureLength = 96;
			PicDispInfo.PictureHeight = 32;
			PicDispInfo.Alic = ALIC_MIDDLE;	
			PicDispInfo.DotBuf = &gImage_SettingsCn[2];
			
			AppScreenPictureDisp(&PicDispInfo);	
			break;
		default:
            /* 显示走路图标 */
            PicDispInfo.AxisX = 0;
            PicDispInfo.AxisY = 0;
            PicDispInfo.AreaLength = 32;
            PicDispInfo.AreaHeight = 32;
            PicDispInfo.PictureLength = 24;
            PicDispInfo.PictureHeight = 24;
            PicDispInfo.Alic = ALIC_MIDDLE; 

            PicDispInfo.DotBuf = &gImage_ic_settings_24[2];
            AppScreenPictureDisp(&PicDispInfo); 
            
            /* 显示文字 */
            LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
            LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
            LetterSource.letterInfo.letterAxisInfo.height = 32; // 显示区域高度
            LetterSource.letterInfo.letterAxisInfo.length = 64; // 显示区域长度
            LetterSource.letterInfo.frontHeight = 16;           // 字符高度
        //  LetterSource.letterInfo.interval = 0;
            LetterSource.letterInfo.alic = ALIC_MIDDLE;
        //  LetterSource.letterInfo.language = 0;
            LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
            LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;  // 英文字符字体
            LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;       // 中文字符字体       

            LetterSource.letterStrAddr = TitleEn;
            LetterSource.letterStrLen = strlen("Settings");         
            AppScreenTextLayoutGBKDisp(&LetterSource);
			break;
	}	
	
	#if 0
	letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	uint8_t TitleEn[] = "Settings";
	uint8_t TitleCn[] = {		
		4,
		0xC9, 0xE8, 0xD6, 0xC3};
	
	/* 显示图标 */
    PicDispInfo.AxisX = 0;
    PicDispInfo.AxisY = 0;
	PicDispInfo.AreaLength = 32;
	PicDispInfo.AreaHeight = 32;
    PicDispInfo.PictureLength = gImage_ic_settings_24[0];
    PicDispInfo.PictureHeight = gImage_ic_settings_24[1];
	PicDispInfo.Alic = ALIC_MIDDLE;	

	PicDispInfo.DotBuf = &gImage_ic_settings_24[2];
	AppScreenPictureDisp(&PicDispInfo);	
    
	/* 显示文字 */
	LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
	LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 16;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		
	switch(LanguageType)
	{
		case SYS_ENGLISH_TYPE:
			LetterSource.letterStrAddr = TitleEn;
			LetterSource.letterStrLen = strlen("Settings");			
			AppScreenTextLayoutGBKDisp(&LetterSource);			
			break;
		case SYS_CHINESE_TYPE:
			LetterSource.letterStrAddr = &TitleCn[1];
			LetterSource.letterStrLen = TitleCn[0];		
			AppScreenTextLayoutGBKDisp(&LetterSource);	
			break;
		default:
            LetterSource.letterStrAddr = TitleEn;
            LetterSource.letterStrLen = strlen("Settings");         
            AppScreenTextLayoutGBKDisp(&LetterSource);
			break;
	}
	#endif
}

//**********************************************************************
// 函数功能：  设置蓝牙开关显示
// 输入参数： Language	语言类型
// 返回参数： 无
void AppSetBleDisp(binary_type_t BleState, systerm_languge_t LanguageType)
{
    letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	uint8_t TmpStringENOn[] = "ON";
	uint8_t TmpStringENOff[] = "OFF";

	switch(LanguageType)
	{
		case SYS_ENGLISH_TYPE:
			/* 显示走路图标 */
		    PicDispInfo.AxisX = 0;
			PicDispInfo.AxisY = 0;
			PicDispInfo.AreaLength = 32;
			PicDispInfo.AreaHeight = 32;
			PicDispInfo.PictureLength = 24;
			PicDispInfo.PictureHeight = 24;
			PicDispInfo.Alic = ALIC_MIDDLE;	

			if(BINARY_TYPE_ON == BleState)
			{
				PicDispInfo.DotBuf = &gImage_ic_bt_24[2];
			}
			else
			{
				PicDispInfo.DotBuf = &gImage_ic_bt_off_24[2];
			}

			AppScreenPictureDisp(&PicDispInfo);	
			
			/* 显示文字 */
			LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
			LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
			LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
			LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
			LetterSource.letterInfo.frontHeight = 16;			// 字符高度
		//	LetterSource.letterInfo.interval = 0;
			LetterSource.letterInfo.alic = ALIC_MIDDLE;
		//	LetterSource.letterInfo.language = 0;
			LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
			LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
			LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		

			if(BINARY_TYPE_ON == BleState)
			{
				LetterSource.letterStrAddr = TmpStringENOn;
				LetterSource.letterStrLen = strlen("ON");	
			}
			else
			{
				LetterSource.letterStrAddr = TmpStringENOff;
				LetterSource.letterStrLen = strlen("OFF");	
			}
		
			AppScreenTextLayoutGBKDisp(&LetterSource);				
			
			break;
		case SYS_CHINESE_TYPE:
		    PicDispInfo.AxisX = 0;
			PicDispInfo.AxisY = 0;
			PicDispInfo.AreaLength = 96;
			PicDispInfo.AreaHeight = 32;
			PicDispInfo.PictureLength = 96;
			PicDispInfo.PictureHeight = 32;
			PicDispInfo.Alic = ALIC_MIDDLE;	
			if(BINARY_TYPE_ON == BleState)
			{
				PicDispInfo.DotBuf = &gImage_BleOnCn[2];
			}
			else
			{
				PicDispInfo.DotBuf = &gImage_BleOffCn[2];
			}
			
			AppScreenPictureDisp(&PicDispInfo);	
			break;
		default:
            /* 显示走路图标 */
            PicDispInfo.AxisX = 0;
            PicDispInfo.AxisY = 0;
            PicDispInfo.AreaLength = 32;
            PicDispInfo.AreaHeight = 32;
            PicDispInfo.PictureLength = 24;
            PicDispInfo.PictureHeight = 24;
            PicDispInfo.Alic = ALIC_MIDDLE; 

            if(BINARY_TYPE_ON == BleState)
            {
                PicDispInfo.DotBuf = &gImage_ic_bt_24[2];
            }
            else
            {
                PicDispInfo.DotBuf = &gImage_ic_bt_off_24[2];
            }

            AppScreenPictureDisp(&PicDispInfo); 
            
            /* 显示文字 */
            LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
            LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
            LetterSource.letterInfo.letterAxisInfo.height = 32; // 显示区域高度
            LetterSource.letterInfo.letterAxisInfo.length = 64; // 显示区域长度
            LetterSource.letterInfo.frontHeight = 16;           // 字符高度
        //  LetterSource.letterInfo.interval = 0;
            LetterSource.letterInfo.alic = ALIC_MIDDLE;
        //  LetterSource.letterInfo.language = 0;
            LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
            LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;  // 英文字符字体
            LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;       // 中文字符字体       

            if(BINARY_TYPE_ON == BleState)
            {
                LetterSource.letterStrAddr = TmpStringENOn;
                LetterSource.letterStrLen = strlen("ON");   
            }
            else
            {
                LetterSource.letterStrAddr = TmpStringENOff;
                LetterSource.letterStrLen = strlen("OFF");  
            }
        
            AppScreenTextLayoutGBKDisp(&LetterSource);
			break;
	}	
	
	
	#if 0
	letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	
	/* 显示图标 */
    PicDispInfo.AxisX = 0;
    PicDispInfo.AxisY = 0;
	PicDispInfo.AreaLength = 32;
	PicDispInfo.AreaHeight = 32;
    PicDispInfo.PictureLength = gImage_ic_bt_24[0];
    PicDispInfo.PictureHeight = gImage_ic_bt_24[1];
	PicDispInfo.Alic = ALIC_MIDDLE;
	switch (BleState)
    {
    	case BINARY_TYPE_ON:
			PicDispInfo.DotBuf = &gImage_ic_bt_24[2];
			AppScreenPictureDisp(&PicDispInfo);				
    		break;
    	case BINARY_TYPE_OFF:
			PicDispInfo.DotBuf = &gImage_ic_bt_off_24[2];
			AppScreenPictureDisp(&PicDispInfo);				
    		break;
    	default:
            PicDispInfo.DotBuf = &gImage_ic_bt_24[2];
            AppScreenPictureDisp(&PicDispInfo);
    		break;
    }
		
	/* 显示文字 */
	uint8_t TmpStringENOn[] = "ON";
	uint8_t TmpStringENOff[] = "OFF";
	uint8_t TmpStringCNOn[] = {2, 0xBF, 0xAA};
	uint8_t TmpStringCNOff[] = {2, 0xB9, 0xD8};
	LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
	LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 16;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		
	switch(Language)
	{
		case SYS_ENGLISH_TYPE:
			if(BINARY_TYPE_ON == BleState)
			{
				LetterSource.letterStrAddr = TmpStringENOn;
				LetterSource.letterStrLen = strlen("ON");			
				AppScreenTextLayoutGBKDisp(&LetterSource);					
			}
			else
			{
				LetterSource.letterStrAddr = TmpStringENOff;
				LetterSource.letterStrLen = strlen("OFF");			
				AppScreenTextLayoutGBKDisp(&LetterSource);					
			}
			break;
		case SYS_CHINESE_TYPE:
			if(BINARY_TYPE_ON == BleState)
			{
				LetterSource.letterStrAddr = &TmpStringCNOn[1];
				LetterSource.letterStrLen = TmpStringCNOn[0];			
				AppScreenTextLayoutGBKDisp(&LetterSource);					
			}
			else
			{
				LetterSource.letterStrAddr = &TmpStringCNOff[1];
				LetterSource.letterStrLen = TmpStringCNOff[0];		
				AppScreenTextLayoutGBKDisp(&LetterSource);					
			}			
			break;
		default:
			break;
	}
	#endif
}

//**********************************************************************
// 函数功能：  设置查找手机显示
// 输入参数： Language	语言类型
// 返回参数： 无
void AppSetFindPhotoDisp(systerm_languge_t LanguageType)
{
    letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	uint8_t TitleEn[] = "Find My Phone";

	switch(LanguageType)
	{
		case SYS_ENGLISH_TYPE:
			/* 显示走路图标 */
		   PicDispInfo.AxisX = 0;
			PicDispInfo.AxisY = 0;
			PicDispInfo.AreaLength = 32;
			PicDispInfo.AreaHeight = 32;
			PicDispInfo.PictureLength = 24;
			PicDispInfo.PictureHeight = 24;
			PicDispInfo.Alic = ALIC_MIDDLE;	

			PicDispInfo.DotBuf = &gImage_ic_phone_where_24[2];
			AppScreenPictureDisp(&PicDispInfo);	
			
			/* 显示文字 */
			LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
			LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
			LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
			LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
			LetterSource.letterInfo.frontHeight = 16;			// 字符高度
		//	LetterSource.letterInfo.interval = 0;
			LetterSource.letterInfo.alic = ALIC_MIDDLE;
		//	LetterSource.letterInfo.language = 0;
			LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
			LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
			LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		

			LetterSource.letterStrAddr = TitleEn;
			LetterSource.letterStrLen = strlen("Find My Phone");			
			AppScreenTextLayoutGBKDisp(&LetterSource);				
			
			break;
		case SYS_CHINESE_TYPE:
		    PicDispInfo.AxisX = 0;
			PicDispInfo.AxisY = 0;
			PicDispInfo.AreaLength = 96;
			PicDispInfo.AreaHeight = 32;
			PicDispInfo.PictureLength = 96;
			PicDispInfo.PictureHeight = 32;
			PicDispInfo.Alic = ALIC_MIDDLE;	
			PicDispInfo.DotBuf = &gImage_FindMyPhoneCn[2];
			
			AppScreenPictureDisp(&PicDispInfo);	
			break;
		default:
            /* 显示走路图标 */
            PicDispInfo.AxisX = 0;
            PicDispInfo.AxisY = 0;
            PicDispInfo.AreaLength = 32;
            PicDispInfo.AreaHeight = 32;
            PicDispInfo.PictureLength = 24;
            PicDispInfo.PictureHeight = 24;
            PicDispInfo.Alic = ALIC_MIDDLE; 

            PicDispInfo.DotBuf = &gImage_ic_phone_where_24[2];
            AppScreenPictureDisp(&PicDispInfo); 
            
            /* 显示文字 */
            LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
            LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
            LetterSource.letterInfo.letterAxisInfo.height = 32; // 显示区域高度
            LetterSource.letterInfo.letterAxisInfo.length = 64; // 显示区域长度
            LetterSource.letterInfo.frontHeight = 16;           // 字符高度
        //  LetterSource.letterInfo.interval = 0;
            LetterSource.letterInfo.alic = ALIC_MIDDLE;
        //  LetterSource.letterInfo.language = 0;
            LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
            LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;  // 英文字符字体
            LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;       // 中文字符字体       

            LetterSource.letterStrAddr = TitleEn;
            LetterSource.letterStrLen = strlen("Find My Phone");            
            AppScreenTextLayoutGBKDisp(&LetterSource);
			break;
	}
	
	#if 0
	letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	
	/* 显示图标 */
    PicDispInfo.AxisX = 0;
    PicDispInfo.AxisY = 0;
	PicDispInfo.AreaLength = 32;
	PicDispInfo.AreaHeight = 32;
    PicDispInfo.PictureLength = gImage_ic_phone_where_24[0];
    PicDispInfo.PictureHeight = gImage_ic_phone_where_24[1];
	PicDispInfo.Alic = ALIC_MIDDLE;	
	PicDispInfo.DotBuf = &gImage_ic_phone_where_24[2];
	AppScreenPictureDisp(&PicDispInfo);				

	/* 显示文字 */
	uint8_t TmpStringEN[] = "Find My Phone";
	uint8_t TmpStringCN[] = {
		8, 
		0xB2, 0xE9, 0xD5, 0xD2, 0xCA, 0xD6, 0xBB, 0xFA};
	LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
	LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 16;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		
	switch(Language)
	{
		case SYS_ENGLISH_TYPE:
			LetterSource.letterStrAddr = TmpStringEN;
			LetterSource.letterStrLen = strlen("Find My Phone");			
			AppScreenTextLayoutGBKDisp(&LetterSource);					
			break;
		case SYS_CHINESE_TYPE:
			LetterSource.letterStrAddr = &TmpStringCN[1];
			LetterSource.letterStrLen = TmpStringCN[0];		
			AppScreenTextLayoutGBKDisp(&LetterSource);								
			break;
		default:
            LetterSource.letterStrAddr = TmpStringEN;
            LetterSource.letterStrLen = strlen("Find My Phone");            
            AppScreenTextLayoutGBKDisp(&LetterSource);
			break;
	}	
	#endif
}

//**********************************************************************
// 函数功能：  版本号显示
// 输入参数： Version	版本号，详细定义见： PROJECT_VERSION
// 返回参数： 无
void AppSetVersionDisp(uint32_t Version, uint8 soc,uint8 batVol)
{
    letter_info_s  leterInfoTemp;
    letter_source_s LetterSource;
    uint8           tmpNum;
    uint8       tempBuf[10], tempExchangeBuf[10], tempLen;		

	/* 显示版本号 */
	uint8_t TmpBuf[16], TmpLen = 0;
	TmpBuf[TmpLen++] = 'V';
	
	if((Version >> 20) & 0x0F) TmpBuf[TmpLen++] = ((Version >> 20) & 0x0F) + '0';
	TmpBuf[TmpLen++] = ((Version >> 16) & 0x0F) + '0';
	TmpBuf[TmpLen++] = '.';
	
	if((Version >> 12) & 0x0F) TmpBuf[TmpLen++] = ((Version >> 12) & 0x0F) + '0';
	TmpBuf[TmpLen++] = ((Version >> 8) & 0x0F) + '0';	
	TmpBuf[TmpLen++] = '.';

	if((Version >> 4) & 0x0F) TmpBuf[TmpLen++] = ((Version >> 4) & 0x0F) + '0';
	TmpBuf[TmpLen++] = ((Version >> 0) & 0x0F) + '0';	

	LetterSource.letterInfo.letterAxisInfo.axis_X = 0;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 16;
	LetterSource.letterInfo.letterAxisInfo.height = 16;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 96;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 16;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体	
		
	LetterSource.letterStrAddr = TmpBuf;
	LetterSource.letterStrLen = TmpLen;			
	AppScreenTextLayoutGBKDisp(&LetterSource);					


	// 显示电量图标
    leterInfoTemp.letterAxisInfo.axis_X = 8;
    leterInfoTemp.letterAxisInfo.axis_Y = 0;
    leterInfoTemp.letterAxisInfo.height = NULL;
    leterInfoTemp.letterAxisInfo.length = NULL;
    leterInfoTemp.letterAxisInfo.dispmode = INCRULE;

    BatDisp(&leterInfoTemp, batVol);


    /* 显示完成百分比 */ 
    // 计算百分比值共有几位
    tempLen = 0;
    tmpNum = soc;
    do
    {
        tempBuf[tempLen] = tmpNum % 10;
        tmpNum /= 10;
        tempLen++;
    }while(tmpNum > 0);
    
    // 交换数组值，并转换为ASCII码
    for(uint8_t i = 0; i < tempLen; i++)
    {
        tempExchangeBuf[i] = tempBuf[tempLen-1-i] + '0';    
    }
    
    tempExchangeBuf[tempLen] = '%'; // 百分比符号％
    tempLen++;
    
    // 显示百分比值.
    LetterSource.letterStrAddr = tempExchangeBuf;
    LetterSource.letterStrLen = tempLen;
    LetterSource.letterInfo.letterAxisInfo.axis_X = 44;
    LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
    LetterSource.letterInfo.letterAxisInfo.height = 16; // 显示区域高度
    LetterSource.letterInfo.letterAxisInfo.length = 44; // 显示区域长度
    LetterSource.letterInfo.frontHeight = 16;           // 字符高度
//  LetterSource.letterInfo.interval = 0;
    LetterSource.letterInfo.alic = ALIC_MIDDLE;
//  LetterSource.letterInfo.language = 0;
    LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
    LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;    // 英文字符字体
    LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;       // 中文字符字体
    
    AppScreenTextLayoutGBKDisp(&LetterSource);
}

//**********************************************************************
// 函数功能：  来电提醒显示
// 输入参数： String	字符串地址
//			  stringLen 字符串长度
//			  strCodeType 字符串编码类型
// 返回参数： 无
void AppRemindCallDisp(uint8_t *String,uint8_t stringLen, letter_code_type_t strCodeType)
{
	letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	
	/* 显示图标 */
    PicDispInfo.AxisX = 0;
    PicDispInfo.AxisY = 0;
	PicDispInfo.AreaLength = 32;
	PicDispInfo.AreaHeight = 32;
    PicDispInfo.PictureLength = gImage_ic_call_24[0];
    PicDispInfo.PictureHeight = gImage_ic_call_24[1];
	PicDispInfo.Alic = ALIC_MIDDLE;	
	PicDispInfo.DotBuf = &gImage_ic_call_24[2];
	AppScreenPictureDisp(&PicDispInfo);				

	/* 显示文字 */
	LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
	LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 16;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = strCodeType;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		

	LetterSource.letterStrAddr = String;
	LetterSource.letterStrLen = stringLen;

	switch (strCodeType)
    {
    	case LETTER_TYPTE_UTF8: break;
    	case LETTER_TYPTE_UNICODE: 
			AppScreenTextLayoutUnicodeDisp(&LetterSource);
			break;
		case LETTER_TYPTE_GB:
			AppScreenTextLayoutGBKDisp(&LetterSource);
    		break;
    	default :
    		break;
    }
}

//**********************************************************************
// 函数功能：  手机短信提醒显示
// 输入参数： String	字符串地址
//			  stringLen 字符串长度
//			  strCodeType 字符串编码类型
// 返回参数： 无
void AppRemindPhoneMsgDisp(uint8_t *String,uint8_t stringLen, letter_code_type_t strCodeType)
{
	letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	
	/* 显示图标 */
    PicDispInfo.AxisX = 0;
    PicDispInfo.AxisY = 0;
	PicDispInfo.AreaLength = 32;
	PicDispInfo.AreaHeight = 32;
    PicDispInfo.PictureLength = gImage_ic_smg_24[0];
    PicDispInfo.PictureHeight = gImage_ic_smg_24[1];
	PicDispInfo.Alic = ALIC_MIDDLE;	
	PicDispInfo.DotBuf = &gImage_ic_smg_24[2];
	AppScreenPictureDisp(&PicDispInfo);				

	/* 显示文字 */
	LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
	LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 16;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = strCodeType;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		

	LetterSource.letterStrAddr = String;
	LetterSource.letterStrLen = stringLen;

	switch (strCodeType)
    {
    	case LETTER_TYPTE_UTF8: break;
    	case LETTER_TYPTE_UNICODE: 
			AppScreenTextLayoutUnicodeDisp(&LetterSource);
			break;
		case LETTER_TYPTE_GB:
			AppScreenTextLayoutGBKDisp(&LetterSource);
    		break;
    	default :
    		break;
    }
}

//**********************************************************************
// 函数功能：  应用消息提醒显示
// 输入参数： String	字符串地址
//			  stringLen 字符串长度
//			  strCodeType 字符串编码类型
// 返回参数： 无
void AppRemindAppMsgDisp(uint8_t *String,uint8_t stringLen, letter_code_type_t strCodeType)
{
	letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	
	/* 显示图标 */
    PicDispInfo.AxisX = 0;
    PicDispInfo.AxisY = 0;
	PicDispInfo.AreaLength = 32;
	PicDispInfo.AreaHeight = 32;
    PicDispInfo.PictureLength = gImage_ic_msg_24[0];
    PicDispInfo.PictureHeight = gImage_ic_msg_24[1];
	PicDispInfo.Alic = ALIC_MIDDLE;	
	PicDispInfo.DotBuf = &gImage_ic_msg_24[2];
	AppScreenPictureDisp(&PicDispInfo);				

	/* 显示文字 */
	LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
	LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 16;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = strCodeType;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		

	LetterSource.letterStrAddr = String;
	LetterSource.letterStrLen = stringLen;

	switch (strCodeType)
    {
    	case LETTER_TYPTE_UTF8: break;
    	case LETTER_TYPTE_UNICODE: 
			AppScreenTextLayoutUnicodeDisp(&LetterSource);
			break;
		case LETTER_TYPTE_GB:
			AppScreenTextLayoutGBKDisp(&LetterSource);
    		break;
    	default :
    		break;
    }
}

//**********************************************************************
// 函数功能：  OLED全屏显示消息内容
// 输入参数： String	字符串地址
//			  stringLen 字符串长度
//			  strCodeType 字符串编码类型
// 返回参数： 无
//**********************************************************************
void AppRemindMsgContentDisp(uint8_t *String,uint8_t stringLen, letter_code_type_t strCodeType)
{
	letter_source_s LetterSource;

	/* 显示文字 */
	LetterSource.letterInfo.letterAxisInfo.axis_X = 0;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
	LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 96;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 12;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_LEFT;//ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = strCodeType;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_12_B_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_12X12;		// 中文字符字体		

	LetterSource.letterStrAddr = String;
	LetterSource.letterStrLen = stringLen;

	switch (strCodeType)
    {
    	case LETTER_TYPTE_UTF8: break;
    	case LETTER_TYPTE_UNICODE: 
			AppScreenTextLayoutUnicodeDisp(&LetterSource);
			break;
		case LETTER_TYPTE_GB:
			AppScreenTextLayoutGBKDisp(&LetterSource);
    		break;
    	default :
    		break;
    }
}

//**********************************************************************
// 函数功能：  闹钟提醒
// 输入参数： TimeDate	闹钟时间，时：分：秒
//			  TimeFormat 格式，12小时制或24小时制
// 返回参数： 无
//**********************************************************************
void AppRemindClockDisp(time_data_t* TimeDate, time_format_t TimeFormat)
{
	letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	uint8_t TmpBuf[32], TmpLen;

	/* 显示闹钟图标 */
    PicDispInfo.AxisX = 0;
    PicDispInfo.AxisY = 0;
	PicDispInfo.AreaLength = 32;
	PicDispInfo.AreaHeight = 32;
    PicDispInfo.PictureLength = gImage_ic_notifications_active_24[0];
    PicDispInfo.PictureHeight = gImage_ic_notifications_active_24[1];
	PicDispInfo.Alic = ALIC_MIDDLE;	
	PicDispInfo.DotBuf = &gImage_ic_notifications_active_24[2];	
	AppScreenPictureDisp(&PicDispInfo);	

	/* 显示内容 */
    LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
    LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
    LetterSource.letterInfo.letterAxisInfo.height = 32;
    LetterSource.letterInfo.letterAxisInfo.length = 64;   
    LetterSource.letterInfo.letterAxisInfo.dispmode = EXRULE;   
    
	if(TimeDate->Hour > 24) TimeDate->Hour = 24;
	if(TimeDate->Min > 60) TimeDate->Min = 60;
	
    switch(TimeFormat)
    {
        case TIME_FORMAT_24:                 
            TmpLen        = 5;
            TmpBuf[0]     = TimeDate->Hour / 10;
            TmpBuf[1]     = TimeDate->Hour % 10;
            TmpBuf[2]     = 10;                        // 冒号
            TmpBuf[3]     = TimeDate->Min  / 10;
            TmpBuf[4]     = TimeDate->Min  % 10;
            Mid_Screen_FontTgl24AlignDisp(&LetterSource.letterInfo, TmpBuf, TmpLen);  
            break;
        case TIME_FORMAT_12:   
			if(TimeDate->Hour >=12)
			{
				TmpBuf[0] = 'P';
				AppScreenLocalAsciiDisp(34, 6, *TmpBuf, APP_ASCII_SIZE_12_B_A);
			}
			else
			{
				TmpBuf[0] = 'A';
				AppScreenLocalAsciiDisp(34, 6, *TmpBuf, APP_ASCII_SIZE_12_B_A);
			}

			if(TimeDate->Hour >=12)
				TimeDate->Hour -= 12;
			
            TmpLen        = 5;
            TmpBuf[0]     = TimeDate->Hour / 10;
            TmpBuf[1]     = TimeDate->Hour % 10;
            TmpBuf[2]     = 10;                        // 冒号
            TmpBuf[3]     = TimeDate->Min  / 10;
            TmpBuf[4]     = TimeDate->Min  % 10;
            Mid_Screen_FontTgl24AlignDisp(&LetterSource.letterInfo, TmpBuf, TmpLen); 			
            break;  
        default:
            break;
    }	
}

//**********************************************************************
// 函数功能：  计步目标达成提醒
// 输入参数： Language 语言类型			
// 返回参数： 无
//**********************************************************************
void AppRemindGoalAttainedDisp(systerm_languge_t LanguageType)
{
    letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	uint8_t TitleEn[] = "Goal Attained";

	switch(LanguageType)
	{
		case SYS_ENGLISH_TYPE:
			/* 显示走路图标 */
		   PicDispInfo.AxisX = 0;
			PicDispInfo.AxisY = 0;
			PicDispInfo.AreaLength = 32;
			PicDispInfo.AreaHeight = 32;
			PicDispInfo.PictureLength = 24;
			PicDispInfo.PictureHeight = 24;
			PicDispInfo.Alic = ALIC_MIDDLE;	

			PicDispInfo.DotBuf = &gImage_ic_goal_24[2];
			AppScreenPictureDisp(&PicDispInfo);	
			
			/* 显示文字 */
			LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
			LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
			LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
			LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
			LetterSource.letterInfo.frontHeight = 16;			// 字符高度
		//	LetterSource.letterInfo.interval = 0;
			LetterSource.letterInfo.alic = ALIC_MIDDLE;
		//	LetterSource.letterInfo.language = 0;
			LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
			LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
			LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		

			LetterSource.letterStrAddr = TitleEn;
			LetterSource.letterStrLen = strlen("Goal Attained");			
			AppScreenTextLayoutGBKDisp(&LetterSource);				
			
			break;
		case SYS_CHINESE_TYPE:
		    PicDispInfo.AxisX = 0;
			PicDispInfo.AxisY = 0;
			PicDispInfo.AreaLength = 96;
			PicDispInfo.AreaHeight = 32;
			PicDispInfo.PictureLength = 96;
			PicDispInfo.PictureHeight = 32;
			PicDispInfo.Alic = ALIC_MIDDLE;	
			PicDispInfo.DotBuf = &gImage_GoalAttainedCn[2];
			
			AppScreenPictureDisp(&PicDispInfo);	
			break;
		default:
            /* 显示走路图标 */
           PicDispInfo.AxisX = 0;
            PicDispInfo.AxisY = 0;
            PicDispInfo.AreaLength = 32;
            PicDispInfo.AreaHeight = 32;
            PicDispInfo.PictureLength = 24;
            PicDispInfo.PictureHeight = 24;
            PicDispInfo.Alic = ALIC_MIDDLE; 

            PicDispInfo.DotBuf = &gImage_ic_goal_24[2];
            AppScreenPictureDisp(&PicDispInfo); 
            
            /* 显示文字 */
            LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
            LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
            LetterSource.letterInfo.letterAxisInfo.height = 32; // 显示区域高度
            LetterSource.letterInfo.letterAxisInfo.length = 64; // 显示区域长度
            LetterSource.letterInfo.frontHeight = 16;           // 字符高度
        //  LetterSource.letterInfo.interval = 0;
            LetterSource.letterInfo.alic = ALIC_MIDDLE;
        //  LetterSource.letterInfo.language = 0;
            LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
            LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;  // 英文字符字体
            LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;       // 中文字符字体       

            LetterSource.letterStrAddr = TitleEn;
            LetterSource.letterStrLen = strlen("Goal Attained");            
            AppScreenTextLayoutGBKDisp(&LetterSource);
			break;
	}
	
	#if 0
	letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	
	/* 显示图标 */
    PicDispInfo.AxisX = 0;
    PicDispInfo.AxisY = 0;
	PicDispInfo.AreaLength = 32;
	PicDispInfo.AreaHeight = 32;
    PicDispInfo.PictureLength = gImage_ic_goal_24[0];
    PicDispInfo.PictureHeight = gImage_ic_goal_24[1];
	PicDispInfo.Alic = ALIC_MIDDLE;	
	PicDispInfo.DotBuf = &gImage_ic_goal_24[2];
	AppScreenPictureDisp(&PicDispInfo);				

	/* 显示文字 */
	uint8_t TmpStringEN[] = "Goal Attained";
	uint8_t TmpStringCN[] = {
		8, 
		0xC4, 0xBF, 0xB1, 0xEA, 0xB4, 0xEF, 0xB3, 0xC9};
	LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
	LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 16;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		
	switch(Language)
	{
		case SYS_ENGLISH_TYPE:
			LetterSource.letterStrAddr = TmpStringEN;
			LetterSource.letterStrLen = strlen("Goal Attained");			
			AppScreenTextLayoutGBKDisp(&LetterSource);					
			break;
		case SYS_CHINESE_TYPE:
			LetterSource.letterStrAddr = &TmpStringCN[1];
			LetterSource.letterStrLen = TmpStringCN[0];		
			AppScreenTextLayoutGBKDisp(&LetterSource);								
			break;
		default:
            LetterSource.letterStrAddr = TmpStringEN;
            LetterSource.letterStrLen = strlen("Goal Attained");            
            AppScreenTextLayoutGBKDisp(&LetterSource);
			break;
	}
	#endif
}


//**********************************************************************
// 函数功能：  久坐提醒
// 输入参数： Language 语言类型			
// 返回参数： 无
//**********************************************************************
void AppRemindLongSitDisp(systerm_languge_t LanguageType)
{
    letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	uint8_t TitleEn[] = "Move!";

	switch(LanguageType)
	{
		case SYS_ENGLISH_TYPE:
			/* 显示走路图标 */
		   PicDispInfo.AxisX = 0;
			PicDispInfo.AxisY = 0;
			PicDispInfo.AreaLength = 32;
			PicDispInfo.AreaHeight = 32;
			PicDispInfo.PictureLength = 24;
			PicDispInfo.PictureHeight = 24;
			PicDispInfo.Alic = ALIC_MIDDLE;	

			PicDispInfo.DotBuf = &gImage_ic_stand_24[2];
			AppScreenPictureDisp(&PicDispInfo);	
			
			/* 显示文字 */
			LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
			LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
			LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
			LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
			LetterSource.letterInfo.frontHeight = 16;			// 字符高度
		//	LetterSource.letterInfo.interval = 0;
			LetterSource.letterInfo.alic = ALIC_MIDDLE;
		//	LetterSource.letterInfo.language = 0;
			LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
			LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
			LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		

			LetterSource.letterStrAddr = TitleEn;
			LetterSource.letterStrLen = strlen("Move!");			
			AppScreenTextLayoutGBKDisp(&LetterSource);				
			
			break;
		case SYS_CHINESE_TYPE:
		    PicDispInfo.AxisX = 0;
			PicDispInfo.AxisY = 0;
			PicDispInfo.AreaLength = 96;
			PicDispInfo.AreaHeight = 32;
			PicDispInfo.PictureLength = 96;
			PicDispInfo.PictureHeight = 32;
			PicDispInfo.Alic = ALIC_MIDDLE;	
			PicDispInfo.DotBuf = &gImage_MoveCn[2];
			
			AppScreenPictureDisp(&PicDispInfo);	
			break;
		default:
            /* 显示走路图标 */
           PicDispInfo.AxisX = 0;
            PicDispInfo.AxisY = 0;
            PicDispInfo.AreaLength = 32;
            PicDispInfo.AreaHeight = 32;
            PicDispInfo.PictureLength = 24;
            PicDispInfo.PictureHeight = 24;
            PicDispInfo.Alic = ALIC_MIDDLE; 

            PicDispInfo.DotBuf = &gImage_ic_stand_24[2];
            AppScreenPictureDisp(&PicDispInfo); 
            
            /* 显示文字 */
            LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
            LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
            LetterSource.letterInfo.letterAxisInfo.height = 32; // 显示区域高度
            LetterSource.letterInfo.letterAxisInfo.length = 64; // 显示区域长度
            LetterSource.letterInfo.frontHeight = 16;           // 字符高度
        //  LetterSource.letterInfo.interval = 0;
            LetterSource.letterInfo.alic = ALIC_MIDDLE;
        //  LetterSource.letterInfo.language = 0;
            LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
            LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;  // 英文字符字体
            LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;       // 中文字符字体       

            LetterSource.letterStrAddr = TitleEn;
            LetterSource.letterStrLen = strlen("Move!");            
            AppScreenTextLayoutGBKDisp(&LetterSource);
			break;
	}	
	
	#if 0
	letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	
	/* 显示图标 */
    PicDispInfo.AxisX = 0;
    PicDispInfo.AxisY = 0;
	PicDispInfo.AreaLength = 32;
	PicDispInfo.AreaHeight = 32;
    PicDispInfo.PictureLength = gImage_ic_stand_24[0];
    PicDispInfo.PictureHeight = gImage_ic_stand_24[1];
	PicDispInfo.Alic = ALIC_MIDDLE;	
	PicDispInfo.DotBuf = &gImage_ic_stand_24[2];
	AppScreenPictureDisp(&PicDispInfo);				

	/* 显示文字 */
	uint8_t TmpStringEN[] = "Move!";
	uint8_t TmpStringCN[] = {
		8, 
		0xBB, 0xEE, 0xB6, 0xAF, 0xD2, 0xBB, 0xCF, 0xC2};
	LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
	LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 16;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		
	switch(Language)
	{
		case SYS_ENGLISH_TYPE:
			LetterSource.letterStrAddr = TmpStringEN;
			LetterSource.letterStrLen = strlen("Move!");			
			AppScreenTextLayoutGBKDisp(&LetterSource);					
			break;
		case SYS_CHINESE_TYPE:
			LetterSource.letterStrAddr = &TmpStringCN[1];
			LetterSource.letterStrLen = TmpStringCN[0];		
			AppScreenTextLayoutGBKDisp(&LetterSource);								
			break;
		default:
            LetterSource.letterStrAddr = TmpStringEN;
            LetterSource.letterStrLen = strlen("Move!");            
            AppScreenTextLayoutGBKDisp(&LetterSource);
			break;
	}
	#endif
}

//**********************************************************************
// 函数功能：  蓝牙状态提醒
// 输入参数：  BleState 蓝牙状态，开/关
//				Language 语言类型
// 返回参数： 无
//**********************************************************************
void AppRemindBleStateDisp(binary_type_t BleState, systerm_languge_t Language)
{
	letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	
	/* 显示图标 */
    PicDispInfo.AxisX = 0;
    PicDispInfo.AxisY = 0;
	PicDispInfo.AreaLength = 32;
	PicDispInfo.AreaHeight = 32;
    PicDispInfo.PictureLength = gImage_ic_bt_24[0];
    PicDispInfo.PictureHeight = gImage_ic_bt_24[1];
	PicDispInfo.Alic = ALIC_MIDDLE;	
	switch (BleState)
    {
    	case BINARY_TYPE_ON:
			PicDispInfo.DotBuf = &gImage_ic_bt_24[2];
			AppScreenPictureDisp(&PicDispInfo);				
    		break;
    	case BINARY_TYPE_OFF:
			PicDispInfo.DotBuf = &gImage_ic_bt_off_24[2];
			AppScreenPictureDisp(&PicDispInfo);				
    		break;
    	default:
            PicDispInfo.DotBuf = &gImage_ic_bt_24[2];
            AppScreenPictureDisp(&PicDispInfo); 
    		break;
    }	
		
	/* 显示文字 */
	uint8_t TmpStringENOn[] = "Connected";
	uint8_t TmpStringENOff[] = "Disconnect";
	uint8_t TmpStringCNOn[] = {8, 0xC0, 0xB6, 0xD1, 0xC0, 0xC1, 0xAC, 0xBD, 0xD3};
	uint8_t TmpStringCNOff[] = {8, 0xC0, 0xB6, 0xD1, 0xC0, 0xB6, 0xCF, 0xBF, 0xAA};
	LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
	LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 16;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		
	switch(Language)
	{
		case SYS_ENGLISH_TYPE:
			if(BINARY_TYPE_ON == BleState)
			{
				LetterSource.letterStrAddr = TmpStringENOn;
				LetterSource.letterStrLen = strlen("Connected");			
				AppScreenTextLayoutGBKDisp(&LetterSource);					
			}
			else
			{
				LetterSource.letterStrAddr = TmpStringENOff;
				LetterSource.letterStrLen = strlen("Disconnect");			
				AppScreenTextLayoutGBKDisp(&LetterSource);					
			}
			break;
		case SYS_CHINESE_TYPE:
			if(BINARY_TYPE_ON == BleState)
			{
				LetterSource.letterStrAddr = &TmpStringCNOn[1];
				LetterSource.letterStrLen = TmpStringCNOn[0];			
				AppScreenTextLayoutGBKDisp(&LetterSource);					
			}
			else
			{
				LetterSource.letterStrAddr = &TmpStringCNOff[1];
				LetterSource.letterStrLen = TmpStringCNOff[0];		
				AppScreenTextLayoutGBKDisp(&LetterSource);					
			}			
			break;
		default:
            if(BINARY_TYPE_ON == BleState)
            {
                LetterSource.letterStrAddr = TmpStringENOn;
                LetterSource.letterStrLen = strlen("Connected");            
                AppScreenTextLayoutGBKDisp(&LetterSource);                  
            }
            else
            {
                LetterSource.letterStrAddr = TmpStringENOff;
                LetterSource.letterStrLen = strlen("Disconnect");           
                AppScreenTextLayoutGBKDisp(&LetterSource);                  
            }
			break;
	}
}

//**********************************************************************
// 函数功能：  显示提醒：请连接手机上传数据
// 输入参数：  LanguageType 语言类型
// 返回参数： 无
//**********************************************************************
void AppRemindPlzUploadData(systerm_languge_t LanguageType)
{
	#if 1
    letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	uint8_t TitleEn1[] = "Upload Data First";
	uint8_t TitleEn2[] = "Upload";

	switch(LanguageType)
	{
		case SYS_ENGLISH_TYPE:
		//	LetterSource.letterInfo.interval = 0;
			LetterSource.letterInfo.alic = ALIC_MIDDLE;
		//	LetterSource.letterInfo.language = 0;
			LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
			LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
			LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体	
		
			LetterSource.letterInfo.letterAxisInfo.axis_X = 0;
			LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
			LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
			LetterSource.letterInfo.letterAxisInfo.length = 96;	// 显示区域长度
			LetterSource.letterInfo.frontHeight = 16;			// 字符高度
			LetterSource.letterStrAddr = TitleEn1;
			LetterSource.letterStrLen = strlen("Upload Data First");			
			AppScreenTextLayoutGBKDisp(&LetterSource);
			
			LetterSource.letterInfo.alic = ALIC_LEFT;
			LetterSource.letterInfo.letterAxisInfo.axis_X = 0;
			LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
			LetterSource.letterInfo.letterAxisInfo.height = 16;	// 显示区域高度
			LetterSource.letterInfo.letterAxisInfo.length = 96;	// 显示区域长度
			LetterSource.letterInfo.frontHeight = 16;			// 字符高度		
			LetterSource.letterStrAddr = TitleEn2;
			LetterSource.letterStrLen = strlen("Upload");	// 重新覆盖越界显示部分
			AppScreenTextLayoutGBKDisp(&LetterSource);	
			break;
		case SYS_CHINESE_TYPE:
		    PicDispInfo.AxisX = 0;
			PicDispInfo.AxisY = 0;
			PicDispInfo.AreaLength = 96;
			PicDispInfo.AreaHeight = 32;
			PicDispInfo.PictureLength = 96;
			PicDispInfo.PictureHeight = 32;
			PicDispInfo.Alic = ALIC_MIDDLE;	
			PicDispInfo.DotBuf = &gImage_UploadData[2];
			
			AppScreenPictureDisp(&PicDispInfo);	
			break;
		default:
            //  LetterSource.letterInfo.interval = 0;
            LetterSource.letterInfo.alic = ALIC_MIDDLE;
        //  LetterSource.letterInfo.language = 0;
            LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
            LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;  // 英文字符字体
            LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;       // 中文字符字体   
        
            LetterSource.letterInfo.letterAxisInfo.axis_X = 0;
            LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
            LetterSource.letterInfo.letterAxisInfo.height = 32; // 显示区域高度
            LetterSource.letterInfo.letterAxisInfo.length = 96; // 显示区域长度
            LetterSource.letterInfo.frontHeight = 16;           // 字符高度
            LetterSource.letterStrAddr = TitleEn1;
            LetterSource.letterStrLen = strlen("Upload Data First");            
            AppScreenTextLayoutGBKDisp(&LetterSource);
            
            LetterSource.letterInfo.alic = ALIC_LEFT;
            LetterSource.letterInfo.letterAxisInfo.axis_X = 0;
            LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
            LetterSource.letterInfo.letterAxisInfo.height = 16; // 显示区域高度
            LetterSource.letterInfo.letterAxisInfo.length = 96; // 显示区域长度
            LetterSource.letterInfo.frontHeight = 16;           // 字符高度     
            LetterSource.letterStrAddr = TitleEn2;
            LetterSource.letterStrLen = strlen("Upload");   // 重新覆盖越界显示部分
            AppScreenTextLayoutGBKDisp(&LetterSource);
			break;
	}	
	#endif
	
	#if 0
	letter_source_s LetterSource;
	uint8_t TitleEn1[] = "Upload Data First";
	uint8_t TitleEn2[] = "Upload";
	uint8_t TitleCn1[] = {		
		10,
		0xC7, 0xEB, 0xC1, 0xAC, 0xBD, 0xD3, 0xCA, 0xD6, 0xBB, 0xFA};
	uint8_t TitleCn2[] = {		
		12,
		0xC9, 0xCF, 0xB4, 0xAB, 0xC0, 0xFA, 0xCA, 0xB7, 0xCA, 0xFD, 0xBE, 0xDD};
	
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体	
	
	switch(LanguageType)
	{
		case SYS_ENGLISH_TYPE:
			LetterSource.letterInfo.letterAxisInfo.axis_X = 0;
			LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
			LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
			LetterSource.letterInfo.letterAxisInfo.length = 96;	// 显示区域长度
			LetterSource.letterInfo.frontHeight = 16;			// 字符高度
			LetterSource.letterStrAddr = TitleEn1;
			LetterSource.letterStrLen = strlen("Upload Data First");			
			AppScreenTextLayoutGBKDisp(&LetterSource);
			
			LetterSource.letterInfo.alic = ALIC_LEFT;
			LetterSource.letterInfo.letterAxisInfo.axis_X = 0;
			LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
			LetterSource.letterInfo.letterAxisInfo.height = 16;	// 显示区域高度
			LetterSource.letterInfo.letterAxisInfo.length = 96;	// 显示区域长度
			LetterSource.letterInfo.frontHeight = 16;			// 字符高度		
			LetterSource.letterStrAddr = TitleEn2;
			LetterSource.letterStrLen = strlen("Upload");	// 重新覆盖越界显示部分
			AppScreenTextLayoutGBKDisp(&LetterSource);			
		
			break;
		case SYS_CHINESE_TYPE:
			LetterSource.letterInfo.letterAxisInfo.axis_X = 0;
			LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
			LetterSource.letterInfo.letterAxisInfo.height = 16;	// 显示区域高度
			LetterSource.letterInfo.letterAxisInfo.length = 96;	// 显示区域长度
			LetterSource.letterInfo.frontHeight = 16;			// 字符高度			
			LetterSource.letterStrAddr = &TitleCn1[1];
			LetterSource.letterStrLen = TitleCn1[0];		
			AppScreenTextLayoutGBKDisp(&LetterSource);		// 请连接手机
		
			LetterSource.letterInfo.letterAxisInfo.axis_X = 0;
			LetterSource.letterInfo.letterAxisInfo.axis_Y = 16;
			LetterSource.letterInfo.letterAxisInfo.height = 16;	// 显示区域高度
			LetterSource.letterInfo.letterAxisInfo.length = 96;	// 显示区域长度
			LetterSource.letterInfo.frontHeight = 16;			// 字符高度			
			LetterSource.letterStrAddr = &TitleCn2[1];
			LetterSource.letterStrLen = TitleCn2[0];		
		AppScreenTextLayoutGBKDisp(&LetterSource);		// 上传历史数据		
			break;
		default: break;
	}	
	#endif
}

//**********************************************************************
// 函数功能：  跑步主界面显示
// 输入参数：  LanguageType 语言类型
// 返回参数： 无
//**********************************************************************
void AppRunMainInterfaceDisp(systerm_languge_t LanguageType)
{
    letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	uint8_t TitleEn[] = "Run";

	switch(LanguageType)
	{
		case SYS_ENGLISH_TYPE:
			/* 显示走路图标 */
		   PicDispInfo.AxisX = 0;
			PicDispInfo.AxisY = 0;
			PicDispInfo.AreaLength = 32;
			PicDispInfo.AreaHeight = 32;
			PicDispInfo.PictureLength = 24;
			PicDispInfo.PictureHeight = 24;
			PicDispInfo.Alic = ALIC_MIDDLE;	

			PicDispInfo.DotBuf = &gImage_ic_run_24[2];
			AppScreenPictureDisp(&PicDispInfo);	
			
			/* 显示文字 */
			LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
			LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
			LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
			LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
			LetterSource.letterInfo.frontHeight = 16;			// 字符高度
		//	LetterSource.letterInfo.interval = 0;
			LetterSource.letterInfo.alic = ALIC_MIDDLE;
		//	LetterSource.letterInfo.language = 0;
			LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
			LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
			LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		

			LetterSource.letterStrAddr = TitleEn;
			LetterSource.letterStrLen = strlen("Run");			
			AppScreenTextLayoutGBKDisp(&LetterSource);				
			
			break;
		case SYS_CHINESE_TYPE:
		    PicDispInfo.AxisX = 0;
			PicDispInfo.AxisY = 0;
			PicDispInfo.AreaLength = 96;
			PicDispInfo.AreaHeight = 32;
			PicDispInfo.PictureLength = 96;
			PicDispInfo.PictureHeight = 32;
			PicDispInfo.Alic = ALIC_MIDDLE;	
			PicDispInfo.DotBuf = &gImage_RunCn[2];
			
			AppScreenPictureDisp(&PicDispInfo);	
			break;
		default:
            /* 显示走路图标 */
           PicDispInfo.AxisX = 0;
            PicDispInfo.AxisY = 0;
            PicDispInfo.AreaLength = 32;
            PicDispInfo.AreaHeight = 32;
            PicDispInfo.PictureLength = 24;
            PicDispInfo.PictureHeight = 24;
            PicDispInfo.Alic = ALIC_MIDDLE; 

            PicDispInfo.DotBuf = &gImage_ic_run_24[2];
            AppScreenPictureDisp(&PicDispInfo); 
            
            /* 显示文字 */
            LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
            LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
            LetterSource.letterInfo.letterAxisInfo.height = 32; // 显示区域高度
            LetterSource.letterInfo.letterAxisInfo.length = 64; // 显示区域长度
            LetterSource.letterInfo.frontHeight = 16;           // 字符高度
        //  LetterSource.letterInfo.interval = 0;
            LetterSource.letterInfo.alic = ALIC_MIDDLE;
        //  LetterSource.letterInfo.language = 0;
            LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
            LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;  // 英文字符字体
            LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;       // 中文字符字体       

            LetterSource.letterStrAddr = TitleEn;
            LetterSource.letterStrLen = strlen("Run");          
            AppScreenTextLayoutGBKDisp(&LetterSource);
			break;
	}
	
	#if 0
	letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	uint8_t TitleEn[] = "Run";
	uint8_t TitleCn[] = {		
		6,
		0xBB, 0xA7, 0xCD, 0xE2, 0xC5, 0xDC};
	
	/* 显示图标 */
    PicDispInfo.AxisX = 0;
    PicDispInfo.AxisY = 0;
	PicDispInfo.AreaLength = 32;
	PicDispInfo.AreaHeight = 32;
    PicDispInfo.PictureLength = gImage_ic_run_24[0];
    PicDispInfo.PictureHeight = gImage_ic_run_24[1];
	PicDispInfo.Alic = ALIC_MIDDLE;	

	PicDispInfo.DotBuf = &gImage_ic_run_24[2];
	AppScreenPictureDisp(&PicDispInfo);	

		
	/* 显示文字 */
	LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
	LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 16;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		
	switch(LanguageType)
	{
		case SYS_ENGLISH_TYPE:
			LetterSource.letterStrAddr = TitleEn;
			LetterSource.letterStrLen = strlen("Run");			
			AppScreenTextLayoutGBKDisp(&LetterSource);			
			break;
		case SYS_CHINESE_TYPE:
			LetterSource.letterStrAddr = &TitleCn[1];
			LetterSource.letterStrLen = TitleCn[0];		
			AppScreenTextLayoutGBKDisp(&LetterSource);	
			break;
		default:
            LetterSource.letterStrAddr = TitleEn;
            LetterSource.letterStrLen = strlen("Run");          
            AppScreenTextLayoutGBKDisp(&LetterSource);  
			break;
	}
	#endif	
}

//**********************************************************************
// 函数功能：  跑步的步数和步频显示
// 输入参数：  StepNum 步数
//			   StepFrequency 步频
// 返回参数： 无
//**********************************************************************
void AppRunStepNumFrequencyDisp(uint32_t StepNum, uint32_t StepFrequency)
{
	letter_source_s LetterSource;
	uint8       tempBuf[10], tempExchangeBuf[10], tempLen;
	uint32_t TmpNum;	
	
	/* 双竖线显示 */
	Mid_Screen_DoubleLine(47);
	
	/* 步数显示 */ 
	// 计算计步值共有几位
	if(StepNum > 99999)	StepNum = 99999;
	tempLen = 0;
	TmpNum = StepNum;
	do
	{
		tempBuf[tempLen] = TmpNum % 10;
		TmpNum /= 10;
		tempLen++;
	}while(TmpNum > 0);

	// 交换数组值
	for(uint32_t i = 0; i < tempLen; i++)
	{
		tempExchangeBuf[i] = tempBuf[tempLen-1-i];
	}
	
    LetterSource.letterInfo.letterAxisInfo.axis_X = 0;
    LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
    LetterSource.letterInfo.letterAxisInfo.height = 20;
    LetterSource.letterInfo.letterAxisInfo.length = 47;   
    LetterSource.letterInfo.letterAxisInfo.dispmode = EXRULE;   
	Mid_Screen_FontTgl24AlignDisp(&LetterSource.letterInfo, tempExchangeBuf, tempLen);  	
	
	/* 显示字符串 */
	uint8_t KcalString[] = "step";
	LetterSource.letterStrAddr = KcalString;
	LetterSource.letterStrLen = strlen("step");
	LetterSource.letterInfo.letterAxisInfo.axis_X = 0;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 20;
	LetterSource.letterInfo.letterAxisInfo.height = 12;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 47;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 12;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_12_B_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体
	AppScreenTextLayoutGBKDisp(&LetterSource);	
	
	/* 步频显示 */
	// 计算计步值共有几位
	if(StepFrequency > 99999)	StepFrequency = 99999;
	tempLen = 0;
	TmpNum = StepFrequency;
	do
	{
		tempBuf[tempLen] = TmpNum % 10;
		TmpNum /= 10;
		tempLen++;
	}while(TmpNum > 0);

	// 交换数组值
	for(uint32_t i = 0; i < tempLen; i++)
	{
		tempExchangeBuf[i] = tempBuf[tempLen-1-i];
	}
	
    LetterSource.letterInfo.letterAxisInfo.axis_X = 49;
    LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
    LetterSource.letterInfo.letterAxisInfo.height = 20;
    LetterSource.letterInfo.letterAxisInfo.length = 47;   
    LetterSource.letterInfo.letterAxisInfo.dispmode = EXRULE;   
	Mid_Screen_FontTgl24AlignDisp(&LetterSource.letterInfo, tempExchangeBuf, tempLen);  	
	
	/* 显示字符串 */
	uint8_t StepFrequencyString[] = "spm";
	LetterSource.letterStrAddr = StepFrequencyString;
	LetterSource.letterStrLen = strlen("spm");
	LetterSource.letterInfo.letterAxisInfo.axis_X = 49;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 20;
	LetterSource.letterInfo.letterAxisInfo.height = 12;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 47;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 12;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_12_B_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体
	AppScreenTextLayoutGBKDisp(&LetterSource);		
}

//**********************************************************************
// 函数功能：  显示跑步的距离和速度
// 输入参数：  DistanceMeter 距离，单位米
//			   SpeedSec 速度，每1千米耗时，单位秒
// 返回参数： 无
//**********************************************************************
void AppRunDistanceSpeedDisp(uint32_t DistanceMeter, uint32_t SpeedSec)
{
	letter_source_s LetterSource;
	uint8       tempBuf[10], tempExchangeBuf[10], tempLen;
	uint32_t TmpNum;	
	
	/* 双竖线显示 */
	Mid_Screen_DoubleLine(42);
	
	/* 步数显示 */ 
	/* 显示数值 */
	/*
	e:计算结果  a：被除数  b：除数:
	1(四舍五入) ： e=(a+(b/2))/b   
	2(进一法)    ： e=(a+(b-1))/b	
		*/
	// 最大显示999.9km
	if(DistanceMeter > 999900) DistanceMeter = 999900;
	
	tempLen = 0;
	TmpNum = (DistanceMeter + 5) / 10;	// 4舍5入保留2位小数方法（个位和百位为小数）
	do
	{
		tempBuf[tempLen] = TmpNum % 10;
		TmpNum /= 10;
		tempLen++;
	}while(TmpNum > 0);

	// 若小于位数小于3，用0补齐，因为最小显示0.00
	if(tempLen < 3)
	{
		for(;tempLen < 3;tempLen++)
		{
			tempBuf[tempLen] = 0;
		}
	}
	
	// 交换数组值
	for(uint32_t i = 0; i < tempLen; i++)
	{
		tempExchangeBuf[i] = tempBuf[tempLen-1-i];
	}	
	
	// 插入小数点
	tempLen++;
	tempExchangeBuf[tempLen - 1] = tempExchangeBuf[tempLen - 2];	// 后移1位
	tempExchangeBuf[tempLen - 2] = tempExchangeBuf[tempLen - 3];
	tempExchangeBuf[tempLen - 3] = 11;		// 小数点	

    LetterSource.letterInfo.letterAxisInfo.axis_X = 0;
    LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
    LetterSource.letterInfo.letterAxisInfo.height = 20;
    LetterSource.letterInfo.letterAxisInfo.length = 42;   
    LetterSource.letterInfo.letterAxisInfo.dispmode = EXRULE;   
	Mid_Screen_FontTgl24AlignDisp(&LetterSource.letterInfo, tempExchangeBuf, tempLen);  		
	
	/* 显示字符串 */
	uint8_t KcalString[] = "km";
	LetterSource.letterStrAddr = KcalString;
	LetterSource.letterStrLen = strlen("km");
	LetterSource.letterInfo.letterAxisInfo.axis_X = 0;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 20;
	LetterSource.letterInfo.letterAxisInfo.height = 12;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 42;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 12;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_12_B_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体
	AppScreenTextLayoutGBKDisp(&LetterSource);	
	
	/* 步频显示 */
	// 计算计步值共有几位
	// 最大显示99分59秒
	if(SpeedSec > 5999) SpeedSec = 5999;
	
	if((SpeedSec / 60) >= 10)
	{
		tempLen = 7;
		tempExchangeBuf[0] = (SpeedSec / 60) / 10;
		tempExchangeBuf[1] = (SpeedSec / 60) % 10;
		tempExchangeBuf[2] = 15;	// "`"符号
		tempExchangeBuf[3] = (SpeedSec % 60) / 10;
		tempExchangeBuf[4] = (SpeedSec % 60) % 10;
		tempExchangeBuf[5] = 15;	// "`"符号
		tempExchangeBuf[6] = 15;	// "`"符号
	}
	else
	{
		tempLen = 6;
		tempExchangeBuf[0] = SpeedSec / 60;
		tempExchangeBuf[1] = 15;	// "`"符号
		tempExchangeBuf[2] = (SpeedSec % 60) / 10;
		tempExchangeBuf[3] = (SpeedSec % 60) % 10;
		tempExchangeBuf[4] = 15;	// "`"符号
		tempExchangeBuf[5] = 15;	// "`"符号		
	}
	
    LetterSource.letterInfo.letterAxisInfo.axis_X = 44;
    LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
    LetterSource.letterInfo.letterAxisInfo.height = 20;
    LetterSource.letterInfo.letterAxisInfo.length = 52;   
    LetterSource.letterInfo.letterAxisInfo.dispmode = EXRULE;   
	Mid_Screen_FontTgl24AlignDisp(&LetterSource.letterInfo, tempExchangeBuf, tempLen);  	
	
	/* 显示字符串 */
	uint8_t StepFrequencyString[] = "min/km";
	LetterSource.letterStrAddr = StepFrequencyString;
	LetterSource.letterStrLen = strlen("min/km");
	LetterSource.letterInfo.letterAxisInfo.axis_X = 44;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 20;
	LetterSource.letterInfo.letterAxisInfo.height = 12;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 52;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 12;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_12_B_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体
	AppScreenTextLayoutGBKDisp(&LetterSource);		
}

//**********************************************************************
// 函数功能： 显示登山模式主界面
// 输入参数：  LanguageType 语言类型
// 返回参数： 无
//**********************************************************************
void AppClimbMainInterfaceDisp(systerm_languge_t LanguageType)
{
    letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	uint8_t TitleEn[] = "Climb";

	switch(LanguageType)
	{
		case SYS_ENGLISH_TYPE:
			/* 显示走路图标 */
		   PicDispInfo.AxisX = 0;
			PicDispInfo.AxisY = 0;
			PicDispInfo.AreaLength = 32;
			PicDispInfo.AreaHeight = 32;
			PicDispInfo.PictureLength = 24;
			PicDispInfo.PictureHeight = 24;
			PicDispInfo.Alic = ALIC_MIDDLE;	

			PicDispInfo.DotBuf = &gImage_ic_climb_24[2];
			AppScreenPictureDisp(&PicDispInfo);	
			
			/* 显示文字 */
			LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
			LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
			LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
			LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
			LetterSource.letterInfo.frontHeight = 16;			// 字符高度
		//	LetterSource.letterInfo.interval = 0;
			LetterSource.letterInfo.alic = ALIC_MIDDLE;
		//	LetterSource.letterInfo.language = 0;
			LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
			LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
			LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		

			LetterSource.letterStrAddr = TitleEn;
			LetterSource.letterStrLen = strlen("Climb");			
			AppScreenTextLayoutGBKDisp(&LetterSource);				
			
			break;
		case SYS_CHINESE_TYPE:
		    PicDispInfo.AxisX = 0;
			PicDispInfo.AxisY = 0;
			PicDispInfo.AreaLength = 96;
			PicDispInfo.AreaHeight = 32;
			PicDispInfo.PictureLength = 96;
			PicDispInfo.PictureHeight = 32;
			PicDispInfo.Alic = ALIC_MIDDLE;	
			PicDispInfo.DotBuf = &gImage_ClimbCn[2];
			
			AppScreenPictureDisp(&PicDispInfo);	
			break;
		default:
            /* 显示走路图标 */
           PicDispInfo.AxisX = 0;
            PicDispInfo.AxisY = 0;
            PicDispInfo.AreaLength = 32;
            PicDispInfo.AreaHeight = 32;
            PicDispInfo.PictureLength = 24;
            PicDispInfo.PictureHeight = 24;
            PicDispInfo.Alic = ALIC_MIDDLE; 

            PicDispInfo.DotBuf = &gImage_ic_climb_24[2];
            AppScreenPictureDisp(&PicDispInfo); 
            
            /* 显示文字 */
            LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
            LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
            LetterSource.letterInfo.letterAxisInfo.height = 32; // 显示区域高度
            LetterSource.letterInfo.letterAxisInfo.length = 64; // 显示区域长度
            LetterSource.letterInfo.frontHeight = 16;           // 字符高度
        //  LetterSource.letterInfo.interval = 0;
            LetterSource.letterInfo.alic = ALIC_MIDDLE;
        //  LetterSource.letterInfo.language = 0;
            LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
            LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;  // 英文字符字体
            LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;       // 中文字符字体       

            LetterSource.letterStrAddr = TitleEn;
            LetterSource.letterStrLen = strlen("Climb");            
            AppScreenTextLayoutGBKDisp(&LetterSource);
			break;
	}
	
	#if 0
	letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	uint8_t TitleEn[] = "Climb";
	uint8_t TitleCn[] = {		
		4,
		0xB5, 0xC7, 0xC9, 0xBD};
	
	/* 显示图标 */
    PicDispInfo.AxisX = 0;
    PicDispInfo.AxisY = 0;
	PicDispInfo.AreaLength = 32;
	PicDispInfo.AreaHeight = 32;
    PicDispInfo.PictureLength = gImage_ic_climb_24[0];
    PicDispInfo.PictureHeight = gImage_ic_climb_24[1];
	PicDispInfo.Alic = ALIC_MIDDLE;	

	PicDispInfo.DotBuf = &gImage_ic_climb_24[2];
	AppScreenPictureDisp(&PicDispInfo);	

	/* 显示文字 */
	LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
	LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 16;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		
	switch(LanguageType)
	{
		case SYS_ENGLISH_TYPE:
			LetterSource.letterStrAddr = TitleEn;
			LetterSource.letterStrLen = strlen("Climb");			
			AppScreenTextLayoutGBKDisp(&LetterSource);			
			break;
		case SYS_CHINESE_TYPE:
			LetterSource.letterStrAddr = &TitleCn[1];
			LetterSource.letterStrLen = TitleCn[0];		
			AppScreenTextLayoutGBKDisp(&LetterSource);	
			break;
		default:
            LetterSource.letterStrAddr = &TitleCn[1];
            LetterSource.letterStrLen = TitleCn[0];     
            AppScreenTextLayoutGBKDisp(&LetterSource);
			break;
	}	
	#endif
}

//**********************************************************************
// 函数功能： 显示海拔和垂直速度
// 输入参数： Altitude 海拔，单位米
//            VerticalSpeed 垂直速度，单位m/h ，有符号数
// 返回参数： 无
//**********************************************************************
void AppClimbAltitudeVerticalSpeedDisp(uint32_t Altitude, int32_t VerticalSpeed)
{
	letter_source_s LetterSource;
	uint8       tempBuf[10], tempExchangeBuf[10], tempLen;
	uint32_t TmpNum;	
	
	/* 双竖线显示 */
	Mid_Screen_DoubleLine(47);
	
	/* 步数显示 */ 
	// 计算计步值共有几位
	if(Altitude > 99999) Altitude = 99999;
	tempLen = 0;
	TmpNum = Altitude;
	do
	{
		tempBuf[tempLen] = TmpNum % 10;
		TmpNum /= 10;
		tempLen++;
	}while(TmpNum > 0);

	// 交换数组值
	for(uint32_t i = 0; i < tempLen; i++)
	{
		tempExchangeBuf[i] = tempBuf[tempLen-1-i];
	}
	
    LetterSource.letterInfo.letterAxisInfo.axis_X = 0;
    LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
    LetterSource.letterInfo.letterAxisInfo.height = 20;
    LetterSource.letterInfo.letterAxisInfo.length = 47;   
    LetterSource.letterInfo.letterAxisInfo.dispmode = EXRULE;   
	Mid_Screen_FontTgl24AlignDisp(&LetterSource.letterInfo, tempExchangeBuf, tempLen);  	
	
	/* 显示字符串 */
	uint8_t KcalString[] = "m";
	LetterSource.letterStrAddr = KcalString;
	LetterSource.letterStrLen = strlen("m");
	LetterSource.letterInfo.letterAxisInfo.axis_X = 0;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 20;
	LetterSource.letterInfo.letterAxisInfo.height = 12;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 47;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 12;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_12_B_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体
	AppScreenTextLayoutGBKDisp(&LetterSource);	
	
	/* 步频显示 */
	// 计算计步值共有几位
	if(VerticalSpeed > 99999) VerticalSpeed = 99999;
	if(VerticalSpeed < -9999) VerticalSpeed = -9999;
	if(VerticalSpeed < 0)	
	{
		tempLen = 0;
		TmpNum = ~VerticalSpeed +1;
		do
		{
			tempBuf[tempLen] = TmpNum % 10;
			TmpNum /= 10;
			tempLen++;
		}while(TmpNum > 0);

		// 交换数组值
		for(uint32_t i = 0; i < tempLen; i++)
		{
			tempExchangeBuf[i] = tempBuf[tempLen-1-i];
		}	

		// 数组整体后移一位，首位用来放置负号
		for(uint32_t i = tempLen;i > 0;i--)
		{
			tempExchangeBuf[i] = tempExchangeBuf[i-1];
		}
		tempExchangeBuf[0] = 12;	// 负号
		tempLen++;
	}
	else
	{
		tempLen = 0;
        TmpNum = VerticalSpeed;
		do
		{
			tempBuf[tempLen] = TmpNum % 10;
			TmpNum /= 10;
			tempLen++;
		}while(TmpNum > 0);

		// 交换数组值
		for(uint32_t i = 0; i < tempLen; i++)
		{
			tempExchangeBuf[i] = tempBuf[tempLen-1-i];
		}		
	}

    LetterSource.letterInfo.letterAxisInfo.axis_X = 49;
    LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
    LetterSource.letterInfo.letterAxisInfo.height = 20;
    LetterSource.letterInfo.letterAxisInfo.length = 47;   
    LetterSource.letterInfo.letterAxisInfo.dispmode = EXRULE;   
	Mid_Screen_FontTgl24AlignDisp(&LetterSource.letterInfo, tempExchangeBuf, tempLen);  	
	
	/* 显示字符串 */
	uint8_t StepFrequencyString[] = "m/h";
	LetterSource.letterStrAddr = StepFrequencyString;
	LetterSource.letterStrLen = strlen("m/h");
	LetterSource.letterInfo.letterAxisInfo.axis_X = 49;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 20;
	LetterSource.letterInfo.letterAxisInfo.height = 12;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 47;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 12;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_12_B_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体
	AppScreenTextLayoutGBKDisp(&LetterSource);		
}

//**********************************************************************
// 函数功能： 游泳模式主界面显示
// 输入参数： LanguageType 语言类型
// 返回参数： 无
//**********************************************************************
void AppSwimMainInterfaceDisp(systerm_languge_t LanguageType)
{
    letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	uint8_t TitleEn[] = "Swim";

	switch(LanguageType)
	{
		case SYS_ENGLISH_TYPE:
			/* 显示走路图标 */
		   PicDispInfo.AxisX = 0;
			PicDispInfo.AxisY = 0;
			PicDispInfo.AreaLength = 32;
			PicDispInfo.AreaHeight = 32;
			PicDispInfo.PictureLength = 24;
			PicDispInfo.PictureHeight = 24;
			PicDispInfo.Alic = ALIC_MIDDLE;	

			PicDispInfo.DotBuf = &gImage_ic_swim_24[2];
			AppScreenPictureDisp(&PicDispInfo);	
			
			/* 显示文字 */
			LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
			LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
			LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
			LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
			LetterSource.letterInfo.frontHeight = 16;			// 字符高度
		//	LetterSource.letterInfo.interval = 0;
			LetterSource.letterInfo.alic = ALIC_MIDDLE;
		//	LetterSource.letterInfo.language = 0;
			LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
			LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
			LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		

			LetterSource.letterStrAddr = TitleEn;
			LetterSource.letterStrLen = strlen("Swim");			
			AppScreenTextLayoutGBKDisp(&LetterSource);				
			
			break;
		case SYS_CHINESE_TYPE:
		    PicDispInfo.AxisX = 0;
			PicDispInfo.AxisY = 0;
			PicDispInfo.AreaLength = 96;
			PicDispInfo.AreaHeight = 32;
			PicDispInfo.PictureLength = 96;
			PicDispInfo.PictureHeight = 32;
			PicDispInfo.Alic = ALIC_MIDDLE;	
			PicDispInfo.DotBuf = &gImage_SwimCn[2];
			
			AppScreenPictureDisp(&PicDispInfo);	
			break;
		default:
            /* 显示走路图标 */
           PicDispInfo.AxisX = 0;
            PicDispInfo.AxisY = 0;
            PicDispInfo.AreaLength = 32;
            PicDispInfo.AreaHeight = 32;
            PicDispInfo.PictureLength = 24;
            PicDispInfo.PictureHeight = 24;
            PicDispInfo.Alic = ALIC_MIDDLE; 

            PicDispInfo.DotBuf = &gImage_ic_swim_24[2];
            AppScreenPictureDisp(&PicDispInfo); 
            
            /* 显示文字 */
            LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
            LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
            LetterSource.letterInfo.letterAxisInfo.height = 32; // 显示区域高度
            LetterSource.letterInfo.letterAxisInfo.length = 64; // 显示区域长度
            LetterSource.letterInfo.frontHeight = 16;           // 字符高度
        //  LetterSource.letterInfo.interval = 0;
            LetterSource.letterInfo.alic = ALIC_MIDDLE;
        //  LetterSource.letterInfo.language = 0;
            LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
            LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;  // 英文字符字体
            LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;       // 中文字符字体       

            LetterSource.letterStrAddr = TitleEn;
            LetterSource.letterStrLen = strlen("Swim");         
            AppScreenTextLayoutGBKDisp(&LetterSource);
			break;
	}	
	
	#if 0
	letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	uint8_t TitleEn[] = "Swim";
	uint8_t TitleCn[] = {		
		8,
		0xCA, 0xD2, 0xC4, 0xDA, 0xD3, 0xCE, 0xD3, 0xBE};
	
	/* 显示图标 */
    PicDispInfo.AxisX = 0;
    PicDispInfo.AxisY = 0;
	PicDispInfo.AreaLength = 32;
	PicDispInfo.AreaHeight = 32;
    PicDispInfo.PictureLength = gImage_ic_swim_24[0];
    PicDispInfo.PictureHeight = gImage_ic_swim_24[1];
	PicDispInfo.Alic = ALIC_MIDDLE;	

	PicDispInfo.DotBuf = &gImage_ic_swim_24[2];
	AppScreenPictureDisp(&PicDispInfo);	
		
	/* 显示文字 */
	LetterSource.letterInfo.letterAxisInfo.axis_X = 32;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
	LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 64;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 16;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		
	switch(LanguageType)
	{
		case SYS_ENGLISH_TYPE:
			LetterSource.letterStrAddr = TitleEn;
			LetterSource.letterStrLen = strlen("Swim");			
			AppScreenTextLayoutGBKDisp(&LetterSource);			
			break;
		case SYS_CHINESE_TYPE:
			LetterSource.letterStrAddr = &TitleCn[1];
			LetterSource.letterStrLen = TitleCn[0];		
			AppScreenTextLayoutGBKDisp(&LetterSource);	
			break;
		default:
            LetterSource.letterStrAddr = TitleEn;
            LetterSource.letterStrLen = strlen("Swim");         
            AppScreenTextLayoutGBKDisp(&LetterSource);  
			break;
	}	
	#endif
}

//**********************************************************************
// 函数功能： 显示划水数和划水率
// 输入参数： StrokeNum 划水数
// 输入参数： StrokeFrequency 划水率
// 返回参数： 无
//**********************************************************************
void AppSwimbAltitudeSpeedDisp(uint32_t StrokeNum, uint32_t StrokeFrequency)
{
	letter_source_s LetterSource;
	uint8       tempBuf[10], tempExchangeBuf[10], tempLen;
	uint32_t TmpNum;	
	
	/* 双竖线显示 */
	Mid_Screen_DoubleLine(47);
	
	/* 步数显示 */ 
	// 计算计步值共有几位
	if(StrokeNum > 99999) StrokeNum = 99999;
	tempLen = 0;
	TmpNum = StrokeNum;
	do
	{
		tempBuf[tempLen] = TmpNum % 10;
		TmpNum /= 10;
		tempLen++;
	}while(TmpNum > 0);

	// 交换数组值
	for(uint32_t i = 0; i < tempLen; i++)
	{
		tempExchangeBuf[i] = tempBuf[tempLen-1-i];
	}
	
    LetterSource.letterInfo.letterAxisInfo.axis_X = 0;
    LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
    LetterSource.letterInfo.letterAxisInfo.height = 20;
    LetterSource.letterInfo.letterAxisInfo.length = 47;   
    LetterSource.letterInfo.letterAxisInfo.dispmode = EXRULE;   
	Mid_Screen_FontTgl24AlignDisp(&LetterSource.letterInfo, tempExchangeBuf, tempLen);  	
	
	/* 显示字符串 */
	uint8_t KcalString[] = "stroke";
	LetterSource.letterStrAddr = KcalString;
	LetterSource.letterStrLen = strlen("stroke");
	LetterSource.letterInfo.letterAxisInfo.axis_X = 0;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 20;
	LetterSource.letterInfo.letterAxisInfo.height = 12;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 47;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 12;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_12_B_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体
	AppScreenTextLayoutGBKDisp(&LetterSource);	
	
	/* 步频显示 */
	// 计算计步值共有几位
	if(StrokeFrequency > 99999) StrokeFrequency = 99999;
	tempLen = 0;
	TmpNum = StrokeFrequency;
	do
	{
		tempBuf[tempLen] = TmpNum % 10;
		TmpNum /= 10;
		tempLen++;
	}while(TmpNum > 0);

	// 交换数组值
	for(uint32_t i = 0; i < tempLen; i++)
	{
		tempExchangeBuf[i] = tempBuf[tempLen-1-i];
	}
	
    LetterSource.letterInfo.letterAxisInfo.axis_X = 49;
    LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
    LetterSource.letterInfo.letterAxisInfo.height = 20;
    LetterSource.letterInfo.letterAxisInfo.length = 47;   
    LetterSource.letterInfo.letterAxisInfo.dispmode = EXRULE;   
	Mid_Screen_FontTgl24AlignDisp(&LetterSource.letterInfo, tempExchangeBuf, tempLen);  	
	
	/* 显示字符串 */
	uint8_t StepFrequencyString[] = "spm";
	LetterSource.letterStrAddr = StepFrequencyString;
	LetterSource.letterStrLen = strlen("spm");
	LetterSource.letterInfo.letterAxisInfo.axis_X = 49;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 20;
	LetterSource.letterInfo.letterAxisInfo.height = 12;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 47;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 12;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_12_B_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体
	AppScreenTextLayoutGBKDisp(&LetterSource);		
}

//**********************************************************************
// 函数功能： 倒计时状态
// 输入参数： CountDownState 根据UI，传入参数为 1，2，3
// 返回参数： 无
//**********************************************************************
void AppCommonCountDownStateDisp(uint32_t CountDownState)
{
    uint8       tempBuf[10], tempLen;
    letter_source_s letterSourceInfo;   
    
    letterSourceInfo.letterInfo.letterAxisInfo.axis_X = 0;
    letterSourceInfo.letterInfo.letterAxisInfo.axis_Y = 0;
    letterSourceInfo.letterInfo.letterAxisInfo.height = 32;
    letterSourceInfo.letterInfo.letterAxisInfo.length = 96;   
    letterSourceInfo.letterInfo.letterAxisInfo.dispmode = EXRULE;   

	tempLen        = 1;
	tempBuf[0]     = CountDownState;
	Mid_Screen_FontTgl24AlignDisp(&letterSourceInfo.letterInfo, tempBuf, tempLen); 
}

//**********************************************************************
// 函数功能： 显示心率和卡路里值
// 输入参数： Hrd 心率值
//			  Kcal 卡路里值
// 返回参数： 无
//**********************************************************************
void AppCommonHrdKcalDisp(uint32_t Hrd, uint32_t Kcal)
{
	letter_source_s LetterSource;
	uint8       tempBuf[10], tempExchangeBuf[10], tempLen;
	uint32_t TmpNum;	
	
	/* 双竖线显示 */
	Mid_Screen_DoubleLine(47);
	
	/* 步数显示 */ 
	// 计算计步值共有几位
	if(Hrd > 99999) Hrd = 99999;

    if (Hrd != 0xff)
    {
        tempLen = 0;
        TmpNum = Hrd;
        do
        {
            tempBuf[tempLen] = TmpNum % 10;
            TmpNum /= 10;
            tempLen++;
        }while(TmpNum > 0);

        // 交换数组值
        for(uint32_t i = 0; i < tempLen; i++)
        {
            tempExchangeBuf[i] = tempBuf[tempLen-1-i];
        }
    }
    else
    {
        tempLen = 2;
        tempExchangeBuf[0] = 12;    // 连字符‘-’
        tempExchangeBuf[1] = 12;
    }
	
    LetterSource.letterInfo.letterAxisInfo.axis_X = 0;
    LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
    LetterSource.letterInfo.letterAxisInfo.height = 20;
    LetterSource.letterInfo.letterAxisInfo.length = 47;   
    LetterSource.letterInfo.letterAxisInfo.dispmode = EXRULE;   
	Mid_Screen_FontTgl24AlignDisp(&LetterSource.letterInfo, tempExchangeBuf, tempLen);  	
	
	/* 显示字符串 */
	uint8_t KcalString[] = "HR";
	LetterSource.letterStrAddr = KcalString;
	LetterSource.letterStrLen = strlen("HR");
	LetterSource.letterInfo.letterAxisInfo.axis_X = 0;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 20;
	LetterSource.letterInfo.letterAxisInfo.height = 12;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 47;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 12;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_12_B_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体
	AppScreenTextLayoutGBKDisp(&LetterSource);	
	
	/* 步频显示 */
	// 计算计步值共有几位
	if(Kcal > 99999) Kcal = 99999;
	tempLen = 0;
	TmpNum = Kcal;
	do
	{
		tempBuf[tempLen] = TmpNum % 10;
		TmpNum /= 10;
		tempLen++;
	}while(TmpNum > 0);

	// 交换数组值
	for(uint32_t i = 0; i < tempLen; i++)
	{
		tempExchangeBuf[i] = tempBuf[tempLen-1-i];
	}
	
    LetterSource.letterInfo.letterAxisInfo.axis_X = 49;
    LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
    LetterSource.letterInfo.letterAxisInfo.height = 20;
    LetterSource.letterInfo.letterAxisInfo.length = 47;   
    LetterSource.letterInfo.letterAxisInfo.dispmode = EXRULE;   
	Mid_Screen_FontTgl24AlignDisp(&LetterSource.letterInfo, tempExchangeBuf, tempLen);  	
	
	/* 显示字符串 */
	uint8_t StepFrequencyString[] = "kcal";
	LetterSource.letterStrAddr = StepFrequencyString;
	LetterSource.letterStrLen = strlen("kcal");
	LetterSource.letterInfo.letterAxisInfo.axis_X = 49;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 20;
	LetterSource.letterInfo.letterAxisInfo.height = 12;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 47;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 12;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_12_B_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体
	AppScreenTextLayoutGBKDisp(&LetterSource);	
}

//**********************************************************************
// 函数功能： 显示运动时间
// 输入参数： TimeSec 已运动的时间，单位秒
// 返回参数： 无
//**********************************************************************
void AppCommonSportTimeDisp(uint32_t TimeSec)
{
    uint8       tempBuf[10], tempLen;
	uint8_t TmpHour, TmpMin, TmpSec;
    letter_source_s letterSourceInfo;   
    
    letterSourceInfo.letterInfo.letterAxisInfo.axis_X = 0;
    letterSourceInfo.letterInfo.letterAxisInfo.axis_Y = 0;
    letterSourceInfo.letterInfo.letterAxisInfo.height = 32;
    letterSourceInfo.letterInfo.letterAxisInfo.length = 96;   
    letterSourceInfo.letterInfo.letterAxisInfo.dispmode = EXRULE;   
                 
	if(TimeSec > 359999) TimeSec = 359999;
	
	TmpHour = TimeSec / 3600;
	TmpMin = (TimeSec % 3600) / 60;
	TmpSec = (TimeSec % 3600) % 60;
	
	tempLen        = 8;
	tempBuf[0]     = TmpHour / 10;
	tempBuf[1]     = TmpHour % 10;
	tempBuf[2]     = 10;                        // 冒号
	tempBuf[3]     = TmpMin  / 10;
	tempBuf[4]     = TmpMin  % 10;
	tempBuf[5]     = 10;                        // 冒号
	tempBuf[6]     = TmpSec  / 10;
	tempBuf[7]     = TmpSec  % 10;
	Mid_Screen_FontTgl24AlignDisp(&letterSourceInfo.letterInfo, tempBuf, tempLen);  
}	

//**********************************************************************
// 函数功能： 显示时间和温度
// 输入参数： 
//			rtc_time_s 当前rtc时间
//			time_format_t 显示格式，12小时制或24小时制
//			Temperature 当前温度，有符号数
// 返回参数： 无
//**********************************************************************
void AppCommontimeTemperatureDisp(rtc_time_s RtcTime, time_format_t TimeFormat, int32_t Temperature)
{
    uint8       tempBuf[10], tempLen;
    letter_source_s letterSourceInfo;   
    
	/* 双竖线显示 */
	Mid_Screen_DoubleLine(52);	
	
	/* 时间显示 */
	if(RtcTime.hour > 24) RtcTime.hour = 24;
	if(RtcTime.min > 60) RtcTime.min = 60;
    switch(TimeFormat)
    {
        case TIME_FORMAT_24:         
            tempLen        = 5;
            tempBuf[0]     = RtcTime.hour / 10;
            tempBuf[1]     = RtcTime.hour % 10;
            tempBuf[2]     = 10;                        // 冒号
            tempBuf[3]     = RtcTime.min  / 10;
            tempBuf[4]     = RtcTime.min  % 10;
//            tempBuf[5]     = 10;                        // 冒号
//            tempBuf[6]     = RtcTime.sec  / 10;
//            tempBuf[7]     = RtcTime.sec  % 10;
		
			letterSourceInfo.letterInfo.letterAxisInfo.axis_X = 0;
			letterSourceInfo.letterInfo.letterAxisInfo.axis_Y = 0;
			letterSourceInfo.letterInfo.letterAxisInfo.height = 32;
			letterSourceInfo.letterInfo.letterAxisInfo.length = 52;   
			letterSourceInfo.letterInfo.letterAxisInfo.dispmode = EXRULE;		
            Mid_Screen_FontTgl24AlignDisp(&letterSourceInfo.letterInfo, tempBuf, tempLen);  
            break;
        case TIME_FORMAT_12: 
			if(RtcTime.hour >=12)
			{
				tempBuf[0] = 'P';
				AppScreenLocalAsciiDisp(0, 6, *tempBuf, APP_ASCII_SIZE_12_B_A);
			}
			else
			{
				tempBuf[0] = 'A';
				AppScreenLocalAsciiDisp(0, 6, *tempBuf, APP_ASCII_SIZE_12_B_A);
			}

			if(RtcTime.hour >=12)
				RtcTime.hour -= 12;
		
            tempLen        = 5;
            tempBuf[0]     = RtcTime.hour / 10;
            tempBuf[1]     = RtcTime.hour % 10;
            tempBuf[2]     = 10;                        // 冒号
            tempBuf[3]     = RtcTime.min  / 10;
            tempBuf[4]     = RtcTime.min  % 10;
//            tempBuf[5]     = 10;                        // 冒号
//            tempBuf[6]     = RtcTime.sec  / 10;
//            tempBuf[7]     = RtcTime.sec  % 10;  
			letterSourceInfo.letterInfo.letterAxisInfo.axis_X = 10;
			letterSourceInfo.letterInfo.letterAxisInfo.axis_Y = 0;
			letterSourceInfo.letterInfo.letterAxisInfo.height = 32;
			letterSourceInfo.letterInfo.letterAxisInfo.length = 42;   
			letterSourceInfo.letterInfo.letterAxisInfo.dispmode = EXRULE;				
            Mid_Screen_FontTgl24AlignDisp(&letterSourceInfo.letterInfo, tempBuf, tempLen); 
            break;  
        default:
            break;
    }	
	
	/* 温度显示 */
	if(Temperature >= 0)
	{
		if(Temperature > 99) Temperature = 99;
		
		tempBuf[0] = 16;   	// 占位符
		tempBuf[1] = Temperature / 10;   // 十位
		tempBuf[2] = Temperature % 10;   // 个位
	}
	else
	{
		if(Temperature < -99) Temperature = -99;
		
		tempBuf[0] = 12;   	// 负号
		tempBuf[1] = (~Temperature + 1) / 10;   // 十位
		tempBuf[2] = (~Temperature + 1) % 10;   // 个位	
	}
	tempLen        = 3;
	letterSourceInfo.letterInfo.letterAxisInfo.axis_X = 54;
	letterSourceInfo.letterInfo.letterAxisInfo.axis_Y = 0;
	letterSourceInfo.letterInfo.letterAxisInfo.height = 32;
	letterSourceInfo.letterInfo.letterAxisInfo.length = 28;   
	letterSourceInfo.letterInfo.letterAxisInfo.dispmode = EXRULE;
	Mid_Screen_FontTgl24AlignDisp(&letterSourceInfo.letterInfo, tempBuf, tempLen); 		
	
	tempBuf[0] = 0xA1, tempBuf[1] = 0xE6;		// 显示温度图标 ℃
	AppScreenGBKChineseDisp(84, 6, tempBuf, APP_GB_SIZE_12X12, LETTER_TYPTE_GB);		
}

//**********************************************************************
// 函数功能： 显示保存状态
// 输入参数： 
//			SaveState 保存状态，0已保存 1保存. 2保存.. 3保存... 
//			systerm_languge_t 语言类型
// 返回参数： 无
//**********************************************************************
void AppCommonSaveStateDisp(uint32_t SaveState, systerm_languge_t LanguageType)
{
	letter_source_s LetterSource;
	pic_disp_info_t PicDispInfo;
	uint8_t EnString_0[] = "Saved";	
	uint8_t EnString_1[] = "Saving.";
	uint8_t EnString_2[] = "Saving..";
	uint8_t EnString_3[] = "Saving...";
	
	switch(LanguageType)
	{
		case SYS_ENGLISH_TYPE:
			/* 显示文字 */
			LetterSource.letterInfo.letterAxisInfo.axis_X = 0;
			LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
			LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
			LetterSource.letterInfo.letterAxisInfo.length = 96;	// 显示区域长度
			LetterSource.letterInfo.frontHeight = 16;			// 字符高度
		//	LetterSource.letterInfo.interval = 0;
		//	LetterSource.letterInfo.alic = ALIC_MIDDLE;
		//	LetterSource.letterInfo.language = 0;
			LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
			LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
			LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体	
		
			switch(SaveState)
			{
				case 0:
					LetterSource.letterInfo.alic = ALIC_MIDDLE;
					LetterSource.letterStrAddr = EnString_0;
					LetterSource.letterStrLen = strlen("Saved");			
					AppScreenTextLayoutGBKDisp(&LetterSource);						
					break;
				case 1:
					LetterSource.letterInfo.alic = ALIC_LEFT;
					LetterSource.letterStrAddr = EnString_1;
					LetterSource.letterStrLen = strlen("Saving.");			
					AppScreenTextLayoutGBKDisp(&LetterSource);						
					break;
				case 2:
					LetterSource.letterInfo.alic = ALIC_LEFT;
					LetterSource.letterStrAddr = EnString_2;
					LetterSource.letterStrLen = strlen("Saving..");			
					AppScreenTextLayoutGBKDisp(&LetterSource);						
					break;
				case 3:
					LetterSource.letterInfo.alic = ALIC_LEFT;
					LetterSource.letterStrAddr = EnString_3;
					LetterSource.letterStrLen = strlen("Saving...");			
					AppScreenTextLayoutGBKDisp(&LetterSource);						
					break;
				default: break;
			}		
			break;
		case SYS_CHINESE_TYPE:
		    PicDispInfo.AxisX = 0;
			PicDispInfo.AxisY = 0;
			PicDispInfo.AreaLength = 96;
			PicDispInfo.AreaHeight = 32;
			PicDispInfo.PictureLength = 96;
			PicDispInfo.PictureHeight = 32;
			PicDispInfo.Alic = ALIC_MIDDLE;				
			switch(SaveState)
			{
				case 0:
					PicDispInfo.DotBuf = &gImage_SavedCn[2];
					AppScreenPictureDisp(&PicDispInfo);						
					break;
				case 1:
					PicDispInfo.DotBuf = &gImage_Saving1Cn[2];
					AppScreenPictureDisp(&PicDispInfo);		
					break;
				case 2:
					PicDispInfo.DotBuf = &gImage_Saving2Cn[2];
					AppScreenPictureDisp(&PicDispInfo);		
					break;
				case 3:
					PicDispInfo.DotBuf = &gImage_Saving3Cn[2];
					AppScreenPictureDisp(&PicDispInfo);	
					break;
				default: break;
			}
			break;
		default: 
            /* 显示文字 */
            LetterSource.letterInfo.letterAxisInfo.axis_X = 0;
            LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
            LetterSource.letterInfo.letterAxisInfo.height = 32; // 显示区域高度
            LetterSource.letterInfo.letterAxisInfo.length = 96; // 显示区域长度
            LetterSource.letterInfo.frontHeight = 16;           // 字符高度
        //  LetterSource.letterInfo.interval = 0;
        //  LetterSource.letterInfo.alic = ALIC_MIDDLE;
        //  LetterSource.letterInfo.language = 0;
            LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
            LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;  // 英文字符字体
            LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;       // 中文字符字体   
        
            switch(SaveState)
            {
                case 0:
                    LetterSource.letterInfo.alic = ALIC_MIDDLE;
                    LetterSource.letterStrAddr = EnString_0;
                    LetterSource.letterStrLen = strlen("Saved");            
                    AppScreenTextLayoutGBKDisp(&LetterSource);                      
                    break;
                case 1:
                    LetterSource.letterInfo.alic = ALIC_LEFT;
                    LetterSource.letterStrAddr = EnString_1;
                    LetterSource.letterStrLen = strlen("Saving.");          
                    AppScreenTextLayoutGBKDisp(&LetterSource);                      
                    break;
                case 2:
                    LetterSource.letterInfo.alic = ALIC_LEFT;
                    LetterSource.letterStrAddr = EnString_2;
                    LetterSource.letterStrLen = strlen("Saving..");         
                    AppScreenTextLayoutGBKDisp(&LetterSource);                      
                    break;
                case 3:
                    LetterSource.letterInfo.alic = ALIC_LEFT;
                    LetterSource.letterStrAddr = EnString_3;
                    LetterSource.letterStrLen = strlen("Saving...");            
                    AppScreenTextLayoutGBKDisp(&LetterSource);                      
                    break;
                default: break;
            }
            break;
	}	
	
	
	#if 0
	letter_source_s LetterSource;
	uint8_t EnString_0[] = "Saved";	
	uint8_t EnString_1[] = "Saving.";
	uint8_t EnString_2[] = "Saving..";
	uint8_t EnString_3[] = "Saving...";
	uint8_t CnString_0[] = {		// 已保存
		6,	
		0xD2, 0xD1, 0xB1, 0xA3, 0xB4, 0xE6};		
	uint8_t CnString_1[] = {		// 保存中.
		7,
		0xB1, 0xA3, 0xB4, 0xE6, 0xD6, 0xD0, 0x2E};
	uint8_t CnString_2[] = {		// 保存中..
		8,
		0xB1, 0xA3, 0xB4, 0xE6, 0xD6, 0xD0, 0x2E, 0x2E};
	uint8_t CnString_3[] = {		// 保存中...
		9,
		0xB1, 0xA3, 0xB4, 0xE6, 0xD6, 0xD0, 0x2E, 0x2E, 0x2E};	
	
	/* 显示文字 */
	LetterSource.letterInfo.letterAxisInfo.axis_X = 0;
	LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
	LetterSource.letterInfo.letterAxisInfo.height = 32;	// 显示区域高度
	LetterSource.letterInfo.letterAxisInfo.length = 96;	// 显示区域长度
	LetterSource.letterInfo.frontHeight = 16;			// 字符高度
//	LetterSource.letterInfo.interval = 0;
//	LetterSource.letterInfo.alic = ALIC_MIDDLE;
//	LetterSource.letterInfo.language = 0;
	LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
	LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;	// 英文字符字体
	LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;		// 中文字符字体		
	switch(LanguageType)
	{
		case SYS_ENGLISH_TYPE:
			switch(SaveState)
			{
				case 0:
					LetterSource.letterInfo.alic = ALIC_MIDDLE;
					LetterSource.letterStrAddr = EnString_0;
					LetterSource.letterStrLen = strlen("Saved");			
					AppScreenTextLayoutGBKDisp(&LetterSource);						
					break;
				case 1:
					LetterSource.letterInfo.alic = ALIC_LEFT;
					LetterSource.letterStrAddr = EnString_1;
					LetterSource.letterStrLen = strlen("Saving.");			
					AppScreenTextLayoutGBKDisp(&LetterSource);						
					break;
				case 2:
					LetterSource.letterInfo.alic = ALIC_LEFT;
					LetterSource.letterStrAddr = EnString_2;
					LetterSource.letterStrLen = strlen("Saving..");			
					AppScreenTextLayoutGBKDisp(&LetterSource);						
					break;
				case 3:
					LetterSource.letterInfo.alic = ALIC_LEFT;
					LetterSource.letterStrAddr = EnString_3;
					LetterSource.letterStrLen = strlen("Saving...");			
					AppScreenTextLayoutGBKDisp(&LetterSource);						
					break;
				default: break;
			}		
			break;
		case SYS_CHINESE_TYPE:
			switch(SaveState)
			{
				case 0:
					LetterSource.letterInfo.alic = ALIC_MIDDLE;
					LetterSource.letterStrAddr = &CnString_0[1];
					LetterSource.letterStrLen = CnString_0[0];		
					AppScreenTextLayoutGBKDisp(&LetterSource);						
					break;
				case 1:
					LetterSource.letterInfo.alic = ALIC_LEFT;
					LetterSource.letterStrAddr = &CnString_1[1];
					LetterSource.letterStrLen = CnString_1[0];		
					AppScreenTextLayoutGBKDisp(&LetterSource);	
					break;
				case 2:
					LetterSource.letterInfo.alic = ALIC_LEFT;
					LetterSource.letterStrAddr = &CnString_2[1];
					LetterSource.letterStrLen = CnString_2[0];		
					AppScreenTextLayoutGBKDisp(&LetterSource);	
					break;
				case 3:
					LetterSource.letterInfo.alic = ALIC_LEFT;
					LetterSource.letterStrAddr = &CnString_3[1];
					LetterSource.letterStrLen = CnString_3[0];		
					AppScreenTextLayoutGBKDisp(&LetterSource);	
					break;
				default: break;
			}
			break;
		default: break;
	}		
	#endif
}

//**********************************************************************
// 函数功能：  仓储状态下的电池图标
// 输入参数：  无 
// 返回参数： 无
//**********************************************************************
void App_StoreBatLow(void)
{
    letter_info_s  leterInfoTemp;
    leterInfoTemp.letterAxisInfo.axis_X = 30;
    leterInfoTemp.letterAxisInfo.axis_Y = 10;
    leterInfoTemp.letterAxisInfo.height = NULL;
    leterInfoTemp.letterAxisInfo.length = NULL;
    leterInfoTemp.letterAxisInfo.dispmode = INCRULE;

    BatDisp(&leterInfoTemp, BAT_LEVER_0);
}

//**********************************************************************
// 函数功能：  充电状态下的图标显示
// 输入参数：  无 
// 返回参数： 无
//**********************************************************************
void App_ChargeTittle(uint8 soc,uint8 batVol)
{
    letter_info_s  leterInfoTemp;
    letter_source_s LetterSource;
    uint8           tmpNum;
    uint8       tempBuf[10], tempExchangeBuf[10], tempLen;

    leterInfoTemp.letterAxisInfo.axis_X = 10;
    leterInfoTemp.letterAxisInfo.axis_Y = 10;
    leterInfoTemp.letterAxisInfo.height = NULL;
    leterInfoTemp.letterAxisInfo.length = NULL;
    leterInfoTemp.letterAxisInfo.dispmode = INCRULE;

    BatDisp(&leterInfoTemp, batVol);


    /* 显示完成百分比 */ 
    // 计算百分比值共有几位
    tempLen = 0;
    tmpNum = soc;
    do
    {
        tempBuf[tempLen] = tmpNum % 10;
        tmpNum /= 10;
        tempLen++;
    }while(tmpNum > 0);
    
    // 交换数组值，并转换为ASCII码
    for(uint8_t i = 0; i < tempLen; i++)
    {
        tempExchangeBuf[i] = tempBuf[tempLen-1-i] + '0';    
    }
    
    tempExchangeBuf[tempLen] = '%'; // 百分比符号％
    tempLen++;
    
    // 显示百分比值.
    LetterSource.letterStrAddr = tempExchangeBuf;
    LetterSource.letterStrLen = tempLen;
    LetterSource.letterInfo.letterAxisInfo.axis_X = 40;
    LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
    LetterSource.letterInfo.letterAxisInfo.height = 32; // 显示区域高度
    LetterSource.letterInfo.letterAxisInfo.length = 56; // 显示区域长度
    LetterSource.letterInfo.frontHeight = 16;           // 字符高度
//  LetterSource.letterInfo.interval = 0;
    LetterSource.letterInfo.alic = ALIC_MIDDLE;
//  LetterSource.letterInfo.language = 0;
    LetterSource.letterInfo.letterType = LETTER_TYPTE_GB;
    LetterSource.letterInfo.asciiLetterSize = APP_ASCII_SIZE_16_A;    // 英文字符字体
    LetterSource.letterInfo.gbLetterSize = APP_GB_SIZE_16X16;       // 中文字符字体
    
    AppScreenTextLayoutGBKDisp(&LetterSource);

}


//**********************************************************************
// 函数功能：  仓储状态下的仓储标题显示
// 输入参数：  语言类型 
// 返回参数： 无
//**********************************************************************
void App_StoreTittle(systerm_languge_t lantype)
{
    pic_info_s  imageInfo;

    imageInfo.axis_X = 0;
    imageInfo.axis_Y = 0;
    imageInfo.height = 32;
    imageInfo.length = 96;
    imageInfo.dispmode = INCRULE;

    switch(lantype)
    {
        case SYS_CHINESE_TYPE:
        Mid_Screen_UpdataToPicRam(&imageInfo, gImage_storetile);
        break;

        case SYS_ENGLISH_TYPE:
        Mid_Screen_UpdataToPicRam(&imageInfo, gImage_storetile);
        break;

        default:
        Mid_Screen_UpdataToPicRam(&imageInfo, gImage_storetile);
        break;
    }   
}

//**********************************************************************
// 函数功能：  仓储状态下的smart标题显示
// 输入参数：  语言类型 
// 返回参数： 无
//**********************************************************************
void App_PowerUpTittle(systerm_languge_t lantype)
{
    pic_info_s  imageInfo;

    imageInfo.axis_X = 0;
    imageInfo.axis_Y = 0;
    imageInfo.height = NULL;
    imageInfo.length = NULL;
    imageInfo.dispmode = INCRULE;

    switch(lantype)
    {
        case SYS_CHINESE_TYPE:
        Mid_Screen_UpdataToPicRam(&imageInfo, gImage_lenovotitle);
        break;

        case SYS_ENGLISH_TYPE:
        Mid_Screen_UpdataToPicRam(&imageInfo, gImage_lenovotitle);
        break;

        default:
        Mid_Screen_UpdataToPicRam(&imageInfo, gImage_lenovotitle);
        break;
    }   
}

//**********************************************************************
// 函数功能：  仓储状态下的升级标题显示
// 输入参数：  语言类型 
// 返回参数： 无
//**********************************************************************
void App_OtaTittle(systerm_languge_t lantype,uint8 level)
{
    pic_info_s  imageInfo;

    imageInfo.axis_X = 0;
    imageInfo.axis_Y = 0;
    imageInfo.height = NULL;
    imageInfo.length = NULL;
    imageInfo.dispmode = INCRULE;

    switch(lantype)
    {
        case SYS_CHINESE_TYPE:
        switch(level)
        {
            case 0:
            Mid_Screen_UpdataToPicRam(&imageInfo, gImage_otatitleCh0);
            break;

            case 1:
            Mid_Screen_UpdataToPicRam(&imageInfo, gImage_otatitleCh1);
            break;

            case 2:
            Mid_Screen_UpdataToPicRam(&imageInfo, gImage_otatitleCh2);
            break;

            case 3:
            Mid_Screen_UpdataToPicRam(&imageInfo, gImage_otatitleCh3);
            break;

            default:
            Mid_Screen_UpdataToPicRam(&imageInfo, gImage_otatitleCh3);
            break;
        }
        
        break;

        case SYS_ENGLISH_TYPE:
        switch(level)
        {
            case 0:
            Mid_Screen_UpdataToPicRam(&imageInfo, gImage_otatitleEh0);
            break;

            case 1:
            Mid_Screen_UpdataToPicRam(&imageInfo, gImage_otatitleEh1);
            break;

            case 2:
            Mid_Screen_UpdataToPicRam(&imageInfo, gImage_otatitleEh2);
            break;

            case 3:
            Mid_Screen_UpdataToPicRam(&imageInfo, gImage_otatitleEh3);
            break;

            default:
            Mid_Screen_UpdataToPicRam(&imageInfo, gImage_otatitleEh3);
            break;
        }
        break;

        default:
        Mid_Screen_UpdataToPicRam(&imageInfo, gImage_otatitleEh3);
        break;
    }   
}

//**********************************************************************
// 函数功能： 拍照图标显示
// 输入参数： 无
// 返回参数： 无
//**********************************************************************
void App_TakepictureDisp(void)
{
    pic_info_s  imageInfo;

    imageInfo.axis_X = 0;
    imageInfo.axis_Y = 0;
    imageInfo.height = NULL;
    imageInfo.length = NULL;
    imageInfo.dispmode = INCRULE;

    Mid_Screen_UpdataToPicRam(&imageInfo, gImage_takepicture);
}

//**********************************************************************
// 函数功能： 配对码显示
// 输入参数： 无
// 返回参数： 无
//**********************************************************************
void App_PairCodeDisp(uint32 paireCode)
{
    letter_source_s LetterSource;
    uint8       tempBuf[10], tempExchangeBuf[10], tempLen;
    uint32       TmpNum;
    
    /* 显示计步数量 */
    // 计算计步值共有几位
    tempLen = 0;
    TmpNum = paireCode;
    do
    {
        tempBuf[tempLen] = TmpNum % 10;
        TmpNum /= 10;
        tempLen++;
    }while(TmpNum > 0);

    // 交换数组值
    for(uint8 i = 0; i < tempLen; i++)
    {
        tempExchangeBuf[i] = tempBuf[tempLen-1-i];
    }

    LetterSource.letterInfo.letterAxisInfo.axis_X = 0;
    LetterSource.letterInfo.letterAxisInfo.axis_Y = 0;
    LetterSource.letterInfo.letterAxisInfo.height = 32;
    LetterSource.letterInfo.letterAxisInfo.length = 96;
    LetterSource.letterInfo.letterAxisInfo.dispmode = EXRULE;
    Mid_Screen_FontTgl24AlignDisp(&LetterSource.letterInfo, tempExchangeBuf, tempLen);
}


// 打印ASCII点阵信息
void App_PrintfAsciiDotInfo(uint8_t* CodeValue, letter_size_t LetterSize)
{
    uint8 frontStreamBuf[255] = {0};	// 字体点阵存储buf
    flash_task_msg_t flashMsg;			// 传入的FLASH参数
    front_size_t frontsizeinfo;			// 从FLASH读出的字体参数

	flashMsg.id                                      = FRONT_ID;
	flashMsg.flash.frontEvent.id                     = FRONT_EVENT_READ_UNICODE; 
	flashMsg.flash.frontEvent.para.dataAddr          = frontStreamBuf;
	flashMsg.flash.frontEvent.para.sizeKind          = LetterSize;
	flashMsg.flash.frontEvent.para.code.codeUnicode = (uint16)(CodeValue[1] << 8 | CodeValue[0]);
	flashMsg.flash.frontEvent.para.wordSize          = &frontsizeinfo;
	FlashTask_EventSet(&flashMsg);						

	switch (LetterSize)
    {
    	case APP_ASCII_SIZE_16_A:
			SEGGER_RTT_printf(0,"const unsigned char AsciiInfo16A_%d[] = { // %c\r\n",CodeValue[0], CodeValue[0]);
			SEGGER_RTT_printf(0,"0x%02X, 0x%02X, 0x%02X, 0x%02X, // WordWidth, WordHeigh, ValidWidth, DotByteLen\r\n",frontsizeinfo.wordWidth,frontsizeinfo.wordHeight,frontsizeinfo.validWidth,frontsizeinfo.dataStringLengh);
			for(uint32_t i = 0;i < frontsizeinfo.dataStringLengh;i++)
			{
				if((0 == i%16) && (i != 0))
					SEGGER_RTT_printf(0,"\r\n");		
				SEGGER_RTT_printf(0,"0x%02X, ",frontStreamBuf[i]);
			}	
			SEGGER_RTT_printf(0,"};\r\n");			
    		break;
    	case APP_ASCII_SIZE_8X16:
			SEGGER_RTT_printf(0,"const unsigned char AsciiInfo8x16_%d[] = { // %c\r\n",CodeValue[0], CodeValue[0]);
			SEGGER_RTT_printf(0,"0x%02X, 0x%02X, 0x%02X, 0x%02X, // WordWidth, WordHeigh, ValidWidth, DotByteLen\r\n",frontsizeinfo.wordWidth,frontsizeinfo.wordHeight,frontsizeinfo.validWidth,frontsizeinfo.dataStringLengh);
			for(uint32_t i = 0;i < frontsizeinfo.dataStringLengh;i++)
			{
				SEGGER_RTT_printf(0,"0x%02X, ",frontStreamBuf[i]);	
			}	
			SEGGER_RTT_printf(0,"};\r\n");			
    		break;
		case APP_ASCII_SIZE_12_B_A:
			SEGGER_RTT_printf(0,"const unsigned char AsciiInfo12BA_%d[] = { // %c\r\n",CodeValue[0], CodeValue[0]);
			SEGGER_RTT_printf(0,"0x%02X, 0x%02X, 0x%02X, 0x%02X, // WordWidth, WordHeigh, ValidWidth, DotByteLen\r\n",frontsizeinfo.wordWidth,frontsizeinfo.wordHeight,frontsizeinfo.validWidth,frontsizeinfo.dataStringLengh);
			for(uint32_t i = 0;i < frontsizeinfo.dataStringLengh;i++)
			{
				SEGGER_RTT_printf(0,"0x%02X, ",frontStreamBuf[i]);	
			}	
			SEGGER_RTT_printf(0,"};\r\n");				
    	default:
    		break;
    }
}

// 打印当前UI界面，
void App_PrintfCurrUIInfo(void)
{
	Mid_Screen_PrintDotInfo();
}





