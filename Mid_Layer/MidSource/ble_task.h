#ifndef         BLE_TASK_H
#define         BLE_TASK_H

/* FreeRTOS includes */
#include "rtos.h"

#include "mid_common.h"

//蓝牙缓存的长度
#define BLE_TASK_QUEUE_SIZE     32

// 蓝牙缓存最大长度
//#define	BLE_BUF_LENGTH_MAX		64


// *************************************************************
// protocal common define
// *************************************************************


//蓝牙路由宏定义（参考蓝牙通信协议）
typedef enum
{
    MCU_TO_BLE          = 0x12,  //由MCU端发送到蓝牙端
    BLE_TO_MCU          = 0x21,  //由蓝牙端发送到MCU端
    MCU_TO_APP          = 0x13,  //由MCU端发送到应用端
    APP_TO_MCU          = 0x31,  //由应用端发送到MCU端
}communicate_route_t;

//接口类型宏定义（参考蓝牙通信协议）
typedef enum
{
    PROTOCAL_SET            = 0x01, //SET接口类型。设置某些参数，发送该类型接口需要等待应答
    PROTOCAL_GET            = 0x02, //GET接口类型。获取某些参数，发送该类型接口需要等待应答
    PROTOCAL_CALL           = 0x04, //CALL接口类型。执行某些参数，发送该类型接口需要等待应答
    PROTOCAL_RET            = 0x08, //RET接口类型。返回某些参数，接收端点接收到数据帧之后，
                                    //    若需要返回应答，可用该接口类型的接口帧
    PROTOCAL_ECHO           = 0x80, //ECHO接口类型。通知，发送该类型接口不需要等待应答
}communicate_interface_type;

//ACK帧返回参数宏定义（参考蓝牙通信协议）
typedef enum
{
    SUCCESS                 = 0x00, //成功
    INTERFACE_UNEXIST       = 0x01, //接口不存在
    INTERFACE_UNSURPPOT     = 0x02, //接口类型不支持
    PARAMATER_ERROR         = 0x03, //接口参数错误
    CHECKSUM_ERROR          = 0x04, //帧校验错误
}ack_id;

//接口类型
typedef enum
{
    PROT_LINK               = 0x00, //链路管理
    PROT_DEV_INFO           = 0x01, //设备信息
    PROT_EVENT              = 0x02, //暂无定义
    PROT_INTERACT           = 0x03, //交互控制
    PROT_DEV_TEST           = 0x04, //设备测试
    PROT_SENSOR             = 0x05, //传感器
    PROT_UPDATE             = 0x06, //升级
    PROT_APP_SPORT_SCENE  	= 0x10, //日常运动场景
    PROT_MOUNTAINEER_SCENE  = 0x11, //登山场景
    PROT_RUNNING_SCENE      = 0x12, //登山场景
    PROT_SWIMING_SCENE      = 0x13, //登山场景
    PROT_SLEEP_SCENE        = 0x14, //登山场景
    PROT_HRS_SCENE          = 0x15, //登山场景
    PROT_PACK_TRANS         = 0xF0, //数据包传输
    PROT_SCENE_PACK_TRANS   = 0xF2, //场景数据包传输
}app_protocal_interface_type;



// 链路接口
enum
{
	PROT_BT_STATE	      	= 0x01,
	PROT_BT_CONN_PARAM     	= 0x02,
	PROT_BT_ADV_NAME       	= 0x03,
	PROT_BT_ADV_PROJ_NUM   	= 0x04,
	PROT_BT_CONN_INTV      	= 0x05,
	PROT_BT_FW_VERSION		= 0x06,
	PROT_IOS_PAIR_REQ       = 0x07,
	PROT_LINKLOSS_REMIND_SWITCH = 0x11,
    PROT_LINK_HEARBEAT      = 0x12,
};


