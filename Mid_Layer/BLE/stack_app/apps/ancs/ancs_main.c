//*****************************************************************************
//
//! @file   m_ble_profile_main.c
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

/**********************************************************************
**
**模块说明: ancs
**软件版本，修改日志(时间，内容),修改人:
**  fix 1:  2018.6.8  ANCS服务发现，解决手机只开蓝牙，不开应用APP，也可以支持ANCS功能
**  fix 2:  2018.6.12 system will be in OTA mode, other module will stop working.    
**  fix 3:  
**
**********************************************************************/

#include "platform_common.h"
#include "platform_debugcof.h"
#include "wsf_types.h"
#include "bstream.h"
#include "wsf_msg.h"
#include "wsf_trace.h"
#include "wsf_assert.h"
#include "hci_api.h"
#include "dm_api.h"
#include "smp_api.h"
#include "att_api.h"
#include "app_cfg.h"
#include "app_api.h"
#include "app_db.h"
#include "app_ui.h"
#include "app_hw.h"
#include "svc_ch.h"
#include "svc_core.h"
#include "svc_dis.h"

#include "gatt_api.h"

// m_ble_profile
#include "app_m_ble_profile.h"
#include "ancs_api.h"
#include "ancc_api.h"
#include "svc_lowsapp.h"
#include "amotas_api.h"
#include "svc_amotas.h"

/**************************************************************************************************
  Macros
**************************************************************************************************/    
#if(BLE_STACK_DEBUG_ENABLE ==1)    
#define Ble_Debug(x) SEGGER_RTT_printf x
#else
#define Ble_Debug(x)
#endif

/**************************************************************************************************
  Data Types
**************************************************************************************************/

/*! Application message type */
typedef union
{
    wsfMsgHdr_t     hdr;
    dmEvt_t         dm;
    attsCccEvt_t    ccc;
    attEvt_t        att;
} m_ble_profile_msg_t;

typedef struct  
{
  bool_t            lowsapp_tx_ready;
  bool_t            ch1_enable;
  bool_t            ch2_enable;
  bool_t            ch3_enable;
}m_ble_lowsapp_cb_t;

/**************************************************************************************************
  Global Variables
**************************************************************************************************/
/*! application control block */
static struct
{
  /* m_ble_profile tracking variables */
  uint16_t          hdlList[APP_DB_HDL_LIST_LEN];
  wsfHandlerId_t    handlerId;
  uint8_t           discState;
  uint16_t          connInterval;     /* connection interval */
  m_ble_lowsapp_cb_t  m_ble_lowsapp_cb;

} m_ble_profile_cb;

static  m_ble_ancs_app_id_process_handler_t  m_ble_ancs_app_id_process_handler = NULL;
static  m_ble_ancs_app_attr_process_handler_t m_ble_ancs_app_attr_process_handler = NULL;

/*! ANCC Configurable parameters */
static const anccCfg_t m_ble_profileAnccCfg =
{
   200         /*! action timer expiration period in ms */
};

/*! AMOTAS configuration */
static const AmotasCfg_t amotasCfg =
{
    0
};

/**************************************************************************************************
  ATT Client Discovery Data
**************************************************************************************************/

/*! Discovery states:  enumeration of services to be discovered */
enum
{
  ANCS_DISC_GATT_SVC,      /* GATT service */
  ANCS_DISC_ANCS_SVC,      /* discover ANCS service */
  ANCS_DISC_SVC_MAX        /* Discovery complete */
};


/*! Start of each service's handles in the the handle list */
#define ANCS_DISC_GATT_START        0
#define ANCS_DISC_ANCS_START        (ANCS_DISC_GATT_START + GATT_HDL_LIST_LEN)
#define ANCS_DISC_HDL_LIST_LEN      (ANCS_DISC_ANCS_START + ANCC_HDL_LIST_LEN)

/*! Pointers into handle list for each service's handles */
static uint16_t *pAncsGattHdlList = &m_ble_profile_cb.hdlList[ANCS_DISC_GATT_START];
static uint16_t *pAncsAnccHdlList = &m_ble_profile_cb.hdlList[ANCS_DISC_ANCS_START];

/* sanity check:  make sure handle list length is <= app db handle list length */
WSF_CT_ASSERT(ANCS_DISC_HDL_LIST_LEN <= APP_DB_HDL_LIST_LEN);

/**************************************************************************************************
  ATT Client Configuration Data
**************************************************************************************************/

/*
 * Data for configuration after service discovery
 */

/* Default value for CCC notifications */
static const uint8_t m_ble_profileCccNtfVal[] = {UINT16_TO_BYTES(ATT_CLIENT_CFG_NOTIFY)}; 

