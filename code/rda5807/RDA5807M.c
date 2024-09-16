#include <intrins.h>
#include <stdio.h>
#include <stc15.h>
#include "Delay.h"
#include "RDA5807M.h"
#include "I2C.h"
#include "led/myLed.h"

uint8t MUTE_STATUS = 0;

/**
 * 写寄存器 16bit
 */
void RDA5807M_Write_Reg(uint8t Address, uint16t Data)
{
    uint8t Buf[2] = {0};
    Buf[0] = (Data & 0xff00) >> 8; // 高位
    Buf[1] = Data & 0x00ff;        // 低位

    I2C_Start();
    I2C_SendByte(0x22);
    I2C_SendByte(Address);
    I2C_SendByte(Buf[0]);
    I2C_SendByte(Buf[1]);
    I2C_End();
}

/**
 * @brief 读寄存器
 * @param Address:寄存器地址
 * @return 读取的数据（16bit）
 */
uint16t RDA5807M_Read_Reg(uint8t Address)
{
    uint8t Buf[2] = {0};

    I2C_Start();
    I2C_SendByte(0x22);
    I2C_SendByte(Address);
    I2C_Start();
    I2C_SendByte(0x23);
    Buf[0] = I2C_ReadByte(0);
    Buf[1] = I2C_ReadByte(1);
    I2C_End();

    return ((Buf[0] << 8) | Buf[1]);
}

/**
 * @brief init
 * @param 无
 */
void RDA5807M_init(void)
{
    RDA5807M_Write_Reg(0x02, 0x0002); // reset
    Delay(50);
    RDA5807M_Write_Reg(0x02, 0xc001);
    Delay(50);
    RDA5807M_Write_Reg(0x03, 0x0010 | ((sys_freq - 8700) / 10) << 6); // space 00 100kHz band 00 87-108MHz  （中国band）
    RDA5807M_Write_Reg(0x04, 0x0400);
    RDA5807M_Write_Reg(0x05, 0x86a0 | sys_vol); // seek SNR 0110  --> 6
    RDA5807M_Write_Reg(0x06, 0x8000);
    RDA5807M_Write_Reg(0x07, 0x5F1A);
    LED_FRE_REAL = sys_freq;
}

/**
 * @brief 将频率转为信道值
 * @param Freq:频率(以MHz为单位*100)(如108MHz=>10800)
 * @return 转换为的信道值

 * @date 2022-07-21 22:01:08
 */
uint16t RDA5807M_FreqToChan(uint16t Freq)
{
    uint16t Start = 0; // 频率开始
    uint16t End = 0;   // 频率结束
    uint16t Space = 0; // 频率间隔
    uint16t band = 0;
    // 0x000C--->0000 0000 0000 1100
    band = (RDA5807M_Read_Reg(0x03) & 0x000C) >> 2; // 0x03的3，2位（band）

    if (band == 0 /*0b00*/)
    {
        Start = 8700;
        End = 10800;
    }
    else if (band == 1 /*0b01*/)
    {
        Start = 7600;
        End = 9100;
    }
    else if (band == 2 /*0b10*/)
    {
        Start = 7600;
        End = 10800;
    }
    else if (band == 3 /*0b11*/)
    {
        if ((RDA5807M_Read_Reg(0x07) >> 9) & 0x01)
        {
            Start = 6500;
            End = 7600;
        }
        else
        {
            Start = 5000;
            End = 7600;
        }
    }
    else
        return 0;

    band = (RDA5807M_Read_Reg(0x03) & 0x0003);

    if (band == 0 /*0b00*/)
        Space = 10;
    else if (band == 1 /*0b01*/)
        Space = 20;
    else if (band == 2 /*0b10*/)
        Space = 5;
    else if (band == 3 /*0b11*/)
        Space = 2; // 有问题
    else
        return 0;

    if (Freq < Start)
        return 0;
    if (Freq > End)
        return 0;

    return ((Freq - Start) / Space);
}
/**
 * @brief 将信道值转为频率
 * @param Chan:信道值
 * @return 频率(以MHz为单位*100)(如108MHz=>10800)

 * @date 2022-07-21 22:03:01
 */
