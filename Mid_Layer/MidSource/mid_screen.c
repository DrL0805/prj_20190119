#include "platform_common.h"
/* FreeRTOS includes */
#include "rtos.h"

#include "drv_screen.h"
#include "mid_screen.h"



//图片缓存
uint8 midPicRam[OLEDRAMLENGTH];

#define     ELIDESIGNLENGTH                 10          //the "..."length


static uint16               oledInitFlag    = 0;
static SemaphoreHandle_t    oledMutex;


// *******************************************************************************
// *Funtion name: InverseDispRam()
// *Description : Inverse the area of display ram
// *
// *Input: imageInfo: the pictrue imformation
// *Output:None
// /******************************************************************************
void Mid_Screen_InversePicRam(pic_info_s *imageInfo)
{
    uint8 AreaStartAddr, AreaStartPage, AreaHigh, AreaLenth;
    uint8 yaxisoffet1, yaxisoffetbit1, yaxisoffetbit2;
    uint8 heightoffset1, heightoffset2;
    uint16 i, j, picturerow;
    uint16 rambasicaddr;

    AreaStartAddr   = imageInfo->axis_X;
    AreaStartPage   = imageInfo->axis_Y / 8;

    AreaHigh        = imageInfo->height;
    AreaLenth       = imageInfo->length;
    picturerow      = AreaHigh / 8;
    if (AreaHigh % 8) 
    {
        picturerow++;
    }
    yaxisoffetbit1  = (uint8)(imageInfo->axis_Y % 8);
    yaxisoffetbit2  = 8 - yaxisoffetbit1;

    yaxisoffet1     = (uint8)(0xff << yaxisoffetbit1);

    heightoffset1   = (uint8)(0xff << (AreaHigh % 8));
    heightoffset2   = 0xff ^ heightoffset1;

    for (i = 0; i < picturerow; i++) 
    {
        rambasicaddr = (i + AreaStartPage) * ROWLENGTH + AreaStartAddr;
        if (rambasicaddr > OLEDRAMLENGTH - 1) 
        {
            break;
        }

        for (j = 0; j < AreaLenth; j++) 
        {
            if (AreaHigh > 7) 
            {
                midPicRam[rambasicaddr + j] ^= (uint8)(0xFF << yaxisoffetbit1);           //move the low bit to height bit

                if ((yaxisoffet1 != 0xFF) && (rambasicaddr < OLEDRAMLENGTH - ROWLENGTH)) 
                {
                    midPicRam[rambasicaddr + j + ROWLENGTH] ^= (uint8)(0xFF >> (yaxisoffetbit2));  //move the height bit to low bit
                }
            } 
            else 
            {
                midPicRam[rambasicaddr + j] ^= (uint8)((0xFF & heightoffset2) << yaxisoffetbit1);      //move the low bit to height bit

                if ((yaxisoffet1 != 0xFF) && (rambasicaddr < OLEDRAMLENGTH - ROWLENGTH)) 
                {
                    midPicRam[rambasicaddr + j + ROWLENGTH] ^= (uint8)((0xFF & heightoffset2) >> (yaxisoffetbit2)); //move the height bit to low bit
                }
            }
        }
        AreaHigh -= 8;
    }

}
// *******************************************************************************
// *Funtion name: InitAreaDispRam()
// *Description : updata the display ram
// *
// *Input: imageInfo: the pictrue imformation
// *       imagedata: picture image src point
// *Output:None
// /******************************************************************************
void Mid_Screen_InitAreaPicRam(pic_info_s  *imageInfo, uint8 initdata)
{
    uint8 AreaStartAddr, AreaStartPage, AreaHigh, AreaLenth;
    uint8 yaxisoffet1, yaxisoffet2, yaxisoffetbit1, yaxisoffetbit2;
    uint8 heightoffset1, heightoffset2;
    uint16 i, j, picturerow;
    uint16 rambasicaddr, PicStaAddr;

    AreaStartAddr   = imageInfo->axis_X;
    AreaStartPage   = imageInfo->axis_Y / 8;
    AreaHigh        = imageInfo->height;
    AreaLenth       = imageInfo->length;

    picturerow      = AreaHigh / 8;
    if (AreaHigh % 8) 
    {
        picturerow++;
    }

    yaxisoffetbit1      = (uint8)(imageInfo->axis_Y % 8);
    yaxisoffetbit2      = 8 - yaxisoffetbit1;

    yaxisoffet1         = (uint8)(0xff << yaxisoffetbit1);
    yaxisoffet2         = 0xff ^ yaxisoffet1;

    heightoffset1       = (uint8)(0xff << (AreaHigh % 8));
    heightoffset2       = 0xff ^ heightoffset1;

    for (i = 0; i < picturerow; i++) 
    {
        PicStaAddr = i * AreaLenth;
        if (imageInfo->dispmode == INCRULE) 
        {
            PicStaAddr += 2;
        }
        rambasicaddr = (i + AreaStartPage) * ROWLENGTH + AreaStartAddr;
        if (rambasicaddr > OLEDRAMLENGTH - 1) 
        {
            break;
        }


        for (j = 0; j < AreaLenth; j++) 
        {
            if (AreaHigh > 7) 
            {
                midPicRam[rambasicaddr + j] |= 
                    (uint8)(initdata << yaxisoffetbit1);     //move the low bit to height bit
                midPicRam[rambasicaddr + j] &= 
                    (uint8)(initdata << yaxisoffetbit1) | yaxisoffet2; //move the low bit to height bit

                if (yaxisoffet1 != 0xff && (rambasicaddr < OLEDRAMLENGTH - ROWLENGTH)) 
                {
                    midPicRam[rambasicaddr + j + ROWLENGTH] |= 
                        (uint8)(initdata >> (yaxisoffetbit2)); //move the height bit to low bit
                    midPicRam[rambasicaddr + j + ROWLENGTH] &= 
                        (uint8)(initdata >> (yaxisoffetbit2)) | yaxisoffet1;     //move the height bit to low bit
                }
            } 
            else 
            {
                midPicRam[rambasicaddr + j] |=
                    (uint8)((initdata & heightoffset2) << yaxisoffetbit1);          //move the low bit to height bit
                midPicRam[rambasicaddr + j] &=
                    (uint8)((initdata | heightoffset1) << yaxisoffetbit1) | yaxisoffet2; //move the low bit to height bit

                if (yaxisoffet1 != 0xff && (rambasicaddr < OLEDRAMLENGTH - ROWLENGTH)) 
                {
                    midPicRam[rambasicaddr + j + ROWLENGTH] |= 
                        (uint8)((initdata & heightoffset2) >> (yaxisoffetbit2)); //move the height bit to low bit
                    midPicRam[rambasicaddr + j + ROWLENGTH] &=
                        (uint8)((initdata | heightoffset1) >> (yaxisoffetbit2)) | yaxisoffet1;           //move the height bit to low bit
                }
            }
        }
        AreaHigh -= 8;
    }
}

