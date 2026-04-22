#include <Delay.h>
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


void Delay1us(void)	//@11.0592MHz
{
	_nop_();
	_nop_();
	_nop_();
}

void Delay_us_(unsigned int xms)
{
	while (xms--)
	{
		Delay1us();
	}
}



void Delay(unsigned int xms)
{
	while (xms--)
	{
		Delay1ms();
	}
}

