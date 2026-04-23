#include "time.h"
#include "stc15.h"
#include "../uart/UART.h"

void Timer0_Init(void)		//1毫秒@11.0592MHz
{
	AUXR &= 0x7F;			//定时器时钟12T模式
	TMOD &= 0xF0;			//设置定时器模式
	TMOD |= 0x01;			//设置定时器模式
	TL0 = 0x66;				//设置定时初始值
	TH0 = 0xFC;				//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
	ET0 = 1;				//使能定时器0中断
}






// 16位自动重装
void Timer2_Init(void) // 20毫秒@11.0592MHz
{
  if (UartStatus())
  {
    return;
  }
  AUXR &= 0xFB;      // 定时器时钟12T模式
  AUXR &= ~(1 << 3); // 清零 Bit 3 显示地设置为定时器模式（一般复位后就是0）
  T2L = 0x00;        // 设置定时初始值
  T2H = 0xB8;        // 设置定时初始值
  AUXR |= 0x10;      // 定时器2开始计时
  IE2 |= 0x04;       // 使能定时器2中断
}

void Timer2_Stop(void)
{
  if (UartStatus())
  {
    return;
  }
  AUXR &= ~0x10; // 定时器2停止计时
  IE2 &= ~0x04;  // 关闭定时器2中断
}

/**

void Timer0_Rountine(void) interrupt 1{
}

void Timer2_Isr(void) interrupt 12
{
}

**/
