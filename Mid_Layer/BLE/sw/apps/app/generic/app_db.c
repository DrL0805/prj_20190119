/*************************************************************************************************/
/*!
 *  \file   app_db.c
 *
 *  \brief  Application framework device database example, using simple RAM-based storage.
 *
 *          $Date: 2017-03-21 16:17:43 -0500 (Tue, 21 Mar 2017) $
 *          $Revision: 11622 $
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
#include "platform_common.h"
#include "platform_debugcof.h"
#include "platform_feature.h"

#include "wsf_types.h"
#include "wsf_assert.h"
#include "bda.h"
#include "app_api.h"
#include "app_main.h"
#include "app_db.h"
#include "app_cfg.h"

#include "mid_extflash.h"   //for BLE passkey
#include "mid_front.h"
#include "flash_task.h"

#if(BLE_PASSKEY_DEBUG == 1)
#define Db_Debug(x) SEGGER_RTT_printf x
#else
#define Db_Debug(x)
#endif
/**************************************************************************************************
  Data Types
**************************************************************************************************/

/*! Database record */
typedef struct
{
  /*! Common for all roles */
  bdAddr_t    peerAddr;                     /*! Peer address */
  uint8_t     addrType;                     /*! Peer address type */
  dmSecIrk_t  peerIrk;                      /*! Peer IRK */
  dmSecCsrk_t peerCsrk;                     /*! Peer CSRK */
  uint8_t     keyValidMask;                 /*! Valid keys in this record */
  bool_t      inUse;                        /*! TRUE if record in use */
  bool_t      valid;                        /*! TRUE if record is valid */
  bool_t      peerAddedToRl;                /*! TRUE if peer device's been added to resolving list */
  bool_t      peerRpao;                     /*! TRUE if RPA Only attribute's present on peer device */

  /*! For slave local device */
  dmSecLtk_t  localLtk;                     /*! Local LTK */
  uint8_t     localLtkSecLevel;             /*! Local LTK security level */
  bool_t      peerAddrRes;                  /*! TRUE if address resolution's supported on peer device (master) */

  /*! For master local device */
  dmSecLtk_t  peerLtk;                      /*! Peer LTK */
  uint8_t     peerLtkSecLevel;              /*! Peer LTK security level */

  /*! for ATT server local device */
  uint16_t    cccTbl[APP_DB_NUM_CCCD];      /*! Client characteristic configuration descriptors */
  uint32_t    peerSignCounter;              /*! Peer Sign Counter */

  /*! for ATT client */
  uint16_t    hdlList[APP_DB_HDL_LIST_LEN]; /*! Cached handle list */
  uint8_t     discStatus;                   /*! Service discovery and configuration status */
} appDbRec_t;

/*! Database type */
typedef struct
{
  appDbRec_t  rec[APP_DB_NUM_RECS];               /*! Device database records */
  char        devName[ATT_DEFAULT_PAYLOAD_LEN];   /*! Device name */
  uint8_t     devNameLen;                         /*! Device name length */
} appDb_t;

/**************************************************************************************************
  Local Variables
**************************************************************************************************/

/*! Database */
static appDb_t appDb;

/*! When all records are allocated use this index to determine which to overwrite */
static appDbRec_t *pAppDbNewRec = appDb.rec;

typedef struct 
{
    uint32_t u32KeyNum;   //记录有多少个设备与此设备配对
    appDbRec_t appdb[APP_DB_NUM_RECS];
}Ble_PassKey;

static Ble_PassKey passkey;

#define	BLE_PAIRING_PASSKEY_ADDR       0x9000

