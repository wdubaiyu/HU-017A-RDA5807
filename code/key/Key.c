#include <STC15.H>
#include <stdio.h>
#include "Key.h"

unsigned char KeyNum = 0;

sbit KEY1 = P1 ^ 6; // v+
sbit KEY2 = P1 ^ 7; // v-
sbit KEY3 = P1 ^ 5; // f+
sbit KEY4 = P1 ^ 4; // f-

unsigned char k1_lock_flage = 0;
unsigned char k2_lock_flage = 0;
unsigned char k3_lock_flage = 0;
unsigned char k4_lock_flage = 0;

unsigned int k1_cnt = 0; // 单击按键基数
unsigned int k2_cnt = 0; // 单击按键基数
unsigned int k3_cnt = 0; // 单击按键基数
unsigned int k4_cnt = 0; // 单击按键基数

unsigned char k1_short_flag = 0; // 短按标志
unsigned char k2_short_flag = 0; // 短按标志
unsigned char k3_short_flag = 0; // 短按标志
unsigned char k4_short_flag = 0; // 短按标志

unsigned char k1_long_flag = 0; // 长按标志
unsigned char k2_long_flag = 0; // 长按标志

unsigned char combination_12 = 0; // 组合按键按下标识
unsigned char combination_13 = 0; // 组合按键按下标识
unsigned char combination_14 = 0; // 组合按键按下标识

/**
 * @brief  获取按键键码(获取清零)
 * @param  无
 * @retval 按下按键的键码，范围：0,1~4,0表示无按键按下
 */
unsigned char POP_KEY(void)
{
	unsigned char temp = 0;
	temp = KeyNum;
	KeyNum = 0;
	return temp;
}

/**
 *  按键驱动函数，在中断中调用
 */
void Key_Loop(void)
{
	if (KEY1) // v+按键松开
	{

		k1_cnt = 0;
		if (k1_short_flag)
		{
			k1_short_flag = 0;
			KeyNum = 1;
		}
		if (k1_long_flag) // 在key1 长按的基础上 按住了key3
		{
			if (combination_12)
			{
				combination_12 = 0;
			}
			else if (combination_13)
			{
				combination_13 = 0;
			}
			else if (combination_14)
			{
				combination_14 = 0;
			}
			else
			{
				KeyNum = 11;
			}
			k1_long_flag = 0;
		}

		k1_lock_flage = 0; // 清除自锁标志
	}
	else if (!k1_lock_flage && !combination_12 && !combination_13 && !combination_14) // 按键持续按下
	{
		if (++k1_cnt > KEY_DELAY_TIME)
		{
			k1_short_flag = 1;
		}
		if (++k1_cnt > KEY_LONG_TIME)
		{
			if (!KEY2)
			{
				k1_lock_flage = 1; // 自锁防止再次进入
				k1_long_flag = 1;
				k1_short_flag = 0; // 清空短按
				combination_12 = 1;
				KeyNum = 12;
			}
			else if (!KEY3)
			{
				k1_lock_flage = 1; // 自锁防止再次进入
				k1_long_flag = 1;
				k1_short_flag = 0; // 清空短按
				combination_13 = 1;
				KeyNum = 13;
			}
			else if (!KEY4)
			{
				k1_lock_flage = 1; // 自锁防止再次进入
				k1_long_flag = 1;
				k1_short_flag = 0; // 清空短按
				combination_14 = 1;
				KeyNum = 14;
			}
			else
			{
				k1_short_flag = 0;
				k1_long_flag = 1;
			}
		}
	}
	// 下面的按键长按足够时间后就生效，key1长按足够要松开才生效（为了兼容两个键长按）
	if (KEY2) // v+按键松开
	{
		if (k2_short_flag)
		{
			k2_short_flag = 0;
			KeyNum = 2;
		}
		k2_lock_flage = 0; // 清除自锁标志
		k2_cnt = 0;
	}
	else if (!k2_lock_flage && !k1_short_flag && !k1_long_flag && !combination_12 && !combination_13 && !combination_14) // 按键持续按下
	{
		if (++k2_cnt > KEY_DELAY_TIME)
		{
			k2_short_flag = 2;
		}
		if (++k2_cnt > KEY_LONG_TIME)
		{
			k2_short_flag = 0;
			k2_lock_flage = 2;
			KeyNum = 22;
		}
	}

	///////////////////////////////////////////////////////////////
	if (KEY3) // v+按键松开
	{
		if (k3_short_flag)
		{
			k3_short_flag = 0;
			KeyNum = 3;
		}
		k3_lock_flage = 0; // 清除自锁标志
		k3_cnt = 0;
	}
	else if (!k3_lock_flage && !k1_short_flag && !k1_long_flag && !combination_12 && !combination_13 && !combination_14) // 按键持续按下
	{
		if (++k3_cnt > KEY_DELAY_TIME)
		{
			k3_short_flag = 1;
		}
		if (++k3_cnt > KEY_LONG_TIME)
		{
			k3_short_flag = 0;
			k3_lock_flage = 1;
			KeyNum = 33;
		}
	}

	if (KEY4) // v+按键松开
	{
		if (k4_short_flag)
		{
			k4_short_flag = 0;
			KeyNum = 4;
		}
		k4_lock_flage = 0; // 清除自锁标志
		k4_cnt = 0;
	}
	else if (!k4_lock_flage && !k1_short_flag && !k1_long_flag && !combination_12 && !combination_13 && !combination_14) // 按键持续按下
	{
		if (++k4_cnt > KEY_DELAY_TIME)
		{
			k4_short_flag = 1;
		}
		if (++k4_cnt > KEY_LONG_TIME)
		{
			k4_short_flag = 0;
			k4_lock_flage = 1;
			KeyNum = 44;
		}
	}
}