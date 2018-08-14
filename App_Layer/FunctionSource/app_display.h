#ifndef         APP_DISPLAY_H
#define         APP_DISPLAY_H

#include "mid_interface.h"
#include "app_variable.h"

typedef enum 
{
	BLE_TYPE_16X16 	= 0,//实际图标大小类型
	BLE_TYPE_24X24
}ble_type_t;

typedef enum 
{
	PHONE_TYPE_16X16 	= 0,	//实际图标大小类型
	PHONE_TYPE_24X24
}phone_pic_type_t;

typedef enum
{
    BAT_LEVER_0 = 0,
	BAT_LEVER_1,
	BAT_LEVER_2,
	BAT_LEVER_3,
	BAT_LEVER_4,
	BAT_LEVER_5,
}bat_level_t;		// 电池电量等级


typedef enum
{
    WEATHER_SUNNY 				= 0x0000,
	WEATHER_CLOUDY,
	WEATHER_OVERCAST,
	WEATHER_HAZE,
	WEATHER_ICERAIN,
	WEATHER_FOGGY,
    
	WEATHER_LIGHT_RAIN 			= 0x0100,
	WEATHER_MODERATE_RAIN,
	WEATHER_HEAVY_RAIN,
	WEATHER_RAINSTORM,
	WEATHER_BIG_RAINSTORM,
	WEATHER_SUPER_RAINSTORM,


	WEATHER_SNOW_SHOWER 		= 0x0200,
	WEATHER_LIGHT_SNOW,
	WEATHER_MODERATE_SNOW,
	WEATHER_HEAVY_SNOW,
	WEATHER_BLIZZARD,
    
	WEATHER_DUST       			= 0x0300,
	WEATHER_DUSTBLOW,
	WEATHER_SANDSTORM,
	WEATHER_STRONG_SANDSTORM,
    
	WEATHER_SHOWER    			=  0x0400,
	WEATHER_THUNDER_RAIN,
	WEATHER_SLEETY,
	WEATHER_HAIL,
    WEATHER_NO_INFO 			= 0xFFFF		// 无天气信息
}weather_status_t;			

typedef enum 
{
	RTC_DATE_TYPE 	= 0,
	RTC_WEEK_TYPE,
	RTC_TIME_TYPE,
	RTC_AM_PM_TYPE,
}rtc_type_t;

typedef enum 
{
	SPORT_TYPE_16X16 	= 0,
	SPORT_TYPE_24X24
}sport_pic_type_t;

typedef enum 
{
	SPORT_ACT_TYPE 	= 0,
	SPORT_STEP_TYPE,
	SPORT_CAL_TYPE,
	SPORT_TIME_TYPE,
	SPORT_DISTANCE_TYPE,
	SPORT_COMPLETE_TYPE,
}sport_type_t;

//编码方式：UTF-8/unicode/GB
typedef enum 
{
	LETTER_TYPTE_UTF8 	= 0,
	LETTER_TYPTE_UNICODE,
	LETTER_TYPTE_GB,
}letter_code_type_t;

typedef enum
{
	ENGLISH_TYPE 	= 0,
	CHINESE_TYPE,	
}letter_languge_type_t;

typedef enum 
{
	ALIC_MIDDLE 	= 0,		// 居中显示
	ALIC_LEFT,					// 左对齐
	ALIC_RIGHT,					// 右对齐
}alic_type_t;


/*************自制与字库的字型应统一排列***************/
//字型
typedef enum
{
	APP_SIZE_DEFAULT 		= 0,//默认使用自制的本地字型
	/***********字库的字型***************/
	APP_ASCII_SIZE_5X7 	    = 1,
	APP_ASCII_SIZE_7X8 	    = 2,
	APP_ASCII_SIZE_6X12 	= 3,
	APP_ASCII_SIZE_12_B_A 	= 4,
	APP_ASCII_SIZE_12_B_T 	= 5,
	APP_ASCII_SIZE_8X16 	= 6,
	APP_ASCII_SIZE_8X16_BOLD = 7,

	APP_ASCII_SIZE_16_A 	= 8,
	APP_ASCII_SIZE_16_T 	= 9,
	APP_GB_SIZE_12X12 	    = 10,
	APP_GB_SIZE_16X16 	    = 11,
	/***********字库的字型***************/
	APP_GB_SIZE_NULL2 	    = 13,
	APP_GB_SIZE_NULL3 	    = 14,
	/***********自制的字型***************/
	APP_F_TGL_24 			= 15,
	APP_F_TGL_48 			= 16,
	APP_F_TGL_64			= 17,
	APP_F_TGL_96 			= 18,
	/***********自制的字型***************/
}letter_size_t;


