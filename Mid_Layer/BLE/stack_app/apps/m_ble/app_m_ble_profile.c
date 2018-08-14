/**********************************************************************
**
**模块说明: BLE 协议栈提供给APP的服务接口定义
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.5.1  初版  ZSL  
**  fix 1:  2018.5.27 手动关闭后，会自动开广播，并自动连上的问题  
**  fix 2:  2018.5.31 开机后进仓储，蓝牙开广播，后power off引起的问题
**  fix 3:  2018.6.5  修改广播参数定义方式
**  fix 4:  2018.6.12 修改在OTA中频繁消息提醒，导致升级失败
**  fix 5:  2018.
**
**********************************************************************/
#include "platform_common.h"
#include "platform_debugcof.h"
#include "platform_feature.h"
#include "sm_sys.h"

#include "wsf_types.h"
#include "bstream.h"
#include "wsf_msg.h"
#include "wsf_trace.h"
#include "hci_api.h"
#include "dm_api.h"
#include "att_api.h"
#include "smp_api.h"
#include "app_api.h"
#include "app_db.h"
#include "app_ui.h"
#include "app_hw.h"
#include "svc_ch.h"
#include "svc_core.h"
#include "svc_dis.h"

#include "svc_lowsapp.h"
#include "gatt_api.h"
// ancs
#include "ancc_api.h"
#include "ancs_api.h"
#include "app_m_ble_profile.h"
#include "amotas_api.h"

#include "hci_em9304.h"
//#include "drv_em9304.h"
#include "BLE_Stack.h"
#include "radio_task.h"

#if(BLE_STACK_DEBUG_ENABLE ==1)
#define Ble_Debug(x) SEGGER_RTT_printf x
#else
#define Ble_Debug(x)
#endif

#define MIN(A,B) (A>B)? B:A

#define BLE_STACK_BOOT   0
#define BLE_STACK_ON     1

static m_ble_profile_recv_data_handler_t app_m_ble_recv_data_handler = NULL;

#define ADV_DATA_LEN_MAX 31

//fix 3:修改广播参数定义方式
static uint8_t adv_data[ADV_DATA_LEN_MAX]=
{
	2,                             /*! length */
    DM_ADV_TYPE_FLAGS,             /*! AD type */
    DM_FLAG_LE_GENERAL_DISC | DM_FLAG_LE_BREDR_NOT_SUP,  /*! flags */

    14,                            //length
    DM_ADV_TYPE_MANUFACTURER,      //Manufacturer specific data
    0x03,0xA3,                     //0xA303
    PROJECT_ID,                    //项目编号，5个字符
    0x1c,0x87,0x65,0x23,0x00,0x25, //默认MAC地址，6个字符，数据下标:  12~ 17

    BLE_ADV_LOCALNAME_LEN + 1,     //length   数据下标: 18
    DM_ADV_TYPE_LOCAL_NAME,        //local name
    BLE_ADV_LOCALNAME,             //广播名:  数据下标: 20
}; 
//fix 3: 2018.6.5

static void send_ancs_msg_detail_handler(uint8_t *str,uint8_t len,uint8_t type);

/*
 App Identifier Р????
 **********************************************************************************
 */
const uint8_t QQIdentifier[] =               {"com.tencent.mqq"};           // tecent qq
const uint8_t WechatIdentifier[] =           {"com.tencent.xin"};           // tencent weixin
const uint8_t TencentWeiboIdentifier[] =     {"com.tencent.WeiBo"};         // tecent weibo
const uint8_t SkypeIdentifier[] =            {"com.skype.tomskype"};        // skype
const uint8_t SinaWeiboIdentifier[] =        {"com.sina.weibo"};            // sina weibo
const uint8_t MessengerIdentifier[] =        {"com.facebook.Messenger"};    // facebook
const uint8_t FacebookIdentifier[] =         {"com.facebook.Facebook"};     // facebook2
const uint8_t TwitterIdentifier[] =          {"com.atebits.Tweetie2"};      // twitter
const uint8_t WhatsappIdentifier[] =         {"net.whatsapp.WhatsApp"};     // whatsapp
const uint8_t LineIdentifier[] =             {"jp.naver.line"};             // line
const uint8_t SMSIdentifier1[] =             {"rrrreee"};
const uint8_t SMSIdentifier[] =              {"com.apple.MobileSMS"};       // message
const uint8_t InSmartmovtIdentifier[] =      {"com.smartmovt.w07-001"};		// selfdefine
const uint8_t momoIndentifier[]=             {"com.wemomo.momoappde"};
const uint8_t CalllingIdentifier[] =         {"com.apple.mobilephone_calling"};          //call     为了区分来电和未接来电，自定义
const uint8_t MissCalllingIdentifier[] =     {"com.apple.mobilephone_miss_calling"};     //miss call
const uint8_t TimIdentifier[]      =         {"com.tencent.tim"};   //tim


static ios_ancc_bitmap_match_t  ios_ancc_bitmap_match[] =
{
    {QQIdentifier,               TYPE_ANCS_MSG_QQ ,          0, send_ancs_msg_detail_handler},
    {WechatIdentifier,           TYPE_ANCS_MSG_WECHAT ,      0, send_ancs_msg_detail_handler},
    {TencentWeiboIdentifier,     TYPE_ANCS_MSG_TENCENT_WEIBO,0, send_ancs_msg_detail_handler},
    {SkypeIdentifier,            TYPE_ANCS_MSG_SKYPE ,       0, send_ancs_msg_detail_handler},
    {SinaWeiboIdentifier,        TYPE_ANCS_MSG_SINA_WEIBO ,  0, send_ancs_msg_detail_handler},
    {FacebookIdentifier,         TYPE_ANCS_MSG_FACEBOOK ,    0, send_ancs_msg_detail_handler},
    {TwitterIdentifier,          TYPE_ANCS_MSG_TWITTER ,     0, send_ancs_msg_detail_handler},  
    {WhatsappIdentifier,         TYPE_ANCS_MSG_WHATAPP ,     0, send_ancs_msg_detail_handler},
    {LineIdentifier,             TYPE_ANCS_MSG_LINE ,        0, send_ancs_msg_detail_handler},
    {CalllingIdentifier,         TYPE_ANCS_MSG_INCALL,       0, send_ancs_msg_detail_handler},      
    {SMSIdentifier,              TYPE_ANCS_MSG_MESSAGE,      0, send_ancs_msg_detail_handler},
    {MissCalllingIdentifier,     TYPE_ANCS_MSG_MISSCALL,     0, send_ancs_msg_detail_handler},
    {TimIdentifier,              TYPE_ANCS_MSG_TIM,          0, send_ancs_msg_detail_handler}, 
};
   