//清除配对信息，Ram和flash中都要清除
uint8_t AppClearRecList(void)
{
//	flash_task_msg_t		flashMsg;

    //step 1:判断是否有配对信息
    if((*(uint32_t*)(passkey.appdb[0].peerAddr) == 0xFFFFFFFF) && (*(uint32_t*)(passkey.appdb[1].peerAddr) == 0xFFFFFFFF) && \
       (*(uint32_t*)(passkey.appdb[2].peerAddr) == 0xFFFFFFFF))
    {
        return TRUE;
    }

    //step 2:擦除flash中保存的判断信息
//	flashMsg.id 	= EXTFLASH_ID;
//	flashMsg.flash.extflashEvent.para.startAddr 	= BLE_PAIRING_PASSKEY_ADDR;
//	flashMsg.flash.extflashEvent.para.dataAddr 		= (uint8*)(NULL);
//	flashMsg.flash.extflashEvent.para.length 		= 4096;		
//	flashMsg.flash.extflashEvent.para.endAddr 		= flashMsg.flash.extflashEvent.para.startAddr + flashMsg.flash.extflashEvent.para.length - 1;
//	flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_4K_ERASE;	
//	flashMsg.flash.extflashEvent.para.result		= FlashTask_EventSet(&flashMsg);

    //step 3: 清除ram中配对信息
    memset(appDb.rec,0x00,sizeof(appDbRec_t));
    return TRUE;
}

// Copy Record list from NVM into the active record list, if any
void AppCopyRecListInNvm(appDbRec_t *pRecord)
{
//    extflash_para_t extflash;
    uint8_t i; 

    //step 1: Get passkey
//    extflash.dataAddr = (uint8*)&passkey;
//    extflash.length = sizeof(Ble_PassKey);
//    extflash.result =0;
//    extflash.Cb = NULL;
//    Mid_ExtFlash_ReadBleInfo(EXTFLASH_EVENT_READ_BLE_PASSKEY,&extflash);

    //step 2: Copy Record list from NVM into the active record list, if any
    for(i=0;i<APP_DB_NUM_RECS;i++)
    {
        if(((*(uint32_t*)(passkey.appdb[i].peerAddr) != 0xFFFFFFFF))&&(*(uint32_t*)(passkey.appdb[i].peerAddr) != 0x00000000))
        {
            //valid record in NVM
            memcpy(pRecord, &passkey.appdb[i], sizeof(appDbRec_t));
            
            //pRecord->inUse = FALSE;
            //pRecord->valid = TRUE;
            pRecord++;
        }
        else
        {
            break;  //break the for loop
        }
    }

    //step 3: get passkey nums
    if((passkey.u32KeyNum == 0xffffffff) || (i == 0))
        passkey.u32KeyNum = 0;

//    Db_Debug((0,"passkey.u32KeyNum: %d,result=%d\n",passkey.u32KeyNum,extflash.result));
}

int32_t AppSaveNewRecInNvm(uint8_t handle, appDbHdl_t hdl)
{
//    flash_task_msg_t flashMsg;
    uint8_t u8ident = 0; //记录record信息是否已存在
    uint8_t i;  

    //step 1: 判断新的信息在flash中是否存在
    for(i = 0; i < APP_DB_NUM_RECS; i++)
    {
        if((*(uint32_t*)(passkey.appdb[i].peerAddr) != 0xFFFFFFFF))
        {
            if(BdaCmp(((appDbRec_t*)hdl)->peerAddr, passkey.appdb[i].peerAddr))
            {
                Db_Debug((0,"NVM save: record is identical, update the record.\n"));
                u8ident = 1;
                break;
            }
            //Db_Debug((0,"NVM save: target flash not empty, check next record.\n"));
        }
        else
        {
            Db_Debug((0,"NVM save: target flash is empty\n"));
            break;
        }
    }

    //step 2: 用新的配对信息更新flash中数据
    //case 1: 如果此连接的设备已存在，则更新数据
    //case 2: 与此设备配对的个数小于3，则直接更新数据
    //case 3: 与此设备配对的个数小于3，则覆盖最先配对的那个
    if(u8ident == 0)
    {
        //如果record信息已存在，只需update数据;否则记录u32KeyNum和record都要update
        passkey.u32KeyNum++; 
    }
    if(i >= APP_DB_NUM_RECS)
    {
        i = (passkey.u32KeyNum - 1) % 3;
    }
    Db_Debug((0,"update %d id target flash info\n",i));
    memcpy((uint8_t *)&passkey.appdb[i], (uint8_t *)hdl, sizeof(appDbRec_t));

    //step 3:更新的数据写到flash中
//	flashMsg.id 									= EXTFLASH_ID;
//	flashMsg.flash.extflashEvent.para.dataAddr 		= (uint8*)&passkey;
//	flashMsg.flash.extflashEvent.para.length 		= sizeof(Ble_PassKey);
//    flashMsg.flash.extflashEvent.para.result        =0;
//	flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_WRITE_BLE_PASSKEY;
//	FlashTask_EventSet(&flashMsg);

    return 0;
}