/* List of characteristics to configure after service discovery */
static const attcDiscCfg_t m_ble_profileDiscCfgList[] =
{
  /* Write:  GATT service changed ccc descriptor */
  {m_ble_profileCccNtfVal, sizeof(m_ble_profileCccNtfVal), (GATT_SC_CCC_HDL_IDX + ANCS_DISC_GATT_START)},

  /* Write:  ANCS setting ccc descriptor */
  {m_ble_profileCccNtfVal, sizeof(m_ble_profileCccNtfVal), (ANCC_NOTIFICATION_SOURCE_CCC_HDL_IDX + ANCS_DISC_ANCS_START)},

  /* Write:  ANCS setting ccc descriptor */
  {m_ble_profileCccNtfVal, sizeof(m_ble_profileCccNtfVal), (ANCC_DATA_SOURCE_CCC_HDL_IDX + ANCS_DISC_ANCS_START)},
};

/* Characteristic configuration list length */
#define ANCS_DISC_CFG_LIST_LEN   (sizeof(m_ble_profileDiscCfgList) / sizeof(attcDiscCfg_t))

/* sanity check:  make sure configuration list length is <= handle list length */
WSF_CT_ASSERT(ANCS_DISC_CFG_LIST_LEN <= ANCS_DISC_HDL_LIST_LEN);

/**************************************************************************************************
  Client Characteristic Configuration Descriptors
**************************************************************************************************/

/*! enumeration of client characteristic configuration descriptors */
enum
{
    ANCS_GATT_SC_CCC_IDX,               /*! GATT service, service changed characteristic */
    AMOTA_AMOTAS_TX_CCC_IDX,
    LOWSAPP_CH1_CCC_IDX,
    LOWSAPP_CH2_CCC_IDX,
    LOWSAPP_CH3_CCC_IDX,
    ANCS_NUM_CCC_IDX
};

/*! client characteristic configuration descriptors settings, indexed by above enumeration */
static const attsCccSet_t m_ble_profileCccSet[ANCS_NUM_CCC_IDX] =
{
    /* cccd handle          value range               security level */
    {GATT_SC_CH_CCC_HDL,    ATT_CLIENT_CFG_INDICATE,  DM_SEC_LEVEL_NONE},    /* ANCS_GATT_SC_CCC_IDX */
    {AMOTAS_TX_CH_CCC_HDL,  ATT_CLIENT_CFG_NOTIFY,    DM_SEC_LEVEL_NONE},    /* AMOTA_AMOTAS_TX_CCC_IDX */
    {LOWSAPP_CH1_CH_CCC_HDL,         ATT_CLIENT_CFG_NOTIFY,    DM_SEC_LEVEL_NONE},   /* LOWSAPP_CH1_CCC_IDX */
    {LOWSAPP_CH2_CH_CCC_HDL,         ATT_CLIENT_CFG_NOTIFY,    DM_SEC_LEVEL_NONE},   /* LOWSAPP_CH2_CCC_IDX */
    {LOWSAPP_CH3_CH_CCC_HDL,         ATT_CLIENT_CFG_NOTIFY,    DM_SEC_LEVEL_NONE}    /* LOWSAPP_CH3_CCC_IDX */
};

uint32_t incall_notify_uid;  //for incoming call

//fix 1:ANCS服务发现，解决手机只开蓝牙，不开应用APP，也可以支持ANCS功能
static ble_echo pAncs_CB = NULL;
static uint8_t u8DiscStatus = 0x00;
//fix 

/************************************************************************************************/
/*!
 *  \fn     m_ble_profileDmCback
 *
 *  \brief  Application DM callback.
 *
 *  \param  pDmEvt  DM callback event
 *
 *  \return None.
 */
/*************************************************************************************************/
static void m_ble_profileDmCback(dmEvt_t *pDmEvt)
{
    dmEvt_t *pMsg;

    if ((pMsg = WsfMsgAlloc(sizeof(dmEvt_t))) != NULL)
    {
        memcpy(pMsg, pDmEvt, sizeof(dmEvt_t));
        WsfMsgSend(m_ble_profile_cb.handlerId, pMsg);
    }
}

/*************************************************************************************************/
/*!
 *  \fn     m_ble_profileAttCback
 *
 *  \brief  Application ATT callback.
 *
 *  \param  pEvt    ATT callback event
 *
 *  \return None.
 */
/*************************************************************************************************/
static void m_ble_profileAttCback(attEvt_t *pEvt)
{
    attEvt_t *pMsg;

    if ((pMsg = WsfMsgAlloc(sizeof(attEvt_t) + pEvt->valueLen)) != NULL)
    {
        memcpy(pMsg, pEvt, sizeof(attEvt_t));
        pMsg->pValue = (uint8_t *) (pMsg + 1);
        memcpy(pMsg->pValue, pEvt->pValue, pEvt->valueLen);
        WsfMsgSend(m_ble_profile_cb.handlerId, pMsg);
    }
}