const ancs_app_id_type ancs_app_id_list[] = 
{
	/* APP NOTIFICATION IDENTIFIER */
	{TYPE_ANCS_MSG_MESSAGE,      SMSIdentifier, 			(sizeof(SMSIdentifier)-1)},
	{TYPE_ANCS_MSG_QQ, 			  QQIdentifier, 			(sizeof(QQIdentifier)-1)},
	{TYPE_ANCS_MSG_WECHAT, 		  WechatIdentifier, 		(sizeof(WechatIdentifier)-1)},
	{TYPE_ANCS_MSG_TENCENT_WEIBO, TencentWeiboIdentifier,	(sizeof(TencentWeiboIdentifier)-1)},
	{TYPE_ANCS_MSG_SKYPE, 		  SkypeIdentifier,		    (sizeof(SkypeIdentifier)-1)},
	{TYPE_ANCS_MSG_SINA_WEIBO, 	  SinaWeiboIdentifier,  	(sizeof(SinaWeiboIdentifier)-1)},
	{TYPE_ANCS_MSG_FACEBOOK, 	  FacebookIdentifier, 	    (sizeof(FacebookIdentifier)-1)},
	{TYPE_ANCS_MSG_TWITTER,       TwitterIdentifier, 		(sizeof(TwitterIdentifier)-1)},
	{TYPE_ANCS_MSG_WHATAPP, 	  WhatsappIdentifier, 	    (sizeof(WhatsappIdentifier)-1)},
	{TYPE_ANCS_MSG_LINE, 		  LineIdentifier, 		    (sizeof(LineIdentifier)-1)},
	{TYPE_ANCS_MSG_CALENDAR, 	  InSmartmovtIdentifier,	(sizeof(InSmartmovtIdentifier)-1)},
    {TYPE_ANCS_MSG_INCALL,        CalllingIdentifier,	    (sizeof(CalllingIdentifier)-1)},
    {TYPE_ANCS_MSG_MISSCALL,      MissCalllingIdentifier,   (sizeof(MissCalllingIdentifier)-1)},
    {TYPE_ANCS_MSG_TIM,           TimIdentifier,	        (sizeof(TimIdentifier)-1)},
    
};
#define ANCS_APP_ID_NUM			(sizeof(ancs_app_id_list)/sizeof(ancs_app_id_type))
    
#define ANCC_MSG_DETAIL_NUM     (sizeof(ios_ancc_bitmap_match)/sizeof(ios_ancc_bitmap_match_t))

ancs_msg_detail_temp_t ancs_msg_detail_temp = { 0,{0}};

static uint32_t volatile ancs_msg_bitmap = 0;
static uint32_t ios_msg_bitmap_switch = 0;
static uint8_t  send_flow = 0;

static uint8_t u8BtStatus = BT_ADV_OFF;      //记录蓝牙连接状态,开机在仓储状态下为非广播
static uint8 u8StackStatus = BLE_STACK_ON;//BLE_STACK_BOOT; //蓝牙开机时视为在boot状态，在power on动作之后才是ON

#if(BLE_STACK_RUN_INMCU == 1)
ble_echo pBleStatus_Echo = NULL;
#endif

/**************************************************************************************************
  Configurable Parameters
**************************************************************************************************/

/**************************************************************************************************
  Configurable Parameters
**************************************************************************************************/

/*! configurable parameters for advertising */
static const appAdvCfg_t m_ble_profileAdvCfg =
{
    {    0,     0,     0},                  /*! Advertising durations in ms */   //默认60000
    {  1600,     0,     0}                   /*! Advertising intervals in 0.625 ms units */
};

/*! configurable parameters for slave */
static const appSlaveCfg_t m_ble_profileSlaveCfg =
{
    ANCS_CONN_MAX,                           /*! Maximum connections */
};

/*! configurable parameters for security */
static const appSecCfg_t m_ble_profileSecCfg =
{
#if(BLE_PASSKEY_PAIRNG_ENABLE == 1)
    DM_AUTH_MITM_FLAG |                     /*! Authentication and bonding flags */
#endif
    DM_AUTH_BOND_FLAG,                      /*! Authentication and bonding flags */
    0,                                      /*! Initiator key distribution flags */
    DM_KEY_DIST_LTK,                        /*! Responder key distribution flags */
    FALSE,                                  /*! TRUE if Out-of-band pairing data is present */
    FALSE                                   /*! TRUE to initiate security upon connection */
};

/*! configurable parameters for ANCS connection parameter update */
static appUpdateCfg_t m_ble_profileUpdateCfg =
{
    3000,                                   /*! Connection idle period in ms before attempting
                                              connection parameter update; set to zero to disable */
    20,                                     /*! *1.25ms = 300ms */
    50,                                     /*! *1.25ms = 400ms*/
    3,                                      /*! Connection latency */
    600,                                    /*! Supervision timeout in 10ms units */
    5                                       /*! Number of update attempts before giving up */
};

/*! SMP security parameter configuration */
/* Configuration structure */
static const smpCfg_t m_ble_profileSmpCfg =
{
    3000,                                   /*! 'Repeated attempts' timeout in msec */
#if(BLE_PASSKEY_PAIRNG_ENABLE == 1)
    SMP_IO_DISP_ONLY,
#else
    SMP_IO_NO_IN_NO_OUT,                    /*! I/O Capability */
#endif
    7,                                      /*! Minimum encryption key length */
    16,                                     /*! Maximum encryption key length */
    3,                                      /*! Attempts to trigger 'repeated attempts' timeout */
    0                                       /*! Device authentication requirements */
};

/*! Configurable parameters for service and characteristic discovery */
static const appDiscCfg_t m_ble_profileDiscCfg =
{
    FALSE                                   /*! TRUE to wait for a secure connection before initiating discovery */
};

/*! scan data, discoverable mode */
static const uint8_t ancsScanDataDisc[] =
{
    /*! service UUID */                     
    3,                                /*! length */
    DM_ADV_TYPE_16_SOLICIT,           /*! AD type */
    0x00,0xA8,
};

//fix 4:修改在OTA中频繁消息提醒，导致升级失败
static uint8 u8BleMode = BLE_MODE_NORMAL;
//fix: 2018.6.12

//**********************************************************************
// 函数功能: 打印收发数据
// 输入参数：
// 返回参数：
//**********************************************************************
static void ble_printdata(uint8_t *buf)
{
#if(BLE_STACK_DEBUG_ENABLE ==1)
    uint8_t	i;

    for(i = 0; i < buf[MSG_LEN] + 6; i++)
    {
        Ble_Debug((0,"0x%x,",buf[i]));
    }
    Ble_Debug((0,"\n"));
#endif
}

