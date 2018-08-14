#ifndef _APP_M_BLE_PROFILE_h
#define _APP_M_BLE_PROFILE_h
#include "stdint.h"
#include "stdbool.h"
#include "ancs_api.h"

#define BLE_HEARD    0x23
#define BLE_VERSON   0X01
#define BLE_FLOW     0X01

#define BT_ADV_OFF   0X00    //关蓝牙，蓝牙关闭状态
#define BT_ADV_ON    0X01    //开蓝牙，蓝牙开启状态
#define BT_CONNECTED 0X02    //蓝牙已连接

#define PROT_BT_LINK                    (0x00)
#define PROT_BT_STATE                   (0x01)
#define PROT_BT_CONN_PARAM              (0x02)
#define PROT_BT_ADV_NAME                (0x03)
#define PROT_BT_ADV_PROJ_NUM			(0x04)
#define PROT_BT_CONN_INTV               (0x05)
#define PROT_BT_FW_VERSION              (0x06)


#define PROT_INTERACT                   (0x03)
#define PROT_NEW_MSG                    (0x01)
#define PROT_MSG_SWITCH                 (0x02)
#define PROT_INCALL_RESP                (0x03)
#define PROT_CANCEL_MSG                 (0x04)
#define PROT_MSG_DETAI_SWITCH           (0x06)

#define PORT_PACK_TRANS         (0xf0)
#define PORT_PACK_TRANS_CH1             (0x01)

#define PROT_UPDATE             (0x06)
#define PROT_UPDATE_IMAGE_DATA          (0x03)


#define BIT(n)			(uint32_t)(0x00000001 << (n))
#define BYTE3(n)		((uint8_t)(((n)>>24) & 0xff))
#define BYTE2(n)		((uint8_t)(((n)>>16) & 0xff))
#define BYTE1(n)		((uint8_t)(((n)>> 8) & 0xff))
#define BYTE0(n)		((uint8_t)(((n)>> 0) & 0xff))

typedef enum
{
    SET =0x01,
    GET =0x02,
    CALL=0x04,
    ECHO=0x80,
    RET =0x08,
}OP_TYPE;


enum
{
	BLE_TO_MCU =0x21,
	BLE_TO_APP =0x23,
	MCU_TO_BLE =0x12,
	MCU_TO_APP =0X13,
	APP_TO_MCU =0X31,
	APP_TO_BLE =0x32,
};


typedef enum
{
    ACK_ERR_SUCCESS,
    ACK_ERR_INTF,
    ACK_ERR_INTF_TYPE,
    ACK_ERR_INTF_PARAM,
    ACK_ERR_CHECKSUM,
    ACK_ERR_NO_ACK,
}ack_err_t;

enum
{
	HEARD_IDX  =0,
	VERSON_IDX =1,
	MSG_LEN    =2,
	ROL_IDX    =3,
	FLOW_IDX   =4,
	INTER_TYPE =5,
	INTER_IDX,
	PARAM_IDX  =8
};

typedef enum
{
	BLE_STATE_IDLE 		= 0,
	BLE_STATE_ADVERTISE = 1,
	BLE_STATE_CONNECTED	= 2
}BLE_STATE_T;


typedef enum
{
	TYPE_ANCS_MSG_QQ			    = (0),
	TYPE_ANCS_MSG_WECHAT		    = (1),
	TYPE_ANCS_MSG_TENCENT_WEIBO		= (2),
	TYPE_ANCS_MSG_SKYPE				= (3),
	TYPE_ANCS_MSG_SINA_WEIBO		= (4),
	TYPE_ANCS_MSG_FACEBOOK			= (5),
	TYPE_ANCS_MSG_TWITTER			= (6),
	TYPE_ANCS_MSG_WHATAPP			= (7),
	TYPE_ANCS_MSG_LINE				= (8),
	TYPE_ANCS_MSG_OTHER1		    = (9),
	TYPE_ANCS_MSG_INCALL		    = (10),
	TYPE_ANCS_MSG_MESSAGE			= (11),
	TYPE_ANCS_MSG_MISSCALL			= (12),
	TYPE_ANCS_MSG_CALENDAR			= (13),
	TYPE_ANCS_MSG_EMAIL				= (14),
	TYPE_ANCS_MSG_OTHER2		    = (15),
    TYPE_ANCS_MSG_INCALL_KNOW	    = (16),
	TYPE_ANCS_MSG_TIM               = (17),
    TYPE_ANCS_MSG_TYPE_INVALID      = (0xFF)
}ANCS_EVENT_TYPE;

