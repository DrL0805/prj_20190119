#ifndef         MID_UART_H
#define         MID_UART_H


typedef enum
{
	UARTAPP_SENDFINISH,
	UARTAPP_RECEIVE,
}UARTAPP_INTERFACE_T;

typedef struct 
{
	uint16	id;
	uint8 	uartmodule;
}uartapp_event_s;


void Mid_Uart_Init(uint8 uartmodule,void (*IsrCb)(uint16 isrHandle));
void Mid_Uart_Enable(uint8 uartmodule);
void Mid_Uart_Disable(uint8 uartmodule);
void Mid_Uart_Send(uint8 uartmodule,uint8* dataBuf, uint8 length);
void Mid_Uart_ReceiveCbInit(uint8 uartmodule, void (*Cb)(uint8* dataBuf, uint8 length));
uint16 Mid_Uart_EnvetProcess(uartapp_event_s* msg);



#endif			//UART_APP_H
