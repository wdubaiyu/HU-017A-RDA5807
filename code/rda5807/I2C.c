#include <stc15.h>
#include "config/Config.h"
#include "Delay.h"
#include "I2C.h"

sbit SDA = P2^4;	//这一个数据线
sbit SCL = P2^5;	//时钟线

/**
 * @brief 一段延迟
 */
void I2C_Delay()
{
    // int z = 0xff;
    // while (z--)  ;
	Delay_us_(10);
}
/**
 * @brief 产生I2C起始信号
 * @param 无
 * @return 无
 * @author HZ12138
 * @date 2022-07-27 08:54:48
 */
void I2C_Start(void)
{
    SDA = 1;        //需在SCL之前设定
    SCL = 1;        //硬件进入SDA检测状态
    I2C_Delay();    //延时至少4.7us
    SDA = 0;        //SDA由1->0,产生开始信号
    I2C_Delay();    //延时至少4us
    SCL = 0;        //SCL变为无效
}
/**
 * @brief 产生I2C结束信号
 * @param 无
 * @return 无
 * @author HZ12138
 * @date 2022-07-27 08:57:03
 */
void I2C_End(void)
{
    SDA = 0;        //在SCL之前拉低
    SCL = 1;        //硬件进入SDA检测状态
    I2C_Delay();    //至少延时4us
    SDA = 1;        //SDA由0->1,产生结束信号
    I2C_Delay();    //延时至少4.7us
}

/**
 * @brief主机向从机发送一个应答信号
 * @param 1应答；0非应答
 * @return 无
 * @author HZ12138
 * @date 2022-07-27 08:57:03
 */
void IIC_Send_ACK(uint8t ack)
{
    SDA = ack ;      //产生应答电平
	I2C_Delay();
    SCL = 1;        //发送应答信号
    I2C_Delay();    //延时至少4us
    SCL = 0;        //整个期间保持应答信号
}
/**
 * @brief 接受应答码
 * @param 无
 * @return 应答码 0 应答 1 不应达
 * @author HZ12138
 * @date 2022-07-27 09:04:28
 */
uint8t IIC_Get_ACK(void)
{
    bit ret;        //用来接收返回值
    SDA = 1;        //电阻上拉,进入读
	I2C_Delay();
    SCL = 1;        //进入应答检测
    I2C_Delay();    //至少延时4us
    ret = SDA;      //保存应答信号
    SCL = 0;
    return (ret);
}

/**
 * @brief I2C写1Byte
 * @param dat:1Byte数据
 * @return 应答结果 0 应答 1 不应达
 * @author HZ12138
 * @date 2022-07-27 09:05:14
 */
uint8t I2C_SendByte(uint8t dat)
{
	bit ack;
    uint8t loop = 8;     //必须为一个字节
    while(loop--){
        // 高在前低在后
		if (dat & 0x80)
			SDA = 1;
		else
			SDA = 0;
        I2C_Delay();
				SCL = 1;
        I2C_Delay();    //延时至少4us
        SCL = 0;
        dat <<= 1;      //低位向高位移动
    }
	
	ack = IIC_Get_ACK();
	
	return (ack);
}
/**
 * @brief I2C读取1Byte数据
 * @param ack:应答 0 应答 1 不应达
 * @return 接受到的数据
 * @author HZ12138
 * @date 2022-07-27 09:06:13
 */
uint8t I2C_ReadByte(uint8t ack)
{
    uint8t loop = 8;     //必须为一个字节
    uint8t ret = 0;
	// SDA 设置输入方向
    SDA = 1;
    while(loop--){
		ret <<= 1;
		SCL = 1;
		I2C_Delay();
        // 高在前低在后
        if(SDA){
			ret++;
		}
        SCL = 0;
        I2C_Delay();
    }
	
	IIC_Send_ACK(ack);
	
    return ret;
}
