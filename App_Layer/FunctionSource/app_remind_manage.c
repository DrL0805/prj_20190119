#include "platform_common.h"
/* FreeRTOS includes */
#include "rtos.h"
#include "mid_common.h"
#include "mid_interface.h"
#include "app_variable.h"
#include "app_protocal.h"
#include "app_remind_manage.h"
#include "app_win_common.h"
#include "app_win_process.h"
#include "app_win_remind.h"


/*************** func declaration ********************************/
static int16_t GetSingleUTF8CodeLen(const uint8_t pInput);
static uint16_t SINGLE_UTF8_TO_UNICODE(const uint8_t* UTF8code, uint16_t *Unicode);

static void App_RemindManage_CallProcess(uint8 phoneSystem);
static void App_RemindManage_MissCallProcess(uint8 phoneSystem);
static void App_RemindManage_MessgeProcess(uint8 phoneSystem);
static void App_RemindManage_MsgDetailProcess(uint8 phoneSystem,uint8_t msg_type);

/*******************macro define*******************/
#define 	REMIND_TYPE_MAX 	3
#define 	SECT_END 			0x0000 		//����ν�β�жϷ�

typedef enum 
{
	FLOW_START 			= 0x00,
	FLOW_PROCESS 		= 0x01,
	FLOW_END			= 0xFF,
}flow_state;

typedef enum
{
	PHONE_FAMILIAR	= 0,	//������ϵ��
	PHONE_UNFAMILIAR,
}phone_relation_type;

typedef enum
{
	QQ_MESAGE 		= 0,
	WECHAT_MESAGE   = 1,	//΢��
	PHONE_CALL_IN 	= 10,	// ����
	PHONE_MESSAGE   = 11,	// ����
	MISS_PHONE_CALL = 12,	// δ������
	FAMILIAR_PHONE_CALL = 16,	// ��ϵ������
    TIM_MESAGE = 17,		// TIM
}detail_remind_type;


/************** variable define *****************************/
app_remind_msg_t appRemindMsg;

//δ��/δ������Ŀ¼
uint32_t 	misssRemindCatalog[REMIND_TYPE_MAX][APP_REMIND_MSG_MAX_LIST];
//δ��/δ������
uint8_t 	misssRemindNum[REMIND_TYPE_MAX];


uint8_t 	protocalCache[APP_REMIND_MSG_MAX_LENGHT];
uint8_t 	protocalCacheCnt;
uint8_t 	flowCnt;
uint8_t 	firstPackFlag;


//**********************************************************************
// ��������: �ж���Ϣ�������Ƿ���null:null
// �����������
// ���ز�����true:��ϢΪnull
//**********************************************************************
static uint8 App_RemindManag_IsNULL(void)
{
    //0x6E,0x75,0x6C,0x6C,0x3A,0x6E,0x75,0x6C,0x6C
    uint8 u8null[10] = "null:null";
    uint8 u8i,u8j,u8z;

    for(u8i = 0; u8i < protocalCacheCnt; u8i++)
    {
        u8j = 0;
        if(protocalCache[u8i] == u8null[u8j])
        {
            for(u8z = u8i + 1,u8j = 1; u8z < protocalCacheCnt; u8z++,u8j++)
            {
                if(protocalCache[u8z] != u8null[u8j])
                    break;
                //��Ϣ��������null:null
                if(u8j >= 8)
                    return TRUE;
            }
        }
    }
    return FALSE;
}

//**********************************************************************
// ��������:	��ʼ��������Ϣ�洢
// ���������	��
// ���ز�����	��
void App_RemindManage_SaveInit(void)
{
	uint8_t 	i;
	uint16_t 	sizeTemp;

	for (i = 0; i < REMIND_TYPE_MAX; i++)
	{
		memset(misssRemindCatalog[i],0,APP_REMIND_MSG_MAX_LIST);
	}

	for (i = 0; i < REMIND_TYPE_MAX; i++)
	{
		misssRemindNum[i] 	= 0;
	}
	firstPackFlag = 1;
	sizeTemp = sizeof(appRemindMsg);//������Ϣ�Ĵ洢����

//	Mid_RemindMsg_ManageInit();
//	Mid_RemindMsg_Init(REMIND_MSG_BRIEF, 2*REMIND_DATA_SECTOR_LENGTH, sizeTemp);//����2����
//	Mid_RemindMsg_Init(REMIND_MSG_WEIXIN, 2*REMIND_DATA_SECTOR_LENGTH, sizeTemp);//����2����
//	Mid_RemindMsg_Init(REMIND_CALL_PHONE, 2*REMIND_DATA_SECTOR_LENGTH, sizeTemp);//����2����
}

//**********************************************************************
// ��������:	δ������������Ϣ��������ȡ
// ��������� 	msgClassify��       ��Ϣ����
// 				listNum�� 			��Ϣ��ţ���¼���ݽ��յ����Ⱥ�˳���1��ʼ���
// ���ز�����	��

void App_RemindManage_MissRemindSumRead(uint32_t msgClassify,uint8_t *listSumNum)
{	
	*listSumNum = misssRemindNum[msgClassify];
}

//**********************************************************************
// ��������:	������Ϣ��ȡ
// ���������	dailyMissRemindMsg: �洢��Ϣ�Ľṹ��
// 				msgClassify��       ��Ϣ����
// 				listNum�� 			��Ϣ��ţ���¼���ݽ��յ����Ⱥ�˳���1��ʼ���
// ���ز�����	��

void App_RemindManage_MissRemindRead(app_remind_msg_t *dailyMissRemindMsg,uint32_t msgClassify,uint8_t listNum)
{	
//	if (msgClassify > REMIND_CLASSIFY_NUM)
//	{
//		return;
//	}

//	if (misssRemindNum[msgClassify] == 0)
//	{
//		return;
//	}

//	if (listNum == 0 || listNum > APP_REMIND_MSG_MAX_LIST)
//	{
//		return;
//	}

//	Mid_RemindMsg_Read(dailyMissRemindMsg->data,misssRemindCatalog[msgClassify][listNum - 1],msgClassify);	
}

//**********************************************************************
// ��������:	ɾ��һ��������Ϣ
// ���������	msgClassify: ɾ��������
// 				listnum�� 	 ɾ���ı�ţ���¼���ݽ��յ����Ⱥ�˳���1��ʼ���
// ���ز�����	��

void App_RemindManage_MissRemindDelete(uint32_t msgClassify,uint8_t listnum)
{	
//	uint8_t 	i;

//	if (msgClassify > REMIND_CLASSIFY_NUM)
//	{
//		return;
//	}

//	if (misssRemindNum[msgClassify] == 0)
//	{
//		return;
//	}

//	if (listnum == 0)
//	{
//		return;
//	}
//	//Ŀ¼����
//	for (i = listnum - 1; i < misssRemindNum[msgClassify] - 1; ++i)
//	{
//		misssRemindCatalog[msgClassify][i] 	= misssRemindCatalog[msgClassify][i + 1];
//	}
//	misssRemindCatalog[msgClassify][i] 		= 0;
//	misssRemindNum[msgClassify] 			-= 1;	
}

//**********************************************************************
// ��������:	��ȡ��ǰ�����������Ϣ
// ���������	protocal�� Э������
// ���ز�����	��
void App_RemindManage_DetailRemindRead(app_remind_msg_t *detailRemindMsg)
{
	*detailRemindMsg  = appRemindMsg;
}

//**********************************************************************
// ��������:	������Ϣ���������Ȱ����ݴ���CACHE
// ���������	protocal�� Э������
// ���ز�����	��

void App_RemindManage_Process(protocal_msg_t protocal,uint8 phoneSystem)
{
	uint8_t 	i;
	//uint16_t 	ret;
 	//uint32_t 	catalogStartAddrTemp = 0;

	switch(protocal.att.load.content.parameter[1])
	{
		case FLOW_END:
			if (firstPackFlag)//ֻ��һ�������������ر���
			{
				memset(protocalCache,0,APP_REMIND_MSG_MAX_LENGHT);
				flowCnt 			= 0;
				protocalCacheCnt 	= 0;
			}
			
			for(i = 2; i <= protocal.att.loadLength - 4; i ++)
			{	
				if (protocalCacheCnt >= APP_REMIND_MSG_MAX_LENGHT)
				{
					break;
				}
				protocalCache[protocalCacheCnt] = protocal.att.load.content.parameter[i];
				protocalCacheCnt ++;    
			}
			firstPackFlag = 1;	//���յ�β������һ�ν���ӦΪ��һ�ΰ���ʼ

			//fix :�����Ϣ��������Ч�ģ��Ͳ���ʾ,�������ʾ"null:null"
			if(App_RemindManag_IsNULL() == TRUE)
				return;
			//fix: 2018.6.26

			// utf-8����ԭʼ���ݴ�ӡ
//			for(uint32_t t = 0; t < protocalCacheCnt;t++)
//			{
//				APP_REMIND_RTT_LOG(0,"%02X ",protocalCache[t]);
//			}APP_REMIND_RTT_LOG(0,"\r\n");
			
			switch(protocal.att.load.content.parameter[0])//��Ϣ�����꣬�ֱ�������
			{
				case PHONE_CALL_IN:					
				case FAMILIAR_PHONE_CALL:
					App_RemindManage_CallProcess(phoneSystem);
					break;
				case MISS_PHONE_CALL:
					App_RemindManage_MissCallProcess(phoneSystem);
					//            //fix :��δ�����磬��timeģʽ��δ��ʾδ������ͼ��
					//            App_Remind_Win_MissCallAdd();
					//            //fix :2018.7.16
					break;
				case PHONE_MESSAGE:
					App_RemindManage_MessgeProcess(phoneSystem);
					break;
				case TIM_MESAGE:
				case WECHAT_MESAGE:
				case QQ_MESAGE:
					App_RemindManage_MsgDetailProcess(phoneSystem,protocal.att.load.content.parameter[0]);
					break;
				default:
					App_RemindManage_MsgDetailProcess(phoneSystem,protocal.att.load.content.parameter[0]);
					break;
			}
			break;
		case FLOW_START://����ǵ�һ�����ݣ���0������
			memset(protocalCache,0,APP_REMIND_MSG_MAX_LENGHT);
			flowCnt 			= 0;
			protocalCacheCnt 	= 0;
			firstPackFlag 		= 0;//�װ����յ������ʶ

		/*
		//��ȡutc��ʼһ�������ݴ洢
		appRemindMsg.remindMsg.utc = Mid_Rtc_ReadCurUtc();

		switch(protocal.att.load.content.parameter[0])
		{
			case PHONE_CALL_IN:		
			case MISS_PHONE_CALL:
			case FAMILIAR_PHONE_CALL:
			ret 	= Mid_RemindMsg_CreatNewList(appRemindMsg.remindMsg.utc,REMIND_CALL_PHONE,&catalogStartAddrTemp);
			if (ret == 0)
			{
				if (misssRemindNum[REMIND_CALL_PHONE] < APP_REMIND_MSG_MAX_LIST)//δ��������¼����
				{
					misssRemindCatalog[REMIND_CALL_PHONE][misssRemindNum[REMIND_CALL_PHONE]] = catalogStartAddrTemp;
					misssRemindNum[REMIND_CALL_PHONE] 			+= 1;
				}
				else
				{
					for (i = 0; i < APP_REMIND_MSG_MAX_LIST - 1; i++)//��¼Խ�磬ɾ������ļ�¼
					{
						misssRemindCatalog[REMIND_CALL_PHONE][i] = misssRemindCatalog[REMIND_CALL_PHONE][i + 1];
					}
					misssRemindCatalog[REMIND_CALL_PHONE][i] 	 = catalogStartAddrTemp;
				}				
			}
			break;

			case PHONE_MESSAGE:
			ret = Mid_RemindMsg_CreatNewList(appRemindMsg.remindMsg.utc,REMIND_MSG_BRIEF,&catalogStartAddrTemp);
			if (ret == 0)
			{
				if (misssRemindNum[REMIND_MSG_BRIEF] < APP_REMIND_MSG_MAX_LIST)//δ��������¼����
				{
					misssRemindCatalog[REMIND_MSG_BRIEF][misssRemindNum[REMIND_MSG_BRIEF]] = catalogStartAddrTemp;
					misssRemindNum[REMIND_MSG_BRIEF] 			+= 1;
				}
				else
				{
					for (i = 0; i < APP_REMIND_MSG_MAX_LIST - 1; i++)//��¼Խ�磬ɾ������ļ�¼
					{
						misssRemindCatalog[REMIND_MSG_BRIEF][i] = misssRemindCatalog[REMIND_MSG_BRIEF][i + 1];
					}
					misssRemindCatalog[REMIND_MSG_BRIEF][i] 	 = catalogStartAddrTemp;
				}				
			}
			break;

			case WECHAT_MESAGE:
			case QQ_MESAGE:
			ret = Mid_RemindMsg_CreatNewList(appRemindMsg.remindMsg.utc,REMIND_MSG_WEIXIN,&catalogStartAddrTemp);
			if (ret == 0)
			{
				if (misssRemindNum[REMIND_MSG_WEIXIN] < APP_REMIND_MSG_MAX_LIST)//δ��������¼����
				{
					misssRemindCatalog[REMIND_MSG_WEIXIN][misssRemindNum[REMIND_MSG_WEIXIN]] = catalogStartAddrTemp;
					misssRemindNum[REMIND_MSG_WEIXIN] 			+= 1;
				}
				else
				{
					for (i = 0; i < APP_REMIND_MSG_MAX_LIST - 1; i++)//��¼Խ�磬ɾ������ļ�¼
					{
						misssRemindCatalog[REMIND_MSG_WEIXIN][i] = misssRemindCatalog[REMIND_MSG_WEIXIN][i + 1];
					}
					misssRemindCatalog[REMIND_MSG_WEIXIN][i] 	 = catalogStartAddrTemp;
				}				
			}
			break;

			default:
			break;
		}
		*/
		//break; ����default�Ĳ���


		default://ֻ�������뻺��		
			if (flowCnt != protocal.att.load.content.parameter[1])
			{
				return;
			}

			for(i = 2; i <= protocal.att.loadLength - 4; i ++)
			{	
				if (protocalCacheCnt >= APP_REMIND_MSG_MAX_LENGHT)
				{
					break;
				}
				protocalCache[protocalCacheCnt] = protocal.att.load.content.parameter[i];
				protocalCacheCnt ++;    
			}
			flowCnt ++;
			break;
	}
}