// *******************************************************************************
// *Funtion name: UpdataToDispRam()
// *Description : updata the display ram
// *
// *Input: imageInfo: the pictrue imformation
// *       imagedata: picture image src point
// *Output:None
// /******************************************************************************
void Mid_Screen_UpdataToPicRam(pic_info_s  *imageInfo, const uint8 *imagedata)
{
    uint8 AreaStartAddr, AreaStartPage, AreaHigh, AreaLenth;
    uint8 yaxisoffet1, yaxisoffet2, yaxisoffetbit1, yaxisoffetbit2;
    uint8 heightoffset1, heightoffset2;
    uint16 i, j, picturerow;
    uint16 rambasicaddr, PicStaAddr;


    AreaStartAddr       = imageInfo->axis_X;
    AreaStartPage       = imageInfo->axis_Y / 8;
    if (imageInfo->dispmode == EXRULE) 
    {
        AreaHigh        = imageInfo->height;
        AreaLenth       = imageInfo->length;
    } 
    else 
    {
        AreaHigh        = imagedata[1];
        AreaLenth       = imagedata[0];
    }

    picturerow          = AreaHigh / 8;
    if (AreaHigh % 8) 
    {
        picturerow++;
    }

    yaxisoffetbit1      = (uint8)(imageInfo->axis_Y % 8);
    yaxisoffetbit2      = 8 - yaxisoffetbit1;

    yaxisoffet1         = (uint8)(0xff << yaxisoffetbit1);
    yaxisoffet2         = 0xff ^ yaxisoffet1;

    heightoffset1       = (uint8)(0xff << (AreaHigh % 8));
    heightoffset2       = 0xff ^ heightoffset1;

    for (i = 0; i < picturerow; i++) 
    {
        PicStaAddr = i * AreaLenth;
        if (imageInfo->dispmode == INCRULE) 
        {
            PicStaAddr += 2;
        }
        rambasicaddr = (i + AreaStartPage) * ROWLENGTH + AreaStartAddr;
        if (rambasicaddr > OLEDRAMLENGTH - 1) 
        {
            break;
        }

        for (j = 0; j < AreaLenth; j++) 
        {
            if (AreaHigh > 7) 
            {
                midPicRam[rambasicaddr + j] |=
                    (uint8)(imagedata[PicStaAddr + j] << yaxisoffetbit1);     //move the low bit to height bit
                midPicRam[rambasicaddr + j] &=
                    (uint8)(imagedata[PicStaAddr + j] << yaxisoffetbit1) |
                    yaxisoffet2; //move the low bit to height bit

                if ((yaxisoffet1 != 0xff) && (rambasicaddr < OLEDRAMLENGTH - ROWLENGTH)) 
                {
                    midPicRam[rambasicaddr + j + ROWLENGTH] |=
                        (uint8)(imagedata[PicStaAddr + j] >> (yaxisoffetbit2)); //move the height bit to low bit
                    midPicRam[rambasicaddr + j + ROWLENGTH] &=
                        (uint8)(imagedata[PicStaAddr + j] >> (yaxisoffetbit2)) | yaxisoffet1;     //move the height bit to low bit
                }
            } 
            else 
            {
                midPicRam[rambasicaddr + j] |=
                    (uint8)((imagedata[PicStaAddr + j] & heightoffset2) << yaxisoffetbit1);          //move the low bit to height bit
                midPicRam[rambasicaddr + j] &=
                    (uint8)((imagedata[PicStaAddr + j] | heightoffset1) << yaxisoffetbit1) | yaxisoffet2; //move the low bit to height bit

                if ((yaxisoffet1 != 0xff) && (rambasicaddr < OLEDRAMLENGTH - ROWLENGTH)) 
                {
                    midPicRam[rambasicaddr + j + ROWLENGTH] |=
                        (uint8)((imagedata[PicStaAddr + j] & heightoffset2) >> (yaxisoffetbit2)); //move the height bit to low bit
                    midPicRam[rambasicaddr + j + ROWLENGTH] &=
                        (uint8)((imagedata[PicStaAddr + j] | heightoffset1) >> (yaxisoffetbit2)) | yaxisoffet1;   //move the height bit to low bit
                }
            }
        }
        AreaHigh -= 8;
    }
}

