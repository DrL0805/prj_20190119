/**********************************************************************
**
**模块说明: 对接MCU SPI驱动接口，并根据项目原理图作配置
**软件版本，修改日志(时间，内容),修改人:
**   V1.0   2018.4.13  初版  ZSL 
**   V2.0   2018.8.8   增加Apollo3 driver  ZSL  
**
**********************************************************************/
//在此模块中使用SPI IO口，因此需定义SPI_MODULE
#define SPI_MODULE
#ifdef AM_PART_APOLLO3
#define FONT_MODULE
#define FLASH_MODULE
#endif

#include "am_mcu_apollo.h"
#include "sm_gpio.h"
#include "sm_spi.h"
#include "io_config.h"

//定义SPI口
enum
{
    SPI0=0,
    SPI1,
    SPI2,
    SPI3,
    SPI4,
    SPI5,
    SPI_MAX,
};

typedef struct
{
    uint32 u32spi_id;    //spi number
    uint32 u32pin_sck;   //spi sck pin
    uint32 u32pin_miso;  //spi miso pin
    uint32 u32pin_mosi;  //spi mosi pin

    uint32 u32pad_sck;   //spi sck pad mode,把pin脚设置为spi sck模式
    uint32 u32pad_miso;  //spi miso pad mode
    uint32 u32pad_mosi;  //spi mosi pad mode 

    uint32 u32Interrupt;  //Interrupt for spi
}spip_info_s;

static void spi_isr(uint32 u32module)
{
    uint32 ui32IntStatus;

#ifdef AM_PART_APOLLO3

#else
    // Read and clear the interrupt status.
    ui32IntStatus = am_hal_iom_int_status_get(u32module, false);
    am_hal_iom_int_clear(u32module, ui32IntStatus);

    // Service FIFO interrupts as necessary, and call IOM callbacks as
    // transfers are completed.
    am_hal_iom_int_service(u32module, ui32IntStatus);
#endif
}

//*****************************************************************************
//
// Interrupt handler for IOM0~5 SPI
//
//*****************************************************************************
#if (SPI0_SCK != IO_UNKNOW)
void am_iomaster0_isr(void)
{
    spi_isr(0);
}
#endif
#if (SPI1_SCK != IO_UNKNOW)
void am_iomaster1_isr(void)
{
    spi_isr(1);
}
#endif
#if (SPI2_SCK != IO_UNKNOW)
void am_iomaster2_isr(void)
{
    spi_isr(2);
}
#endif
#if (SPI3_SCK != IO_UNKNOW)
void am_iomaster3_isr(void)
{
    spi_isr(3);
}
#endif
#if (SPI4_SCK != IO_UNKNOW)
void am_iomaster4_isr(void)
{
    spi_isr(4);
}
#endif
#if (SPI5_SCK != IO_UNKNOW)
void am_iomaster5_isr(void)
{
    spi_isr(5);
}
#endif

#ifdef AM_PART_APOLLO3
static void *gSpi_Handle[SPI_MAX] = {NULL};