//**********************************************************************
// 函数功能: 计算数据校验值
// 输入参数：
// 返回参数：
//**********************************************************************
static uint8_t checksum(uint8_t *buf, uint8_t len)
{
    uint8_t i;
    uint8_t volatile ret = 0;

    for (i = 0; i < len; i++)
    {
        ret += (buf[i] ^ i);
    }

    return ret;
}

/*************************************************************************************************/
/*!
 *  \fn     以可被查找/可被连接的方式开启蓝牙广播
 *
 *  \brief  Set up advertising and other procedures that need to be performed after
 *          device reset.
 *
 *  \param  pMsg    Pointer to message.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void ble_adv_setup(void)
{
    /* set advertising and scan response data for discoverable mode */
    AppAdvSetData(APP_ADV_DATA_DISCOVERABLE, sizeof(adv_data), (uint8_t *) adv_data);
    AppAdvSetData(APP_SCAN_DATA_DISCOVERABLE, sizeof(ancsScanDataDisc), (uint8_t *) ancsScanDataDisc);

    /* set advertising and scan response data for connectable mode */
    AppAdvSetData(APP_ADV_DATA_CONNECTABLE, sizeof(adv_data), (uint8_t *) adv_data);
    AppAdvSetData(APP_SCAN_DATA_CONNECTABLE, sizeof(ancsScanDataDisc), (uint8_t *) ancsScanDataDisc);

    /* start advertising; automatically set connectable/discoverable mode and bondable mode */
    AppAdvStart(APP_MODE_AUTO_INIT);
}


/*打包数据，帧头等信息*/
static void send_msg_to_mcu(uint8_t *buf, uint8_t len)
{
    uint8_t frame_buf[25] = {0};
    uint8_t volatile buf_len = 0;

    frame_buf[buf_len++] = BLE_HEARD;
    frame_buf[buf_len++] = BLE_VERSON;
    frame_buf[buf_len++] = len;
    frame_buf[buf_len++] = BLE_TO_MCU;
    frame_buf[buf_len++] = send_flow++;

    memcpy(&frame_buf[buf_len], buf, len);
    buf_len += len;

    frame_buf[buf_len++] = checksum(frame_buf, buf_len);
    if(app_m_ble_recv_data_handler != NULL)
    {
        app_m_ble_recv_data_handler(frame_buf,buf_len);
    }
}

/*根据相应的提醒类型，打包数据帧的参数数据
 * @brief 		ancs app identifier notification event process
 *
 * @param[in] 	type		 ANCS_EVENT_TYPE
 * @param[in] 	action_type	 ancc_evt_id_values_t
*/
static void send_ancs_msg(ANCS_EVENT_TYPE type,uint8_t action_type)
{
    uint32_t msg_bit = 0;

    ancs_msg_bitmap = 0xffffffff;
    switch (type)
    {
    case TYPE_ANCS_MSG_QQ:
        msg_bit = ANCS_MSG_QQ & ancs_msg_bitmap;
        break;
    case TYPE_ANCS_MSG_WECHAT:
        msg_bit = ANCS_MSG_WECHAT & ancs_msg_bitmap;
        break;
    case TYPE_ANCS_MSG_TENCENT_WEIBO:
        msg_bit = ANCS_MSG_TENCENT_WEIBO & ancs_msg_bitmap;
        break;
    case TYPE_ANCS_MSG_SKYPE:
        msg_bit = ANCS_MSG_SKYPE & ancs_msg_bitmap;
        break;
    case TYPE_ANCS_MSG_SINA_WEIBO:
        msg_bit = ANCS_MSG_SINA_WEIBO & ancs_msg_bitmap;
        break;
    case TYPE_ANCS_MSG_FACEBOOK:
        msg_bit = ANCS_MSG_FACEBOOK & ancs_msg_bitmap;
        break;
    case TYPE_ANCS_MSG_TWITTER:
        msg_bit = ANCS_MSG_TWITTER & ancs_msg_bitmap;
        break;
    case TYPE_ANCS_MSG_WHATAPP:
        msg_bit = ANCS_MSG_WHATAPP & ancs_msg_bitmap;
        break;
    case TYPE_ANCS_MSG_LINE:
        msg_bit = ANCS_MSG_LINE & ancs_msg_bitmap;
        break;
    case TYPE_ANCS_MSG_OTHER1:
        msg_bit = ANCS_MSG_OTHER1 & ancs_msg_bitmap;
        break;
    case TYPE_ANCS_MSG_MESSAGE:
        msg_bit = ANCS_MSG_MESSAGE & ancs_msg_bitmap;
        break;
    case TYPE_ANCS_MSG_INCALL:
        msg_bit = ANCS_MSG_INCALL & ancs_msg_bitmap;
        break;        
    case TYPE_ANCS_MSG_INCALL_KNOW:
        msg_bit = (ANCS_MSG_INCALL_KNOW | ANCS_MSG_INCALL) & ancs_msg_bitmap;
        break;
    case TYPE_ANCS_MSG_TIM:
        msg_bit = ANCS_MSG_TIM & ancs_msg_bitmap;
        break;
    }

   
    if (msg_bit != 0)
    {
        uint8_t volatile idx = 0;
        uint8_t payload_buf[10] = {0};

        payload_buf[idx++] = CALL;
        payload_buf[idx++] = PROT_INTERACT;
        if(action_type == BLE_ANCS_EVENT_ID_NOTIFICATION_ADDED)
        {
            payload_buf[idx++] = PROT_NEW_MSG;
        }
        else if(action_type == BLE_ANCS_EVENT_ID_NOTIFICATION_REMOVED)
        {
            payload_buf[idx++] = PROT_CANCEL_MSG;
        }   
        payload_buf[idx++] = BYTE3(msg_bit);
        payload_buf[idx++] = BYTE2(msg_bit);
        payload_buf[idx++] = BYTE1(msg_bit);
        payload_buf[idx++] = BYTE0(msg_bit);

        send_msg_to_mcu(payload_buf, idx);
    }
}

/**
 ****************************************************************************************
 * @brief 		ancs app identifier notification event process
 *
 * @param[in] 	data		data
 * @param[in] 	data_len	data lenght
 * @param[in]   action_type        
 * @return ancs event type: ANCS_EVENT_TYPE
 ****************************************************************************************
 */