// *******************************************************************************
// *Funtion name: InitDispRam()
// *Description :init the dispram data
// *
// *Input: the init data
// *Output:None
// /******************************************************************************
void Mid_Screen_InitPicRam(uint8 data)
{
    uint16 i;
	
    for (i = 0; i < OLEDRAMLENGTH; i++)
        midPicRam[i] = data;
}

void Mid_Screen_PicRamTest(uint8_t TestValue)
{
    uint16 i;
	
	
    for (i = 0; i < 384; i++)
        midPicRam[i] = TestValue;	
}

// *******************************************************************************
// 函数功能:    把图像缓存指定区域的数据更新至显示缓存相应区域
// 输入参数：   
// sourceZoneInfo：  显示区域信息：x,y坐标，宽、高
// 返回参数:    无
// /******************************************************************************
void Mid_Screen_UpdataAreaToDispRam(pic_info_s *sourceZoneInfo)
{
    Drv_Screen_PicRamToDispRam(sourceZoneInfo, midPicRam);
}

// *******************************************************************************
// 函数功能:    把图像缓存数据滚动入到显示缓存中，同时显示缓存中的数据滚动移出
//              把midPicRam[]里的像素点，根据要求移动到bspOledDispRam[]中				
// 输入参数：   
// rounDiret:  　滚动方向
//　roundPixel： 滚动像素数
// 返回参数:    无
// /******************************************************************************