/*************************************************************************************************/
/*!
 *  \fn     AppDbInit()
 *        
 *  \brief  Initialize the device database.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppDbInit(void)
{
    //passkey pairing
    AppCopyRecListInNvm(pAppDbNewRec);
    return;
}

/*************************************************************************************************/
/*!
 *  \fn     AppDbNewRecord
 *        
 *  \brief  Create a new device database record.
 *
 *  \param  addrType  Address type.
 *  \param  pAddr     Peer device address.
 *
 *  \return Database record handle.
 */
/*************************************************************************************************/
appDbHdl_t AppDbNewRecord(uint8_t addrType, uint8_t *pAddr)
{
  appDbRec_t  *pRec = appDb.rec;
  uint8_t     i;
  
  /* find a free record */
  for (i = APP_DB_NUM_RECS; i > 0; i--, pRec++)
  {
    if (!pRec->inUse)
    {
      break;
    }
  }
  
  /* if all records were allocated */
  if (i == 0)
  {
    /* overwrite a record */
    pRec = pAppDbNewRec;
    
    /* get next record to overwrite */
    pAppDbNewRec++;
    if (pAppDbNewRec == &appDb.rec[APP_DB_NUM_RECS])
    {
      pAppDbNewRec = appDb.rec;
    }
  }
  
  /* initialize record */
  memset(pRec, 0, sizeof(appDbRec_t));
  pRec->inUse = TRUE;
  pRec->addrType = addrType;
  BdaCpy(pRec->peerAddr, pAddr);
  pRec->peerAddedToRl = FALSE;
  pRec->peerRpao = FALSE;

  return (appDbHdl_t) pRec;
}

/*************************************************************************************************/
/*!
*  \fn     AppDbGetNextRecord
*
*  \brief  Get the next database record for a given record. For the first record, the function
*          should be called with 'hdl' set to 'APP_DB_HDL_NONE'.
*
*  \param  hdl  Database record handle.
*
*  \return Next record handle found. APP_DB_HDL_NONE, otherwise.
*/
/*************************************************************************************************/
appDbHdl_t AppDbGetNextRecord(appDbHdl_t hdl)
{
  appDbRec_t  *pRec;

  /* if first record is requested */
  if (hdl == APP_DB_HDL_NONE)
  {
    pRec = appDb.rec;
  }
  /* if valid record passed in */
  else if (AppDbRecordInUse(hdl))
  {
    pRec = (appDbRec_t *)hdl;
    pRec++;
  }
  /* invalid record passed in */
  else
  {
    return APP_DB_HDL_NONE;
  }

  /* look for next valid record */
  while (pRec < &appDb.rec[APP_DB_NUM_RECS])
  {
    /* if record is in use */
    if (pRec->inUse && pRec->valid)
    {
      /* record found */
      return (appDbHdl_t)pRec;
    }

    /* look for next record */
    pRec++;
  }

  /* end of records */
  return APP_DB_HDL_NONE;
}

