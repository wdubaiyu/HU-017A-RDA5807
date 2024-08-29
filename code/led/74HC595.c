#include <STC15.H>
#include "74HC595.h"
#include "Delay.h"


sbit RCK=P3^3;		//RCLK P33 74HC595_12  全部移出去
sbit SCK=P3^4;		//SCLK P34 74HC595_11  向下移动
sbit SER=P3^2;		//SER  P32 74HC595_14

/**
	* 重高到底移出去
  * @brief  74HC595写入一个字节
  * @param  Byte 要写入的字节
  * @retval 无
  */
void _74HC595_WriteByte(unsigned char Byte)
{
	unsigned char i;
	for(i=0;i<8;i++)
	{
		SER=Byte&(0x80>>i);
		SCK=1;
		SCK=0;
	}
	RCK=1;
	RCK=0;
}