/*
	获取某个像素点的值，0或者1，
	约定像素点坐标如下：
	[0000][0001]···[0095]
	[0096][0097]···[0191]
	···
	[2976][2977]···[3071]
	*/
uint8_t Mid_Screen_GetPixelValue(uint8_t* RamBuf, uint32_t PixelPos, uint32_t Format)
{
	uint32_t TmpPixelValue, i, j, k, m;
//	uint8_t Tmp1,Tmp2,Tmp3,Tmp4;
	uint8_t Tmp4;
	
	switch(Format)
	{
		case 0:			// 从原始的点阵数组中获取像素点值
			i = (PixelPos / 768);	// 在第几大行，0,1,2,3共4大行，每行有96个字节，96*8=768个像素点
			j = (PixelPos % 768);		// 在每大行的第几个像素点
			k = j % 96;					// 在每大行的第几个字节，序号从0开始
			m = j / 96;					// 在每个字节的第几位，序号从0开始
//			Tmp1 = i*96 + k;
//			Tmp2 = RamBuf[i*96 + k];
//			Tmp3 = (RamBuf[i*96 + k] >> m);
			Tmp4 = (RamBuf[i*96 + k] >> m) & 0x01;
			TmpPixelValue = Tmp4;
//			TmpPixelValue = (RamBuf[i*96 + k] >> m) & 0x01;
			break;
		case 1:			// 从"横向"转换后的点阵数组中获取像素点值
			i = (PixelPos / 96);		// 第几大行，共32行，序号从0开始，每行12个字节，
			j = (PixelPos % 96);		// 每行的第几个像素点，
			k = j / 8; 		// 每行第几个字节
			m = j % 8;		// 每个字节第几位
			TmpPixelValue = (RamBuf[i*12 + k] >> m) & 0x01;
			break;
		default:
			break;
	}
	
	return TmpPixelValue;
}

/* 往某个像素点设置值 */
void Mid_Screen_SetPixelValue(uint8_t* RamBuf, uint32_t PixelPos, uint32_t Format, uint8_t PixelValue)
{
	uint32_t i, j, k, m;
	
	switch(Format)
	{
		case 0:			// 向原始的点阵数组中写入像素点值
			i = (PixelPos / 768);	// 在第几大行，0,1,2,3共4大行，每行有96个字节，96*8=768个像素点
			j = (PixelPos % 768);		// 在每大行的第几个像素点
			k = j % 96;					// 在每大行的第几个字节，序号从0开始
			m = j / 96;					// 在每个字节的第几位，序号从0开始
			RamBuf[i*96 + k] &= (~(0x01 << m));
			RamBuf[i*96 + k] |= (PixelValue << m);			
			break;
		case 1:			// 向"横向"转换后的点阵数组中写入像素点值
			i = (PixelPos / 96);		// 第几大行，共32行，序号从0开始，每行12个字节，
			j = (PixelPos % 96);		// 每行的第几个像素点，
			k = j / 8; 		// 每行第几个字节
			m = j % 8;		// 每个字节第几位
			RamBuf[i*12 + k] &= (~(0x01 << m));
			RamBuf[i*12 + k] |= PixelValue << m;
			break;
		default:
			break;
	}	
}

