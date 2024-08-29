#include <Delay.h>
#include <intrins.h>

//不准确。按键显示已经调整好了。不改了
void Delay1ms()	
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

// // 这个跟准确
// void Delay1ms()		//@27.000MHz
// {
// 	unsigned char i, j;

// 	i = 27;
// 	j = 64;
// 	do
// 	{
// 		while (--j);
// 	} while (--i);
// }


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

void Delay_Ms_(unsigned int xms){
	Delay(xms);
}

void Delay_us_(unsigned int xms){
	while(xms--){
		Delay1us();
	}
}


