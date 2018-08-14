#include "platform_common.h"
#include "ble_task.h"
#include "mid_ble_linkInv_manage.h"


/* FreeRTOS includes */
#include "rtos.h"



static const protocal_msg_t	PROT_BT_CONN_INTV_FAST_CONN =
{
	0x23, 0x01, 0x04, 0x12, 0xff, 0x04, 0x00, 0x05, BLE_LINKINV_SWITCH_FAST_CONNECT,
};

static const protocal_msg_t	PROT_BT_CONN_INTV_FAST_DISCONN =
{
	0x23, 0x01, 0x04, 0x12, 0xff, 0x04, 0x00, 0x05, BLE_LINKINV_SWITCH_FAST_DISCONNECT,
};

static const protocal_msg_t	PROT_BT_CONN_INTV_SLOW_CONN =
{
	0x23, 0x01, 0x04, 0x12, 0xff, 0x04, 0x00, 0x05, BLE_LINKINV_SWITCH_SLOW_CONNECT,
};

static const protocal_msg_t	PROT_BT_CONN_INTV_SLOW_DISCONN =
{
	0x23, 0x01, 0x04, 0x12, 0xff, 0x04, 0x00, 0x05, BLE_LINKINV_SWITCH_SLOW_DISCONNECT,
};




static TimerHandle_t bleLinkInvToutTimer	= NULL;				// 修改超时定时器
static TimerHandle_t bleLinkInvFastToutTimer	= NULL;				// 处于快速链接间隔状态超时未处理

static void (*BleInvSwitchResultCb)(uint16 result);

static	uint16	bleLinkInvState;
static	uint16	bleSwitchWay;

// 函数功能:	更改连接间隔超时回调
// 输入参数：	无
// 返回参数：	无
static void BleInvSwitchToutCb(TimerHandle_t xTimer)
{
	ble_msg_t bleMsg;
	switch(bleLinkInvState)
	{
		case BLE_LINKINV_SLOW_TO_FAST:
		BleInvSwitchResultCb(LINKINV_SWITCH_FAST_TOUT);
		break;

		case BLE_LINKINV_FAST_TO_SLOW:
		// 改慢超时不成功，继续改慢
		xTimerReset(bleLinkInvToutTimer, 1);	
		xTimerStart(bleLinkInvToutTimer, 1);
		bleMsg.id			= BLE_MSG_SEND;
		bleMsg.packet	= PROT_BT_CONN_INTV_SLOW_DISCONN;
		Mid_Ble_SendMsg(&bleMsg);

		BleInvSwitchResultCb(LINKINV_SWITCH_SLOW_TOUT);
		break;
	}
}


// 函数功能:	改快成功后，未进行其他改快后的相应时间处理且关闭定时器，会自动进行改慢
// 输入参数：	无
// 返回参数：	无
static void BleInvFastToutCb(TimerHandle_t xTimer)
{
	ble_msg_t bleMsg;

	if(bleLinkInvState == BLE_LINKINV_FAST)
	{
		bleMsg.id = BLE_MSG_SEND;
		if(bleSwitchWay == BLE_LINKINV_SWITCH_CONNECT)
		{
			// 发送连接改快指令
			bleMsg.packet = PROT_BT_CONN_INTV_SLOW_CONN;
		}
		else
		{
			// 发送断开改快指令
			bleMsg.packet = PROT_BT_CONN_INTV_SLOW_DISCONN;
		}

		bleLinkInvState	= BLE_LINKINV_FAST_TO_SLOW;
		xTimerReset(bleLinkInvToutTimer, 1);	
		xTimerStart(bleLinkInvToutTimer, 1);
        Mid_Ble_SendMsg(&bleMsg);
	}
}

// 函数功能:	初始化蓝牙连接间隔管理
// 输入参数：	ResultCb:修改超时1分钟回调函数，多次赋值只有最后一次有效，非用户级别中断
// 				result:		ble_linkinv_result_e
// 返回参数：	无
void Mid_BleLinkInv_Init(void (*ResultCb)(uint16 result))
{
	bleLinkInvState			= BLE_LINKINV_SLOW;
	bleLinkInvToutTimer 	= xTimerCreate("BleInvTout", APP_1SEC_TICK*60, pdFALSE, 0, BleInvSwitchToutCb);
	bleLinkInvFastToutTimer = xTimerCreate("BleFastTout", APP_1SEC_TICK*60, pdFALSE, 0, BleInvFastToutCb);
	BleInvSwitchResultCb	= ResultCb;
}