void Mid_Screen_RoundPicRamToDisRam(bool FirstMoveFlg, uint8 rounDiret, uint8 roundPixel)
{
    uint8 *dispRam;
	static uint8_t OldRamBuf[OLEDRAMLENGTH];
	uint8_t TmpDispMidRam[OLEDRAMLENGTH];
	uint8_t TmpDispRam[OLEDRAMLENGTH];
	uint32_t i;

    dispRam = Drv_Screen_GetDispRam();
	
    switch(rounDiret)
    {
        case ROUND_UP:
			/*	把OldRamBuf[]点阵信息"横着"按字节存储在TmpDispRam[]中，如下图
				| |···| 		[0  ]···[95 ]        			
				| |···| 		[96 ]···[191]       		
				| |···| 		[192]···[287]           
				| |···| 		[288]···[383] 
				按bit转为：
				—— ——···—— 		[0  ][1  ]···[11 ]
				—— ——···—— 		[12 ][13 ]···[23 ]
				···				···
				—— ——···—— 		[372][373]···[383]		
			*/
			/* 如果是首次调用滚屏显示函数，需要把原有的显存保存起来 */ 
			if(FirstMoveFlg)
			{
				for(uint32_t TmpPos = 0;TmpPos < OLED_PIXEL_NUM;TmpPos++)
				{
					Mid_Screen_SetPixelValue(OldRamBuf, TmpPos, 1, Mid_Screen_GetPixelValue(dispRam, TmpPos, 0));
				}								
			}		
		
//			for(uint32_t TmpPos = 0;TmpPos < 3072;TmpPos++)
//			{
//				Mid_Screen_SetPixelValue(TmpDispOldRam, TmpPos, 1, Mid_Screen_GetPixelValue(OldRamBuf, TmpPos, 0));
//			}
			
			for(uint32_t TmpPos = 0;TmpPos < OLED_PIXEL_NUM;TmpPos++)
			{
				Mid_Screen_SetPixelValue(TmpDispMidRam, TmpPos, 1, Mid_Screen_GetPixelValue(midPicRam, TmpPos, 0));
			}			
			
			/* 根据参数要求向上/下移动TmpDispRam[]点阵 */ 
			// 原有点阵向上移动
			memcpy(TmpDispRam, &OldRamBuf[roundPixel*OLED_ROW_BYTE], (LISTLENGTH - roundPixel)*OLED_ROW_BYTE);
			
			// 新的点阵填补空缺
			memcpy(&TmpDispRam[(LISTLENGTH - roundPixel)*OLED_ROW_BYTE], TmpDispMidRam, roundPixel*OLED_ROW_BYTE);			

			/* 把移动完成的TmpDispRam[]点阵信息逆向竖着存入dispRam[]中 */ 
			for(uint32_t TmpPos = 0;TmpPos < OLED_PIXEL_NUM;TmpPos++)
			{
				Mid_Screen_SetPixelValue(dispRam, TmpPos, 0, Mid_Screen_GetPixelValue(TmpDispRam, TmpPos, 1));
			}						
			break;
        case ROUND_DOWN:
			/* 把OldRamBuf[]点阵信息"横着"按字节存储在TmpDispRam[]中 */
			if(FirstMoveFlg)
			{
				for(uint32_t TmpPos = 0;TmpPos < OLED_PIXEL_NUM;TmpPos++)
				{
					Mid_Screen_SetPixelValue(OldRamBuf, TmpPos, 1, Mid_Screen_GetPixelValue(dispRam, TmpPos, 0));
				}			
			}

			for(uint32_t TmpPos = 0;TmpPos < OLED_PIXEL_NUM;TmpPos++)
			{
				Mid_Screen_SetPixelValue(TmpDispMidRam, TmpPos, 1, Mid_Screen_GetPixelValue(midPicRam, TmpPos, 0));
			}			
			
			/* 根据参数要求向上/下移动TmpDispRam[]点阵 */ 
			// 原有点阵向下移动
			memcpy(&TmpDispRam[roundPixel*OLED_ROW_BYTE], OldRamBuf, (LISTLENGTH - roundPixel)*OLED_ROW_BYTE);
			
			// 新的点阵填补空缺
			memcpy(TmpDispRam, &TmpDispMidRam[(LISTLENGTH - roundPixel)*OLED_ROW_BYTE], roundPixel*OLED_ROW_BYTE);			

			/* 把移动完成的TmpDispRam[]点阵信息逆向竖着存入dispRam[]中 */ 
			for(uint32_t TmpPos = 0;TmpPos < OLED_PIXEL_NUM;TmpPos++)
			{
				Mid_Screen_SetPixelValue(dispRam, TmpPos, 0, Mid_Screen_GetPixelValue(TmpDispRam, TmpPos, 1));
			}			
			break;
        case ROUND_RIGHT:
			/* 如果是首次调用滚屏显示函数，需要把原有的显存保存起来 */ 
			if(FirstMoveFlg)
			{
				memcpy(OldRamBuf, dispRam, OLEDRAMLENGTH);
			}			
		
			// 把dispRam[]的像素点，整体向右移动roundPixel个点阵
			for(i = 0;i < 4;i++)
			{
				memcpy(&dispRam[i*ROWLENGTH+roundPixel], &OldRamBuf[i*ROWLENGTH], ROWLENGTH - roundPixel);
			}
			
			// 把midPicRam[]的像素点，向右移动填补dispRam[]里的空缺
			for(i = 0;i < 4;i++)
			{
				memcpy(&dispRam[i*ROWLENGTH], &midPicRam[i*ROWLENGTH + (ROWLENGTH-roundPixel)], roundPixel);
			}			
			break;
        case ROUND_LEFT:
			/* 如果是首次调用滚屏显示函数，需要把原有的显存保存起来 */ 
			if(FirstMoveFlg)
			{
				memcpy(OldRamBuf, dispRam, OLEDRAMLENGTH);
			}			
		
			// 把dispRam[]的像素点，整体向左移动roundPixel个点阵
			for(i = 0;i < 4;i++)
			{
				memcpy(&dispRam[i*ROWLENGTH], &OldRamBuf[i*ROWLENGTH+roundPixel], ROWLENGTH - roundPixel);
			}
			
			// 把midPicRam[]的像素点，向左移动填补dispRam[]里的空缺
			for(i = 0;i < 4;i++)
			{
				memcpy(&dispRam[i*ROWLENGTH + (ROWLENGTH-roundPixel)], &midPicRam[i*ROWLENGTH], roundPixel);
			}
			break;
		default:
			break;
    }
}
    

