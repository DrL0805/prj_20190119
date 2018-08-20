/**********************************************************************
**
**ģ��˵��: �Խ�BLE mid�ӿ�
**����汾���޸���־(ʱ�䣬����):
**   V1.0   2018.5.2  ����  
**   fix 1  2018.5.29 ��flash��ȡ�����㲥��һֱΪ0xff
**   fix 2  2018.5.30 ������δ���ӵ�״̬�·����ݣ��ᵼ��ϵͳ����
**   fix 3  2018.5.31 ����������æ��ʱ�򣬼����������ݣ�����ϵͳ����
**   fix 4:  2018.6.9  OTAʱIOS�ֻ�APP�쳣�˳���������Ļ���������
**********************************************************************/
#include "platform_common.h"
#include "platform_debugcof.h"
#include "sm_sys.h"
#include "rtos.h"
#include "mid_common.h"
#include "mid_front.h"

#include "mid_extflash.h"   //for BLE MAC addr
#include "flash_task.h"
#include "ble_task.h"
#include "BLE_Stack.h"  //BLE stack

#if(MID_BLE_DEBUG ==1)
#define Ble_Debug(x) SEGGER_RTT_printf x
#else
#define Ble_Debug(x)
#endif

//�Ƿ���Ҫ������������checksumУ�鹦�ܣ���ʽ�汾�����Ҫenable
//���Խ׶ο���disable
#define BLE_CHECKSUM_ENABLE        0

//�ط���Ϣ�������Ŀ
#define		RESEND_MSG_MAX			8
#define		RESEND_CNT_MAX			4

//�ط���Ϣ����Ϣ֡
typedef struct 
{
    uint8         id;       //ID
    uint8         valid;    //�ط���־
    uint8         resendCnt;//�ط�����
    TimerHandle_t timer;    //��ʱ�����
    ble_msg_t   protocal; //�ط�Э��
    func *      TimerOutCb;   //��ʱ�ص�����
}resend_msg_t;

//�����������Ϣ����
static QueueHandle_t		ble_task_msg_queue;				/**< Queue handler for ble task */
//�����������Ϣ������ʱ�������ı�����Ϣ���У�����Ҫ�������Ϣ
static QueueHandle_t		ble_task_msg_queue_send;			/**< Queue handler for ble task */


//�������ݵĻ�������
//static uint8 rxBuf[BLE_BUF_LENGTH_MAX];

//����ָ������
static protocal_cb ProtocalAnalysis = NULL;

static resend_msg_t	resendMsg[RESEND_MSG_MAX];	//ble resend buf flag

//������Ϣ
//һ������£����Ͷ�ÿ����һ������֡�����ն˶�Ӧ����������һ��Ӧ��֡�����������ǣ�echo  ���͵�����֡������ͷ
//���м���������Ϣ���ɴﵽ����Ŀ�ģ�
//?  1�����Ͷ���������δ������ն�ִ��ĳһ����ʱ��������Ϣ������������֡�Ƕ�ζ���������һ�ζ����ĳ�ʱ�ط�
//?  2�������Ͷ�����������һЩ����������˳��ִ�е�����ʱ��ʹ��������Ϣ�ɲ�׽�������
//���ص����λΪ����λ�����صĵ� 7 λ��ʾ��ˮ��
//���Ͷ�ÿ����һ֡���ݣ���ˮ���Զ���һ�����ն�����Ӧ��ʱ������ˮ��ԭ�����ظ����Ͷ�
//��ˮ�Ž��������֡�ķ���ʱ��㣬����֡�����ԡ������Ե�У��
static uint8 pipe;

//**********************************************************************
// ��������: ��ӡ�շ�����
// ���������
// ���ز�����
// ��ӡЭ��: ble_printdata(msg->packet.data,msg->packet.att.loadLength + 6)
//**********************************************************************
static void ble_printdata(uint8 *pBuf,uint8 u8len)
{
#if(MID_BLE_DEBUG ==1)
    uint16	i;
    for(i = 0; i < u8len; i++)
    {
        Ble_Debug((0,"0x%x,",pBuf[i]));
    }
    Ble_Debug((0,"\n"));
#endif
}