//**********************************************************************
// ��������:	�����������
// ���������	��
// ���ز�����	��
void App_RemindManage_CallProcess(uint8 phoneSystem)
{
	// uint32_t 	msgLenghtCnt 	= 0;
	uint8_t 	phoneType;
	uint8_t 	utfbytesIndex 	= 0;
	uint16_t 	utfbytes,utfbytes1,utfbytes2;
	uint16_t 	unicodeTemp,unicodeTemp1,unicodeTemp2;

	uint8_t 	endFlagNum = 0;
	
	uint32_t 	size = 0;

//	menuMessage message={0};

	//�����Ϣ�ṹ��
	size = sizeof(appRemindMsg);
	memset(appRemindMsg.data,0,size);
	
	//memset(appRemindMsg.data,0,APP_REMIND_MSG_MAX_LENGHT);
	

	//��UTF8ת����Unicode����,ʹ��ǰ��λ��Ϊ�Ƿ���ͨѶ¼�ж�
	if (phoneSystem == IOS)
	{
		if (*(protocalCache + utfbytesIndex) == 0xE2)//�ɣϣӺ���ǰ����2��80������
		{
			utfbytesIndex += 3;
		}
	}

	utfbytes 		= SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp);
	utfbytesIndex 	+= utfbytes;
	utfbytes1 		= SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp1);
	utfbytesIndex 	+= utfbytes1;
	utfbytes2 		= SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp2);
	utfbytesIndex 	+= utfbytes2;
	

	if (utfbytes == 1 && utfbytes1 == 1 && utfbytes2 == 1)
	{
		//��ʼ3����Ϊ����
		if (((unicodeTemp >= 0x0030 && unicodeTemp <= 0x0039) || unicodeTemp == 0x002B) && 
			(unicodeTemp1 >= 0x0030 && unicodeTemp1 <= 0x0039) &&
			(unicodeTemp2 >= 0x0030 && unicodeTemp2 <= 0x0039))
		{
			phoneType = PHONE_UNFAMILIAR;
		}
		else
		{
			phoneType 		= PHONE_FAMILIAR;
			utfbytesIndex 	= 0;
		}
	}
	else
	{
		phoneType 			= PHONE_FAMILIAR;
		utfbytesIndex 		= 0;
	}

    switch(phoneType)
    {
    	//���뿪ʼ������ͨѶ¼��ϵ��
    	case PHONE_UNFAMILIAR:
    	// msgLenghtCnt 	+= 3; 
    	appRemindMsg.remindMsg.phonenumber[appRemindMsg.remindMsg.phonenumberlen] = unicodeTemp;
    	appRemindMsg.remindMsg.phonenumberlen ++;
    	appRemindMsg.remindMsg.phonenumber[appRemindMsg.remindMsg.phonenumberlen] = unicodeTemp1;
    	appRemindMsg.remindMsg.phonenumberlen ++;
    	appRemindMsg.remindMsg.phonenumber[appRemindMsg.remindMsg.phonenumberlen] = unicodeTemp2;
    	appRemindMsg.remindMsg.phonenumberlen ++;
    	//�������
    	for(;;)
    	{
    		if (utfbytesIndex >= protocalCacheCnt)
    		{
    			break;
    		}

    		utfbytes 	 = SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp);

			if (utfbytes == 1)//ACSII��
			{
				if (unicodeTemp >= 0x0030 && unicodeTemp <= 0x0039)
	    		{
	    			appRemindMsg.remindMsg.phonenumber[appRemindMsg.remindMsg.phonenumberlen] = unicodeTemp;
	    			appRemindMsg.remindMsg.phonenumberlen ++;
	    		}else if(unicodeTemp == SECT_END)
				{
					utfbytesIndex 	+= utfbytes;
					break;
				}
	    		utfbytesIndex 	+= utfbytes;
			}
			else
			{
				break;//��ACSII��
			}
    	}

    	//�����ؽ���
    	for(;;)
    	{
    		if (utfbytesIndex >= protocalCacheCnt)
    		{
    			break;
    		}
    		utfbytes 	 = SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp); 

			if(utfbytes == 0)//�쳣����
				break;
			
    		switch(endFlagNum)
			{
				case 0://����-������
				if (utfbytes == 1 && (unicodeTemp == SECT_END))//���ַ�
	    		{
	    			endFlagNum += 1;
	    		}
	    		else
	    		{
	    			appRemindMsg.remindMsg.content.u.ascription[appRemindMsg.remindMsg.content.u.ascriptionlen] = unicodeTemp;
					appRemindMsg.remindMsg.content.u.ascriptionlen ++;
	    		}	    		
				break;

				case 1://����-����绰
				if (utfbytes == 1 && (unicodeTemp == SECT_END))//���ַ�
	    		{
	    			endFlagNum += 1;
	    		}
	    		else
	    		{
	    			appRemindMsg.remindMsg.content.u.detail[appRemindMsg.remindMsg.content.u.detaillen] = unicodeTemp;
					appRemindMsg.remindMsg.content.u.detaillen ++;
	    		}
				break;

				case 2://������

				break;

				default:
				break;
			}
			utfbytesIndex 	+= utfbytes;
    	}
