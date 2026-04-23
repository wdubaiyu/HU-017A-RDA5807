#include "Delay.h"
#include <intrins.h>

void Delay1ms(void)	//@11.0592MHz
{
	unsigned char data i, j;

	_nop_();
	_nop_();
	_nop_();
	i = 11;
	j = 190;
	do
	{
		while (--j);
	} while (--i);
}



void Delay_us_(unsigned int xus)
{
	while (xus--)
	{
		_nop_();
		_nop_();
		_nop_();
	}
}

void Delay(unsigned int xms)
{
	while (xms--)
	{
		Delay1ms();
	}
}
