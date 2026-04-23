#ifndef __UART_H__
#define __UART_H__

void UartInit(void);
void Uart_SendChar(unsigned char Byte);
// 获取串口是否工作
bit UartStatus(void);

#endif
