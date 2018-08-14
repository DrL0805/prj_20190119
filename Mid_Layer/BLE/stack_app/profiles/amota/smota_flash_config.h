#ifndef __SMOTA_FLASE_CONFIG_H
#define __SMOTA_FLASE_CONFIG_H

#define AM_HAL_FLASH_ADDR                   0x00000000
#define AM_HAL_FLASH_PAGE_SIZE              ( 8 * 1024 )
#define AM_HAL_FLASH_INFO_SIZE              AM_HAL_FLASH_PAGE_SIZE
#define AM_HAL_FLASH_INSTANCE_SIZE          ( 512 * 1024 )
#define AM_HAL_FLASH_INSTANCE_PAGES         ( AM_HAL_FLASH_INSTANCE_SIZE / AM_HAL_FLASH_PAGE_SIZE )
#define AM_HAL_FLASH_TOTAL_SIZE             ( AM_HAL_FLASH_INSTANCE_SIZE * 2 )
#define AM_HAL_FLASH_LARGEST_VALID_ADDR     ( AM_HAL_FLASH_ADDR + AM_HAL_FLASH_TOTAL_SIZE - 1 )

//*****************************************************************************
//
// Max Size of Bootloader.
//
//*****************************************************************************
// The value here must match (at least) with the ROLength restriction imposed at
// bootloader linker configuration
#define MAX_BOOTLOADER_SIZE                0x00004000

//*****************************************************************************
//
// Run with flag page.
//
//*****************************************************************************
#define USE_FLAG_PAGE                      1

//*****************************************************************************
//
// Location of the flag page.
//
//*****************************************************************************
#define FLAG_PAGE_LOCATION                 0x00006000

//*****************************************************************************
//
// Location of the new image storage in internal flash.
//
//*****************************************************************************
//
// Note: Internal flash area to be used for OTA temporary storage
// The address must be aligned to flash page
// This should be customized to the desired memory map of the design
//
#define AMOTA_INT_FLASH_OTA_ADDRESS         0x84000

//
// User specified maximum size of OTA storage area.
// Make sure the size is flash page multiple
// (Default value is determined based on rest of flash from the start)
//
#if (USE_LAST_PAGE_FOR_FLAG == 1)
#define AMOTA_INT_FLASH_OTA_MAX_SIZE        (AM_HAL_FLASH_LARGEST_VALID_ADDR - AMOTA_INT_FLASH_OTA_ADDRESS + 1 - AM_HAL_FLASH_PAGE_SIZE)
#else
#define AMOTA_INT_FLASH_OTA_MAX_SIZE        (AM_HAL_FLASH_LARGEST_VALID_ADDR - AMOTA_INT_FLASH_OTA_ADDRESS + 1)
#endif //#if (USE_LAST_PAGE_FOR_FLAG == 1)


// OTA Descriptor address
// For this implementation, we are setting OTA_POINTER in the flag page, following the
// image info to avoid wasting another flash page
// This means that care needs to be taken to preserve the existing contents
// of the page when updating the OTA descriptor
#define OTA_POINTER_LOCATION                (FLAG_PAGE_LOCATION + 256)


#define OTA_INFO_OPTIONS_EXT_FLASH  0x1
#define OTA_INFO_OPTIONS_DATA       0x2
#define OTA_INFO_MAGIC_NUM          0xDEADCAFE

//同am_multiboot_ota_t
typedef struct
{
    // Should be set to OTA_INFO_MAGIC_NUM
    uint32    magicNum;
    // Address in flash where the new image should be programmed
    uint32    *pui32LinkAddress;
    // Length of image blob
    uint32    ui32NumBytes;
    // CRC of the image blob
    uint32    ui32ImageCrc;
    // (Optional) Security Info length
    uint32    secInfoLen;
    // Options - e.g. Read from external flash device
    uint32    ui32Options;
    // (optional) Security Information location
    uint32    *pui32SecInfoPtr;
    // Location of image blob - Address needs to be aligned to 4 Byte address
    uint32    *pui32ImageAddr;
    // CRC to confirm integrity of the OTA Descriptor structure
    uint32    ui32Crc;
}sm_boot_ota_t;

//*****************************************************************************
//
// Structure to keep track of boot image information.
// In the flash, the structure contains a 4 byte CRC for integrity
// verification. It needs to be ensured that the size of this structure is
// not more than AM_HAL_FLASH_PAGE_SIZE bytes
//
//*****************************************************************************
typedef struct
{
    // Starting address where the image was linked to run.
    uint32 *pui32LinkAddress;

    // Length of the executable image in bytes.
    uint32 ui32NumBytes;

    // CRC-32 Value for the full image.
    uint32 ui32CRC;

    // Override GPIO number. (Can be used to force a new image load)
    uint32 ui32OverrideGPIO;

    // Polarity for the override pin.
    uint32 ui32OverridePolarity;

    // Stack pointer location.
    uint32 *pui32StackPointer;

    // Reset vector location.
    uint32 *pui32ResetVector;

    // Protection status of image in flash
    uint32 bEncrypted;

    // CRC-32 value of this structure
    uint32 ui32Checksum;
}sm_bootloader_image_t;

#endif