/*************************************************************************************************/
/*!
 *  \fn     m_ble_profileCccCback
 *
 *  \brief  Application ATTS client characteristic configuration callback.
 *
 *  \param  pDmEvt  DM callback event
 *
 *  \return None.
 */
/*************************************************************************************************/
static void m_ble_profileCccCback(attsCccEvt_t *pEvt)
{
    attsCccEvt_t  *pMsg;
    appDbHdl_t    dbHdl;

    /* if CCC not set from initialization and there's a device record */
    if ((pEvt->handle != ATT_HANDLE_NONE) &&
        ((dbHdl = AppDbGetHdl((dmConnId_t) pEvt->hdr.param)) != APP_DB_HDL_NONE))
    {
        /* store value in device database */
        AppDbSetCccTblValue(dbHdl, pEvt->idx, pEvt->value);
    }

    if ((pMsg = WsfMsgAlloc(sizeof(attsCccEvt_t))) != NULL)
    {
        memcpy(pMsg, pEvt, sizeof(attsCccEvt_t));
        WsfMsgSend(m_ble_profile_cb.handlerId, pMsg);
    }
}

/*************************************************************************************************/
/*!
 *  \fn     m_ble_profileClose
 *
 *  \brief  Perform UI actions on connection close.
 *
 *  \param  pMsg    Pointer to message.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void m_ble_profileClose(m_ble_profile_msg_t *pMsg)
{
//    amotas_conn_close((dmConnId_t) pMsg->hdr.param);
//    lowsapp_conn_close((dmConnId_t) pMsg->hdr.param);
}

/*************************************************************************************************/
/*!
 *  \fn     m_ble_profileValueUpdate
 *
 *  \brief  Process a received ATT read response, notification, or indication.
 *
 *  \param  pMsg    Pointer to ATT callback event message.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void m_ble_profileValueUpdate(attEvt_t *pMsg)
{
    /* iOS notification */
    if ((pMsg->handle == pAncsAnccHdlList[ANCC_NOTIFICATION_SOURCE_HDL_IDX]) ||
        (pMsg->handle == pAncsAnccHdlList[ANCC_DATA_SOURCE_HDL_IDX]) ||
        (pMsg->handle == pAncsAnccHdlList[ANCC_CONTROL_POINT_HDL_IDX]))
    {
        AnccNtfValueUpdate(pAncsAnccHdlList, pMsg, ANCC_ACTION_TIMER_IND);
    }
    else
    {
        Ble_Debug((0,"Data received from other other handle\n"));
    }

    /* GATT */
    if (GattValueUpdate(pAncsGattHdlList, pMsg) == ATT_SUCCESS)
    {
        return;
    }
}

/*************************************************************************************************/
/*!
 *  \fn     m_ble_profileDiscCback
 *
 *  \brief  Discovery callback.
 *
 *  \param  connId    Connection identifier.
 *  \param  status    Service or configuration status.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void m_ble_profileDiscCback(dmConnId_t connId, uint8_t status)
{
    switch(status)
    {
    case APP_DISC_INIT:
        /* set handle list when initialization requested */
        AppDiscSetHdlList(connId, ANCS_DISC_HDL_LIST_LEN, m_ble_profile_cb.hdlList);
        break;
    case APP_DISC_SEC_REQUIRED:
        /* request security */
        //AppSlaveSecurityReq(connId);
        break;
    case APP_DISC_START:
        /* initialize discovery state */
        m_ble_profile_cb.discState = ANCS_DISC_GATT_SVC;

        /* discover GATT service */
        GattDiscover(connId, pAncsGattHdlList);
        break;
    case APP_DISC_FAILED:
        u8DiscStatus = 0x01;
        Ble_Debug((0,"!!!!!Disc Failed. discState = %d!!!!!\n", m_ble_profile_cb.discState));
    case APP_DISC_CMPL:
        //expecting only m_ble_profile service to be discovered
        m_ble_profile_cb.discState++;
        if (m_ble_profile_cb.discState == ANCS_DISC_ANCS_SVC)
        {
            /* discover ANCS service */
            AnccSvcDiscover(connId, pAncsAnccHdlList);
            Ble_Debug((0,"Discovering ANCS.\n"));
        }
        else
        {
            /* discovery complete */
            AppDiscComplete(connId, APP_DISC_CMPL);
            Ble_Debug((0,"Finished ANCS discovering.\n"));

            /* start configuration */
            Ble_Debug((0,"Disc CFG start.\n"));
            AppDiscConfigure(connId, APP_DISC_CFG_START, ANCS_DISC_CFG_LIST_LEN,
                         (attcDiscCfg_t *) m_ble_profileDiscCfgList, ANCS_DISC_HDL_LIST_LEN, m_ble_profile_cb.hdlList);

            //fix 1:ANCS服务发现，解决手机只开蓝牙，不开应用APP，也可以支持ANCS功能
            // IOS手机不会发生APP_DISC_FAILED事件，Android手机会有此事件
            if((pAncs_CB != NULL) && (u8DiscStatus != 0x01))
            {
                (pAncs_CB)(BLE_ANCS_DISCOVERING,0,0);
                u8DiscStatus = 0x00;
            }
            //fix: 2018.6.8
        }
        break;
    case APP_DISC_CFG_START:
        /* start configuration */
        AppDiscConfigure(connId, APP_DISC_CFG_START, ANCS_DISC_CFG_LIST_LEN,
                       (attcDiscCfg_t *) m_ble_profileDiscCfgList, ANCS_DISC_HDL_LIST_LEN, m_ble_profile_cb.hdlList);
        break;
    case APP_DISC_CFG_CMPL:
        AppDiscComplete(connId, status);
        Ble_Debug((0,"Finished Disc CFG."));
        break;
    case APP_DISC_CFG_CONN_START:
        /* start connection setup configuration */
        AppDiscConfigure(connId, APP_DISC_CFG_CONN_START, ANCS_DISC_CFG_LIST_LEN,
                       (attcDiscCfg_t *) m_ble_profileDiscCfgList, ANCS_DISC_HDL_LIST_LEN, m_ble_profile_cb.hdlList);
        break;
    default:
        break;
    }
}

