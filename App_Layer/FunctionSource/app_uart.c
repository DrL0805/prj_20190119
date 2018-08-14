#include "platform_common.h"
#include "rtos.h"
#include "mid_interface.h"
#include "app_task.h"
#include "app_variable.h"
#include "app_protocal.h"
#include "app_key.h"
#include "app_systerm.h"
#include "app_win_common.h"
#include "app_win_process.h"
#include "app_display.h"
#include "sm_sys.h"
#include "app_uart.h"

//**********************************************************************
// 函数功能:    生成协议校验证码
// 输入参数：    
// 返回参数：    无
//**********************************************************************
uint8_t CheckSumFunc(unsigned char *data, unsigned short length)
{
    unsigned char checksum = 0;
    unsigned char i;
    for(i = 0; i < length; i++)
    {
        checksum +=data[i];
    }
    return checksum;
}

//**********************************************************************
// 函数功能:    串口接收数据解析
// 输入参数：    
//dataBuf:      数据buf
//length :  	数据长度
// 返回参数：    无
//**********************************************************************
void App_Uart_Analysis(uint8_t *dataBuf, uint8_t length)
{
	flash_task_msg_t    flashMsg;

    unsigned short ret = 0;
    uint8_t dataRead[35];

//    uint8_t CLEAR_SCREEN[]    = {0x22,0x05,0x00,0x27};
    uint8_t WRITE_SUCCESS[]   = {0x22,0x07,0x00,0x29};
    
    if(dataBuf[0] == 0x22)
    {        
        switch(dataBuf[1])//////////
        {
            case 0x02: 
			case 0x04:

            break;
            case 0x06:
            break;
            case 0x08:			
			flashMsg.id 									= EXTFLASH_ID;
			flashMsg.flash.extflashEvent.para.dataAddr 		= &dataBuf[3];
			flashMsg.flash.extflashEvent.para.length 		= 20;
			flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_WRITE_SN;			
			flashMsg.flash.extflashEvent.para.result 		= FlashTask_EventSet(&flashMsg);

			ret |= flashMsg.flash.extflashEvent.para.result;			    

            flashMsg.id 									= EXTFLASH_ID;
			flashMsg.flash.extflashEvent.para.dataAddr 		= &dataBuf[23];
			flashMsg.flash.extflashEvent.para.length 		= 6;
			flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_WRITE_BLEMAC;		
			flashMsg.flash.extflashEvent.para.result 		= FlashTask_EventSet(&flashMsg);
								
            if(ret == 0)
            {
				Mid_Uart_Enable(DebugUartModule);
                Mid_Uart_Send(DebugUartModule,WRITE_SUCCESS,4);
                SMDrv_SYS_DelayMs(2);  
            }		
			flashMsg.id 									= EXTFLASH_ID;
			flashMsg.flash.extflashEvent.para.dataAddr 		= &dataRead[0];
			flashMsg.flash.extflashEvent.para.length 		= 20;
			flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_READ_SN;			
			flashMsg.flash.extflashEvent.para.result 		= FlashTask_EventSet(&flashMsg);
			

			ret |= flashMsg.flash.extflashEvent.para.result;	
			
            flashMsg.id 									= EXTFLASH_ID;
			flashMsg.flash.extflashEvent.para.dataAddr 		= &dataRead[20];
			flashMsg.flash.extflashEvent.para.length 		= 6;
			flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_READ_BLEMAC;			
			flashMsg.flash.extflashEvent.para.result 		= FlashTask_EventSet(&flashMsg);
			
			ret |= flashMsg.flash.extflashEvent.para.result;
			if(ret == 0)
			{
				Mid_Uart_Send(DebugUartModule,dataRead,26);           
				SMDrv_SYS_DelayMs(8); 
			}
			
            App_DebugTestDecInfo();
            break;
       }
    }
    else if(dataBuf[0] == 0x23 && dataBuf[1] == 0x1b)
    {	
			flashMsg.id 									= EXTFLASH_ID;
			flashMsg.flash.extflashEvent.para.dataAddr 		= &dataRead[2];
			flashMsg.flash.extflashEvent.para.length 		= 20;
			flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_WRITE_SN;			
			flashMsg.flash.extflashEvent.para.result 		= FlashTask_EventSet(&flashMsg);
			
			ret |= flashMsg.flash.extflashEvent.para.result;			    

            flashMsg.id 									= EXTFLASH_ID;
			flashMsg.flash.extflashEvent.para.dataAddr 		= &dataRead[22];
			flashMsg.flash.extflashEvent.para.length 		= 6;
			flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_WRITE_BLEMAC;			
			flashMsg.flash.extflashEvent.para.result 		= FlashTask_EventSet(&flashMsg);
			
			ret |= flashMsg.flash.extflashEvent.para.result;			    
            if(ret == 0)
            {
                Mid_Uart_Enable(DebugUartModule);
                Mid_Uart_Send(DebugUartModule,WRITE_SUCCESS,4);
                SMDrv_SYS_DelayMs(2);  
            }

            flashMsg.id 									= EXTFLASH_ID;
			flashMsg.flash.extflashEvent.para.dataAddr 		= &dataRead[0];
			flashMsg.flash.extflashEvent.para.length 		= 20;
			flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_READ_SN;			
			flashMsg.flash.extflashEvent.para.result 		= FlashTask_EventSet(&flashMsg);
				
			ret |= flashMsg.flash.extflashEvent.para.result;			    

            flashMsg.id 									= EXTFLASH_ID;
			flashMsg.flash.extflashEvent.para.dataAddr 		= &dataRead[20];
			flashMsg.flash.extflashEvent.para.length 		= 6;
			flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_READ_BLEMAC;			
			flashMsg.flash.extflashEvent.para.result 		= FlashTask_EventSet(&flashMsg);
			
			ret |= flashMsg.flash.extflashEvent.para.result;			    
			if(ret == 0)
			{
				Mid_Uart_Send(DebugUartModule,dataRead,26);            
				SMDrv_SYS_DelayMs(10); 
			}				
			
            App_DebugTestDecInfo();
    }
}