//宏定义设备信息获取命令（参考蓝牙通信协议）
typedef enum
{
    PROT_PROD_NAME     = 0x01, //产品名称
    PROT_PROD_VER      = 0x02, //固件版本号
    PROT_PROD_DATE     = 0x03, //生产日期
    PROT_PROD_SOC      = 0x04, //剩余电量
    PROT_PROD_AUTH     = 0x05, //授权
    PROT_PROD_SN       = 0x06, //SN 码
    PROT_PROD_PHONE_MOD= 0x07, //手机型号
    PROT_DATE          = 0x08, //时间日期
    PROT_SEC_CITY_DATE = 0x09, //第二城市时间日期
    PROT_ALARM_CLK     = 0x0A, //闹钟设置
	PROT_ALARM_CLK2    = 0x0B, //闹钟设置2
    PROT_WEIGHT_HEIGHT = 0x0E, //身高、体重
	PROT_SEC_CITY_DST  = 0x0D, //第二城市DST
	PROT_PHONE_WEATHER = 0x10, //手机下发天气信息
	PROT_PROD_SOC_NEW  = 0x14, // 新增剩余电量
}app_protocal_interface_dev_info;


typedef enum
{
	PROT_EVENT_OP      = 0x01,
	PROT_EVENT_INFO    = 0x02,
}app_protocal_interface_event;

//宏定义交互设置命令（参考蓝牙通信协议）
typedef enum
{
    PROT_NEW_MSG                    = 0x01, //新消息
    PROT_MSG_SWITCH                 = 0x02, //通知开关
    PROT_INCALL_RESP                = 0x03, //来电通知反馈，只产品发送
    PROT_CANCEL_MSG                 = 0x04, //消息取消通知
    PROT_MSG_DETAIL_SWITCH          = 0x05, //消息详情开关，只产品发送
    PROT_MSG_DETAIL                 = 0x06, //消息详情
    PROT_FIND_PHONE                 = 0x11, //查找手机，只产品发送
    PROT_PHOTO_MODE                 = 0x21, //进入、退出拍照
    PROT_PHOTO_REQ                  = 0x22, //拍照请求
    PROT_PHOTO_HEARTBEAT            = 0x23, //拍照状态心跳信息
    PROT_WATCH_HAND_MODE            = 0x31, //进入、退出校针模式
    PROT_WATCH_HAND_CTRL            = 0x32, //指针控制
    PROT_WATCH_HAND_PARAM           = 0x33, //设置校针参数
    PROT_WATCH_HAND_HEARTBEAT       = 0x34, //校针模式心跳信息
	PROT_LONG_SIT_SWITCH 			= 0x51, //久坐
    PROT_LONG_SIT_PARAM             = 0x52, //久坐参数
    PROT_DND_SWITCH                 = 0x61, //勿扰开关
    PROT_DND_PARAM                  = 0x62, //勿扰参数
    PROT_SYS_SETTING                = 0x91, //系统设置
}app_protocal_interface_interact;

//运动场景命令宏定义
typedef enum
{
    PROT_SCENE_DS_MODE              = 0x01, //场景使能
    PROT_SCENE_DS_GOAL              = 0x02, //运动目标
    PROT_SCENE_DS_TOTAL_APP_STEPS 	= 0x03, //当天总步数
    PROT_SCENE_DS_HIS_STEPS         = 0x04,
    PROT_SCENE_DS_HIS_STEPS_TRANS   = 0x05,
}app_protocal_interface_daily_sport_scene;

//运动场景命令宏定义
typedef enum
{
    PROT_MEASURE_DATA_HRS_GET       = 0x02, //获取心率值
    PROT_MEASURE_DATA_HRS_START     = 0x03, //启动单次心率测量
}app_protocal_interface_hrs_scene;

//传感器控制命令宏定义
typedef enum
{
	PORT_SENSOR_DATA_UPDATE         = 0x01,
    PROT_CAMPASS_CTRL               = 0x02, //指南针控制
    PROT_DECLINATION_CONFIG         = 0x03, //磁偏角设置
    PROT_PR_TMP_UPLOAD              = 0x11, //气压、环境温度实时数据上传
    PROT_PR_TMP_HIS_UPLOAD          = 0x12, //气压、环境温度传感器历史数据信息
    PROT_PR_TMP_TRANS               = 0x13, //气压、环境温度传感器历史数据传输
	  
}app_protocal_interface_sensor;


