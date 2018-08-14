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
// ��������: ��ȡflash pSrc��ַ��ui32Length��������ݵ�ui32DestAddr
// ���������ui32WriteAddr:����
//           pSrc:��ȡflash��λ��
// ���ز���: 
//**********************************************************************
extern int32 SMOTA_Flash_ReadPage(uint32_t ui32DestAddr, uint32_t *pSrc, uint32_t ui32Length);

//**********************************************************************
// ��������: ��pSrc����д��flash ui32DestAddrΪ�׵�ַ��page����
// ���������ui32WriteAddr:Ҫд��page������ʼ��ַ
//           pSrc/ui32Length:Ҫд�����ݣ������ݴ�С
// ���ز���: 
// ˵    ��:�ýӿ�дflash��С��λ��һ��page
//**********************************************************************
extern int32 SMOTA_Flash_WritePage(uint32_t ui32DestAddr, uint32_t *pSrc, uint32_t ui32Length);

//**********************************************************************
// ��������: ����flashһ��page����
// ���������ui32Addr:Ҫ������page������ʼ��ַ 
// ���ز���: 
// ˵    ��:�ýӿڽ���������page����
//**********************************************************************
extern int32 SMOTA_Flash_ErasePage(uint32_t ui32DestAddr);
#endif

