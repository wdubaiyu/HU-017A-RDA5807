#include <STC15.H>
#include <stdio.h>
#include "Delay.h"
#include "led/74HC595.h"
#include "led/myLed.h"
//,0xC7 -->L11  0x89-->H12
uint8t code NixieTable[] = {
	0xC0, 0xF9, 0xA4, 0xB0, 0x99,		// (0)0,1,2,3,4
	0x92, 0x82, 0xF8, 0x80, 0x90, 0x7F, // (5)5,6,7,8,9,点
	0x88, 0x83, 0xC6, 0xA1, 0x86,		// (11)A,b,C,d,E
	0x8E, 0x8C, 0xC1, 0xCE, 0x91,		// (16)F,P,U,T,y
	0x89, 0xC7, 0x12, 0xC8, 0xAB,		// (21)L,H,S,N,n
	0xF7};								//(26)_
uint8t code NixieTableDp[] = {
	0x40, 0x79, 0x24, 0x30, 0x19,
	0x12, 0x02, 0x78, 0x00, 0x10, 0x7F,
	0xC7, 0x89};

uint16t LED_FRE_REAL = 8700;
uint16t LED_DISPLAY_REC_COUNT = 0x00;
uint8t LED_DISPLAY_TYPE = 0x0A;
uint8t LED_RSSI = 0x00;
uint8t LED_SNR = 0x00;
uint8t LED_TIMED_STANDBY = 0x00;
// 手动搜台方向
bit LED_SEEK_D = 1;
// 睡眠模式剩余时间
uint16t LED_SHOW_TIME = 0x1F40; // 统计睡眠模式剩余时间8s

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

	// 正向  数码管频率在增加的效果
	if (LED_SEEK_D)
	{
		++LED_FRE_REAL;
		if (LED_FRE_REAL > 10800)
		{
			LED_FRE_REAL = 8700;
		}
	}
	else
	{
		--LED_FRE_REAL;
		if (LED_FRE_REAL < 8700)
		{
			LED_FRE_REAL = 10800;
		}
	}

	DispayF(LED_FRE_REAL);
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

void DispayTimedStandby()
{
	uint8t NUM_GE, NUM_SHI, NUM_BAI;
	NUM_BAI = LED_TIMED_STANDBY / 100;
	NUM_SHI = (LED_TIMED_STANDBY % 100) / 10;
	NUM_GE = (LED_TIMED_STANDBY % 10);
	if (NUM_BAI)
	{
		DisplayNUM(22, NUM_BAI, NUM_SHI, NUM_GE, 0xFF);
	}
	else
	{ // 百位不显示0，大多数情况下百位都达不到1
		DisplayNUM(22, 0xFF, NUM_SHI, NUM_GE, 0xFF);
	}
}

void DispalyConfirm()
{
	DisplayNUM(0xFF, 0xFF, 24, 0, 0xFF);
}

void DispalCancel()
{
	DisplayNUM(0xFF, 0xFF, 26, 26, 0xFF);
}

// 显示数字 内部使用
void DisplayNUM(uint8t a, b, c, d, dp)
{
	// 数码管显示位数轮询（0~3）
	static uint8t LED_POLLING_POSTITION = 0;
	uint8t code de = 0;
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
	if (LED_DISPLAY_TYPE == 10)
		DispayFRE(); // 展示频率
	if (LED_DISPLAY_TYPE == 0)
		DispalCancel(); // 显示OFF
	if (LED_DISPLAY_TYPE == 1)
		DispalyConfirm(); // 显示NO
	if (LED_DISPLAY_TYPE == 2)
		DispayRSSI(); // 展示信号强度
	if (LED_DISPLAY_TYPE == 3)
		DispaySELLP(); // 展示睡眠模式
	if (LED_DISPLAY_TYPE == 4)
		DispayVl(); // 音量

	if (LED_DISPLAY_TYPE == 101)
		DispaySNR(); // 展示SNR
	if (LED_DISPLAY_TYPE == 102)
		DispayTimedStandby(); // 展示定时待机
}

void LED_CHANGE_SLEEP_MODE()
{
	// 设置省电模式显示
	LED_SET_DISPLY_TYPE(3);
	sys_sleep_mode = ~sys_sleep_mode;
}

// 睡眠模式剩余时间重置
void LED_RESET_SLEEP_TIME()
{
	if (sys_sleep_mode == 0)
	{
		LED_SHOW_TIME = 0x1F40; // 8秒8000
	}
}

// 判断是否显示
void Led_Loop()
{
	// sys_sleep_mode>0一直显示
	if (sys_sleep_mode || LED_DISPLAY_TYPE > 100)
	{
		DISPLY();
	}
	// 睡眠倒计时
	else if (LED_SHOW_TIME > 10)
	{
		DISPLY();
		LED_SHOW_TIME--;
	}
	// 不显示关闭数量管电源
	else
	{
		P20 = P21 = P22 = P23 = 1;
	}
}

bit LED_DISPLY_NEET_REC()
{
	if (LED_DISPLAY_TYPE < 100)
	{
		return 1;
	}

	return 0;
}


void LED_SET_DISPLY_TYPE(uint8t display_type)
{
	LED_DISPLAY_REC_COUNT = 0x00;
	if (LED_DISPLAY_TYPE == display_type)
	{
		return;
	}

	// 需要持久化保存 音量
	if (display_type == 4)
	{
		sys_write_vol_flag = 1;
	}

	// 需要持久化保存 睡眠模式
	if (display_type == 3)
	{
		sys_write_sleep_flag = 1;
	}

	LED_DISPLAY_TYPE = display_type;
}

void LED_TIMED_STANDBY_U()
{
	if (LED_TIMED_STANDBY < 20)
	{
		LED_TIMED_STANDBY += 5;
	}
	else if (LED_TIMED_STANDBY < 60)
	{
		LED_TIMED_STANDBY += 10;
	}
	else if (LED_TIMED_STANDBY < 230)
	{
		LED_TIMED_STANDBY += 20;
	}
	LED_SET_DISPLY_TYPE(102);
}

void LED_TIMED_STANDBY_D()
{

	if (LED_TIMED_STANDBY > 60)
	{
		LED_TIMED_STANDBY -= 20;
	}
	else if (LED_TIMED_STANDBY > 20)
	{
		LED_TIMED_STANDBY -= 10;
	}
	else if (LED_TIMED_STANDBY >= 5)
	{
		LED_TIMED_STANDBY -= 5;
	}
	LED_SET_DISPLY_TYPE(102);
}