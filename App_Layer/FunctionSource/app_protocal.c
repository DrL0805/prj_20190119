/**********************************************************************************************
**
**模块说明: BLE 协议栈提供给APP的服务接口定义
**软件版本，修改日志(时间，内容),修改人:
**  fix 1:  2018.5.27 增加机芯app蓝牙状态标示，以解决机芯app蓝牙连接状态和蓝牙硬件连接同步问题
**  fix 2:  2018.6.4  手机请求授权后，中途断开，连接上后不发请求授权，机芯还发授权成功的命令
**  fix 3:  2018.6.6  蓝牙连接和机芯app连接同步问题，需要重启蓝牙广播
**  fix 4:  2018.6.7  增加OTA过程中各状态的反馈,以及相关控制
**  fix 5:  2018.6.9  蓝牙异常断开，出现卡死的情况,开断开提醒
**  fix 6:  2018.6.12 修改在OTA中频繁消息提醒，导致升级失败
**  fix 7:  2018.6.13 从开始OTA的时候计时，问题:有些手机出现接收数据就超时
**  fix 8:  2018.6.20 IOS消息开关关闭，仍可响应消息(提醒，显示详情)。
**  
***********************************************************************************************/
#include "platform_common.h"
#include "platform_debugcof.h"
#include "mid_common.h"

#include "mid_interface.h"
#include "app_variable.h"
#include "app_protocal.h"
#include "app_cachedata_manage.h"
#include "app_packdata_manage.h"
#include "app_sleepdata_manage.h"
#include "app_scenedata_manage.h"
#include "app_rtc.h"
#include "app_task.h"
#include "app_remind_manage.h"
#include "app_win_common.h"
#include "app_win_process.h"
#include "app_systerm.h"
#include "BLE_Stack.h"  //BLE stack
#include "sm_os.h"
#include "sm_sys.h"
#include "main.h"

#define APP_ProtocalTimeOut   15
#define APP_AuthorTimeOut     60
#define APP_TakePhotoTimeOut  15
#define APP_PaireTimeOut      60
#define APP_AdjustTimeOut     15
#define APP_Hrm_TimeOut       30
#define APP_OTATIMERFREQ      1     //OTA数据接收监测频率 1s


#if (APP_PROTOCAL_DEBUG ==1)
#define ENABLE_PRINT_DATA       0
#define Analy_Debug(x) SEGGER_RTT_printf x
#else
#define ENABLE_PRINT_DATA       0
#define Analy_Debug(x)
#endif

#define BCD_TO_DEC(bcd) ( ((bcd) >> 4) * 10 + ((bcd) & 0x0f) )
#define DEC_TO_BCD(dec) ( (((dec) / 10) << 4) + (((dec) % 10) & 0x0f) )

#define BLE_OTA_RECV_TIMEOUT          15 * 1000   //15S
#define BLE_CONNECT_ERROR_TIMEOUT     15 * 1000   //15S

// protocal
//ble 
const protocal_msg_t    OPEN_BLE_PROTOCAL =
{
    0x23,0x01,0x04,0x12,0x80,0x01,0x00,0x1,0x01
};

const protocal_msg_t    CLOSE_BLE_PROTOCAL =
{
    0x23,0x01,0x04,0x12,0x80,0x01,0x00,0x1,0x00
};

const protocal_msg_t    GET_BLE_STATUS_PROTOCAL =
{
    0x23,0x01,0x04,0x21,0x80,0x02,0x00,0x1,0x00
};

//app interface 
const protocal_msg_t    FIND_PHONE_PROTOCAL=
{
    0x23,0x01,0x03,0x13,0x80,0x04,0x03,0x11
};

const protocal_msg_t    TACK_PICTURE_PROTOCAL=
{
    0x23,0x01,0x03,0x13,0x80,0x04,0x03,0x22,
};

//adjust mode ret
const protocal_msg_t    EXIT_ADJUST_PROTOCAL=
{
    0x23,0x01,0x04,0x13,0x80,0x04,0x03,0x31,0x00,
};

const protocal_msg_t    ACCEPT_CALL_PROTOCAL=
{
    0x23,0x01,0x04,0x13,0x80,0x04,0x03,0x03,01
};

const protocal_msg_t    ACCEPT_CALL_IOS_PROTOCAL =
{
    0x23,0x01,0x04,0x12,0x80,0x04,0x03,0x03,0x01
};

//version
const protocal_msg_t    PROJECT_VER_PROTOCAL =
{
    0x23,0x01,0x06,0x13,0x80,0x08,0x01,0x02
};

//author
const protocal_msg_t AUTHOR_PASS_PROTOCAL =
{
    0x23,0x01,0x04,0x13,0x80,0x01,0x01,0x05,0x01
};

const protocal_msg_t AUTHOR_NG_PROTOCAL =
{
    0x23,0x01,0x04,0x13,0x80,0x01,0x01,0x05,0x00
};

const protocal_msg_t ADJUST_TIME_PROTOCAL =
{
    0x23,0x01,0x03,0x13,0x80,0x02,0x01,0x08
};
const protocal_msg_t GET_WEATHER_PROTOCAL =
{
    0x23,0x01,0x03,0x13,0x80,0x02,0x01,0x10
};
const protocal_msg_t BLE_LINKINV_FEEDBACK =
{
    0x23,0x01,0x05,0x13,0x80,0x80,0x00,0x05,0xff,0xff
};
const protocal_msg_t APP_SPORTSCENE_PROTOCAL =
{
	0x23,0x01,0x05,0x13,0x80,0x80,0x10,0x03,
};
    
const protocal_msg_t ALARM_TO_APP_PROTOCAL = 
{
    0x23,0x01,0x04,0x13,0x80,0x80,0x01,0x0A,
};

const protocal_msg_t HRM_TO_APP_PROTOCAL = 
{
    0x23,0x01,0x04,0x13,0x80,0x80,0x15,0x03,
};


//设置连接间隔
const protocal_msg_t CONN_INBTERAVLE_PROTOCAL =
{
    0x23,0x01,0x04,0x12,0x80,0x04,0x00,0x05
};

//设置连接间隔
const protocal_msg_t COMMON_ACK_PROTOCAL =
{
    0x23,0x01,0x01,0x13,0x80,0x00,0x00
};


uint16 dailyBleLinkInvSwitchWay;

static uint8 BrocastName[32];

uint8  debugu8Option;

typedef enum
{
	APP_PROTOCAL_DISCONNECTED = 0,  //APP未授权
	APP_PROTOCAL_RECVAUTHREQEST,    //APP接收到请求授权命令
	APP_PROTOCAL_ANCSDISCOVER,      //ANCS服务发现
	APP_PROTOCAL_CONNECTED,         //APP发送授权成功，连接上
}app_conn_type;

//fix 1: 增加机芯app蓝牙状态标示，以解决机芯app蓝牙连接状态和蓝牙硬件连接同步问题
//机芯app蓝牙连接状态: 以连接授权成功与否为准
//蓝牙硬件连接:是协议栈连接
static app_conn_type eAppProBleConn = APP_PROTOCAL_DISCONNECTED;
//fix 1: 2018.6.1


//fix 4:增加OTA过程中各状态的反馈
//OTA接收数据超时记录
static uint32 u32OtaRecvTimeout =0;

//**********************************************************************
// 函数功能: 提醒解析
// 输入参数：无
// 返回参数：    
//**********************************************************************
void App_NewMsgAnalysis(uint8_t PhoneSystem, uint32 remindstate, uint8 protocaltype)
{
    uint8_t      i;
    uint32_t     temp;
    uint8        validflag = 0;
//    menuMessage  message;
  
    for(i = 0; i < 32; i++)
    {
        temp = remindstate & (0x00000001 << i);
        switch(temp)
        {
            case APP_REMIND_QQ:
            case APP_REMIND_WECHAT:
            case APP_REMIND_TXWEIBO:
            case APP_REMIND_SKYPE:
            case APP_REMIND_XLWEIBO:
            case APP_REMIND_FACEBOOK:
            case APP_REMIND_TWITTER:
            case APP_REMIND_WHATAPP:
            case APP_REMIND_LINE:
            case APP_REMIND_OTHER1:
            case APP_REMIND_CALENDARMSG:
            case APP_REMIND_EMAIL:
            case APP_REMIND_OTHER2:           
            case APP_REMIND_PERSON:
            case APP_REMIND_TIM:
//            validflag       = 1;
//            message.val = PhoneSystem;
//            message.state   = protocaltype;
//            message.op   = UASUAL_MSG_REMIND;//应用消息
//            WinDowHandle = App_Window_Process(WinDowHandle,TAG_REMIND,message);
            break;

            case APP_REMIND_MSG:    //手机短信
//            validflag       = 1;
//            message.val = PhoneSystem;
//            message.state   = protocaltype;
//            message.op   = MESSAGE_REMIND;
//            WinDowHandle = App_Window_Process(WinDowHandle,TAG_REMIND,message);
            break;

            case APP_REMIND_CALL:   //来电   
//			validflag       = 1;
//            message.val = PhoneSystem;
//            message.state   = protocaltype;           
//            message.op    = PHONE_CALL_REMIND;
//            WinDowHandle  = App_Window_Process(WinDowHandle,TAG_REMIND,message);
            break;
            
            case APP_REMIND_MISSCALL:
//            validflag       = 1;
//            message.val = PhoneSystem;
//            message.state   = protocaltype; 
//            message.op    = PHONE_MISS_CALL;
//            WinDowHandle  = App_Window_Process(WinDowHandle,TAG_REMIND,message);
            break;

            default:break;
        }

        if (validflag)
        {
           break;
        }
   }
}

//**********************************************************************
// 函数功能: 通用ＡＣＫ
// 输入参数：无
// 返回参数：    
//**********************************************************************
static void Protocal_SendACK(ble_msg_t *protocal,uint8 u8Status)
{
	ble_msg_t bleMsg;
	uint8 u8route =  protocal->packet.att.routeMsg;

    bleMsg.id       = BLE_MSG_SEND;
    bleMsg.packet   = COMMON_ACK_PROTOCAL;
	bleMsg.packet.att.routeMsg   = ((u8route & 0xf0) >> 4) + (u8route << 4);
	bleMsg.packet.att.flowControl =protocal->packet.att.flowControl; 
	bleMsg.packet.att.load.data[0]   = u8Status;
	bleMsg.packet.att.load.data[1]   = Mid_Ble_CheckSum(&bleMsg.packet);
    Mid_Ble_SendMsg(&bleMsg);
}

//////////////////////////////////////////////////////////////////////////////////
//                       链路管理（00H, PROT_LINK）
//////////////////////////////////////////////////////////////////////////////////

//**********************************************************************
// 函数功能: 处理:蓝牙连接状态发生改变的情况
// 输入参数：u8Status:BLE状态
// 返回参数：	
//**********************************************************************
static void App_BleStatus_Change(uint8 u8Status)
{
//    movt_task_msg_t     movtMsg;
//	menuMessage        message;

    if(u8Status == BT_ADV_OFF) //蓝牙休眠
    {
        if ((systermConfig.bleDiscRemindSwitch == SWITCH_ON) && bleState == BLE_CONNECT)//连接断开
        {
//			MOD_PDU_RTT_LOG(0,"BLE OFF \n");
//            message.val   = debugu8Option;
//            message.state = BLE_DICONNECT_STATE;
//            message.op    = BLE_REMIND;
//            WinDowHandle  = App_Window_Process(WinDowHandle,TAG_REMIND,message);
        }      

        bleState    = BLE_SLEEP;
        //校针过程蓝牙关闭
        #if 0
        if (movtState.state == MOVT_STATE_ADJUST)
        {    
			movtState.timeCnt = 0;
            movtState.state   = MOVT_STATE_NORMAL;
            movtMsg.id  	  = MOVT_MSG_MC_RECOVER;
            MovtTask_EventSet(movtMsg);
        } 
        #endif
    
        if (phoneState.state == PHONE_STATE_PHOTO)
        {
//            message.state   = EXIT_TAKE_PHOTO_MODE;
//            message.op   	= TAKE_PHOTO_REMIND;
//            WinDowHandle 	= App_Window_Process(WinDowHandle,TAG_REMIND,message);      
            phoneState.state = PHONE_STATE_NORMAL;
        }             
    }
    
    if(u8Status == BT_ADV_ON)  //蓝牙广播
    {
        if ((systermConfig.bleDiscRemindSwitch == SWITCH_ON) && bleState == BLE_CONNECT)//连接断开
        {
//             message.val   = debugu8Option;
//            message.state = BLE_DICONNECT_STATE;
//            message.op    = BLE_REMIND;
//            WinDowHandle  = App_Window_Process(WinDowHandle,TAG_REMIND,message);
        }      

        bleState      = BLE_BROADCAST;
        //校针过程蓝牙断开
        #if 0
        if (movtState.state == MOVT_STATE_ADJUST)
        {
			movtState.timeCnt = 0;
            movtState.state   = MOVT_STATE_NORMAL;
            movtMsg.id  = MOVT_MSG_MC_RECOVER;
            MovtTask_EventSet(movtMsg);
        }
        #endif
    
        if (phoneState.state == PHONE_STATE_PHOTO)
        {
			phoneState.timeCnt  = 0;
//            message.state   = EXIT_TAKE_PHOTO_MODE;
//            message.op   	= TAKE_PHOTO_REMIND;
//            WinDowHandle 	= App_Window_Process(WinDowHandle,TAG_REMIND,message);
            phoneState.state = PHONE_STATE_NORMAL;
        }                          
    }
    
    if(u8Status == BT_CONNECTED)  //蓝牙连接
    {                    
        bleState      = BLE_CONNECT; 
		#if(AIR_PRESSURE_SUPPORT == 0)
        //连接上，请求一次气温
        App_Protocal_GetWeatherProcess();   
		#endif

        if (resetStatus)
        {
            App_Protocal_AdjustTimeprocess();
            resetStatus = 0;
        }
    }
}