static uint8 spi_modul2info(spi_module modul,spip_info_s *pspi_info,am_hal_iom_config_t *pspi_att)
{
    if(pspi_info == NULL)
        return FALSE;

    if(pspi_att != NULL)
    {
        //set spi param
        pspi_att->eInterfaceMode = AM_HAL_IOM_SPI_MODE;    
        pspi_att->ui32ClockFreq = AM_HAL_IOM_8MHZ;

        //AM_HAL_IOM_SPI_MODE_0,     // CPOL = 0; CPHA = 0
        //AM_HAL_IOM_SPI_MODE_2,     // CPOL = 1; CPHA = 0
        //AM_HAL_IOM_SPI_MODE_1,     // CPOL = 0; CPHA = 1
        //AM_HAL_IOM_SPI_MODE_3,     // CPOL   = 1; CPHA = 1
        pspi_att->eSpiMode= AM_HAL_IOM_SPI_MODE_3;

        //如下两个配置:设置nonblocking非阻塞buffer，buffer是32位的,
        //使用阻塞方式设置为NULL
        pspi_att->pNBTxnBuf=NULL;
        pspi_att->ui32NBTxnBufLength = 0;
    }

    switch(modul)
    {
    case FLASH_SPI_MODULE:  //FLASH使用spi0
    case FONT_SPI_MODULE:
        pspi_info->u32spi_id = SPI0;
        pspi_info->u32pin_sck = SPI0_SCK;
        pspi_info->u32pin_miso = SPI0_MISO;
        pspi_info->u32pin_mosi = SPI0_MOSI;

        pspi_info->u32pad_sck = AM_HAL_PIN_5_M0SCK;
        pspi_info->u32pad_miso = AM_HAL_PIN_6_M0MISO;
        pspi_info->u32pad_mosi = AM_HAL_PIN_7_M0MOSI;
        //pspi_info->u32Interrupt = AM_HAL_INTERRUPT_IOMASTER0;

        //spi配置若与默认不同，需根据实际情况进行配置
        if(pspi_att != NULL)
        {
        }
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

static uint8 spi_modul2Cs(spi_module modul,am_hal_gpio_pincfg_t *pCs_config,uint32 *pu32CsPin)
{
    if(pCs_config == NULL)
        return FALSE;

    pCs_config->eDriveStrength      = AM_HAL_GPIO_PIN_DRIVESTRENGTH_12MA;
    pCs_config->eGPOutcfg           = AM_HAL_GPIO_PIN_OUTCFG_PUSHPULL;
    pCs_config->eGPInput            = AM_HAL_GPIO_PIN_INPUT_NONE;
    pCs_config->eIntDir             = AM_HAL_GPIO_PIN_INTDIR_LO2HI;
    pCs_config->eCEpol              = AM_HAL_GPIO_PIN_CEPOL_ACTIVELOW;
    switch(modul)
    {
    case FLASH_SPI_MODULE:
        if(pu32CsPin != NULL)
        {
            *pu32CsPin              = FLASH_CS_PIN;
        }
        pCs_config->uFuncSel        = AM_HAL_PIN_37_NCE37;
        pCs_config->uIOMnum         = 0;
        pCs_config->uNCE            = 2;
        break;
    case FONT_SPI_MODULE:
        if(pu32CsPin != NULL)
        {
            *pu32CsPin               = FONT_CS_PIN;
        }
        pCs_config->uFuncSel         = AM_HAL_PIN_28_NCE28;
        pCs_config->uIOMnum          = 0;
        pCs_config->uNCE             = 0;
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

void spi_config_pin(spi_module modul,spip_info_s *pspi_info)
{
    am_hal_gpio_pincfg_t gPin_config,gCs_config;
    uint32_t u32CsPin;

    //open SPI flow:
    //step 1:config spi io pin as spi mode
    gPin_config.uFuncSel = pspi_info->u32pad_miso;    //MISO
    gPin_config.uIOMnum = pspi_info->u32spi_id;
    am_hal_gpio_pinconfig(pspi_info->u32pin_miso, gPin_config);

    gPin_config.uFuncSel = pspi_info->u32pad_mosi;    //MOSI
    gPin_config.eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_12MA;
    am_hal_gpio_pinconfig(pspi_info->u32pin_mosi,gPin_config);

    gPin_config.uFuncSel = pspi_info->u32pad_sck;      //SCK
    am_hal_gpio_pinconfig(pspi_info->u32pin_sck, gPin_config);

    spi_modul2Cs(modul,&gCs_config,&u32CsPin);       //CS
    am_hal_gpio_pinconfig(u32CsPin, gPin_config);
}

void spi_disable_pin(spi_module modul,spip_info_s *pspi_info,am_hal_iom_config_t *pSPI_Config)
{
    am_hal_gpio_pincfg_t pin_config,gCs_config;
    uint32 u32CsPin;

    pin_config.uFuncSel       = 3;
    pin_config.eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_2MA;
    pin_config.eGPOutcfg      = AM_HAL_GPIO_PIN_OUTCFG_PUSHPULL;
    am_hal_gpio_pinconfig(pspi_info->u32pin_sck, pin_config);

    pin_config.eGPOutcfg      = AM_HAL_GPIO_PIN_OUTCFG_DISABLE;   //MISO,MOSI set as disable
    am_hal_gpio_pinconfig(pspi_info->u32pin_mosi,pin_config);
    am_hal_gpio_pinconfig(pspi_info->u32pin_miso,pin_config);

    spi_modul2Cs(modul,&gCs_config,&u32CsPin);       //CS
    am_hal_gpio_pinconfig(u32CsPin, pin_config);

    if((pSPI_Config->eSpiMode == AM_HAL_IOM_SPI_MODE_2) || (pSPI_Config->eSpiMode == AM_HAL_IOM_SPI_MODE_3))  //空闲的时候为high
    {
        SMDrv_GPIO_BitSet(pspi_info->u32pin_sck);
    }
    else   //空闲的时候为low
    {
        SMDrv_GPIO_BitClear(pspi_info->u32pin_sck);    
    }
}

//**********************************************************************
// 函数功能: 初始化SPI
// 输入参数：	
// 返回参数：
//**********************************************************************
void SMDrv_SPI_Init(void)
{
}

//**********************************************************************
// 函数功能: 根据driver module ID打开硬件对应的SPI
// 输入参数：	
//    modul: driver module ID,值参考spi_module
// 返回参数：Ret_InvalidParam或Ret_OK
//**********************************************************************
ret_type SMDrv_SPI_Open(spi_module modul)
{
    spip_info_s spi_info;
    am_hal_iom_config_t SPI_Config;

    if(spi_modul2info(modul,&spi_info,&SPI_Config) == FALSE)
        return Ret_InvalidParam;

    //open SPI flow:
    //step 1:config spi io pin as spi mode
    spi_config_pin(modul,&spi_info);

    //step 3:init/enable spi
    //Initialize the IOM instance.
    if(am_hal_iom_initialize(spi_info.u32spi_id, &gSpi_Handle[spi_info.u32spi_id]) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - init spi failed.\n"));

    // Enable power to the IOM instance.
    if(am_hal_iom_power_ctrl(gSpi_Handle[spi_info.u32spi_id], AM_HAL_SYSCTRL_WAKE, false) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - spi power ctrl failed.\n"));

    // Configure the IOM for Serial operation during initialization.
    if(am_hal_iom_configure(gSpi_Handle[spi_info.u32spi_id], &SPI_Config) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - config spi failed.\n"));

    // Enable the IOM.
    if((am_hal_iom_enable(gSpi_Handle[spi_info.u32spi_id]) != AM_HAL_STATUS_SUCCESS) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - enable spi failed.\n"));
    return Ret_OK;
}

//**********************************************************************
// 函数功能: 关闭driver module ID硬件对应的SPI,以实现低功耗
// 输入参数:
//    modul: driver module ID,值参考spi_module
// 返回参数：Ret_InvalidParam或Ret_OK
//**********************************************************************
ret_type SMDrv_SPI_Close(spi_module modul)
{
    spip_info_s spi_info;
    am_hal_iom_config_t SPI_Config;

    if(spi_modul2info(modul,&spi_info,&SPI_Config) == FALSE)
        return Ret_InvalidParam;

    //step 1:disable spi power control
    if(am_hal_iom_disable(gSpi_Handle[spi_info.u32spi_id]) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - disable spi failed.\n"));
    if(am_hal_iom_power_ctrl(gSpi_Handle[spi_info.u32spi_id], AM_HAL_SYSCTRL_DEEPSLEEP, false) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - spi power ctrl failed.\n"));
    if(am_hal_iom_uninitialize(gSpi_Handle[spi_info.u32spi_id]) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - spi deinit failed.\n"));

    //step 2:config spi io pin as gpio mode
    spi_disable_pin(modul,&spi_info,&SPI_Config);

    return Ret_OK;
}

//**********************************************************************
// 函数功能: 唤醒driver module ID打开硬件对应的SPI
// 输入参数：	
//    modul: driver module ID,值参考spi_module
// 返回参数：Ret_InvalidParam或Ret_OK
//**********************************************************************
ret_type SMDrv_SPI_Wake(spi_module modul)
{
    spip_info_s spi_info;

    if(spi_modul2info(modul,&spi_info,NULL) == FALSE)
        return Ret_InvalidParam;

    //wake SPI flow:
    //step 1:config spi io pin as spi mode
    spi_config_pin(modul,&spi_info);

    //step 2:Enable power to the selected IOM module
    if(am_hal_iom_power_ctrl(gSpi_Handle[spi_info.u32spi_id], AM_HAL_SYSCTRL_WAKE, false) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - spi power ctrl failed.\n"));
 
    //step 3:enable spi
    if(am_hal_iom_enable(gSpi_Handle[spi_info.u32spi_id]) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - enable spi failed.\n"));

    return Ret_OK;
}

//**********************************************************************
// 函数功能: 将driver module ID硬件对应的SPI进入sleep模式
// 输入参数:
//    modul: driver module ID,值参考spi_module
// 返回参数：Ret_InvalidParam或Ret_OK
//**********************************************************************
ret_type SMDrv_SPI_Sleep(spi_module modul)
{
    spip_info_s spi_info;
    am_hal_iom_config_t SPI_Config;

    if(spi_modul2info(modul,&spi_info,&SPI_Config) == FALSE)
        return Ret_InvalidParam;

    //step 1:disable spi power control
    if(am_hal_iom_disable(gSpi_Handle[spi_info.u32spi_id]) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - disable spi failed.\n"));
    if(am_hal_iom_power_ctrl(gSpi_Handle[spi_info.u32spi_id], AM_HAL_SYSCTRL_DEEPSLEEP, false) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - spi power ctrl failed.\n"));

    //step 2:config spi io pin as gpio mode
    spi_disable_pin(modul,&spi_info,&SPI_Config);
    return Ret_OK;
}

//**********************************************************************
// 函数功能: 根据driver module获取spi module ID
// 输入参数：	
//    modul: driver module ID,值参考spi_module
// 返回参数：spi Module ID
//**********************************************************************
uint32 SMDrv_SPI_GetModuleId(spi_module modul)
{
    spip_info_s spi_info;

    spi_modul2info(modul,&spi_info,NULL);
    return spi_info.u32spi_id;
}

//**********************************************************************
// 函数功能: 获取SPI资源，如果多个driver模块使用同一个SPI时需要加锁保护
// 输入参数：	
//    modul: driver module ID,值参考spi_module
//    u32timeout:等待超时时间
// 返回参数：
//**********************************************************************
ret_type SMDrv_SPI_LockMutex(spi_module modul,uint32 u32timeout)
{
    return Ret_OK;
}

//**********************************************************************
// 函数功能: 释放SPI资源
// 输入参数：	
//    modul: driver module ID,值参考spi_module
// 返回参数：
//**********************************************************************
ret_type SMDrv_SPI_UnLockMutex(spi_module modul)
{
    return Ret_OK;
}

//**********************************************************************
// 函数功能: 向driver module ID对应的SPI写1 个字节命令字cmd
// 输入参数：	
//    modul: driver module ID,值参考spi_module
// 返回参数：
//**********************************************************************
void SMDrv_SPI_WriteByte(spi_module modul,uint8 u8Cmd)
{
    spip_info_s spi_info;
    am_hal_gpio_pincfg_t cs_config;
    am_hal_iom_transfer_t Transaction;

    if(spi_modul2info(modul,&spi_info,NULL) == FALSE)
        return ;

    //for CS
    spi_modul2Cs(modul,&cs_config,NULL);

    // Create the transaction.
    Transaction.eDirection      = AM_HAL_IOM_TX;
    Transaction.ui32Instr       = u8Cmd;
    Transaction.ui32InstrLen    = 1;
    Transaction.ui32NumBytes    = 0;
    Transaction.pui32TxBuffer   = NULL;
    Transaction.uPeerInfo.ui32SpiChipSelect = cs_config.uNCE;
    Transaction.bContinue       = false;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    // Execute the transction over IOM.
    if (am_hal_iom_blocking_transfer(gSpi_Handle[spi_info.u32spi_id], &Transaction))
        Err_Info((0,"Error - spi Write CMD failed.\n"));
}

//**********************************************************************
// 函数功能: 向driver module ID对应的SPI写多个字节
// 输入参数：	
//    modul: driver module ID,值参考spi_module
// 返回参数：
//**********************************************************************
void SMDrv_SPI_WriteBytes(spi_module modul,uint8 *pDataBuffer, uint16 length)
{
    spip_info_s spi_info;
    am_hal_gpio_pincfg_t cs_config;
    am_hal_iom_transfer_t Transaction;

    if(spi_modul2info(modul,&spi_info,NULL) == FALSE)
        return ;

    //for CS
    spi_modul2Cs(modul,&cs_config,NULL);

    // Create the transaction.
    Transaction.eDirection      = AM_HAL_IOM_TX;
    Transaction.ui32Instr       = 0;
    Transaction.ui32InstrLen    = 0;
    Transaction.ui32NumBytes    = length;
    Transaction.pui32TxBuffer   = (uint32_t *)pDataBuffer;
    Transaction.uPeerInfo.ui32SpiChipSelect = cs_config.uNCE;
    Transaction.bContinue       = false;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    // Execute the transction over IOM.
    if (am_hal_iom_blocking_transfer(gSpi_Handle[spi_info.u32spi_id], &Transaction))
        Err_Info((0,"Error - spi Write CMD failed.\n"));
}

//**********************************************************************
// 函数功能: 向driver module ID对应的SPI读多个字节
// 输入参数：	
//    modul: driver module ID,值参考spi_module
// 返回参数：
//**********************************************************************
void SMDrv_SPI_ReadBytes(spi_module modul,uint8* pDataBuffer, uint16 length)
{
    spip_info_s spi_info;
    am_hal_gpio_pincfg_t cs_config;
    am_hal_iom_transfer_t Transaction;

    if(spi_modul2info(modul,&spi_info,NULL) == FALSE)
        return ;

    //for CS
    spi_modul2Cs(modul,&cs_config,NULL);

    // Create the transaction.
    Transaction.eDirection      = AM_HAL_IOM_RX;
    Transaction.ui32Instr       = 0;
    Transaction.ui32InstrLen    = 0;
    Transaction.ui32NumBytes    = length;
    Transaction.pui32TxBuffer   = (uint32_t *)pDataBuffer;
    Transaction.uPeerInfo.ui32SpiChipSelect = cs_config.uNCE;
    Transaction.bContinue       = false;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    // Execute the transction over IOM.
    if (am_hal_iom_blocking_transfer(gSpi_Handle[spi_info.u32spi_id], &Transaction))
        Err_Info((0,"Error - spi Write CMD failed.\n"));
}

#else

static uint8 spi_modul2info(spi_module modul,spip_info_s *pspi_info,am_hal_iom_config_t *pspi_att)
{
    if(pspi_info == NULL)
        return FALSE;

    if(pspi_att != NULL)
    {
        //set spi param
        pspi_att->ui32InterfaceMode = AM_HAL_IOM_SPIMODE;    
        pspi_att->ui32ClockFrequency = AM_HAL_IOM_8MHZ;    
        pspi_att->ui8WriteThreshold=4;
        pspi_att->ui8ReadThreshold=60;
        pspi_att->bSPHA = 1;   //相位
        pspi_att->bSPOL = 1;   //极性
    }

    switch(modul)
    {
    case FLASH_SPI_MODULE:  //FLASH使用spi0
        pspi_info->u32spi_id = SPI0;
        pspi_info->u32pin_sck = SPI0_SCK;
        pspi_info->u32pin_miso = SPI0_MISO;
        pspi_info->u32pin_mosi = SPI0_MOSI;

        pspi_info->u32pad_sck = AM_HAL_PIN_5_M0SCK;
        pspi_info->u32pad_miso = AM_HAL_PIN_6_M0MISO;
        pspi_info->u32pad_mosi = AM_HAL_PIN_7_M0MOSI;
        pspi_info->u32Interrupt = AM_HAL_INTERRUPT_IOMASTER0;
        
        if(pspi_att != NULL)
        {
        }
        break;
    case FONT_SPI_MODULE:     //字库使用spi1 
        pspi_info->u32spi_id = SPI1;
        pspi_info->u32pin_sck = SPI1_SCK;
        pspi_info->u32pin_miso = SPI1_MISO;
        pspi_info->u32pin_mosi = SPI1_MOSI;

        pspi_info->u32pad_sck = AM_HAL_PIN_8_M1SCK;
        pspi_info->u32pad_miso = AM_HAL_PIN_9_M1MISO;
        pspi_info->u32pad_mosi = AM_HAL_PIN_10_M1MOSI;
        pspi_info->u32Interrupt = AM_HAL_INTERRUPT_IOMASTER1;
        
        if(pspi_att != NULL)
        {
        }
        break;
        #if 1
    case BLE_SPI_MODULE:  //BLE使用SPI5
        pspi_info->u32spi_id = SPI5;
        pspi_info->u32pin_sck = SPI5_SCK;
        pspi_info->u32pin_miso = SPI5_MISO;
        pspi_info->u32pin_mosi = SPI5_MOSI;

        pspi_info->u32pad_sck = AM_HAL_PIN_48_M5SCK;
        pspi_info->u32pad_miso = AM_HAL_PIN_49_M5MISO;
        pspi_info->u32pad_mosi = AM_HAL_PIN_47_M5MOSI;
        pspi_info->u32Interrupt = AM_HAL_INTERRUPT_IOMASTER5;

        if(pspi_att != NULL)
        {
            pspi_att->bSPHA = 0;
            pspi_att->bSPOL = 0;
            pspi_att->ui8ReadThreshold = 20;
            pspi_att->ui8WriteThreshold = 20;
        }
        break;
        #else
        case BLE_SPI_MODULE:  //BLE使用SPI3
            pspi_info->u32spi_id = SPI3;
            pspi_info->u32pin_sck = SPI3_SCK;
            pspi_info->u32pin_miso = SPI3_MISO;
            pspi_info->u32pin_mosi = SPI3_MOSI;
        
            pspi_info->u32pad_sck = AM_HAL_PIN_42_M3SCK;
            pspi_info->u32pad_miso = AM_HAL_PIN_43_M3MISO;
            pspi_info->u32pad_mosi = AM_HAL_PIN_38_M3MOSI;
            pspi_info->u32Interrupt = AM_HAL_INTERRUPT_IOMASTER3;
        
            if(pspi_att != NULL)
            {
                pspi_att->bSPHA = 0;
                pspi_att->bSPOL = 0;
                pspi_att->ui8ReadThreshold = 20;
                pspi_att->ui8WriteThreshold = 20;
            }
            break;

        #endif
    default:
        return FALSE;
    }
    return TRUE;
}

//**********************************************************************
// 函数功能: 初始化SPI
// 输入参数：	
// 返回参数：
//**********************************************************************
void SMDrv_SPI_Init(void)
{
}

//**********************************************************************
// 函数功能: 根据driver module ID打开硬件对应的SPI
// 输入参数：	
//    modul: driver module ID,值参考spi_module
// 返回参数：Ret_InvalidParam或Ret_OK
//**********************************************************************
ret_type SMDrv_SPI_Open(spi_module modul)
{
    spip_info_s spi_info;
    am_hal_iom_config_t SPI_Config;

    if(spi_modul2info(modul,&spi_info,&SPI_Config) == FALSE)
        return Ret_InvalidParam;

    //open SPI flow:
    //step 1:enable spi power control and set iom as spi mode
    am_hal_iom_pwrctrl_enable(spi_info.u32spi_id);
    am_hal_iom_config(spi_info.u32spi_id, &SPI_Config);

    //step 2:config spi io pin as spi mode
    SMDrv_GPIO_SetIOPad(spi_info.u32pin_sck, spi_info.u32pad_sck | AM_HAL_GPIO_HIGH_DRIVE);
    SMDrv_GPIO_SetIOPad(spi_info.u32pin_miso,spi_info.u32pad_miso);
    SMDrv_GPIO_SetIOPad(spi_info.u32pin_mosi,spi_info.u32pad_mosi);
    //step 3:enable spi
    am_hal_iom_enable(spi_info.u32spi_id);

    if(modul == BLE_SPI_MODULE)  //BLE使用中断
    {
        // Enable interrupts.
        am_hal_iom_int_clear(spi_info.u32spi_id, AM_HAL_IOM_INT_CMDCMP | AM_HAL_IOM_INT_THR);
        am_hal_iom_int_enable(spi_info.u32spi_id, AM_HAL_IOM_INT_CMDCMP | AM_HAL_IOM_INT_THR);
        am_hal_interrupt_enable(spi_info.u32Interrupt);
    }
    return Ret_OK;
}

//**********************************************************************
// 函数功能: 关闭driver module ID硬件对应的SPI,以实现低功耗
// 输入参数:
//    modul: driver module ID,值参考spi_module
// 返回参数：Ret_InvalidParam或Ret_OK
//**********************************************************************
ret_type SMDrv_SPI_Close(spi_module modul)
{
    spip_info_s spi_info;
    am_hal_iom_config_t SPI_Config;

    if(spi_modul2info(modul,&spi_info,&SPI_Config) == FALSE)
        return Ret_InvalidParam;

    //step 1:disable spi power control
    am_hal_iom_disable(spi_info.u32spi_id);
    am_hal_iom_pwrctrl_disable(spi_info.u32spi_id);

    //step 2:config spi io pin as gpio mode
    SMDrv_GPIO_SetIOPad(spi_info.u32pin_sck, AM_HAL_PIN_OUTPUT);
    SMDrv_GPIO_SetIOPad(spi_info.u32pin_miso,AM_HAL_PIN_OUTPUT);
    SMDrv_GPIO_SetIOPad(spi_info.u32pin_mosi,AM_HAL_PIN_OUTPUT);
    if(SPI_Config.bSPOL == 1)  //空闲的时候为high
    {
        SMDrv_GPIO_BitSet(spi_info.u32pin_sck);
        SMDrv_GPIO_BitSet(spi_info.u32pin_miso);
        SMDrv_GPIO_BitSet(spi_info.u32pin_mosi);
    }
    else   //空闲的时候为low
    {
        SMDrv_GPIO_BitClear(spi_info.u32pin_sck);    
        SMDrv_GPIO_BitClear(spi_info.u32pin_miso);
        SMDrv_GPIO_BitClear(spi_info.u32pin_mosi);
    }
        
    return Ret_OK;
}

//**********************************************************************
// 函数功能: 唤醒driver module ID打开硬件对应的SPI
// 输入参数：	
//    modul: driver module ID,值参考spi_module
// 返回参数：Ret_InvalidParam或Ret_OK
//**********************************************************************
ret_type SMDrv_SPI_Wake(spi_module modul)
{
    spip_info_s spi_info;

    if(spi_modul2info(modul,&spi_info,NULL) == FALSE)
        return Ret_InvalidParam;

    //wake SPI flow:
    //step 1:Enable power to the selected IOM module
#if defined(AM_PART_APOLLO2)
     am_hal_iom_power_on_restore(spi_info.u32spi_id);
#endif

    //step 2:config spi io pin as spi mode
    SMDrv_GPIO_SetIOPad(spi_info.u32pin_sck, spi_info.u32pad_sck );
    SMDrv_GPIO_SetIOPad(spi_info.u32pin_miso,spi_info.u32pad_miso);
    SMDrv_GPIO_SetIOPad(spi_info.u32pin_mosi,spi_info.u32pad_mosi);
    //step 3:enable spi
    am_hal_iom_enable(spi_info.u32spi_id);

    return Ret_OK;
}

//**********************************************************************
// 函数功能: 将driver module ID硬件对应的SPI进入sleep模式
// 输入参数:
//    modul: driver module ID,值参考spi_module
// 返回参数：Ret_InvalidParam或Ret_OK
//**********************************************************************
ret_type SMDrv_SPI_Sleep(spi_module modul)
{
    spip_info_s spi_info;
    am_hal_iom_config_t SPI_Config;

    if(spi_modul2info(modul,&spi_info,&SPI_Config) == FALSE)
        return Ret_InvalidParam;

    //step 1:disable spi power control
    am_hal_iom_disable(spi_info.u32spi_id);
#if defined(AM_PART_APOLLO2)
    //Disable power to the selected IOM module
    am_hal_iom_power_off_save(spi_info.u32spi_id);
#endif

    //step 2:config spi io pin as gpio mode
    SMDrv_GPIO_SetIOPad(spi_info.u32pin_sck, AM_HAL_PIN_OUTPUT);
    SMDrv_GPIO_SetIOPad(spi_info.u32pin_miso,AM_HAL_PIN_OUTPUT);
    SMDrv_GPIO_SetIOPad(spi_info.u32pin_mosi,AM_HAL_PIN_OUTPUT);
    if(SPI_Config.bSPOL == 1)  //空闲的时候为high
    {
        SMDrv_GPIO_BitSet(spi_info.u32pin_sck);
        SMDrv_GPIO_BitSet(spi_info.u32pin_miso);
        SMDrv_GPIO_BitSet(spi_info.u32pin_mosi);
    }
    else   //空闲的时候为low
    {
        SMDrv_GPIO_BitClear(spi_info.u32pin_sck);    
        SMDrv_GPIO_BitClear(spi_info.u32pin_miso);
        SMDrv_GPIO_BitClear(spi_info.u32pin_mosi);
    }
    return Ret_OK;
}

//**********************************************************************
// 函数功能: 根据driver module获取spi module ID
// 输入参数：	
//    modul: driver module ID,值参考spi_module
// 返回参数：spi Module ID
//**********************************************************************
uint32 SMDrv_SPI_GetModuleId(spi_module modul)
{
    spip_info_s spi_info;

    spi_modul2info(modul,&spi_info,NULL);
    return spi_info.u32spi_id;
}

//**********************************************************************
// 函数功能: 获取SPI资源，如果多个driver模块使用同一个SPI时需要加锁保护
// 输入参数：	
//    modul: driver module ID,值参考spi_module
//    u32timeout:等待超时时间
// 返回参数：
//**********************************************************************
ret_type SMDrv_SPI_LockMutex(spi_module modul,uint32 u32timeout)
{
    return Ret_OK;
}

//**********************************************************************
// 函数功能: 释放SPI资源
// 输入参数：	
//    modul: driver module ID,值参考spi_module
// 返回参数：
//**********************************************************************
ret_type SMDrv_SPI_UnLockMutex(spi_module modul)
{
    return Ret_OK;
}

//**********************************************************************
// 函数功能: 向driver module ID对应的SPI写1 个字节
// 输入参数：	
//    modul: driver module ID,值参考spi_module
// 返回参数：
//**********************************************************************
void SMDrv_SPI_WriteByte(spi_module modul,uint8 u8ch)
{
    spip_info_s spi_info;

    if(spi_modul2info(modul,&spi_info,NULL) == FALSE)
        return ;

	am_hal_iom_spi_write(spi_info.u32spi_id, 0, (unsigned int*)(&u8ch), 1, AM_HAL_IOM_RAW);
	am_hal_iom_poll_complete(spi_info.u32spi_id);
}

//**********************************************************************
// 函数功能: 向driver module ID对应的SPI写多个字节
// 输入参数：	
//    modul: driver module ID,值参考spi_module
// 返回参数：
//**********************************************************************
void SMDrv_SPI_WriteBytes(spi_module modul,uint8 *pData, uint16 length)
{
    spip_info_s spi_info;

    if(spi_modul2info(modul,&spi_info,NULL) == FALSE)
        return ;

	am_hal_iom_spi_write(spi_info.u32spi_id, 0, (unsigned int*)(pData), length, AM_HAL_IOM_RAW);
	am_hal_iom_poll_complete(spi_info.u32spi_id);
}

//**********************************************************************
// 函数功能: 向driver module ID对应的SPI读多个字节
// 输入参数：	
//    modul: driver module ID,值参考spi_module
// 返回参数：
//**********************************************************************
void SMDrv_SPI_ReadBytes(spi_module modul,uint8* pData, uint16 length)
{
    spip_info_s spi_info;

    if(spi_modul2info(modul,&spi_info,NULL) == FALSE)
        return ;

	am_hal_iom_spi_read(spi_info.u32spi_id, 0, (unsigned int*)(pData), length, AM_HAL_IOM_RAW);
}
#endif