/*************************************************************************************************/
/*!
 *  \fn     AppDbDeleteRecord
 *        
 *  \brief  Delete a new device database record.
 *
 *  \param  hdl       Database record handle.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppDbDeleteRecord(appDbHdl_t hdl)
{
  ((appDbRec_t *) hdl)->inUse = FALSE;
}

/*************************************************************************************************/
/*!
 *  \fn     AppDbValidateRecord
 *        
 *  \brief  Validate a new device database record.  This function is called when pairing is
 *          successful and the devices are bonded.
 *
 *  \param  hdl       Database record handle.
 *  \param  keyMask   Bitmask of keys to validate.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppDbValidateRecord(appDbHdl_t hdl, uint8_t keyMask)
{
  ((appDbRec_t *) hdl)->valid = TRUE;
  ((appDbRec_t *) hdl)->keyValidMask = keyMask;
}

/*************************************************************************************************/
/*!
 *  \fn     AppDbCheckValidRecord
 *        
 *  \brief  Check if a record has been validated.  If it has not, delete it.  This function
 *          is typically called when the connection is closed.
 *
 *  \param  hdl       Database record handle.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppDbCheckValidRecord(appDbHdl_t hdl)
{
  if (((appDbRec_t *) hdl)->valid == FALSE)
  {
    AppDbDeleteRecord(hdl);
  }
}

/*************************************************************************************************/
/*!
*  \fn     AppDbRecordInUse
*
*  \brief  Check if a database record is in use.

*  \param  hdl       Database record handle.
*
*  \return TURE if record in use. FALSE, otherwise.
*/
/*************************************************************************************************/
bool_t AppDbRecordInUse(appDbHdl_t hdl)
{
  appDbRec_t  *pRec = appDb.rec;
  uint8_t     i;

  /* see if record is in database record list */
  for (i = APP_DB_NUM_RECS; i > 0; i--, pRec++)
  {
    if (pRec->inUse && pRec->valid && (pRec == ((appDbRec_t *)hdl)))
    {
      return TRUE;
    }
  }

  return FALSE;
}

/*************************************************************************************************/
/*!
 *  \fn     AppDbCheckBonded
 *        
 *  \brief  Check if there is a stored bond with any device.
 *
 *  \param  hdl       Database record handle.
 *
 *  \return TRUE if a bonded device is found, FALSE otherwise.
 */
/*************************************************************************************************/
bool_t AppDbCheckBonded(void)
{
  appDbRec_t  *pRec = appDb.rec;
  uint8_t     i;
  
  /* find a record */
  for (i = APP_DB_NUM_RECS; i > 0; i--, pRec++)
  {
    if (pRec->inUse)
    {
      return TRUE;
    }
  }
  
  return FALSE;
}

/*************************************************************************************************/
/*!
 *  \fn     AppDbDeleteAllRecords
 *        
 *  \brief  Delete all database records.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppDbDeleteAllRecords(void)
{
  appDbRec_t  *pRec = appDb.rec;
  uint8_t     i;

  /* set in use to false for all records */
  for (i = APP_DB_NUM_RECS; i > 0; i--, pRec++)
  {
    pRec->inUse = FALSE;
  }  
}

/*************************************************************************************************/
/*!
 *  \fn     AppDbFindByAddr
 *        
 *  \brief  Find a device database record by peer address.
 *
 *  \param  addrType  Address type.
 *  \param  pAddr     Peer device address.
 *
 *  \return Database record handle or APP_DB_HDL_NONE if not found.
 */
/*************************************************************************************************/
appDbHdl_t AppDbFindByAddr(uint8_t addrType, uint8_t *pAddr)
{
  appDbRec_t  *pRec = appDb.rec;
  uint8_t     peerAddrType = DmHostAddrType(addrType);
  uint8_t     i;
  
  /* find matching record */
  for (i = APP_DB_NUM_RECS; i > 0; i--, pRec++)
  {
    if (pRec->inUse && (pRec->addrType == peerAddrType) && BdaCmp(pRec->peerAddr, pAddr))
    {
      return (appDbHdl_t) pRec;
    }
  }
  
  return APP_DB_HDL_NONE;
}

/*************************************************************************************************/
/*!
 *  \fn     AppDbFindByLtkReq
 *        
 *  \brief  Find a device database record by data in an LTK request.
 *
 *  \param  encDiversifier  Encryption diversifier associated with key.
 *  \param  pRandNum        Pointer to random number associated with key.
 *
 *  \return Database record handle or APP_DB_HDL_NONE if not found.
 */