//    	message.val   = phoneSystem;   
//    	message.state = NEW_REMIND_STATE;           
//        message.op    = PHONE_CALL_DETAIL_REMIND;
//        WinDowHandle  = App_Window_Process(WinDowHandle,TAG_REMIND,message);
    	break;

    	//�Ǻ��뿪ͷ����ͨѶ¼��ϵ��
    	case PHONE_FAMILIAR:
    	for(;;)
    	{
    		if (utfbytesIndex >= protocalCacheCnt)
    		{
    			break;
    		}
    		utfbytes 	 = SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp);  

    		if(utfbytes == 0)//�쳣����
				break;

    		switch(endFlagNum)
			{
				case 0://����
				if (utfbytes == 1 && (unicodeTemp == SECT_END))//���ַ�
	    		{
	    			endFlagNum += 1;
	    		}
	    		else
	    		{
	    			appRemindMsg.remindMsg.name[appRemindMsg.remindMsg.namelen] = unicodeTemp;
					appRemindMsg.remindMsg.namelen ++;
	    		}	    		
				break;

				case 1://����-����绰
				if (utfbytes == 1 && (unicodeTemp == SECT_END))//���ַ�
	    		{
	    			endFlagNum += 1;
	    		}
	    		else
	    		{
	    			appRemindMsg.remindMsg.content.u.detail[appRemindMsg.remindMsg.content.u.detaillen] = unicodeTemp;
					appRemindMsg.remindMsg.content.u.detaillen ++;
	    		}
				break;

				case 2://������

				break;

				default:
				break;
			}
			utfbytesIndex 	+= utfbytes;
    	}
//    	message.val   = phoneSystem;          
//        message.op    = PHONE_CALL_DETAIL_REMIND;
//        WinDowHandle  = App_Window_Process(WinDowHandle,TAG_REMIND,message);
    	break;

    	default:
    	break;
    }
}

