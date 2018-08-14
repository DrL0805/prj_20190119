/**********************************************************************
**
**模块说明: OLED驱动接口
**chip: M01330
**pin脚说明: VDD2和VDD1--供电
** D0--SCLK, D1---MOSI,RES#--reset脚，低电平复位
** CS#--片选，A0--数据/命令控制脚，H:data, L:cmd
**
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.21  修改流程  ZSL  
**
**********************************************************************/
#define OLED_MODULE

#include "io_config.h"
#include "sm_gpio.h"
#include "sm_spi.h"
#include "sm_swspi.h"
#include "drv_screen.h"
#include "sm_sys.h"




/*******************macro define*******************/
    
//选择是否使用IO口模拟spi，默认用硬件spi
#define SOFT_SPI_ENABLE    1

//oled列地址偏移量
#define     COLUMNOFFECT                    18


// /*******************variable define*******************/


//初始化标记
static uint8 oledinit = 0;
//显示缓存
uint8 bspOledDispRam[OLEDRAMLENGTH];

//oled 页首地址
static const uint8 PageAddress_OLED[8] = {0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7};



/*******************function define*******************/

//**********************************************************************
// 函数功能: oled写一字节
// 输入参数： 
// x        :    写一字节
// 返回参数： 无
void OLED_Write_One_Byte(uint8_t x)
{
    SMDrv_GPIO_BitSet(OLED_CD_PIN);
     SMDrv_SWSPI_WriteByte(x);
}

//**********************************************************************
// 函数功能: oled写命令或数据
// 输入参数： 
// Data    :     命令或数据
// 返回参数： 无
void OLED_Write_CommandOrData(uint8_t Data)
{
    SMDrv_GPIO_BitClear(OLED_CS_PIN);
    SMDrv_GPIO_BitClear(OLED_CD_PIN);
    SMDrv_SWSPI_WriteByte(Data);
    SMDrv_GPIO_BitSet(OLED_CD_PIN);
    SMDrv_GPIO_BitSet(OLED_CS_PIN);
}

//**********************************************************************
// 函数功能: 设置oled显示地址
// 输入参数： 
// Page_Addr:    oled页地址
// ColumnAddr： oled列地址
// 返回参数： 无
void Set_OLED_Address(unsigned char *Page_Addr, unsigned char ColumnAddr)
{
 unsigned char tempA, tempB;
 ColumnAddr += COLUMNOFFECT;
 tempA = ColumnAddr & 0x0F;
 tempB = (ColumnAddr >> 4) + 0x10;
 OLED_Write_CommandOrData(*Page_Addr);
 OLED_Write_CommandOrData(tempA);
 OLED_Write_CommandOrData(tempB);
}

//**********************************************************************
// 函数功能: oled硬件初始化
// 输入参数： 
// Data    :     命令或数据
// 返回参数： 
// 0x00     :    初始化成功
// 0xff    :     初始化失败
uint8 Drv_Screen_Open(void)
{
    SMDrv_SWSPI_Open();

    //open gpio for CS,EN,RST,CD,all as output 
    SMDrv_GPIO_Open(OLED_CS_PIN,NULL,NULL);
    SMDrv_GPIO_BitSet(OLED_CS_PIN);  
    SMDrv_GPIO_Open(OLED_CD_PIN,NULL,NULL); 
    SMDrv_GPIO_Open(OLED_EN_PIN,NULL,NULL);
    SMDrv_GPIO_Open(OLED_RST_PIN,NULL,NULL);
    SMDrv_GPIO_BitSet(OLED_RST_PIN);

    oledinit = 1;
 return 0;
}


//**********************************************************************
// 函数功能: 设置oled断电
// 输入参数： 无
// 返回参数： 
// 0x00     :    设置成功
// 0xff    :     设置失败
uint8 Drv_Screen_Close(void)
{
     OLED_Write_CommandOrData(0xAD);      //DC-DC Control Mode Set
     OLED_Write_CommandOrData(0x8A);      //DC-DC OFF Mode Set
     OLED_Write_CommandOrData(0xAE);      //Set Display Off

    SMDrv_GPIO_Close(OLED_CS_PIN);
    SMDrv_SWSPI_Close();
    SMDrv_GPIO_Close(OLED_CD_PIN);
    SMDrv_GPIO_BitSet(OLED_EN_PIN); 

     oledinit = 0;   
     return 0;
}



