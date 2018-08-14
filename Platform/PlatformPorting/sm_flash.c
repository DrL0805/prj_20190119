/**********************************************************************
**
**模块说明: 对接MCU 内部Flash驱动接口
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.5.19  初版  ZSL  
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
// 函数功能: init Flash
// 输入参数： 
// 返回参数: flash size
//**********************************************************************
uint32 SMDrv_Flash_Init(void)
{
    return 0;
}

//**********************************************************************
// 函数功能: deinit Flash
// 输入参数： 
// 返回参数: flash size
//**********************************************************************
uint32 SMDrv_Flash_DeInit(void)
{
    return 0;
}

//**********************************************************************
// 函数功能: enable Flash
// 输入参数： 
// 返回参数: flash size
//**********************************************************************
uint32 SMDrv_Flash_Enable(void)
{
    return 0;
}

//**********************************************************************
// 函数功能: Disable Flash
// 输入参数： 
// 返回参数: 
//**********************************************************************
uint32 SMDrv_Flash_Disable(void)
{
    return 0;
}

//**********************************************************************
// 函数功能: 获取internal Flash总大小
// 输入参数： 
// 返回参数: flash size
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
// 函数功能: 获取internal Flash Page size
// 输入参数： 
// 返回参数: flash Page size
//**********************************************************************
uint32 SMDrv_Flash_GetPageSize(void)
{
    return AM_HAL_FLASH_PAGE_SIZE;
}

//**********************************************************************
// 函数功能: 获取internal Flash Sector size
// 输入参数： 
// 返回参数: flash Sector size
//**********************************************************************
uint32 SMDrv_Flash_GetSectorSize(void)
{
    return AM_HAL_FLASH_PAGE_SIZE;
}

//**********************************************************************
// 函数功能: 擦除flash一块page区域
// 输入参数：ui32Addr:要擦除的page区域起始地址 
// 返回参数: 
// 说    明:该接口将擦除整个page内容
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
// 函数功能: 擦除flash一块sector区域
// 输入参数：ui32Addr:要擦除的sector区域起始地址 
// 返回参数: 
// 说    明:该接口将擦除整个sector内容
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
// 函数功能: 将pu32Buffer数据写到flash ui32WriteAddr为首地址的 page区域
// 输入参数：ui32WriteAddr:要写的page区域起始地址
//           pu32Buffer/numBytes:要写的数据，及数据大小
// 返回参数: 
// 说    明:该接口写flash最小单位是一个page
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
// 函数功能: 增加一个image用来OTA
// 输入参数：imageMagic:image magic
//           pImage:指向image
// 返回参数: 
//**********************************************************************
void SMDrv_OTA_Add(uint8 imageMagic, uint32 *pImage)
{
    // Set OTAPOINTER
    am_hal_ota_add(AM_HAL_FLASH_PROGRAM_KEY, imageMagic, pImage);
}

//**********************************************************************
// 函数功能: Initialize OTA state
// 输入参数：pOtaDesc:should be start of a flash page designated for OTA Descriptor
// 返回参数: 
//**********************************************************************
void SMDrv_OTA_Init(uint32_t *pOtaDesc)
{
    // Initialize OTA descriptor - This should ideally be initiated through a separate command
    // to facilitate multiple image upgrade in a single reboot
    // Will need change in the AMOTA app to do so
    am_hal_ota_init(AM_HAL_FLASH_PROGRAM_KEY, pOtaDesc);
}
#endif

