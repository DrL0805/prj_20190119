/**********************************************************************************************
**
**ģ��˵��: BLE Э��ջ�ṩ��APP�ķ���ӿڶ���
**����汾���޸���־(ʱ�䣬����),�޸���:
**  fix 1:  2018.5.27 ���ӻ�оapp����״̬��ʾ���Խ����оapp��������״̬������Ӳ������ͬ������
**  fix 2:  2018.6.4  �ֻ�������Ȩ����;�Ͽ��������Ϻ󲻷�������Ȩ����о������Ȩ�ɹ�������
**  fix 3:  2018.6.6  �������Ӻͻ�оapp����ͬ�����⣬��Ҫ���������㲥
**  fix 4:  2018.6.7  ����OTA�����и�״̬�ķ���,�Լ���ؿ���
**  fix 5:  2018.6.9  �����쳣�Ͽ������ֿ��������,���Ͽ�����
**  fix 6:  2018.6.12 �޸���OTA��Ƶ����Ϣ���ѣ���������ʧ��
**  fix 7:  2018.6.13 �ӿ�ʼOTA��ʱ���ʱ������:��Щ�ֻ����ֽ������ݾͳ�ʱ
**  fix 8:  2018.6.20 IOS��Ϣ���عرգ��Կ���Ӧ��Ϣ(���ѣ���ʾ����)��
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
#define APP_OTATIMERFREQ      1     //OTA���ݽ��ռ��Ƶ�� 1s


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


//�������Ӽ��
const protocal_msg_t CONN_INBTERAVLE_PROTOCAL =
{
    0x23,0x01,0x04,0x12,0x80,0x04,0x00,0x05
};

//�������Ӽ��
const protocal_msg_t COMMON_ACK_PROTOCAL =
{
    0x23,0x01,0x01,0x13,0x80,0x00,0x00
};


uint16 dailyBleLinkInvSwitchWay;

static uint8 BrocastName[32];

uint8  debugu8Option;

typedef enum
{
	APP_PROTOCAL_DISCONNECTED = 0,  //APPδ��Ȩ
	APP_PROTOCAL_RECVAUTHREQEST,    //APP���յ�������Ȩ����
	APP_PROTOCAL_ANCSDISCOVER,      //ANCS������
	APP_PROTOCAL_CONNECTED,         //APP������Ȩ�ɹ���������
}app_conn_type;

//fix 1: ���ӻ�оapp����״̬��ʾ���Խ����оapp��������״̬������Ӳ������ͬ������
//��оapp��������״̬: ��������Ȩ�ɹ����Ϊ׼
//����Ӳ������:��Э��ջ����
static app_conn_type eAppProBleConn = APP_PROTOCAL_DISCONNECTED;
//fix 1: 2018.6.1


//fix 4:����OTA�����и�״̬�ķ���
//OTA�������ݳ�ʱ��¼
static uint32 u32OtaRecvTimeout =0;

//**********************************************************************
// ��������: ���ѽ���
// �����������
// ���ز�����    
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
//            message.op   = UASUAL_MSG_REMIND;//Ӧ����Ϣ
//            WinDowHandle = App_Window_Process(WinDowHandle,TAG_REMIND,message);
            break;

            case APP_REMIND_MSG:    //�ֻ�����
//            validflag       = 1;
//            message.val = PhoneSystem;
//            message.state   = protocaltype;
//            message.op   = MESSAGE_REMIND;
//            WinDowHandle = App_Window_Process(WinDowHandle,TAG_REMIND,message);
            break;

            case APP_REMIND_CALL:   //����   
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
// ��������: ͨ�ã��ã�
// �����������
// ���ز�����    
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
//                       ��·����00H, PROT_LINK��
//////////////////////////////////////////////////////////////////////////////////

//**********************************************************************
// ��������: ����:��������״̬�����ı�����
// ���������u8Status:BLE״̬
// ���ز�����	
//**********************************************************************
static void App_BleStatus_Change(uint8 u8Status)
{
//    movt_task_msg_t     movtMsg;
//	menuMessage        message;

    if(u8Status == BT_ADV_OFF) //��������
    {
        if ((systermConfig.bleDiscRemindSwitch == SWITCH_ON) && bleState == BLE_CONNECT)//���ӶϿ�
        {
//			MOD_PDU_RTT_LOG(0,"BLE OFF \n");
//            message.val   = debugu8Option;
//            message.state = BLE_DICONNECT_STATE;
//            message.op    = BLE_REMIND;
//            WinDowHandle  = App_Window_Process(WinDowHandle,TAG_REMIND,message);
        }      

        bleState    = BLE_SLEEP;
        //У����������ر�
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
    
    if(u8Status == BT_ADV_ON)  //�����㲥
    {
        if ((systermConfig.bleDiscRemindSwitch == SWITCH_ON) && bleState == BLE_CONNECT)//���ӶϿ�
        {
//             message.val   = debugu8Option;
//            message.state = BLE_DICONNECT_STATE;
//            message.op    = BLE_REMIND;
//            WinDowHandle  = App_Window_Process(WinDowHandle,TAG_REMIND,message);
        }      

        bleState      = BLE_BROADCAST;
        //У����������Ͽ�
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
    
    if(u8Status == BT_CONNECTED)  //��������
    {                    
        bleState      = BLE_CONNECT; 
		#if(AIR_PRESSURE_SUPPORT == 0)
        //�����ϣ�����һ������
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
// ��������:    ���������㲥��
// ���������    
// ���ز�����    ��
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
// ��������: ������·��������
// ���������protocal:Э������
// ���ز�����	
//**********************************************************************
static uint8 Analysis_Link(ble_msg_t *protocal)
{
    uint8       u8Route;     //��¼·��
    uint8       u8FlowCtl;   //��¼���غ�
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
        if (protocal->packet.att.load.content.interfaceType == PROTOCAL_ECHO)//ble�����乤��״̬
        {
            App_BleStatus_Change(protocal->packet.att.load.content.parameter[0]);
        }
        if (protocal->packet.att.load.content.interfaceType == PROTOCAL_GET)  //��ȡ��������״̬
        {
                        
        }               
        break;

    case PROT_BT_CONN_PARAM:  //0x02: 
        
        break;
    case PROT_BT_ADV_NAME:    //0x03:    ���������㲥��
        //step 1: set adv name to BLE
        if(protocal->packet.att.loadLength - 3 > 11)   //��Э��������������ת��Ϊ�ַ���
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
    case PROT_BT_FW_VERSION:  //0x06: ������������汾��Ϣ
        
        break;
    
    case PROT_LINKLOSS_REMIND_SWITCH: //0x11: �Ͽ����ѿ���
     if (protocal->packet.att.load.content.interfaceType == PROTOCAL_SET)
     {
        Protocal_SendACK(protocal, SUCCESS); 
        systermConfig.bleDiscRemindSwitch = protocal->packet.att.load.content.parameter[0];

        //���ñ����¼�
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

    case PROT_LINK_HEARBEAT:    //0x12:  ��·����
        
        break;
    }
    return Ret_OK;
}

//////////////////////////////////////////////////////////////////////////////////
//                       �豸��Ϣ��01H, PROT_DEV_INFO��
//////////////////////////////////////////////////////////////////////////////////

//**********************************************************************
// ��������: ��������״̬
// ���������u8status:����״̬
// ���ز�����
//**********************************************************************
static void App_Ble_UpdateConnStatus(app_conn_type status)
{
    //status:
    //APP_PROTOCAL_RECVAUTHREQEST: ��һ�׶�����
    //APP_PROTOCAL_ANCSDISCOVER:   ��һ�׶�����,ANCS����
    //APP_PROTOCAL_CONNECTED:      �ڶ������׶�����
    //�����׶�������ӣ������ܲ���Ӱ�죬���ʧ�ܾͻ��Զ��Ͽ���������
    eAppProBleConn = status;
}

//**********************************************************************
// ��������: �����豸��Ϣ����
// ���������protocal:Э������
// ���ز�����	
//**********************************************************************
static uint8 Analysis_DeviceInfo(ble_msg_t *protocal)
{
	Mid_Weahter_Param_t tWeather;
    uint32  u32temp;
    uint8   u8temp;      //��¼�м�ֵ
    uint8   u8Route;     //��¼·��
	uint8 	u8FlowCtl;   //��¼���غ�
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
    case PROT_PROD_NAME:  //0x01:  ��Ʒ����
        
         break;
    case PROT_PROD_VER:   //0x02:  �̼��汾��
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
    case PROT_PROD_DATE:   //0x03: ��������
        
        break;
    case PROT_PROD_SOC:     //0x04:  ʣ�����
    case PROT_PROD_SOC_NEW:
//    batType = Mid_Bat_SocRead(&u8temp);
    protocal->packet.att.routeMsg = u8Route;
	protocal->packet.att.flowControl = u8FlowCtl;
	protocal->packet.att.load.content.interfaceType = PROTOCAL_RET; 
    if(protocal->packet.att.load.content.interfaceIndex2 == PROT_PROD_SOC)
    {           
        protocal->packet.att.loadLength += 1;
        protocal->packet.att.load.content.parameter[0] = u8temp;   //ʣ�����
        protocal->packet.att.load.content.parameter[1] = Mid_Ble_CheckSum(&protocal->packet);
    }
    else
    {
        protocal->packet.att.loadLength += 2;
        protocal->packet.att.load.content.parameter[0] = batType;  //�ɳ����
        protocal->packet.att.load.content.parameter[1] = u8temp;   //ʣ�����
        protocal->packet.att.load.content.parameter[2] = Mid_Ble_CheckSum(&protocal->packet);
    }
    Mid_Ble_SendMsg(protocal);
    break;

    case PROT_PROD_AUTH:    //0x05: ��Ȩ
		if(protocal->packet.att.load.content.parameter[0] == 0x00)  //apply author
		{
			//step 1:send ACK
			App_Ble_UpdateConnStatus(APP_PROTOCAL_RECVAUTHREQEST);
			Protocal_SendACK(protocal, SUCCESS);

			//step 2:set author phonestate
			phoneState.timeCnt  = 0;
			phoneState.timeMax  = APP_AuthorTimeOut;
			phoneState.state    = PHONE_STATE_AUTHOR;

			// �������ʾ��Ȩ
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
       
       //���ñ����¼�
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

        //���ñ����¼�
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
//                       �������ƣ�03H, PROT_INTERACT��
//////////////////////////////////////////////////////////////////////////////////


//**********************************************************************
// ��������: ����������������
// ���������protocal:Э������
// ���ز�����	
//**********************************************************************
static uint8 Analysis_Interact(ble_msg_t *protocal)
{
//    movt_task_msg_t movtMsg;
//    menuMessage message={0};
    // ble_msg_t bleMsg;
    uint8_t u8temp;
	uint8   u8Route;     //��¼·��
	uint8 	u8FlowCtl;   //��¼���غ�
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
    case PROT_NEW_MSG:  //0x01: ����Ϣ֪ͨ
    Protocal_SendACK(protocal,SUCCESS);
        appRemindState = ((uint32)protocal->packet.att.load.content.parameter[0] << 24) 
                       + ((uint32)protocal->packet.att.load.content.parameter[1] << 16)
                       + ((uint32)protocal->packet.att.load.content.parameter[2] << 8)
                       + protocal->packet.att.load.content.parameter[3];
   
        appRemindState &= systermConfig.appRemindSwitch;//����δ����������
                       
        if (appRemindState && (phoneState.state != PHONE_STATE_PHOTO))//switch open systermConfig.
        {
            if(!((systermConfig.notDisturbSwitch == SWITCH_ON) && (App_NotDisturdTimeCheck())))//����ʱ����ڲ�����
            {
                if (protocal->packet.att.routeMsg == 0x21)
                {
                    u8temp = IOS;
                }
                else
                {
                    u8temp = ANDROID;
                }
                // App_NewMsgAnalysis(u8temp, appRemindState,NEW_REMIND); //������
            }
        }               
        break;
    case PROT_MSG_SWITCH://0x02��֪ͨ����
        if (protocal->packet.att.load.content.interfaceType == PROTOCAL_GET)//��ȡ
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
        }else if (protocal->packet.att.load.content.interfaceType == PROTOCAL_SET)//���� 
        {
           systermConfig.appRemindSwitch     = ((uint32)protocal->packet.att.load.content.parameter[0] << 24) 
                                                + ((uint32)protocal->packet.att.load.content.parameter[1] << 16)
                                                + ((uint32)protocal->packet.att.load.content.parameter[2] << 8)
                                                + ((uint32)protocal->packet.att.load.content.parameter[3]);
           Protocal_SendACK(protocal,SUCCESS);

           //���ñ����¼�
//            appMsg.id   = APP_CACEH_DATA_SAVE;
//            appMsg.para = CACHE_SYSTERM_TYPE;
//            App_Task_EventSet(appMsg);
        }
        else
        {
            Protocal_SendACK(protocal,PARAMATER_ERROR);
        }
        break;
    case PROT_INCALL_RESP:   // 0x03: ����֪ͨ������ֻ��Ʒ����
        break;
    case PROT_CANCEL_MSG:    //0x04:��Ϣȡ��֪ͨ 
    Protocal_SendACK(protocal,SUCCESS);
    appRemindState = ((uint32)protocal->packet.att.load.content.parameter[0] << 24) 
                   + ((uint32)protocal->packet.att.load.content.parameter[1] << 16)
                   + ((uint32)protocal->packet.att.load.content.parameter[2] << 8)
                   + protocal->packet.att.load.content.parameter[3];

    appRemindState &= systermConfig.appRemindSwitch;//����δ����������
                   
    if (appRemindState)//switch open systermConfig.
    {
//        if(!((systermConfig.notDisturbSwitch == SWITCH_ON) && (App_NotDisturdTimeCheck())))//����ʱ����ڲ�����
//        {
//            if (protocal->packet.att.routeMsg == 0x21)
//            {
//                u8temp = IOS;
//            }
//            else
//            {
//                u8temp = ANDROID;
//            }
//            App_NewMsgAnalysis(u8temp, appRemindState,CANCEL_REMIND);//����ȡ��
//        }
    }  
        break;
        case PROT_MSG_DETAIL_SWITCH: //0x05: ��Ϣ���鿪��
        if (protocal->packet.att.load.content.interfaceType == PROTOCAL_SET)//���� 
        {
           systermConfig.appDetailRemindSwitch     = ((uint32)protocal->packet.att.load.content.parameter[0] << 24) 
                                                    + ((uint32)protocal->packet.att.load.content.parameter[1] << 16)
                                                    + ((uint32)protocal->packet.att.load.content.parameter[2] << 8)
                                                    + ((uint32)protocal->packet.att.load.content.parameter[3]);
            //���ñ����¼�                                        
//            appMsg.id   = APP_CACEH_DATA_SAVE;
//            appMsg.para = CACHE_SYSTERM_TYPE;
//            App_Task_EventSet(appMsg);
        }
        break;
    case PROT_MSG_DETAIL://0x06:��Ϣ����֪ͨ
        //����������Ϣ���鿪�أ��������ѿ���
        systermConfig.appDetailRemindSwitch = systermConfig.appRemindSwitch;

        //Analy_Debug((0,"appDetailRemindSwitch=0x%x,remind type=%d\n",systermConfig.appDetailRemindSwitch,protocal->packet.att.load.content.parameter[0]));
        //fix 8:IOS��Ϣ���عرգ��Կ���Ӧ��Ϣ(���ѣ���ʾ����)��
        if((systermConfig.appDetailRemindSwitch & (0x01 << protocal->packet.att.load.content.parameter[0])) == 0x0000)
        {
            return Ret_OK;
        }
        //fix 8:2018.6.20
        if (systermConfig.appDetailRemindSwitch && (phoneState.state != PHONE_STATE_PHOTO))//����֪ͨ�Ž��д���
        { 
//            if(!((systermConfig.notDisturbSwitch == SWITCH_ON) && (App_NotDisturdTimeCheck())))//����ʱ����ڲ�����
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

         case PROT_PHOTO_MODE: //����
        if (protocal->packet.att.load.content.parameter[0] == 0x00)//�˳�
        {
//            message.state   = EXIT_TAKE_PHOTO_MODE;
//            message.op      = TAKE_PHOTO_REMIND;
//            WinDowHandle    = App_Window_Process(WinDowHandle,TAG_REMIND,message);
            phoneState.state   = PHONE_STATE_NORMAL;
            phoneState.timeCnt = 0;

        }else if (protocal->packet.att.load.content.parameter[0] == 0x01)//����
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

        case PROT_PHOTO_HEARTBEAT: //��������
        if (phoneState.state == PHONE_STATE_PHOTO)
        {
            phoneState.state   = PHONE_STATE_PHOTO;
            phoneState.timeCnt = 0;
            phoneState.timeMax = APP_TakePhotoTimeOut;
        }
        break;

        case PROT_WATCH_HAND_MODE://У��:0x31
        #if 0
        if(protocal->packet.att.load.content.parameter[0] == 0x01)//����У��
        {
			movtState.timeCnt 	= 0;
            movtState.state     = MOVT_STATE_ADJUST;
            movtMsg.id          = MOVT_MSG_MC_STOP;
            MovtTask_EventSet(movtMsg);
            Protocal_SendACK(protocal,SUCCESS);
        }
        else if(protocal->packet.att.load.content.parameter[0] == 0x00)//�˳�У��
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

        case PROT_WATCH_HAND_PARAM: // У�������0x33
        Protocal_SendACK(protocal, SUCCESS);
        #if 0
        if (movtState.state == MOVT_STATE_ADJUST)
        {
            //if(protocal->packet.att.load.content.parameter[0] == MOVT_2PIN_M) // ����RTCʱ���Ǹ�����
             if(protocal->packet.att.load.content.parameter[0] == MOVT_3PIN_M || protocal->packet.att.load.content.parameter[0] == MOVT_2PIN_M) // ����RTCʱ���Ǹ�����
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

        case PROT_WATCH_HAND_HEARTBEAT: // У����������0x34
		if(movtState.state == MOVT_STATE_ADJUST)
		{
			movtState.state   = MOVT_STATE_ADJUST;
			movtState.timeCnt = 0;
		}
        
        break;

    case PROT_LONG_SIT_SWITCH:    //0x51: ����
		if(protocal->packet.att.load.content.interfaceType == PROTOCAL_SET)
		{
			Protocal_SendACK(protocal,SUCCESS);
			systermConfig.longSitRemindSwitch = protocal->packet.att.load.content.parameter[0];

            //���ñ����¼�                                        
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
    case PROT_LONG_SIT_PARAM:     //0x52: ��������
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
            //fix :����ʱ��������ϴ󡣸��¾������Ѳ���֮������������������¼���
//            if(systermConfig.longSitRemindSwitch == 0x01)
//            {
//                Mid_UsualScene_SedentaryClear();
//            }

//            //���ñ����¼�                                        
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
    case PROT_DND_SWITCH:         //0x61: ���ſ���
		if(protocal->packet.att.load.content.interfaceType == PROTOCAL_SET)
		{
			Protocal_SendACK(protocal,SUCCESS);
			systermConfig.notDisturbSwitch = protocal->packet.att.load.content.parameter[0];

            //���ñ����¼�                                        
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
    case PROT_DND_PARAM:          //0x62: ���Ų���
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

        //���ñ����¼�                                        
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

    case PROT_SYS_SETTING:  //0x91ϵͳ����
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

        //���ñ����¼�                                        
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
//                       �豸���ԣ�04H, PROT_DEV_TEST��
//////////////////////////////////////////////////////////////////////////////////


//**********************************************************************
// ��������: ����������������
// ���������bleMsg:Э������
// ���ز�����	
//**********************************************************************
static uint8 Analysis_DevTest(ble_msg_t *protocal)
{
    return Ret_OK;

}

//////////////////////////////////////////////////////////////////////////////////
//                       ��������05H, PORT_SENSOR��
//////////////////////////////////////////////////////////////////////////////////


//**********************************************************************
// ��������: ����������������
// ���������bleMsg:Э������
// ���ز�����	
//**********************************************************************
static uint8 Analysis_Sensor(ble_msg_t *protocal)
{
    return Ret_OK;

}

//////////////////////////////////////////////////////////////////////////////////
//                       ������06H, PROT_UPDATE��
//////////////////////////////////////////////////////////////////////////////////


//**********************************************************************
// ��������: OTA����
// ���������bleMsg:Э������
// ���ز�����	
//**********************************************************************
static uint8 Analysis_Update(ble_msg_t *protocal)
{
    return Ret_OK;
}

//////////////////////////////////////////////////////////////////////////////////
//                   �ճ��˶�������10H, PROT_DAILY_SPORT_SCENE��
//////////////////////////////////////////////////////////////////////////////////


//**********************************************************************
// ��������: ����������������
// ���������protocal:Э������
// ���ز�����	
//**********************************************************************
static uint8 Analysis_Sport(ble_msg_t *protocal)
{
    uint32  u32temp;
    uint8   u8Route;     //��¼·��
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
        if (protocal->packet.att.load.content.parameter[0] == 0x01) //�����ճ��˶�������
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

             //���ñ����¼�                                        
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
//                       ��ɽ������11H��
//////////////////////////////////////////////////////////////////////////////////


//**********************************************************************
// ��������: ����������������
// ���������protocal:Э������
// ���ز�����	
//**********************************************************************
static uint8 Analysis_Climbing(ble_msg_t *protocal)
{
    return Ret_OK;

}

//////////////////////////////////////////////////////////////////////////////////
//                       �ܲ�������12H��
//////////////////////////////////////////////////////////////////////////////////


//**********************************************************************
// ��������: ����������������
// ���������protocal:Э������
// ���ز�����    
//**********************************************************************
static uint8 Analysis_RunningScene(ble_msg_t *protocal)
{
    return Ret_OK;

}

//////////////////////////////////////////////////////////////////////////////////
//                       ��Ӿ������13H��
//////////////////////////////////////////////////////////////////////////////////


//**********************************************************************
// ��������: ����������������
// ���������protocal:Э������
// ���ز�����    
//**********************************************************************
static uint8 Analysis_SwingScene(ble_msg_t *protocal)
{
    return Ret_OK;

}

//////////////////////////////////////////////////////////////////////////////////
//                      ˯�߳�����14H��
//////////////////////////////////////////////////////////////////////////////////


//**********************************************************************
// ��������: ����������������
// ���������protocal:Э������
// ���ز�����    
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
        case PORT_SLEEP_USAUL_PARA://0x01��
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
//                      ˯�߳�����15H��
//////////////////////////////////////////////////////////////////////////////////


//**********************************************************************
// ��������: ����������������
// ���������protocal:Э������
// ���ز�����    
//**********************************************************************
static uint8 Analysis_HeartScene(ble_msg_t *protocal)
{
    //uint8   u8temp;      //��¼�м�ֵ
    uint8   u8Route;     //��¼·��
    //uint8   u8FlowCtl;   //��¼���غ�
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
        #if 0 //�ݲ�����app���ʳ���
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
        #if 0//�ݲ�����app���ʳ���
        if (phoneState.state != PHONE_STATE_HRM)
        {
            //�������ʳ�����ʱ
            phoneState.state = PHONE_STATE_HRM;
            phoneState.timeCnt  = 0;
            phoneState.timeMax = APP_Hrm_TimeOut;

            //����ʵʱ���ʼ��
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
//                    ���ݰ����䣨f0H, PROT_PACK_TRANS��
//////////////////////////////////////////////////////////////////////////////////


//**********************************************************************
// ��������: ����������������
// ���������protocal:Э������
// ���ز�����	
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
            case PORT_PACK_TRANS_GET_CONTENT_COUNT://0x10����ȡĿ¼���������ݳ���
//            App_PackData_DataRequestTotalInfoAck(((uint32_t)(protocal->packet.att.load.content.parameter[0]) << 8) + protocal->packet.att.load.content.parameter[1],
//                                            protocal->packet.att.flowControl);
            break;

            case PORT_PACK_TRANS_GET_CONTENT_INFO://0x11���ѻ�֪��Ŀ¼����£�ָ��Ŀ¼��ż��������ͣ���ȡָ�����Ŀ¼����Ϣ
//            App_PackData_DataCatalogInfoRead(((uint32_t)(protocal->packet.att.load.content.parameter[0]) << 8) + protocal->packet.att.load.content.parameter[1],
//                                    ((uint32_t)(protocal->packet.att.load.content.parameter[2]) << 8) + protocal->packet.att.load.content.parameter[3],
//                                    protocal->packet.att.flowControl);
            break;

            case PORT_PACK_TRANS_GET_PKG://0x12�������ϴ�ָ����UTC��Ŀ¼�����������ݶΣ���Ӱ���ż�������ԣ�
//            App_PackData_DataRequestData(((uint32_t)(protocal->packet.att.load.content.parameter[0]) << 8) + protocal->packet.att.load.content.parameter[1],
//                                ((uint32_t)(protocal->packet.att.load.content.parameter[2]) << 24) + ((uint32_t)(protocal->packet.att.load.content.parameter[3]) << 16) +
//                                ((uint32_t)(protocal->packet.att.load.content.parameter[4]) << 8) + protocal->packet.att.load.content.parameter[5],
//                                ((uint32_t)(protocal->packet.att.load.content.parameter[6]) << 8) + protocal->packet.att.load.content.parameter[7],
//                                protocal->packet.att.flowControl);
            break;


            case PORT_PACK_TRANS_AUTO_UPDATE_CONTENT_COUNT:
            case PORT_PACK_TRANS_AUTO_UPDATE_CONTENT_INFO:
            break;


            case PORT_PACK_TRANS_DEL_ALL://0x30 :ɾ����������
            Protocal_SendACK(protocal,SUCCESS);
//            App_PackData_DataTotalDelete(protocal->packet.att.flowControl);
            break;


            case PORT_PACK_TRANS_DEL_DATATYPE://0x31 �� ɾ��ָ�����͵�����
            Protocal_SendACK(protocal,SUCCESS);
//            App_PackData_DataDeleteClassify(((uint32_t)(protocal->packet.att.load.content.parameter[0]) << 8) + protocal->packet.att.load.content.parameter[1],
//                                    protocal->packet.att.flowControl);
            break;


            case PORT_PACK_TRANS_DEL_CONTENT_UTC://0x32�� ��UTCɾ������
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
//                    ���ݰ����䣨f0H, PROT_PACK_TRANS��
//////////////////////////////////////////////////////////////////////////////////


//**********************************************************************
// ��������: ����������������
// ���������protocal:Э������
// ���ز�����    
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
            case PORT_SCENE_PACK_TRANS_GET_CONTENT_COUNT://0x10����ȡĿ¼���������ݳ���
//            App_SceneData_DataRequestTotalInfoAck(((uint32_t)(protocal->packet.att.load.content.parameter[0]) << 8) + protocal->packet.att.load.content.parameter[1],
//                                            protocal->packet.att.flowControl);
            break;

            case PORT_SCENE_PACK_TRANS_GET_CONTENT_INFO://0x11���ѻ�֪��Ŀ¼����£�ָ��Ŀ¼��ż��������ͣ���ȡָ�����Ŀ¼����Ϣ 
//			App_SceneData_DataCatalogInfoRead(((uint32_t)(protocal->packet.att.load.content.parameter[0]) << 8) + protocal->packet.att.load.content.parameter[1],
//									((uint32_t)(protocal->packet.att.load.content.parameter[2]) << 8) + protocal->packet.att.load.content.parameter[3],
//									protocal->packet.att.flowControl);
            break;

            case PORT_SCENE_PACK_TRANS_GET_PKG://0x12�������ϴ�ָ����UTC��Ŀ¼�����������ݶΣ���Ӱ���ż�������ԣ� 
//            App_SceneData_DataRequestData(((uint32_t)(protocal->packet.att.load.content.parameter[0]) << 8) + protocal->packet.att.load.content.parameter[1],
//                                ((uint32_t)(protocal->packet.att.load.content.parameter[2]) << 24) + ((uint32_t)(protocal->packet.att.load.content.parameter[3]) << 16) +
//                                ((uint32_t)(protocal->packet.att.load.content.parameter[4]) << 8) + protocal->packet.att.load.content.parameter[5],
//                                ((uint32_t)(protocal->packet.att.load.content.parameter[6]) << 8) + protocal->packet.att.load.content.parameter[7],
//                                protocal->packet.att.flowControl);
            break;

            case PORT_SCENE_PACK_TRANS_AUTO_UPDATE_CONTENT_COUNT:
            case PORT_SCENE_PACK_TRANS_AUTO_UPDATE_CONTENT_INFO:
            break;

            case PORT_SCENE_PACK_TRANS_DEL_ALL://0x30 :ɾ����������
            Protocal_SendACK(protocal,SUCCESS);
            break;


            case PORT_SCENE_PACK_TRANS_DEL_DATATYPE://0x31 �� ɾ��ָ�����͵�����
            Protocal_SendACK(protocal,SUCCESS);
            break;


            case PORT_SCENE_PACK_TRANS_DEL_CONTENT_UTC://0x32�� ��UTCɾ������
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


//fix 3: �������Ӻͻ�оapp���Ӳ�ͬ�����⣬��Ҫ���������㲥
//**********************************************************************
// ��������: ���������㲥
// ���������    
// ���ز�����
//**********************************************************************
static void App_Protocal_ResetAdv(void)
{
    uint8 u8BleConnStatus;
    static uint8 u8ConnExpFlag = FALSE;//��ʾ�����쳣��־
    static uint32 u32ConnTimeout = 0;

    //step 1:��ȡ������·����״̬
    BLE_Stack_Interact(BLE_STATUS_GET,&u8BleConnStatus,0x00); //��ȡ��������״̬

    //step 2:�ж������Ƿ�������²㲻ͬ��
    //���²����Ӳ�ͬ������: �����������ϣ�app��Disconnect״̬
    if((u8BleConnStatus == BT_CONNECTED) && (eAppProBleConn == APP_PROTOCAL_DISCONNECTED) && (u8ConnExpFlag == FALSE))
    {
        u32ConnTimeout = SMOS_GetTickCount();
        u8ConnExpFlag = TRUE;  //��⵽���²����Ӳ�ͬ��
        Err_Info((0,"Error: App check that connect is no same\n"));
    }

    //step 2-1:���²�����ͬ����reset�����쳣��־
    if(((u8BleConnStatus == BT_CONNECTED) && (eAppProBleConn >= APP_PROTOCAL_ANCSDISCOVER)) ||
        ((u8BleConnStatus == BT_ADV_ON) && (eAppProBleConn == APP_PROTOCAL_DISCONNECTED)))
    {
        //Err_Info((0,"warning: reset connect flag\n"));
        u8ConnExpFlag = FALSE;   //��λ�����쳣��־
    }

    //step 3:�ϲ���������Ӳ�ͬ��һ:���������ϣ�appδ���ӣ����������㲥
    //���������㲥����:�������²����Ӳ�ͬ�������ҳ���10s
    if((u8ConnExpFlag == TRUE) && ((SMOS_GetTickCount() - u32ConnTimeout) >= BLE_CONNECT_ERROR_TIMEOUT))
    {
        uint8 u8adv;

        Err_Info((0,"Error: connect is wrong,need resart BLE ADV\n"));
        //step 3-1: �رչ㲥
        u8adv = BT_ADV_OFF;
        BLE_Stack_Interact(BLE_STATUS_SET,&u8adv,0);

        //step 3-2: �����㲥
        u8adv = BT_ADV_ON;
        BLE_Stack_Interact(BLE_STATUS_SET,&u8adv,0);
        u8ConnExpFlag = FALSE;     //��λ�����쳣��־
    }

    //step 4:�ϲ���������Ӳ�ͬ����:app�����ϣ������ǶϿ����������Ҫ��λ������ʾ״̬
    //  ����app����״̬Ϊ�Ͽ��������һ������жϣ���ʱ��Ƭ���л�task��ʱ����
    if((u8BleConnStatus == BT_ADV_ON) && (eAppProBleConn >= APP_PROTOCAL_ANCSDISCOVER))
    {        
        App_Ble_UpdateConnStatus(APP_PROTOCAL_DISCONNECTED); //�����Ͽ�
        App_BleStatus_Change((uint8)BT_ADV_ON);
    }
    //step 3 ��step 4:��Ϊ��ͬ���ϲ�����������ϵĲ�ͬ��
}
//fix: 2018.6.6

//fix 4:����OTA�����и�״̬�ķ���
//**********************************************************************
// ��������: OTA�������ݳ�ʱ
// ���������u8status:����״̬
// ���ز�����
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
// ��������: ����OTA״̬
// ���������u8status:����״̬
// ���ز�����
// ˵    ��: ����ʵ���� App_Protocal_OTAStatus��
//**********************************************************************
static void App_Ble_UpdateOTAStatus(Ble_Ota_Status status)
{
    if(status == BLE_OTA_START)   //OTA������ʼ,��Ļ��ʾ"������..."
    {
        //fix 6: system will be in OTA mode, other module will stop working.
        BLE_Stack_SetMode(BLE_MODE_OTA);
        //fix: 2018.6.12
        //fix 7:�ӿ�ʼOTA��ʱ��ʼ��ʱ����Щ�ֻ����ֽ������ݾͳ�ʱ
        u32OtaRecvTimeout = SMOS_GetTickCount();
        //fix 7:2018.6.13

        //step :����Ӧ�����Ĺ��ܣ���Ļ��ʾ
//        App_BleOTA_Cb(0x01);
    }
    else if(status == BLE_OTA_RECV)   //OTA�����������ݣ����ճ�ʱ����
    {
        u32OtaRecvTimeout = SMOS_GetTickCount();
    }
    else if(status == BLE_OTA_RESET)   //OTA������λ
    {
//        App_BleOTA_Cb(0x02);
    }
    else if(status == BLE_OTA_ERROR)   //OTA����ʧ��,��Ļ�˳���ʾ������ʾ
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
// ��������: ����״̬�����ı�֪ͨ
// ���������echo:֪ͨ״̬���ͣ�u8Status:״̬
// ���ز�����
//**********************************************************************
static void App_Ble_Echo(Ble_Echo_Type echo,uint32 u32Status,uint8 u8Option)
{
//     menuMessage         message;

    if(echo == BLE_CONN_STATUS_ECHO)
    {
        //u32Status:����״̬��u8Option:����Э��ջ������Ϣ
        Analy_Debug((0,"App_Ble_Echo =%d,reason=0x%x\n",u32Status,u8Option));
        //fix 1:ֻ���ڻ�оapp�����ϵ�״̬�£��Ÿ����������ӶϿ���״̬��
        //      �Ա������ֻ����о���ӹ����г������Ӳ��ȶ����ص����쳣�Ͽ�״̬
        if((eAppProBleConn > APP_PROTOCAL_DISCONNECTED) && (u32Status == BT_ADV_ON))
        {
            if(eAppProBleConn >= APP_PROTOCAL_ANCSDISCOVER)
            {
//                App_BleDisConnCb();
            }

            App_Ble_UpdateConnStatus(APP_PROTOCAL_DISCONNECTED); //�����Ͽ�
        }
        //fix 1: 2018.6.1
    }
    else if(echo == BLE_PAIRING_PASSKEY_ECHO)
    {
//        Analy_Debug((0,"paring pass key =%d\n",u32Status));
        //��ʾ�������pin��(passkey)
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
//        //u32Status = 0x00:��Գɹ���=0x01:���ʧ��
//        message.val   = u32Status;
//        message.state = EXIT_PAIRE_STATE;
//        message.op    = PAIRE_REMIND;
//        WinDowHandle  = App_Window_Process(WinDowHandle,TAG_REMIND,message);

        //fix 3:�������Ӻͻ�оapp����ͬ������
        if(u32Status == 0x00)
        {
            //��Գɹ������ӳɹ�
            App_Ble_UpdateConnStatus(APP_PROTOCAL_CONNECTED); //��������
        }
        else
        {
            //���ʧ�ܣ������Զ��Ͽ�
            App_Ble_UpdateConnStatus(APP_PROTOCAL_DISCONNECTED); //�����Ͽ�
        }
        //fix
    }
    else if(echo == BLE_OTA_ECHO)      //OTA����״̬����
    {
        Analy_Debug((0,"OTA =%d\n",u32Status));
        //fix 4:����OTA�����и�״̬�ķ���
        App_Ble_UpdateOTAStatus((Ble_Ota_Status)u32Status);
        //fix: 2018.6.7
    }
    else if(echo == BLE_ANCS_DISCOVERING)
    {
        //fix 3:ANCS��λ���֣�����ֻ�ֻ������������Ӧ��APP��Ҳ����֧��ANCS����
        App_Ble_UpdateConnStatus(APP_PROTOCAL_ANCSDISCOVER);

        //fix:IOS�ֻ�ANCS���ܲ���Ҫapp֧�֣��յ����¼���������ʾ����״̬
        bleState = BLE_CONNECT;   // just for ancs
    }
    else
        ;
}

//**********************************************************************
// ��������: Э�����������
// ���������protocal:Э������
// ���ز�����
//**********************************************************************
static void Protocal_Analysis(ble_msg_t *protocal)
{   
    if(protocal->packet.att.loadLength == 1)
    {
        // ����ACK��ɾ���ط�����
        Mid_Ble_DelResendMsg(&protocal->packet);
        return ;
    }

    switch(protocal->packet.att.load.content.interfaceIndex1)
    {
    case PROT_LINK:              //0x00:��·����
        Analysis_Link(protocal);
        break;
    case PROT_DEV_INFO:          //0x01:�豸��Ϣ
        Analysis_DeviceInfo(protocal);
        break;
    case PROT_INTERACT:          //0x03:��������
        Analysis_Interact(protocal);
        break;
    case PROT_DEV_TEST:          //0x04:�豸����
        Analysis_DevTest(protocal);    
        break;
    case PROT_SENSOR:            //0x05:������
        Analysis_Sensor(protocal); 
        break;
    case PROT_UPDATE:            //0x06:����
        Analysis_Update(protocal);
        break;
    case PROT_APP_SPORT_SCENE:   //0x10:�ճ��˶�����
        Analysis_Sport(protocal);
        break;
    case PROT_MOUNTAINEER_SCENE: //0x11:��ɽ����
        Analysis_Climbing(protocal);
        break;
    case PROT_RUNNING_SCENE: //0x12:�ܲ�����
        Analysis_RunningScene(protocal);
        break;
    case PROT_SWIMING_SCENE: //0x13:��Ӿ����
        Analysis_SwingScene(protocal);
        break;
    case PROT_SLEEP_SCENE: //0x14:˯�߳���
        Analysis_SleepScene(protocal);
        break;
    case PROT_HRS_SCENE: //0x15:���ʳ���
        Analysis_HeartScene(protocal);
        break;
    case PROT_PACK_TRANS:        //0xf0:���ݰ�����
        Analysis_PackTrans(protocal);
        break;

    case PROT_SCENE_PACK_TRANS:        //0xf2:�������ݰ�����
    Analysis_ScenePackTrans(protocal);
    break;

    case PROT_EVENT:             //0x02: ���޶���
    default:
        break;
    }
}

//**********************************************************************
// ��������: OTA״̬����
// ���������u16param: = 0x00:�˳�OTA����
//           = 0x01: ����OTA����
//           = 0x02: OTA������λ
// ���ز�����
//**********************************************************************
void App_Protocal_OTAStatus(uint16 u16param)
{
//    menuMessage  message;

    if(u16param == 0x01)   //����OTA����
    {
//        App_OtaSystermSuspend();
//        message.state = OTA_ENTER;
//        message.op    = OTA_REMIND;
//        WinDowHandle  = App_Window_Process(WinDowHandle,TAG_REMIND,message);
    }
    else if(u16param == 0x00)  //�˳�OTA����
    {
//        App_OtaSystermRecover();
//        message.state = OTA_EXIT;
//        message.op    = OTA_REMIND;
//        WinDowHandle  = App_Window_Process(WinDowHandle,TAG_REMIND,message);
    }
    else if(u16param == 0x02)  //OTA������λ
    {
//        //step 1:write OTA reset flag to flash
//        App_WriteResetFlag(OTA_RESET);
//        //step 1: ��mcu��λ֮ǰ��������
//        //�������ݴ洢
//        App_CacheDataSaveAll();    
//        
//        //step 2: ��λMCU
//        SMDrv_SYS_Rest_POI();
    }
    else
        ;
}

//**********************************************************************
// ��������: �����Ͽ�ʱ���öϿ�״̬���˳�OTA��������
// ���������
// ���ز�����
//**********************************************************************
void App_Protocal_BleDisConn(void)
{
//    menuMessage  message;

    //fix 5: �����쳣�Ͽ������ֿ��������,���Ͽ�����
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
// ��������: ��������״̬��OTA�������ճ�ʱ���
// ���������    
// ���ز�����
//**********************************************************************
void App_Protocal_Monitor(void)
{
    //step 1: ��������״̬���
    //App_Protocal_ResetAdv();

    //step 2: OTA�������ݳ�ʱ����
    App_Ble_CheckRecvTimeout();
}

//**********************************************************************
// ��������: ��ʼ��Э�����ģ�飬��mid��ע��Э������ص�����
// ���������    
// ���ز�����
//**********************************************************************
void App_Protocal_Init(void)
{
    //step 2:Set exchange callback
    Mid_Ble_SetCallBack(Protocal_Analysis);
    //Set BLE Echo status callback
    BLE_Stack_SetEcho_CallBack(App_Ble_Echo);
}

//**********************************************************************
// ��������: ��Ȩ�ɹ�
// ���������    
// ���ز�����
//**********************************************************************
void App_Protocal_AuthorPass(void)
{
	ble_msg_t bleMsg;
    uint8 u8len,u8BleConnStatus;

    //fix 2: �ֻ�������Ȩ����;�Ͽ��������Ϻ󲻷�������Ȩ����о������Ȩ�ɹ�������
    //  �ڽ��յ�������Ȩ����������ϵ�״̬ʱ���ܷ�����Ȩ�ɹ�����
    //  a.�н��յ�������Ȩ�����������Ȩ��
    //  b.�Ƚ��յ���Ȩ���󣬺��жϿ������������Ϊû�н��յ���Ȩ����
    if(eAppProBleConn >= APP_PROTOCAL_RECVAUTHREQEST)
    {
    	bleMsg.id = BLE_MSG_SEND;
    	bleMsg.packet = AUTHOR_PASS_PROTOCAL;
        u8len = bleMsg.packet.att.loadLength + 5;
    	bleMsg.packet.data[u8len] = Mid_Ble_CheckSum(&bleMsg.packet);
        Mid_Ble_SendMsg(&bleMsg);
    
        //fix 1: ��о������Ȩ�ɹ�����󣬻�оapp��������״̬
        BLE_Stack_Interact(BLE_STATUS_GET,&u8BleConnStatus,0x00); //��ȡ��������״̬
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
// ��������: ����
// ���������    
// ���ز�����
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
// ��������: �����ֻ�
// ���������    
// ���ز�����
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
// ��������: ��������ʱ�ķ���
// ���������
//phonetype: �绰����
//   op    :���ܽӣ��������
// ���ز�����
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
// ��������: ����״̬����
// ���������newblestate
// ���ز�����
//**********************************************************************
void App_Protocal_BleStateSet(uint8 newblestate)
{
   if((newblestate != BLE_SLEEP) && (newblestate != BLE_BROADCAST) && //��,���㲥
      (newblestate != BLE_POWERON) && (newblestate != BLE_POWEROFF))  //�����ϵ� /����
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
	
	/* �޸�����״̬ */
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
// ��������: �ֱ���������ͬ��ʱ��
// ���������
// ���ز�����
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
// ��������: �ֱ���������������Ϣ
// ���������
// ���ز�����
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
// ��������: ���������㲥��
// ���������    
// ���ز�����
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
    bleMsg.packet.att.loadLength = u8len + 3;  //�ӿ�+����: 3 + len 
	bleMsg.packet.data[bleMsg.packet.att.loadLength + 5] = Mid_Ble_CheckSum(&bleMsg.packet);
    Mid_Ble_SendMsg(&bleMsg);