/*************************************************************************************************/
appDbHdl_t AppDbFindByLtkReq(uint16_t encDiversifier, uint8_t *pRandNum)
{
  appDbRec_t  *pRec = appDb.rec;
  uint8_t     i;

#if(BLE_PASSKEY_DEBUG == 1)
  uint16  j;

  Db_Debug((0,"AppDbFindByLtkReq encDiversifier: =%d\n",encDiversifier));
  Db_Debug((0,"pRandNum:\n"));
  for(j = 0; j < SMP_RAND8_LEN; j++)
  {
      Db_Debug((0,"0x%x,",pRandNum[j]));
  }
  Db_Debug((0,"\n"));
#endif

  /* find matching record */
  for (i = APP_DB_NUM_RECS; i > 0; i--, pRec++)
  {
#if(BLE_PASSKEY_DEBUG == 1)
    Db_Debug((0,"pRec inUse=%d,encDiversifier: =%d\n",pRec->inUse,pRec->localLtk.ediv));
    Db_Debug((0,"pRec pRandNum:\n"));
    for(j = 0; j < SMP_RAND8_LEN; j++)
    {
        Db_Debug((0,"0x%x,",pRec->localLtk.rand[j]));
    }
    Db_Debug((0,"\n"));
#endif

    if (pRec->inUse && (pRec->localLtk.ediv == encDiversifier) &&
        (memcmp(pRec->localLtk.rand, pRandNum, SMP_RAND8_LEN) == 0))
    {
      return (appDbHdl_t) pRec;
    }
  }
  
  return APP_DB_HDL_NONE;
}

/*************************************************************************************************/
/*!
 *  \fn     AppDbGetKey
 *        
 *  \brief  Get a key from a device database record.
 *
 *  \param  hdl       Database record handle.
 *  \param  type      Type of key to get.
 *  \param  pSecLevel If the key is valid, the security level of the key.
 *
 *  \return Pointer to key if key is valid or NULL if not valid.
 */
/*************************************************************************************************/
dmSecKey_t *AppDbGetKey(appDbHdl_t hdl, uint8_t type, uint8_t *pSecLevel)
{
  dmSecKey_t *pKey = NULL;
  
  /* if key valid */
  if ((type & ((appDbRec_t *) hdl)->keyValidMask) != 0)
  {
    switch(type)
    {
      case DM_KEY_LOCAL_LTK:
        *pSecLevel = ((appDbRec_t *) hdl)->localLtkSecLevel;
        pKey = (dmSecKey_t *) &((appDbRec_t *) hdl)->localLtk;
        break;

      case DM_KEY_PEER_LTK:
        *pSecLevel = ((appDbRec_t *) hdl)->peerLtkSecLevel;
        pKey = (dmSecKey_t *) &((appDbRec_t *) hdl)->peerLtk;
        break;

      case DM_KEY_IRK:
        pKey = (dmSecKey_t *)&((appDbRec_t *)hdl)->peerIrk;
        break;

      case DM_KEY_CSRK:
        pKey = (dmSecKey_t *)&((appDbRec_t *)hdl)->peerCsrk;
        break;
        
      default:
        break;
    }
  }
  
  return pKey;
}

/*************************************************************************************************/
/*!
 *  \fn     AppDbSetKey
 *        
 *  \brief  Set a key in a device database record.
 *
 *  \param  hdl       Database record handle.
 *  \param  pKey      Key data.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppDbSetKey(appDbHdl_t hdl, dmSecKeyIndEvt_t *pKey)
{
  switch(pKey->type)
  {
    case DM_KEY_LOCAL_LTK:
      ((appDbRec_t *) hdl)->localLtkSecLevel = pKey->secLevel;
      ((appDbRec_t *) hdl)->localLtk = pKey->keyData.ltk;
      break;

    case DM_KEY_PEER_LTK:
      ((appDbRec_t *) hdl)->peerLtkSecLevel = pKey->secLevel;
      ((appDbRec_t *) hdl)->peerLtk = pKey->keyData.ltk;
      break;

    case DM_KEY_IRK:
      ((appDbRec_t *)hdl)->peerIrk = pKey->keyData.irk;

      /* make sure peer record is stored using its identity address */
      ((appDbRec_t *)hdl)->addrType = pKey->keyData.irk.addrType;
      BdaCpy(((appDbRec_t *)hdl)->peerAddr, pKey->keyData.irk.bdAddr);
      break;

    case DM_KEY_CSRK:
      ((appDbRec_t *)hdl)->peerCsrk = pKey->keyData.csrk;

      /* sign counter must be initialized to zero when CSRK is generated */
      ((appDbRec_t *)hdl)->peerSignCounter = 0;
      break;
      
    default:
      break;
  }
}