uint16t RDA5807M_ChanToFreq(uint16t Chan)
{
    uint16t Start = 0; // 频率开始
    uint16t End = 0;   // 频率结束
    uint16t Space = 0; // 频率间隔
    uint16t band = 0;
    band = (RDA5807M_Read_Reg(0x03) & 0x000C) >> 2; // 0x03的3，2位（波段）

    if (band == 0 /*0b00*/)
    {
        Start = 8700;
        End = 10800;
    }
    else if (band == 1 /*0b01*/)
    {
        Start = 7600;
        End = 9100;
    }
    else if (band == 2 /*0b10*/)
    {
        Start = 7600;
        End = 10800;
    }
    else if (band == 3 /*0b11*/)
    {
        if ((RDA5807M_Read_Reg(0x07) >> 9) & 0x01)
        {
            Start = 6500;
            End = 7600;
        }
        else
        {
            Start = 5000;
            End = 7600;
        }
    }
    else
        return 0;

    band = (RDA5807M_Read_Reg(0x03) & 0x0003);

    if (band == 0 /*0b00*/)
        Space = 10;
    else if (band == 1 /*0b01*/)
        Space = 20;
    else if (band == 2 /*0b10*/)
        Space = 5;
    else if (band == 3 /*0b11*/)
        Space = 2;
    else
        return 0;
    band = Start + Chan * Space;
    if (band > End)
        return 0;
    if (band < Start)
        return 0;
    return band;
}
/**
 * @brief 读取当前频率
 * @param 无
 * @return 频率(以MHz为单位*100)(如108MHz=>10800)
 */
uint16t RDA5807M_Read_Freq(void)
{
    uint16t chan = 0;
    chan = RDA5807M_Read_Reg(0x0A) & 0x03FF;
    return RDA5807M_ChanToFreq(chan);
}
/**
 * @brief 设置频率值
 * @param Freq:频率(以MHz为单位*100)(如108MHz=>10800)
 * @return 无

 * @date 2022-07-21 22:06:22
 */
void RDA5807M_Set_Freq(uint16t Freq)
{
    uint16t chan = RDA5807M_FreqToChan(Freq); // 先转化为信道值
    uint16t band = RDA5807M_Read_Reg(0x03);
    band &= 0x003F;               // 清空信道值
    band |= (chan & 0x03FF) << 6; // 写入信道值
    band |= (1) << 4;             // 调频启用
    RDA5807M_Write_Reg(0x03, band);
    RDA5807M_Write_Reg(0x03, band); // 需要写入两次，咱也不知道为啥
    LED_SET_DISPLY_TYPE(10);
}

/**
 * 查询seek的snr阈值
 */
uint8t RDA5807M_Read_SNR()
{
    // 8~11 位  0~15 系统默认6
    uint16t temp_snr;
    temp_snr = RDA5807M_Read_Reg(0x05);
    temp_snr >>= 8;
    return ((uint8t)temp_snr) & 0x0F;
}

/**
 * 设置收音阈值
 */
void RDA5807M_Set_SNR(uint8t snr)
{
    // 8~11 位  0~15 系统默认6
    uint16t temp_snr;
    temp_snr = RDA5807M_Read_Reg(0x05);
    temp_snr &= 0xF0FF;
    temp_snr |= snr << 8;
    RDA5807M_Write_Reg(0x05, temp_snr);
}

/**
 * @brief 自动搜台
 * @param direction 方向
 * @param round 是否环绕搜台
 * @return 电台频率
 */
uint16t seek(uint8t direction, bit round)
{
    uint16t temp_reg;
    uint16t freq;
    temp_reg = RDA5807M_Read_Reg(0x03);
    temp_reg &= ~(1 << 4); // 禁用调谐
    RDA5807M_Write_Reg(0x03, temp_reg);

    temp_reg = RDA5807M_Read_Reg(0x02);
    if (direction == 1)
    {
        temp_reg |= 1 << 9; // 向上搜索
    }
    else
    {
        temp_reg &= ~(1 << 9); // 向下搜索
    }

    temp_reg |= 1 << 8; // 开启搜索
    if (round)
    {
        temp_reg &= ~(1 << 7); // 环绕搜索
    }
    else
    {
        temp_reg |= 1 << 7; // 边界终止搜台
    }

    RDA5807M_Write_Reg(0x02, temp_reg);
    while ((RDA5807M_Read_Reg(0x0A) & 0x4000) == 0) // 等待搜索完成
    {
        Delay(10);
    }

    // 将搜索到频率设置为播放频率
    freq = RDA5807M_Read_Freq();
    RDA5807M_Set_Freq(freq);
    return freq;
}

/**
 * @brief 手动搜索电台（搜索完成后会设置当前频率为搜到的频率）
 * @param direction 参数
 * @return 电台频率
 */