/*************************************************************************************************/
/*!
 *  \fn     amdtpProcCccState
 *
 *  \brief  Process CCC state change.
 *
 *  \param  pMsg    Pointer to message.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void lowsappProcCccState(lowsappMsg_t *pMsg)
{
    //Ble_Debug((0,"ccc state ind value:%d handle:%d idx:%d\n", pMsg->ccc.value, pMsg->ccc.handle, pMsg->ccc.idx));
    
    //  dmConnId_t connId = (dmConnId_t) pMsg->ccc.hdr.param;
    /* M_BLES TX CCC */
    if (pMsg->ccc.idx == LOWSAPP_CH1_CCC_IDX)
    {
        if (pMsg->ccc.value == ATT_CLIENT_CFG_NOTIFY)
        {        
            // notify enabled
            lowsapp_start((dmConnId_t) pMsg->ccc.hdr.param, 1);
        }
        else
        {
            // notify disabled
            lowsapp_stop((dmConnId_t) pMsg->ccc.hdr.param,1);
        }
    }
    else if (pMsg->ccc.idx == LOWSAPP_CH2_CCC_IDX)
    {
        if (pMsg->ccc.value == ATT_CLIENT_CFG_NOTIFY)
        {
            // notify enabled
            lowsapp_start((dmConnId_t) pMsg->ccc.hdr.param, 2);
        }
        else
        {
            // notify disabled
            lowsapp_stop((dmConnId_t) pMsg->ccc.hdr.param,2);
        }
    }
    else if (pMsg->ccc.idx == LOWSAPP_CH3_CCC_IDX)
    {
        if (pMsg->ccc.value == ATT_CLIENT_CFG_NOTIFY)
        {
            // notify enabled
            lowsapp_start((dmConnId_t) pMsg->ccc.hdr.param, 3);
        }
        else
        {
            // notify disabled
            lowsapp_stop((dmConnId_t) pMsg->ccc.hdr.param,3);
        }
    } 
}

/*************************************************************************************************/
/*!
 *  \fn     amotaProcCccState
 *
 *  \brief  Process CCC state change.
 *
 *  \param  pMsg    Pointer to message.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void amotaProcCccState(m_ble_profile_msg_t *pMsg)
{
    APP_TRACE_INFO3("ccc state ind value:%d handle:%d idx:%d", pMsg->ccc.value, pMsg->ccc.handle, pMsg->ccc.idx);

    /* handle heart rate measurement CCC */
    /* AMOTAS TX CCC */
    if (pMsg->ccc.idx == AMOTA_AMOTAS_TX_CCC_IDX)
    {
        if (pMsg->ccc.value == ATT_CLIENT_CFG_NOTIFY)
        {
            // notify enabled
            amotas_start((dmConnId_t) pMsg->ccc.hdr.param, AMOTA_RESET_TIMER_IND, AMOTA_DISCONNECT_TIMER_IND,AMOTA_AMOTAS_TX_CCC_IDX);
        }
        else
        {
            // notify disabled
            amotas_stop((dmConnId_t) pMsg->ccc.hdr.param);
        }
        return;
    }
}