//**********************************************************************
// ��������:	δ�������������
// ���������	��
// ���ز�����	��
void App_RemindManage_MissCallProcess(uint8 phoneSystem)
{
	// uint32_t 	msgLenghtCnt 	= 0;
	uint8_t 	phoneType;
	uint8_t 	utfbytesIndex 	= 0;
	uint16_t 	utfbytes,utfbytes1,utfbytes2;
	uint16_t 	unicodeTemp,unicodeTemp1,unicodeTemp2;

	uint8_t 	endFlagNum = 0;
	uint32_t 	size = 0;


	//�����Ϣ�ṹ��
	size = sizeof(appRemindMsg);
	memset(appRemindMsg.data,0,size);

	//��UTF8ת����Unicode����,ʹ��ǰ��λ��Ϊ�Ƿ���ͨѶ¼�ж�
	if (phoneSystem == IOS)
	{
		if (*(protocalCache + utfbytesIndex) == 0xE2)//�ɣϣӺ���ǰ����2��80������
		{
			utfbytesIndex += 3;
		}
	}

	utfbytes 		= SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp);
	utfbytesIndex 	+= utfbytes;
	utfbytes1 		= SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp1);
	utfbytesIndex 	+= utfbytes1;
	utfbytes2 		= SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp2);
	utfbytesIndex 	+= utfbytes2;

	if (utfbytes == 1 && utfbytes1 == 1 && utfbytes2 == 1)
	{
		//��ʼ3����Ϊ���ֻ���+
		if (((unicodeTemp >= 0x0030 && unicodeTemp <= 0x0039) || unicodeTemp == 0x002B)&& 
			(unicodeTemp1 >= 0x0030 && unicodeTemp1 <= 0x0039) &&
			(unicodeTemp2 >= 0x0030 && unicodeTemp2 <= 0x0039))
		{
			phoneType = PHONE_UNFAMILIAR;
		}
		else
		{
			phoneType 		= PHONE_FAMILIAR;
			utfbytesIndex	= 0;
		}
	}
	else
	{
		phoneType 			= PHONE_FAMILIAR;
		utfbytesIndex		= 0;
	}

    switch(phoneType)
    {
    	//���뿪ʼ������ͨѶ¼��ϵ��
    	case PHONE_UNFAMILIAR: 
    	// msgLenghtCnt 	+= 3;
    	appRemindMsg.remindMsg.phonenumber[appRemindMsg.remindMsg.phonenumberlen] = unicodeTemp;
    	appRemindMsg.remindMsg.phonenumberlen ++;
    	appRemindMsg.remindMsg.phonenumber[appRemindMsg.remindMsg.phonenumberlen] = unicodeTemp1;
    	appRemindMsg.remindMsg.phonenumberlen ++;
    	appRemindMsg.remindMsg.phonenumber[appRemindMsg.remindMsg.phonenumberlen] = unicodeTemp2;
    	appRemindMsg.remindMsg.phonenumberlen ++;

    	for(;;)//�������
    	{
    		if (utfbytesIndex >= protocalCacheCnt)
    		{
    			break;
    		}

    		utfbytes 	 = SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp);

			if (utfbytes == 1)//ACSII��
			{
				if (unicodeTemp >= 0x0030 && unicodeTemp <= 0x0039)
	    		{
	    			appRemindMsg.remindMsg.phonenumber[appRemindMsg.remindMsg.phonenumberlen] = unicodeTemp;
	    			appRemindMsg.remindMsg.phonenumberlen ++;
	    		}else if(unicodeTemp == SECT_END)
				{
					utfbytesIndex 	+= utfbytes;
					break;
				}
	    		utfbytesIndex 	+= utfbytes;
			}
			else
			{
				break;//��ACSII��
			}
    	}

    	//���ݽ���
    	for(;;)
    	{
    		if (utfbytesIndex >= protocalCacheCnt)
    		{
    			break;
    		}
    		utfbytes 	 = SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp);   	

			if(utfbytes == 0)//�쳣����
				break;

    		switch(endFlagNum)
			{
				case 0://����-������
				if (utfbytes == 1 && (unicodeTemp == SECT_END))//���ַ�
	    		{
	    			endFlagNum += 1;
	    		}
	    		else
	    		{
	    			appRemindMsg.remindMsg.content.u.ascription[appRemindMsg.remindMsg.content.u.ascriptionlen] = unicodeTemp;
					appRemindMsg.remindMsg.content.u.ascriptionlen ++;
	    		}	    		
				break;

				case 1://����-����绰
				if (utfbytes == 1 && (unicodeTemp == SECT_END))//���ַ�
	    		{
	    			endFlagNum += 1;
	    		}
	    		else
	    		{
	    			appRemindMsg.remindMsg.content.u.detail[appRemindMsg.remindMsg.content.u.detaillen] = unicodeTemp;
					appRemindMsg.remindMsg.content.u.detaillen ++;
	    		}
				break;

				case 2://������

				break;

				default:
				break;
			}
			utfbytesIndex 	+= utfbytes;
    	}
    	break;

    	//�Ǻ��뿪ͷ����ͨѶ¼��ϵ��
    	case PHONE_FAMILIAR:
    	for(;;)
    	{
    		if (utfbytesIndex >= protocalCacheCnt)
    		{
    			break;
    		}
    		utfbytes 	 = SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp); 

			if(utfbytes == 0)//�쳣����
				break;

    		switch(endFlagNum)
			{
				case 0://����
				if (utfbytes == 1 && (unicodeTemp == SECT_END))//���ַ�
	    		{
	    			endFlagNum += 1;
	    		}
	    		else
	    		{
	    			appRemindMsg.remindMsg.name[appRemindMsg.remindMsg.namelen] = unicodeTemp;
					appRemindMsg.remindMsg.namelen ++;
	    		}	    		
				break;

				case 1://����-����绰
				if (utfbytes == 1 && (unicodeTemp == SECT_END))//���ַ�
	    		{
	    			endFlagNum += 1;
	    		}
	    		else
	    		{
	    			appRemindMsg.remindMsg.content.u.detail[appRemindMsg.remindMsg.content.u.detaillen] = unicodeTemp;
					appRemindMsg.remindMsg.content.u.detaillen ++;
	    		}
				break;

				case 2://������

				break;

				default:
				break;
			}
			utfbytesIndex 	+= utfbytes;
    	}
    	break;

    	default:
    	break;
    }
    #if 0
    //�����꣬����
	Mid_RemindMsg_SaveList((uint8_t *)(appRemindMsg.data + 6), APP_REMIND_MSG_VALID_MAX_LENGHT, REMIND_CALL_PHONE);
	Mid_RemindMsg_SaveListEnd(REMIND_CALL_PHONE);
	#endif

}