typedef enum
{
    PROT_UPDATE_REQ                 = 0x01,  //升级请求
    PROT_UPDATE_IMAGE_INFO,                  //镜像信息
    PROT_UPDATE_IMAGE_DATA,                  //镜像数据
    PROT_UPDATE_IMAGE_END,                   //镜像传输结束
    PROT_UPDATE_FW,                          //更新固件
}app_protocal_interface_updata;

// 睡眠数据传输接口
typedef enum
{
    PORT_SLEEP_USAUL_PARA                       = 0x01,
    PORT_SLEEP_GOAD_PARA                        = 0x02,
    PORT_SLEEP_HISTORY                          = 0x03,
    PORT_SLEEP_GET_RECORD                       = 0x04,
}app_protocal_sleep_scene_type;

// 数据包传输接口
typedef enum
{
	PORT_SCENE_PACK_TRANS_GET_CONTENT_COUNT			    = 0x10,
	PORT_SCENE_PACK_TRANS_GET_CONTENT_INFO			    = 0x11,
	PORT_SCENE_PACK_TRANS_GET_PKG						= 0x12,
	PORT_SCENE_PACK_TRANS_AUTO_UPDATE_CONTENT_COUNT	    = 0x20,
	PORT_SCENE_PACK_TRANS_AUTO_UPDATE_CONTENT_INFO	    = 0x21,
	PORT_SCENE_PACK_TRANS_DEL_ALL						= 0x30,
	PORT_SCENE_PACK_TRANS_DEL_DATATYPE				    = 0x31,
	PORT_SCENE_PACK_TRANS_DEL_CONTENT_UTC				= 0x32,
}app_protocal_scene_pack_trans_type;


// 数据包传输接口
typedef enum
{
    PORT_PACK_TRANS_CH1                         = 0x01,
    PORT_PACK_TRANS_GET_CONTENT_COUNT           = 0x10,
    PORT_PACK_TRANS_GET_CONTENT_INFO            = 0x11,
    PORT_PACK_TRANS_GET_PKG                     = 0x12,
    PORT_PACK_TRANS_AUTO_UPDATE_CONTENT_COUNT   = 0x20,
    PORT_PACK_TRANS_AUTO_UPDATE_CONTENT_INFO    = 0x21,
    PORT_PACK_TRANS_DEL_ALL                     = 0x30,
    PORT_PACK_TRANS_DEL_DATATYPE                = 0x31,
    PORT_PACK_TRANS_DEL_CONTENT_UTC             = 0x32,
}app_protocal_pack_trans_type;

//接口索引1和接口索引2合成命令宏定义
typedef enum
{
    //PROT_DEV_INFO
    RET_PROT_PROD_NAME                  = 0x0101, //产品名称
    RET_PROT_PROD_VER                   = 0x0102, //固件版本号
    RET_PROT_PROD_DATE                  = 0x0103, //生产日期
    RET_PROT_PROD_SOC                   = 0x0104, //剩余电量
    RET_PROT_PROD_AUTH                  = 0x0105, //授权
    RET_PROT_PROD_SN                    = 0x0106, //SN 码
    RET_PROT_PROD_PHONE_SYS             = 0x0106, //手机操作系统
    RET_PROT_PROD_PHONE_MOD             = 0x0107, //手机型号
    RET_PROT_DATE                       = 0X0108, //时间日期
    RET_PROT_SEC_CITY_DATE              = 0X0109, //第二城市时间日期
    RET_PROT_ALARM_CLK                  = 0X010A, //闹钟设置
    RET_PORT_SCENE_DS_TOTAL_APP_STEPS 	= 0x1003, //当天总步数
    RET_PROT_MSG_SWITCH                 = 0x0302, //通知开关
    RET_PORT_BLEDISCREMIND_SWITCH       = 0x0011, //蓝牙链路管理
	RET_PORT_LONGSIT_SWITCH             = 0x0351, //久坐提醒开关
	RET_PROT_LONG_SIT_PARAM             = 0x0352, //久坐提醒参数设置
	RET_PORT_NOTDISTURB_SWITCH          = 0x0361, //勿扰提醒开关
    RET_PROT_DND_PARAM                  = 0x0362, //勿扰提醒参数设置
}app_protocal_ret_type;

