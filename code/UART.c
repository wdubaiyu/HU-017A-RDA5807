#include <STC15.H>
#include <stdio.h>

void UartInit(void)		//9600bps@27.000MHz
{
	SCON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x01;		//串口1选择定时器2为波特率发生器
	AUXR |= 0x04;		//定时器2时钟为Fosc,即1T
	T2L = 0x41;		//设定定时初值
	T2H = 0xFD;		//设定定时初值
	AUXR |= 0x10;		//启动定时器2
}


void Uart_SendChar(unsigned char  dat)
{
    SBUF = dat; 
    while(!TI); 
    TI = 0; 
}
 
char putchar(char c)//重定向
{
    Uart_SendChar(c);
    return c;
}