static void ancs_app_id_process(const uint8_t * data, uint8_t data_len,uint8_t action_type)
{
	uint8_t i;
	uint8_t msg_type = TYPE_ANCS_MSG_TYPE_INVALID;
	
	/*遍历查找相应的app id，获取提醒类型*/
	for (i = 0; i < ANCS_APP_ID_NUM; i++)
	{
		if (data_len >= ancs_app_id_list[i].str_len)
		{
			if (memcmp(ancs_app_id_list[i].str, data, ancs_app_id_list[i].str_len) == 0)
			{
				msg_type = ancs_app_id_list[i].type;
                send_ancs_msg((ANCS_EVENT_TYPE)msg_type,action_type);
				break;
			}
		}
	}
}

/*
处理ios通知回调的数据，包括app id 属性、title属性、subtitle属性、msg属性等，并把同一个消息的这些属性组合一起
*/
static void ios_msg_detail_state_process(const uint8_t *value,uint8_t lenth,GET_MSG_OP_STATE get_msg_op_state)
{
    Ble_Debug((0," len :%d  str:%s\n",lenth,value));
   
    static msg_detail_t msg_detail_temp;   
    static uint8_t msg_op_flag =0;
    uint8_t lenth_temp = 0;
    uint32_t ios_bitmap   = 0xFFFF;     
    for( int i =0;i< ANCC_MSG_DETAIL_NUM;i++)
    {
        if(ios_bitmap & BIT(i))
        {
          ios_ancc_bitmap_match[i].bitmap_flag = 1;     
        }                    
    }  

    switch(get_msg_op_state)
    {
    case START_GET_ATTR : 
        memset(&ancs_msg_detail_temp,0x00,sizeof(ancs_msg_detail_temp));
        memset(&msg_detail_temp,0x00,sizeof(msg_detail_temp));
        msg_op_flag = 0;
        break;
    case START_GET_APP_ID :
        lenth_temp = (lenth > APPID_TEMP_MAX_LEN)? APPID_TEMP_MAX_LEN : lenth;    
        memcpy(&msg_detail_temp.app_id_buf[0],&value[0],lenth_temp) ;
        msg_detail_temp.app_id_len = lenth_temp;
        Ble_Debug((0,"APP_ID :%s LEN :%d\n",msg_detail_temp.app_id_buf,msg_detail_temp.title_len));                  
        msg_op_flag |= (1<<0);
        break;
    case GET_TITLE :
        lenth_temp = (lenth > TITLE_TEMP_MAX_LEN)? TITLE_TEMP_MAX_LEN : lenth;            
        memcpy(&msg_detail_temp.title_buf[0],&value[0],lenth_temp);
        msg_detail_temp.title_buf[lenth_temp] = '\0'; //
        msg_detail_temp.title_len = lenth_temp + 1;
        Ble_Debug((0,"TITLE :%s LEN :%d\n",msg_detail_temp.title_buf,msg_detail_temp.title_len));
        msg_op_flag |= (1<<1);         
        break;
    case GET_SUBTITLE :
        if(lenth != 0)
        {   
            lenth_temp = (lenth > SUBTITLE_TEMP_MAX_LEN)? SUBTITLE_TEMP_MAX_LEN : lenth;
            memcpy(&msg_detail_temp.subtitle_buf[0],&value[0],lenth_temp);
            msg_detail_temp.subtitle_buf[lenth_temp] = '\0';
            msg_detail_temp.subtitle_len = lenth_temp + 1;
            Ble_Debug((0,"SUBTITLE :%s LEN :%d\n",msg_detail_temp.subtitle_buf,msg_detail_temp.subtitle_len));
        }                 
        msg_op_flag |= (1<<2);
        break;
    case GET_MSG :
        lenth_temp = (lenth > MSG_TEMP_MAX_LEN)? MSG_TEMP_MAX_LEN : lenth ;
        memcpy(&msg_detail_temp.msg_buf[0],&value[0],lenth_temp);  
        msg_detail_temp.msg_buf[lenth_temp] = '\0';
        msg_detail_temp.msg_len = lenth_temp + 1;
        Ble_Debug((0,"MSG :%s LEN :%d\n",msg_detail_temp.msg_buf,msg_detail_temp.msg_len));         
        msg_op_flag |= (1<<3);
        break;        
    }
     
    Ble_Debug((0,"msg_op_flag  :%d\n",msg_op_flag));     
    if(msg_op_flag == 0x0F)
    {
        msg_op_flag =0;
        ancs_msg_detail_temp.len = 0; 
        memcpy(&ancs_msg_detail_temp.value[ancs_msg_detail_temp.len],msg_detail_temp.title_buf,msg_detail_temp.title_len);
        ancs_msg_detail_temp.len += msg_detail_temp.title_len;
        if(msg_detail_temp.subtitle_len != 0)
        {
            memcpy(&ancs_msg_detail_temp.value[ancs_msg_detail_temp.len],msg_detail_temp.subtitle_buf,msg_detail_temp.subtitle_len);
            ancs_msg_detail_temp.len += msg_detail_temp.subtitle_len;
        }
        memcpy(&ancs_msg_detail_temp.value[ancs_msg_detail_temp.len],msg_detail_temp.msg_buf,msg_detail_temp.msg_len);
        ancs_msg_detail_temp.len += msg_detail_temp.msg_len;
          
        for(int i = 0; i< ANCC_MSG_DETAIL_NUM; i++)
        {
            if( (memcmp(ios_ancc_bitmap_match[i].str,msg_detail_temp.app_id_buf,msg_detail_temp.app_id_len) == 0)  &&  (ios_ancc_bitmap_match[i].bitmap_flag) )
            {
                Ble_Debug((0,"ancc_msg_detail_op \r\n"));
                if(ios_ancc_bitmap_match[i].ancc_msg_detail_op != NULL)
                {
                    ios_ancc_bitmap_match[i].ancc_msg_detail_op(ancs_msg_detail_temp.value,ancs_msg_detail_temp.len,ios_ancc_bitmap_match[i].type);                     
                }
            }
        }
    } 
}
#if 1
wsfTimerTicks_t idlePeriod = 10; 
wsfTimer_t              send_msg_timer;           


#define MAX_TEMP_LEN             256 //消息接收最大缓存
#define ONE_FRAME_VALID_LEN      9   //单个数据帧有效的msg内容长度
typedef struct
{
    uint8_t send_lenth ;
    uint8_t send_msg_idx;
//    uint8_t send_buf[MAX_TEMP_LEN];
}send_temp_t;
send_temp_t send_temp;
uint8_t send_buf[MAX_TEMP_LEN]={0};
//uint8_t send_lenth = 0;
//uint8_t send_msg_idx = 0;
send_msg_frame_t  send_msg_frame;
#endif