uint16t RDA5807M_Seek(uint8t direction)
{
    LED_SEEK_D = direction;
    return seek(direction, 1);
}

/**
 * @brief当前频率是否是电台
 * @return 1 = 是   0 = 否
 */
uint8t RDA5807M_Radio_TRUE()
{
    uint16t isRadio;
    isRadio = RDA5807M_Read_Reg(0x0B);
    isRadio >>= 8;
    isRadio &= 1;
    return isRadio;
}

/**
 *  自动搜台并保存
 */
void RDA5807M_Search_Automatic()
{
    uint16t i = 0; // 电台索引
    uint16t band = 0;
    uint16t Start, End;
    band = (RDA5807M_Read_Reg(0x03) & 0x000C) >> 2; // 0x03的3，2位（band）

    if (band == 0 /*0b00*/)
    {
        Start = 8700;
        End = 10800;
    }
    else if (band == 1 /*0b01*/)
    {
        Start = 7600;
        End = 9100;
    }
    else if (band == 2 /*0b10*/)
    {
        Start = 7600;
        End = 10800;
    }
    else if (band == 3 /*0b11*/)
    {
        if ((RDA5807M_Read_Reg(0x07) >> 9) & 0x01)
        {
            Start = 6500;
            End = 7600;
        }
        else
        {
            Start = 5000;
            End = 7600;
        }
    }
    else
    { // 没有匹配到band
        return;
    }
    // 控制数码管显示
    sys_freq = LED_FRE_REAL = Start;
    LED_SEEK_D = 1;
    LED_SET_DISPLY_TYPE(10);
    // 调整搜索开始频点
    RDA5807M_Set_Freq(Start);
    Delay(50);

    // 清空eeprom中的电台数据
    CONF_RADIO_ERASE();
    // 开始搜索
    while (sys_freq != End)
    {
        // 向下搜台 ，边界终止
        sys_freq = seek(1, 0);
        Delay(500); // 延迟等待系统判断电台
        if (RDA5807M_Radio_TRUE())
        {
            // 保存电台
            Delay(500); // 给用户听个声音
            CONF_RADIO_PUT(i, sys_freq);
            i++; // 最后会多加一次
        }
        LED_RESET_SLEEP_TIME();
    }

    i = i - 1;
    // 保存电台最大索引
    CONF_SET_INDEX_MAX(i);

    sys_radio_index = 0;
    LED_FRE_REAL = sys_freq = CONF_GET_RADIO_INDEX(0);
    RDA5807M_Set_Freq(LED_FRE_REAL);
    // 保存播放电台
    CONF_SET_FREQ(sys_freq);
}

/**
 * @brief 设置音量
 * @param Val:音量值(0-15)
 * @return 无
 */
void RDA5807M_Set_Volume(uint8t vol)
{
    uint16t temp_reg;
    temp_reg = RDA5807M_Read_Reg(0x05);
    temp_reg &= 0xFFF0;
    temp_reg |= (vol & 0x0F);
    RDA5807M_Write_Reg(0x05, temp_reg);

    sys_vol = vol;
    // 修改音量时候解除静音
    if (!MUTE_STATUS)
    {
        RDA5807M_SetMUTE(1);
    }
}

/**
 * @brief 设置静音
 * @param Bool：0是静音，1是解除静音
 * @return 无
 */
void RDA5807M_SetMUTE(uint8t mute)
{
    uint16t temp_reg;
    temp_reg = RDA5807M_Read_Reg(0x02);
    if (mute)
    {
        temp_reg |= 1 << 14;
    }
    else
    {
        temp_reg &= ~(1 << 14);
    }
    RDA5807M_Write_Reg(0x02, temp_reg);
}

/**
 * 静音模式和非静音模式来回切换，静音不持久化
 */
void RDA5807M_CHANGE_MUTE()
{
    uint16t temp_reg;
    uint8t mute;
    temp_reg = RDA5807M_Read_Reg(0x02);
    mute = temp_reg >> 14;
    mute = (~mute) & 0x01;
    if (mute)
    {
        temp_reg |= 1 << 14;
        MUTE_STATUS = 1;
    }
    else
    {
        temp_reg &= ~(1 << 14);
        MUTE_STATUS = 0;
    }
    RDA5807M_Write_Reg(0x02, temp_reg);
}