//**********************************************************************
// 函数功能:    保存蓝牙广播名
// 输入参数：    
// 返回参数：    无
//**********************************************************************
static void Analysis_SaveAdvName(protocal_msg_t *protocal)
{
//    flash_task_msg_t flashMsg;
    uint8 j;  //uint16  result;
    uint8 length;
    length = protocal->att.loadLength - 0x03;
    if(length > BLE_BRODCASTNAME_MAXLENGTH)
        length = BLE_BRODCASTNAME_MAXLENGTH;
    memset(BrocastName,0,32);

    for(j=0;j<length;j++)
    {
        BrocastName[j] = protocal->att.load.content.parameter[j];
    }
    BrocastName[j] = '\0';
    Analy_Debug((0,"save avd name=%s\n",BrocastName));
//	flashMsg.id 									= EXTFLASH_ID;
//	flashMsg.flash.extflashEvent.para.dataAddr 		= (uint8*)(&BrocastName);
//	flashMsg.flash.extflashEvent.para.length 		= length;
//	flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_WRITE_BLE_BROCAST;
//	FlashTask_EventSet(&flashMsg);
}

//**********************************************************************
// 函数功能: 解析链路管理数据
// 输入参数：protocal:协议数据
// 返回参数：	
//**********************************************************************
static uint8 Analysis_Link(ble_msg_t *protocal)
{
    uint8       u8Route;     //记录路由
    uint8       u8FlowCtl;   //记录流控号
//    app_task_msg_t  appMsg;

    
#if(ENABLE_PRINT_DATA ==1)
    uint8 i;
    SEGGER_RTT_printf(0,"Link Data:\n");
    for(i = 0; i < protocal->packet.att.loadLength + 6; i++)
    {
        SEGGER_RTT_printf(0,"0x%x,",protocal->packet.data[i]);
    }
    SEGGER_RTT_printf(0,"\n");
#endif

    protocal->id = BLE_MSG_SEND;
    u8Route     = ((protocal->packet.att.routeMsg & 0xf0)>>4) + ( protocal->packet.att.routeMsg<<4); 
    u8FlowCtl   = protocal->packet.att.flowControl; 
    

    switch(protocal->packet.att.load.content.interfaceIndex2)
    {
    case PROT_BT_STATE:     //0x01: 
        if (protocal->packet.att.load.content.interfaceType == PROTOCAL_ECHO)//ble返回其工作状态
        {
            App_BleStatus_Change(protocal->packet.att.load.content.parameter[0]);
        }
        if (protocal->packet.att.load.content.interfaceType == PROTOCAL_GET)  //获取蓝牙工作状态
        {
                        
        }               
        break;

    case PROT_BT_CONN_PARAM:  //0x02: 
        
        break;
    case PROT_BT_ADV_NAME:    //0x03:    设置蓝牙广播名
        //step 1: set adv name to BLE
        if(protocal->packet.att.loadLength - 3 > 11)   //将协议中蓝牙名部分转换为字符串
            protocal->packet.data[11 + 8] = '\0';
        else
            protocal->packet.data[protocal->packet.att.loadLength + 5] = '\0';  //
        App_Protocal_SetBtAdvName(&protocal->packet.data[8]); 

        //step 2: save adv name to flash
        Analysis_SaveAdvName(&protocal->packet);

        //step 3: send back ACK
        Protocal_SendACK(protocal, SUCCESS);
        break;
    case PROT_BT_ADV_PROJ_NUM: //0x04: 
        
        break;
    case PROT_BT_CONN_INTV:    //0x05: 
        
        break;
    case PROT_BT_FW_VERSION:  //0x06: 返回蓝牙软件版本信息
        
        break;
    
    case PROT_LINKLOSS_REMIND_SWITCH: //0x11: 断开提醒开关
     if (protocal->packet.att.load.content.interfaceType == PROTOCAL_SET)
     {
        Protocal_SendACK(protocal, SUCCESS); 
        systermConfig.bleDiscRemindSwitch = protocal->packet.att.load.content.parameter[0];

        //设置保存事件
//        appMsg.id   = APP_CACEH_DATA_SAVE;
//        appMsg.para = CACHE_SYSTERM_TYPE;
//        App_Task_EventSet(appMsg);
		 MOD_PDU_RTT_LOG(0,"PROT_LINKLOSS_REMIND_SWITCH \n");
     }
     else
     {
        protocal->packet.att.loadLength += 1;
        protocal->packet.att.routeMsg = u8Route;
        protocal->packet.att.flowControl = u8FlowCtl;
        protocal->packet.att.load.content.interfaceType = PROTOCAL_RET; 
        protocal->packet.att.load.content.parameter[0]  = systermConfig.bleDiscRemindSwitch;
        protocal->packet.data[protocal->packet.att.loadLength + 5]  = Mid_Ble_CheckSum(&protocal->packet); 
        Mid_Ble_SendMsg(protocal);
     }
		
        break;

    case PROT_LINK_HEARBEAT:    //0x12:  链路心跳
        
        break;
    }
    return Ret_OK;
}

//////////////////////////////////////////////////////////////////////////////////
//                       设备信息（01H, PROT_DEV_INFO）
//////////////////////////////////////////////////////////////////////////////////

//**********************************************************************
// 函数功能: 更新连接状态
// 输入参数：u8status:连接状态
// 返回参数：
//**********************************************************************
static void App_Ble_UpdateConnStatus(app_conn_type status)
{
    //status:
    //APP_PROTOCAL_RECVAUTHREQEST: 第一阶段连接
    //APP_PROTOCAL_ANCSDISCOVER:   第一阶段连接,ANCS功能
    //APP_PROTOCAL_CONNECTED:      第二、三阶段连接
    //第三阶段配对连接，不会受操作影响，配对失败就会自动断开蓝牙连接
    eAppProBleConn = status;
}