/*************************************************************************************************/
/*!
 *  \fn     m_ble_profileProcMsg
 *
 *  \brief  Process messages from the event handler.
 *
 *  \param  pMsg    Pointer to message.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void m_ble_profileProcMsg(m_ble_profile_msg_t *pMsg)
{
    uint8_t uiEvent = APP_UI_NONE;

    switch(pMsg->hdr.event)
    {
    case ANCC_ACTION_TIMER_IND:
        AnccActionHandler(ANCC_ACTION_TIMER_IND);
        break;
    case AMOTA_RESET_TIMER_IND:
    case AMOTA_DISCONNECT_TIMER_IND:
        amotas_proc_msg(&pMsg->hdr);
        break;
    case ATTC_READ_RSP:
    case ATTC_HANDLE_VALUE_NTF:
    case ATTC_HANDLE_VALUE_IND:
        Ble_Debug((0,"-------ATTC_HANDLE_VALUE_NTF/IND-------\n"));
        m_ble_profileValueUpdate((attEvt_t *) pMsg);
        break;
    case ATTC_WRITE_RSP:    // write respose after Control point operation. 
        Ble_Debug((0,"------ATTC_WRITE_RSP--------\n"));
        break;
    case ATTS_HANDLE_VALUE_CNF:
        //Ble_Debug((0,"------ATTS_HANDLE_VALUE_CNF--------\n"));
		lowsapp_proc_msg(&pMsg->hdr);
        break;
    case ATTS_CCC_STATE_IND:
        lowsappProcCccState((lowsappMsg_t*)pMsg);
        amotaProcCccState(pMsg);
        break;
    case ATT_MTU_UPDATE_IND:
        Ble_Debug((0,"Negotiated MTU %d\n", ((attEvt_t *)pMsg)->mtu));
        break;
    case DM_RESET_CMPL_IND:
        Ble_Debug((0,"------DM_RESET_CMPL_IND--------\n"));
        //DmSecGenerateEccKeyReq();
        uiEvent = APP_UI_RESET_CMPL;
        break;
    case DM_ADV_START_IND:
        Ble_Debug((0,"------DM_ADV_START_IND--------\n"));
        uiEvent = APP_UI_ADV_START;
        break;
    case DM_ADV_STOP_IND:
        Ble_Debug((0,"------DM_ADV_STOP_IND--------\n"));
        uiEvent = APP_UI_ADV_STOP;
        break;
    case DM_CONN_OPEN_IND:
        /* set bondable here to enable bond/pair after disconnect */
        AppSetBondable(TRUE);   //20180508 ver 1.2.12 delete
        lowsapp_proc_msg(&pMsg->hdr);
        amotas_proc_msg(&pMsg->hdr);
        AnccConnOpen(pMsg->hdr.param, pAncsAnccHdlList);
        Ble_Debug((0,"DM_CONN_OPEN_IND: %d\n",pMsg->hdr.param));
        uiEvent = APP_UI_CONN_OPEN;
        break;
    case DM_CONN_CLOSE_IND:
        lowsapp_proc_msg(&pMsg->hdr);
        amotas_proc_msg(&pMsg->hdr);
        m_ble_profileClose(pMsg);
        AnccConnClose();

        u8DiscStatus = 0x00;
        //Err_Info((0,"Warning: Connection Closed due to: 0x%0x\n",((hciDisconnectCmplEvt_t*)pMsg)->reason));
        uiEvent = APP_UI_CONN_CLOSE;
        break;
    case DM_CONN_UPDATE_IND:
        Ble_Debug((0,"------DM_CONN_UPDATE_IND--------\n"));
        lowsapp_proc_msg(&pMsg->hdr);
        amotas_proc_msg(&pMsg->hdr);
        break;
    case DM_SEC_PAIR_CMPL_IND:
        uiEvent = APP_UI_SEC_PAIR_CMPL;
        Ble_Debug((0,"------MTU SIZE = %d-------\n", AttGetMtu(pMsg->hdr.param)));
        break;
    case DM_SEC_PAIR_FAIL_IND:
        uiEvent = APP_UI_SEC_PAIR_FAIL;
        break;
    case DM_SEC_ENCRYPT_IND:
        uiEvent = APP_UI_SEC_ENCRYPT;
        break;
    case DM_SEC_ENCRYPT_FAIL_IND:
        uiEvent = APP_UI_SEC_ENCRYPT_FAIL;
        break;
    case DM_SEC_AUTH_REQ_IND:
        AppHandlePasskey(&pMsg->dm.authReq);
        break;
    default:
        break;
    }

    if (uiEvent != APP_UI_NONE)
    {
        AppUiAction(uiEvent);
    }
}