// 函数功能:	开启处于快速间隔的超时定时器，修改快速成功后调用
// 输入参数：	无
// 返回参数：	无
void Mid_BleLinkInv_FastState_Start(void)
{
	xTimerReset(bleLinkInvFastToutTimer, 1);
	xTimerStart(bleLinkInvFastToutTimer, 1);
}

// 函数功能:	当超时间隔超时的权限被其他功能拿到后可调用。
// 输入参数：	无
// 返回参数：	无
void Mid_BleLinkInv_FastState_Stop(void)
{
	xTimerStop(bleLinkInvFastToutTimer, 1);
}

// 函数功能:	改快连接间隔，超时或修改不成功不会进行其他操作,保持低速
// 输入参数：	switchWay:	BLE_LINKINV_SWITCH_CONNECT:不断开修改
// 							BLE_LINKINV_SWITCH_DISCONNECT:断开修改
// 返回参数：	无
void Mid_BleLinkInv_SwitchToFast(uint16 switchWay)
{
	ble_msg_t bleMsg;

	bleMsg.id			= BLE_MSG_SEND;
	// 无论状态为什么，直接发送改快
	switch(bleLinkInvState)
	{
		case BLE_LINKINV_SLOW:
		case BLE_LINKINV_SLOW_TO_FAST:
		case BLE_LINKINV_FAST_TO_SLOW:
		case BLE_LINKINV_FAST:
		xTimerReset(bleLinkInvToutTimer, 1);	
		xTimerStart(bleLinkInvToutTimer, 1);
		bleLinkInvState		= BLE_LINKINV_SLOW_TO_FAST;
		if(switchWay == BLE_LINKINV_SWITCH_CONNECT)
		{
			// 发送连接改快指令
			bleMsg.packet	= PROT_BT_CONN_INTV_FAST_CONN;
		}
		else
		{
			// 发送断开改快指令
			bleMsg.packet	= PROT_BT_CONN_INTV_FAST_DISCONN;
		}

        Mid_Ble_SendMsg(&bleMsg);
		bleSwitchWay			= switchWay;
		break;
	}
}


// 函数功能:	改慢连接间隔,修改方式按之前改快的方式进行修改
// 输入参数：	ResultCb:修改超时1分钟回调函数，多次赋值只有最后一次有效
// 返回参数：	无
void Mid_BleLinkInv_SwitchToSlow(void)
{
	ble_msg_t bleMsg;

	bleMsg.id			= BLE_MSG_SEND;
	// 无论状态为什么，直接发送改慢
	switch(bleLinkInvState)
	{
	case BLE_LINKINV_FAST:
	case BLE_LINKINV_SLOW_TO_FAST:
	case BLE_LINKINV_FAST_TO_SLOW:
	case BLE_LINKINV_SLOW:
		xTimerReset(bleLinkInvToutTimer, 1);	
		xTimerStart(bleLinkInvToutTimer, 1);
		bleLinkInvState		= BLE_LINKINV_FAST_TO_SLOW;
		if(bleSwitchWay == BLE_LINKINV_SWITCH_CONNECT)
		{
			// 发送连接改快指令
			bleMsg.packet	= PROT_BT_CONN_INTV_SLOW_CONN;
		}
		else
		{
			// 发送断开改快指令
			bleMsg.packet	= PROT_BT_CONN_INTV_SLOW_DISCONN;
		}

        Mid_Ble_SendMsg(&bleMsg);
		break;
	}
}