//**********************************************************************
// 函数功能: 解析设备信息数据
// 输入参数：protocal:协议数据
// 返回参数：	
//**********************************************************************
static uint8 Analysis_DeviceInfo(ble_msg_t *protocal)
{
	Mid_Weahter_Param_t tWeather;
    uint32  u32temp;
    uint8   u8temp;      //记录中间值
    uint8   u8Route;     //记录路由
	uint8 	u8FlowCtl;   //记录流控号
    uint8         batType;
    rtc_time_s    appRtcTemp;
    alarm_clock_t alarmconfiginfo;
    bodyInfo_s    bodyinfo;
//    app_task_msg_t  appMsg;
    

#if(ENABLE_PRINT_DATA ==1)
    uint8 i;
    SEGGER_RTT_printf(0,"DeviceInfo Data:\n");
    for(i = 0; i < protocal->packet.att.loadLength + 6; i++)
    {
        SEGGER_RTT_printf(0,"0x%x,",protocal->packet.data[i]);
    }
    SEGGER_RTT_printf(0,"\n");
#endif
	
	protocal->id = BLE_MSG_SEND;
	u8Route 	= ((protocal->packet.att.routeMsg & 0xf0)>>4) + ( protocal->packet.att.routeMsg<<4); 
	u8FlowCtl 	= protocal->packet.att.flowControl; 
	
    switch(protocal->packet.att.load.content.interfaceIndex2)
    {
    case PROT_PROD_NAME:  //0x01:  产品名称
        
         break;
    case PROT_PROD_VER:   //0x02:  固件版本号
        u32temp = VersionRead();
        protocal->packet = PROJECT_VER_PROTOCAL;
        protocal->packet.att.routeMsg = u8Route;
		protocal->packet.att.flowControl = u8FlowCtl;
		protocal->packet.att.load.content.interfaceType = PROTOCAL_RET; 
        protocal->packet.att.load.content.parameter[0] = (uint8)(u32temp >> 16);
        protocal->packet.att.load.content.parameter[1] = (uint8)(u32temp >> 8);
        protocal->packet.att.load.content.parameter[2] = (uint8)(u32temp);
        protocal->packet.att.load.content.parameter[3] = Mid_Ble_CheckSum(&protocal->packet);
        Mid_Ble_SendMsg(protocal);
        break;
    case PROT_PROD_DATE:   //0x03: 生产日期
        
        break;
    case PROT_PROD_SOC:     //0x04:  剩余电量
    case PROT_PROD_SOC_NEW:
//    batType = Mid_Bat_SocRead(&u8temp);
    protocal->packet.att.routeMsg = u8Route;
	protocal->packet.att.flowControl = u8FlowCtl;
	protocal->packet.att.load.content.interfaceType = PROTOCAL_RET; 
    if(protocal->packet.att.load.content.interfaceIndex2 == PROT_PROD_SOC)
    {           
        protocal->packet.att.loadLength += 1;
        protocal->packet.att.load.content.parameter[0] = u8temp;   //剩余电量
        protocal->packet.att.load.content.parameter[1] = Mid_Ble_CheckSum(&protocal->packet);
    }
    else
    {
        protocal->packet.att.loadLength += 2;
        protocal->packet.att.load.content.parameter[0] = batType;  //可充电电池
        protocal->packet.att.load.content.parameter[1] = u8temp;   //剩余电量
        protocal->packet.att.load.content.parameter[2] = Mid_Ble_CheckSum(&protocal->packet);
    }
    Mid_Ble_SendMsg(protocal);
    break;

    case PROT_PROD_AUTH:    //0x05: 授权
		if(protocal->packet.att.load.content.parameter[0] == 0x00)  //apply author
		{
			//step 1:send ACK
			App_Ble_UpdateConnStatus(APP_PROTOCAL_RECVAUTHREQEST);
			Protocal_SendACK(protocal, SUCCESS);

			//step 2:set author phonestate
			phoneState.timeCnt  = 0;
			phoneState.timeMax  = APP_AuthorTimeOut;
			phoneState.state    = PHONE_STATE_AUTHOR;

			// 马达震动提示授权
			Mid_Motor_ParamSet(eMidMotorShake4Hz, 2);
			Mid_Motor_ShakeStart();			
		}
		else if(protocal->packet.att.load.content.parameter[0] == 0x01)  //enforce author
		{
			App_Ble_UpdateConnStatus(APP_PROTOCAL_RECVAUTHREQEST);
			Protocal_SendACK(protocal, SUCCESS); 
			App_Protocal_AuthorPass();
			phoneState.state = PHONE_STATE_NORMAL;  
			phoneState.timeCnt  = 0;		
		}
		else
		{
			Protocal_SendACK(protocal, PARAMATER_ERROR);
		}
    break;

    case PROT_PROD_SN:  //0x06		
    Protocal_SendACK(protocal, SUCCESS);
    App_Protocal_RetSN(protocal);
    break;

    case PROT_DATE:   //rtc 0x08
		MOD_PDU_RTT_LOG(0, "PROT_DATE \r\n");
    if (protocal->packet.att.load.content.interfaceType == PROTOCAL_SET || 
        protocal->packet.att.load.content.interfaceType == PROTOCAL_RET)
    {
        Protocal_SendACK(protocal, SUCCESS);
       appRtcTemp.year   = ((protocal->packet.att.load.content.parameter[0]>>4) & 0x0f) * 10 
                   + (protocal->packet.att.load.content.parameter[0] & 0x0f);

       appRtcTemp.month  = ((protocal->packet.att.load.content.parameter[1]>>4) & 0x0f) * 10 
                       + (protocal->packet.att.load.content.parameter[1] & 0x0f);                              

       appRtcTemp.day    = ((protocal->packet.att.load.content.parameter[2]>>4) & 0x0f) * 10 
                       + (protocal->packet.att.load.content.parameter[2] & 0x0f);

       appRtcTemp.hour   = ((protocal->packet.att.load.content.parameter[3]>>4) & 0x0f) * 10 
                       + (protocal->packet.att.load.content.parameter[3] & 0x0f);
      
       appRtcTemp.min    = ((protocal->packet.att.load.content.parameter[4]>>4) & 0x0f) * 10 
                       + (protocal->packet.att.load.content.parameter[4] & 0x0f);

       appRtcTemp.sec    = ((protocal->packet.att.load.content.parameter[5]>>4) & 0x0f) * 10 
                       + (protocal->packet.att.load.content.parameter[5] & 0x0f);

       appRtcTemp.zone   = ((uint16)protocal->packet.att.load.content.parameter[6] & 0x8000) 
                       | ((uint16)protocal->packet.att.load.content.parameter[6] << 8)
                       | ((uint16)protocal->packet.att.load.content.parameter[7]);

       appRtcTemp.week   = protocal->packet.att.load.content.parameter[8];

       Mid_Rtc_TimeWrite(&appRtcTemp);
    }else if (protocal->packet.att.load.content.interfaceType == PROTOCAL_GET)
    {
        Mid_Rtc_TimeRead(&appRtcTemp);
        protocal->packet.att.loadLength += 9;
        protocal->packet.att.routeMsg = u8Route;
        protocal->packet.att.flowControl = u8FlowCtl;
        protocal->packet.att.load.content.interfaceType = PROTOCAL_RET; 
        protocal->packet.att.load.content.parameter[0] = (appRtcTemp.year  / 10)<<4 | (appRtcTemp.year % 10);
        protocal->packet.att.load.content.parameter[1] = (appRtcTemp.month  / 10)<<4 | (appRtcTemp.month % 10);
        protocal->packet.att.load.content.parameter[2] = (appRtcTemp.day  / 10)<<4 | (appRtcTemp.day % 10);
        protocal->packet.att.load.content.parameter[3] = (appRtcTemp.hour  / 10)<<4 | (appRtcTemp.hour % 10);
        protocal->packet.att.load.content.parameter[4] = (appRtcTemp.min  / 10)<<4 | (appRtcTemp.min % 10);
        protocal->packet.att.load.content.parameter[5] = (appRtcTemp.sec  / 10)<<4 | (appRtcTemp.sec % 10);
        protocal->packet.att.load.content.parameter[6] = (uint8_t)((appRtcTemp.zone & 0x8f00) >> 8);
        protocal->packet.att.load.content.parameter[7] = (uint8_t)(appRtcTemp.zone & 0x00ff);
        protocal->packet.att.load.content.parameter[8] = appRtcTemp.week;
        protocal->packet.data[protocal->packet.att.loadLength + 5]  = Mid_Ble_CheckSum(&protocal->packet); 
        Mid_Ble_SendMsg(protocal);
    }
    break;

    case PROT_SEC_CITY_DATE: //0x09 
    if (protocal->packet.att.load.content.interfaceType == PROTOCAL_SET)
    {
        Protocal_SendACK(protocal, SUCCESS);

        WorldRtc.rtc.year   = ((protocal->packet.att.load.content.parameter[0]>>4) & 0x0f) * 10 
                   + (protocal->packet.att.load.content.parameter[0] & 0x0f);

       WorldRtc.rtc.month  = ((protocal->packet.att.load.content.parameter[1]>>4) & 0x0f) * 10 
                       + (protocal->packet.att.load.content.parameter[1] & 0x0f);                              

       WorldRtc.rtc.day    = ((protocal->packet.att.load.content.parameter[2]>>4) & 0x0f) * 10 
                       + (protocal->packet.att.load.content.parameter[2] & 0x0f);

       WorldRtc.rtc.hour   = ((protocal->packet.att.load.content.parameter[3]>>4) & 0x0f) * 10 
                       + (protocal->packet.att.load.content.parameter[3] & 0x0f);
      
       WorldRtc.rtc.min    = ((protocal->packet.att.load.content.parameter[4]>>4) & 0x0f) * 10 
                       + (protocal->packet.att.load.content.parameter[4] & 0x0f);

       WorldRtc.rtc.sec    = ((protocal->packet.att.load.content.parameter[5]>>4) & 0x0f) * 10 
                       + (protocal->packet.att.load.content.parameter[5] & 0x0f);

       WorldRtc.rtc.zone   = ((uint16)protocal->packet.att.load.content.parameter[6] & 0x8000) 
                       | ((uint16)protocal->packet.att.load.content.parameter[6] << 8)
                       | ((uint16)protocal->packet.att.load.content.parameter[7]);

       WorldRtc.rtc.week   = protocal->packet.att.load.content.parameter[8];

       WorldRtc.cityCode    = ((uint16)protocal->packet.att.load.content.parameter[9] << 8)
                            |(uint16)protocal->packet.att.load.content.parameter[10];                     
    }
    if (protocal->packet.att.load.content.interfaceType == PROTOCAL_GET)
    {
        protocal->packet.att.loadLength += 11;
        protocal->packet.att.routeMsg = u8Route;
        protocal->packet.att.flowControl = u8FlowCtl;
        protocal->packet.att.load.content.interfaceType = PROTOCAL_RET; 
        protocal->packet.att.load.content.parameter[0] = (WorldRtc.rtc.year  / 10)<<4 | (WorldRtc.rtc.year % 10);
        protocal->packet.att.load.content.parameter[1] = (WorldRtc.rtc.month  / 10)<<4 | (WorldRtc.rtc.month % 10);
        protocal->packet.att.load.content.parameter[2] = (WorldRtc.rtc.day  / 10)<<4 | (WorldRtc.rtc.day % 10);
        protocal->packet.att.load.content.parameter[3] = (WorldRtc.rtc.hour  / 10)<<4 | (WorldRtc.rtc.hour % 10);
        protocal->packet.att.load.content.parameter[4] = (WorldRtc.rtc.min  / 10)<<4 | (WorldRtc.rtc.min % 10);
        protocal->packet.att.load.content.parameter[5] = (WorldRtc.rtc.sec  / 10)<<4 | (WorldRtc.rtc.sec % 10);
        protocal->packet.att.load.content.parameter[6] = (uint8_t)((WorldRtc.rtc.zone & 0x8f00) >> 8);
        protocal->packet.att.load.content.parameter[7] = (uint8_t)(WorldRtc.rtc.zone & 0x00ff);
        protocal->packet.att.load.content.parameter[8] = WorldRtc.rtc.week;
        protocal->packet.att.load.content.parameter[9] = (uint8_t)((WorldRtc.cityCode & 0x8f00) >> 8);
        protocal->packet.att.load.content.parameter[10] = (uint8_t)(WorldRtc.cityCode& 0x00ff);
        protocal->packet.data[protocal->packet.att.loadLength + 5]  = Mid_Ble_CheckSum(&protocal->packet); 
        Mid_Ble_SendMsg(protocal);
    }          
    break; 

    case PROT_SEC_CITY_DST: //0x0D

    break;

    case PROT_ALARM_CLK: //0x10
   if (protocal->packet.att.load.content.interfaceType == PROTOCAL_SET)
   {	
	    Protocal_SendACK(protocal, SUCCESS);
       alarmconfiginfo.hour = ((protocal->packet.att.load.content.parameter[1]>>4) & 0x0f) * 10 
                            + (protocal->packet.att.load.content.parameter[1] & 0x0f);

       alarmconfiginfo.min  = ((protocal->packet.att.load.content.parameter[2]>>4) & 0x0f) * 10 
                            + (protocal->packet.att.load.content.parameter[2] & 0x0f);

       alarmconfiginfo.reptswitch  = protocal->packet.att.load.content.parameter[3];

       alarmconfiginfo.alarmswitch = protocal->packet.att.load.content.parameter[4];

       Mid_AlarmClock_Write(&alarmconfiginfo,protocal->packet.att.load.content.parameter[0]-1);
       
       //设置保存事件
//        appMsg.id   = APP_CACEH_DATA_SAVE;
//        appMsg.para = CACHE_ALARM_TYPE;
//        App_Task_EventSet(appMsg);
      
   }  
    if (protocal->packet.att.load.content.interfaceType == PROTOCAL_GET)
    {                           
        if((protocal->packet.att.load.content.parameter[0]) > 0)//ret assigned alarm
        {
            Mid_AlarmClock_Read(&alarmconfiginfo, protocal->packet.att.load.content.parameter[0]-1);
			protocal->packet.att.loadLength += 4;
			protocal->packet.att.routeMsg = u8Route;
			protocal->packet.att.flowControl = u8FlowCtl;
			protocal->packet.att.load.content.interfaceType = PROTOCAL_RET; 
            protocal->packet.att.load.content.parameter[1] = (alarmconfiginfo.hour / 10)<<4 | (alarmconfiginfo.hour % 10);
            protocal->packet.att.load.content.parameter[2] = (alarmconfiginfo.min / 10)<<4  | (alarmconfiginfo.min % 10);
            protocal->packet.att.load.content.parameter[3] = alarmconfiginfo.reptswitch;
            protocal->packet.att.load.content.parameter[4] = alarmconfiginfo.alarmswitch;
            protocal->packet.data[protocal->packet.att.loadLength + 5]  = Mid_Ble_CheckSum(&protocal->packet); 
            Mid_Ble_SendMsg(protocal);
        }
        else//all alarm
        {
            //group 1
            Mid_AlarmClock_Read(&alarmconfiginfo, 0);
			protocal->packet.att.loadLength += 5;
			protocal->packet.att.routeMsg = u8Route;
			protocal->packet.att.flowControl = u8FlowCtl;
			protocal->packet.att.load.content.interfaceType = PROTOCAL_RET; 
            protocal->packet.att.load.content.parameter[0] = 0x01;
            protocal->packet.att.load.content.parameter[1] = (alarmconfiginfo.hour / 10)<<4 | (alarmconfiginfo.hour % 10);
            protocal->packet.att.load.content.parameter[2] = (alarmconfiginfo.min / 10)<<4  | (alarmconfiginfo.min % 10);
            protocal->packet.att.load.content.parameter[3] = alarmconfiginfo.reptswitch;
            protocal->packet.att.load.content.parameter[4] = alarmconfiginfo.alarmswitch;
            protocal->packet.data[protocal->packet.att.loadLength + 5]  = Mid_Ble_CheckSum(&protocal->packet); 
            Mid_Ble_SendMsg(protocal);
            vTaskDelay(50);//delay 100 tick
            
            //group 2
            Mid_AlarmClock_Read(&alarmconfiginfo, 1);
            protocal->packet.att.load.content.parameter[0] = 0x02;
            protocal->packet.att.load.content.parameter[1] = (alarmconfiginfo.hour / 10)<<4 | (alarmconfiginfo.hour % 10);
            protocal->packet.att.load.content.parameter[2] = (alarmconfiginfo.min / 10)<<4  | (alarmconfiginfo.min % 10);
            protocal->packet.att.load.content.parameter[3] = alarmconfiginfo.reptswitch;
            protocal->packet.att.load.content.parameter[4] = alarmconfiginfo.alarmswitch;

            protocal->packet.data[protocal->packet.att.loadLength + 5]  = Mid_Ble_CheckSum(&protocal->packet);  
            Mid_Ble_SendMsg(protocal);
            vTaskDelay(50);//delay 100 tick
            
            //group 3
            Mid_AlarmClock_Read(&alarmconfiginfo, 2);
            protocal->packet.att.load.content.parameter[0] = 0x03;
            protocal->packet.att.load.content.parameter[1] = (alarmconfiginfo.hour / 10)<<4 | (alarmconfiginfo.hour % 10);
            protocal->packet.att.load.content.parameter[2] = (alarmconfiginfo.min / 10)<<4  | (alarmconfiginfo.min % 10);
            protocal->packet.att.load.content.parameter[3] = alarmconfiginfo.reptswitch;
            protocal->packet.att.load.content.parameter[4] = alarmconfiginfo.alarmswitch;

            protocal->packet.data[protocal->packet.att.loadLength + 5]  = Mid_Ble_CheckSum(&protocal->packet);  
            Mid_Ble_SendMsg(protocal);
        }   
    }               
    break;

    case PROT_WEIGHT_HEIGHT://0x0E
    if (protocal->packet.att.load.content.interfaceType == PROTOCAL_SET)
    {
        Protocal_SendACK(protocal, SUCCESS);
        bodyinfo.bodyHeight   = protocal->packet.att.load.content.parameter[0];
        bodyinfo.bodyWeight   = protocal->packet.att.load.content.parameter[1];     
        bodyinfo.age          = protocal->packet.att.load.content.parameter[2];
        bodyinfo.sex          = protocal->packet.att.load.content.parameter[3];
        Mid_SportScene_BodyInfoSet(&bodyinfo);

        //设置保存事件
//        appMsg.id   = APP_CACEH_DATA_SAVE;
//        appMsg.para = CACHE_SPORT_TYPE;
//        App_Task_EventSet(appMsg);
    }
    else if (protocal->packet.att.load.content.interfaceType == PROTOCAL_GET)
    {
        Mid_SportScene_BodyInfoRead(&bodyinfo);
        protocal->packet.att.loadLength += 4;
        protocal->packet.att.routeMsg = u8Route;
        protocal->packet.att.flowControl = u8FlowCtl;
        protocal->packet.att.load.content.interfaceType = PROTOCAL_RET; 
        protocal->packet.att.load.content.parameter[0] = bodyinfo.bodyHeight;
        protocal->packet.att.load.content.parameter[1] = bodyinfo.bodyWeight;
        protocal->packet.att.load.content.parameter[2] = bodyinfo.age;
        protocal->packet.att.load.content.parameter[3] = bodyinfo.sex;
        protocal->packet.data[protocal->packet.att.loadLength + 5]  = Mid_Ble_CheckSum(&protocal->packet);  
        Mid_Ble_SendMsg(protocal);        
    }
    else
    {
        Protocal_SendACK(protocal, PARAMATER_ERROR);
    }
    break;

    case PROT_PHONE_WEATHER: //0x10
    Protocal_SendACK(protocal, SUCCESS);
	
    tWeather.Status = (uint16)protocal->packet.att.load.content.parameter[0] <<8 
                                | protocal->packet.att.load.content.parameter[1];
    tWeather.MinTemperature =  protocal->packet.att.load.content.parameter[2]; 
    tWeather.MaxTemperature =  protocal->packet.att.load.content.parameter[3];   
    tWeather.CurTemperature =  protocal->packet.att.load.content.parameter[4];                       
	Mid_WeatherScene_TendencySet(&tWeather);
    break;
    }
    return Ret_OK;
}

//////////////////////////////////////////////////////////////////////////////////
//                       交互控制（03H, PROT_INTERACT）
//////////////////////////////////////////////////////////////////////////////////


