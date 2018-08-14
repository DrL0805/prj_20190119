#ifndef         APP_UART_H
#define         APP_UART_H


uint8_t CheckSumFunc(unsigned char *data, unsigned short length);
void App_Uart_Analysis(uint8_t *dataBuf, uint8_t length);
void App_DebugTestDecInfo(void);

#endif          //APP_UART_H