/*************************************************************************************************/
/*!
 *  \fn     AncsHandlerInit
 *
 *  \brief  Application handler init function called during system initialization.
 *
 *  \param  handlerID  WSF handler ID.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AncsHandlerInit(wsfHandlerId_t handlerId)
{
    /* store handler ID */
    m_ble_profile_cb.handlerId = handlerId;

    /* Initialize application framework */
    amotas_init(handlerId, (AmotasCfg_t *) &amotasCfg);
    AnccInit(handlerId, (anccCfg_t*)(&m_ble_profileAnccCfg));
	lowsapp_init(handlerId);
}

/*************************************************************************************************/
/*!
 *  \fn     AncsHandler
 *
 *  \brief  WSF event handler for application.
 *
 *  \param  event   WSF event mask.
 *  \param  pMsg    WSF message.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AncsHandler(wsfEventMask_t event, wsfMsgHdr_t *pMsg)
{
    if (pMsg != NULL)
    {
        //Ble_Debug((0,"ANCS got evt %d on handle 0x%04x\n", pMsg->event, ((attEvt_t *)pMsg)->handle));
        if (pMsg->event <= ATT_CBACK_END)    //process discovery-related ATT messages
        {   
            AppDiscProcAttMsg((attEvt_t *) pMsg);	//process ATT messages
        }
        else if (pMsg->event >= DM_CBACK_START && pMsg->event <= DM_CBACK_END)
        {
            /* process advertising and connection-related messages */
            AppSlaveProcDmMsg((dmEvt_t *) pMsg);
            
            /* process security-related messages */
            AppSlaveSecProcDmMsg((dmEvt_t *) pMsg);

            /* process discovery-related messages */
            AppDiscProcDmMsg((dmEvt_t *) pMsg);
        }

        /* perform profile and user interface-related operations */
        m_ble_profileProcMsg((m_ble_profile_msg_t *) pMsg);
    }
}