//**********************************************************************
// 函数功能: 解析交互控制数据
// 输入参数：protocal:协议数据
// 返回参数：	
//**********************************************************************
static uint8 Analysis_Interact(ble_msg_t *protocal)
{
//    movt_task_msg_t movtMsg;
//    menuMessage message={0};
    // ble_msg_t bleMsg;
    uint8_t u8temp;
	uint8   u8Route;     //记录路由
	uint8 	u8FlowCtl;   //记录流控号
//    app_task_msg_t  appMsg;


#if(ENABLE_PRINT_DATA ==1)
    uint8 i;
    SEGGER_RTT_printf(0,"Interact Data:\n");
    for(i = 0; i < protocal->packet.att.loadLength + 6; i++)
    {
        SEGGER_RTT_printf(0,"0x%x,",protocal->packet.data[i]);
    }
    SEGGER_RTT_printf(0,"\n");
#endif
	
	protocal->id = BLE_MSG_SEND;
	u8Route 	= ((protocal->packet.att.routeMsg & 0xf0)>>4) + ( protocal->packet.att.routeMsg<<4); 
	u8FlowCtl 	= protocal->packet.att.flowControl; 
	
	switch(protocal->packet.att.load.content.interfaceIndex2)
    {
    case PROT_NEW_MSG:  //0x01: 新消息通知
    Protocal_SendACK(protocal,SUCCESS);
        appRemindState = ((uint32)protocal->packet.att.load.content.parameter[0] << 24) 
                       + ((uint32)protocal->packet.att.load.content.parameter[1] << 16)
                       + ((uint32)protocal->packet.att.load.content.parameter[2] << 8)
                       + protocal->packet.att.load.content.parameter[3];
   
        appRemindState &= systermConfig.appRemindSwitch;//屏蔽未开启的提醒
                       
        if (appRemindState && (phoneState.state != PHONE_STATE_PHOTO))//switch open systermConfig.
        {
            if(!((systermConfig.notDisturbSwitch == SWITCH_ON) && (App_NotDisturdTimeCheck())))//勿扰时间段内不提醒
            {
                if (protocal->packet.att.routeMsg == 0x21)
                {
                    u8temp = IOS;
                }
                else
                {
                    u8temp = ANDROID;
                }
                // App_NewMsgAnalysis(u8temp, appRemindState,NEW_REMIND); //新提醒
            }
        }               
        break;
    case PROT_MSG_SWITCH://0x02：通知开关
        if (protocal->packet.att.load.content.interfaceType == PROTOCAL_GET)//获取
        {
			protocal->packet.att.loadLength += 4;
			protocal->packet.att.routeMsg = u8Route;
			protocal->packet.att.flowControl = u8FlowCtl;
			protocal->packet.att.load.content.interfaceType = PROTOCAL_RET;
            protocal->packet.att.load.content.parameter[0] = (systermConfig.appRemindSwitch >> 24)& 0xff;
            protocal->packet.att.load.content.parameter[1] = (systermConfig.appRemindSwitch >> 16)& 0xff;
            protocal->packet.att.load.content.parameter[2] = (systermConfig.appRemindSwitch >> 8) & 0xff;
            protocal->packet.att.load.content.parameter[3] = systermConfig.appRemindSwitch & 0xff;
			
            protocal->packet.data[protocal->packet.att.loadLength + 5]  = Mid_Ble_CheckSum(&protocal->packet); 
			Mid_Ble_SendMsg(protocal);
        }else if (protocal->packet.att.load.content.interfaceType == PROTOCAL_SET)//设置 
        {
           systermConfig.appRemindSwitch     = ((uint32)protocal->packet.att.load.content.parameter[0] << 24) 
                                                + ((uint32)protocal->packet.att.load.content.parameter[1] << 16)
                                                + ((uint32)protocal->packet.att.load.content.parameter[2] << 8)
                                                + ((uint32)protocal->packet.att.load.content.parameter[3]);
           Protocal_SendACK(protocal,SUCCESS);

           //设置保存事件
//            appMsg.id   = APP_CACEH_DATA_SAVE;
//            appMsg.para = CACHE_SYSTERM_TYPE;
//            App_Task_EventSet(appMsg);
        }
        else
        {
            Protocal_SendACK(protocal,PARAMATER_ERROR);
        }
        break;
    case PROT_INCALL_RESP:   // 0x03: 来电通知反馈，只产品发送
        break;
    case PROT_CANCEL_MSG:    //0x04:消息取消通知 
    Protocal_SendACK(protocal,SUCCESS);
    appRemindState = ((uint32)protocal->packet.att.load.content.parameter[0] << 24) 
                   + ((uint32)protocal->packet.att.load.content.parameter[1] << 16)
                   + ((uint32)protocal->packet.att.load.content.parameter[2] << 8)
                   + protocal->packet.att.load.content.parameter[3];

    appRemindState &= systermConfig.appRemindSwitch;//屏蔽未开启的提醒
                   
    if (appRemindState)//switch open systermConfig.
    {
//        if(!((systermConfig.notDisturbSwitch == SWITCH_ON) && (App_NotDisturdTimeCheck())))//勿扰时间段内不提醒
//        {
//            if (protocal->packet.att.routeMsg == 0x21)
//            {
//                u8temp = IOS;
//            }
//            else
//            {
//                u8temp = ANDROID;
//            }
//            App_NewMsgAnalysis(u8temp, appRemindState,CANCEL_REMIND);//提醒取消
//        }
    }  
        break;
        case PROT_MSG_DETAIL_SWITCH: //0x05: 消息详情开关
        if (protocal->packet.att.load.content.interfaceType == PROTOCAL_SET)//设置 
        {
           systermConfig.appDetailRemindSwitch     = ((uint32)protocal->packet.att.load.content.parameter[0] << 24) 
                                                    + ((uint32)protocal->packet.att.load.content.parameter[1] << 16)
                                                    + ((uint32)protocal->packet.att.load.content.parameter[2] << 8)
                                                    + ((uint32)protocal->packet.att.load.content.parameter[3]);
            //设置保存事件                                        
//            appMsg.id   = APP_CACEH_DATA_SAVE;
//            appMsg.para = CACHE_SYSTERM_TYPE;
//            App_Task_EventSet(appMsg);
        }
        break;
    case PROT_MSG_DETAIL://0x06:消息详情通知
        //现在暂无消息详情开关，都用提醒开关
        systermConfig.appDetailRemindSwitch = systermConfig.appRemindSwitch;

        //Analy_Debug((0,"appDetailRemindSwitch=0x%x,remind type=%d\n",systermConfig.appDetailRemindSwitch,protocal->packet.att.load.content.parameter[0]));
        //fix 8:IOS消息开关关闭，仍可响应消息(提醒，显示详情)。
        if((systermConfig.appDetailRemindSwitch & (0x01 << protocal->packet.att.load.content.parameter[0])) == 0x0000)
        {
            return Ret_OK;
        }
        //fix 8:2018.6.20
        if (systermConfig.appDetailRemindSwitch && (phoneState.state != PHONE_STATE_PHOTO))//启动通知才进行处理
        { 
//            if(!((systermConfig.notDisturbSwitch == SWITCH_ON) && (App_NotDisturdTimeCheck())))//勿扰时间段内不提醒
//            {
//                if (protocal->packet.att.routeMsg == 0x21)
//                {
//                    u8temp = IOS;
//                }
//                else
//                {
//                    u8temp = ANDROID;
//                }
////               App_RemindManage_Process(protocal->packet,u8temp);
//            }        
        }               
        break;

         case PROT_PHOTO_MODE: //拍照
        if (protocal->packet.att.load.content.parameter[0] == 0x00)//退出
        {
//            message.state   = EXIT_TAKE_PHOTO_MODE;
//            message.op      = TAKE_PHOTO_REMIND;
//            WinDowHandle    = App_Window_Process(WinDowHandle,TAG_REMIND,message);
            phoneState.state   = PHONE_STATE_NORMAL;
            phoneState.timeCnt = 0;

        }else if (protocal->packet.att.load.content.parameter[0] == 0x01)//进入
        {
            phoneState.state   = PHONE_STATE_PHOTO;
            phoneState.timeCnt = 0;
            phoneState.timeMax = APP_TakePhotoTimeOut;

//            message.state   = ENTER_TAKE_PHOTO_MODE;
//            message.op      = TAKE_PHOTO_REMIND;
//            WinDowHandle    = App_Window_Process(WinDowHandle,TAG_REMIND,message);
        }
        Protocal_SendACK(protocal,SUCCESS);
        break;

        case PROT_PHOTO_HEARTBEAT: //拍照心跳
        if (phoneState.state == PHONE_STATE_PHOTO)
        {
            phoneState.state   = PHONE_STATE_PHOTO;
            phoneState.timeCnt = 0;
            phoneState.timeMax = APP_TakePhotoTimeOut;
        }
        break;

        case PROT_WATCH_HAND_MODE://校针:0x31
        #if 0
        if(protocal->packet.att.load.content.parameter[0] == 0x01)//进入校针
        {
			movtState.timeCnt 	= 0;
            movtState.state     = MOVT_STATE_ADJUST;
            movtMsg.id          = MOVT_MSG_MC_STOP;
            MovtTask_EventSet(movtMsg);
            Protocal_SendACK(protocal,SUCCESS);
        }
        else if(protocal->packet.att.load.content.parameter[0] == 0x00)//退出校针
        {
            Protocal_SendACK(protocal,SUCCESS);
            movtState.timeCnt 	= 0;
            movtState.state     = MOVT_STATE_NORMAL;
            movtMsg.id          = MOVT_MSG_MC_RECOVER;
            MovtTask_EventSet(movtMsg);
            App_Rtc_Movt();            
        }
        else
        {
            Protocal_SendACK(protocal, PARAMATER_ERROR);
        }
        #else
        Protocal_SendACK(protocal,SUCCESS);
        #endif
        break;

        case PROT_WATCH_HAND_CTRL:
        Protocal_SendACK(protocal, SUCCESS);
        break;

        case PROT_WATCH_HAND_PARAM: // 校针参数：0x33
        Protocal_SendACK(protocal, SUCCESS);
        #if 0
        if (movtState.state == MOVT_STATE_ADJUST)
        {
            //if(protocal->packet.att.load.content.parameter[0] == MOVT_2PIN_M) // 设置RTC时间那个表盘
             if(protocal->packet.att.load.content.parameter[0] == MOVT_3PIN_M || protocal->packet.att.load.content.parameter[0] == MOVT_2PIN_M) // 设置RTC时间那个表盘
            {
                movtMsg.id          = MOVT_MSG_MC_SET_CUR_AIM;
                movtMsg.cur         = (((uint16_t)protocal->packet.att.load.content.parameter[1] << 8)
                                     + protocal->packet.att.load.content.parameter[2])/10;
                movtMsg.aim         = movtMsg.cur;
                MovtTask_EventSet(movtMsg);           
            }
        }
        #endif
        break;

        case PROT_WATCH_HAND_HEARTBEAT: // 校针心跳包：0x34
		if(movtState.state == MOVT_STATE_ADJUST)
		{
			movtState.state   = MOVT_STATE_ADJUST;
			movtState.timeCnt = 0;
		}
        
        break;

    case PROT_LONG_SIT_SWITCH:    //0x51: 久坐
		if(protocal->packet.att.load.content.interfaceType == PROTOCAL_SET)
		{
			Protocal_SendACK(protocal,SUCCESS);
			systermConfig.longSitRemindSwitch = protocal->packet.att.load.content.parameter[0];

            //设置保存事件                                        
//            appMsg.id   = APP_CACEH_DATA_SAVE;
//            appMsg.para = CACHE_SYSTERM_TYPE;
//            App_Task_EventSet(appMsg);
		}
		else if (protocal->packet.att.load.content.interfaceType == PROTOCAL_GET)
        {
			protocal->packet.att.loadLength += 1;
			protocal->packet.att.routeMsg = u8Route;
			protocal->packet.att.flowControl = u8FlowCtl;
			protocal->packet.att.load.content.interfaceType = PROTOCAL_RET;
			protocal->packet.att.load.content.parameter[0] = systermConfig.longSitRemindSwitch;
	
			protocal->packet.data[protocal->packet.att.loadLength + 5]  = Mid_Ble_CheckSum(&protocal->packet);  
			Mid_Ble_SendMsg(protocal);
		}
        else
        {
            Protocal_SendACK(protocal,PARAMATER_ERROR);
        }
        break;
    case PROT_LONG_SIT_PARAM:     //0x52: 久坐参数
        if(protocal->packet.att.load.content.interfaceType == PROTOCAL_SET)
		{
			Protocal_SendACK(protocal,SUCCESS);
			appLongSitInfo.StartTimeHour = ((protocal->packet.att.load.content.parameter[0]>>4) & 0x0f) * 10 			
										   + (protocal->packet.att.load.content.parameter[0] & 0x0f);
			appLongSitInfo.StartTimeMin  = ((protocal->packet.att.load.content.parameter[1]>>4) & 0x0f) * 10 			
										   + (protocal->packet.att.load.content.parameter[1] & 0x0f);
			appLongSitInfo.StopTimeHour  = ((protocal->packet.att.load.content.parameter[2]>>4) & 0x0f) * 10 			
										   + (protocal->packet.att.load.content.parameter[2] & 0x0f);
			appLongSitInfo.StopTimeMin   = ((protocal->packet.att.load.content.parameter[3]>>4) & 0x0f) * 10 			
										   + (protocal->packet.att.load.content.parameter[3] & 0x0f);
            
            appLongSitInfo.DisturdStartTimehour  = ((protocal->packet.att.load.content.parameter[4]>>4) & 0x0f) * 10 			
                                                    + (protocal->packet.att.load.content.parameter[4] & 0x0f);
            appLongSitInfo.DisturdStartTimeMin   = ((protocal->packet.att.load.content.parameter[5]>>4) & 0x0f) * 10 			
                                                    + (protocal->packet.att.load.content.parameter[5] & 0x0f);
            appLongSitInfo.DisturdStopTimehour   = ((protocal->packet.att.load.content.parameter[6]>>4) & 0x0f) * 10 			
                                                    + (protocal->packet.att.load.content.parameter[6] & 0x0f);
            appLongSitInfo.DisturdStopTimeMin    = ((protocal->packet.att.load.content.parameter[7]>>4) & 0x0f) * 10 			
                                                    + (protocal->packet.att.load.content.parameter[7] & 0x0f);
            appLongSitInfo.intv_mimute           = (protocal->packet.att.load.content.parameter[8] *256) + protocal->packet.att.load.content.parameter[9];
            //fix :久坐时间计数误差较大。更新久坐提醒参数之后，清除久坐计数，重新计数
//            if(systermConfig.longSitRemindSwitch == 0x01)
//            {
//                Mid_UsualScene_SedentaryClear();
//            }

//            //设置保存事件                                        
//            appMsg.id   = APP_CACEH_DATA_SAVE;
//            appMsg.para = CACHE_TIME_TYPE;
//            App_Task_EventSet(appMsg);
            
		}else if(protocal->packet.att.load.content.interfaceType == PROTOCAL_GET)
		{
			protocal->packet.att.loadLength += 10;
			protocal->packet.att.routeMsg = u8Route;
			protocal->packet.att.flowControl = u8FlowCtl;
			protocal->packet.att.load.content.interfaceType = PROTOCAL_RET;
			protocal->packet.att.load.content.parameter[0] = (appLongSitInfo.StartTimeHour / 10)<<4 
												   | (appLongSitInfo.StartTimeHour % 10);
			protocal->packet.att.load.content.parameter[1] = (appLongSitInfo.StartTimeMin / 10)<<4 
												   | (appLongSitInfo.StartTimeMin % 10);
			protocal->packet.att.load.content.parameter[2] = (appLongSitInfo.StopTimeHour / 10)<<4 
												   | (appLongSitInfo.StopTimeHour % 10);
			protocal->packet.att.load.content.parameter[3] = (appLongSitInfo.StopTimeMin / 10)<<4 
												   | (appLongSitInfo.StopTimeMin % 10);
            
            protocal->packet.att.load.content.parameter[4] = (appLongSitInfo.DisturdStartTimehour / 10)<<4 
												   | (appLongSitInfo.DisturdStartTimehour % 10);
			protocal->packet.att.load.content.parameter[5] = (appLongSitInfo.DisturdStartTimeMin / 10)<<4 
												   | (appLongSitInfo.DisturdStartTimeMin % 10);
			protocal->packet.att.load.content.parameter[6] = (appLongSitInfo.DisturdStopTimehour / 10)<<4 
												   | (appLongSitInfo.DisturdStopTimehour % 10);
			protocal->packet.att.load.content.parameter[7] = (appLongSitInfo.DisturdStopTimeMin / 10)<<4 
												   | (appLongSitInfo.DisturdStopTimeMin % 10);
            protocal->packet.att.load.content.parameter[8] = (appLongSitInfo.intv_mimute >> 8)& 0xff;
            protocal->packet.att.load.content.parameter[9] = appLongSitInfo.intv_mimute & 0xff;
			protocal->packet.data[protocal->packet.att.loadLength + 5]  = Mid_Ble_CheckSum(&protocal->packet);  
			Mid_Ble_SendMsg(protocal);
		}
        else
        {
             Protocal_SendACK(protocal,PARAMATER_ERROR);
        }
        break;
    case PROT_DND_SWITCH:         //0x61: 勿扰开关
		if(protocal->packet.att.load.content.interfaceType == PROTOCAL_SET)
		{
			Protocal_SendACK(protocal,SUCCESS);
			systermConfig.notDisturbSwitch = protocal->packet.att.load.content.parameter[0];

            //设置保存事件                                        
//            appMsg.id   = APP_CACEH_DATA_SAVE;
//            appMsg.para = CACHE_SYSTERM_TYPE;
//            App_Task_EventSet(appMsg);
		}
		else if(protocal->packet.att.load.content.interfaceType == PROTOCAL_GET)
		{
			protocal->packet.att.loadLength += 1;
			protocal->packet.att.routeMsg = u8Route;
			protocal->packet.att.flowControl = u8FlowCtl;
			protocal->packet.att.load.content.interfaceType = PROTOCAL_RET;
			protocal->packet.att.load.content.parameter[0] = systermConfig.notDisturbSwitch;
	
			protocal->packet.data[protocal->packet.att.loadLength + 5]  = Mid_Ble_CheckSum(&protocal->packet);  
			Mid_Ble_SendMsg(protocal);
		}
        else
        {
            Protocal_SendACK(protocal,PARAMATER_ERROR);
        }		
        break;
    case PROT_DND_PARAM:          //0x62: 勿扰参数
	if(protocal->packet.att.load.content.interfaceType == PROTOCAL_SET)
	{
		Protocal_SendACK(protocal,SUCCESS);
		appNotDisturdTimeInfo.StartHour = ((protocal->packet.att.load.content.parameter[0]>>4) & 0x0f) * 10 			
									   + (protocal->packet.att.load.content.parameter[0] & 0x0f);
		appNotDisturdTimeInfo.StartMin  = ((protocal->packet.att.load.content.parameter[1]>>4) & 0x0f) * 10 			
									   + (protocal->packet.att.load.content.parameter[1] & 0x0f);
		appNotDisturdTimeInfo.StopHour  = ((protocal->packet.att.load.content.parameter[2]>>4) & 0x0f) * 10 			
									   + (protocal->packet.att.load.content.parameter[2] & 0x0f);
		appNotDisturdTimeInfo.StopMin   = ((protocal->packet.att.load.content.parameter[3]>>4) & 0x0f) * 10 			
									   + (protocal->packet.att.load.content.parameter[3] & 0x0f);

        //设置保存事件                                        
//        appMsg.id   = APP_CACEH_DATA_SAVE;
//        appMsg.para = CACHE_TIME_TYPE;
//        App_Task_EventSet(appMsg);
	}
    else if(protocal->packet.att.load.content.interfaceType == PROTOCAL_GET)
	{
		protocal->packet.att.loadLength += 4;
		protocal->packet.att.routeMsg = u8Route;
		protocal->packet.att.flowControl = u8FlowCtl;
		protocal->packet.att.load.content.interfaceType = PROTOCAL_RET;
		protocal->packet.att.load.content.parameter[0] = (appNotDisturdTimeInfo.StartHour / 10)<<4 
		                                       | (appNotDisturdTimeInfo.StartHour % 10);
		protocal->packet.att.load.content.parameter[1] = (appNotDisturdTimeInfo.StartMin / 10)<<4 
		                                       | (appNotDisturdTimeInfo.StartMin % 10);
		protocal->packet.att.load.content.parameter[2] = (appNotDisturdTimeInfo.StopHour / 10)<<4 
		                                       | (appNotDisturdTimeInfo.StopHour % 10);
		protocal->packet.att.load.content.parameter[3] = (appNotDisturdTimeInfo.StopMin / 10)<<4 
		                                       | (appNotDisturdTimeInfo.StopMin % 10);
		protocal->packet.data[protocal->packet.att.loadLength + 5]  = Mid_Ble_CheckSum(&protocal->packet);  
        Mid_Ble_SendMsg(protocal);
	}
    else
    {
        Protocal_SendACK(protocal,PARAMATER_ERROR);
    }
    break;

    case PROT_SYS_SETTING:  //0x91系统设置
    if(protocal->packet.att.load.content.interfaceType == PROTOCAL_SET)
    {
        Protocal_SendACK(protocal,SUCCESS);
        if (protocal->packet.att.load.content.parameter[0] == 0)
        {
            systermConfig.systermLanguge = SYS_CHINESE_TYPE;
        }
        else
        {
            systermConfig.systermLanguge = SYS_ENGLISH_TYPE;
        }  
        systermConfig.systermTimeType = protocal->packet.att.load.content.parameter[1];

        //设置保存事件                                        
//        appMsg.id   = APP_CACEH_DATA_SAVE;
//        appMsg.para = CACHE_SYSTERM_TYPE;
//        App_Task_EventSet(appMsg);

    }else if (protocal->packet.att.load.content.interfaceType == PROTOCAL_GET)
    {
        protocal->packet.att.loadLength += 2;
        protocal->packet.att.routeMsg = u8Route;
        protocal->packet.att.flowControl = u8FlowCtl;
        protocal->packet.att.load.content.interfaceType = PROTOCAL_RET;
        if (systermConfig.systermLanguge == SYS_CHINESE_TYPE)
        {
            protocal->packet.att.load.content.parameter[0] = 0x00;
        }
        else
        {
            protocal->packet.att.load.content.parameter[0] = 0x01;
        }  
        protocal->packet.att.load.content.parameter[1] = systermConfig.systermTimeType;
        protocal->packet.data[protocal->packet.att.loadLength + 5]  = Mid_Ble_CheckSum(&protocal->packet);  
        Mid_Ble_SendMsg(protocal);
    }
    else
    {
        Protocal_SendACK(protocal,PARAMATER_ERROR);
    }
    break;

    default:
        break;
    }
return Ret_OK;
}

