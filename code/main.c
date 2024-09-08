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
uint8t key_function_flag;

// 电源状态（0正常，1关机倒计时，2已关机）
uint8t POWER_STATUS;

// 需要系统写
bit conf_write_flag;

void triggerWriteFreq()
{
	sys_write_freq_flag = 1;
	CONF_WRITE();
	conf_write_flag = 0;
}

// 按键触发功能
void userInput(uint8t Key_num)
{
	// 有按键操作时重置睡眠时间
	LED_RESET_SLEEP_TIME();

	if (key_function_flag > 10) // 功能菜单触发后不响应基础操作
	{
		// 触发功能后按了V+
		if (Key_num == 1)
		{
			if (key_function_flag == 11)
			{
				LED_SNR = RDA5807M_Read_SNR();
				if (LED_SNR == 15)
				{
					LED_SNR = 0;
				}
				else
				{
					LED_SNR++;
				}
				RDA5807M_Set_SNR(LED_SNR);
			}

			if (key_function_flag == 21)
			{
				LED_TIMED_STANDBY_U();
			}

			return;
		}

		// 触发功能后按了V-
		if (key_function_flag > 10 && Key_num == 2)
		{
			if (key_function_flag == 11)
			{
				LED_SNR = RDA5807M_Read_SNR();
				if (LED_SNR == 0)
				{
					LED_SNR = 15;
				}
				else
				{
					LED_SNR--;
				}
				RDA5807M_Set_SNR(LED_SNR);
			}

			if (key_function_flag == 21)
			{
				LED_TIMED_STANDBY_D();
			}

			return;
		}

		// 触发功能后按了F+  (取消)
		if (key_function_flag > 10 && Key_num == 3)
		{
			if (key_function_flag == 11)
			{
			}

			if (key_function_flag == 21)
			{
				POWER_STATUS = 0;
			}

			// 取消功能直接展示频率
			LED_SET_DISPLY_TYPE(0);
			key_function_flag = 0;
			return;
		}

		// 触发功能后按了F- （确认）
		if (key_function_flag > 10 && Key_num == 4)
		{
			if (key_function_flag == 11)
			{
				RDA5807M_Search_Automatic();
			}

			if (key_function_flag == 21)
			{
				// 定时关机大于0才能确认
				if (LED_TIMED_STANDBY > 0)
				{
					LED_SET_DISPLY_TYPE(1);
					POWER_STATUS = 1;
				}
				else
				{
					LED_SET_DISPLY_TYPE(0);
					POWER_STATUS = 0;
				}
			}

			key_function_flag = 0;
			return;
		}

		// 如果触发了功能则不进行下面的 操作
		return;
	}

	/**
	 * 下面是功能出发区域
	 */
	// K12 设置省电模式（一定时间后关闭数码管）
	if (Key_num == 12)
	{
		LED_CHANGE_SLEEP_MODE();
		return;
	}

	// K11 设置定时关机
	if (Key_num == 11)
	{
		key_function_flag = 21;
		// 设置定时关机时，取消上一次设定
		POWER_STATUS = 0;
		LED_SET_DISPLY_TYPE(102);
		return;
	}

	// K13 自动搜台 触发
	if (Key_num == 13)
	{
		LED_SNR = RDA5807M_Read_SNR();
		key_function_flag = 11;
		LED_SET_DISPLY_TYPE(101); // 显示snr设置
		return;
	}

	// K14 切换rssi显示
	if (Key_num == 14)
	{
		LED_RSSI = RDA5807M_Read_RSSI();
		LED_SET_DISPLY_TYPE(2);
		return;
	}

	// K33 手动搜下一个台
	if (Key_num == 33)
	{
		sys_freq = RDA5807M_Seek(1);
		return;
	}

	// K44 手动搜上一个台
	if (Key_num == 44)
	{
		sys_freq = RDA5807M_Seek(0);
		return;
	}

	// K22 V-长按
	if (Key_num == 22)
	{
		LED_SET_DISPLY_TYPE(4); // 数码管显示音量
		RDA5807M_CHANGE_MUTE();
		return;
	}

	// V+
	if (Key_num == 1)
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
	if (Key_num == 2)
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
	if (Key_num == 3)
	{

		if (sys_radio_index == sys_radio_index_max)
		{
			sys_radio_index = 0;
		}
		else
		{
			++sys_radio_index;
		}
		LED_FRE_REAL = CONF_GET_RADIO_INDEX(sys_radio_index);
		RDA5807M_Set_Freq(LED_FRE_REAL);
		triggerWriteFreq();
		return;
	}
	// F-
	if (Key_num == 4)
	{

		if (sys_radio_index == 0)
		{
			sys_radio_index = sys_radio_index_max;
		}
		else
		{
			--sys_radio_index;
		}

		LED_FRE_REAL = CONF_GET_RADIO_INDEX(sys_radio_index);
		RDA5807M_Set_Freq(LED_FRE_REAL);
		// 需要写立即写配置
		triggerWriteFreq();
		return;
	}
}

