#include <intrins.h>
#include <stdio.h>
#include <stc15.h>
#include "Delay.h"
#include "RDA5807M.h"
#include "I2C.h"
#include "led/myLed.h"

// 静音标记 0 = Mute; 1 = Normal operation 
bit MUTE_STATUS = 1;

// 频段参数结构体
typedef struct {
    uint16t Start;
    uint16t End;
    uint16t Space;
} FreqBand;

// 前向声明
uint16t RDA5807M_Read_Reg(uint8t Address);

// 获取当前频段参数
FreqBand RDA5807M_GetBandParams(void)
{
    FreqBand band;
    uint16t band_sel;
    uint16t space_sel;
    
    band.Start = 0;
    band.End = 0;
    band.Space = 0;
    
    band_sel = (RDA5807M_Read_Reg(0x03) & 0x000C) >> 2;
    space_sel = RDA5807M_Read_Reg(0x03) & 0x0003;
    
    switch (band_sel) {
        case 0: band.Start = 8700; band.End = 10800; break;  // 87-108 MHz
        case 1: band.Start = 7600; band.End = 9100;  break;  // 76-91 MHz (Japan)
        case 2: band.Start = 7600; band.End = 10800; break;  // 76-108 MHz
        case 3:                                              // 65-76 or 50-76 MHz
            if ((RDA5807M_Read_Reg(0x07) >> 9) & 0x01) {
                band.Start = 6500; band.End = 7600;
            } else {
                band.Start = 5000; band.End = 7600;
            }
            break;
        default: return band;
    }
    
    switch (space_sel) {
        case 0: band.Space = 10; break;
        case 1: band.Space = 20; break;
        case 2: band.Space = 5;  break;
        case 3: band.Space = 2;  break;  // 仅RDA5807SP支持
    }
    return band;
}

/**
 * 写寄存器 16bit
 */
void RDA5807M_Write_Reg(uint8t Address, uint16t Data)
{
    uint8t Buf[2] = {0};
    Buf[0] = (Data & 0xff00) >> 8; // 高位
    Buf[1] = Data & 0x00ff;        // 低位

    I2C_Start();
    I2C_SendByte(0x11 << 1);
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
    I2C_SendByte(0x11 << 1);
    I2C_SendByte(Address);
    I2C_Start();
    I2C_SendByte((0x11 << 1) | 1);
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
    RDA5807M_Write_Reg(0x02, 0x0003); // reset
    Delay(50);
    RDA5807M_Write_Reg(0x02, 0xc005);
    Delay(50);
    RDA5807M_Write_Reg(0x03, 0x0010 | ((sys_freq - 8700) / 10) << 6); // 0x0010(TUNE BAND00 SPACE00)--> 87–108 MHz (US/Europe) SPACE 100 kHz  设置ch对应sys_freqMHz
    RDA5807M_Write_Reg(0x05, 0x86a0 | sys_vol); // seek SNR 0110  --> 6
    RDA5807M_Write_Reg(0x06, 0x0000);
    RDA5807M_Write_Reg(0x07, 0x5F1A);
    LED_FRE_REAL = sys_freq;
}

/**
 * @brief 将频率转为信道值
 * @param Freq:频率(以MHz为单位*100)(如108MHz=>10800)
 * @return 转换为的信道值
 */
uint16t RDA5807M_FreqToChan(uint16t Freq)
{
    FreqBand band;
    band = RDA5807M_GetBandParams();
    if (band.Start == 0 && band.End == 0) return 0;
    if (Freq < band.Start || Freq > band.End) return 0;
    return (Freq - band.Start) / band.Space;
}

/**
 * @brief 将信道值转为频率
 * @param Chan:信道值
 * @return 频率(以MHz为单位*100)(如108MHz=>10800)
 */