//////////////////////////////////////////////////////////////////////////////////
//                       设备测试（04H, PROT_DEV_TEST）
//////////////////////////////////////////////////////////////////////////////////


//**********************************************************************
// 函数功能: 解析交互控制数据
// 输入参数：bleMsg:协议数据
// 返回参数：	
//**********************************************************************
static uint8 Analysis_DevTest(ble_msg_t *protocal)
{
    return Ret_OK;

}

//////////////////////////////////////////////////////////////////////////////////
//                       传感器（05H, PORT_SENSOR）
//////////////////////////////////////////////////////////////////////////////////


//**********************************************************************
// 函数功能: 解析交互控制数据
// 输入参数：bleMsg:协议数据
// 返回参数：	
//**********************************************************************
static uint8 Analysis_Sensor(ble_msg_t *protocal)
{
    return Ret_OK;

}

//////////////////////////////////////////////////////////////////////////////////
//                       升级（06H, PROT_UPDATE）
//////////////////////////////////////////////////////////////////////////////////


//**********************************************************************
// 函数功能: OTA升级
// 输入参数：bleMsg:协议数据
// 返回参数：	
//**********************************************************************
static uint8 Analysis_Update(ble_msg_t *protocal)
{
    return Ret_OK;
}

//////////////////////////////////////////////////////////////////////////////////
//                   日常运动场景（10H, PROT_DAILY_SPORT_SCENE）
//////////////////////////////////////////////////////////////////////////////////


//**********************************************************************
// 函数功能: 解析交互控制数据
// 输入参数：protocal:协议数据
// 返回参数：	
//**********************************************************************
static uint8 Analysis_Sport(ble_msg_t *protocal)
{
    uint32  u32temp;
    uint8   u8Route;     //记录路由
    ble_msg_t bleMsg;
    stepSportInfo_s  sportInfoTemp;
//    app_task_msg_t  appMsg;

#if(ENABLE_PRINT_DATA ==1)
    uint8 i;
    SEGGER_RTT_printf(0,"Sport Data:\n");
    for(i = 0; i < protocal->packet.att.loadLength + 6; i++)
    {
        SEGGER_RTT_printf(0,"0x%x,",protocal->packet.data[i]);
    }
    SEGGER_RTT_printf(0,"\n");
#endif
    
    u8Route     = ((protocal->packet.att.routeMsg & 0xf0)>>4) + ( protocal->packet.att.routeMsg<<4); 
	bleMsg.packet = (protocal->packet);
	bleMsg.id     = BLE_MSG_SEND;
    bleMsg.packet.att.routeMsg = u8Route;
	
    switch(protocal->packet.att.load.content.interfaceIndex2)
    {
        case PROT_SCENE_DS_MODE:
        if (protocal->packet.att.load.content.parameter[0] == 0x01) //进入日常运动场景　
        {
            /* code */
        }
        else
        {

        }
        break;

        case PROT_SCENE_DS_GOAL:
        if(protocal->packet.att.load.content.interfaceType == PROTOCAL_SET)     //set
        {
            u32temp     = 0;
            u32temp    = ((uint32_t)protocal->packet.att.load.content.parameter[0] << 8)
                            + protocal->packet.att.load.content.parameter[1];
            Mid_SportScene_StepAimSet(u32temp);
            Protocal_SendACK(protocal, SUCCESS);

             //设置保存事件                                        
//            appMsg.id   = APP_CACEH_DATA_SAVE;
//            appMsg.para = CACHE_SPORT_TYPE;
//            App_Task_EventSet(appMsg);
        }
        else if(protocal->packet.att.load.content.interfaceType == PROTOCAL_GET)//get
        {
            Mid_SportScene_SportInfoRead(&sportInfoTemp); 
            bleMsg.packet.att.loadLength += 2;
            bleMsg.packet.att.load.content.interfaceType = PROTOCAL_RET; 
            bleMsg.packet.att.load.content.parameter[0] = (sportInfoTemp.stepAim >> 8)& 0xff;
            bleMsg.packet.att.load.content.parameter[1] = (uint8)sportInfoTemp.stepAim;
            bleMsg.packet.data[bleMsg.packet.att.loadLength + 5]  = Mid_Ble_CheckSum(&bleMsg.packet); 
            Mid_Ble_SendMsg(&bleMsg);
        } 
        else
        {
            Protocal_SendACK(protocal, PARAMATER_ERROR);
        }            
        break;
        
        case PROT_SCENE_DS_TOTAL_APP_STEPS:
        u32temp = 0;
        Mid_SportScene_StepRead(&u32temp); 
        bleMsg.packet.att.loadLength += 2;
        bleMsg.packet.att.load.content.interfaceType = PROTOCAL_RET; 
        bleMsg.packet.att.load.content.parameter[0] = (u32temp >> 8) & 0xff;
        bleMsg.packet.att.load.content.parameter[1] = (uint8)u32temp;
        bleMsg.packet.data[bleMsg.packet.att.loadLength + 5]  = Mid_Ble_CheckSum(&bleMsg.packet); 
        Mid_Ble_SendMsg(&bleMsg);
        break;
        
        case PROT_SCENE_DS_HIS_STEPS:
        break;
        
        case PROT_SCENE_DS_HIS_STEPS_TRANS:
        break;  
    }               
    return Ret_OK;
}

