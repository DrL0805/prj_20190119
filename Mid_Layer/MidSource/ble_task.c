/**********************************************************************
**
**模块说明: 对接BLE mid接口
**软件版本，修改日志(时间，内容):
**   V1.0   2018.5.2  初版  
**   fix 1  2018.5.29 从flash获取蓝牙广播名一直为0xff
**   fix 2  2018.5.30 在蓝牙未连接的状态下发数据，会导致系统卡死
**   fix 3  2018.5.31 在蓝牙发送忙的时候，继续发送数据，导致系统卡死
**   fix 4:  2018.6.9  OTA时IOS手机APP异常退出，出现屏幕卡死的情况
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

//是否需要开启接收数据checksum校验功能，正式版本软件需要enable
//调试阶段可以disable
#define BLE_CHECKSUM_ENABLE        0

//重发消息的最大数目
#define		RESEND_MSG_MAX			8
#define		RESEND_CNT_MAX			4

//重发消息的消息帧
typedef struct 
{
    uint8         id;       //ID
    uint8         valid;    //重发标志
    uint8         resendCnt;//重发次数
    TimerHandle_t timer;    //定时器句柄
    ble_msg_t   protocal; //重发协议
    func *      TimerOutCb;   //超时回调函数
}resend_msg_t;

//蓝牙任务的消息队列
static QueueHandle_t		ble_task_msg_queue;				/**< Queue handler for ble task */
//蓝牙任务的消息队列满时，创建的备用消息队列，保存要处理的消息
static QueueHandle_t		ble_task_msg_queue_send;			/**< Queue handler for ble task */


//接受数据的缓存数组
//static uint8 rxBuf[BLE_BUF_LENGTH_MAX];

//函数指针数组
static protocal_cb ProtocalAnalysis = NULL;

static resend_msg_t	resendMsg[RESEND_MSG_MAX];	//ble resend buf flag

//流控信息
//一般情况下，发送端每发送一条数据帧，接收端都应该立即返回一条应答帧（例外的情况是，echo  类型的数据帧）；在头
//部中加入流控信息，可达到以下目的：
//?  1、发送端想连续多次触发接收端执行某一动作时，流控信息用于甄别该数据帧是多次动作而不是一次动作的超时重发
//?  2、当发送端想连续发送一些必须连续、顺序执行的配置时，使用流控信息可捕捉丢包情况
//流控的最高位为保留位，流控的低 7 位表示流水号
//发送端每发送一帧数据，流水号自动增一；接收端做出应答时，将流水号原样返回给发送端
//流水号仅用于甄别帧的发送时间点，不做帧连续性、有序性的校验
static uint8 pipe;