//**********************************************************************
// ��������: ��������MAC��ַ
// ���������
// ���ز�����
//**********************************************************************
static void BLE_SetMacAddr(void)
{
//    uint8 u8len;
//    uint8 u8MacAddr[6];
//    extflash_para_t extflash;

//    extflash.dataAddr = u8MacAddr;
//    extflash.Cb = NULL;
//	extflash.length = 6;
//    Mid_ExtFlash_ReadBleInfo(EXTFLASH_EVENT_READ_BLEMAC,&extflash);    
//    for(u8len = 0; u8len < 6; u8len++)
//    {
//        if(u8MacAddr[u8len] != 0xff)
//        {
//            break;
//        }
//    }
//    
//    if(u8len >= 6)
//    {
//        extflash.dataAddr = u8MacAddr;
//        Mid_ExtFlash_ReadBleInfo(EXTFLASH_EVENT_READ_MAC,&extflash);    
//    }

//#if(MID_BLE_DEBUG ==1)
//    Ble_Debug((0,"Mac Addr:"));
//    ble_printdata(u8MacAddr,6);
//#endif
//    BLE_Stack_SetMac(u8MacAddr);
}

//**********************************************************************
// ��������: ���������㲥��
// ���������
// ���ز�����
//**********************************************************************
static void BLE_SetAdvName(void)
{
//    uint8 u8len = 0;
//    uint8 u8AdvName[12];
//    extflash_para_t extflash;

//    extflash.dataAddr = u8AdvName;
//    //fix 1: read avd name is empty
//    extflash.result = 0;
//    extflash.length = 11;
//    //fix
//    extflash.Cb = NULL;
//    Mid_ExtFlash_ReadBleInfo(EXTFLASH_EVENT_READ_BLE_BROCAST,&extflash);    

//    while(u8len < extflash.length)
//    {
//        if((u8AdvName[u8len] == 0xFF) || (u8AdvName[u8len] == '\0'))
//        {
//            u8AdvName[u8len] = '\0';
//            break;
//        }
//        u8len++;
//    }

//    if(u8len == 0)
//        return;
//    if(u8len > 11)
//    {
//        u8len = 11;
//        u8AdvName[11] = '\0';
//    }

//#if(MID_BLE_DEBUG ==1)
//    Ble_Debug((0,"Adv Name:%s,u8len=%d\n",u8AdvName,u8len));
//#endif
//    BLE_Stack_Interact(BLE_AVD_NAME_SET,u8AdvName,u8len);
}