//////////////////////////////////////////////////////////////////////////////////
//                       登山场景（11H）
//////////////////////////////////////////////////////////////////////////////////


//**********************************************************************
// 函数功能: 解析交互控制数据
// 输入参数：protocal:协议数据
// 返回参数：	
//**********************************************************************
static uint8 Analysis_Climbing(ble_msg_t *protocal)
{
    return Ret_OK;

}

//////////////////////////////////////////////////////////////////////////////////
//                       跑步场景（12H）
//////////////////////////////////////////////////////////////////////////////////


//**********************************************************************
// 函数功能: 解析交互控制数据
// 输入参数：protocal:协议数据
// 返回参数：    
//**********************************************************************
static uint8 Analysis_RunningScene(ble_msg_t *protocal)
{
    return Ret_OK;

}

//////////////////////////////////////////////////////////////////////////////////
//                       游泳场景（13H）
//////////////////////////////////////////////////////////////////////////////////


//**********************************************************************
// 函数功能: 解析交互控制数据
// 输入参数：protocal:协议数据
// 返回参数：    
//**********************************************************************
static uint8 Analysis_SwingScene(ble_msg_t *protocal)
{
    return Ret_OK;

}

//////////////////////////////////////////////////////////////////////////////////
//                      睡眠场景（14H）
//////////////////////////////////////////////////////////////////////////////////


//**********************************************************************
// 函数功能: 解析交互控制数据
// 输入参数：protocal:协议数据
// 返回参数：    
//**********************************************************************
static uint8 Analysis_SleepScene(ble_msg_t *protocal)
{
#if(ENABLE_PRINT_DATA ==1)
    uint8 i;
    SEGGER_RTT_printf(0,"DeviceInfo Data:\n");
    for(i = 0; i < protocal->packet.att.loadLength + 6; i++)
    {
        SEGGER_RTT_printf(0,"0x%x,",protocal->packet.data[i]);
    }
    SEGGER_RTT_printf(0,"\n");
#endif

    switch(protocal->packet.att.load.content.interfaceIndex2)
    {
        case PORT_SLEEP_USAUL_PARA://0x01：
        break;

        case PORT_SLEEP_GOAD_PARA://0x02
        break;

        case PORT_SLEEP_HISTORY://0x03
//        App_SleepData_DataRequestTotalInfoAck(protocal->packet.att.flowControl);
        break;

        case PORT_SLEEP_GET_RECORD://0x04 
//        App_SleepData_DataCatalogInfoRead(protocal->packet.att.load.content.parameter[0], protocal->packet.att.flowControl);
        break;

        default:
        break;
    }

    return Ret_OK;
}

//////////////////////////////////////////////////////////////////////////////////
//                      睡眠场景（15H）
//////////////////////////////////////////////////////////////////////////////////


//**********************************************************************
// 函数功能: 解析交互控制数据
// 输入参数：protocal:协议数据
// 返回参数：    
//**********************************************************************
static uint8 Analysis_HeartScene(ble_msg_t *protocal)
{
    //uint8   u8temp;      //记录中间值
    uint8   u8Route;     //记录路由
    //uint8   u8FlowCtl;   //记录流控号
    ble_msg_t bleMsg;
//    multimodule_task_msg_t msg;


#if(ENABLE_PRINT_DATA ==1)
    uint8 i;
    SEGGER_RTT_printf(0,"Sport Data:\n");
    for(i = 0; i < protocal->packet.att.loadLength + 6; i++)
    {
        SEGGER_RTT_printf(0,"0x%x,",protocal->packet.data[i]);
    }
    SEGGER_RTT_printf(0,"\n");
#endif
    
    u8Route       = ((protocal->packet.att.routeMsg & 0xf0)>>4) + ( protocal->packet.att.routeMsg<<4); 
    bleMsg.packet = (protocal->packet);
    bleMsg.id     = BLE_MSG_SEND;
    bleMsg.packet.att.routeMsg = u8Route;
    
    switch(protocal->packet.att.load.content.interfaceIndex2)
    {
        case PROT_MEASURE_DATA_HRS_GET:
        #if 0 //暂不开放app心率场景
        u8temp = 0;
        Mid_Hrm_Read(&u8temp);
        bleMsg.packet.att.loadLength += 1;
        bleMsg.packet.att.load.content.interfaceType = PROTOCAL_RET; 
        bleMsg.packet.att.load.content.parameter[0] = u8temp;
        bleMsg.packet.data[bleMsg.packet.att.loadLength + 5]  = Mid_Ble_CheckSum(&bleMsg.packet); 
        Mid_Ble_SendMsg(&bleMsg);
        #endif
        break;

        case PROT_MEASURE_DATA_HRS_START:
        Protocal_SendACK(protocal, SUCCESS);
        #if 0//暂不开放app心率场景
        if (phoneState.state != PHONE_STATE_HRM)
        {
            //设置心率场景超时
            phoneState.state = PHONE_STATE_HRM;
            phoneState.timeCnt  = 0;
            phoneState.timeMax = APP_Hrm_TimeOut;

            //启动实时心率检测
            msg.id                        = HEARTRATE_ID;       
            msg.module.hrmEvent.id        = HRM_START;
            MultiModuleTask_EventSet(msg);
        }
        #endif    
        break;

        default:
        break;
    }                

    return Ret_OK;
}


//////////////////////////////////////////////////////////////////////////////////
//                    数据包传输（f0H, PROT_PACK_TRANS）
//////////////////////////////////////////////////////////////////////////////////


//**********************************************************************
// 函数功能: 解析交互控制数据
// 输入参数：protocal:协议数据
// 返回参数：	
//**********************************************************************
static uint8 Analysis_PackTrans(ble_msg_t *protocal)
{
    switch(protocal->packet.att.routeMsg)
    {
        case BLE_TO_MCU:
        break;

        case APP_TO_MCU:
        switch(protocal->packet.att.load.content.interfaceIndex2)
        {
            case PORT_PACK_TRANS_GET_CONTENT_COUNT://0x10：获取目录条数总数据长度
//            App_PackData_DataRequestTotalInfoAck(((uint32_t)(protocal->packet.att.load.content.parameter[0]) << 8) + protocal->packet.att.load.content.parameter[1],
//                                            protocal->packet.att.flowControl);
            break;

            case PORT_PACK_TRANS_GET_CONTENT_INFO://0x11：已获知总目录情况下，指定目录序号及数据类型，获取指定序号目录的信息
//            App_PackData_DataCatalogInfoRead(((uint32_t)(protocal->packet.att.load.content.parameter[0]) << 8) + protocal->packet.att.load.content.parameter[1],
//                                    ((uint32_t)(protocal->packet.att.load.content.parameter[2]) << 8) + protocal->packet.att.load.content.parameter[3],
//                                    protocal->packet.att.flowControl);
            break;

            case PORT_PACK_TRANS_GET_PKG://0x12：请求上传指定（UTC）目录所索引的数据段（添加包序号检测连续性）
//            App_PackData_DataRequestData(((uint32_t)(protocal->packet.att.load.content.parameter[0]) << 8) + protocal->packet.att.load.content.parameter[1],
//                                ((uint32_t)(protocal->packet.att.load.content.parameter[2]) << 24) + ((uint32_t)(protocal->packet.att.load.content.parameter[3]) << 16) +
//                                ((uint32_t)(protocal->packet.att.load.content.parameter[4]) << 8) + protocal->packet.att.load.content.parameter[5],
//                                ((uint32_t)(protocal->packet.att.load.content.parameter[6]) << 8) + protocal->packet.att.load.content.parameter[7],
//                                protocal->packet.att.flowControl);
            break;


            case PORT_PACK_TRANS_AUTO_UPDATE_CONTENT_COUNT:
            case PORT_PACK_TRANS_AUTO_UPDATE_CONTENT_INFO:
            break;


            case PORT_PACK_TRANS_DEL_ALL://0x30 :删除所有数据
            Protocal_SendACK(protocal,SUCCESS);
//            App_PackData_DataTotalDelete(protocal->packet.att.flowControl);
            break;


            case PORT_PACK_TRANS_DEL_DATATYPE://0x31 ： 删除指定类型的数据
            Protocal_SendACK(protocal,SUCCESS);
//            App_PackData_DataDeleteClassify(((uint32_t)(protocal->packet.att.load.content.parameter[0]) << 8) + protocal->packet.att.load.content.parameter[1],
//                                    protocal->packet.att.flowControl);
            break;


            case PORT_PACK_TRANS_DEL_CONTENT_UTC://0x32： 按UTC删除数据
            Protocal_SendACK(protocal,SUCCESS);
//            App_PackData_DataDeleteCatalog(((uint32_t)(protocal->packet.att.load.content.parameter[0]) << 8) + protocal->packet.att.load.content.parameter[1],
//                                ((uint32_t)(protocal->packet.att.load.content.parameter[2]) << 24) + ((uint32_t)(protocal->packet.att.load.content.parameter[3]) << 16)+
//                                ((uint32_t)(protocal->packet.att.load.content.parameter[4]) << 8) + protocal->packet.att.load.content.parameter[5],
//                                protocal->packet.att.flowControl);
            break;
        }
        break;
    default:
        break;
    }
    return Ret_OK;

}

//////////////////////////////////////////////////////////////////////////////////
//                    数据包传输（f0H, PROT_PACK_TRANS）
//////////////////////////////////////////////////////////////////////////////////


//**********************************************************************
// 函数功能: 解析交互控制数据
// 输入参数：protocal:协议数据
// 返回参数：    
//**********************************************************************
static uint8 Analysis_ScenePackTrans(ble_msg_t *protocal)
{
    switch(protocal->packet.att.routeMsg)
    {
        case BLE_TO_MCU:
        break;

        case APP_TO_MCU:
        switch(protocal->packet.att.load.content.interfaceIndex2)
        {
            case PORT_SCENE_PACK_TRANS_GET_CONTENT_COUNT://0x10：获取目录条数总数据长度
//            App_SceneData_DataRequestTotalInfoAck(((uint32_t)(protocal->packet.att.load.content.parameter[0]) << 8) + protocal->packet.att.load.content.parameter[1],
//                                            protocal->packet.att.flowControl);
            break;

            case PORT_SCENE_PACK_TRANS_GET_CONTENT_INFO://0x11：已获知总目录情况下，指定目录序号及数据类型，获取指定序号目录的信息 
//			App_SceneData_DataCatalogInfoRead(((uint32_t)(protocal->packet.att.load.content.parameter[0]) << 8) + protocal->packet.att.load.content.parameter[1],
//									((uint32_t)(protocal->packet.att.load.content.parameter[2]) << 8) + protocal->packet.att.load.content.parameter[3],
//									protocal->packet.att.flowControl);
            break;

            case PORT_SCENE_PACK_TRANS_GET_PKG://0x12：请求上传指定（UTC）目录所索引的数据段（添加包序号检测连续性） 
//            App_SceneData_DataRequestData(((uint32_t)(protocal->packet.att.load.content.parameter[0]) << 8) + protocal->packet.att.load.content.parameter[1],
//                                ((uint32_t)(protocal->packet.att.load.content.parameter[2]) << 24) + ((uint32_t)(protocal->packet.att.load.content.parameter[3]) << 16) +
//                                ((uint32_t)(protocal->packet.att.load.content.parameter[4]) << 8) + protocal->packet.att.load.content.parameter[5],
//                                ((uint32_t)(protocal->packet.att.load.content.parameter[6]) << 8) + protocal->packet.att.load.content.parameter[7],
//                                protocal->packet.att.flowControl);
            break;

            case PORT_SCENE_PACK_TRANS_AUTO_UPDATE_CONTENT_COUNT:
            case PORT_SCENE_PACK_TRANS_AUTO_UPDATE_CONTENT_INFO:
            break;

            case PORT_SCENE_PACK_TRANS_DEL_ALL://0x30 :删除所有数据
            Protocal_SendACK(protocal,SUCCESS);
            break;


            case PORT_SCENE_PACK_TRANS_DEL_DATATYPE://0x31 ： 删除指定类型的数据
            Protocal_SendACK(protocal,SUCCESS);
            break;


            case PORT_SCENE_PACK_TRANS_DEL_CONTENT_UTC://0x32： 按UTC删除数据
            Protocal_SendACK(protocal,SUCCESS);
//            App_SceneData_DataDeleteCatalog(((uint32_t)(protocal->packet.att.load.content.parameter[0]) << 8) + protocal->packet.att.load.content.parameter[1],
//                                ((uint32_t)(protocal->packet.att.load.content.parameter[2]) << 24) + ((uint32_t)(protocal->packet.att.load.content.parameter[3]) << 16)+
//                                ((uint32_t)(protocal->packet.att.load.content.parameter[4]) << 8) + protocal->packet.att.load.content.parameter[5],
//                                protocal->packet.att.flowControl);
            break;
        }
        break;
    default:
        break;
    }
    return Ret_OK;

}


