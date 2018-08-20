//*****************************************************************************
//
//! @file   ancs_aph.h
//!
//! @brief  Ambiq Micro's demonstration of ANCSC
//
//*****************************************************************************

//*****************************************************************************
//
// Copyright (c) 2017, Ambiq Micro
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is part of revision 1.2.11 of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef ANCS_API_H
#define ANCS_API_H

#include "wsf_os.h"
#include "stdint.h"
#include "stdbool.h"
#include "lowsapp_main.h"
#include "BLE_Stack.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
  Macros
**************************************************************************************************/

#ifndef ANCS_CONN_MAX
#define ANCS_CONN_MAX                  1
#endif
    
/*! WSF message event starting value */
#define ANCS_MSG_START               0xA0

/*! WSF message event enumeration */
enum
{
    ANCC_ACTION_TIMER_IND = ANCS_MSG_START,     /*! ANCC action timer expired */
    AMOTA_RESET_TIMER_IND,                            /*! AMOTA reset timer expired */ 
    AMOTA_DISCONNECT_TIMER_IND,                 /*! AMOTA disconnect timer expired */
    APP_SEND_ANCS_MSG_TIMEOUT_IND,
};

typedef enum
{   
    START_GET_ATTR,
    START_GET_APP_ID,
    GET_TITLE,
    GET_SUBTITLE,
    GET_MSG,
    POSITIVE_ACTION,
    NEGATIVE_ACTION,
    MSG_OP_END
}GET_MSG_OP_STATE;
  
typedef void (*m_ble_ancs_app_id_process_handler_t)(const uint8_t *buf,uint8_t len,uint8_t type);
typedef void (*m_ble_ancs_app_attr_process_handler_t)(const uint8_t *buf,uint8_t len,GET_MSG_OP_STATE get_msg_op_state);


#define BIT_0 0x01 /**< The value of bit 0 */
#define BIT_1 0x02 /**< The value of bit 1 */
#define BIT_2 0x04 /**< The value of bit 2 */
#define BIT_3 0x08 /**< The value of bit 3 */
#define BIT_4 0x10 /**< The value of bit 4 */
#define BIT_5 0x20 /**< The value of bit 5 */
#define BIT_6 0x40 /**< The value of bit 6 */
#define BIT_7 0x80 /**< The value of bit 7 */
#define BIT_8 0x0100 /**< The value of bit 8 */
#define BIT_9 0x0200 /**< The value of bit 9 */
#define BIT_10 0x0400 /**< The value of bit 10 */
#define BIT_11 0x0800 /**< The value of bit 11 */
#define BIT_12 0x1000 /**< The value of bit 12 */
#define BIT_13 0x2000 /**< The value of bit 13 */
#define BIT_14 0x4000 /**< The value of bit 14 */
#define BIT_15 0x8000 /**< The value of bit 15 */
#define BIT_16 0x00010000 /**< The value of bit 16 */
#define BIT_17 0x00020000 /**< The value of bit 17 */
#define BIT_18 0x00040000 /**< The value of bit 18 */
#define BIT_19 0x00080000 /**< The value of bit 19 */
#define BIT_20 0x00100000 /**< The value of bit 20 */
#define BIT_21 0x00200000 /**< The value of bit 21 */
#define BIT_22 0x00400000 /**< The value of bit 22 */
#define BIT_23 0x00800000 /**< The value of bit 23 */
#define BIT_24 0x01000000 /**< The value of bit 24 */
#define BIT_25 0x02000000 /**< The value of bit 25 */
#define BIT_26 0x04000000 /**< The value of bit 26 */
#define BIT_27 0x08000000 /**< The value of bit 27 */
#define BIT_28 0x10000000 /**< The value of bit 28 */
#define BIT_29 0x20000000 /**< The value of bit 29 */
#define BIT_30 0x40000000 /**< The value of bit 30 */
#define BIT_31 0x80000000 /**< The value of bit 31 */


#define MSG_BIT_QQ              BIT_0
#define MSG_BIT_WECHAT          BIT_1
#define MSG_BIT_TC_WEIBO        BIT_2
#define MSG_BIT_SKYPE           BIT_3
#define MSG_BIT_XL_WEIBO        BIT_4
#define MSG_BIT_FACEBOOK        BIT_5
#define MSG_BIT_TWITTER         BIT_6
#define MSG_BIT_WHATSAPP        BIT_7
#define MSG_BIT_LINE            BIT_8
#define MSG_BIT_OTHER1          BIT_9
#define MSG_BIT_CALLIN          BIT_10
#define MSG_BIT_MSG             BIT_11
#define MSG_BIT_MISS_CALL       BIT_12
#define MSG_BIT_CALENDAR        BIT_13
#define MSG_BIT_EMAIL           BIT_14
#define MSG_BIT_OTHER2          BIT_15
#define MSG_BIT_CONTEXT_CALLIN  BIT_16
#define MSG_BIT_TIM             BIT_17

#define CHECK_VALID_BIT(bitmap, bit)            ( (((bitmap) & (bit)) == (bit)) ? true : false )
#define SET_VALID_BIT(bitmap, bit, bit_mask)    (bitmap) |= ((bit) & (bit_mask))

//**********************************************************************
// 函数功能: 来电拒接，拒接当前来电
// 输入参数：
// 返回参数：
//**********************************************************************
extern void Ble_Ancs_RejectCall(void);

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/
/*************************************************************************************************/
/*!
 *  \fn     AncsStart
 *        
 *  \brief  Start the application.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AncsStart(void);

/*************************************************************************************************/
/*!
 *  \fn     AncsHandlerInit
 *        
 *  \brief  Application handler init function called during system initialization.
 *
 *  \param  handlerID  WSF handler ID for App.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AncsHandlerInit(wsfHandlerId_t handlerId);

/*************************************************************************************************/
/*!
 *  \fn     AncsHandler
 *        
 *  \brief  WSF event handler for the application.
 *
 *  \param  event   WSF event mask.
 *  \param  pMsg    WSF message.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AncsHandler(wsfEventMask_t event, wsfMsgHdr_t *pMsg);
//static void m_ble_lowsapp_init(void);
//static void amdtps_conn_close(dmEvt_t *pMsg);
void m_ble_ancs_process_handler_init(m_ble_ancs_app_id_process_handler_t ancs_app_id_process_handler,
                                     m_ble_ancs_app_attr_process_handler_t ancs_app_attr_process_handler);

void m_ble_profile_transfer_data_init(m_ble_profile_recv_data_handler_t recv_data_handler);

void m_ble_profile_send_data(uint8_t *buf, uint8_t len,uint8_t type);
	
bool_t m_ble_profile_send_idle_check(void);

//fix :ANCS复位发现，解决手机只开蓝牙，不开应用APP，也可以支持ANCS功能
void m_ble_ancs_callback(ble_echo pfun);
//fix

#ifdef __cplusplus
};
#endif

#endif /* ANCS_API_H */
