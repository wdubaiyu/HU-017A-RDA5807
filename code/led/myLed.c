#include <STC15.H>
#include <stdio.h>
#include "Delay.h"
#include "led/74HC595.h"
#include "led/myLed.h"
//,0xC7 -->L11  0x89-->H12
uint8t code NixieTable[] = {
	0xC0, 0xF9, 0xA4, 0xB0, 0x99,		// 0,1,2,3,4
	0x92, 0x82, 0xF8, 0x80, 0x90, 0x7F, // 5,6,7,8,9,点
	0x88, 0x83, 0xC6, 0xA1, 0x86,		// A,b,C,d,E
	0x8E, 0x8C, 0xC1, 0xCE, 0x91,		// F,P,U,T,y
	0x89, 0xC7, 0x12, 0xC8, 0xAB};		// L,H,S,N,n
uint8t code NixieTableDp[] = {
	0x40, 0x79, 0x24, 0x30, 0x19,
	0x12, 0x02, 0x78, 0x00, 0x10, 0x7F,
	0xC7, 0x89};

uint16t LED_FRE_REAL = 8700;
uint8t DISPLAY_type = 0x0A;
uint8t LED_RSSI = 0x00;
uint8t LED_SNR = 0x00;
// 手动搜台方向
bit LED_SEEK_D = 1;
// 手动搜台标志 1列表换，0搜台
bit LED_HAND_MARK = 1;
// 睡眠模式剩余时间
uint16t LED_SELL_TIME = 0x1F40; // 统计睡眠模式剩余时间8s

void DisplayNUM(uint8t a, b, c, d, dp);

/**
 内部使用 根据dpf判断是否需要小数点
**/
char getData(uint8t a, dpf)
{
	if (dpf)
	{
		return NixieTableDp[a];
	}
	else
	{
		return NixieTable[a];
	}
}

// 显示睡眠模式
void DispaySELLP()
{
	if (sys_sleep_mode)
	{
		DisplayNUM(0xFF, 21, 0xFF, 0xFF, 0xFF);
	}
	else
	{
		DisplayNUM(0xFF, 22, 0xFF, 0xFF, 0xFF);
	}
}

// 显示频率
void DispayF(uint16t temp)
{
	uint8t NUM_GE, NUM_SHI, NUM_BAI, NUM_DEC;
	NUM_BAI = temp / 10000;
	NUM_SHI = (temp % 10000) / 1000;
	NUM_GE = (temp % 1000) / 100;
	NUM_DEC = (temp % 100) / 10;
	if (NUM_BAI < 1)
	{
		DisplayNUM(100, NUM_SHI, NUM_GE, NUM_DEC, 3);
	}
	else
	{
		DisplayNUM(NUM_BAI, NUM_SHI, NUM_GE, NUM_DEC, 3);
	}
}
// 显示频率(loop调用时候显示为递增效果)
void DispayFRE(void)
{

	if (LED_FRE_REAL == sys_freq)
	{
		DispayF(LED_FRE_REAL);
		return;
	}

	if (LED_HAND_MARK) // 列表换台
	{
		LED_FRE_REAL = sys_freq;
		DispayF(sys_freq);
		return;
	}

	// 正向  数码管频率在增加的效果
	if (LED_SEEK_D)
	{
		while (LED_FRE_REAL != sys_freq)
		{
			DispayF(++LED_FRE_REAL);
			if (LED_FRE_REAL > 10800)
			{
				LED_FRE_REAL = 8700;
			}
		}
	}
	else if (LED_SEEK_D == 0)
	{
		while (LED_FRE_REAL != sys_freq)
		{
			DispayF(--LED_FRE_REAL);
			if (LED_FRE_REAL < 8700)
			{
				LED_FRE_REAL = 10800;
			}
		}
	}
}

// 显示音量
void DispayVl()
{
	DisplayNUM(100, sys_vol / 10, sys_vol % 10, 100, 100);
}

// 显示信号质量
void DispayRSSI()
{
	uint8t NUM_GE, NUM_SHI, NUM_BAI;
	NUM_BAI = LED_RSSI / 100;
	NUM_SHI = (LED_RSSI % 100) / 10;
	NUM_GE = (LED_RSSI % 10);
	if (NUM_BAI)
	{
		DisplayNUM(0xFF, NUM_BAI, NUM_SHI, NUM_GE, 0xFF);
	}
	else
	{ // 百位不显示0，大多数情况下百位都达不到1
		DisplayNUM(0xFF, 0xFF, NUM_SHI, NUM_GE, 0xFF);
	}
}

void DispaySNR()
{
	uint8t NUM_GE, NUM_SHI;
	NUM_GE = (LED_SNR % 10);
	NUM_SHI = (LED_SNR % 100) / 10;
	DisplayNUM(23, 25, NUM_SHI, NUM_GE, 0xFF);
}

// 显示数字 内部使用
void DisplayNUM(uint8t a, b, c, d, dp)
{
	// 数码管显示位数轮询（0~3）
	static uint8t LED_POLLING_POSTITION = 0;
	uint8t code de = 1;
	uint8t sizeOfNixie = sizeof(NixieTable);

	if (LED_POLLING_POSTITION == 0)
	{
		if (a < sizeOfNixie)
		{
			P21 = P22 = P23 = 1;
			_74HC595_WriteByte(getData(a, dp == 1));
			P20 = 0;
			Delay(de);
		}
	}
	else if (LED_POLLING_POSTITION == 1)
	{
		if (b < sizeOfNixie)
		{
			P20 = P22 = P23 = 1;
			_74HC595_WriteByte(getData(b, dp == 2));
			P21 = 0;
			Delay(de);
		}
	}
	else if (LED_POLLING_POSTITION == 2)
	{

		if (c < sizeOfNixie)
		{
			P20 = P21 = P23 = 1;
			_74HC595_WriteByte(getData(c, dp == 3));
			P22 = 0;
			Delay(de);
		}
	}
	else if (LED_POLLING_POSTITION == 3)
	{
		if (d < sizeOfNixie)
		{

			P20 = P21 = P22 = 1;
			_74HC595_WriteByte(getData(d, dp == 4));
			P23 = 0;
			Delay(de);
		}
	}

	if (++LED_POLLING_POSTITION > 3)
	{
		LED_POLLING_POSTITION = 0;
	}
}

// 显示内容
void DISPLY()
{
	if (DISPLAY_type == 10)
		DispayFRE(); // 展示频率
	if (DISPLAY_type == 1)
		DispayVl(); // 音量
	if (DISPLAY_type == 2)
		DispayRSSI(); // 展示信号强度
	if (DISPLAY_type == 3)
		DispaySELLP(); // 展示睡眠模式
	if (DISPLAY_type == 14)
		DispaySNR(); // 展示SNR
}

void Led_CHANGE_SLEEP_MODE()
{

	CONF_CHANGE_SLEEP_MODE();
	DISPLAY_type = 3;
	resetSleepTime();
}

// 睡眠模式剩余时间重置
void resetSleepTime()
{
	if (sys_sleep_mode == 0)
	{
		LED_SELL_TIME = 0x1F40;
	}
}

// 判断是否显示
void Led_Loop()
{
	// 等于1 一直显示
	if (sys_sleep_mode)
	{
		DISPLY();
	}
	// 睡眠倒计时
	else if (LED_SELL_TIME > 10)
	{
		DISPLY();
		LED_SELL_TIME--;
	}
	// 不显示关闭数量管电源
	else
	{
		P20 = P21 = P22 = P23 = 1;
	}
}