//**********************************************************************
// ��������:	������Ϣ��������
// ���������	��
// ���ز�����	��
void App_RemindManage_MessgeProcess(uint8 phoneSystem)
{
	// uint32_t 	msgLenghtCnt 	= 0;
	uint8_t 	phoneType;
	uint8_t 	utfbytesIndex 	= 0;
	uint16_t 	utfbytes,utfbytes1,utfbytes2;
	uint16_t 	unicodeTemp,unicodeTemp1,unicodeTemp2;
	uint8_t 	endFlagNum = 0;
	uint32_t 	size = 0;
//	menuMessage message={0};


	//�����Ϣ�ṹ��
	size = sizeof(appRemindMsg);
	memset(appRemindMsg.data,0,size);

	//��UTF8ת����Unicode����,ʹ��ǰ��λ��Ϊ�Ƿ���ͨѶ¼�ж�
	if (phoneSystem == IOS)
	{
		if (*(protocalCache + utfbytesIndex) == 0xE2)//�ɣϣӺ���ǰ����2��80������
		{
			utfbytesIndex += 3;
		}
	}
	utfbytes 		= SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp);
	utfbytesIndex 	+= utfbytes;
	utfbytes1 		= SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp1);
	utfbytesIndex 	+= utfbytes1;
	utfbytes2 		= SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp2);
	utfbytesIndex 	+= utfbytes2;

	if (utfbytes == 1 && utfbytes1 == 1 && utfbytes2 == 1)
	{
		//��ʼ3����Ϊ����
		if (((unicodeTemp >= 0x0030 && unicodeTemp <= 0x0039) || unicodeTemp == 0x002B)&& 
			(unicodeTemp1 >= 0x0030 && unicodeTemp1 <= 0x0039) &&
			(unicodeTemp2 >= 0x0030 && unicodeTemp2 <= 0x0039))
		{
			phoneType = PHONE_UNFAMILIAR;
		}
		else
		{
			phoneType 		= PHONE_FAMILIAR;
			utfbytesIndex	= 0;
		}
	}
	else
	{
		phoneType 			= PHONE_FAMILIAR;
		utfbytesIndex		= 0;
	}

	switch(phoneType)
	{
		case PHONE_UNFAMILIAR: 	//���뿪ʼ������ͨѶ¼��ϵ��
			// msgLenghtCnt 	+= 3;
			appRemindMsg.remindMsg.phonenumber[appRemindMsg.remindMsg.phonenumberlen] = unicodeTemp;
			appRemindMsg.remindMsg.phonenumberlen ++;
			appRemindMsg.remindMsg.phonenumber[appRemindMsg.remindMsg.phonenumberlen] = unicodeTemp1;
			appRemindMsg.remindMsg.phonenumberlen ++;
			appRemindMsg.remindMsg.phonenumber[appRemindMsg.remindMsg.phonenumberlen] = unicodeTemp2;
			appRemindMsg.remindMsg.phonenumberlen ++;
			
			for(;;)	//�������
			{
				if (utfbytesIndex >= protocalCacheCnt)
				{
					break;
				}

				utfbytes 	 = SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp);

				if (utfbytes == 1)//ACSII��
				{
					if (unicodeTemp >= 0x0030 && unicodeTemp <= 0x0039)
					{
						appRemindMsg.remindMsg.phonenumber[appRemindMsg.remindMsg.phonenumberlen] = unicodeTemp;
						appRemindMsg.remindMsg.phonenumberlen ++;

						//if(appRemindMsg.remindMsg.phonenumberlen >= APP_REMIND_PHONENUM_MAX_LENGHT)
						//	break;			
					}
					else if(unicodeTemp == SECT_END)	
					{
						utfbytesIndex 	+= utfbytes;
						break;
					}
					utfbytesIndex 	+= utfbytes;
				}
				else
				{
					break;//��ACSII��
				}
			}

	
			for(;;)	//��Ϣ���ݽ���
			{
				if (utfbytesIndex >= protocalCacheCnt)
				{
					break;
				}
				utfbytes 	 = SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp);  

				if(utfbytes == 0)//�쳣����
					break;

				switch(endFlagNum)
				{
					case 0://����-��Ϣ����
						if (utfbytes == 1 && (unicodeTemp == SECT_END))//���ַ�
						{
							endFlagNum += 1;
						}
						else
						{
							appRemindMsg.remindMsg.content.u.detail[appRemindMsg.remindMsg.content.u.detaillen] = unicodeTemp;
							appRemindMsg.remindMsg.content.u.detaillen ++;
							if(appRemindMsg.remindMsg.content.u.detaillen >= APP_REMIND_DATAIL_MAX_LENGHT)
							{
								endFlagNum += 1;
							}
						}	    		
						break;
					case 1: break;
					default: break;
				}
				utfbytesIndex 	+= utfbytes;
			}
			//    	message.val   = phoneSystem;   
			//    	message.state = NEW_REMIND_STATE;           
			//        message.op    = MESSAGE_DETAIL_REMIND;
			//        WinDowHandle  = App_Window_Process(WinDowHandle,TAG_REMIND,message);
			break;

		//�Ǻ��뿪ͷ����ͨѶ¼��ϵ��
		case PHONE_FAMILIAR:
			for(;;)
			{
				if (utfbytesIndex >= protocalCacheCnt)
				{
					break;
				}
				utfbytes 	 = SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp);   		

				if(utfbytes == 0)//�쳣����
					break;

				switch(endFlagNum)
				{
					case 0://����
						if (utfbytes == 1 && (unicodeTemp == SECT_END))//���ַ�
						{
							endFlagNum += 1;
						}
						else
						{
							appRemindMsg.remindMsg.name[appRemindMsg.remindMsg.namelen] = unicodeTemp;
							appRemindMsg.remindMsg.namelen ++;
							if(appRemindMsg.remindMsg.namelen >= APP_REMIND_NANE_MAX_LENGHT)
							{
								endFlagNum += 1;
							}
						}	    		
						break;

					case 1://����-��Ϣ����
						if (utfbytes == 1 && (unicodeTemp == SECT_END))//���ַ�
						{
							endFlagNum += 1;
						}
						else
						{
							appRemindMsg.remindMsg.content.u.detail[appRemindMsg.remindMsg.content.u.detaillen] = unicodeTemp;
							appRemindMsg.remindMsg.content.u.detaillen ++;
							if(appRemindMsg.remindMsg.content.u.detaillen >= APP_REMIND_DATAIL_MAX_LENGHT)
							{
								endFlagNum += 1;
							}
						}
						break;
					case 2: break;
					default: break;
				}
				utfbytesIndex 	+= utfbytes;
			}
			//    	message.val   = phoneSystem;   
			//    	message.state = NEW_REMIND_STATE;           
			//        message.op    = MESSAGE_DETAIL_REMIND;
			//        WinDowHandle  = App_Window_Process(WinDowHandle,TAG_REMIND,message);
			break;
		default: break;
	}

	for(uint32_t t = 0; t < appRemindMsg.remindMsg.phonenumberlen;t++)
	{
		APP_REMIND_RTT_LOG(0,"\\u%04X ", appRemindMsg.remindMsg.phonenumber[t]);
	}APP_REMIND_RTT_LOG(0,"\r\n");

	// ��ӡ��Ϣ����Unicode����
	for(uint32_t t = 0; t < appRemindMsg.remindMsg.content.u.detaillen;t++)
	{
		APP_REMIND_RTT_LOG(0,"\\u%04X ", appRemindMsg.remindMsg.content.u.detail[t]);
	}APP_REMIND_RTT_LOG(0,"\r\n");		
	
    #if 0
    //�����꣬����
	Mid_RemindMsg_SaveList((uint8_t *)(appRemindMsg.data + 5), APP_REMIND_MSG_VALID_MAX_LENGHT, REMIND_MSG_BRIEF);
	Mid_RemindMsg_SaveListEnd(REMIND_MSG_BRIEF);	
	#endif
}

