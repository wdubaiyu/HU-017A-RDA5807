#include <Delay.h>
#include <intrins.h>

void Delay1ms()		//@33.1776MHz
{
	unsigned char i, j;

	_nop_();
	_nop_();
	i = 33;
	j = 66;
	do
	{
		while (--j);
	} while (--i);
}



void Delay1us()		//@27.000MHz
{
	unsigned char i;

	_nop_();
	i = 4;
	while (--i);
}


void Delay(unsigned int xms)
{
	while(xms--){
		Delay1ms();
	}
}

void Delay_us_(unsigned int xms){
	while(xms--){
		Delay1us();
	}
}