/**
 * 启动收音函数
 */
void InitSystem()
{
	key_function_flag = 0x00;
	POWER_STATUS = 0x00;
	LED_TIMED_STANDBY = 0x00;
	// 初始化收音机
	RDA5807M_init();
	// 打开数码管显示、键盘轮询
	LED_RESET_SLEEP_TIME();
	Timer0Init();
	Delay(8);

	if (CONF_SYS_INIT()) // 加载上一次系统配置,返回是否需要自动搜台
	{
		RDA5807M_Search_Automatic();
	}
	else
	{ // 播放上次关机时的电台
		LED_FRE_REAL = sys_freq;
		RDA5807M_Set_Freq(sys_freq);
	}

	// 设置系统音量
	RDA5807M_Set_Volume(sys_vol);
}

void main()
{
	uint8t Key_num;
	// 初始化串口
	// UartInit();
	InitSystem();
	// printf("power on \r\n");
	while (1)
	{
		// 读取用户按键输入
		Key_num = POP_KEY();

		// 在关机中
		if (POWER_STATUS == 2)
		{
			// 关机中长按F-开机
			if (Key_num == 44)
			{
				InitSystem();
			}

			continue;
		}
		// 关机时间到
		if (POWER_STATUS == 1 && LED_TIMED_STANDBY < 1)
		{
			POWER_STATUS = 2;
			RDA5807M_OFF();
			P20 = P21 = P22 = P23 = 1; // 关闭数码管
			continue;
		}

		// 按键有效,响应用户操作
		if (Key_num)
		{
			userInput(Key_num);
		}

		// 主程序持久化触发（保护音量和睡眠模式）
		if (conf_write_flag)
		{
			CONF_WRITE();
			conf_write_flag = 0;
		}
	}
}

/**
 * 定时器零的中断函数
 */
void Timer0_Rountine(void) interrupt 1
{
	static uint16t timed_stanby_count;
	// 不是关机状态才显示数码管
	if (POWER_STATUS < 2)
	{
		Led_Loop();
	}

	Key_Loop();

	// 是否需要显示恢复为频率
	if (LED_NEET_DISPLY_REC())
	{
		if (++LED_DISPLAY_TYPE_REC >= 0x7D0)
		{
			LED_SET_DISPLY_TYPE(10);
			// 数码管恢复显示时触发写操作
			conf_write_flag = 1;
		}
	}
	// 定时关机功能开启,循环减时间
	if (POWER_STATUS == 1)
	{
		if (++timed_stanby_count > 60000)
		{
			LED_TIMED_STANDBY -= 1;
			timed_stanby_count = 0;
		}
	}

	TL0 = 0x88; // 设置定时初值
	TH0 = 0x96; // 设置定时初值
	TF0 = 0;	// 清除TF0标志,进入下一次计时
}
