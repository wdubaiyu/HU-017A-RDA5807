#include "time0.h"
#include <stc15.h>

void Timer0Init(void) // 1毫秒@27.000MHz
{
	AUXR |= 0x80; // 定时器时钟1T模式
	TMOD &= 0xF0; // 设置定时器模式
	TMOD |= 0x01; // 设置定时器模式
	TL0 = 0x88;	  // 设置定时初值
	TH0 = 0x96;	  // 设置定时初值

	TF0 = 0; // 清除TF0标志
	TR0 = 1; // 定时器0开始计时
	ET0 = 1; // 打开中断允许寄存器
	EA = 1;	 // 打开中断控制总开关
}

/**

void Timer0_Rountine(void) interrupt 1{
	if(++countl == 50){
		countl=0;
		++VOL;
		CHAGNE_VOl(VOL);
	}

}


**/