//**********************************************************************
// 函数功能:    使能OLED接口，并初始化队列
// 输入参数：   
// 返回参数:    无
void Mid_Screen_Enable(void)
{
    if(oledInitFlag == 0)
    {
        oledMutex   = xSemaphoreCreateMutex();
		oledInitFlag = 1;
    }
    xSemaphoreTake(oledMutex, portMAX_DELAY);
    Drv_Screen_Open();
    xSemaphoreGive(oledMutex);
}

void Mid_Screen_Disable(void)
{
	xSemaphoreTake(oledMutex, portMAX_DELAY);
    Drv_Screen_Close();
	xSemaphoreGive(oledMutex);
	
}

void Mid_Screen_SoftInit(void)
{
    xSemaphoreTake(oledMutex, portMAX_DELAY);
    Drv_Screen_SoftInit(1);
    xSemaphoreGive(oledMutex);
}

void Mid_Screen_LightOn(void)
{
    xSemaphoreTake(oledMutex, portMAX_DELAY);
    Drv_Screen_LightOn();
    xSemaphoreGive(oledMutex);
}

void Mid_Screen_LightOff(void)
{
    xSemaphoreTake(oledMutex, portMAX_DELAY);
    Drv_Screen_LightOff();
    xSemaphoreGive(oledMutex);	
}