//天气状况枚举
typedef enum
{
	WEATHER_0000_SUNNY  			= 0x0000, //晴
	WEATHER_0001_CLOUDY  			= 0x0001, //多云
	WEATHER_0002_OVERCAST  			= 0x0002, //阴
	WEATHER_0003_HAZE  				= 0x0003, //霾
	WEATHER_0004_ICERAIN  			= 0x0004, //冻雨	
	WEATHER_0005_FOGGY 		    	= 0x0005, //雾
	
	WEATHER_0100_LIGHT_RAIN  		= 0x0100, //小雨
	WEATHER_0101_MODERATE_RAIN 		= 0x0101, //中雨
	WEATHER_0102_HEAVY_RAIN  		= 0x0102, //大雨
	WEATHER_0103_RAINSTORM  		= 0x0103, //暴雨
	WEATHER_0104_BIG_RAINSTORM 		= 0x0104, //大暴雨
	WEATHER_0105_SUPPER_RAINSTORM 	= 0x0105, //特大暴雨
	
	WEATHER_0200_SNOW_SHOWER 		= 0x0200, //阵雪
	WEATHER_0201_LIGHT_SHOWER  		= 0x0201, //小雪
	WEATHER_0202_MODERATE_SNOW 		= 0x0202, //中雪
	WEATHER_0203_HEAVY_SNOW  		= 0x0203, //大雪
	WEATHER_0204_BLIZZARD 			= 0x0204, //暴雪
	
	WEATHER_0300_DUST  				= 0x0300, //浮尘
	WEATHER_0301_DUSTBLOW  			= 0x0301, //扬沙
	WEATHER_0302_SANDSTORM 			= 0x0302, //沙尘暴
	WEATHER_0303_STRONG_SANDSTORM	= 0x0303, //强沙尘暴

	WEATHER_0400_SHOWER  			= 0x0400, //阵雨
	WEATHER_0401_THUNDER_RAIN  		= 0x0401, //雷阵雨
	WEATHER_0402_SLEETY 			= 0x0402, //雨夹雪
	WEATHER_0403_HAIL 				= 0x0403, //阵雨伴有冰雹

	WEATHER_0500_S_TO_M_RAIN  		= 0x0500, //小雨-中雨
	WEATHER_0501_M_TO_H_RAIN 		= 0x0501, //中雨-大雨
	WEATHER_0502_H_TO_TH_RAIN  		= 0x0502, //大雨-暴雨
	WEATHER_0503_TH_TO_BH_RAIN  	= 0x0503, //暴雨-大暴雨
	WEATHER_0504_BH_TO_SH_RAIN 		= 0x0504, //大暴雨-特大暴雨

	WEATHER_0600_S_TO_M_SNOW  		= 0x0600, //小雪-中雪
	WEATHER_0601_M_TO_H_SNOW  		= 0x0601, //中雪-大雪
	WEATHER_0602_H_TO_TH_SNOW   	= 0x0602, //大雪-暴雪
	
}app_protocal_weather_type;


//**********************************************************************
// 函数功能:  计算接收数据校验值
// 输入参数： protocal
// 返回参数： 校验值 
//**********************************************************************
extern uint8 Mid_Ble_CheckSum(protocal_msg_t *protocal);

//**********************************************************************
// 函数功能: 把重发消息从数组中删除，并关闭超时定时器
// 输入参数：    
//         msg：消息地址
// 返回参数： 
//**********************************************************************
extern ret_type Mid_Ble_DelResendMsg(protocal_msg_t *msg);

//**********************************************************************
// 函数功能: 发送通信协议数据，包含数据，命令以及ACK
// 输入参数：msg；指向发送协议数据指针
// 返回参数：
//    Ret_QueueFull:消息队列满
//    Ret_OK:       发送成功
//**********************************************************************
extern ret_type Mid_Ble_SendMsg(ble_msg_t *msg);

//**********************************************************************
// 函数功能:  设置callback
// 输入参数： cb
// 返回参数： 无 
//**********************************************************************
extern void Mid_Ble_SetCallBack(protocal_cb cb);

//**********************************************************************
// 函数功能:  初始化蓝牙模块
// 输入参数： 无
// 返回参数： 无 
//**********************************************************************
extern void Mid_Ble_Init(void);

#endif      //BLE_APP_H

