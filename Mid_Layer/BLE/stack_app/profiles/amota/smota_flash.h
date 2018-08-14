#ifndef __SMOTA_FLASE_H
#define __SMOTA_FLASE_H

//*****************************************************************************
//
//! @brief CRC-32 implementation allowing multiple partial images.
//!
//! @param pvData - Pointer to the data to be checked.
//! @param ui32NumBytes - Number of bytes to check.
//! @param pui32CRC - Location to store the partial CRC32 result.
//!
//! This function performs a CRC-32 on the input data and returns the 32-bit
//! result. This version uses a 256-entry lookup table to speed up the
//! computation of the result. The result of the CRC32 is stored in the
//! location given by the caller. This allows the caller to keep a "running"
//! CRC for individual chunks of an image.
//!
//! @return 32-bit CRC value.
//
//*****************************************************************************
extern void SMOTA_Partial_CRC32(const void *pvData, uint32 ui32NumBytes,uint32 *pui32CRC);

//*****************************************************************************
//
//! @brief Get the information about the main image
//!
//! @param pui32LinkAddr - Used to return the link address for main image
//! @param pui32Length - Used to return the length of the image
//!
//! This function is used to determine attributes of the main image currently
//! in flash
//!
//! @return false if there is no valid flag page
//
//*****************************************************************************
extern uint8 SMOTA_GetMainImageInfo(uint32 *pui32LinkAddr, uint32 *pui32Length);

//**********************************************************************
// 函数功能: 读取flash pSrc地址起ui32Length区域的数据到ui32DestAddr
// 输入参数：ui32WriteAddr:数据
//           pSrc:读取flash的位置
// 返回参数: 
//**********************************************************************
extern int32 SMOTA_Flash_ReadPage(uint32_t ui32DestAddr, uint32_t *pSrc, uint32_t ui32Length);

//**********************************************************************
// 函数功能: 将pSrc数据写到flash ui32DestAddr为首地址的page区域
// 输入参数：ui32WriteAddr:要写的page区域起始地址
//           pSrc/ui32Length:要写的数据，及数据大小
// 返回参数: 
// 说    明:该接口写flash最小单位是一个page
//**********************************************************************
extern int32 SMOTA_Flash_WritePage(uint32_t ui32DestAddr, uint32_t *pSrc, uint32_t ui32Length);

//**********************************************************************
// 函数功能: 擦除flash一块page区域
// 输入参数：ui32Addr:要擦除的page区域起始地址 
// 返回参数: 
// 说    明:该接口将擦除整个page内容
//**********************************************************************
extern int32 SMOTA_Flash_ErasePage(uint32_t ui32DestAddr);
#endif