#else
    BLE_Stack_Interact(BLE_AVD_NAME_SET,name,u8len);
#endif
}

//**********************************************************************
// ��������: ������������
// ������������ӱ��    
// ���ز�����
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
// ��������: �������ʳ����µ�����ֵ
// ���������ʵʱ����ֵ    
// ���ز�����
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
// ��������: �������Ӽ��
// ���������
// ���ز�����
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
// ��������: ����SN��
// ���������
// protocal: Э������ָ��
// ���ز�����
//**********************************************************************
void App_Protocal_RetSN(ble_msg_t *protocal)
{
     uint8           snData[30];
//    flash_task_msg_t flashMsg;

    //uint32  u32Temp;
    //uint8_t u8temp;
    uint8   u8Route;     //��¼·��
    uint8   u8FlowCtl;   //��¼���غ�

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

    protocal->packet.att.load.content.parameter[0]  = 0; //��ʮ�ֽ�
    for(uint8_t i = 1;i < 11;i++)
    {
        protocal->packet.att.load.content.parameter[i] = snData[i-1];
    }
    protocal->packet.data[protocal->packet.att.loadLength + 5]  = Mid_Ble_CheckSum(&protocal->packet); 
    Mid_Ble_SendMsg(protocal);
    vTaskDelay(2);
	
    protocal->packet.att.load.content.parameter[0] = 1; //��ʮ�ֽ�
    for(uint8_t i = 1;i < 11;i++)
    {
        protocal->packet.att.load.content.parameter[i] = snData[i+9];
    }
    protocal->packet.data[protocal->packet.att.loadLength + 5]  = Mid_Ble_CheckSum(&protocal->packet); 
    Mid_Ble_SendMsg(protocal);
}



    
    