//fix 3: 蓝牙连接和机芯app连接不同步问题，需要重启蓝牙广播
//**********************************************************************
// 函数功能: 重启蓝牙广播
// 输入参数：    
// 返回参数：
//**********************************************************************
static void App_Protocal_ResetAdv(void)
{
    uint8 u8BleConnStatus;
    static uint8 u8ConnExpFlag = FALSE;//标示连接异常标志
    static uint32 u32ConnTimeout = 0;

    //step 1:获取蓝牙链路连接状态
    BLE_Stack_Interact(BLE_STATUS_GET,&u8BleConnStatus,0x00); //获取蓝牙连接状态

    //step 2:判断连接是否出现上下层不同步
    //上下层连接不同步条件: 蓝牙已连接上，app是Disconnect状态
    if((u8BleConnStatus == BT_CONNECTED) && (eAppProBleConn == APP_PROTOCAL_DISCONNECTED) && (u8ConnExpFlag == FALSE))
    {
        u32ConnTimeout = SMOS_GetTickCount();
        u8ConnExpFlag = TRUE;  //检测到上下层连接不同步
        Err_Info((0,"Error: App check that connect is no same\n"));
    }

    //step 2-1:上下层连接同步，reset连接异常标志
    if(((u8BleConnStatus == BT_CONNECTED) && (eAppProBleConn >= APP_PROTOCAL_ANCSDISCOVER)) ||
        ((u8BleConnStatus == BT_ADV_ON) && (eAppProBleConn == APP_PROTOCAL_DISCONNECTED)))
    {
        //Err_Info((0,"warning: reset connect flag\n"));
        u8ConnExpFlag = FALSE;   //复位连接异常标志
    }

    //step 3:上层和蓝牙连接不同步一:蓝牙连接上，app未连接，重启蓝牙广播
    //重启蓝牙广播条件:出现上下层连接不同步，并且持续10s
    if((u8ConnExpFlag == TRUE) && ((SMOS_GetTickCount() - u32ConnTimeout) >= BLE_CONNECT_ERROR_TIMEOUT))
    {
        uint8 u8adv;

        Err_Info((0,"Error: connect is wrong,need resart BLE ADV\n"));
        //step 3-1: 关闭广播
        u8adv = BT_ADV_OFF;
        BLE_Stack_Interact(BLE_STATUS_SET,&u8adv,0);

        //step 3-2: 开启广播
        u8adv = BT_ADV_ON;
        BLE_Stack_Interact(BLE_STATUS_SET,&u8adv,0);
        u8ConnExpFlag = FALSE;     //复位连接异常标志
    }

    //step 4:上层和蓝牙连接不同步二:app连接上，蓝牙是断开的情况，需要复位连接显示状态
    //  更新app连接状态为断开，此情况一般会在中断，或时间片到切换task的时候发生
    if((u8BleConnStatus == BT_ADV_ON) && (eAppProBleConn >= APP_PROTOCAL_ANCSDISCOVER))
    {        
        App_Ble_UpdateConnStatus(APP_PROTOCAL_DISCONNECTED); //蓝牙断开
        App_BleStatus_Change((uint8)BT_ADV_ON);
    }
    //step 3 和step 4:是为了同步上层和蓝牙连接上的不同步
}
//fix: 2018.6.6

//fix 4:增加OTA过程中各状态的反馈
//**********************************************************************
// 函数功能: OTA接收数据超时
// 输入参数：u8status:连接状态
// 返回参数：
//**********************************************************************
static uint8 App_Ble_CheckRecvTimeout(void)
{
    if(BLE_Stack_GetMode() == BLE_MODE_OTA)
    {
//        menuMessage message;

        if((SMOS_GetTickCount() - u32OtaRecvTimeout) >= BLE_OTA_RECV_TIMEOUT)
        {
            //fix 6: system will go back normal mode, due to ota fail.
            BLE_Stack_SetMode(BLE_MODE_NORMAL);
            //fix: 2018.6.12

            //step 1: exit ota mode
//            App_OtaSystermRecover();
//            message.state = OTA_EXIT;
//            message.op    = OTA_REMIND;
//            WinDowHandle  = App_Window_Process(WinDowHandle,TAG_REMIND,message);
        }

        return TRUE;
    }
    return FALSE;
}

//**********************************************************************
// 函数功能: 更新OTA状态
// 输入参数：u8status:连接状态
// 返回参数：
// 说    明: 具体实现在 App_Protocal_OTAStatus中
//**********************************************************************
static void App_Ble_UpdateOTAStatus(Ble_Ota_Status status)
{
    if(status == BLE_OTA_START)   //OTA升级开始,屏幕显示"升级中..."
    {
        //fix 6: system will be in OTA mode, other module will stop working.
        BLE_Stack_SetMode(BLE_MODE_OTA);
        //fix: 2018.6.12
        //fix 7:从开始OTA的时候开始计时，有些手机出现接收数据就超时
        u32OtaRecvTimeout = SMOS_GetTickCount();
        //fix 7:2018.6.13

        //step :不响应其他的功能，屏幕显示
//        App_BleOTA_Cb(0x01);
    }
    else if(status == BLE_OTA_RECV)   //OTA升级接收数据，接收超时处理
    {
        u32OtaRecvTimeout = SMOS_GetTickCount();
    }
    else if(status == BLE_OTA_RESET)   //OTA升级复位
    {
//        App_BleOTA_Cb(0x02);
    }
    else if(status == BLE_OTA_ERROR)   //OTA升级失败,屏幕退出显示升级提示
    {
        //fix 6: system will go back normal mode, due to ota fail.
        BLE_Stack_SetMode(BLE_MODE_NORMAL);
        //fix: 2018.6.12
        
        //step : exit ota mode
//        App_BleOTA_Cb(0x00);
    }
    else
        ;
}
//fix: 2018.6.7

//**********************************************************************
// 函数功能: 蓝牙状态发生改变通知
// 输入参数：echo:通知状态类型，u8Status:状态
// 返回参数：
//**********************************************************************
static void App_Ble_Echo(Ble_Echo_Type echo,uint32 u32Status,uint8 u8Option)
{
//     menuMessage         message;

    if(echo == BLE_CONN_STATUS_ECHO)
    {
        //u32Status:蓝牙状态；u8Option:蓝牙协议栈错误信息
        Analy_Debug((0,"App_Ble_Echo =%d,reason=0x%x\n",u32Status,u8Option));
        //fix 1:只有在机芯app连接上的状态下，才更新蓝牙连接断开的状态。
        //      以避免在手机与机芯连接过程中出现连接不稳定，回调的异常断开状态
        if((eAppProBleConn > APP_PROTOCAL_DISCONNECTED) && (u32Status == BT_ADV_ON))
        {
            if(eAppProBleConn >= APP_PROTOCAL_ANCSDISCOVER)
            {
//                App_BleDisConnCb();
            }

            App_Ble_UpdateConnStatus(APP_PROTOCAL_DISCONNECTED); //蓝牙断开
        }
        //fix 1: 2018.6.1
    }
    else if(echo == BLE_PAIRING_PASSKEY_ECHO)
    {
//        Analy_Debug((0,"paring pass key =%d\n",u32Status));
        //显示蓝牙配对pin码(passkey)
//        MultiModuleTask_EventSet(PAIRE_MOTO);
//		phoneState.timeCnt  = 0;
//		phoneState.timeMax  = APP_PaireTimeOut;
//        phoneState.state    = PHONE_STATE_PAIRE;
//        message.val   = u32Status;
//        message.state = ENTER_PAIRE_STATE;
//        message.op    = PAIRE_REMIND;
//        WinDowHandle  = App_Window_Process(WinDowHandle,TAG_REMIND,message);
    }
    else if(echo == BLE_PAIRING_AUTH_STATUS)
    {
//        Analy_Debug((0,"paring auth result =%d\n",u32Status));
//        //u32Status = 0x00:配对成功；=0x01:配对失败
//        message.val   = u32Status;
//        message.state = EXIT_PAIRE_STATE;
//        message.op    = PAIRE_REMIND;
//        WinDowHandle  = App_Window_Process(WinDowHandle,TAG_REMIND,message);

        //fix 3:蓝牙连接和机芯app连接同步问题
        if(u32Status == 0x00)
        {
            //配对成功，连接成功
            App_Ble_UpdateConnStatus(APP_PROTOCAL_CONNECTED); //蓝牙连接
        }
        else
        {
            //配对失败，连接自动断开
            App_Ble_UpdateConnStatus(APP_PROTOCAL_DISCONNECTED); //蓝牙断开
        }
        //fix
    }
    else if(echo == BLE_OTA_ECHO)      //OTA升级状态处理
    {
        Analy_Debug((0,"OTA =%d\n",u32Status));
        //fix 4:增加OTA过程中各状态的反馈
        App_Ble_UpdateOTAStatus((Ble_Ota_Status)u32Status);
        //fix: 2018.6.7
    }
    else if(echo == BLE_ANCS_DISCOVERING)
    {
        //fix 3:ANCS复位发现，解决手机只开蓝牙，不开应用APP，也可以支持ANCS功能
        App_Ble_UpdateConnStatus(APP_PROTOCAL_ANCSDISCOVER);

        //fix:IOS手机ANCS功能不需要app支持，收到此事件，蓝牙显示连接状态
        bleState = BLE_CONNECT;   // just for ancs
    }
    else
        ;
}

//**********************************************************************
// 函数功能: 协议解析主函数
// 输入参数：protocal:协议数据
// 返回参数：
//**********************************************************************
static void Protocal_Analysis(ble_msg_t *protocal)
{   
    if(protocal->packet.att.loadLength == 1)
    {
        // 返回ACK，删除重发队列
        Mid_Ble_DelResendMsg(&protocal->packet);
        return ;
    }

    switch(protocal->packet.att.load.content.interfaceIndex1)
    {
    case PROT_LINK:              //0x00:链路管理
        Analysis_Link(protocal);
        break;
    case PROT_DEV_INFO:          //0x01:设备信息
        Analysis_DeviceInfo(protocal);
        break;
    case PROT_INTERACT:          //0x03:交互控制
        Analysis_Interact(protocal);
        break;
    case PROT_DEV_TEST:          //0x04:设备测试
        Analysis_DevTest(protocal);    
        break;
    case PROT_SENSOR:            //0x05:传感器
        Analysis_Sensor(protocal); 
        break;
    case PROT_UPDATE:            //0x06:升级
        Analysis_Update(protocal);
        break;
    case PROT_APP_SPORT_SCENE:   //0x10:日常运动场景
        Analysis_Sport(protocal);
        break;
    case PROT_MOUNTAINEER_SCENE: //0x11:登山场景
        Analysis_Climbing(protocal);
        break;
    case PROT_RUNNING_SCENE: //0x12:跑步场景
        Analysis_RunningScene(protocal);
        break;
    case PROT_SWIMING_SCENE: //0x13:游泳场景
        Analysis_SwingScene(protocal);
        break;
    case PROT_SLEEP_SCENE: //0x14:睡眠场景
        Analysis_SleepScene(protocal);
        break;
    case PROT_HRS_SCENE: //0x15:心率场景
        Analysis_HeartScene(protocal);
        break;
    case PROT_PACK_TRANS:        //0xf0:数据包传输
        Analysis_PackTrans(protocal);
        break;

    case PROT_SCENE_PACK_TRANS:        //0xf2:场景数据包传输
    Analysis_ScenePackTrans(protocal);
    break;

    case PROT_EVENT:             //0x02: 暂无定义
    default:
        break;
    }
}

//**********************************************************************
// 函数功能: OTA状态处理
// 输入参数：u16param: = 0x00:退出OTA界面
//           = 0x01: 进入OTA界面
//           = 0x02: OTA结束复位
// 返回参数：
//**********************************************************************
void App_Protocal_OTAStatus(uint16 u16param)
{
//    menuMessage  message;

    if(u16param == 0x01)   //进入OTA界面
    {
//        App_OtaSystermSuspend();
//        message.state = OTA_ENTER;
//        message.op    = OTA_REMIND;
//        WinDowHandle  = App_Window_Process(WinDowHandle,TAG_REMIND,message);
    }
    else if(u16param == 0x00)  //退出OTA界面
    {
//        App_OtaSystermRecover();
//        message.state = OTA_EXIT;
//        message.op    = OTA_REMIND;
//        WinDowHandle  = App_Window_Process(WinDowHandle,TAG_REMIND,message);
    }
    else if(u16param == 0x02)  //OTA结束复位
    {
//        //step 1:write OTA reset flag to flash
//        App_WriteResetFlag(OTA_RESET);
//        //step 1: 在mcu复位之前保存数据
//        //缓存数据存储
//        App_CacheDataSaveAll();    
//        
//        //step 2: 复位MCU
//        SMDrv_SYS_Rest_POI();
    }
    else
        ;
}