// 函数功能:	接收蓝牙连接间隔反馈结果，并返回操作结果,未到超时时间与没有
// 				明确的对应修改错误会保持等待状态。
// 输入参数：	protocal:蓝牙连接间隔反馈协议指针，解析蓝牙协议
// 返回参数：	ble_linkinv_result_e
uint16 Mid_BleLinkInv_Analysis(ble_msg_t *protocal)
{
	ble_msg_t bleMsg;

	if(protocal->packet.att.routeMsg != BLE_TO_MCU)
		return 0xff;

	if((protocal->packet.att.load.content.interfaceIndex2 != PROT_BT_CONN_INTV) ||
		(protocal->packet.att.load.content.interfaceIndex1 !=  PROT_LINK))
		return 0xff;

	switch(protocal->packet.att.load.content.parameter[0])
    {
    case BLE_LINKINV_SWITCH_FAST_CONNECT:
    case BLE_LINKINV_SWITCH_FAST_DISCONNECT:
		
		if(protocal->packet.att.load.content.parameter[1] == 0x01)
		{
			switch(bleLinkInvState)
			{
            case BLE_LINKINV_SLOW_TO_FAST:
				xTimerStop(bleLinkInvToutTimer, 1);
				bleLinkInvState	= BLE_LINKINV_FAST;
				return LINKINV_SWITCH_FAST_SUCESS;
            case BLE_LINKINV_FAST:
				bleLinkInvState		= BLE_LINKINV_FAST;
				return LINKINV_SWITCH_FAST_SUCESS;
            case BLE_LINKINV_FAST_TO_SLOW:
				// 正在改慢过程中,无需变更
				return LINKINV_IN_SWITCH_SLOW;				
            case BLE_LINKINV_SLOW:
				// 状态错误，自动进行改慢操作
				bleLinkInvState		= BLE_LINKINV_FAST_TO_SLOW;
				xTimerReset(bleLinkInvToutTimer, 1);	
				xTimerStart(bleLinkInvToutTimer, 1);
				bleMsg.id = BLE_MSG_SEND;
				bleMsg.packet = PROT_BT_CONN_INTV_SLOW_DISCONN;
                Mid_Ble_SendMsg(&bleMsg);
				return LINKINV_STATUS_ERROR_FAST;
			}
		}
		else
		{
			switch(bleLinkInvState)
			{
			case BLE_LINKINV_FAST:
			case BLE_LINKINV_SLOW:
				// 进行改慢操作，并返回状态错误
				bleLinkInvState		= BLE_LINKINV_FAST_TO_SLOW;
				xTimerReset(bleLinkInvToutTimer, 1);	
				xTimerStart(bleLinkInvToutTimer, 1);
				bleMsg.id = BLE_MSG_SEND;
				bleMsg.packet = PROT_BT_CONN_INTV_SLOW_DISCONN;
                Mid_Ble_SendMsg(&bleMsg);
				return LINKINV_STATUS_ERROR_SLOW;
			case BLE_LINKINV_SLOW_TO_FAST:
				xTimerStop(bleLinkInvToutTimer, 1);
				bleLinkInvState		= BLE_LINKINV_SLOW;
				return LINKINV_SWITCH_FAST_FAIL;
			case BLE_LINKINV_FAST_TO_SLOW:
				return LINKINV_IN_SWITCH_SLOW;				
			}
		}

		break;
    case BLE_LINKINV_SWITCH_SLOW_CONNECT:
    case BLE_LINKINV_SWITCH_SLOW_DISCONNECT:
		if(protocal->packet.att.load.content.parameter[1] == 0x01)
		{
			switch(bleLinkInvState)
			{
            case BLE_LINKINV_SLOW_TO_FAST:
				// 无需操作，返回正在改快过程中
				return LINKINV_IN_SWITCH_FAST;

            case BLE_LINKINV_FAST:
				// 返回状态错误，应为快，不进行自动改慢处理
				bleLinkInvState		= BLE_LINKINV_FAST;
				return LINKINV_STATUS_ERROR_SLOW;
				
            case BLE_LINKINV_FAST_TO_SLOW:
				// 改慢成功，关闭定时器，状态变更
				xTimerStop(bleLinkInvToutTimer, 1);
				bleLinkInvState		= BLE_LINKINV_SLOW;
				return LINKINV_SWITCH_SLOW_SUCESS;
                
            case BLE_LINKINV_SLOW:
				// 状态相同，无需改变
				return LINKINV_SWITCH_SLOW_SUCESS;
			}
		}
		else
		{
			switch(bleLinkInvState)
			{
			case BLE_LINKINV_FAST:
				// 反馈错误，修改为改慢，并返回状态错误
				xTimerReset(bleLinkInvToutTimer, 1);	
				xTimerStart(bleLinkInvToutTimer, 1);
				bleMsg.id = BLE_MSG_SEND;
				bleMsg.packet = PROT_BT_CONN_INTV_SLOW_DISCONN;
                Mid_Ble_SendMsg(&bleMsg);
				return LINKINV_STATUS_ERROR_SLOW;
			case BLE_LINKINV_SLOW_TO_FAST:
				return LINKINV_IN_SWITCH_FAST;
			case BLE_LINKINV_FAST_TO_SLOW:
			case BLE_LINKINV_SLOW:
				// 改慢失败，继续改慢
				xTimerReset(bleLinkInvToutTimer, 1);
				xTimerStart(bleLinkInvToutTimer, 1);
				bleMsg.id = BLE_MSG_SEND;
				bleMsg.packet = PROT_BT_CONN_INTV_SLOW_DISCONN;
                Mid_Ble_SendMsg(&bleMsg);
				return LINKINV_SWITCH_SLOW_FAIL;
			}
		}
		break;
	}
	//couldn't run here
	return 0xff;
}

uint16 Mid_BleLinkInv_StateRead(void)
{
	return bleLinkInvState;
}