//**********************************************************************
// 函数功能: 打印收发数据
// 输入参数：
// 返回参数：
// 打印协议: ble_printdata(msg->packet.data,msg->packet.att.loadLength + 6)
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
// 函数功能: 设置蓝牙MAC地址
// 输入参数：
// 返回参数：
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
// 函数功能: 设置蓝牙广播名
// 输入参数：
// 返回参数：
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
// 函数功能: 将msg发送到消息队列ble_task_msg_queue中
// 输入参数：msg ；蓝牙任务消息帧
// 返回参数：
//**********************************************************************
static ret_type Ble_SendMsgToQueue(ble_msg_t *msg)
{
    ret_type ret = Ret_OK;
    
    if(xPortIsInsideInterrupt() == pdTRUE)    //在中断中设置event
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
// 函数功能:  重发消息初始化
//     通信超时，并通知通信超时事件
// 输入参数：    
// 返回参数： 
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
// 函数功能:  重发消息超时
//     重发消息超时，并通知重发消息超时事件
// 输入参数：    
//         xTimer：定时器句柄
// 返回参数： 
//**********************************************************************
static void Ble_ResendTimeoutIsr(TimerHandle_t xTimer)
{
    ble_msg_t msg;

    msg.id = BLE_MSG_RESEND_TIMEOUT;
    msg.packet.resendToutId = (int32)pvTimerGetTimerID( xTimer );
    Ble_SendMsgToQueue(&msg);
}

//**********************************************************************
// 函数功能: 把重发消息压入数组，并启动超时定时器
// 输入参数：    
//         msg：消息地址
// 返回参数： 
//  FAILURE:操作失败
//  SUCCESS:操作成功 
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
// 函数功能: 重发消息
//如果重发次数小于最大次数，通知事件重发，否则，关闭超时定时器，并删除重发消息
// 输入参数：    
//         timerId：消息重发次数
// 返回参数： 
//          无
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
// 函数功能:  把接收数据完成的回调函数
//     接受完成，把接收的数据存入缓存数组中，并通知接收数据完成事件
// 输入参数：protocalTemp/length: 数据buffer和len
// 返回参数： 
//**********************************************************************
void ble_recv_data_cb(const uint8 *buffer, uint16 length)
{
    uint16 i;
    ble_msg_t msg;

    if(length > 32)
        length = 32;

    //检查一帧数据头，长度是否正确，有问题则把数据丢掉
    if((buffer[0] != 0x23) || ((buffer[2] + 6) != length))
    {
        Ble_Debug((0,"Recv Data is Error:\n"));
        ble_printdata((uint8 *)buffer,length);

        //返回接收数据异常数据包给app
        
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
// 函数功能:  蓝牙任务实现
// 初始化蓝牙硬件，注册下载完成、下载失败、发送完成，接受完成，接受请求回调函数。
// 然后创建超时检测定时器，以及消息队列，以及备用消息队列。并初始化重发消息数组。
// 最后根据消息队列的消息，处理对应事件。
// 输入参数： pvParameters：任务参数
// 返回参数：    
//**********************************************************************
void Mid_BleTask(void *pvParameters)
{
#if(BLE_CHECKSUM_ENABLE ==1)
    uint8  *pbuf;
#endif
    uint8  u8BleTxStatus = 1,u8BleConnStatus = BT_ADV_ON;    
    uint32 u32Qsend_num,u32Qmain_num;  //记录两个队列中消息数量
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
            
        //以下两种情况从ble_task_msg_queue_send中获取消息:
        // 1.蓝牙为可发送状态，且ble_task_msg_queue_send不为空
        // 2.ble_task_msg_queue_send不为空，且ble_task_msg_queue空
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
        else   //主队列有数据，从中取消息
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
        case BLE_MSG_SEND:                   //发送
            if(BLE_Stack_CheckSendStatus() == 1)  //可以向ble stack发送数据
            {
                BLE_Stack_Send(msg.packet.data, msg.packet.att.loadLength+6);
            }
            else
            {
                //fix 2: 在蓝牙未连接的状态下还发数据，导致数据在task中一直跑，其他优先级低的task
                // 没有机会运行，出现卡住的情况
                //超过1s蓝牙处于未连接状态，则将数据丢掉
                if(u32DisConnTime < 200)
                {
                    xQueueSendToBack(ble_task_msg_queue_send, &msg, 1);  //ble stack发送数据忙时，把msg放到send队列
                }

#if(BLE_STACK_RUN_INMCU == 1)
                BLE_Stack_Interact(BLE_STATUS_GET,&u8BleConnStatus,0x00); //获取蓝牙连接状态
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

                //fix 3:有些手机请求连接授权成功后，马达一直震动
                //原因:蓝牙发送数据后，未必很快就发送出去，队列中还有发送的数据时，会不停在从队列取
                //消息，然后放回队列，一直占用CPU资源，导致蓝牙task不动。
                //解法:在蓝牙发送数据忙的情况下，先等待5 ticks给蓝牙发送数据
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
                //数据校验不正确
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
// 函数功能:  计算数据校验值
// 输入参数： protocal
// 返回参数： 校验值 
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
// 函数功能: 把重发消息从数组中删除，并关闭超时定时器
// 输入参数：    
//         msg：消息地址
// 返回参数： 
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
// 函数功能: 发送通信协议数据，包含数据，命令以及ACK
// 输入参数：msg；指向发送协议数据指针
// 返回参数：
//    Ret_QueueFull:消息队列满
//    Ret_OK:       发送成功
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
        uint32 u32Qsend_num,u32Qmain_num;  //记录两个队列中消息数量
        u32Qsend_num = uxQueueMessagesWaiting(ble_task_msg_queue_send);
        u32Qmain_num = uxQueueMessagesWaiting(ble_task_msg_queue);
        
        //两个队列都是空，并且蓝牙为可发送状态，就直接发送msg数据，否则就把msg放到主队列中
        // 原因:ble_task_msg_queue_send中的数据时要发送的，为非空，说明有数据在等待发送
        // ble_task_msg_queue为主队列，为非空，此时无法知道其中是否有等待发送的msg。
        // 为了确保发送数据先后次序，两个队列其一为非空，都先把msg先放到主队列中
        if((u32Qsend_num == 0) && (u32Qmain_num == 0) && (BLE_Stack_CheckSendStatus() == 1))
        {
            BLE_Stack_Send(msg->packet.data, msg->packet.att.loadLength+6);
        }
        else
        {
            //fix 2: 在蓝牙未连接的状态下还发数据，导致数据在task中一直跑，其他优先级低的task
            // 没有机会运行，出现卡住的情况
            // 解法:在蓝牙未连接的情况下，不往队列放数据
#if(BLE_STACK_RUN_INMCU == 1)
            BLE_Stack_Interact(BLE_STATUS_GET,&u8BleConnStatus,0x00); //获取蓝牙连接状态
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
// 函数功能:  设置callback
// 输入参数： cb
// 返回参数： 无 
//**********************************************************************
void Mid_Ble_SetCallBack(protocal_cb cb)
{
    if(cb != NULL)
    {
        ProtocalAnalysis = cb;
    }
}

//**********************************************************************
// 函数功能:  初始化蓝牙模块
// 输入参数： 无
// 返回参数： 无 
//**********************************************************************
void Mid_Ble_Init(void)
{
    //step 1: 设置蓝牙MAC地址和广播名
    //BLE_SetMacAddr();
    //BLE_SetAdvName();
    //step 2: init BLE stack,and set recieve data callback
    BLE_Stack_Init();
	BLE_Stack_SetRecvCallBack(ble_recv_data_cb);
    //step 3: create mid ble task
    // 分配的栈空间为：  1024*4    
    // 任务的优先等级为：Middle priority
    xTaskCreate(Mid_BleTask, "BleTask", 1024, NULL, TASK_PRIORITY_MIDDLE, NULL);
}

