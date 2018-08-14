#ifndef         MID_SCREEN_H
#define         MID_SCREEN_H

#ifndef     PIC_INFO_S
#define     PIC_INFO_S

#define     EXRULE                          0           //the picture attribute
#define     INCRULE                         1

//INCRULE:the image data first 2byte was length and height,needn't indicate the lenhth and height
//EXRULE:the image not involve the length, need indicate the lenhth and height
typedef struct
{
    uint8  axis_X;
    uint8  axis_Y;
    uint8  height;
    uint8  length;
    uint8  dispmode; 
} pic_info_s;

#endif      //PIC_INFO_S



typedef struct
{
    pic_info_s      imageInfo;
    uint8_t         initdata;
}area_ram_init_s;

typedef struct
{
    pic_info_s      imageInfo;
    uint8_t         *initdata;
}area_ram_updata_s;

typedef struct
{
    pic_info_s      imageInfo;
    uint8_t         *characters;
    uint8_t         space;
}area_ram_chara_s;

typedef union
{
    struct 
    {
        pic_info_s      areaInfo;
    }dispArea;

    struct 
    {
        uint8_t     initRam;        
    }initDispRam;

    struct
    {
        pic_info_s  imageInfo;
        uint8_t     initRam;
    }initDispAreaRam;

    struct
    {
        pic_info_s  imageInfo;
        uint8_t *   imagedatas;
    }updataDispAreaRam;

    struct
    {
        pic_info_s  imageInfo;
        uint8_t *   characters;
        uint8_t     space;
    }dispCharacters;

    struct
    {
        pic_info_s  imageInfo;
    }inverseDispRam;
}sreen_event_para_s;


typedef struct 
{
    uint16_t            id;
    sreen_event_para_s   para;
}screen_event_s;



typedef enum
{
    SCREEN_BSP_ENABLE = 0,
    SCREEN_BSP_DISABLE,
    SCREEN_SOFT_INIT,
    SCREEN_LIGHT_ON,
    SCREEN_LIGHT_OFF,
    SCREEN_DISP_ALL,
    SCREEN_DISP_NORMAL,
    SCREEN_DISP_AREA,
    SCREEN_INIT_RAM,
    SCREEN_INIT_RAM_AREA,
    SCREEN_UPDATA_RAM_AREA,
    SCREEN_UPDATA_CHARA_RAM_AREA,
    SCREEN_INVERSE_RAM_AREA,
}SCREEN_INTERFACE_T;

typedef enum 
{
    ROUND_UP    = 0,
    ROUND_DOWN,
    ROUND_LEFT,
    ROUND_RIGHT,
    ROUND_NONE,
}round_dirct_t;


void Mid_Screen_InversePicRam(pic_info_s *imageInfo);
void Mid_Screen_InitAreaPicRam(pic_info_s  *imageInfo, uint8 initdata);
void Mid_Screen_UpdataToPicRam(pic_info_s  *imageInfo, const uint8 *imagedata);
void Mid_Screen_InitPicRam(uint8 data);
void Mid_Screen_PicRamTest(uint8_t TestValue);
void Mid_Screen_UpdataAreaToDispRam(pic_info_s *sourceZoneInfo);
void Mid_Screen_RoundPicRamToDisRam(bool FirstMoveFlg, uint8 rounDiret, uint8 roundPixel);
void Mid_Screen_Enable(void);
void Mid_Screen_Disable(void);
void Mid_Screen_SoftInit(void);
void Mid_Screen_LightOn(void);
void Mid_Screen_LightOff(void);
void Mid_Screen_DispAll(void);
void Mid_Screen_DispNormal(void);
void Mid_Screen_DispArea(pic_info_s  *imageInfo);
void Mid_Screen_EventSet(screen_event_s * parametreTemp);
void Mid_Screen_DoubleLine(uint32_t XPox);
void Mid_Screen_PrintDotInfo(void);

#endif          //SREEN_APP_H