static void send_ancs_msg_detail_handler(uint8_t *str,uint8_t len,uint8_t type)
{
    Ble_Debug((0,"len :%d\n",len));

#if 1   //use timer to send info data to mcu
    memset(&send_msg_frame,0,sizeof(send_msg_frame));
    memset(&send_temp,0,sizeof(send_temp));
    memset(send_buf,0,sizeof(send_buf));
     
    //memcpy(send_temp.send_buf,str,len % MAX_TEMP_LEN);
    memcpy(&send_buf[0],str,len % MAX_TEMP_LEN);
     
    Ble_Debug((0,"ancs_msg_detail :%s\n",send_temp)); 
    
    send_temp.send_lenth = len;
    send_msg_frame.frame_head = BLE_HEARD;
    send_msg_frame.version = BLE_VERSON;
    send_msg_frame.route= BLE_TO_MCU;
    send_msg_frame.flow = send_flow++;    
    send_msg_frame.interface[0] = ECHO;
    send_msg_frame.interface[1] = PROT_INTERACT;
    send_msg_frame.interface[2] = PROT_MSG_DETAI_SWITCH;
    send_msg_frame.buf[0] = type;
    WsfTimerStartMs(&send_msg_timer, 10);
#else  
    while(send_lenth)
    {
        Ble_Debug((0,"send_lenth :%d\n",send_lenth)); 
        memset(&send_msg_frame.buf[1],0x00,11);
        if(send_lenth > ONE_FRAME_VALID_LEN)
        {
            send_msg_frame.par_lenth = ONE_FRAME_VALID_LEN + 5;  // 内容长度 9 + idx 1  + type 1 + 接口 3
            send_msg_frame.buf[1] = send_msg_idx;
            memcpy(&send_msg_frame.buf[2],&send_temp[send_msg_idx*ONE_FRAME_VALID_LEN],ONE_FRAME_VALID_LEN);
            send_msg_frame.buf[11] = checksum((uint8_t *)&send_msg_frame,send_msg_frame.par_lenth + 5);
              
            app_m_ble_recv_data_handler((uint8_t *)&send_msg_frame,send_msg_frame.par_lenth + 6);
                     
            send_lenth -= ONE_FRAME_VALID_LEN;
            send_msg_idx++;
        }
        else 
        {
            send_msg_frame.par_lenth = send_lenth + 5; //内容长度 + idx 1 + type 1 + 接口 3
            send_msg_frame.buf[1] = 0xff;
            memcpy(&send_msg_frame.buf[2],&send_temp[send_msg_idx*ONE_FRAME_VALID_LEN],send_lenth);
            send_msg_frame.buf[send_lenth + 2] = checksum((uint8_t *)&send_msg_frame,send_msg_frame.par_lenth + 5);
             
            app_m_ble_recv_data_handler((uint8_t *)&send_msg_frame,send_msg_frame.par_lenth + 6);       
            send_lenth = 0;
        }
    } 
#endif    
}

static void send_ancs_msg_detail_timerout_handler()
{   
    //fix 4:system will be in OTA mode, other module will stop working.
    if(app_m_ble_getmode() == BLE_MODE_OTA)
    {
        return;
    }
    //fix: 2018.6.12

    Ble_Debug((0,"send_ancs_msg_detail_timerout_handler\n")); 
    memset(&send_msg_frame.buf[1],0x00,11);
    if(send_temp.send_lenth > ONE_FRAME_VALID_LEN)
    {
        send_msg_frame.par_lenth = ONE_FRAME_VALID_LEN + 5;  // 内容长度 9 + idx 1  + type 1 + 接口 3
        send_msg_frame.buf[1] = send_temp.send_msg_idx;
        memcpy(&send_msg_frame.buf[2],&send_buf[send_temp.send_msg_idx*ONE_FRAME_VALID_LEN],ONE_FRAME_VALID_LEN);
        send_msg_frame.buf[11] = checksum((uint8_t *)&send_msg_frame,send_msg_frame.par_lenth + 5);

        if(app_m_ble_recv_data_handler != NULL)
        {
            (app_m_ble_recv_data_handler)((uint8_t *)&send_msg_frame,send_msg_frame.par_lenth + 6);
        }
                 
        send_temp.send_lenth -= ONE_FRAME_VALID_LEN;
        send_temp.send_msg_idx++;
        WsfTimerStartMs(&send_msg_timer, 50);
    }
    else 
    {
        send_msg_frame.par_lenth = send_temp.send_lenth + 5; //内容长度 + idx 1 + type 1 + 接口 3
        send_msg_frame.buf[1] = 0xff;
        memcpy(&send_msg_frame.buf[2],&send_buf[send_temp.send_msg_idx*ONE_FRAME_VALID_LEN],send_temp.send_lenth);
        send_msg_frame.buf[send_temp.send_lenth + 2] = checksum((uint8_t *)&send_msg_frame,send_msg_frame.par_lenth + 5);

        if(app_m_ble_recv_data_handler != NULL)
        {
            (app_m_ble_recv_data_handler)((uint8_t *)&send_msg_frame,send_msg_frame.par_lenth + 6);
        }
        send_temp.send_lenth = 0;
    }
 
}

//**********************************************************************
// 函数功能: 判断蓝牙是否在关闭状态
// 输入参数:
// 返回参数：1: 关闭广播或掉电
//**********************************************************************
static uint8 app_m_ble_isoff(void)
{
    if((u8BtStatus == BT_ADV_OFF) || (u8BtStatus == BT_POWEROFF))
    {
        return 1;
    }
    return 0;
}

static void Ble_PowerOnOff(uint8_t u8status)
{
    if(u8status == BT_POWERON)     //开蓝牙
    {
        Ble_Debug((0,"BT_POWERON \n"));
        SMDrv_SYS_DelayMs(200);
        HciDrvRadioBoot(0);
        #ifndef AM_PART_APOLLO3
        Drv_EM9304_EnableInterrupt();
        #endif
        DmDevReset();
        //fix 2:开机后进仓储，蓝牙开广播，后power off引起的问题
        u8StackStatus = BLE_STACK_ON;
        //fix 2018.5.31
    }
    else                           //关蓝牙
    {
        Ble_Debug((0,"BT_POWEROFF \n"));
        SMDrv_SYS_DelayMs(100);
        HciDrvRadioShutdown();
    }
    u8BtStatus = u8status;
}

static void Ble_SetStatus(uint8_t u8status)
{
    if(u8status == BT_ADV_ON)     //开蓝牙
    {
        Ble_Debug((0,"BT_ADV_ON \n"));
        ble_adv_setup();
    }
    else                           //关蓝牙
    {
        Ble_Debug((0,"BT_ADV_OFF \n"));
        dmConnId_t  connId;
        if ((connId = AppConnIsOpen()) != DM_CONN_ID_NONE) /*connected */
        {
            AppConnClose(connId);
        }
        
        AppAdvStop();
    }    
    u8BtStatus = u8status;
}

