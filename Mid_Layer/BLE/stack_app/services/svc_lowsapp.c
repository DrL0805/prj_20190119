/*************************************************************************************************/
/*!
 *  \file   svc_hrs.c
 *        
 *  \brief  Example Heart Rate service implementation.
 *
 *          $Date: 2016-12-28 16:12:14 -0600 (Wed, 28 Dec 2016) $
 *          $Revision: 10805 $
 *  
 *  Copyright (c) 2011-2017 ARM Ltd., all rights reserved.
 *  ARM Ltd. confidential and proprietary.
 *
 *  IMPORTANT.  Your use of this file is governed by a Software License Agreement
 *  ("Agreement") that must be accepted in order to download or otherwise receive a
 *  copy of this file.  You may not use or copy this file for any purpose other than
 *  as described in the Agreement.  If you do not agree to all of the terms of the
 *  Agreement do not use this file and delete all copies in your possession or control;
 *  if you do not have a copy of the Agreement, you must contact ARM Ltd. prior
 *  to any use, copying or further distribution of this software.
 */
/*************************************************************************************************/

#include "wsf_types.h"
#include "att_api.h"
#include "wsf_trace.h"
#include "bstream.h"
#include "svc_ch.h"
#include "svc_lowsapp.h"
#include "svc_cfg.h"

/**************************************************************************************************
  Macros
**************************************************************************************************/


/**************************************************************************************************
 Static Variables
**************************************************************************************************/
#define BLE_UUID_LOWSPP_SERVICE             0xA800
#define BLE_UUID_LOWSPP_CH1_CHARACTERISTIC  0xA801                      /**< The UUID of the CH1 Characteristic. */
#define BLE_UUID_LOWSPP_CH2_CHARACTERISTIC  0xA802                      /**< The UUID of the CH2 Characteristic. */
#define BLE_UUID_LOWSPP_CH3_CHARACTERISTIC  0xA803                      /**< The UUID of the CH2 Characteristic. */

#define CH1_MTU_SIZE   (20)
#define CH2_MTU_SIZE   (20)
#define CH3_MTU_SIZE   (20)

/* UUIDs */
static const uint8_t Ch1Uuid[] = {UINT16_TO_BYTES(BLE_UUID_LOWSPP_CH1_CHARACTERISTIC)};
static const uint8_t Ch2Uuid[] = {UINT16_TO_BYTES(BLE_UUID_LOWSPP_CH2_CHARACTERISTIC)};
static const uint8_t Ch3Uuid[] = {UINT16_TO_BYTES(BLE_UUID_LOWSPP_CH3_CHARACTERISTIC)};


/**************************************************************************************************
 Service variables
**************************************************************************************************/

/* AMDTP service declaration */
static const uint8_t lowsapp_service[] = {UINT16_TO_BYTES(BLE_UUID_LOWSPP_SERVICE)};
static const uint16_t lowsapp_service_len = sizeof(lowsapp_service);

/* AMDTP RX characteristic */ 
static const uint8_t lowsapp_ch1_ch[] = {(ATT_PROP_WRITE|ATT_PROP_WRITE_NO_RSP|ATT_PROP_NOTIFY), UINT16_TO_BYTES(LOWSAPP_CH1_HDL), UINT16_TO_BYTES(BLE_UUID_LOWSPP_CH1_CHARACTERISTIC)};
static const uint16_t lowsapp_ch1_ch_len = sizeof(lowsapp_ch1_ch);

/* AMDTP TX characteristic */ 
static const uint8_t lowsapp_ch2_ch[] = {(ATT_PROP_NOTIFY), UINT16_TO_BYTES(LOWSAPP_CH2_HDL), UINT16_TO_BYTES(BLE_UUID_LOWSPP_CH2_CHARACTERISTIC)};
static const uint16_t lowsapp_ch2_ch_len = sizeof(lowsapp_ch2_ch);

/* AMDTP RX ack characteristic */ 
static const uint8_t lowsapp_ch3_ch[] = {(ATT_PROP_WRITE|ATT_PROP_WRITE_NO_RSP|ATT_PROP_NOTIFY), UINT16_TO_BYTES(LOWSAPP_CH3_HDL), UINT16_TO_BYTES(BLE_UUID_LOWSPP_CH3_CHARACTERISTIC)};
static const uint16_t lowsapp_ch3_ch_len = sizeof(lowsapp_ch3_ch);

/* AMDTP RX data */
/* Note these are dummy values */
static const uint8_t lowsapp_ch1_val[CH1_MTU_SIZE] = {0};
static const uint16_t ch1_value_len = sizeof(lowsapp_ch1_val);

/* AMDTP TX data */
/* Note these are dummy values */
static const uint8_t lowsapp_ch2_val[CH2_MTU_SIZE] = {0};
static const uint16_t ch2_value_len = sizeof(lowsapp_ch2_val);

/* AMDTP RX ack data */
/* Note these are dummy values */
static const uint8_t lowsapp_ch3_val[CH3_MTU_SIZE] = {0};
static const uint16_t ch3_value_len = sizeof(lowsapp_ch3_val);

/* Proprietary data client characteristic configuration */
static uint8_t lowsapp_ch1_ccc[] = {UINT16_TO_BYTES(0x0000)};
static const uint16_t lowsapp_ch1_ccc_len = sizeof(lowsapp_ch1_ccc);