//**********************************************************************
// 函数功能: 蓝牙断开时设置断开状态，退出OTA升级界面
// 输入参数：
// 返回参数：
//**********************************************************************
void App_Protocal_BleDisConn(void)
{
//    menuMessage  message;

    //fix 5: 蓝牙异常断开，出现卡死的情况,开断开提醒
    if(BLE_Stack_GetMode() == BLE_MODE_OTA)
    {
        //fix 6: system will go back normal mode, due to ota fail.
        BLE_Stack_SetMode(BLE_MODE_NORMAL);
        //fix 6: 2018.6.12
    
        //step : exit ota mode
//        App_OtaSystermRecover();
//        message.state = OTA_EXIT;
//        message.op    = OTA_REMIND;
//        WinDowHandle  = App_Window_Process(WinDowHandle,TAG_REMIND,message);
    }

    App_BleStatus_Change((uint8)BT_ADV_ON);
    //fix 5:  2018.6.9
}

//**********************************************************************
// 函数功能: 蓝牙连接状态，OTA升级接收超时监测
// 输入参数：    
// 返回参数：
//**********************************************************************
void App_Protocal_Monitor(void)
{
    //step 1: 蓝牙连接状态监测
    //App_Protocal_ResetAdv();

    //step 2: OTA接收数据超时处理
    App_Ble_CheckRecvTimeout();
}

//**********************************************************************
// 函数功能: 初始化协议分析模块，向mid层注册协议解析回调函数
// 输入参数：    
// 返回参数：
//**********************************************************************
void App_Protocal_Init(void)
{
    //step 2:Set exchange callback
    Mid_Ble_SetCallBack(Protocal_Analysis);
    //Set BLE Echo status callback
    BLE_Stack_SetEcho_CallBack(App_Ble_Echo);
}

//**********************************************************************
// 函数功能: 授权成功
// 输入参数：    
// 返回参数：
//**********************************************************************
void App_Protocal_AuthorPass(void)
{
	ble_msg_t bleMsg;
    uint8 u8len,u8BleConnStatus;

    //fix 2: 手机请求授权后，中途断开，连接上后不发请求授权，机芯还发授权成功的命令
    //  在接收到请求授权命令，或已连上的状态时才能发送授权成功命令
    //  a.有接收到请求授权命令，才允许授权；
    //  b.先接收到授权请求，后有断开这种情况，视为没有接收到授权请求
    if(eAppProBleConn >= APP_PROTOCAL_RECVAUTHREQEST)
    {
    	bleMsg.id = BLE_MSG_SEND;
    	bleMsg.packet = AUTHOR_PASS_PROTOCAL;
        u8len = bleMsg.packet.att.loadLength + 5;
    	bleMsg.packet.data[u8len] = Mid_Ble_CheckSum(&bleMsg.packet);
        Mid_Ble_SendMsg(&bleMsg);
    
        //fix 1: 机芯发送授权成功命令后，机芯app处于连接状态
        BLE_Stack_Interact(BLE_STATUS_GET,&u8BleConnStatus,0x00); //获取蓝牙连接状态
        if(u8BleConnStatus == BT_CONNECTED)
        {
            App_Ble_UpdateConnStatus(APP_PROTOCAL_CONNECTED);
            App_BleStatus_Change((uint8)BT_CONNECTED);
        }
        else
        {
            App_Ble_UpdateConnStatus(APP_PROTOCAL_DISCONNECTED);
        }
        //fix 1: 2018.6.1
    }
    //fix 2: 2018.6.4
}


//**********************************************************************
// 函数功能: 拍照
// 输入参数：    
// 返回参数：
//**********************************************************************
void App_Protocal_TakePhoto(void)
{
	ble_msg_t bleMsg;
    uint8 u8len;

	bleMsg.id = BLE_MSG_SEND;
	bleMsg.packet = TACK_PICTURE_PROTOCAL;
    u8len = bleMsg.packet.att.loadLength + 5;
	bleMsg.packet.data[u8len] = Mid_Ble_CheckSum(&bleMsg.packet);
    Mid_Ble_SendMsg(&bleMsg);
}

//**********************************************************************
// 函数功能: 查找手机
// 输入参数：    
// 返回参数：
//**********************************************************************
void App_Protocal_FinePhone(void)
{
	ble_msg_t bleMsg;
    uint8 u8len;

	bleMsg.id = BLE_MSG_SEND;
	bleMsg.packet   = FIND_PHONE_PROTOCAL;
    u8len = bleMsg.packet.att.loadLength + 5;
	bleMsg.packet.data[u8len] = Mid_Ble_CheckSum(&bleMsg.packet);
    Mid_Ble_SendMsg(&bleMsg);
}

//**********************************************************************
// 函数功能: 来电提醒时的反馈
// 输入参数：
//phonetype: 电话类型
//   op    :　拒接，或接听　
// 返回参数：
//**********************************************************************
void App_Protocal_PhoneCallRet(uint8 phonetype,uint8 op)
{
    ble_msg_t bleMsg;
    uint8 u8len;

    bleMsg.id = BLE_MSG_SEND;
     if (phonetype == IOS )
     {
        bleMsg.packet = ACCEPT_CALL_IOS_PROTOCAL;
     }
     else
     {
        bleMsg.packet = ACCEPT_CALL_PROTOCAL;
     }

    bleMsg.packet.att.load.content.parameter[0] = op;
    u8len = bleMsg.packet.att.loadLength + 5;
    bleMsg.packet.data[u8len] = Mid_Ble_CheckSum(&bleMsg.packet);
    Mid_Ble_SendMsg(&bleMsg);
    
}

//**********************************************************************
// 函数功能: 蓝牙状态设置
// 输入参数：newblestate
// 返回参数：
//**********************************************************************
void App_Protocal_BleStateSet(uint8 newblestate)
{
   if((newblestate != BLE_SLEEP) && (newblestate != BLE_BROADCAST) && //关,开广播
      (newblestate != BLE_POWERON) && (newblestate != BLE_POWEROFF))  //蓝牙上电 /掉电
       return ;
   
#if(BLE_STACK_RUN_INMCU == 0)
	ble_msg_t bleMsg;
    uint8 u8len;

	bleMsg.id = BLE_MSG_SEND;
	if (newblestate == BLE_SLEEP)
		bleMsg.packet   = CLOSE_BLE_PROTOCAL;
    else if (newblestate == BLE_BROADCAST)
		bleMsg.packet   = OPEN_BLE_PROTOCAL;
	else
		;
    
    u8len = bleMsg.packet.att.loadLength + 5;
    bleMsg.packet.data[u8len] = Mid_Ble_CheckSum(&bleMsg.packet);
    Mid_Ble_SendMsg(&bleMsg);
#else
    
    if ((newblestate == BLE_POWERON) || (newblestate == BLE_POWEROFF))
    {
        BLE_Stack_Interact(BLE_POWERONOFF,&newblestate,0);
    }
    else
    {
        BLE_Stack_Interact(BLE_STATUS_SET,&newblestate,0);
    }

#endif
	
	/* 修改蓝牙状态 */
	if((newblestate == BLE_POWERON) || (newblestate == BLE_BROADCAST))
	{
		bleState = BLE_BROADCAST;	
	}
	
	if(newblestate == BLE_SLEEP)
	{
		bleState = BLE_SLEEP;	
	}
	
	if(newblestate == BLE_POWEROFF)
	{
		bleState = BLE_POWEROFF;
	}	
}

//**********************************************************************
// 函数功能: 手表主动请求同步时间
// 输入参数：
// 返回参数：
//**********************************************************************
void App_Protocal_AdjustTimeprocess(void)
{
    ble_msg_t bleMsg;
    uint8 u8len;

    bleMsg.id = BLE_MSG_SEND;
    bleMsg.packet   = ADJUST_TIME_PROTOCAL;
    u8len = bleMsg.packet.att.loadLength + 5;
    bleMsg.packet.data[u8len] = Mid_Ble_CheckSum(&bleMsg.packet);
    Mid_Ble_SendMsg(&bleMsg);
}

//**********************************************************************
// 函数功能: 手表主动请求天气信息
// 输入参数：
// 返回参数：
//**********************************************************************
void App_Protocal_GetWeatherProcess(void)
{
    ble_msg_t bleMsg;
    uint8 u8len;
    bleMsg.id = BLE_MSG_SEND;
    bleMsg.packet   = GET_WEATHER_PROTOCAL;
    u8len = bleMsg.packet.att.loadLength + 5;
    bleMsg.packet.data[u8len] = Mid_Ble_CheckSum(&bleMsg.packet);
    Mid_Ble_SendMsg(&bleMsg);
}

//**********************************************************************
// 函数功能: 设置蓝牙广播名
// 输入参数：    
// 返回参数：
//**********************************************************************
void App_Protocal_SetBtAdvName(uint8 *name)
{
    uint8 u8len = 0;

    if(name == NULL) return ;

    while(name[u8len] != '\0')u8len++;
    u8len %= 12;

#if(BLE_STACK_RUN_INMCU == 0)
	ble_msg_t bleMsg;

	bleMsg.id = BLE_MSG_SEND;
	bleMsg.packet   = BROCAST_NAME_RENAME;
    memcpy(bleMsg.packet.att.load.content.parameter, name, u8len);
    bleMsg.packet.att.loadLength = u8len + 3;  //接口+数据: 3 + len 
	bleMsg.packet.data[bleMsg.packet.att.loadLength + 5] = Mid_Ble_CheckSum(&bleMsg.packet);
    Mid_Ble_SendMsg(&bleMsg);
#else
    BLE_Stack_Interact(BLE_AVD_NAME_SET,name,u8len);
#endif
}

//**********************************************************************
// 函数功能: 返回闹钟响闹
// 输入参数：闹钟编号    
// 返回参数：
//**********************************************************************
void App_Protocal_AlarmRing(uint8 alarmId)
{
    ble_msg_t bleMsg;

    bleMsg.id       = BLE_MSG_SEND;
    bleMsg.packet   = ALARM_TO_APP_PROTOCAL;
    bleMsg.packet.att.load.content.parameter[0] = alarmId;
    bleMsg.packet.data[bleMsg.packet.att.loadLength + 5] = Mid_Ble_CheckSum(&bleMsg.packet);
    Mid_Ble_SendMsg(&bleMsg);
}

//**********************************************************************
// 函数功能: 返回心率场景下的心率值
// 输入参数：实时心率值    
// 返回参数：
//**********************************************************************
void App_Protocal_HrmRet(uint8 hrval)
{
    ble_msg_t bleMsg;

    bleMsg.id       = BLE_MSG_SEND;
    bleMsg.packet   = HRM_TO_APP_PROTOCAL;
    bleMsg.packet.att.load.content.parameter[0] = hrval;
    bleMsg.packet.data[bleMsg.packet.att.loadLength + 5] = Mid_Ble_CheckSum(&bleMsg.packet);
    Mid_Ble_SendMsg(&bleMsg);
}

//**********************************************************************
// 函数功能: 设置连接间隔
// 输入参数：
// 返回参数：
//**********************************************************************
void App_Protocal_SetConnInterval(uint8 u8Inter)
{
#if(BLE_STACK_RUN_INMCU == 0)
    ble_msg_t bleMsg;

    bleMsg.id = BLE_MSG_SEND;
    bleMsg.packet = CONN_INBTERAVLE_PROTOCAL;
    bleMsg.packet.att.load.content.parameter[0] = u8Inter;
    bleMsg.packet.data[bleMsg.packet.att.loadLength + 5] = Mid_Ble_CheckSum(&bleMsg.packet);
    Mid_Ble_SendMsg(&bleMsg);
#else
    BLE_Stack_Interact(BLE_AVD_NAME_SET,&u8Inter,0);
#endif
}

//**********************************************************************
// 函数功能: 返回SN码
// 输入参数：
// protocal: 协议内容指针
// 返回参数：
//**********************************************************************
void App_Protocal_RetSN(ble_msg_t *protocal)
{
     uint8           snData[30];
//    flash_task_msg_t flashMsg;

    //uint32  u32Temp;
    //uint8_t u8temp;
    uint8   u8Route;     //记录路由
    uint8   u8FlowCtl;   //记录流控号

    protocal->id = BLE_MSG_SEND;
    u8Route     = ((protocal->packet.att.routeMsg & 0xf0)>>4) + ( protocal->packet.att.routeMsg<<4); 
    u8FlowCtl   = protocal->packet.att.flowControl; 

//    flashMsg.id                                     = EXTFLASH_ID;
//    flashMsg.flash.extflashEvent.para.dataAddr      = (uint8*)(&snData);
//    flashMsg.flash.extflashEvent.para.length        = 20;
//    flashMsg.flash.extflashEvent.id                 = EXTFLASH_EVENT_READ_SN;
//    FlashTask_EventSet(&flashMsg);


    protocal->packet.att.loadLength += 0x0B;
    protocal->packet.att.routeMsg = u8Route;
    protocal->packet.att.flowControl = u8FlowCtl;
    protocal->packet.att.load.content.interfaceType = PROTOCAL_RET;

    protocal->packet.att.load.content.parameter[0]  = 0; //低十字节
    for(uint8_t i = 1;i < 11;i++)
    {
        protocal->packet.att.load.content.parameter[i] = snData[i-1];
    }
    protocal->packet.data[protocal->packet.att.loadLength + 5]  = Mid_Ble_CheckSum(&protocal->packet); 
    Mid_Ble_SendMsg(protocal);
    vTaskDelay(2);
	
    protocal->packet.att.load.content.parameter[0] = 1; //高十字节
    for(uint8_t i = 1;i < 11;i++)
    {
        protocal->packet.att.load.content.parameter[i] = snData[i+9];
    }
    protocal->packet.data[protocal->packet.att.loadLength + 5]  = Mid_Ble_CheckSum(&protocal->packet); 
    Mid_Ble_SendMsg(protocal);
}



    
    