#if(BLE_STACK_RUN_INMCU == 0)
//**********************************************************************
// 函数功能: 设置/获取蓝牙状态
// 输入参数：buf:协议数据
// 返回参数：
//**********************************************************************
static void BLE_OperateBtStatus(uint8_t *buf)
{
    uint8_t intf_type = buf[INTER_TYPE];

    //Ble_Debug((0,"BLE_OperateBtStatus Data:\n"));
    //ble_printdata(buf);
    switch (intf_type)
    {
    case SET:   //设置蓝牙状态
        Ble_Debug((0,"u8BtStatus =%d,swtich=%d\n",u8BtStatus,buf[PARAM_IDX]));
        if((u8BtStatus == BT_ADV_OFF) && (buf[PARAM_IDX] == BT_ADV_ON))  //开蓝牙/广播
        {
            Ble_SetStatus(BT_ADV_ON);
        }
        else if(((u8BtStatus == BT_ADV_ON) || (u8BtStatus == BT_CONNECTED)) && (buf[PARAM_IDX] == BT_ADV_OFF))  //关蓝牙/广播
        {
            Ble_SetStatus(BT_ADV_OFF);
        }
        else
            ;
        break;
    case GET:       //获取蓝牙状态
        buf[MSG_LEN]    += 1;          //len
        buf[ROL_IDX]    = BLE_TO_MCU;  //route
        buf[INTER_TYPE] = RET;         //interface
        buf[PARAM_IDX]  = u8BtStatus;  //BT状态
        buf[PARAM_IDX+1]= checksum(buf,PARAM_IDX+1);

        if(app_m_ble_recv_data_handler != NULL)
        {
            (app_m_ble_recv_data_handler)(buf,buf[MSG_LEN] + 6);
        }
        break;
    default:
        break;
    }
}
#endif

//**********************************************************************
// 函数功能: 设置蓝牙广播名
// 输入参数：buf:协议数据
// 返回参数：
//**********************************************************************
static void BLE_SetAdvName(uint8 *buf,uint8 u8len)
{
    //fix 3:修改广播参数定义方式
    //step 1:更新广播信息中的广播名
    u8len = (u8len > 11)? 11: u8len;
    adv_data[18] = u8len + 1;
    memset(&adv_data[20], 0x00, 11);
    memcpy(&adv_data[20], buf, u8len);
    //fix 3: 2018.6.5

    //step 2: add to update device name, 2018.5.26 by zsl
    SvcCoreUpdateDeviceName(buf,u8len);
}

//**********************************************************************
// 函数功能: 设置蓝牙连接间隔
// 输入参数：buf:协议数据
// 返回参数：
//**********************************************************************
static void BLE_SetConnInterval(uint8_t u8Interval)
{
    hciConnSpec_t ConnSpec;
    dmConnId_t  connId;

    /* do update */    
    ConnSpec.minCeLen = 0;
    ConnSpec.maxCeLen = 0xffff;
    ConnSpec.connLatency = m_ble_profileUpdateCfg.connLatency;
    ConnSpec.supTimeout = m_ble_profileUpdateCfg.supTimeout;

    if ((u8Interval == 0) || (u8Interval == 2))
    {
        m_ble_profileUpdateCfg.connIntervalMin = 10;  /*! *1.25ms = 12.5 ms */
        m_ble_profileUpdateCfg.connIntervalMax = 40;  /*! *1.25ms = 50ms*/
    } 
    else
    {
        m_ble_profileUpdateCfg.connIntervalMin = 80;   /*! *1.25ms = 300ms*/
        m_ble_profileUpdateCfg.connIntervalMax = 160;  /*! *1.25ms = 400ms*/
    }
    ConnSpec.connIntervalMin = m_ble_profileUpdateCfg.connIntervalMin;   
    ConnSpec.connIntervalMax = m_ble_profileUpdateCfg.connIntervalMax;

    /*更新pAppUpdateCfg结构体指针参数*/
    pAppUpdateCfg = (appUpdateCfg_t *) &m_ble_profileUpdateCfg;
    if ((connId = AppConnIsOpen()) != DM_CONN_ID_NONE) /*connected */
    {
        DmConnUpdate(connId, &ConnSpec);
    }
}

//**********************************************************************
// 函数功能: 来电通知反馈
// 输入参数：buf:协议数据
// 返回参数：
//**********************************************************************
static void BLE_InCallResp(uint8_t *buf)
{
    if(buf[INTER_TYPE] == CALL)
    {
        if (buf[PARAM_IDX] == 0) // 拒接来电
        {
            Ble_Ancs_RejectCall();
        }
    }
}

//**********************************************************************
// 函数功能: 消息详情开关
// 输入参数：buf:协议数据
// 返回参数：
//**********************************************************************
static void BlE_SwitchMsgDetail(uint8_t *buf)
{
    switch (buf[INTER_TYPE])
    {
    case SET:
        ios_msg_bitmap_switch = (buf[PARAM_IDX+3] << 24) | (buf[PARAM_IDX+2] << 16) | (buf[PARAM_IDX+1] << 8) | buf[PARAM_IDX];
             
        for(int i =0;i< ANCC_MSG_DETAIL_NUM;i++)
        {
            ios_ancc_bitmap_match[i].bitmap_flag = ios_msg_bitmap_switch & BIT(i);        
        }            
        break;
    default:
        break;
    }
}

//**********************************************************************
// 函数功能: 数据传输
// 输入参数：buf:协议数据
// 返回参数：
//**********************************************************************
static void BLE_PacktTransport(uint8_t *buf)
{
    uint8_t trans_data_len = buf[MSG_LEN] - 3;

    switch (buf[INTER_TYPE])
    {
    case SET: //01 f0 01
        if (trans_data_len <= 20)
        {
            uint8_t trans_data_buf[20] = {0};

            for (uint8_t i = 0; i < trans_data_len; i++)
            {
                trans_data_buf[i] = buf[PARAM_IDX + i];
            }
            m_ble_profile_send_data(trans_data_buf,trans_data_len,SEND_CHANNEL_DATA);
        }
        break;
    default:
        break;
    }
}