//**********************************************************************
// 函数功能: 刷新显示oled部分区域
// 输入参数： 
// imageInfo：  显示区域信息：x,y坐标，宽、高
// 返回参数： 
// 0x00     :    设置成功
// 0xff    :     设置失败
uint8 Drv_Screen_DispArea(pic_info_s *imageInfo)
{
 uint8_t i, j;
 uint8_t AreaStartAddr, AreaStartPage, AreaHigh, AreaLenth;

 if (!oledinit)
 {
     return 0xff;
 }

 AreaStartAddr = imageInfo->axis_X;
 AreaStartPage = imageInfo->axis_Y / 8;
 AreaHigh = (imageInfo->axis_Y + imageInfo->height) / 8;
 AreaLenth = imageInfo->length;
 if (((imageInfo->axis_Y + imageInfo->height) % 8) != 0) 
 {
     AreaHigh++;
 }

 for (i = AreaStartPage; i < AreaHigh; i++) 
 {
     Set_OLED_Address((uint8_t*)(PageAddress_OLED + AreaStartPage), AreaStartAddr);
      SMDrv_GPIO_BitClear(OLED_CS_PIN);  
     for (j = AreaStartAddr; j < AreaLenth + AreaStartAddr; j++) 
     {
         OLED_Write_One_Byte(bspOledDispRam[(unsigned short)AreaStartPage * ROWLENGTH + j]);
     }
      SMDrv_GPIO_BitSet(OLED_CS_PIN);  
     AreaStartPage++;
 }
 return 0;
}



//**********************************************************************
// 函数功能: 设置oled所有像素显示为data
// 输入参数： 
// Data  :   像素点显示的内容
// 返回参数： 无
void Drv_Screen_AllOff(uint8_t Data)
{
 uint8_t i, j;
 for (i = 0; i < 8; i++ ) 
 {
     Set_OLED_Address((uint8_t*)(PageAddress_OLED + i), 0);

      SMDrv_GPIO_BitClear(OLED_CS_PIN);  
     for (j = 0; j < ROWLENGTH; j++) 
     {
         OLED_Write_One_Byte(Data);
     }
      SMDrv_GPIO_BitSet(OLED_CS_PIN);  
 }
}

//**********************************************************************
// 函数功能: oled软件、硬件初始化
// 输入参数： dispDiretion: 0:正向显示，1：翻转显示
// 返回参数： 
// 0x00     :    设置成功
// 0xff    :     设置失败
uint8 Drv_Screen_SoftInit(uint8 dispDirect)
{
 uint16_t ret = 0;

 ret     |= Drv_Screen_Open();

 OLED_Write_CommandOrData(0xAE);      //Set Display Off
 OLED_Write_CommandOrData(0xD5);      //Display divide ratio/osc. freq. mode
 OLED_Write_CommandOrData(0x51);      //por,1
 OLED_Write_CommandOrData(0xA8);      //Multiplex ration mode:
 OLED_Write_CommandOrData(0x1F);      //31

 OLED_Write_CommandOrData(0xD3);      //Set Display Offset
 if(dispDirect)
 {
     OLED_Write_CommandOrData(0x10);      //normal start 0x30, reverse start 0x10
 }
 else
 {
     OLED_Write_CommandOrData(0x30);      //normal start 0x30, reverse start 0x10
 }
 OLED_Write_CommandOrData(0x40);      //Set Display Start line:0
 OLED_Write_CommandOrData(0xAD);      //DC-DC Control Mode Set
 OLED_Write_CommandOrData(0x8B);      //DC-DC ON/OFF Mode Set
    
 OLED_Write_CommandOrData(0x31);      //Set Pump voltage value  8.1
 if(dispDirect)
 {
     OLED_Write_CommandOrData(0xA1);      //Segment Remap    --->
     OLED_Write_CommandOrData(0xC8);      //Set COM Output Scan Direction
 }
 else
 {
     OLED_Write_CommandOrData(0xA0);      //Segment normal    --->
     OLED_Write_CommandOrData(0xC0);      //Set COM Output Scan Direction---> normal
 }
 OLED_Write_CommandOrData(0xDA);      //Common pads hardware: alternative
 OLED_Write_CommandOrData(0x12);
    
 OLED_Write_CommandOrData(0x81);      //Contrast control
 OLED_Write_CommandOrData(0xA0);
 OLED_Write_CommandOrData(0xD9);      //Set pre-charge period
 OLED_Write_CommandOrData(0x22);
 OLED_Write_CommandOrData(0xDB);      //VCOM deselect level mode
    
 OLED_Write_CommandOrData(0x25);
 OLED_Write_CommandOrData(0xA4);      //Set Entire Display On/Off
 OLED_Write_CommandOrData(0xA6);      //Set Normal Display
 Drv_Screen_AllOff(0xff);
 //OLED_Write_CommandOrData(0xAE);      //Set Display Off
    
 OLED_Write_CommandOrData(0xAF);      //Set Display On
 return ret;
}

