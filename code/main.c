#include <STC15.H>
#include <stdio.h>
#include "Delay.h"
#include "UART.h"
#include "config/Config.h"
#include "config/EEPROM.h"
#include "rda5807/RDA5807M.h"
#include "led/myLed.h"
#include "time0/time0.h"
#include "key/key.h"

// 搜索触发==1，触发时睡眠模式==11
uint8t search_SELLP_flag = 0;

// 按键触发功能
void userInput()
{
	uint8t snr = 6;
	// 获取按键值，获取后将按键值重置为0;
	uint8t Key_NUM = POP_KEY();
	if (!Key_NUM) // 用户没有输入
	{
		return;
	}

	// if (Key_NUM)
	// {
	// printf("key:%bd\r\n", Key_NUM);
	// }

	// K12 设置省电模式（一定时间后关闭数码管）
	if (Key_NUM == 12)
	{
		Led_CHANGE_SLEEP_MODE();
	}

	// 自动搜台+snr
	if (search_SELLP_flag > 0 && Key_NUM == 1)
	{
		snr = RDA5807M_Read_SNR();
		if (snr == 15)
		{
			snr = 0;
		}
		else
		{
			snr++;
		}
		RDA5807M_Set_SNR(snr);
		// 设置显示屏幕
		LED_SNR = snr;
		return;
	}

	// 自动搜台-snr
	if (search_SELLP_flag > 0 && Key_NUM == 2)
	{
		snr = RDA5807M_Read_SNR();
		if (snr == 0)
		{
			snr = 15;
		}
		else
		{
			snr--;
		}
		RDA5807M_Set_SNR(snr);
		// 设置显示屏幕
		LED_SNR = snr;
		return;
	}

	// 自动搜台 取消
	if (search_SELLP_flag > 0 && Key_NUM == 3)
	{
		search_SELLP_flag = 0;
		DISPLAY_type = 10;
		if (search_SELLP_flag == 11)
		{
			sys_sleep_mode = 0;
		}
		return;
	}

	// 自动搜台 确认开始
	if (search_SELLP_flag > 0 && Key_NUM == 4)
	{
		// 数码管显示频率
		DISPLAY_type = 10;
		LED_HAND_MARK = 0; // 数码管设置为搜台模式
		// 开始自动搜台
		RDA5807M_Search_Automatic();
		LED_FRE_REAL = sys_freq;
		search_SELLP_flag = 0;
		if (search_SELLP_flag == 11)
		{
			sys_sleep_mode = 0;
		}
		LED_HAND_MARK = 1; //  数码管设置为列表换台
		return;
	}

	// K13 自动搜台 触发
	if (Key_NUM == 13)
	{
		// printf("auto serach radio ?\r\n");
		LED_SNR = RDA5807M_Read_SNR();
		search_SELLP_flag = 1;
		DISPLAY_type = 14; // 显示snr设置
		if (!sys_sleep_mode)
		{
			sys_sleep_mode = 1;
			search_SELLP_flag = 11;
		}

		return;
	}

	// K14 切换rssi显示
	if (Key_NUM == 14)
	{
		LED_RSSI = RDA5807M_Read_RSSI();
		DISPLAY_type = 0x02;
		return;
	}

	// K33 手动搜下一个台
	if (Key_NUM == 33)
	{
		LED_SEEK_D = 1;	   // 数码频率改变方向
		DISPLAY_type = 10; // 数码管显示频率
		LED_HAND_MARK = 0; // 手动搜台
		resetSleepTime();  // 数码重置熄灭时间
		sys_freq = RDA5807M_Seek(1);
		return;
	}

	// K44 手动搜上一个台
	if (Key_NUM == 44)
	{
		LED_SEEK_D = 0;	   // 数码频率改变方向
		DISPLAY_type = 10; // 数码管显示频率
		LED_HAND_MARK = 0; // 手动搜台
		resetSleepTime();  // 数码重置熄灭时间
		sys_freq = RDA5807M_Seek(0);
		return;
	}

	// K22 V-长按
	if (Key_NUM == 22)
	{
		DISPLAY_type = 1; // 数码管显示音量
		resetSleepTime(); // 数码重置熄灭时间
		RDA5807M_CHANGE_MUTE();
		return;
	}

	// V+
	if (Key_NUM == 1)
	{
		DISPLAY_type = 1; // 数码管显示音量
		resetSleepTime(); // 数码重置熄灭时间
		// 最大音量15
		if (sys_vol + 1 < 16)
		{
			RDA5807M_Set_Volume(sys_vol + 1);
		}
		return;
	}
	// V-
	if (Key_NUM == 2)
	{
		DISPLAY_type = 1; // 数码管显示音量
		resetSleepTime(); // 数码重置熄灭时间
		// 最小音量1
		if (sys_vol > 0)
		{
			RDA5807M_Set_Volume(sys_vol - 1);
		}
		return;
	}
	// F+
	if (Key_NUM == 3)
	{

		if (sys_radio_index == sys_radio_index_max)
		{
			sys_radio_index = 0;
		}
		else
		{
			++sys_radio_index;
		}

		LED_SEEK_D = 1;	   // 数码频率改变方向
		DISPLAY_type = 10; // 数码管展示频率
		LED_HAND_MARK = 1; // 切换列表台
		resetSleepTime();  // 数码重置熄灭时间
		RDA5807M_Set_Freq(CONF_GET_RADIO_INDEX(sys_radio_index));
		return;
	}
	// F-
	if (Key_NUM == 4)
	{

		if (sys_radio_index == 0)
		{
			sys_radio_index = sys_radio_index_max;
		}
		else
		{
			--sys_radio_index;
		}

		LED_SEEK_D = 0;	   // 数码频率改变方向
		DISPLAY_type = 10; // 数码管展示频率
		LED_HAND_MARK = 1; // 切换列表台
		resetSleepTime();  // 数码重置熄灭时间
		RDA5807M_Set_Freq(CONF_GET_RADIO_INDEX(sys_radio_index));
		return;
	}
}

void main()
{
	// // 初始化串口
	// UartInit();
	// 初始化收音机
	RDA5807M_init();
	Delay(10);
	LED_FRE_REAL = sys_freq;

	// 打开数码管显示、键盘轮询
	Timer0Init();

	if (CONF_SYS_INIT()) // 加载上一次系统配置,返回是否需要自动搜台
	{
		if (!sys_sleep_mode)
		{
			sys_sleep_mode = 1;
			search_SELLP_flag == 11;
		}

		RDA5807M_Search_Automatic();
		LED_FRE_REAL = sys_freq;
		if (search_SELLP_flag == 11)
		{
			sys_sleep_mode = 0;
		}
	}
	else
	{
		RDA5807M_Set_Freq(sys_freq);
	}

	// 设置系统音量
	RDA5807M_Set_Volume(sys_vol);

	// printf("setup complete\r\n");

	while (1)
	{
		userInput();
	}
}

/**
 * 定时器零的中断函数
 */
void Timer0_Rountine(void) interrupt 1
{
	// 循环次数记数
	static uint16t T0Count1, T0Count2;
	Led_Loop();
	Key_Loop();

	// 不是显示在频率模式中,开始记录次数（大概 4ms*1000）后显示频率
	if (DISPLAY_type < 10)
	{ // 检查是否需要重置标记位

		if (++T0Count2 >= 4000)
		{
			T0Count2 = 0;
			DISPLAY_type = 10;
		}
	}

	TL0 = 0x88; // 设置定时初值
	TH0 = 0x96; // 设置定时初值
	TF0 = 0;	// 清除TF0标志
}