// /**
//  * @brief 将输出设为空闲状态（喇叭高阻）
//  * @param Bool：1是空闲，0是解除空闲
//  * @return 无
//  */
// void RDA5807M_Set_Output_Idle(uint8t flage)
// {
//     uint16t impedance;
//     impedance = RDA5807M_Read_Reg(0x02);
//     if (flage)
//     {
//         impedance &= ~(1 << 15);
//     }
//     else
//     {
//         impedance |= 1 << 15;
//     }
//     RDA5807M_Write_Reg(0x02, impedance);
// }

/**
 * @brief 获取当前频率的信号强度
 * @param 无
 * @return 信号强度(0-127)
 */
uint8t RDA5807M_Read_RSSI(void)
{
    uint16t temp_rssi;
    temp_rssi = RDA5807M_Read_Reg(0x0B);
    temp_rssi >>= 9;
    return (uint8t)temp_rssi;
}

// /**
//  * @brief  芯片id CHIPID[7:0]  ---- 15:8
//  * @param 无
//  * @return CHIPID
//  */
// uint16t RDA5807M_CHIPID(void)
// {
//     return RDA5807M_Read_Reg(0x00);
// }

// /**
//  * @brief 设置频率段
//  * @param band：频率段，来自频率段选择组的宏定义，如BAND_0
//  *  BAND = 0 Frequency = Channel Spacing (kHz) x CHAN+ 87.0 MHz
//  *  BAND = 1 or 2 Frequency = Channel Spacing (kHz) x CHAN + 76.0 MHz
//  *  BAND = 3 Frequency = Channel Spacing (kHz) x CHAN + 65.0 MHz
//  * @return 无

//  * @date 2022-07-23 11:16:42
//  */

// void RDA5807M_Set_Freqband(uint8t band)
// {
//     uint16t temp_reg;
//     temp_reg = RDA5807M_Read_Reg(0x03);
//     temp_reg &= 0xFFF3; // 1111 1111 1111 0011
//     if (band == BAND_87_108)
//     { /*0x03[3:2]=00 0x07[9]=x*/
//         // temp_reg |= 0x00;
//     }
//     else if (band == BAND_76_91)
//     { /*0x03[3:2]=01 0x07[9]=x*/
//         temp_reg |= 0x01;
//     }
//     else if (band == BAND_76_108)
//     { /*0x03[3:2]=10 0x07[9]=x*/
//         temp_reg |= 0x02;
//     }
//     else if (band == BAND_65_76)
//     { /*0x03[3:2]=11 0x07[9]=1*/
//         temp_reg |= 0x03;
//         RDA5807M_Write_Reg(0x07, RDA5807M_Read_Reg(0x07) | (~0xFDFF));
//     }
//     else if (band == BAND_50_76)
//     { /*0x03[3:2]=11 0x07[9]=0*/
//         temp_reg |= 0x03;
//         RDA5807M_Write_Reg(0x07, RDA5807M_Read_Reg(0x07) & 0xFDFF);
//     }

//     RDA5807M_Write_Reg(0x03, temp_reg);
// }

// /**
//  * @brief 设置频率步进
//  * @param space_step_step：间隔，从频率间隔选择组宏定义里选取，如space_step_step_100kHz
//  * @return 无
//  */
// void RDA5807M_Set_Freqspace_step_step(uint8t space_step)
// {
//     uint16t temp_reg;
//     temp_reg = RDA5807M_Read_Reg(0x03);
//     temp_reg &= 0xFFFC; // 把最后两位SPACE[1:0] 清零
//     if (space_step == Space_100kHz)
//     { /*0x03[1:0]=00*/
//         // temp_reg |= 0x00;
//     }
//     else if (space_step == Space_200kHz)
//     { /*0x03[1:0]=01*/
//         temp_reg |= 0x01;
//     }
//     else if (space_step == Space_50KHz)
//     { /*0x03[1:0]=10*/
//         temp_reg |= 0x02;
//     }
//     else if (space_step == Space_25KHz)
//     { /*0x03[1:0]=11*/
//         temp_reg |= 0x03;
//     }
//     RDA5807M_Write_Reg(0x03, temp_reg);
// }

void RDA5807M_OFF(void)
{
    RDA5807M_Write_Reg(0x02, RDA5807M_Read_Reg(0x02) & 0xFFFE);
}

// /**
//  * @brief 软件复位 Soft reset && NEW_METHOD +1dB.
//  * @param 无
//  * @return
//  */
// void RDA5807M_Reast(void)
// {
//     RDA5807M_Write_Reg(0x02, 0x0003); // 复位
//     Delay(50);
// }