typedef struct 
{
    pic_info_s picAxisInfo;         //图片显示区域信息
    uint8      *picStrAddr;      //数据流地址
}pic_source_s;

typedef enum
{
	ALIGN_CENTER = 0,
}align_e;

typedef struct  
{
    pic_info_s 				letterAxisInfo;//字体显示坐标信息
	uint8					frontHeight;	// 字体高度，梁杰添加的变量
    uint8  					interval;//字间间隔
    alic_type_t  			alic;//对齐方式
    letter_languge_type_t	language;//语言类型		
    letter_code_type_t 		letterType;//编码方式：UTF-8/unicode/GB	，界面上只有GB，暂时只支持GB
    letter_size_t  			asciiLetterSize;//字母字形（字体）
    letter_size_t  			gbLetterSize;//国标字形
}letter_info_s;

typedef struct __letter_source_s
{
    letter_info_s  letterInfo;      		//字体显示特征信息
    uint8  letterStrLen;                  //字体编号流长度
    uint8  *letterStrAddr;                //字体编号流首地址
}letter_source_s;

void Mid_Screen_FontTgl24AlignDisp(letter_info_s *leterInfoTemp, uint8_t *numString, uint8_t numLen);

void BatDisp(letter_info_s  *leterInfoTemp, uint8 batVol);
void App_PicRamClear(void);
void App_RoundDisp(round_dirct_t dect,uint8 pixel);
void App_DispOff(void);

/***************************************************************
	以下是梁杰定义的函数以及结构体类型
***************************************************************/

/* 图片显示信息结构体 */
typedef struct __pic_disp_info_t
{
    uint8_t 	AxisX;			// 显示原点的X坐标
    uint8_t 	AxisY;			// 显示原点的Y坐标
	uint8_t		AreaLength; // 显示区域长
	uint8_t		AreaHeight; // 显示区域高
    uint8_t 	PictureLength;		// 显示图片长
    uint8_t 	PictureHeight;		// 显示图片高
	alic_type_t	Alic;			// 对齐方式
	const uint8_t*	DotBuf;			// 点阵信息
}pic_disp_info_t;	

/* 图片显示状态（显示/清除） */
typedef enum
{
    PIC_DISP_ON,		
	PIC_DISP_OFF		
}pic_disp_state_t;	// 图片显示状态（显示/清除）

typedef enum
{
    TIME_FORMAT_24 = 0,		
	TIME_FORMAT_12,		
}time_format_t;	

typedef struct
{
    uint8_t Hour;
	uint8_t Min;
	uint8_t Sec;
}time_data_t;	// 时间显示格式

typedef enum
{
	BINARY_TYPE_ON,
	BINARY_TYPE_OFF
}binary_type_t;			// 通用二值类型

// typedef enum
// {
//     LANGUAGE_EN,		
// 	LANGUAGE_CN		
// }language_type_t;	// 语言类型

typedef enum
{
    SLEEP_GRADE_EXCELLENT,		
	SLEEP_GRADE_GOOD,
	SLEEP_GRADE_ONLY_FAIR,
	SLEEP_GRADE_POOR,
}sleep_grade_type_t;	// 睡眠评级

typedef enum
{
    CLOCK_NAME_1 = 0,		
	CLOCK_NAME_2,
	CLOCK_NAME_3,
}clock_name_t;