/* Proprietary data client characteristic configuration */
static uint8_t lowsapp_ch2_ccc[] = {UINT16_TO_BYTES(0x0000)};
static const uint16_t lowsapp_ch2_ccc_len = sizeof(lowsapp_ch2_ccc);

/* Proprietary data client characteristic configuration */
static uint8_t lowsapp_ch3_ccc[] = {UINT16_TO_BYTES(0x0000)};
static const uint16_t lowsapp_ch3_ccc_len = sizeof(lowsapp_ch3_ccc);


/* Attribute list for AMDTP group */
static const attsAttr_t lowsappList[] =
{
  {
    attPrimSvcUuid, 
    (uint8_t *) lowsapp_service,
    (uint16_t *) &lowsapp_service_len, 
    sizeof(lowsapp_service),
    0,
    ATTS_PERMIT_READ
  },
  
  {
    attChUuid,
    (uint8_t *) lowsapp_ch1_ch,
    (uint16_t *) &lowsapp_ch1_ch_len,
    sizeof(lowsapp_ch1_ch),
    0,
    ATTS_PERMIT_READ
  },
  {
    Ch1Uuid,
    (uint8_t *) lowsapp_ch1_val,
    (uint16_t *) &ch1_value_len,
    ATT_VALUE_MAX_LEN,
    ( ATTS_SET_VARIABLE_LEN | ATTS_SET_WRITE_CBACK),
    ATTS_PERMIT_WRITE
  },
  {
    attCliChCfgUuid,
    (uint8_t *) lowsapp_ch1_ccc,
    (uint16_t *) &lowsapp_ch1_ccc_len,
    sizeof(lowsapp_ch1_ccc),
    ATTS_SET_CCC,
    (ATTS_PERMIT_READ | ATTS_PERMIT_WRITE)
  },
  
  {
    attChUuid,
    (uint8_t *) lowsapp_ch2_ch,
    (uint16_t *) &lowsapp_ch2_ch_len,
    sizeof(lowsapp_ch1_ch),
    0,
    ATTS_PERMIT_READ
  },
  {
    Ch2Uuid,
    (uint8_t *) lowsapp_ch2_val,
    (uint16_t *) &ch2_value_len,
    ATT_VALUE_MAX_LEN,
    (ATTS_SET_VARIABLE_LEN | ATTS_SET_WRITE_CBACK),
    ATTS_PERMIT_WRITE
  },
  {
    attCliChCfgUuid,
    (uint8_t *) lowsapp_ch2_ccc,
    (uint16_t *) &lowsapp_ch2_ccc_len,
    sizeof(lowsapp_ch1_ccc),
    ATTS_SET_CCC,
    (ATTS_PERMIT_READ | ATTS_PERMIT_WRITE)
  },
  
  {
    attChUuid,
    (uint8_t *) lowsapp_ch3_ch,
    (uint16_t *) &lowsapp_ch3_ch_len,
    sizeof(lowsapp_ch3_ch),
    0,
    ATTS_PERMIT_READ
  },
  {
    Ch3Uuid,
    (uint8_t *) lowsapp_ch3_val,
    (uint16_t *) &ch3_value_len,
    ATT_VALUE_MAX_LEN,
    (ATTS_SET_VARIABLE_LEN | ATTS_SET_WRITE_CBACK),
    ATTS_PERMIT_WRITE,
  },
  {
    attCliChCfgUuid,
    (uint8_t *) lowsapp_ch3_ccc,
    (uint16_t *) &lowsapp_ch3_ccc_len,
    sizeof(lowsapp_ch3_ccc),
    ATTS_SET_CCC,
    (ATTS_PERMIT_READ | ATTS_PERMIT_WRITE)
  }
};

/* HRS group structure */
static attsGroup_t svcLowsappGroup =
{
  NULL,
  (attsAttr_t *) lowsappList,
  NULL,
  NULL,
  LOWSAPP_START_HDL,
  LOWSAPP_END_HDL
};

/*************************************************************************************************/
/*!
 *  \fn     SvcHrsAddGroup
 *        
 *  \brief  Add the services to the attribute server.
 *
 *  \return None.
 */
/*************************************************************************************************/
void SvcLowsappAddGroup(void)
{
  AttsAddGroup(&svcLowsappGroup);
}

/*************************************************************************************************/
/*!
 *  \fn     SvcHrsRemoveGroup
 *        
 *  \brief  Remove the services from the attribute server.
 *
 *  \return None.
 */
/*************************************************************************************************/
void SvcLowsappRemoveGroup(void)
{
  AttsRemoveGroup(LOWSAPP_START_HDL);
}

/*************************************************************************************************/
/*!
 *  \fn     SvcHrsCbackRegister
 *        
 *  \brief  Register callbacks for the service.
 *
 *  \param  readCback   Read callback function.
 *  \param  writeCback  Write callback function.
 *
 *  \return None.
 */
/*************************************************************************************************/
void SvcLowsappCbackRegister(attsReadCback_t readCback, attsWriteCback_t writeCback)
{
  svcLowsappGroup.readCback = readCback;
  svcLowsappGroup.writeCback = writeCback;
}
