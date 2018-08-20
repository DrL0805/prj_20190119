#include "platform_debugcof.h"
#include "platform_common.h"
#include "BLE_Stack.h"
#include "wsf_types.h"
#include "wsf_assert.h"
#include "wsf_trace.h"
#include "bstream.h"
#include "att_api.h"
#include "svc_ch.h"
#include "svc_lowsapp.h"
#include "app_api.h"
#include "app_hw.h"
#include "lowsapp_main.h"
#include "svc_lowsapp.h"
#include "crc32.h"
#include "app_m_ble_profile.h"

// Connection control block
volatile uint32_t u16Systime =0,u16time =0;

/* Control block */
static struct
{
    lowsappConn_t           conn[DM_CONN_MAX];    // connection control block
    bool_t                  txReady;              // TRUE if ready to send notifications
    wsfHandlerId_t          appHandlerId;
    m_ble_profile_recv_data_handler_t lowsapp_receive_data_handler;
    bool_t                  ch1_enable;
    bool_t                  ch2_enable;
    bool_t                  ch3_enable;
}lowsappCb;

#if 0  //unused
//*****************************************************************************
//
// Find Next Connection to Send on
//
//*****************************************************************************
static lowsappConn_t* lowsapp_find_next2send(void)
{
    lowsappConn_t *pConn = lowsappCb.conn;
    uint8_t i;

    for (i = 0; i < DM_CONN_MAX; i++, pConn++)
    {
        if (pConn->connId != DM_CONN_ID_NONE && pConn->lowsappToSend)
        {
            //if (AttsCccEnabled(pConn->connId, cccIdx))
            return pConn;
        }
    }
    return NULL;
}
#endif

//*****************************************************************************
//
//! @brief initialize amota service
//!
//! @param handlerId - connection handle
//! @param pCfg - configuration parameters
//!
//! @return None
//
//*****************************************************************************
void lowsapp_init(wsfHandlerId_t handlerId)
{
    lowsappCb.appHandlerId = handlerId;
    lowsappCb.txReady = FALSE;
    for (int i = 0; i < DM_CONN_MAX; i++)
    {
        lowsappCb.conn[i].connId = DM_CONN_ID_NONE;
    }
}

//*****************************************************************************
//
// Connection Open event
//
//*****************************************************************************
static void lowsapp_conn_open(dmEvt_t *pMsg)
{
    lowsappCb.txReady = TRUE;
#ifdef AM_DEBUG_PRINTF
    hciLeConnCmplEvt_t *evt = (hciLeConnCmplEvt_t*) pMsg;

    APP_TRACE_INFO0("connection opened");
    APP_TRACE_INFO1("handle = 0x%x", evt->handle);
    APP_TRACE_INFO1("role = 0x%x", evt->role);
    APP_TRACE_INFO3("addrMSB = %02x%02x%02x%02x%02x%02x", evt->peerAddr[0], evt->peerAddr[1], evt->peerAddr[2]);
    APP_TRACE_INFO3("addrLSB = %02x%02x%02x%02x%02x%02x", evt->peerAddr[3], evt->peerAddr[4], evt->peerAddr[5]);
    APP_TRACE_INFO1("connInterval = 0x%x", evt->connInterval);
    APP_TRACE_INFO1("connLatency = 0x%x", evt->connLatency);
    APP_TRACE_INFO1("supTimeout = 0x%x", evt->supTimeout);
#else
    pMsg = pMsg; //for compile 
#endif
}

static void lowsapp_conn_close(dmEvt_t *pMsg)
{
    hciDisconnectCmplEvt_t *evt = (hciDisconnectCmplEvt_t*) pMsg;
    dmConnId_t connId = evt->hdr.param;
    /* clear connection */
    lowsappCb.conn[connId - 1].connId = DM_CONN_ID_NONE;
    lowsappCb.conn[connId - 1].lowsappToSend = FALSE;
}

//*****************************************************************************
//
// Connection Update event
//
//*****************************************************************************
static void lowsapp_conn_update(dmEvt_t *pMsg)
{
#ifdef AM_DEBUG_PRINTF
    hciLeConnUpdateCmplEvt_t *evt = (hciLeConnUpdateCmplEvt_t*) pMsg;

    APP_TRACE_INFO1("connection update status = 0x%x", evt->status);
    APP_TRACE_INFO1("handle = 0x%x", evt->handle);
    APP_TRACE_INFO1("connInterval = 0x%x", evt->connInterval);
    APP_TRACE_INFO1("connLatency = 0x%x", evt->connLatency);
    APP_TRACE_INFO1("supTimeout = 0x%x", evt->supTimeout);
#else
    pMsg = pMsg; //for compile 
#endif
}

