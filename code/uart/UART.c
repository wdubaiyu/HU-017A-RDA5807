#include "../delay/Delay.h"
#include "stc15.h"
#include "stdio.h"


bit uart_flag = 0;

bit UartStatus() { return uart_flag; }

void UartInit(void) // 115200bps@11.0592MHz
{
  Delay(300);
  uart_flag = 1;
  SCON = 0x50;  // 8位数据,可变波特率
  AUXR |= 0x01; // 串口1选择定时器2为波特率发生器
  AUXR |= 0x04; // 定时器时钟1T模式
  T2L = 0xE8;   // 设置定时初始值
  T2H = 0xFF;   // 设置定时初始值
  AUXR |= 0x10; // 定时器2开始计时
}

void Uart_SendChar(unsigned char dat) {
  SBUF = dat;
  while (!TI)
    ;
  TI = 0;
}

char putchar(char c) // 重定向
{
  Uart_SendChar(c);
  return c;
}
