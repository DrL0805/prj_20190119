#include "mid_uart.h"
#include "main.h"


static void uart_callback(uint32 uart_event)
{
	Mid_Schd_TaskMsg_T Msg;
	
	SEGGER_RTT_printf(0,"uart_event %d \n", uart_event);
	
    if(uart_event == UART_EVENT_RX_TIMEOUT)
	{
		Msg.Id = eSchdTaskMsgUart;
		Msg.Param.Uart.Module = BASE_UART_MODULE;
		Msg.Param.Uart.EventType = UART_EVENT_RX_TIMEOUT;
        Mid_Schd_TaskEventSet(&Msg, 1);
	}
	
    if(uart_event == UART_EVENT_RX)
	{
	}   
}

void Mid_Uart_Init(void)
{
	SMDrv_UART_Init();
	
	SMDrv_UART_Open(BASE_UART_MODULE,uart_callback);
	SMDrv_UART_SetIsrPrio(BASE_UART_MODULE, 3);
	SMDrv_UART_EnableIrq(BASE_UART_MODULE, UART_EVENT_RX|UART_EVENT_RX_TIMEOUT,TRUE);	
}

