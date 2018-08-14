/**********************************************************************
**
**ģ��˵��: �Խ�MCU UART�����ӿڣ���������Ŀԭ��ͼ������
**����汾���޸���־(ʱ�䣬����),�޸���:
**   V1.0   2018.4.14  ����  ZSL  
**
**********************************************************************/
//�ڴ�ģ����ʹ��UART IO�ڣ�����趨��UART_MODULE
#define UART_MODULE
#include "io_config.h"

#if(((UART0_TX != IO_UNKNOW) && (UART0_RX != IO_UNKNOW)) || ((UART1_TX != IO_UNKNOW) && (UART1_RX != IO_UNKNOW)))
#include "am_mcu_apollo.h"
#include "sm_gpio.h"
#include "string.h"
#include "sm_uart.h"

//����UART��
enum
{
    UART_ID0=0,
    UART_ID1,
    UART_MAX,
};

//��¼�ϲ�ע���callback
static uart_cb pUart_CallBack[UART_MAX];

//uart buffer
uint8_t pui8TxBuffer[256];
uint8_t pui8RxBuffer[256];

#ifdef AM_PART_APOLLO3
void *pUart0_Handler = NULL;
void *pUart1_Handler = NULL;

//UART0�жϷ������
void am_uart_isr(void)
{
    uint32_t ui32Status, ui32Idle;

    // Service the FIFOs as necessary, and clear the interrupts.
    am_hal_uart_interrupt_status_get(pUart0_Handler, &ui32Status, true);
    am_hal_uart_interrupt_clear(pUart0_Handler, ui32Status);
    am_hal_uart_interrupt_service(pUart0_Handler, ui32Status, &ui32Idle);

    //UART0���������¼�
    if (AM_HAL_UART_INT_RX & ui32Status)
    {
        //pUart_CallBack[UART0](UART_EVENT_RX);
    }
    //UART0���������¼�
    if (AM_HAL_UART_INT_TX & ui32Status)
    {
        //pUart_CallBack[UART0](UART_EVENT_TX);
    }
    //UART0�������ݳ�ʱ�¼�
    if (AM_HAL_UART_INT_RX_TMOUT & ui32Status)
    {
        //pUart_CallBack[UART0](UART_EVENT_RX_TIMEOUT);
    }

    if(ui32Idle == true)
    {
        ;
    }
}

//UART1�жϷ������
void am_uart1_isr(void)
{
    uint32_t ui32Status, ui32Idle;

    // Service the FIFOs as necessary, and clear the interrupts.
    am_hal_uart_interrupt_status_get(pUart1_Handler, &ui32Status, true);
    am_hal_uart_interrupt_clear(pUart1_Handler, ui32Status);
    am_hal_uart_interrupt_service(pUart1_Handler, ui32Status, &ui32Idle);

    //UART0���������¼�
    if (AM_HAL_UART_INT_RX & ui32Status)
    {
        //pUart_CallBack[UART0](UART_EVENT_RX);
    }
    //UART0���������¼�
    if (AM_HAL_UART_INT_TX & ui32Status)
    {
        //pUart_CallBack[UART0](UART_EVENT_TX);
    }
    //UART0�������ݳ�ʱ�¼�
    if (AM_HAL_UART_INT_RX_TMOUT & ui32Status)
    {
        //pUart_CallBack[UART0](UART_EVENT_RX_TIMEOUT);
    }

    if(ui32Idle == true)
    {
        ;
    }
}

//**********************************************************************
// ��������: ��ʼ��UART
// ���������	
// ���ز�����
//**********************************************************************
void SMDrv_UART_Init(void)
{
    memset(pUart_CallBack,NULL,sizeof(pUart_CallBack));
}