void Mid_Screen_DispAll(void)
{
    xSemaphoreTake(oledMutex, portMAX_DELAY);
    Drv_Screen_DispAll();
    xSemaphoreGive(oledMutex);
}

void Mid_Screen_DispNormal(void)
{
    xSemaphoreTake(oledMutex, portMAX_DELAY);
    Drv_Screen_DispNormal();
    xSemaphoreGive(oledMutex);
}

void Mid_Screen_DispArea(pic_info_s  *imageInfo)
{
    xSemaphoreTake(oledMutex, portMAX_DELAY);
    Drv_Screen_DispArea(imageInfo);
    xSemaphoreGive(oledMutex);
}


//**********************************************************************
// 函数功能:    放置OLED事件处理ID与参数，若是共用资源则会进行在任务中进行执行
// 输入参数：   
// 返回参数:    无
void  Mid_Screen_EventSet(screen_event_s * parametreTemp)
{
    switch(parametreTemp->id)
    {
        case SCREEN_BSP_ENABLE:
        Drv_Screen_Open();
        break;

        case SCREEN_BSP_DISABLE:
        Drv_Screen_Close();
        break;

        case SCREEN_SOFT_INIT:
        Drv_Screen_SoftInit(1);
        break;
        
        case SCREEN_LIGHT_ON:
        Drv_Screen_LightOn();
        break;
        
        case SCREEN_LIGHT_OFF:
        Drv_Screen_LightOff();
        break;
        
        case SCREEN_DISP_ALL:
        Drv_Screen_DispAll();
        break;
        
        case SCREEN_DISP_NORMAL:
        Drv_Screen_DispNormal();
        break;

        case SCREEN_DISP_AREA:
        Drv_Screen_DispArea(&parametreTemp->para.dispArea.areaInfo);
        break;

        case SCREEN_INIT_RAM:
        Mid_Screen_InitPicRam((parametreTemp->para.initDispRam.initRam));
        break;

        case SCREEN_INIT_RAM_AREA:
        Mid_Screen_InitAreaPicRam(&parametreTemp->para.initDispAreaRam.imageInfo, 
									parametreTemp->para.initDispAreaRam.initRam);
        break;

        case SCREEN_UPDATA_RAM_AREA:
        Mid_Screen_UpdataToPicRam(&parametreTemp->para.updataDispAreaRam.imageInfo, 
									parametreTemp->para.updataDispAreaRam.imagedatas);
        break;

        case SCREEN_INVERSE_RAM_AREA:
        Mid_Screen_InversePicRam(&parametreTemp->para.inverseDispRam.imageInfo);
        break;
    }
}

//**********************************************************************
// 函数功能:    对应位置显示双竖线
// 输入参数：   
// 返回参数:    
void Mid_Screen_DoubleLine(uint32_t XPox)
{
	for(uint32_t i = 0;i < 4;i++)
	{
		midPicRam[i*96 + XPox] = 0xFF;
		midPicRam[i*96 + XPox + 1] = 0xFF;
	}
}

//**********************************************************************
// 函数功能:    打印当前UI界面点阵信息
// 输入参数：    midPicRam[OLEDRAMLENGTH];
// 返回参数:  
void Mid_Screen_PrintDotInfo(void)
{
	for(uint32_t i = 0;i < OLEDRAMLENGTH;i++)
	{
		if(0 == i%16)
			SEGGER_RTT_printf(0,"\r\n");			
		SEGGER_RTT_printf(0,"0x%02X, ",midPicRam[i]);
	}	
}