//**********************************************************************
// 函数功能: 解析MCU发给BLE的协议数据
// 输入参数：buf:协议数据
// 返回参数：
//**********************************************************************
static void BLE_AnalysisData(uint8 *buf)
{
    if(buf[INTER_IDX] == PROT_INTERACT)   // 0x03: 交互控制
    {
        if(buf[INTER_IDX + 1] == PROT_INCALL_RESP)  // /* 03 03 */  
        {
            BLE_InCallResp(buf);
        }
        else if(buf[INTER_IDX + 1] == PROT_MSG_DETAI_SWITCH)
        {
            BlE_SwitchMsgDetail(buf);
        }
    }
    else if((buf[INTER_IDX] == PORT_PACK_TRANS) && (buf[INTER_IDX + 1] == PORT_PACK_TRANS_CH1)) /* f0 01   数据传输，数据通道*/
    {
        BLE_PacktTransport(buf);
    }
#if(BLE_STACK_RUN_INMCU == 0)
    else if(buf[INTER_IDX] == PROT_BT_LINK)    //0X00: 链路管理 
    {
        if(buf[INTER_IDX + 1] == PROT_BT_STATE)  //蓝牙状态  /* 00 01 */
        {
            BLE_OperateBtStatus(buf);
        }
        else if((buf[INTER_IDX + 1] == PROT_BT_ADV_NAME) && (buf[INTER_TYPE] == SET))  //设置蓝牙广播名 /* 00 03 */
        {
            BLE_SetAdvName(&buf[PARAM_IDX],((buf[MSG_LEN] - 3)%12));
        }
        else if((buf[INTER_IDX + 1] == PROT_BT_CONN_INTV) && (buf[INTER_TYPE] == CALL)) //设置连接间隔  /* 00 05 */
        {
            BLE_SetConnInterval(buf[PARAM_IDX]);
        }
    }
#endif
    else
        ;
}

void app_m_ble_handler_init(wsfHandlerId_t handlerId)
{
    /* Set configuration pointers */
    pAppAdvCfg = (appAdvCfg_t *) &m_ble_profileAdvCfg;
    pAppSlaveCfg = (appSlaveCfg_t *) &m_ble_profileSlaveCfg;
    pAppSecCfg = (appSecCfg_t *) &m_ble_profileSecCfg;
    pAppUpdateCfg = (appUpdateCfg_t *) &m_ble_profileUpdateCfg;
    pAppDiscCfg = (appDiscCfg_t *) &m_ble_profileDiscCfg;
    pSmpCfg = (smpCfg_t *) &m_ble_profileSmpCfg;
    /* Initialize application framework */
    AppSlaveInit();
    AppDiscInit();
    //m_ble_profile_handler_init(handlerId);
    AncsHandlerInit(handlerId);
    send_msg_timer.handlerId = handlerId;
    send_msg_timer.msg.event = APP_SEND_ANCS_MSG_TIMEOUT_IND;
}

void app_m_ble_handler(wsfEventMask_t event, wsfMsgHdr_t *pMsg)
{
#if(BLE_STACK_RUN_INMCU == 0)
    uint8 u8cmd[10] = {0x80,0x00,0x01};  //初始化为:echo蓝牙状态接口
#endif
    uint8 u8reason;

    if (pMsg == NULL)
        return ;

    //step 1: 协议栈事件优先处理
    AncsHandler(event, pMsg);

    //step 2: 处理客制化事件
    switch(pMsg->event)
    {
     case APP_SEND_ANCS_MSG_TIMEOUT_IND :
        send_ancs_msg_detail_timerout_handler();
        break;
    case DM_RESET_CMPL_IND:
        //fix 2:开机后进仓储，蓝牙开广播，后power off引起的问题
        //在开机过程先不做广播，等按键做power on后在开启广播
        if(u8StackStatus != BLE_STACK_BOOT)
        {
            //复位事件:开启广播
            ble_adv_setup();
        }
        //fix 2018.5.31
        break;
    case DM_CONN_CLOSE_IND:
        //step a: get BLE conn close reason value
        u8reason = ((hciDisconnectCmplEvt_t*)pMsg)->reason;
        Err_Info((0,"Warning: Connection Closed due to: 0x%0x,u8BtStatus=%d\n",u8reason,u8BtStatus));

        //step b:check need resart adv or not,and echo status to mcu
        //fix 1:手动关闭后，会自动开广播，并自动连上的问题
        //在conn close事件发生，并且广播关闭的情况下，就不做重启广播的动作
        if(app_m_ble_isoff() == 0)
        {
            //restart advertising
            ble_adv_setup();

            //在没有MCU设置关蓝牙的状态下，蓝牙断开后处于广播状态
            //并将蓝牙断开事件，主动告知MCU
            //fix 3:在机芯和app未连接的状态下，将一些异常的连接状态值反馈给固件app
            // 导致在屏幕上显示异常的断开。
            //解法:只有在上一个连接状态为已连接的时候，才反馈断开事件信息
            if(u8BtStatus == BT_CONNECTED)
            {
                u8BtStatus = BT_ADV_ON;
#if(BLE_STACK_RUN_INMCU == 0)
                u8cmd[3] = u8BtStatus;   //蓝牙状态:广播
                send_msg_to_mcu(u8cmd, 4);
#else
                (pBleStatus_Echo)(BLE_CONN_STATUS_ECHO,u8BtStatus,u8reason);
#endif
            }
            //fix 3: 2018.5.31
        }
        //fix 1: 2018.5.27
        break;
    case DM_CONN_OPEN_IND:    //蓝牙连接上事件:echo to MCU
        u8BtStatus = BT_CONNECTED;
#if(BLE_STACK_RUN_INMCU == 0)
        u8cmd[3] = u8BtStatus;   //蓝牙状态:连接
        send_msg_to_mcu(u8cmd, 4);
#else
        (pBleStatus_Echo)(BLE_CONN_STATUS_ECHO,u8BtStatus,0x00);
#endif
        break;
    case DM_CONN_UPDATE_IND:   //连接参数改变
    default:
        break;
    }
}
    
/*************************************************************************************************/
/*!
 *  \fn     m_ble_profile_start
 *
 *  \brief  Start the application.
 *
 *  \return None.
 */
/*************************************************************************************************/
void app_m_ble_main_start(void)
{
    AncsStart();
    m_ble_ancs_process_handler_init(ancs_app_id_process,ios_msg_detail_state_process);
}

//fix 4:修改在OTA中频繁消息提醒，导致升级失败
//Get BLE mode
uint8 app_m_ble_getmode(void)
{
    return u8BleMode;
}
//fix