/*BLE数据发送函数，通过notify发送
type :命令通道或者数据通道*/
void lowsapp_send_data(uint8_t *buf, uint8_t len,uint8_t type)
{
    dmConnId_t connId = AppConnIsOpen();
    /* send notification */
    if (connId)
    {        
        if((type == SEND_CHANNEL_CMD) && (lowsappCb.ch1_enable))
        {
            lowsappCb.txReady = false;
            AttsHandleValueNtf(connId, LOWSAPP_CH1_HDL, len, buf);            
        }
        else if((type == SEND_CHANNEL_DATA) && (lowsappCb.ch2_enable))
        {
            lowsappCb.txReady = false;
            AttsHandleValueNtf(connId, LOWSAPP_CH2_HDL, len, buf);  
        }
        else    
        {
            Err_Info((0,"Invalid CH or Not enable notification"));
        }
        
        // Signal radio task to run 
        WsfTaskSetReady(0,0);
    } 
    else
    {
        Err_Info((0,"Invalid Conn = %d\n\r\n", lowsappCb.appHandlerId));
    }
}

/*ble数据接收回调，app发过来的数据在此处理*/
uint8_t lowsapp_write_cback(dmConnId_t connId, uint16_t handle, uint8_t operation,
                       uint16_t offset, uint16_t len, uint8_t *pValue, attsAttr_t *pAttr)
{
    //fix :system will be in OTA mode, other module will stop working.
    if(app_m_ble_getmode() == BLE_MODE_OTA)
    {
        return ATT_SUCCESS;
    }
    //fix: 2018.6.12

    if(handle == LOWSAPP_CH2_HDL)
    {
        //SEGGER_RTT_printf(0,"data from CH2 len : %d\n",len);
        if(lowsappCb.lowsapp_receive_data_handler != NULL)
        {
            lowsappCb.lowsapp_receive_data_handler(pValue,len);
        }
    }
    else if(handle == LOWSAPP_CH1_HDL)
    {
        //SEGGER_RTT_printf(0,"data from CH1 len : %d\n",len);
        if((len == 9) && (pValue[3] == 0x32)&&(pValue[5] == 0x80) && (pValue[6] == 0x00) && (pValue[7] == 0x07))//ios app 请求配对
        {
            //SEGGER_RTT_printf(0,"AppSlaveSecurityReq connId : %d\n",connId);
            AppSlaveSecurityReq(connId);
        }
        else
        {
            if(lowsappCb.lowsapp_receive_data_handler != NULL)
            {
                lowsappCb.lowsapp_receive_data_handler(pValue,len);
            }
        }
    }
    else
        ;
    return ATT_SUCCESS;
}

void lowsapp_start(dmConnId_t connId,uint8_t lowsappCccIdx)
{
    // set conn id
    lowsappCb.conn[connId - 1].connId = connId;
    lowsappCb.conn[connId - 1].lowsappToSend = TRUE;
    if(lowsappCccIdx == 1)
    {
        lowsappCb.ch1_enable = true;
    }
    else if(lowsappCccIdx == 2)
    {
        lowsappCb.ch2_enable = true;
    }
    else if(lowsappCccIdx == 3)
    {
        lowsappCb.ch3_enable = true;
    }
}

void lowsapp_stop(dmConnId_t connId,uint8_t lowsappCccIdx)
{
    // clear connection
    lowsappCb.conn[connId - 1].connId = DM_CONN_ID_NONE;
    lowsappCb.conn[connId - 1].lowsappToSend = FALSE;
    
    if(lowsappCccIdx == 1)
    {
        lowsappCb.ch1_enable = false;
    }
    else if(lowsappCccIdx == 2)
    {
        lowsappCb.ch2_enable = false;
    }
    else if(lowsappCccIdx == 3)
    {
        lowsappCb.ch3_enable = false;
    }
}

static void lowsapp_handle_value_cnf(attEvt_t *pMsg)
{
    //SEGGER_RTT_printf(0,"Cnf status = %d, handle = 0x%x\r\n", pMsg->hdr.status, pMsg->handle);
    if (pMsg->hdr.status == ATT_SUCCESS)
    {
        if (pMsg->handle == LOWSAPP_CH1_HDL)
        {
            lowsappCb.txReady = true;
        }
        else if (pMsg->handle == LOWSAPP_CH2_HDL)
        {
            lowsappCb.txReady = true;
        }
    }
}

//*****************************************************************************
//
//! @brief initialize amota service
//!
//! @param pMsg - WSF message
//!
//! @return None
//
//*****************************************************************************
void lowsapp_proc_msg(wsfMsgHdr_t *pMsg) 
{
    if (pMsg->event == DM_CONN_OPEN_IND)
    {
        lowsapp_conn_open((dmEvt_t *) pMsg);
    }
    else if (pMsg->event == DM_CONN_UPDATE_IND)
    {
        lowsapp_conn_update((dmEvt_t *) pMsg);
    }
    else if (pMsg->event == DM_CONN_CLOSE_IND)
    {
        lowsapp_conn_close((dmEvt_t *) pMsg);
    }
    else if (pMsg->event == ATTS_HANDLE_VALUE_CNF)
    {
        lowsapp_handle_value_cnf((attEvt_t *) pMsg);
    }
}

void lowsapp_transfer_data_init(m_ble_profile_recv_data_handler_t recv_data_handler)
{
    memset(&lowsappCb, 0, sizeof(lowsappCb));
    lowsappCb.lowsapp_receive_data_handler = recv_data_handler;
}   

bool lowsapp_send_idle_check(void)
{
	return lowsappCb.txReady;
}