void m_ble_profileAnccAttrCback(active_notif_t* pAttr)
{   
    uint8_t event_flags = anccCb.anccList[pAttr->handle].event_flags;

    //fix 2:system will be in OTA mode, other module will stop working.
    if(app_m_ble_getmode() == BLE_MODE_OTA)
    {
        return;
    }
    //fix: 2018.6.12
      
    if( event_flags & (0x1 << BLE_ANCS_EVENT_FLAG_PREEXISTING))
    {
        Ble_Debug((0,"* pre_existing nitification \n"));
        return;
    }
    Ble_Debug((0,"* event_id             = %d\n", anccCb.anccList[pAttr->handle].event_id));    
    // this is an application demo, print the notification info
    if(pAttr->attrId == BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER)
    {
        Ble_Debug((0,"************************************************************\n"));
        Ble_Debug((0,"* Notification Received \n"));
        Ble_Debug((0,"* UID             = %d\n", anccCb.anccList[pAttr->handle].notification_uid));
        if(m_ble_ancs_app_attr_process_handler != NULL)
        {
            m_ble_ancs_app_attr_process_handler(&(pAttr->attrDataBuf[pAttr->parseIndex]),pAttr->attrLength,START_GET_ATTR);
        }        
       
        switch(anccCb.anccList[pAttr->handle].event_id)
        {
        case BLE_ANCS_EVENT_ID_NOTIFICATION_ADDED:
             Ble_Debug((0,"* Event ID        = Added\n"));
             //新来的消息是已经存在的，过滤掉
             
             
             switch(anccCb.anccList[pAttr->handle].category_id)
             {
                case BLE_ANCS_CATEGORY_ID_INCOMING_CALL:
                Ble_Debug((0,"* Category        = Incoming Call\n"));
                incall_notify_uid = anccCb.anccList[pAttr->handle].notification_uid;
                if(m_ble_ancs_app_id_process_handler!=NULL)
                {
                    const uint8_t CalllingIdentifier[] =         {"com.apple.mobilephone_calling"};          //call     为了区分来电和未接来电，自定义                    
                    m_ble_ancs_app_id_process_handler(CalllingIdentifier,sizeof(CalllingIdentifier)-1,BLE_ANCS_EVENT_ID_NOTIFICATION_ADDED);
                    if(m_ble_ancs_app_attr_process_handler != NULL)
                    {
                        m_ble_ancs_app_attr_process_handler(CalllingIdentifier,sizeof(CalllingIdentifier)-1,START_GET_APP_ID);
                    }
                }
                break;
                
                case BLE_ANCS_CATEGORY_ID_MISSED_CALL:
                Ble_Debug((0,"* Category        = Missed Call\n"));
                if(m_ble_ancs_app_id_process_handler!=NULL)
                {
                    const uint8_t MissCalllingIdentifier[] =     {"com.apple.mobilephone_miss_calling"};
                    m_ble_ancs_app_id_process_handler(MissCalllingIdentifier,sizeof(MissCalllingIdentifier)-1,BLE_ANCS_EVENT_ID_NOTIFICATION_ADDED);
                    if(m_ble_ancs_app_attr_process_handler != NULL)
                    {
                        m_ble_ancs_app_attr_process_handler(MissCalllingIdentifier,sizeof(MissCalllingIdentifier)-1,START_GET_APP_ID);
                    }
                }
                break;
                default:
                    if(m_ble_ancs_app_id_process_handler!=NULL)
                    {
                        m_ble_ancs_app_id_process_handler(&(pAttr->attrDataBuf[pAttr->parseIndex]),pAttr->attrLength,BLE_ANCS_EVENT_ID_NOTIFICATION_ADDED);
                        if(m_ble_ancs_app_attr_process_handler != NULL)
                        {
                            m_ble_ancs_app_attr_process_handler(&(pAttr->attrDataBuf[pAttr->parseIndex]),pAttr->attrLength,START_GET_APP_ID);
                        }
                    }
                break;
            }           
            break;
            
        case BLE_ANCS_EVENT_ID_NOTIFICATION_REMOVED:
            Ble_Debug((0,"* Event ID        = Removed\n"));
            Ble_Debug((0,"* Event ID        = Added\n"));
             switch(anccCb.anccList[pAttr->handle].category_id)
             {       
                case BLE_ANCS_CATEGORY_ID_MISSED_CALL:
                Ble_Debug((0,"* Category        = Missed Call\n"));
                if(m_ble_ancs_app_id_process_handler!=NULL)
                {
                    const uint8_t MissCalllingIdentifier[] =     {"com.apple.mobilephone_miss_calling"};
                    m_ble_ancs_app_id_process_handler(MissCalllingIdentifier,sizeof(MissCalllingIdentifier)-1,BLE_ANCS_EVENT_ID_NOTIFICATION_REMOVED);
                    if(m_ble_ancs_app_attr_process_handler != NULL)
                    {
                        m_ble_ancs_app_attr_process_handler(MissCalllingIdentifier,sizeof(MissCalllingIdentifier)-1,START_GET_APP_ID);
                    }
                }
                break;
                default:
                    if(m_ble_ancs_app_id_process_handler!=NULL)
                    {
                        m_ble_ancs_app_id_process_handler(&(pAttr->attrDataBuf[pAttr->parseIndex]),pAttr->attrLength,BLE_ANCS_EVENT_ID_NOTIFICATION_ADDED);
                        if(m_ble_ancs_app_attr_process_handler != NULL)
                        {
                            m_ble_ancs_app_attr_process_handler(&(pAttr->attrDataBuf[pAttr->parseIndex]),pAttr->attrLength,START_GET_APP_ID);
                        }
                    }
                break;
            }
            break;
        default:
            break;
        }
//        Ble_Debug((0,"* EventFlags      = \n"));
//        Ble_Debug((0,"* Category Count  = %d\n", anccCb.anccList[pAttr->handle].category_count));
    }
    
    else if (pAttr->attrId == BLE_ANCS_NOTIF_ATTR_ID_TITLE)
    {
        if(pAttr->attrLength != 0)
        {
            Ble_Debug((0,"* Title           = \n"));
            Ble_Debug((0,"%s\n",&(pAttr->attrDataBuf[pAttr->parseIndex])));
            if(m_ble_ancs_app_attr_process_handler != NULL)
            {
                m_ble_ancs_app_attr_process_handler(&(pAttr->attrDataBuf[pAttr->parseIndex]),pAttr->attrLength,GET_TITLE);
            }
        }
    }
    else if (pAttr->attrId == BLE_ANCS_NOTIF_ATTR_ID_SUBTITLE)
    {
        if(pAttr->attrLength != 0)
        {
//            Ble_Debug((0,"* Subtitle        = \n"));
//            Ble_Debug((0,"%s\n",&(pAttr->attrDataBuf[pAttr->parseIndex])));
            
        }
        if(m_ble_ancs_app_attr_process_handler != NULL)
        {
            m_ble_ancs_app_attr_process_handler(&(pAttr->attrDataBuf[pAttr->parseIndex]),pAttr->attrLength,GET_SUBTITLE);
        }
    }
    else if (pAttr->attrId == BLE_ANCS_NOTIF_ATTR_ID_MESSAGE)
    {
        if(pAttr->attrLength != 0)
        {
            Ble_Debug((0,"* Message         = \n"));
            Ble_Debug((0,"%s\n",&(pAttr->attrDataBuf[pAttr->parseIndex])));
            if(m_ble_ancs_app_attr_process_handler != NULL)
            {
                m_ble_ancs_app_attr_process_handler(&(pAttr->attrDataBuf[pAttr->parseIndex]),pAttr->attrLength,GET_MSG);
            }
        }
    }
    else if (pAttr->attrId == BLE_ANCS_NOTIF_ATTR_ID_DATE)
    {
        if(pAttr->attrLength != 0)
        {
            Ble_Debug((0,"* Date & Time     = \n"));
            Ble_Debug((0,"%s\n",&(pAttr->attrDataBuf[pAttr->parseIndex])));
        }
    }
    else if (pAttr->attrId == BLE_ANCS_NOTIF_ATTR_ID_POSITIVE_ACTION_LABEL)
    {
        if(pAttr->attrLength != 0)
        {
            Ble_Debug((0,"* Positive Action = \n"));
            Ble_Debug((0,"%s\n",&(pAttr->attrDataBuf[pAttr->parseIndex])));
        }
    }
    else if (pAttr->attrId == BLE_ANCS_NOTIF_ATTR_ID_NEGATIVE_ACTION_LABEL)
    {
        if(pAttr->attrLength != 0)
        {
            Ble_Debug((0,"* Negative Action = \n"));
            Ble_Debug((0,"%s\n",&(pAttr->attrDataBuf[pAttr->parseIndex])));
        }
    }
}

