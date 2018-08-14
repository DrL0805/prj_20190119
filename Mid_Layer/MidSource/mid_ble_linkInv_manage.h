#ifndef			MID_BLE_LINKINV_MANAGE_H
#define			MID_BLE_LINKINV_MANAGE_H

#include "mid_common.h"

typedef enum
{
	BLE_LINKINV_SLOW	= 0,		// 连接间隔为慢
	BLE_LINKINV_SLOW_TO_FAST,		// 连接间隔处于由慢改快状态
	BLE_LINKINV_FAST,				// 连接间隔处于快速状态
	BLE_LINKINV_FAST_TO_SLOW,		// 连接间隔处于由快改慢状态
}ble_linkinv_status;

typedef enum
{
	BLE_LINKINV_SWITCH_CONNECT	= 0,	// 不断开改变连接间隔
	BLE_LINKINV_SWITCH_DISCONNECT,		// 断开改变连接间隔
}ble_linkinv_switch_way;

typedef enum
{
	LINKINV_IN_SWITCH_FAST			= 0,		//	正处于改快过程中
	LINKINV_IN_SWITCH_SLOW,						//	正处于改慢过程中
	LINKINV_SWITCH_FAST_SUCESS,					//	改快成功
	LINKINV_SWITCH_SLOW_SUCESS,					//	改慢成功，
	LINKINV_SWITCH_FAST_FAIL,					//	改快失败，状态结束，如需继续改快，调用改快指令
	LINKINV_SWITCH_SLOW_FAIL,					//	改慢失败，改慢失败会自动继续发送改慢指令，除非调用改快指令才会停止改慢
	LINKINV_SWITCH_FAST_TOUT,					//	改快超时失败，状态结束，如需继续改快，调用改快指令
	LINKINV_SWITCH_SLOW_TOUT,					//	改慢超时失败，改慢失败会自动继续发送改慢指令，除非调用改快指令才会停止改慢
	LINKINV_STATUS_ERROR_FAST,					//	状态错误，应为慢状态，自动进行改慢操作
	LINKINV_STATUS_ERROR_SLOW,					//	状态错误，应为快状态，需进行改快指令调用
}ble_linkinv_result_e;


typedef enum
{
	BLE_LINKINV_SWITCH_FAST_CONNECT		= 0x00,
	BLE_LINKINV_SWITCH_SLOW_CONNECT		= 0x01,
	BLE_LINKINV_SWITCH_FAST_DISCONNECT	= 0x02,
	BLE_LINKINV_SWITCH_SLOW_DISCONNECT	= 0x03,
}ble_protocal_linkinv_switch_way;



void   Mid_BleLinkInv_Init(void (*ResultCb)(uint16 result));
void   Mid_BleLinkInv_SwitchToFast(uint16 switchWay);
void   Mid_BleLinkInv_SwitchToSlow(void);
uint16 Mid_BleLinkInv_Analysis(ble_msg_t *protocal);
uint16 Mid_BleLinkInv_StateRead(void);
void   Mid_BleLinkInv_FastState_Start(void);
void   Mid_BleLinkInv_FastState_Stop(void);



#endif			// BLE_LINKINV_MANAGE_APP_H