//**********************************************************************
// ��������:	΢����Ϣ��������
// ���������	��
// ���ز�����	��
static void App_RemindManage_MsgDetailProcess(uint8 phoneSystem,uint8_t msg_type)
{
	// uint32_t 	msgLenghtCnt 	= 0;
	uint8_t 	utfbytesIndex 	= 0;
	uint16_t 	utfbytes;
	uint16_t 	unicodeTemp;
	uint8_t 	endFlagNum = 0;
	uint32_t 	size = 0;
//	menuMessage message={0};

	//�����Ϣ�ṹ��
	size = sizeof(appRemindMsg);
	memset(appRemindMsg.data,0,size);

	for(;;)
	{
		if (utfbytesIndex >= protocalCacheCnt)
		{
			break;
		}
		utfbytes 	 = SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp);   		
		
		if(utfbytes == 0)//�쳣����
			break;

		switch(endFlagNum)
		{
			case 0://΢����
            {
                switch(msg_type)
                {
                    case WECHAT_MESAGE:
                    if (utfbytes == 1 && (unicodeTemp == SECT_END))//ð��
                    {
                        endFlagNum += 1;
                    }
                    else
                    {
                        appRemindMsg.remindMsg.name[appRemindMsg.remindMsg.namelen] = unicodeTemp;
                        appRemindMsg.remindMsg.namelen ++;

                        if(appRemindMsg.remindMsg.namelen >= APP_REMIND_NANE_MAX_LENGHT)
                        {
                            endFlagNum += 1;
                        }
                    }                   
                    break;
                    
                    default:
                    if (utfbytes == 1 && (unicodeTemp == SECT_END))//ð��
                    {
                        endFlagNum += 1;
                    }
                    else
                    {
                        appRemindMsg.remindMsg.name[appRemindMsg.remindMsg.namelen] = unicodeTemp;
                        appRemindMsg.remindMsg.namelen ++;

                        if(appRemindMsg.remindMsg.namelen >= APP_REMIND_NANE_MAX_LENGHT)
                        {
                            endFlagNum += 1;
                        }
                    }
                    break;
                }
            }  		
			break;

			case 1://����-��Ϣ����
			if (utfbytes == 1 && (unicodeTemp == SECT_END))//���ַ�
    		{
    			endFlagNum += 1;
    		}
    		else
    		{
    			appRemindMsg.remindMsg.content.u.detail[appRemindMsg.remindMsg.content.u.detaillen] = unicodeTemp;
				appRemindMsg.remindMsg.content.u.detaillen ++;
				
				if(appRemindMsg.remindMsg.content.u.detaillen >= APP_REMIND_DATAIL_MAX_LENGHT)
				{
					endFlagNum += 1;
				}
    		}
			break;

			case 2://������

			break;

			default:
			break;
		}
		utfbytesIndex 	+= utfbytes;
	}

	// ��ӡ��Ϣ����Unicode����
	for(uint32_t t = 0; t < appRemindMsg.remindMsg.content.u.detaillen;t++)
	{
		APP_REMIND_RTT_LOG(0,"\\u%04X ", appRemindMsg.remindMsg.content.u.detail[t]);
	}APP_REMIND_RTT_LOG(0,"\r\n");	
	
	/* ֪ͨ�ϲ����Ϣ���ݽ��д��� */