//**********************************************************************
// ��������: ����driver module ID��Ӳ����Ӧ��UART
// ���������	
//    modul: driver module ID,ֵ�ο�uart_module
//    ptype_info:Ҫ���õ�uart�ж����ͣ�FIFO����, �������жϣ�������ΪNULL
//    ut_callback:�ϲ�ע����жϻص�����
// ���ز�����Ret_InvalidParam��Ret_OK
//
// ����: ʹ��uart���շ����жϣ�����fifo������Ϊ1/8ʱ���������ж�
//      uart_openinfo type_info;
//      type_info.u32event_type = UART_EVENT_RX | UART_EVENT_TX;
//      type_info.u32fifo_type =  UART_RX_FIFO_1_8;
//      SMDrv_UART_Open(GPS_UART_MODULE,&type_info,ut_callback);
//**********************************************************************
ret_type SMDrv_UART_Open(uart_module modul,uart_openinfo *ptype_info,uart_cb ut_callback)
{
    uint32 u32Modul = UART_MAX,u32RxPin = IO_UNKNOW,u32TxPin = IO_UNKNOW;
    uint32 u32TxConfig = 0x00,u32RxConfig = 0x00;
    am_hal_gpio_pincfg_t g_UartConfig;
    void *handler = NULL;
#if AM_CMSIS_REGS
    IRQn_Type irq;
#else
    uint32 irq;
#endif

    //Ĭ�ϵ�UART����
    am_hal_uart_config_t UartConfig = 
    {
        // Standard UART settings: 115200-8-N-1
        .ui32BaudRate = 115200,
        .ui32DataBits = AM_HAL_UART_DATA_BITS_8,
        .ui32Parity = AM_HAL_UART_PARITY_NONE,
        .ui32StopBits = AM_HAL_UART_ONE_STOP_BIT,
        .ui32FlowControl = AM_HAL_UART_FLOW_CTRL_NONE,
    };    
    g_UartConfig.eDriveStrength  = AM_HAL_GPIO_PIN_DRIVESTRENGTH_2MA;

    //step 1:����ģ��UART����
    if(modul == GPS_UART_MODULE)
    {
        // Set TX and RX FIFOs to interrupt at half-full.
        UartConfig.ui32FifoLevels = (AM_HAL_UART_TX_FIFO_1_2 | AM_HAL_UART_RX_FIFO_1_2);
        
        // Buffers
        UartConfig.pui8TxBuffer = pui8TxBuffer;
        UartConfig.ui32TxBufferSize = sizeof(pui8TxBuffer);
        UartConfig.pui8RxBuffer = pui8RxBuffer;
        UartConfig.ui32RxBufferSize = sizeof(pui8RxBuffer);

        handler = pUart1_Handler;
#if AM_CMSIS_REGS
        irq = UART1_IRQn;
#else
        irq = AM_HAL_INTERRUPT_UART1;
#endif
        //���µ�code��Ҫ������Ŀԭ��ͼ����
        u32Modul = UART_ID1;
        u32RxPin = UART1_RX;
        u32TxPin = UART1_TX;
        u32TxConfig=AM_HAL_PIN_14_UART1TX;
        u32RxConfig=AM_HAL_PIN_15_UART1RX;
    }
    else if(modul == BASE_UART_MODULE)
    {
        // Set TX and RX FIFOs to interrupt at half-full.
        UartConfig.ui32FifoLevels = (AM_HAL_UART_TX_FIFO_1_2 | AM_HAL_UART_RX_FIFO_1_2);

        // Buffers ͨѶ������С�������ô�buffer
        UartConfig.pui8TxBuffer = NULL;
        UartConfig.ui32TxBufferSize = 0;
        UartConfig.pui8RxBuffer = NULL;
        UartConfig.ui32RxBufferSize = 0;
    
        handler = pUart1_Handler;
#if AM_CMSIS_REGS
        irq = UART0_IRQn;
#else
        irq = AM_HAL_INTERRUPT_UART0;
#endif
        //���µ�code��Ҫ������Ŀԭ��ͼ����
        u32Modul = UART_ID0;
        u32RxPin = UART0_RX;
        u32TxPin = UART0_TX;
        //u32TxConfig=AM_HAL_PIN_14_UART1TX;
        //u32RxConfig=AM_HAL_PIN_15_UART1RX;
    }
    else
    {
        return Ret_InvalidParam;
    }

    //step 2:��ʼ��������������uart
    if(am_hal_uart_initialize(u32Modul, &handler) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - init uart failed.\n"));
    if(am_hal_uart_power_control(handler, AM_HAL_SYSCTRL_WAKE, false) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - power on uart failed.\n"));
    if(am_hal_uart_configure(handler, &UartConfig) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - config uart failed.\n"));

    //step 3:����Ӧ��IO������ΪUART���ܣ�������UART����
    g_UartConfig.uFuncSel = u32RxConfig,
    am_hal_gpio_pinconfig(u32RxPin, g_UartConfig);
    g_UartConfig.uFuncSel = u32TxConfig,
    am_hal_gpio_pinconfig(u32TxPin, g_UartConfig);

    // Enable interrupts.
#if AM_CMSIS_REGS
    NVIC_EnableIRQ(irq);
#else
    am_hal_interrupt_enable(irq);
#endif

    //step 4:�����жϲ���ʹ��uart�ж�
    if(ut_callback != NULL)
    {
        pUart_CallBack[u32Modul] = ut_callback;
    }

    return Ret_OK;
}

//**********************************************************************
// ��������: �ر�driver module IDӲ����Ӧ��UART,��ʵ�ֵ͹���
// ���������	
//    modul: driver module ID,ֵ�ο�uart_module
// ���ز�����Ret_InvalidParam��Ret_OK
//**********************************************************************
ret_type SMDrv_UART_Close(uart_module modul)
{
    uint32 u32Modul = UART_MAX,u32RxPin = IO_UNKNOW,u32TxPin = IO_UNKNOW;
    //ui32Interrupt��ֵΪ: AM_HAL_INTERRUPT_UART1,AM_HAL_INTERRUPT_UART0
    void *handler = NULL;
#if AM_CMSIS_REGS
    IRQn_Type irq;
#else
    uint32 irq;
#endif

    am_hal_gpio_pincfg_t g_AM_HAL_GPIO_DISABLE =
    {
        .uFuncSel       = 3,
        .eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_2MA,
        .eGPOutcfg      = AM_HAL_GPIO_PIN_OUTCFG_DISABLE
    };

    //step 1:����ģ��UART����
    if(modul == GPS_UART_MODULE)
    {
        handler = pUart1_Handler;
#if AM_CMSIS_REGS
        irq = UART1_IRQn;
#else
        irq = AM_HAL_INTERRUPT_UART1;
#endif
        //���µ�code��Ҫ������Ŀԭ��ͼ����
        u32Modul = UART_ID1;
        u32RxPin = UART1_RX;
        u32TxPin = UART1_TX;
    }
    else if(modul == BASE_UART_MODULE)
    {
        handler = pUart0_Handler;
#if AM_CMSIS_REGS
        irq = UART0_IRQn;
#else
        irq = AM_HAL_INTERRUPT_UART0;
#endif
        //���µ�code��Ҫ������Ŀԭ��ͼ����
        u32Modul = UART_ID0;
        u32RxPin = UART0_RX;
        u32TxPin = UART0_TX;
    }
    else
        ;
    if(u32Modul >= UART_MAX)
        return Ret_InvalidParam;

    //step 2:disable UARTʱ�ӣ���UART
    am_hal_uart_power_control(handler, AM_HAL_SYSCTRL_DEEPSLEEP, false);
    am_hal_uart_deinitialize(handler);
    //step 3:��������uart�Ź���
    am_hal_gpio_pinconfig(u32RxPin, g_AM_HAL_GPIO_DISABLE);
    am_hal_gpio_pinconfig(u32TxPin, g_AM_HAL_GPIO_DISABLE);

    //step 4:����жϻص�������open��ʱ����������
#if AM_CMSIS_REGS
    NVIC_DisableIRQ(irq);
#else
    am_hal_interrupt_disable(irq);
#endif
    pUart_CallBack[u32Modul] = NULL;
    return Ret_OK;
}

//**********************************************************************
// ��������:  ����GPIO�ж����ȼ�,������uart�ж�
// �������:
//     modul: driver module ID,ֵ�ο�uart_module
//     prio:�ж����ȼ���bit0~2λֵ��Ч
// ���ز����� ��
//**********************************************************************
ret_type SMDrv_UART_SetIsrPrio(uart_module modul,uint32 prio)
{
    //ui32Interrupt��ֵΪ: AM_HAL_INTERRUPT_UART1,AM_HAL_INTERRUPT_UART0
#if AM_CMSIS_REGS
    IRQn_Type irq;
#else
    uint32 irq;
#endif
    if(modul == GPS_UART_MODULE)
    {
#if AM_CMSIS_REGS
        irq = UART1_IRQn;
#else
        irq = AM_HAL_INTERRUPT_UART1;
#endif
    }
    else if(modul == BASE_UART_MODULE)
    {
#if AM_CMSIS_REGS
        irq = UART0_IRQn;
#else
        irq = AM_HAL_INTERRUPT_UART0;
#endif
    }
    else
        return Ret_InvalidParam;

#if AM_CMSIS_REGS
    NVIC_SetPriority(irq,prio);
    NVIC_EnableIRQ(irq);
#else
    //step:���ò�ʹ��uart�ж�
    am_hal_interrupt_priority_set(irq, AM_HAL_INTERRUPT_PRIORITY(prio));
    am_hal_interrupt_enable(irq);
#endif
    return Ret_OK;
}

//**********************************************************************
// ��������: ʹ��/ʧ��UARTĳ�����͵��ж�
// �������:
//     modul: driver module ID,ֵ�ο�uart_module
//     irq_type:�ж�����
//     benable: ʹ��/ʧ��UARTĳ�����ж�  =1--ʹ�ܣ� =0--ʧ��
// ���ز����� ��
//**********************************************************************
ret_type SMDrv_UART_EnableIrq(uart_module modul,uint32 irq_type,uint8 benable)
{
    void *handler = NULL;

    if(modul == GPS_UART_MODULE)
        handler = pUart1_Handler;
    else if(modul == BASE_UART_MODULE)
        handler = pUart0_Handler;
    else
        return Ret_InvalidParam;

    if(irq_type != UART_EVENT_NONE)
    {
        am_hal_uart_interrupt_disable(handler, irq_type);
        if(benable == 1)
            am_hal_uart_interrupt_enable(handler, irq_type);
        else
            am_hal_uart_interrupt_disable(handler, irq_type);
    }
    return Ret_OK;
}

//**********************************************************************
// ��������: ��driver module ID��Ӧ��UARTд��n���ֽ�
// ���������	
//    modul: driver module ID
//    pData: Ҫ����������
//    len: Ҫ���������ݵĳ���
// ���ز�����
//    pu16LenWritten: ʵ�ʷ��͵ĳ���
//**********************************************************************
ret_type SMDrv_UART_WriteBytes(uart_module modul,uint8 *pData,uint16 len,uint16 *pu16LenWritten)
{
    am_hal_uart_transfer_t sUartWrite;
    void *handler = NULL;
    
    if(modul == GPS_UART_MODULE)
        handler = pUart1_Handler;
    else if(modul == BASE_UART_MODULE)
        handler = pUart0_Handler;
    else
        return Ret_InvalidParam;

    sUartWrite.ui32Direction = AM_HAL_UART_WRITE;
    sUartWrite.pui8Data = (uint8_t *) pData;
    sUartWrite.ui32NumBytes = len;
    sUartWrite.ui32TimeoutMs = 0;
    sUartWrite.pui32BytesTransferred = (uint32_t *)pu16LenWritten;

    if(am_hal_uart_transfer(handler, &sUartWrite) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - write uart failed.\n"));

    return Ret_OK;
}

//**********************************************************************
// ��������: ��driver module ID��Ӧ��UARTдn���ֽ�
// ���������	
//    modul: driver module ID
//    pBuffer: Ҫ��ȡ�����ݻ���buffer
//    len: Ҫ��ȡ�����ݵĳ���
// ���ز�����
//    pu16LenWritten: ʵ�ʶ�ȡ�����ݳ���
//**********************************************************************
ret_type SMDrv_UART_ReadBytes(uart_module modul,uint8 *pBuffer,uint16 len,uint16 *pu16ReadLen)
{
    am_hal_uart_transfer_t sUartRead;
    void *handler = NULL;
    
    if(modul == GPS_UART_MODULE)
        handler = pUart1_Handler;
    else if(modul == BASE_UART_MODULE)
        handler = pUart0_Handler;
    else
        return Ret_InvalidParam;

    sUartRead.ui32Direction = AM_HAL_UART_READ;
    sUartRead.pui8Data = (uint8_t *) pBuffer;
    sUartRead.ui32NumBytes = len;
    sUartRead.ui32TimeoutMs = 0;
    sUartRead.pui32BytesTransferred = (uint32_t *)pu16ReadLen;

    if(am_hal_uart_transfer(handler, &sUartRead) != AM_HAL_STATUS_SUCCESS)
        Err_Info((0,"Error - read uart failed.\n"));

    return Ret_OK;
}
#else

/*
//UART0�жϷ������
void am_uart_isr1(void)
{
    uint32 statuvalue;

    statuvalue = am_hal_uart_int_status_get(UART0,true);
    am_hal_uart_int_clear(UART0,0xFFFFFFFF);

    //UART0���������¼�
    if (AM_HAL_UART_INT_RX & statuvalue)
    {
        pUart_CallBack[UART0](UART_EVENT_RX);
    }
    //UART0���������¼�
    if (AM_HAL_UART_INT_TX & statuvalue)
    {
        pUart_CallBack[UART0](UART_EVENT_TX);
    }
    //UART0�������ݳ�ʱ�¼�
    if (AM_HAL_UART_INT_RX_TMOUT & statuvalue)
    {
        pUart_CallBack[UART0](UART_EVENT_RX_TIMEOUT);
    }
}

//UART1�жϷ������
void am_uart1_isr1(void)
{
    uint32    statuvalue;
    statuvalue = am_hal_uart_int_status_get(UART1,true);
    am_hal_uart_int_clear(UART1,0xFFFFFFFF);

    //UART1���������¼�
    if (AM_HAL_UART_INT_RX & statuvalue)
    {
        pUart_CallBack[UART1](UART_EVENT_RX);
    }
    //UART1���������¼�
    if (AM_HAL_UART_INT_TX & statuvalue)
    {
        pUart_CallBack[UART1](UART_EVENT_TX);
    }
    //UART1�������ݳ�ʱ�¼�
    if (AM_HAL_UART_INT_RX_TMOUT & statuvalue)
    {
        pUart_CallBack[UART1](UART_EVENT_RX_TIMEOUT);
    }
}
*/
//**********************************************************************
// ��������: ����FIFO ��UART �ж�����
// ���������u32Modul: driver module ID,ֵ�ο�uart_module
//    ptype_info:FIFO���ͺ��ж�����   
// ���ز�����TRUE:����
//           FALSE:���õ��ж�������Ч������ע��ص�����
//**********************************************************************
static uint8 Uart_Irq_Init(uint32 u32Modul,uart_openinfo *ptype_info)
{
    //����UART fifo
    if(ptype_info->u32fifo_type != UART_FIFO_NONE)
    {
        am_hal_uart_fifo_config(u32Modul, ptype_info->u32fifo_type);
    }

    //����uart�ж����ͼ�ʹ��
    if(ptype_info->u32event_type != UART_EVENT_NONE)
    {
        am_hal_uart_int_clear(u32Modul,ptype_info->u32event_type);
        am_hal_uart_int_enable(u32Modul,ptype_info->u32event_type);
    }
    else
        return FALSE;
    return TRUE;
}

//**********************************************************************
// ��������: ��ʼ��UART
// ���������	
// ���ز�����
//**********************************************************************
void SMDrv_UART_Init(void)
{
    memset(pUart_CallBack,NULL,sizeof(pUart_CallBack));
}

//**********************************************************************
// ��������: ����driver module ID��Ӳ����Ӧ��UART
// ���������	
//    modul: driver module ID,ֵ�ο�uart_module
//    ptype_info:Ҫ���õ�uart�ж����ͣ�FIFO����, �������жϣ�������ΪNULL
//    ut_callback:�ϲ�ע����жϻص�����
// ���ز�����Ret_InvalidParam��Ret_OK
//
// ����: ʹ��uart���շ����жϣ�����fifo������Ϊ1/8ʱ���������ж�
//      uart_openinfo type_info;
//      type_info.u32event_type = UART_EVENT_RX | UART_EVENT_TX;
//      type_info.u32fifo_type =  UART_RX_FIFO_1_8;
//      SMDrv_UART_Open(GPS_UART_MODULE,&type_info,ut_callback);
//**********************************************************************
ret_type SMDrv_UART_Open(uart_module modul,uart_openinfo *ptype_info,uart_cb ut_callback)
{
    uint32 u32Modul = UART_MAX,u32RxPin = IO_UNKNOW,u32TxPin = IO_UNKNOW;
    uint32 u32TxConfig = 0x00,u32RxConfig = 0x00;
    //Ĭ�ϵ�UART����
    am_hal_uart_config_t UartConfig = 
    {
        .ui32BaudRate   = 9600,
        .ui32DataBits   = AM_HAL_UART_DATA_BITS_8,
        .bTwoStopBits   = false,
        .ui32Parity     = AM_HAL_UART_PARITY_NONE,
        .ui32FlowCtrl   = AM_HAL_UART_FLOW_CTRL_NONE,
    };

    //step 1:����ģ��UART����
    if(modul == GPS_UART_MODULE)
    {
        u32Modul = UART1;
        u32RxPin = UART1_RX;
        u32TxPin = UART1_TX;
        u32TxConfig=AM_HAL_PIN_14_UART1TX;
        u32RxConfig=AM_HAL_PIN_15_UART1RX;
    }
    else
    {
        return Ret_InvalidParam;
    }

    //step 2:ʹ��UART���Ĺ����ʱ��
    am_hal_uart_pwrctrl_enable(u32Modul);	
    am_hal_uart_clock_enable(u32Modul);
    //Ӧ�ڹر�uart������£�����UART����
    am_hal_uart_disable(u32Modul);

    //step 3:����Ӧ��IO������ΪUART���ܣ�������UART����
    SMDrv_GPIO_SetIOPad(u32TxPin,u32TxConfig);
    SMDrv_GPIO_SetIOPad(u32RxPin,u32RxConfig);
    am_hal_uart_config(u32Modul,&UartConfig);

    //step 4:�����жϲ���ʹ��uart�ж�
    if((ptype_info != NULL) && (ut_callback != NULL))
    {
        //��ʼ��uart�ж����ͺ�fifo���
        if(Uart_Irq_Init(u32Modul,ptype_info) == TRUE)
        {
            //ֻ����
            pUart_CallBack[u32Modul] = ut_callback;
        }
    }

    //step 5: ʹ��UART
	am_hal_uart_enable(u32Modul);
    return Ret_OK;
}

//**********************************************************************
// ��������: �ر�driver module IDӲ����Ӧ��UART,��ʵ�ֵ͹���
// ���������	
//    modul: driver module ID,ֵ�ο�uart_module
// ���ز�����Ret_InvalidParam��Ret_OK
//**********************************************************************
ret_type SMDrv_UART_Close(uart_module modul)
{
    uint32 u32Modul = UART_MAX,u32RxPin = IO_UNKNOW,u32TxPin = IO_UNKNOW;
    //ui32Interrupt��ֵΪ: AM_HAL_INTERRUPT_UART1,AM_HAL_INTERRUPT_UART0
    uint32 ui32Interrupt;

    //step 1:����ģ��UART����
    if(modul == GPS_UART_MODULE)
    {
        u32Modul = UART1;
        u32RxPin = UART1_RX;
        u32TxPin = UART1_TX;
        ui32Interrupt = AM_HAL_INTERRUPT_UART1;
    }
    
    if(u32Modul >= UART_MAX)
        return Ret_InvalidParam;

    //step 2:disable UARTʱ�ӣ���UART
    am_hal_uart_clock_disable(u32Modul);
    am_hal_uart_disable(u32Modul);
    //step 3:��������uart�Ź���
    SMDrv_GPIO_SetIOPad(u32RxPin,AM_HAL_PIN_INPUT |AM_HAL_GPIO_PULL24K);
    SMDrv_GPIO_SetIOPad(u32TxPin,AM_HAL_PIN_INPUT |AM_HAL_GPIO_PULL24K ); 
    //step 4:disable uart���Ĺ���
    am_hal_uart_pwrctrl_disable(u32Modul);

    //step 5:����жϻص�������open��ʱ����������
    am_hal_interrupt_disable(ui32Interrupt);
    pUart_CallBack[u32Modul] = NULL;
    return Ret_OK;
}

//**********************************************************************
// ��������:  ����GPIO�ж����ȼ�,������uart�ж�
// �������:
//     modul: driver module ID,ֵ�ο�uart_module
//     prio:�ж����ȼ���bit0~2λֵ��Ч
// ���ز����� ��
//**********************************************************************
ret_type SMDrv_UART_SetIsrPrio(uart_module modul,uint32 prio)
{
    //ui32Interrupt��ֵΪ: AM_HAL_INTERRUPT_UART1,AM_HAL_INTERRUPT_UART0
    uint32 ui32Interrupt;
    if(modul == GPS_UART_MODULE)
        ui32Interrupt = AM_HAL_INTERRUPT_UART1;
    else
        return Ret_InvalidParam;

    //step:���ò�ʹ��uart�ж�
    am_hal_interrupt_priority_set(ui32Interrupt, AM_HAL_INTERRUPT_PRIORITY(prio));
    am_hal_interrupt_enable(ui32Interrupt);
    return Ret_OK;
}

//**********************************************************************
// ��������: ʹ��/ʧ��UARTĳ�����͵��ж�
// �������:
//     modul: driver module ID,ֵ�ο�uart_module
//     irq_type:�ж�����
//     benable: ʹ��/ʧ��UARTĳ�����ж�  =1--ʹ�ܣ� =0--ʧ��
// ���ز����� ��
//**********************************************************************
ret_type SMDrv_UART_EnableIrq(uart_module modul,uint32 irq_type,uint8 benable)
{
    uint32 u32Modul = UART_MAX;

    if(modul == GPS_UART_MODULE)
        u32Modul = UART1;
    else
        return Ret_InvalidParam;

    if(irq_type != UART_EVENT_NONE)
    {
        am_hal_uart_int_clear(u32Modul,irq_type);
        if(benable == 1)
            am_hal_uart_int_enable(u32Modul,irq_type);
        else
            am_hal_uart_int_disable(u32Modul,irq_type);
    }
    return Ret_OK;
}

//**********************************************************************
// ��������: ��driver module ID��Ӧ��UARTд1 ���ֽ�
// ���������	
//    modul: driver module ID
//    u8ch: Ҫ�������ַ�
// ���ز�����
//**********************************************************************
ret_type SMDrv_UART_WriteByte(uart_module modul,uint8 u8ch)
{
    uint32 u32Modul = UART_MAX;

    if(modul == GPS_UART_MODULE)
        u32Modul = UART1;
    else
        return Ret_InvalidParam;

    am_hal_uart_char_transmit_polled(u32Modul,u8ch);
    return Ret_OK;
}

//**********************************************************************
// ��������: ��driver module ID��Ӧ��UARTд�ȶ���ֽ�
// ���������	
//    modul: driver module ID
//    pData: Ҫ����������
//    len: Ҫ���������ݵĳ���
// ���ز�����
//**********************************************************************
ret_type SMDrv_UART_WriteBytes(uart_module modul,uint8 *pData,uint16 len)
{
    uint32 u32Modul = UART_MAX;

    if(modul == GPS_UART_MODULE)
        u32Modul = UART1;
    else
        return Ret_InvalidParam;

    while(len > 0)
    {
        am_hal_uart_char_transmit_polled(u32Modul,*pData);
        pData++,len--;
    }
    return Ret_OK;
}

//**********************************************************************
// ��������: ��driver module ID��Ӧ��UARTд1 ���ֽ�
// ���������	
//    modul: driver module ID,ֵ�ο�uart_module
// ���ز�������ȡ����ֵ
//**********************************************************************
uint8 SMDrv_UART_ReadByte(uart_module modul)
{
    uint32 u32Modul = UART1;
    uint8 u8Char;

    if(modul == GPS_UART_MODULE)
    {
        u32Modul = UART1;
    }
    
    am_hal_uart_char_receive_polled (u32Modul,(char*)&u8Char);
    return u8Char;
}
#endif

#endif

