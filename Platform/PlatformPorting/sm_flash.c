/**********************************************************************
**
**ģ��˵��: �Խ�MCU �ڲ�Flash�����ӿ�
**����汾���޸���־(ʱ�䣬����),�޸���:
**   V1.0   2018.5.19  ����  ZSL  
**
**********************************************************************/
#include "platform_debugcof.h"
#include "sm_flash.h"
#include "am_mcu_apollo.h"

#if(SMDRV_FLASH_DEBUG == 1)
#define Flash_Debug(x) SEGGER_RTT_printf x
#else
#define Flash_Debug(x)
#endif

//**********************************************************************
// ��������: init Flash
// ��������� 
// ���ز���: flash size
//**********************************************************************
uint32 SMDrv_Flash_Init(void)
{
    return 0;
}

//**********************************************************************
// ��������: deinit Flash
// ��������� 
// ���ز���: flash size
//**********************************************************************
uint32 SMDrv_Flash_DeInit(void)
{
    return 0;
}

//**********************************************************************
// ��������: enable Flash
// ��������� 
// ���ز���: flash size
//**********************************************************************
uint32 SMDrv_Flash_Enable(void)
{
    return 0;
}

//**********************************************************************
// ��������: Disable Flash
// ��������� 
// ���ز���: 
//**********************************************************************
uint32 SMDrv_Flash_Disable(void)
{
    return 0;
}

//**********************************************************************
// ��������: ��ȡinternal Flash�ܴ�С
// ��������� 
// ���ز���: flash size
//**********************************************************************
uint32 SMDrv_Flash_GetTotalSize(void)
{
    am_hal_mcuctrl_device_t sDevice;
    
#if AM_APOLLO3_MCUCTRL
    am_hal_mcuctrl_info_get(AM_HAL_MCUCTRL_INFO_DEVICEID, &sDevice);
#else
    am_hal_mcuctrl_device_info_get(&sDevice);
#endif
    Flash_Debug((0,"Internal Flash Total Size=0x%x\n",sDevice.ui32FlashSize));
    return sDevice.ui32FlashSize;
}

//**********************************************************************
// ��������: ��ȡinternal Flash Page size
// ��������� 
// ���ز���: flash Page size
//**********************************************************************
uint32 SMDrv_Flash_GetPageSize(void)
{
    return AM_HAL_FLASH_PAGE_SIZE;
}

//**********************************************************************
// ��������: ��ȡinternal Flash Sector size
// ��������� 
// ���ز���: flash Sector size
//**********************************************************************
uint32 SMDrv_Flash_GetSectorSize(void)
{
    return AM_HAL_FLASH_PAGE_SIZE;
}

//**********************************************************************
// ��������: ����flashһ��page����
// ���������ui32Addr:Ҫ������page������ʼ��ַ 
// ���ز���: 
// ˵    ��:�ýӿڽ���������page����
//**********************************************************************
void SMDrv_Flash_ErasePage(uint32 ui32Addr)
{
    uint32 ui32Critical;
    uint32 ui32CurrentPage, ui32CurrentBlock;

    // Figure out what page and block we're working on.
    ui32CurrentPage =  AM_HAL_FLASH_ADDR2PAGE(ui32Addr);
    ui32CurrentBlock = AM_HAL_FLASH_ADDR2INST(ui32Addr);

    // Start a critical section.
    ui32Critical = am_hal_interrupt_master_disable();
    am_hal_flash_page_erase(AM_HAL_FLASH_PROGRAM_KEY,ui32CurrentBlock, ui32CurrentPage);

    // Exit the critical section.
    am_hal_interrupt_master_set(ui32Critical);
}

//**********************************************************************
// ��������: ����flashһ��sector����
// ���������ui32Addr:Ҫ������sector������ʼ��ַ 
// ���ز���: 
// ˵    ��:�ýӿڽ���������sector����
//**********************************************************************
void SMDrv_Flash_EraseSector(uint32 ui32Addr)
{
    uint32 ui32Critical;
    uint32 ui32CurrentPage, ui32CurrentBlock;

    // Figure out what page and block we're working on.
    ui32CurrentPage =  AM_HAL_FLASH_ADDR2PAGE(ui32Addr);
    ui32CurrentBlock = AM_HAL_FLASH_ADDR2INST(ui32Addr);

    // Start a critical section.
    ui32Critical = am_hal_interrupt_master_disable();
    am_hal_flash_page_erase(AM_HAL_FLASH_PROGRAM_KEY,ui32CurrentBlock, ui32CurrentPage);

    // Exit the critical section.
    am_hal_interrupt_master_set(ui32Critical);
}

//**********************************************************************
// ��������: ��pu32Buffer����д��flash ui32WriteAddrΪ�׵�ַ�� page����
// ���������ui32WriteAddr:Ҫд��page������ʼ��ַ
//           pu32Buffer/numBytes:Ҫд�����ݣ������ݴ�С
// ���ز���: 
// ˵    ��:�ýӿ�дflash��С��λ��һ��page
//**********************************************************************
void SMDrv_Flash_ProgramPage(uint32 ui32WriteAddr,uint32 *pu32Buffer, uint32 numBytes)
{
    uint32 ui32Critical;
    uint32 ui32WordsInBuffer;

    SMDrv_Flash_ErasePage(ui32WriteAddr);

    ui32WordsInBuffer = (numBytes + 3) / 4;
    // Start a critical section.
    ui32Critical = am_hal_interrupt_master_disable();

    // Program the flash page with the data we just received.
    am_hal_flash_program_main(AM_HAL_FLASH_PROGRAM_KEY, pu32Buffer,(uint32 *)ui32WriteAddr, ui32WordsInBuffer);

    // Exit the critical section.
    am_hal_interrupt_master_set(ui32Critical);
}

#ifdef AM_PART_APOLLO3
//**********************************************************************
// ��������: ����һ��image����OTA
// ���������imageMagic:image magic
//           pImage:ָ��image
// ���ز���: 
//**********************************************************************
void SMDrv_OTA_Add(uint8 imageMagic, uint32 *pImage)
{
    // Set OTAPOINTER
    am_hal_ota_add(AM_HAL_FLASH_PROGRAM_KEY, imageMagic, pImage);
}

//**********************************************************************
// ��������: Initialize OTA state
// ���������pOtaDesc:should be start of a flash page designated for OTA Descriptor
// ���ز���: 
//**********************************************************************
void SMDrv_OTA_Init(uint32_t *pOtaDesc)
{
    // Initialize OTA descriptor - This should ideally be initiated through a separate command
    // to facilitate multiple image upgrade in a single reboot
    // Will need change in the AMOTA app to do so
    am_hal_ota_init(AM_HAL_FLASH_PROGRAM_KEY, pOtaDesc);
}
#endif