void m_ble_profileAnccNotifCback(active_notif_t* pAttr, uint32_t notiUid)
{
    // removes notifications received
    // AncsPerformNotiAction(pNotiAnccHdlList, notiUid, BLE_ANCS_NOTIF_ACTION_ID_NEGATIVE);
    //Ble_Debug((0,"************************************************************\n"));
}


/*BLE数据发送函数，通过notify发送
type :命令通道或者数据通道*/
void m_ble_profile_send_data(uint8_t *buf, uint8_t len,uint8_t type)
{
    if((buf == NULL) || (len == 0))
    {
        Err_Info((0,"BLE Send Data param is invalid\n"));
        return ;
    }
    lowsapp_send_data(buf,len,type);
}

//**********************************************************************
// 函数功能: 来电拒接，拒接当前来电
// 输入参数：
// 返回参数：
//**********************************************************************
void Ble_Ancs_RejectCall(void)
{
    AncsPerformNotiAction(anccCb.hdlList, incall_notify_uid, BLE_ANCS_NOTIF_ACTION_ID_NEGATIVE);
}

/*************************************************************************************************/
/*!
 *  \fn     AncsStart
 *
 *  \brief  Start the application.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AncsStart(void)
{
    /* Register for stack callbacks */
    DmRegister(m_ble_profileDmCback);
    DmConnRegister(DM_CLIENT_ID_APP, m_ble_profileDmCback);
    AttRegister(m_ble_profileAttCback);
    AttConnRegister(AppServerConnCback);
    AttsCccRegister(ANCS_NUM_CCC_IDX, (attsCccSet_t *) m_ble_profileCccSet, m_ble_profileCccCback);

    /* Register for app framework discovery callbacks */
    AppDiscRegister(m_ble_profileDiscCback);

    // Register for ancc callbacks
    AnccCbackRegister(m_ble_profileAnccAttrCback, m_ble_profileAnccNotifCback);

    /* Initialize attribute server database */
    SvcCoreAddGroup();
    SvcDisAddGroup();
       
    SvcAmotasCbackRegister(NULL, amotas_write_cback);
    SvcAmotasAddGroup();
    
    SvcLowsappAddGroup();
    //SvcLowsappCbackRegister(NULL,m_ble_profile_write_cback);
	SvcLowsappCbackRegister(NULL,lowsapp_write_cback);

    /* Reset the device */
    DmDevReset();
}

//fix :ANCS复位发现，解决手机只开蓝牙，不开应用APP，也可以支持ANCS功能
//set callback for ANCS
void m_ble_ancs_callback(ble_echo pfun)
{
    if(pfun == NULL)
        return ;
    pAncs_CB = pfun;
}
//fix

/*注册ancs回调数据处理函数
ancs_app_id_process_handler     app id   处理回调函数，可用来判断提醒类型
ancs_app_attr_process_handler   app attr 处理回调函数，可用来获取具体消息内容
*/
void m_ble_ancs_process_handler_init(m_ble_ancs_app_id_process_handler_t ancs_app_id_process_handler,
                                     m_ble_ancs_app_attr_process_handler_t ancs_app_attr_process_handler)
{
    m_ble_ancs_app_id_process_handler = ancs_app_id_process_handler;
    m_ble_ancs_app_attr_process_handler = ancs_app_attr_process_handler;
}

/*初始化，数据及命令通道接收回调和ota数据通道回调
提供给上层调用注册*/
void m_ble_profile_transfer_data_init(m_ble_profile_recv_data_handler_t recv_data_handler)
{
	if(recv_data_handler != NULL)
    {
		lowsapp_transfer_data_init(recv_data_handler);
	}
}

/*检查ble发送是否空闲*/
bool_t m_ble_profile_send_idle_check(void)
{
    return lowsapp_send_idle_check();
}

