#include "platform_common.h"
#include "mid_uart.h"
#include "mid_common.h"

/* FreeRTOS includes */
#include "rtos.h"



/*******************macro define*******************/
#define dec(output,var,n) if(var>=n) output = (var/n)%10+0x30;else output = 0x30

enum
{
	UART_APP_RECEIVE_FINISH_ISR_CB,
	UART_APP_SEND_FINISH_ISR_CB,
};



/*******************variable define*******************/
#if 0
static SemaphoreHandle_t    uartMutex0;
static SemaphoreHandle_t    uartMutex1;

static void (*UartIsrCb[2])(uint16 isrHandle);
static uint8 receiveDataBuf0[32];
static uint8 receiveLength0;
static uint8 receiveDataBuf1[32];
static uint8 receiveLength1;

void (*UartAnalysis[2])(uint8* dataBuf, uint8 length);
#endif



/*******************func define*******************/

//**********************************************************************
// 函数功能:	uart 接收完成中断
// 输入参数：	
// 返回参数：
void Mid_Uart_RecieveFinishIsr(uint8 uartmodule,uint8* dataBuf, uint8 length)
{
}



//**********************************************************************
// 函数功能:	uart 发送完成中断
// 输入参数：	
// 返回参数：
void Mid_Uart_SendFinishIsr(uint8 uartmodule)
{
	//UartIsrCb[uartmodule](UART_APP_SEND_FINISH_ISR_CB);
}

//**********************************************************************
// 函数功能:	uart 初始化
// 输入参数：	
// 返回参数：
void Mid_Uart_Init(uint8 uartmodule,void (*IsrCb)(uint16 isrHandle))
{
	
}



//**********************************************************************
// 函数功能:	应用层UART 回调初始化
// 输入参数：	appHandle:	应用句柄
// 				Cb:			应用层回调函数
// 返回参数：
void Mid_Uart_ReceiveCbInit(uint8 uartmodule,void (*Cb)(uint8* dataBuf, uint8 length))
{
	//UartAnalysis[uartmodule]     = Cb;
}

//**********************************************************************
// 函数功能:	开启uart口功能
// 输入参数:
// 返回参数:
 void Mid_Uart_Enable(uint8 uartmodule)
{

}

//**********************************************************************
// 函数功能:	关闭uart口功能
// 输入参数：	
// 返回参数：
 void Mid_Uart_Disable(uint8 uartmodule)
{

}


//**********************************************************************
// 函数功能:	通过硬件uart口发送数据，
// 输入参数：	
// dataBuf 		： 	数据指针
// length 		： 	数据长度
// 返回参数：
 void Mid_Uart_Send(uint8 uartmodule,uint8* dataBuf, uint8 length)
{

}

//**********************************************************************
// 函数功能:	通过硬件uart口发送数据，
// 输入参数：	
// dataBuf 		：	数据指针
// length 		：	数据长度
// 返回参数：
uint16 Mid_Uart_EnvetProcess(uartapp_event_s* msg)
{
	return 0;
}


