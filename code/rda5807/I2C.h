#ifndef _I2C_H_
#define _I2C_H_

#include <intrins.h>

void I2C_Start(void);
void I2C_End(void);
void IIC_Send_ACK(uint8t ack);
uint8t IIC_Get_ACK(void);
uint8t I2C_SendByte(uint8t dat);
uint8t I2C_ReadByte(uint8t ack);

#endif