//	message.state = NEW_REMIND_STATE;           
//    message.op    = UASUAL_MSG_DETAIL_REMIND;
//    WinDowHandle  = App_Window_Process(WinDowHandle,TAG_REMIND,message);
	
	#if 0
    //�����꣬����
	Mid_RemindMsg_SaveList((uint8_t *)(appRemindMsg.data + 5), APP_REMIND_MSG_VALID_MAX_LENGHT, REMIND_MSG_WEIXIN);
	Mid_RemindMsg_SaveListEnd(REMIND_MSG_WEIXIN);
	#endif	
}

//**********************************************************************
// ��������:	�������������ֵ�UTF-8���볤��
// ���������	pInput: UTF-8����
// ���ز�����	��
static int16_t GetSingleUTF8CodeLen(const uint8_t pInput)
{
	unsigned char c = pInput;
	// 0xxxxxxx ����0
	// 10xxxxxx ������
	// 110xxxxx ����2
	// 1110xxxx ����3
	// 11110xxx ����4
	// 111110xx ����5
	// 1111110x ����6
	if(c< 0x80) return 0;
	if(c>=0x80 && c<0xC0) return -1;
	if(c>=0xC0 && c<0xE0) return 2;
	if(c>=0xE0 && c<0xF0) return 3;
	if(c>=0xF0 && c<0xF8) return 4;
	if(c>=0xF8 && c<0xFC) return 5;
	if(c>=0xFC) return 6;
	return -1;
}
	
//**********************************************************************
// ��������:	��һ���ַ���UTF8����ת����Unicode(UCS-2��UCS-4)����
// ���������	UTF8code��  ��UTF-8����������
//				Unicode��   ת�����ɵ�Unicode����
// ���ز�����	�ɹ��򷵻ظ��ַ���UTF8������ռ�õ��ֽ���; ʧ���򷵻�0. 

// * ע��: 
// *     1. UTF8û���ֽ�������, ����Unicode���ֽ���Ҫ��; 
// *        �ֽ����Ϊ���(Big Endian)��С��(Little Endian)����; 
// *        ��Intel�������в���С�˷���ʾ, �ڴ˲���С�˷���ʾ. (�͵�ַ���λ) 
static uint16_t SINGLE_UTF8_TO_UNICODE(const uint8_t* UTF8code, uint16_t *Unicode)  
{  
//    assert(pInput != NULL && Unic != NULL);  
  
    // b1 ��ʾUTF-8�����pInput�еĸ��ֽ�, b2 ��ʾ�θ��ֽ�, ...  
	char b1, b2, b3, b4, b5, b6;  
  
    *Unicode = 0x0; // �� *Unic ��ʼ��Ϊȫ��  
    int utfbytes = GetSingleUTF8CodeLen(*UTF8code);  
    unsigned char *pOutput = (unsigned char *) Unicode;  
  
	switch ( utfbytes )  
    {  
		case 0:  
        *pOutput     = *UTF8code;  
        utfbytes    += 1;  
        break;  

        case 2:  
        b1 = *UTF8code;  
        b2 = *(UTF8code + 1);  
        if ( (b2 & 0xE0) != 0x80 )  
            return 0;  
        *pOutput     = (b1 << 6) + (b2 & 0x3F);  
        *(pOutput+1) = (b1 >> 2) & 0x07;  
        break;  

        case 3:  
        b1 = *UTF8code;  
        b2 = *(UTF8code + 1);  
        b3 = *(UTF8code + 2);  
        if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80) )  
            return 0;  
        *pOutput     = (b2 << 6) + (b3 & 0x3F);  
        *(pOutput+1) = (b1 << 4) + ((b2 >> 2) & 0x0F);  
        break;  

        case 4:  
        b1 = *UTF8code;  
        b2 = *(UTF8code + 1);  
        b3 = *(UTF8code + 2);  
        b4 = *(UTF8code + 3);  
        if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)  
                || ((b4 & 0xC0) != 0x80) )  
            return 0;  
        *pOutput     = (b3 << 6) + (b4 & 0x3F);  
        *(pOutput+1) = (b2 << 4) + ((b3 >> 2) & 0x0F);  
        *(pOutput+2) = ((b1 << 2) & 0x1C)  + ((b2 >> 4) & 0x03);  
        break;  

        case 5:  
        b1 = *UTF8code;  
        b2 = *(UTF8code + 1);  
        b3 = *(UTF8code + 2);  
        b4 = *(UTF8code + 3);  
        b5 = *(UTF8code + 4);  
        if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)  
                || ((b4 & 0xC0) != 0x80) || ((b5 & 0xC0) != 0x80) )  
            return 0;  
        *pOutput     = (b4 << 6) + (b5 & 0x3F);  
        *(pOutput+1) = (b3 << 4) + ((b4 >> 2) & 0x0F);  
        *(pOutput+2) = (b2 << 2) + ((b3 >> 4) & 0x03);  
        *(pOutput+3) = (b1 << 6);  
        break; 

        case 6:  
        b1 = *UTF8code;  
        b2 = *(UTF8code + 1);  
        b3 = *(UTF8code + 2);  
        b4 = *(UTF8code + 3);  
        b5 = *(UTF8code + 4);  
        b6 = *(UTF8code + 5);  
        if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)  
                || ((b4 & 0xC0) != 0x80) || ((b5 & 0xC0) != 0x80)  
                || ((b6 & 0xC0) != 0x80) )  
            return 0;  
        *pOutput     = (b5 << 6) + (b6 & 0x3F);  
        *(pOutput+1) = (b5 << 4) + ((b6 >> 2) & 0x0F);  
        *(pOutput+2) = (b3 << 2) + ((b4 >> 4) & 0x03);  
        *(pOutput+3) = ((b1 << 6) & 0x40) + (b2 & 0x3F);  
        break;  

        default:  
        return 0;  
//            break;  
    }   
    return utfbytes;  
}  