/*************************************************************************************************/
/*!
 *  \fn     AppDbGetCccTbl
 *        
 *  \brief  Get the client characteristic configuration descriptor table.
 *
 *  \param  hdl       Database record handle.
 *
 *  \return Pointer to client characteristic configuration descriptor table.
 */
/*************************************************************************************************/
uint16_t *AppDbGetCccTbl(appDbHdl_t hdl)
{
  return ((appDbRec_t *) hdl)->cccTbl;
}

/*************************************************************************************************/
/*!
 *  \fn     AppDbSetCccTblValue
 *        
 *  \brief  Set a value in the client characteristic configuration table.
 *
 *  \param  hdl       Database record handle.
 *  \param  idx       Table index.
 *  \param  value     client characteristic configuration value.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppDbSetCccTblValue(appDbHdl_t hdl, uint16_t idx, uint16_t value)
{
  WSF_ASSERT(idx < APP_DB_NUM_CCCD);
  
  ((appDbRec_t *) hdl)->cccTbl[idx] = value;
}

/*************************************************************************************************/
/*!
 *  \fn     AppDbGetDiscStatus
 *        
 *  \brief  Get the discovery status.
 *
 *  \param  hdl       Database record handle.
 *
 *  \return Discovery status.
 */
/*************************************************************************************************/
uint8_t AppDbGetDiscStatus(appDbHdl_t hdl)
{
  return ((appDbRec_t *) hdl)->discStatus;
}

/*************************************************************************************************/
/*!
 *  \fn     AppDbSetDiscStatus
 *        
 *  \brief  Set the discovery status.
 *
 *  \param  hdl       Database record handle.
 *  \param  state     Discovery status.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppDbSetDiscStatus(appDbHdl_t hdl, uint8_t status)
{
  ((appDbRec_t *) hdl)->discStatus = status;
}

/*************************************************************************************************/
/*!
 *  \fn     AppDbGetHdlList
 *        
 *  \brief  Get the cached handle list.
 *
 *  \param  hdl       Database record handle.
 *
 *  \return Pointer to handle list.
 */
/*************************************************************************************************/
uint16_t *AppDbGetHdlList(appDbHdl_t hdl)
{
  return ((appDbRec_t *) hdl)->hdlList;
}

/*************************************************************************************************/
/*!
 *  \fn     AppDbSetHdlList
 *        
 *  \brief  Set the cached handle list.
 *
 *  \param  hdl       Database record handle.
 *  \param  pHdlList  Pointer to handle list.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppDbSetHdlList(appDbHdl_t hdl, uint16_t *pHdlList)
{
  memcpy(((appDbRec_t *) hdl)->hdlList, pHdlList, sizeof(((appDbRec_t *) hdl)->hdlList));
}

/*************************************************************************************************/
/*!
 *  \fn     AppDbGetDevName
 *        
 *  \brief  Get the device name.
 *
 *  \param  pLen      Returned device name length.
 *
 *  \return Pointer to UTF-8 string containing device name or NULL if not set.
 */
/*************************************************************************************************/
char *AppDbGetDevName(uint8_t *pLen)
{
  /* if first character of name is NULL assume it is uninitialized */
  if (appDb.devName[0] == 0)
  {
    *pLen = 0;
    return NULL;
  }
  else
  {
    *pLen = appDb.devNameLen;
    return appDb.devName;
  }
}

