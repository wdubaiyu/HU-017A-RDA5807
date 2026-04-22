#ifndef _I2C_H_
#define _I2C_H_

#include <intrins.h>

void I2C_Start(void);
void I2C_End(void);
void IIC_Send_ACK(uint8_t ack);
uint8_t IIC_Get_ACK(void);
uint8_t I2C_SendByte(uint8_t dat);
uint8_t I2C_ReadByte(uint8_t ack);

#endif