//**********************************************************************
// ��������: ��msg���͵���Ϣ����ble_task_msg_queue��
// ���������msg ������������Ϣ֡
// ���ز�����
//**********************************************************************
static ret_type Ble_SendMsgToQueue(ble_msg_t *msg)
{
    ret_type ret = Ret_OK;
    
    if(xPortIsInsideInterrupt() == pdTRUE)    //���ж�������event
    {
        portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
        if(uxQueueMessagesWaitingFromISR(ble_task_msg_queue) < BLE_TASK_QUEUE_SIZE)
        {
            if (errQUEUE_FULL == xQueueSendToBackFromISR( ble_task_msg_queue, (void *)msg, &xHigherPriorityTaskWoken ))
            {
                // ("errQUEUE_FULL");
            }
            if( pdTRUE ==  xHigherPriorityTaskWoken )
            {
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
        }
        else
        {
            Ble_Debug((0,"ble_task_msg_queue is full\n"));
            ret = Ret_QueueFull;
        }
    }
    else
    {
        if(uxQueueMessagesWaiting(ble_task_msg_queue) < BLE_TASK_QUEUE_SIZE)
        {
            xQueueSendToBack(ble_task_msg_queue,(void *)msg, 1);
        }
        else
        {
            Ble_Debug((0,"ble_task_msg_queue is full\n"));
            ret = Ret_QueueFull;
        }
    }
	return ret;
}

//**********************************************************************
// ��������:  �ط���Ϣ��ʼ��
//     ͨ�ų�ʱ����֪ͨͨ�ų�ʱ�¼�
// ���������    
// ���ز����� 
//**********************************************************************
static void Ble_ResendMsgInit(void)
{
    uint8		i; 
    for(i = 0; i < RESEND_MSG_MAX; i++)
    {
        resendMsg[i].valid	= 0;
    }
}

//**********************************************************************
// ��������:  �ط���Ϣ��ʱ
//     �ط���Ϣ��ʱ����֪ͨ�ط���Ϣ��ʱ�¼�
// ���������    
//         xTimer����ʱ�����
// ���ز����� 
//**********************************************************************
static void Ble_ResendTimeoutIsr(TimerHandle_t xTimer)
{
    ble_msg_t msg;

    msg.id = BLE_MSG_RESEND_TIMEOUT;
    msg.packet.resendToutId = (int32)pvTimerGetTimerID( xTimer );
    Ble_SendMsgToQueue(&msg);
}

//**********************************************************************
// ��������: ���ط���Ϣѹ�����飬��������ʱ��ʱ��
// ���������    
//         msg����Ϣ��ַ
// ���ز����� 
//  FAILURE:����ʧ��
//  SUCCESS:�����ɹ� 
//**********************************************************************
static uint16 Ble_SetResendMsg(ble_msg_t *msg)
{
	uint32 i, j;
    uint8 u8ReMsgId = msg->packet.att.flowControl & 0x7F;
    
	for(i = 0; i < RESEND_MSG_MAX; i++)
	{
	    //if msg is in resend msg queue,then break;
        if((resendMsg[i].id == u8ReMsgId) && (resendMsg[i].valid == 1))
        {
           // Ble_Debug((0,"msg has be in resend queue\n"));
           // break;
        }
		if(resendMsg[i].valid == 0)
		{
			resendMsg[i].id			= u8ReMsgId;
			resendMsg[i].valid		= 1;
			resendMsg[i].resendCnt	= 0;
			resendMsg[i].timer			= xTimerCreate("RESEND_TOUT", APP_1SEC_TICK * 4, pdTRUE,
													(void * )(i), Ble_ResendTimeoutIsr);

			for(j = 0; j < PROTOCAL_LENGTH; j++)
			{
			    resendMsg[i].protocal.packet.data[j] = msg->packet.data[j];
			}

			xTimerStart(resendMsg[i].timer, 0);
			break;
		}
	}
	if(i == RESEND_MSG_MAX)
	{
		return 0xFF;
	}
	else
	{
		return 0x00;
	}
}

//**********************************************************************
// ��������: �ط���Ϣ
//����ط�����С����������֪ͨ�¼��ط������򣬹رճ�ʱ��ʱ������ɾ���ط���Ϣ
// ���������    
//         timerId����Ϣ�ط�����
// ���ز����� 
//          ��
static void Ble_ResendToutProcess(uint8 timerId)
{
	if(timerId < RESEND_MSG_MAX)
	{
		if(resendMsg[timerId].resendCnt < RESEND_CNT_MAX)
		{
			resendMsg[timerId].resendCnt++;
            resendMsg[timerId].protocal.id = BLE_MSG_SEND;
			Ble_SendMsgToQueue(&resendMsg[timerId].protocal);
		}
		else
		{
			xTimerStop(resendMsg[timerId].timer, 0);
			xTimerDelete(resendMsg[timerId].timer, 0);
			resendMsg[timerId].valid		= 0;
		}
	}
}

//**********************************************************************
// ��������:  �ѽ���������ɵĻص�����
//     ������ɣ��ѽ��յ����ݴ��뻺�������У���֪ͨ������������¼�
// ���������protocalTemp/length: ����buffer��len
// ���ز����� 
//**********************************************************************
void ble_recv_data_cb(const uint8 *buffer, uint16 length)
{
    uint16 i;
    ble_msg_t msg;

    if(length > 32)
        length = 32;

    //���һ֡����ͷ�������Ƿ���ȷ��������������ݶ���
    if((buffer[0] != 0x23) || ((buffer[2] + 6) != length))
    {
        Ble_Debug((0,"Recv Data is Error:\n"));
        ble_printdata((uint8 *)buffer,length);

        //���ؽ��������쳣���ݰ���app
        
        return;
    }

    msg.id = BLE_MSG_RECEIVE;
    for(i = 0; i < length; i++)
    {
        msg.packet.data[i] = buffer[i];
    }
    Ble_SendMsgToQueue(&msg);
}

//**********************************************************************
// ��������:  ��������ʵ��
// ��ʼ������Ӳ����ע��������ɡ�����ʧ�ܡ�������ɣ�������ɣ���������ص�������
// Ȼ�󴴽���ʱ��ⶨʱ�����Լ���Ϣ���У��Լ�������Ϣ���С�����ʼ���ط���Ϣ���顣
// ��������Ϣ���е���Ϣ�������Ӧ�¼���
// ��������� pvParameters���������
// ���ز�����    
//**********************************************************************
void Mid_BleTask(void *pvParameters)
{
#if(BLE_CHECKSUM_ENABLE ==1)
    uint8  *pbuf;
#endif
    uint8  u8BleTxStatus = 1,u8BleConnStatus = BT_ADV_ON;    
    uint32 u32Qsend_num,u32Qmain_num;  //��¼������������Ϣ����
    uint32 u32DisConnTime =0;
    ble_msg_t msg;

	//set software timer for ble communicate timeout
	ble_task_msg_queue		= xQueueCreate(BLE_TASK_QUEUE_SIZE, sizeof(ble_msg_t));
	ble_task_msg_queue_send	= xQueueCreate(BLE_TASK_QUEUE_SIZE, sizeof(ble_msg_t));

    if(ble_task_msg_queue_send == NULL)
    {
        Err_Info((0,"Error:Create Send Queue fail\n"));
        while(1);
    }
    else if(ble_task_msg_queue == NULL)
    {
        Err_Info((0,"Error:Create main Queue fail\n"));        
        while(1);
    }

    pipe = 1;
    Ble_ResendMsgInit();   // resend process init
    for(;;)
    {   
        u8BleTxStatus = BLE_Stack_CheckSendStatus();
        u32Qsend_num = uxQueueMessagesWaiting(ble_task_msg_queue_send);
        u32Qmain_num = uxQueueMessagesWaiting(ble_task_msg_queue);
            
        //�������������ble_task_msg_queue_send�л�ȡ��Ϣ:
        // 1.����Ϊ�ɷ���״̬����ble_task_msg_queue_send��Ϊ��
        // 2.ble_task_msg_queue_send��Ϊ�գ���ble_task_msg_queue��
        if((u32Qsend_num > 0) && ((u8BleTxStatus == 1) || (u32Qmain_num == 0)))
        {
            //Ble_Debug((0,"recv from ble_task_msg_queue_send\n"));
            if(xQueueReceive(ble_task_msg_queue_send, &msg, 100) != pdPASS)
            {
                Err_Info((0,"Error:recv from ble_task_msg_queue_send fail\n"));
                vTaskDelay(2);
                continue;
            }
        }
        else   //�����������ݣ�����ȡ��Ϣ
        {
            //Ble_Debug((0,"recv from ble_task_msg_queue\n"));
            if(xQueueReceive(ble_task_msg_queue, &msg, portMAX_DELAY) != pdPASS)
            {
                Ble_Debug((0,"recv from ble_task_msg_queue fail\n"));
                vTaskDelay(2);
                continue;
            }
        }

        //deal with recv queue msg
        switch(msg.id)
        {
        case BLE_MSG_SEND_WITH_RESEND:
            Ble_SetResendMsg(&msg);
            //Ble_Debug((0,"with resend\n"));
        case BLE_MSG_SEND:                   //����
            if(BLE_Stack_CheckSendStatus() == 1)  //������ble stack��������
            {
                BLE_Stack_Send(msg.packet.data, msg.packet.att.loadLength+6);
            }
            else
            {
                //fix 2: ������δ���ӵ�״̬�»������ݣ�����������task��һֱ�ܣ��������ȼ��͵�task
                // û�л������У����ֿ�ס�����
                //����1s��������δ����״̬�������ݶ���
                if(u32DisConnTime < 200)
                {
                    xQueueSendToBack(ble_task_msg_queue_send, &msg, 1);  //ble stack��������æʱ����msg�ŵ�send����
                }

#if(BLE_STACK_RUN_INMCU == 1)
                BLE_Stack_Interact(BLE_STATUS_GET,&u8BleConnStatus,0x00); //��ȡ��������״̬
#endif
                if(u8BleConnStatus == BT_CONNECTED)
                {
                    u32DisConnTime = 0;
                }
                else
                {
                    u32DisConnTime++;
                }
                //fix 2018.5.31

                //fix 3:��Щ�ֻ�����������Ȩ�ɹ������һֱ��
                //ԭ��:�����������ݺ�δ�غܿ�ͷ��ͳ�ȥ�������л��з��͵�����ʱ���᲻ͣ�ڴӶ���ȡ
                //��Ϣ��Ȼ��Żض��У�һֱռ��CPU��Դ����������task������
                //�ⷨ:��������������æ������£��ȵȴ�5 ticks��������������
                vTaskDelay(5);
                //fix 2018.5.31
            }
            break;
        case BLE_MSG_RECEIVE:
            //Ble_Debug((0,"Recv Data:\n"));
            //ble_printdata(&msg);
#if(BLE_CHECKSUM_ENABLE ==1)
			pbuf = msg.packet.data;
            if(Mid_Ble_CheckSum(&msg.packet) == pbuf[pbuf[2] + 5])
#endif
            {
                if(ProtocalAnalysis != NULL)
                {
                    (ProtocalAnalysis)(&msg);
                }
                else
                {
                    Ble_Debug((0,"ProtocalAnalysis is null\n"));
                }
            }
#if(BLE_CHECKSUM_ENABLE ==1)
            else
            {
                //����У�鲻��ȷ
                Ble_Debug((0,"Chech Sum Error\n"));
            }
#endif
            break;
        case BLE_MSG_RESEND_TIMEOUT:
            Ble_ResendToutProcess(msg.packet.resendToutId);
            break;
        }
    }
}

//**********************************************************************
// ��������:  ��������У��ֵ
// ��������� protocal
// ���ز����� У��ֵ 
//**********************************************************************
uint8 Mid_Ble_CheckSum(protocal_msg_t *protocal)
{
    uint8 i;
    uint8 checkCode = 0;
    uint8 length = protocal->att.loadLength + 5;

    for(i = 0; i < length; i++)
    {
        checkCode += protocal->data[i] ^ i;
    }
    
    //Ble_Debug((0,"Chech Sum= 0x%x \n",checkCode));
    return checkCode;
}

//**********************************************************************
// ��������: ���ط���Ϣ��������ɾ�������رճ�ʱ��ʱ��
// ���������    
//         msg����Ϣ��ַ
// ���ز����� 
//**********************************************************************
ret_type Mid_Ble_DelResendMsg(protocal_msg_t *msg)
{
	uint8 i;
	ret_type ret = Ret_Fail;

	for(i = 0; i < RESEND_MSG_MAX; i++)
	{
		if(((resendMsg[i].id & 0x7f) == (msg->att.flowControl & 0x7f)) && (resendMsg[i].valid))
		{
			xTimerStop(resendMsg[i].timer, 0);
			xTimerDelete(resendMsg[i].timer, 0);
			resendMsg[i].valid = 0;
            resendMsg[i].id =0;
			ret	= Ret_OK;
		}
	}
	return ret;
}

//**********************************************************************
// ��������: ����ͨ��Э�����ݣ��������ݣ������Լ�ACK
// ���������msg��ָ����Э������ָ��
// ���ز�����
//    Ret_QueueFull:��Ϣ������
//    Ret_OK:       ���ͳɹ�
//**********************************************************************
ret_type Mid_Ble_SendMsg(ble_msg_t *msg)
{
	ret_type ret = Ret_OK;
    uint8 u8BleConnStatus = BT_ADV_ON; 
    
	switch(msg->id)
    {
    case BLE_MSG_SEND_WITH_RESEND:
    {
		uint8  i;
		uint8 checkCode = 0;
		uint8	length;

		if(pipe > 0x7f)
			pipe = 1;
        
        msg->packet.att.flowControl = (msg->packet.att.flowControl | 0x80) + pipe++;
        length = msg->packet.att.loadLength + 5;
		for(i = 0; i < length; i++)
		{
			checkCode += msg->packet.data[i] ^ i;
		}
		msg->packet.data[length] = checkCode;
        Ble_Debug((0,"BLE_MSG_SEND Sum= 0x%x \n",checkCode));
    }
    case BLE_MSG_SEND:
    {
        uint32 u32Qsend_num,u32Qmain_num;  //��¼������������Ϣ����
        u32Qsend_num = uxQueueMessagesWaiting(ble_task_msg_queue_send);
        u32Qmain_num = uxQueueMessagesWaiting(ble_task_msg_queue);
        
        //�������ж��ǿգ���������Ϊ�ɷ���״̬����ֱ�ӷ���msg���ݣ�����Ͱ�msg�ŵ���������
        // ԭ��:ble_task_msg_queue_send�е�����ʱҪ���͵ģ�Ϊ�ǿգ�˵���������ڵȴ�����
        // ble_task_msg_queueΪ�����У�Ϊ�ǿգ���ʱ�޷�֪�������Ƿ��еȴ����͵�msg��
        // Ϊ��ȷ�����������Ⱥ��������������һΪ�ǿգ����Ȱ�msg�ȷŵ���������
        if((u32Qsend_num == 0) && (u32Qmain_num == 0) && (BLE_Stack_CheckSendStatus() == 1))
        {
            BLE_Stack_Send(msg->packet.data, msg->packet.att.loadLength+6);
        }
        else
        {
            //fix 2: ������δ���ӵ�״̬�»������ݣ�����������task��һֱ�ܣ��������ȼ��͵�task
            // û�л������У����ֿ�ס�����
            // �ⷨ:������δ���ӵ�����£��������з�����
#if(BLE_STACK_RUN_INMCU == 1)
            BLE_Stack_Interact(BLE_STATUS_GET,&u8BleConnStatus,0x00); //��ȡ��������״̬
#endif
            if(u8BleConnStatus == BT_CONNECTED)
            {
                Ble_SendMsgToQueue(msg);
    
            }
            //fix 2018.5.30
        }
        break;
    }
	default:
        break;
	}
	return ret;
}

//**********************************************************************
// ��������:  ����callback
// ��������� cb
// ���ز����� �� 
//**********************************************************************
void Mid_Ble_SetCallBack(protocal_cb cb)
{
    if(cb != NULL)
    {
        ProtocalAnalysis = cb;
    }
}

//**********************************************************************
// ��������:  ��ʼ������ģ��
// ��������� ��
// ���ز����� �� 
//**********************************************************************
void Mid_Ble_Init(void)
{
    //step 1: ��������MAC��ַ�͹㲥��
    //BLE_SetMacAddr();
    //BLE_SetAdvName();
    //step 2: init BLE stack,and set recieve data callback
    BLE_Stack_Init();
	BLE_Stack_SetRecvCallBack(ble_recv_data_cb);
    //step 3: create mid ble task
    // �����ջ�ռ�Ϊ��  1024*4    
    // ��������ȵȼ�Ϊ��Middle priority
    xTaskCreate(Mid_BleTask, "BleTask", 1024, NULL, TASK_PRIORITY_MIDDLE, NULL);
}