uint16t RDA5807M_ChanToFreq(uint16t Chan)
{
    FreqBand band;
    uint16t freq;
    band = RDA5807M_GetBandParams();
    if (band.Start == 0 && band.End == 0) return 0;
    freq = band.Start + Chan * band.Space;
    if (freq > band.End || freq < band.Start) return 0;
    return freq;
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
    band |= 1 << 4;             // 调频启用
    RDA5807M_Write_Reg(0x03, band);

    // 等待调谐完成
    while (!(RDA5807M_Read_Reg(0x0A) & (1 << 14)))
        ; // STC=1

    // The tune bit is reset to low automatically when the tune operation completes..
    // RDA5807M_Write_Reg(0x03, band & ~(1 << 4));
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
    uint16t timeout = 0;
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
    // 添加超时保护，避免I2C失败时永久阻塞
    while (!(RDA5807M_Read_Reg(0x0A) & (1 << 14)))
    {
        Delay(10);
        if (++timeout > 200) break;  // 超时约2秒
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
 * @brief 当前频率是否是有效电台
 * @return 1 = 是电台   0 = 不是电台
 */
uint8t RDA5807M_IsStationPresent(void)
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
    FreqBand band;
    uint16t found_freq;
    
    band = RDA5807M_GetBandParams();
    if (band.Start == 0 && band.End == 0) return; // 无效频段
    
    // 控制数码管显示
    sys_freq = LED_FRE_REAL = band.Start;
    LED_SEEK_D = 1;
    LED_SET_DISPLY_TYPE(10);
    // 调整搜索开始频点
    RDA5807M_Set_Freq(band.Start);
    Delay(50);

    // 清空eeprom中的电台数据
    CONF_RADIO_ERASE();
    
    // 开始搜索
    while (sys_freq != band.End)
    {
        // 搜台后立即检查是否是有效电台
        sys_freq = seek(1, 0);
        Delay(200);  // 短暂延迟让芯片稳定
        
        if (RDA5807M_IsStationPresent())
        {
            // 再次确认频率是否匹配（避免频率漂移）
            found_freq = RDA5807M_Read_Freq();
            if (found_freq == sys_freq)
            {
                // 保存电台
                Delay(300); // 给用户听个声音
                CONF_RADIO_PUT(i, sys_freq);
                i++;
            }
        }
        LED_RESET_SLEEP_TIME();
    }

    // 保存电台最大索引 (i-1是因为最后一个i++后没保存)
    if (i > 0) i--;
    CONF_SET_INDEX_MAX(i);

    sys_radio_index = 0;
    LED_FRE_REAL = sys_freq = CONF_GET_RADIO_INDEX(0);
    RDA5807M_Set_Freq(LED_FRE_REAL);
    // 保存播放电台
    CONF_SET_FREQ(sys_freq);
}


/**
 * @brief 设置音量 优先解除静音
 * @param Val:音量值(0-15)
 * @return 无
 */
void RDA5807M_Set_Volume(uint8t vol)
{
    uint16t temp_reg;

    // 优先解除静音
    if (!MUTE_STATUS)
    {
        temp_reg = RDA5807M_Read_Reg(0x02);
        temp_reg |= (1 << 14); // 设置14位1 解除静音
        RDA5807M_Write_Reg(0x02, temp_reg);
        MUTE_STATUS = 1;
    }else
    {
        temp_reg = RDA5807M_Read_Reg(0x05);
        temp_reg &= 0xFFF0;
        temp_reg |= (vol & 0x0F);
        RDA5807M_Write_Reg(0x05, temp_reg);

        sys_vol = vol;
    }
}



/**
 * 开启静音模式 静音不持久化
 */
void RDA5807M_SET_MUTE()
{
    uint16t temp_reg;
    temp_reg = RDA5807M_Read_Reg(0x02);
    temp_reg &= ~(1 << 14); //14位设置为0 开启静音
    MUTE_STATUS = 0;
    RDA5807M_Write_Reg(0x02, temp_reg);
}


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


void RDA5807M_OFF(void)
{
    RDA5807M_Write_Reg(0x02, RDA5807M_Read_Reg(0x02) & 0xFFFE);
}