// 通用底层函数 =======================================================
void AppScreenPictureDisp(pic_disp_info_t* PicDispInfo);
uint8_t AppScreenLocalAsciiDisp(uint8_t AxisX, uint8_t AxisY, uint8_t AsciiCode, letter_size_t LetterSize);
uint8_t AppScreenGBKAsciiDisp(uint8_t AxisX, uint8_t AxisY, uint8_t* CodeValue, letter_size_t LetterSize, letter_code_type_t CodeType);
uint8_t AppScreenGBKChineseDisp(uint8_t AxisX, uint8_t AxisY, uint8_t* CodeValue, letter_size_t LetterSize, letter_code_type_t CodeType);
uint8_t AppScreenUnicodeCharDisp(uint8_t AxisX, uint8_t AxisY, uint8_t* CodeValue, letter_size_t LetterSize, letter_code_type_t CodeType);
uint8_t AppScreenFrontValidWidthGet(uint8_t* CodeValue, letter_size_t LetterSize, letter_code_type_t CodeType);
void AppScreenFrontWidthGet(uint8_t* CodeValue, letter_size_t LetterSize, letter_code_type_t CodeType, uint8_t* WordWidth, uint8_t* ValidWidth);
void AppScreenTextLayoutUnicodeDisp(letter_source_s *LetterSource);
void AppScreenTextUnicodeDisp(letter_source_s *LetterSource);
void AppScreenTextGBKDisp(letter_source_s *LetterSource);
void AppScreenTextLayoutGBKDisp(letter_source_s *LetterSource);


// 时间模式接口函数 ===================================================
void AppTimeDateDisp(rtc_time_s RtcTime);
void AppTimeWeekDisp(rtc_time_s RtcTime);
void AppTimeTimeDisp(rtc_time_s RtcTime, time_format_t TimeFormat);
void AppTimeBatDisp(bat_level_t BatLevel);
void AppTimeMisscallDisp(pic_disp_state_t PicDispState);
void AppTimeBleDisp(pic_disp_state_t PicDispState);
void AppTimeWeatherDisp(weather_status_t WeatherStatus, int8_t Temperature);
void AppTimeBatFlick(pic_disp_state_t PicDispState);

// 计步模式接口函数 ===================================================
void AppStepMainInterfaceDisp(systerm_languge_t LanguageType);	// 计步主界面显示
void AppStepCompleteDisp(uint32_t StepNum, uint32_t Percent);	// 计步完成度显示
void AppStepKcalDisp(uint32_t Kcal);	// 计步卡路里显示
void AppStepDurationDisp(uint32_t Duration);	// 计步活动时长显示，单位分钟
void AppStepDistanceDisp(uint32_t Distance);	// 计步运动距离显示，单位米

// 心率模式接口函数 ===================================================
void AppHeartMainInterfaceDisp(systerm_languge_t LanguageType);
void AppHeartNoWearDisp(systerm_languge_t LanguageType);
void AppHeartDataDisp(binary_type_t status, uint8_t RestHeart, uint8_t DynamicHeart);
void AppHeartCheckFlick(binary_type_t status,uint8_t RestHeart);
void AppHeartPicDisp(binary_type_t status);		// 心率图标显示
void AppRestHeartDisp(uint8_t RestHeart);	// 静息心率显示	
void AppDynamicHeartDisp(uint8_t DynamicHeart);	// 动态心率显示	

// 睡眠模式接口函数 ===================================================
void AppSleepMainInterfaceDisp(systerm_languge_t LanguageType);
void AppSleepNodataDisp(systerm_languge_t LanguageType);
void AppSleepDurationDisp(uint32_t Minute, sleep_grade_type_t SleepGrade);
void AppSleepPointDisp(time_data_t* StartPoint, time_data_t* StopPoint, time_format_t TimeFormat);

// 闹钟模式接口函数 ===================================================
void AppClockMainInterfaceDisp(systerm_languge_t LanguageType);
void AppClockNoDataDisp(systerm_languge_t LanguageType);
void AppClockStateDisp(clock_name_t ClockName, binary_type_t ClockState, time_data_t* TimeDate, time_format_t TimeFormat);