/*************************************************************************************************/
/*!
 *  \fn     AppDbSetDevName
 *        
 *  \brief  Set the device name.
 *
 *  \param  len       Device name length.
 *  \param  pStr      UTF-8 string containing device name.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppDbSetDevName(uint8_t len, char *pStr)
{
  /* check for maximum device length */
  len = (len <= sizeof(appDb.devName)) ? len : sizeof(appDb.devName);
  
  memcpy(appDb.devName, pStr, len);
}

/*************************************************************************************************/
/*!
 *  \fn     AppDbGetPeerAddrRes
 *
 *  \brief  Get address resolution attribute value read from a peer device.
 *
 *  \param  hdl        Database record handle.
 *
 *  \return TRUE if address resolution is supported in peer device. FALSE, otherwise.
 */
/*************************************************************************************************/
bool_t AppDbGetPeerAddrRes(appDbHdl_t hdl)
{
  return ((appDbRec_t *)hdl)->peerAddrRes;
}

/*************************************************************************************************/
/*!
 *  \fn     AppDbSetPeerAddrRes
 *
 *  \brief  Set address resolution attribute value for a peer device.
 *
 *  \param  hdl        Database record handle.
 *  \param  addrRes    Address resolution attribue value.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppDbSetPeerAddrRes(appDbHdl_t hdl, uint8_t addrRes)
{
  ((appDbRec_t *)hdl)->peerAddrRes = addrRes;
}

/*************************************************************************************************/
/*!
 *  \fn     AppDbGetPeerSignCounter
 *
 *  \brief  Get sign counter for a peer device.
 *
 *  \param  hdl        Database record handle.
 *
 *  \return Sign counter for peer device.
 */
/*************************************************************************************************/
uint32_t AppDbGetPeerSignCounter(appDbHdl_t hdl)
{
  return ((appDbRec_t *)hdl)->peerSignCounter;
}

/*************************************************************************************************/
/*!
 *  \fn     AppDbSetPeerSignCounter
 *
 *  \brief  Set sign counter for a peer device.
 *
 *  \param  hdl          Database record handle.
 *  \param  signCounter  Sign counter for peer device.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppDbSetPeerSignCounter(appDbHdl_t hdl, uint32_t signCounter)
{
  ((appDbRec_t *)hdl)->peerSignCounter = signCounter;
}

/*************************************************************************************************/
/*!
 *  \fn     AppDbGetPeerAddedToRl
 *
 *  \brief  Get the peer device added to resolving list flag value.
 *
 *  \param  hdl        Database record handle.
 *
 *  \return TRUE if peer device's been added to resolving list. FALSE, otherwise.
 */
/*************************************************************************************************/
bool_t AppDbGetPeerAddedToRl(appDbHdl_t hdl)
{
  return ((appDbRec_t *)hdl)->peerAddedToRl;
}

/*************************************************************************************************/
/*!
 *  \fn     AppDbSetPeerAddedToRl
 *
 *  \brief  Set the peer device added to resolving list flag to a given value.
 *
 *  \param  hdl           Database record handle.
 *  \param  peerAddedToRl Peer device added to resolving list flag value.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppDbSetPeerAddedToRl(appDbHdl_t hdl, bool_t peerAddedToRl)
{
  ((appDbRec_t *)hdl)->peerAddedToRl = peerAddedToRl;
}

/*************************************************************************************************/
/*!
 *  \fn     AppDbGetPeerRpao
 *
 *  \brief  Get the resolvable private address only attribute flag for a given peer device.
 *
 *  \param  hdl        Database record handle.
 *
 *  \return TRUE if RPA Only attribute is present on peer device. FALSE, otherwise.
 */
/*************************************************************************************************/
bool_t AppDbGetPeerRpao(appDbHdl_t hdl)
{
  return ((appDbRec_t *)hdl)->peerRpao;
}

/*************************************************************************************************/
/*!
 *  \fn     AppDbSetPeerRpao
 *
 *  \brief  Set the resolvable private address only attribute flag for a given peer device.
 *
 *  \param  hdl        Database record handle.
 *  \param  peerRpao   Resolvable private address only attribute flag value.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppDbSetPeerRpao(appDbHdl_t hdl, bool_t peerRpao)
{
  ((appDbRec_t *)hdl)->peerRpao = peerRpao;
}
