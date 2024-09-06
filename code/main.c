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

// （自动搜台设置==11）
// （定时关机设置==21）
uint8t key_function_flag = 0;
bit timed_stanby_status = 0;

void autoRadio()
{
	RDA5807M_Search_Automatic();
	LED_FRE_REAL = sys_freq;
	LED_HAND_MARK = 1; //  数码管设置为列表换台
	key_function_flag = 0;
}

// 按键触发功能
void userInput(uint8t Key_NUM)
{
	uint8t snr = 6;
	// 有按键操作时重置睡眠时间
	resetSleepTime();

	// printf("key:%bd\r\n", Key_NUM);

	// K12 设置省电模式（一定时间后关闭数码管）
	if (Key_NUM == 12)
	{
		LED_CHANGE_SLEEP_MODE();
		return;
	}

	// K11 设置定时关机
	if (Key_NUM == 11)
	{
		key_function_flag = 21;
		// 设置定时关机时，取消上一次设定
		timed_stanby_status = 0;
		LED_SET_DISPLY_TYPE(102);
		return;
	}

	// 自动搜台+snr
	if (key_function_flag > 10 && Key_NUM == 1)
	{
		if (key_function_flag == 11)
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
		}
		if (key_function_flag == 21)
		{
			LED_TIMED_STANDBY_U();
		}

		return;
	}

	// 自动搜台-snr
	if (key_function_flag > 10 && Key_NUM == 2)
	{
		if (key_function_flag == 11)
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
		}

		if (key_function_flag == 21)
		{
			LED_TIMED_STANDBY_D();
		}

		return;
	}

	// 功能取消
	if (key_function_flag > 10 && Key_NUM == 3)
	{
		if (key_function_flag == 11)
		{

			if (key_function_flag == 12)
			{
				sys_sleep_mode = 0;
			}
		}

		if (key_function_flag == 21)
		{
			timed_stanby_status = 0;
		}

		// 取消功能直接展示频率
		LED_SET_DISPLY_TYPE(0);
		key_function_flag = 0;
		return;
	}

	// 功能确认
	if (key_function_flag > 10 && Key_NUM == 4)
	{
		if (key_function_flag == 11)
		{
			// 开始自动搜台
			autoRadio();
		}

		if (key_function_flag == 21)
		{
			// 定时关机大于0才能确认
			if (LED_TIMED_STANDBY > 0)
			{
				LED_SET_DISPLY_TYPE(1);
				timed_stanby_status = 1;
			}
			else
			{
				LED_SET_DISPLY_TYPE(0);
				timed_stanby_status = 0;
			}
		}

		key_function_flag = 0;
		return;
	}

	// K13 自动搜台 触发
	if (Key_NUM == 13)
	{
		// printf("auto serach radio ?\r\n");
		LED_SNR = RDA5807M_Read_SNR();
		key_function_flag = 11;
		LED_SET_DISPLY_TYPE(101); // 显示snr设置
		return;
	}

	// K14 切换rssi显示
	if (Key_NUM == 14)
	{
		LED_RSSI = RDA5807M_Read_RSSI();
		LED_SET_DISPLY_TYPE(2);
		return;
	}

	// K33 手动搜下一个台
	if (Key_NUM == 33)
	{
		LED_SEEK_D = 1;	   // 数码频率改变方向
		LED_HAND_MARK = 0; // 手动搜台
		sys_freq = RDA5807M_Seek(1);
		return;
	}

	// K44 手动搜上一个台
	if (Key_NUM == 44)
	{
		LED_SEEK_D = 0;	   // 数码频率改变方向
		LED_HAND_MARK = 0; // 手动搜台
		sys_freq = RDA5807M_Seek(0);
		return;
	}

	// K22 V-长按
	if (Key_NUM == 22)
	{
		LED_SET_DISPLY_TYPE(4); // 数码管显示音量
		RDA5807M_CHANGE_MUTE();
		return;
	}

	// V+
	if (Key_NUM == 1)
	{
		LED_SET_DISPLY_TYPE(4); // 数码管显示音量
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
		LED_SET_DISPLY_TYPE(4); // 数码管显示音量
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
		LED_HAND_MARK = 1; // 切换列表台
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

		LED_HAND_MARK = 1; // 切换列表台
		RDA5807M_Set_Freq(CONF_GET_RADIO_INDEX(sys_radio_index));
		return;
	}
}

void main()
{
	uint8t Key_NUM;
	// // 初始化串口
	// UartInit();
	// 初始化收音机
	RDA5807M_init();
	// 打开数码管显示、键盘轮询
	Timer0Init();
	Delay(8);

	if (CONF_SYS_INIT()) // 加载上一次系统配置,返回是否需要自动搜台
	{
		autoRadio();
	}
	else
	{ // 播放上次关机时的电台
		RDA5807M_Set_Freq(sys_freq);
	}

	// 设置系统音量
	RDA5807M_Set_Volume(sys_vol);

	while (1)
	{
		if (timed_stanby_status && LED_TIMED_STANDBY < 1)
		{
			RDA5807M_OFF();
			TR0 = 0; // 定时器0关闭
			ET0 = 0; // 关闭中断允许寄存器
			EA = 0;	 // 关闭中断控制总开关

			P20 = P21 = P22 = P23 = 1;// 关闭数码管
		}
		else
		{
			// 循环按键操作
			Key_NUM = POP_KEY();
			if (Key_NUM)
			{
				userInput(Key_NUM);
			}
		}
	}
}

/**
 * 定时器零的中断函数
 */
void Timer0_Rountine(void) interrupt 1
{
	static uint16t timed_stanby_count;
	Led_Loop();
	Key_Loop();

	// 还原显示类型
	if (LED_NEET_DISPLY_REC())
	{
		if (++LED_DISPLAY_TYPE_REC >= 0x7D0)
		{
			LED_DISPLAY_TYPE_REC = 0;
			LED_SET_DISPLY_TYPE(10);
		}
	}

	if (timed_stanby_status)
	{
		if (++timed_stanby_count > 60000)
		{
			LED_TIMED_STANDBY -= 1;
			timed_stanby_count = 0;
		}
	}

	TL0 = 0x88; // 设置定时初值
	TH0 = 0x96; // 设置定时初值
	TF0 = 0;	// 清除TF0标志
}