#if(BLE_STACK_RUN_INMCU == 1)
//**********************************************************************
// 函数功能: MCU与BLE之间数据交互
// 输入参数：type:交互类型
//    param: 数据参数
//    option:可选参数，如广播名长度
// 返回参数：
//**********************************************************************
ret_type BLE_Stack_Interact(Ble_Interact_Type type,uint8 *param,uint8 option)
{
    if(type == BLE_STATUS_SET)        //开关蓝牙广播
    {
        //if BLE is in OTA, then stop to set BLE on/off

        Ble_Debug((0,"u8BtStatus =%d,swtich=%d\n",u8BtStatus,param[0]));
        //set BLE on/off
        if(((u8BtStatus == BT_ADV_ON) || (u8BtStatus == BT_CONNECTED)) && (param[0] == BT_ADV_OFF))
        {
            Ble_SetStatus(BT_ADV_OFF);
        }
        else if((u8BtStatus == BT_ADV_OFF) && (param[0] == BT_ADV_ON))
        {
            Ble_SetStatus(BT_ADV_ON);
        }
        else
            ;
    }
    else if(type == BLE_POWERONOFF)
    {
        if((param[0] != BT_POWERON) && (param[0] != BT_POWEROFF))
            return Ret_InvalidParam;
        
        Ble_PowerOnOff(param[0]);
    }
    else if(type == BLE_STATUS_GET)            //获取蓝牙状态
    {
        *param = u8BtStatus;
    }
    else if(type == BLE_AVD_NAME_SET)      //设置广播名
    {
        BLE_SetAdvName(param,option);
    }
    else if(type == BLE_LINKINTV_SET)      //设置连接间隔
    {
        BLE_SetConnInterval(param[0]);
    }
    else
        ;
    return Ret_OK;
}
#endif

//**********************************************************************
// 函数功能: 设置蓝牙Echo动作callback
// 输入参数：pfun:callback
// 返回参数：ret_type
//**********************************************************************
ret_type BLE_Stack_SetEcho_CallBack(ble_echo pfun)
{
    if(pfun == NULL) 
        return Ret_InvalidParam;

    //step 1: set ble status echo callback
    pBleStatus_Echo = pfun;

#if(BLE_PASSKEY_PAIRNG_ENABLE == 1)
    //step 2: set diplay passkey echo callback
    AppUiDisplayCallBack(pfun);
#endif

    //step 3: set call back for ota
    amotas_set_callback(pfun);

    //step 4: set call back for ancs
    m_ble_ancs_callback(pfun);
    return Ret_OK;
}

//**********************************************************************
// 函数功能: 应用层发送数据到ble的接口，ble模块根据opid执行相应动作
// 输入参数：buf/len: 发送数据的buffer和长度
// 返回参数：Ret_OK:正常，  
//    Ret_InvalidParam: 参数非法
//**********************************************************************
ret_type BLE_Stack_Send(uint8 *buf,uint8 len)
{
    if((buf == NULL) || (len == 0))
    {
        return Ret_InvalidParam;
    }
 
    if((buf[HEARD_IDX] != BLE_HEARD) || (buf[VERSON_IDX] != BLE_VERSON) || ((buf[MSG_LEN] + 6) != len))
    {
        ble_printdata(buf);  //print error data
        return Ret_Fail;  //数据不正确
    }

    if (buf[ROL_IDX] == MCU_TO_BLE)
    {
        BLE_AnalysisData(buf); //解析MCU发给BLE的协议数据
    }
    else if (buf[ROL_IDX] == MCU_TO_APP)//直接传输，命令通道
    {
        uint8_t cmd_len = 0;

        cmd_len = buf[MSG_LEN] + 6;
        if(cmd_len != len)
        {
            len = (cmd_len > 20) ? 20 : cmd_len;
        }

        m_ble_profile_send_data(buf,len,SEND_CHANNEL_CMD);
    }
    else
    {
         ;
    }
    return Ret_OK;
}

//**********************************************************************
// 函数功能: 初始化,上层应用的数据及命令通道接收回调函数和ota数据通道
//           回调函数通过此接口注册到下层
// 输入参数：recv_data_handler:数据接收callback，
// 返回参数：
//    Ret_OK:正常，  
//    Ret_InvalidParam: 参数非法
//**********************************************************************
ret_type BLE_Stack_SetRecvCallBack(ble_recv recv_data_handler)
{
    if(recv_data_handler == NULL)
    {
        return Ret_InvalidParam;   //param is invalid
    }
    
    app_m_ble_recv_data_handler = recv_data_handler;
    m_ble_profile_transfer_data_init(recv_data_handler);
    return Ret_OK;
}

//**********************************************************************
// 函数功能: 检查ble发送是否空闲，空闲返回true
// 输入参数：无
// 返回参数：1:空闲，可发送；  0: 发送忙
//**********************************************************************
uint8 BLE_Stack_CheckSendStatus(void)
{
	return m_ble_profile_send_idle_check();
}

//**********************************************************************
// 函数功能: 设置MAC地址
// 输入参数：mac_buf
// 返回参数：Ret_OK:正常，  
//    Ret_InvalidParam: 参数非法
//**********************************************************************
ret_type BLE_Stack_SetMac(uint8 *mac_buf)
{
    uint8 u8index;
    if(mac_buf == NULL)
    {
        return Ret_InvalidParam;
    }

    //fix 3:修改广播参数定义方式
    //设置MAC地址
    for(u8index = 0; u8index < 6; u8index++)
    {
        adv_data[17 - u8index] = mac_buf[u8index];
    }
    //fix 3: 2018.6.5

    hciDrvSetMac(mac_buf);
    return Ret_OK;
}

//fix 4:修改在OTA中频繁消息提醒，导致升级失败
//**********************************************************************
// 函数功能: 获取ble模式
// 输入参数: u8Mode 
// 返回参数： BLE_MODE_OTA       OTA模式
//            BLE_MODE_NORMAL   正常通讯模式
//**********************************************************************
uint8 BLE_Stack_GetMode(void)
{
    return u8BleMode;
}

//**********************************************************************
// 函数功能: 设置ble模式
// 输入参数: u8Mode = BLE_MODE_OTA OTA模式
//           =  BLE_MODE_NORMAL    正常通讯模式
// 返回参数：无
//**********************************************************************
void BLE_Stack_SetMode(uint8 u8Mode)
{
    if(u8Mode > BLE_MODE_OTA)
    {
        Err_Info((0,"Error:Set BLE mode param is invalid\n"));
        return;
    }
    u8BleMode = u8Mode;
}
//fix 4

//**********************************************************************
// 函数功能: 初始化并创建BLE协议栈消息事件处理任务
// 输入参数：无
// 返回参数：无
//**********************************************************************
void BLE_Stack_Init(void)
{
    //ble is in normal mode,after board setup
    u8BleMode = BLE_MODE_NORMAL;
    // Run setup functions.
    RadioTaskSetup();

    // Create the functional tasks
    RadioCreateTask();
}