//**********************************************************************
// 函数功能: oled显示屏亮
// 输入参数： 无
// 返回参数： 

// 0x00     :    设置成功
// 0xff    :     设置失败
uint8 Drv_Screen_LightOn(void)
{
 if (!oledinit)
 {
     return 0xff;
 }
 OLED_Write_CommandOrData(0xAF);      //Set Display On
 return 0;
}

//**********************************************************************
// 函数功能: oled显示屏灭
// 输入参数： 无
// 返回参数： 
// 0x00     :    设置成功
// 0xff    :     设置失败
uint8 Drv_Screen_LightOff(void)
{
 if (!oledinit)
 {
     return 0xff;
 }
 OLED_Write_CommandOrData(0xAE);  
 return 0;    
}

//**********************************************************************
// 函数功能: oled全屏显示
// 输入参数： 无
// 返回参数： 
// 0x00     :    设置成功
// 0xff    :     设置失败
uint8 Drv_Screen_DispAll(void)
{
 if (!oledinit)
 {
     return 0xff;
 }

 OLED_Write_CommandOrData(0xA4);   
 return 0;   
} 

//**********************************************************************
// 函数功能: oled正常显示
// 输入参数： 无
// 返回参数： 
// 0x00     :    设置成功
// 0xff    :     设置失败
uint8 Drv_Screen_DispNormal(void)
{
 if (!oledinit)
 {
     return 0xff;
 }

 OLED_Write_CommandOrData(0xA6);  
 return 0;    
}

//**********************************************************************
// 函数功能: 图像缓存数据更新到显示缓存，暂支持相同区域的数据复制,且8bits对齐
// 输入参数：
// imageInfo：  显示区域信息：x,y坐标，宽、高
//picRamTemp：　图像缓存的首地址
// 返回参数：返回oled 显示Ram首地址
//**********************************************************************
uint8 Drv_Screen_PicRamToDispRam(pic_info_s *imageInfo, const uint8 *picRamTemp)
{
    uint16 pixCnt;

    for (pixCnt = 0; pixCnt < OLEDRAMLENGTH; pixCnt ++)
    {
        bspOledDispRam[pixCnt] = picRamTemp[pixCnt];
    }

    /*时间原因，未作验证，默认全部复制
    pic_info_s zoneInfo;
    uint16 axis_Y_temp,axis_X_temp;
    uint16 axisX_Cnt,axisY_Cnt;
    uint16 axisX_Max,axisY_Max;
    uint16 axisY_Base;

    zoneInfo.axis_X = imageInfo->axis_X;
    zoneInfo.axis_Y = imageInfo->axis_Y;
    zoneInfo.height = imageInfo->height;
    zoneInfo.length = imageInfo->length;

    axis_Y_temp = zoneInfo.axis_Y % 8;

    if (axis_Y_temp)
    {
        zoneInfo.axis_Y -= axis_Y_temp;
        zoneInfo.height += axis_Y_temp;
    }

    axis_Y_temp = (zoneInfo.axis_Y + zoneInfo.height) % 8;
    if (axis_Y_temp)
    {
        zoneInfo.height += axis_Y_temp;
    }

    axisX_Max  = zoneInfo.axis_X + zoneInfo.length;
    axisY_Max  = zoneInfo.height / 8;
    axisY_Base = zoneInfo.axis_Y / 8 * ROWLENGTH;

    for (axisY_Cnt = 0; axisY_Cnt < axisY_Max; axisY_Cnt ++)
    {
         for (axisX_Cnt = zoneInfo.axis_X; axisX_Cnt < axisX_Max; axisX_Cnt ++)
        {
            bspOledDispRam[axisY_Base + axisX_Cnt] = picRamTemp[axisY_Base + axisX_Cnt];
        }
        axisY_Base  += ROWLENGTH;
    }
    */
     return 0;  
}

//**********************************************************************
// 函数功能: oled正常显示
// 输入参数：无
// 返回参数：返回oled 显示Ram首地址
//**********************************************************************
uint8 *Drv_Screen_GetDispRam(void)
{
    return bspOledDispRam;
}