// 设置模式接口函数 ===================================================
void AppSetMainInterfaceDisp(systerm_languge_t LanguageType);
void AppSetBleDisp(binary_type_t BleState, systerm_languge_t LanguageType);
void AppSetFindPhotoDisp(systerm_languge_t LanguageType);
void AppSetVersionDisp(uint32_t Version, uint8 soc,uint8 batVol);


// 提醒模式接口函数 ===================================================
void AppRemindCallDisp(uint8_t *String,uint8_t stringLen, letter_code_type_t strCodeType);
void AppRemindPhoneMsgDisp(uint8_t *String,uint8_t stringLen, letter_code_type_t strCodeType);
void AppRemindAppMsgDisp(uint8_t *String,uint8_t stringLen, letter_code_type_t strCodeType);
void AppRemindMsgContentDisp(uint8_t *String,uint8_t stringLen, letter_code_type_t strCodeType);
void AppRemindClockDisp(time_data_t* TimeDate, time_format_t TimeFormat);
void AppRemindGoalAttainedDisp(systerm_languge_t LanguageType);
void AppRemindLongSitDisp(systerm_languge_t LanguageType);
void AppRemindBleStateDisp(binary_type_t BleState, systerm_languge_t Language);
void AppRemindPlzUploadData(systerm_languge_t LanguageType);

// 跑步模式接口函数 ===================================================
void AppRunMainInterfaceDisp(systerm_languge_t LanguageType);	// 主界面显示
void AppRunStepNumFrequencyDisp(uint32_t StepNum, uint32_t StepFrequency);	// 步数步频显示
void AppRunDistanceSpeedDisp(uint32_t DistanceMeter, uint32_t SpeedSec);	// 距离配速

// 登山模式接口函数 ===================================================
void AppClimbMainInterfaceDisp(systerm_languge_t LanguageType);	// 主界面显示
void AppClimbAltitudeVerticalSpeedDisp(uint32_t Altitude, int32_t VerticalSpeed);	// 海拔和垂直速度

// 游泳模式接口函数 ===================================================
void AppSwimMainInterfaceDisp(systerm_languge_t LanguageType);	// 主界面显示
void AppSwimbAltitudeSpeedDisp(uint32_t StrokeNum, uint32_t StrokeFrequency);	

// 通用界面接口函数 ===================================================
void AppCommonCountDownStateDisp(uint32_t CountDownState);	// 倒计时状态
void AppCommonHrdKcalDisp(uint32_t Hrd, uint32_t Kcal);	// 心率卡路里
void AppCommonSportTimeDisp(uint32_t TimeSec);	// 运动计时
void AppCommontimeTemperatureDisp(rtc_time_s RtcTime, time_format_t TimeFormat, int32_t Temperature);
void AppCommonSaveStateDisp(uint32_t SaveState, systerm_languge_t LanguageType);	// 保存状态

//
void App_StoreBatLow(void);
void App_StoreTittle(systerm_languge_t lantype);
void App_PowerUpTittle(systerm_languge_t lantype);
void App_OtaTittle(systerm_languge_t lantype,uint8 level);

void App_ChargeTittle(uint8 soc,uint8 batVol);



//**********************************************************************
// 函数功能：    显示模块初始化
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
void App_DispInit(void (*TimerCb)(TimerHandle_t xTimer));

//**********************************************************************
// 函数功能：    滚动事件处理
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
void App_EventRoundProcess(void);

//**********************************************************************
// 函数功能：  拍照图标显示
// 输入参数：  无
// 返回参数： 无
void App_TakepictureDisp(void);


//**********************************************************************
// 函数功能： 配对码显示
// 输入参数： 无
// 返回参数： 无
//**********************************************************************
void App_PairCodeDisp(uint32 paireCode);


// 其他函数
void App_PrintfAsciiDotInfo(uint8_t* CodeValue, letter_size_t LetterSize);
void App_PrintfCurrUIInfo(void);



#endif          //APP_DISPLAY_H