//**********************************************************************
// 函数功能:    厂测信息打印，固件版本、SN等
// 输入参数：    
// 返回参数：    无
void App_DebugTestDecInfo(void)
{    
    unsigned char   debugData[30] = {0};
    unsigned char 	i;
    unsigned char 	dataTemp[6];
    unsigned char 	dataTemp2[12];
    uint8_t  		debugdata2[35];
    uint32_t 		firmversion;

    flash_task_msg_t    flashMsg;

    // disp version
    debugData[0]  = 0x22;
    debugData[1]  = 0x03;
    debugData[2]  = 16;
    debugData[3]  = 0x02;   //start line 0
    debugData[4]  = 0x00;   //start list 0
    debugData[5]  = 'V';
    debugData[6]  = 'E';
    debugData[7]  = 'R';
    debugData[8]  = ':';
    firmversion = VersionRead();
    
    debugData[16] = firmversion % 10 +'0';//16进制转ASCII，小于9 加‘0’（0的ASCII为48）   
	debugData[15] = (firmversion / 10 % 10) +'0';
	debugData[14] = (firmversion / 100 % 10) +'0';
	debugData[13] = (firmversion / 1000 % 10) +'0';
    debugData[12] = (firmversion / 10000 % 10) +'0';
	debugData[11] = (firmversion / 100000 % 10) +'0';
	debugData[10] = (firmversion / 1000000 % 10) +'0';
	debugData[9]  = (firmversion / 10000000 % 10) +'0';
    debugData[17] = CheckSumFunc(debugData, 17);

	Mid_Uart_Enable(DebugUartModule);	
    Mid_Uart_Send(DebugUartModule,debugData,18);
    SMDrv_SYS_DelayMs(10);  

    // disp SN
    debugData[0]  = 0x22;
    debugData[1]  = 0x03;
    debugData[2]  = 5;
    debugData[3]  = 0x03;   //start line 0
    debugData[4]  = 0x00;   //start list 0
    debugData[5]  = 'S';
    debugData[6]  = 'N';
    debugData[7]  = ':';
    debugData[8] = CheckSumFunc(debugData, 8); 

    Mid_Uart_Enable(DebugUartModule);	
    Mid_Uart_Send(DebugUartModule,debugData,9);
    SMDrv_SYS_DelayMs(2);  
  
    debugData[0]  = 0x22;
    debugData[1]  = 0x03;
    debugData[2]  = 22;
    debugData[3]  = 0x04;   //start line 0
    debugData[4]  = 0x00;   //start list 0
    flashMsg.id 									= EXTFLASH_ID;
	flashMsg.flash.extflashEvent.para.dataAddr 		= &debugdata2[0];
	flashMsg.flash.extflashEvent.para.length 		= 20;
	flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_READ_SN;	
	flashMsg.flash.extflashEvent.para.result 		= FlashTask_EventSet(&flashMsg);
		    	
	if(flashMsg.flash.extflashEvent.para.result == 0)
	{
		for(i = 0; i < 20; i++)
		{
			if (debugdata2[i] == 0xff)
			{
				debugdata2[i] = 'F';
			}
			debugData[5+i] = debugdata2[i];	
		}		
	}
	else
	{
		for(i = 0; i < 20; i++)
		debugData[5+i] = 'F';
	}
    debugData[25] = CheckSumFunc(debugData, 25);
    Mid_Uart_Enable(DebugUartModule);	
    Mid_Uart_Send(DebugUartModule,debugData,26); 
    SMDrv_SYS_DelayMs(8);  

    // disp BLEMAC
    debugData[0]  = 0x22;
    debugData[1]  = 0x03;
    debugData[2]  = 22;
    debugData[3]  = 0x05;   //start line 0
    debugData[4]  = 0x00;   //start list 0
    debugData[5]  = 'B';
    debugData[6]  = 'L';
    debugData[7]  = 'E';
    debugData[8]  = '-';
    debugData[9]  = 'M';
    debugData[10] = 'A';
    debugData[11] = 'C';
    debugData[12] = ':';

	flashMsg.id 									= EXTFLASH_ID;
	flashMsg.flash.extflashEvent.para.dataAddr 		= &dataTemp[0];
	flashMsg.flash.extflashEvent.para.length 		= 6;
	flashMsg.flash.extflashEvent.id 				= EXTFLASH_EVENT_READ_BLEMAC;
	flashMsg.flash.extflashEvent.para.result 		= FlashTask_EventSet(&flashMsg);
	
	if(flashMsg.flash.extflashEvent.para.result == 0)
	{
		for(i = 0; i < 6; i++)
		{
			dataTemp2[i*2 + 1]  = dataTemp[i] % 16;
			dataTemp2[i*2]      = dataTemp[i] / 16;
		}
		for(i = 0; i < 12; i++)
		{
			if(dataTemp2[i] > 9)
			{
				dataTemp2[i] += 'A' - 10;
			}
			else
			{
				dataTemp2[i] += '0';
			}
		}
	}
	else
	{
		for(i = 0; i < 12; i++)
		{
			dataTemp2[i] = 'F';
		}
	}
	
    for(i = 0; i < 12; i++)
    debugData[13+i] = dataTemp2[i];
    debugData[25] = CheckSumFunc(debugData, 25);

    Mid_Uart_Enable(DebugUartModule);
    Mid_Uart_Send(DebugUartModule,debugData,26);
    SMDrv_SYS_DelayMs(8);  
}

