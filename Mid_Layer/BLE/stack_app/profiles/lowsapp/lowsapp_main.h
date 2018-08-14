
#ifndef LOWSAPP_API_H
#define LOWSAPP_API_H

#include "wsf_os.h"
#include "stdint.h"
#include "stdbool.h"
#include "att_api.h"
#ifdef __cplusplus
extern "C" {
#endif
    


typedef void (*m_ble_profile_recv_data_handler_t)(const uint8_t *buf,uint16_t len);
    
typedef struct
{
    dmConnId_t    connId;               // Connection ID
    bool_t        lowsappToSend;        // AMOTA notify ready to be sent on this channel
}
lowsappConn_t;

/*! Application message type */
typedef union
{
    wsfMsgHdr_t     hdr;
    dmEvt_t         dm;
    attsCccEvt_t    ccc;
    attEvt_t        att;
} lowsappMsg_t;
    
    
typedef enum
{
    SEND_CHANNEL_FIRST          = 0U,
    SEND_CHANNEL_CMD,
    SEND_CHANNEL_DATA,
    SEND_CHANNEL_LAST,
}send_channel_t;


void lowsapp_start(dmConnId_t connId,uint8_t lowsappCccIdx);
void lowsapp_stop(dmConnId_t connId,uint8_t lowsappCccIdx);

//static void m_ble_lowsapp_init(void);
//static void amdtps_conn_close(dmEvt_t *pMsg);


//void lowsappProcCccState(lowsappMsg_t *pMsg);
//void lowsapp_handle_value_cnf(attEvt_t *pMsg);
void lowsapp_proc_msg(wsfMsgHdr_t *pMsg);

//void lowsapp_conn_close(dmConnId_t connId);
void m_ble_profile_send_data(uint8_t *buf, uint8_t len,uint8_t type);
void lowsapp_init(wsfHandlerId_t handlerId);

void lowsapp_send_data(uint8_t *buf, uint8_t len,uint8_t type);
uint8_t lowsapp_write_cback(dmConnId_t connId, uint16_t handle, uint8_t operation,
                       uint16_t offset, uint16_t len, uint8_t *pValue, attsAttr_t *pAttr);

void lowsapp_transfer_data_init(m_ble_profile_recv_data_handler_t recv_data_handler);
bool lowsapp_send_idle_check(void);    

#ifdef __cplusplus
};
#endif

#endif /* ANCS_API_H */