typedef enum
{
	ANCS_MSG_QQ					= BIT(0),
	ANCS_MSG_WECHAT				= BIT(1),
	ANCS_MSG_TENCENT_WEIBO		= BIT(2),
	ANCS_MSG_SKYPE				= BIT(3),
	ANCS_MSG_SINA_WEIBO			= BIT(4),
	ANCS_MSG_FACEBOOK			= BIT(5),
	ANCS_MSG_TWITTER			= BIT(6),
	ANCS_MSG_WHATAPP			= BIT(7),
	ANCS_MSG_LINE				= BIT(8),
	ANCS_MSG_OTHER1				= BIT(9),
	ANCS_MSG_INCALL				= BIT(10),
	ANCS_MSG_MESSAGE			= BIT(11),
	ANCS_MSG_MISSCALL			= BIT(12),
	ANCS_MSG_CALENDAR			= BIT(13),
	ANCS_MSG_EMAIL				= BIT(14),
	ANCS_MSG_OTHER2				= BIT(15),
    ANCS_MSG_INCALL_KNOW	    = BIT(16),
	ANCS_MSG_TIM	            = BIT(17),
}ANCS_MSG_TYPE;


typedef void (*ANCC_MSG_DETAIL_OP)(uint8_t *str,uint8_t len ,uint8_t type);
    
typedef struct ancs_msg_detail_temp_t{
     uint8_t len;
     uint8_t value[200];
}ancs_msg_detail_temp_t;

typedef struct ios_ancc_bitmap_match_t
{ 
     const uint8_t *str;
     uint8_t type;
     uint8_t bitmap_flag;
     ANCC_MSG_DETAIL_OP ancc_msg_detail_op; 
}ios_ancc_bitmap_match_t;


#define TITLE_TEMP_MAX_LEN        50
#define SUBTITLE_TEMP_MAX_LEN     50
#define MSG_TEMP_MAX_LEN          150
#define APPID_TEMP_MAX_LEN        50
typedef struct 
{
    uint8_t title_len;
    uint8_t subtitle_len;
    uint8_t msg_len;
    uint8_t app_id_len;
    uint8_t title_buf[TITLE_TEMP_MAX_LEN];               //标题缓存大小
    uint8_t subtitle_buf[SUBTITLE_TEMP_MAX_LEN];         //副标题缓存大小
    uint8_t msg_buf[MSG_TEMP_MAX_LEN];                   //消息缓存大小
    uint8_t app_id_buf[APPID_TEMP_MAX_LEN];              //应用名称缓存大小
}msg_detail_t;

typedef struct
{
	uint8_t type;
	const uint8_t *str;
	uint8_t str_len;
}ancs_app_id_type;


typedef struct
{
    uint16_t                     company_identifier;                  /**< Company identifier code. */
    uint8_t                      proj_num[5];
	uint8_t                      mac_addr[6];
} ble_advdata_manuf_data_t;


typedef struct
{
	uint8_t                      size;
    uint8_t                      adv_name[11];  //最大11字节 
} ble_advdata_adv_name_data_t;

typedef struct 
{
     uint8_t frame_head;
     uint8_t version;
     uint8_t par_lenth;
     uint8_t route;
     uint8_t flow;
     uint8_t interface[3];
     uint8_t buf[12]; //param +check_sum
}send_msg_frame_t;

typedef void (*intf_handler)(uint8_t *buf);

//**********************************************************************
// 函数功能: 获取蓝牙状态
// 输入参数：
// 返回参数：
// BT_ADV_OFF   0X00    //关蓝牙，蓝牙关闭状态
// BT_ADV_ON    0X01    //开蓝牙，蓝牙开启状态
// BT_CONNECTED 0X02    //蓝牙已连接
//**********************************************************************
extern uint8_t Ble_GetStatus(void);

//Get BLE mode
uint8_t app_m_ble_getmode(void);

bool app_m_ble_profile_send_idle_check(void);
void app_m_ble_profile_transfer_data_init(m_ble_profile_recv_data_handler_t recv_data_handler );

void app_m_ble_main_start(void);
void app_m_ble_handler(wsfEventMask_t event, wsfMsgHdr_t *pMsg);
void app_m_ble_handler_init(wsfHandlerId_t handlerId);

void app_send_to_ble(uint8_t *buf,uint8_t len);

#endif 

