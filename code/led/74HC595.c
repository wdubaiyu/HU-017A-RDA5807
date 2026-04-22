#include "stc15.h"
#include "74HC595.h"
#include "Delay.h"

sbit RCK = P3 ^ 3; // RCLK P33 74HC595_12  全部移出去(存储时钟)
sbit SCK = P3 ^ 4; // SCLK P34 74HC595_11  向下移动(移位时钟)
sbit SER = P3 ^ 2; // SER  P32 74HC595_14  串行数据输入（DS）

/**
 * 重高到底移出去
 * ### 74HC595时序要求：
 * - 数据建立时间（t_su）：最小20ns（对于74HC595，典型值）
 * - 时钟高电平时间（t_high）：最小13ns
 * - 锁存脉冲宽度(t_latch)	>25ns
 * @brief  74HC595写入一个字节
 * @param  byte_p 要写入的字节
 * @retval 无
 */
void _74HC595_WriteByte(unsigned char byte_p)
{
	unsigned char i;
	SCK = 0;
	Delay_us_(25);
	for (i = 0; i < 8; i++)
	{
		SER = byte_p & (0x80 >> i);
		_nop_();  // 30ns数据建立 @33.1776MHz
		SCK = 1;
		_nop_();  // 稳定时间 (30ns) @33.1776MHz
		SCK = 0;
	}
	RCK = 1;
	_nop_(); _nop_();  // 锁存脉冲宽度 (60ns > 25ns) @33.1776MHz
	RCK = 0;
}
