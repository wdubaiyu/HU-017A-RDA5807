#include "time2.h"
#include <stc15.h>

void Timer2Init() // 20毫秒@33.1776MHz
{
	AUXR &= 0xFB; // 定时器时钟12T模式
	T2L = 0x00;	  // 设置定时初值
	T2H = 0x28;	  // 设置定时初值
	AUXR |= 0x10; // 定时器2开始计时
}

/**

void Timer2_Rountine(void) interrupt 12
{
	static uint16t timed_stanby_count;
	if (POWER_STATUS == 1)
	{
		if (++timed_stanby_count > 3000)
		{
			LED_TIMED_STANDBY -= 1; // 减去一分钟
			timed_stanby_count = 0; // 重新计数
		}
	}
}
**/
