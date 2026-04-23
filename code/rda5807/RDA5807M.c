#include "stdio.h" //格式化printf
#include "stc15.h"
#include "../delay/Delay.h"
#include "RDA5807M.h"
#include "../config/Config.h"
#include "I2C.h"
#include "../led/myLed.h"

// 频段参数结构体
typedef struct
{
  uint16_t Start;
  uint16_t End;
  uint16_t Space;
} FreqBand;

// 静音标记 0 = Mute; 1 = Normal operation
bit MUTE_STATUS = 1;
FreqBand _band = {0, 0, 0};

/**
 * 写寄存器 16bit
 */
void RDA5807M_Write_Reg(uint8_t Address, uint16_t Data)
{
  uint8_t Buf[2] = {0};
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
uint16_t RDA5807M_Read_Reg(uint8_t Address)
{
  uint8_t Buf[2] = {0};

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
 * @brief 配置和缓存频段到内存
 * @return FreqBand 频段参数结构体
 */
void _BandConfig(uint8_t band_sel, uint8_t space_sel, uint8_t mode)
{

  switch (band_sel)
  {
  case 0:
    _band.Start = 8700;
    _band.End = 10800;
    break; // 87-108 MHz
  case 1:
    _band.Start = 7600;
    _band.End = 9100;
    break; // 76-91 MHz (Japan)
  case 2:
    _band.Start = 7600;
    _band.End = 10800;
    break; // 76-108 MHz
  case 3:  // 65-76 or 50-76 MHz
    if (mode)
    {
      _band.Start = 6500;
      _band.End = 7600;
    }
    else
    {
      _band.Start = 5000;
      _band.End = 7600;
    }
    break;
  }

  switch (space_sel)
  {
  case 0:
    _band.Space = 10;
    break;
  case 1:
    _band.Space = 20;
    break;
  case 2:
    _band.Space = 5;
    break;
  case 3:
    _band.Space = 2;
    break; // 仅RDA5807SP支持
  }
}

/**
 * @brief 将频率转为信道值
 * @param Freq:频率(以MHz为单位*100)(如108MHz=>10800)
 * @return 转换为的信道值
 */
uint16_t RDA5807M_FreqToChan(uint16_t Freq)
{
  if (_band.Space == 0 || _band.Start == 0 || _band.End == 0)
    return 0;
  if (Freq < _band.Start || Freq > _band.End)
    return 0;
  return (Freq - _band.Start) / _band.Space;
}

/**
 * @brief 将信道值转为频率
 * @param Chan:信道值
 * @return 频率(以MHz为单位*100)(如108MHz=>10800)
 */
uint16_t RDA5807M_ChanToFreq(uint16_t Chan)
{
  uint16_t freq;
  if (_band.Space == 0 || _band.Start == 0 || _band.End == 0)
    return 0;
  freq = _band.Start + Chan * _band.Space;
  if (freq > _band.End || freq < _band.Start)
    return 0;
  return freq;
}

/**
 * @brief init
 * @param 无
 */
void RDA5807M_init(void)
{
  uint16_t h03 = 0x0000;
  uint16_t h07 = 0x5F1A;
  uint8_t band_sel = CONF_READ_BAND();
  uint8_t space_sel = CONF_READ_SPACE();
  // printf("RDA5807M_init %bu  %bu \r\n", band_sel, space_sel);

  RDA5807M_Write_Reg(0x02, 0x0003); // reset
  Delay(50);
  RDA5807M_Write_Reg(0x02, 0xc005);
  Delay(50);

  // 默认0x0000 或上space band_sel chan TUNE
  h03 |= space_sel;
  if (band_sel == 4)
  { // 50-65MHz
    h03 |= 0x0c;
    h07 &= (1 << 9); // 65M_50M MODE 设置为0
    _BandConfig(0x03, space_sel, 0);
  }
  else
  {
    h03 |= band_sel << 2;
    _BandConfig(band_sel, space_sel, 1);
  }

  // printf("RDA5807M_init FM range  %d - %d\r\n", _band.Start, _band.End);

  h03 |= RDA5807M_FreqToChan(sys_freq) << 6;
  h03 |= (1 << 4);

  RDA5807M_Write_Reg(0x03, h03);
  // seek SNR 1000->8
  // LNA_ICSEL_BIT 2.5ma(LNA的工作电流直接影响射频前端的性能指标中)
  // 设置默认音量
  RDA5807M_Write_Reg(0x05, 0x88a0 | sys_vol);

  RDA5807M_Write_Reg(0x06, 0x0000);
  RDA5807M_Write_Reg(0x07, h07);
}

/**
 * @brief 读取当前频率
 * @param 无
 * @return 频率(以MHz为单位*100)(如108MHz=>10800)
 */
uint16_t RDA5807M_Read_Freq(void)
{
  return RDA5807M_ChanToFreq(RDA5807M_Read_Reg(0x0A) & 0x03FF);
}
/**
 * @brief 设置频率值
 * @param Freq:频率(以MHz为单位*100)(如108MHz=>10800)
 * @return 无

 * @date 2022-07-21 22:06:22
 */
void RDA5807M_Set_Freq(uint16_t Freq)
{
  uint16_t chan = RDA5807M_FreqToChan(Freq); // 先转化为信道值
  uint16_t h03 = RDA5807M_Read_Reg(0x03);
  h03 &= 0x003F;               // 清空信道值
  h03 |= (chan & 0x03FF) << 6; // 写入信道值
  h03 |= 1 << 4;               // 调频启用
  RDA5807M_Write_Reg(0x03, h03);

  // 等待调谐完成
  while (!(RDA5807M_Read_Reg(0x0A) & (1 << 14)))
    ; // STC=1

  // The tune bit is reset to low automatically when the tune operation
  // completes.. RDA5807M_Write_Reg(0x03, h03 & ~(1 << 4));
  LED_SET_DISPLY_TYPE(10);
}

/**
 * 查询snr阈值
 */
uint8_t RDA5807M_Read_SNR(void)
{
  // 8~11 位  0~15 系统默认6
  uint16_t temp_snr;
  temp_snr = RDA5807M_Read_Reg(0x05);
  temp_snr >>= 8;
  return ((uint8_t)temp_snr) & 0x0F;
}

/**
 * 设置收音阈值
 */
void RDA5807M_Set_SNR(uint8_t snr)
{
  // 8~11 位  0~15 系统默认6
  uint16_t temp_snr;
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
uint16_t SEEK(uint8_t direction, bit round)
{
  uint16_t temp_reg;
  uint16_t freq;
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
  // 添加超时保护，避免死循环
  while (!(RDA5807M_Read_Reg(0x0A) & (1 << 14)))
  { // 0AH STC判断
    Delay(10);
  }

  // 将搜索到频率设置为播放频率
  freq = RDA5807M_Read_Freq();

  temp_reg = RDA5807M_Read_Reg(0x02);
  temp_reg &= ~(1 << 8); // 清除 SEEK 位
  temp_reg |= (1 << 14); // 恢复声音 (DMUTE=1)
  RDA5807M_Write_Reg(0x02, temp_reg);
  return freq;
}

/**
 * @brief 手动搜索电台（搜索完成后会设置当前频率为搜到的频率）
 * @param direction 参数
 * @return 电台频率
 */
uint16_t RDA5807M_Seek(uint8_t direction)
{
  LED_SEEK_D = direction;
  return SEEK(direction, 1);
}

/**
 * @brief当前频率是否是电台
 * @return 1 = 是   0 = 否
 */
bit RDA5807M_Radio_TRUE()
{
  uint16_t isRadio;
  isRadio = RDA5807M_Read_Reg(0x0B);
  isRadio >>= 8;
  return isRadio & 1;
}

/**
 *  自动搜台并保存
 */
void RDA5807M_Search_Automatic()
{
  uint8_t i = 0; // 电台索引

  // 控制数码管显示
  sys_freq = LED_FRE_REAL = _band.Start;
  LED_SEEK_D = 1;
  LED_SET_DISPLY_TYPE(10);
  // 调整搜索开始频点
  RDA5807M_Set_Freq(_band.Start);
  Delay(50);
  // 清空eeprom中的电台数据
  CONF_RADIO_ERASE();
  // 开始搜索
  while (sys_freq < _band.End)
  {
    // 向下搜台 ，边界终止
    sys_freq = SEEK(1, 0);
    Delay(500); // 延迟等待系统判断电台

    // 检查 0AH 寄存器的 SF 位 (Seek Fail)
    if (RDA5807M_Read_Reg(0x0A) & (1 << 13))
    {
      break; // 搜索失败，跳出
    }
    // printf("save radio  %bu  %d \r\n", i, sys_freq);

    if (RDA5807M_Radio_TRUE())
    {
      // 保存电台
      Delay(500); // 给用户听个声音
      CONF_RADIO_PUT(i, sys_freq);
      i++; // 最后会多加一次
    }
    LED_RESET_SLEEP_TIME();
  }

  if (i == 0)
  {
    CONF_WRITE_INDEX_MAX(0);
    sys_freq = 0x0000;
    sys_radio_index = 0;
    sys_radio_index_max = 0;
    // printf("no radio found \r\n");
    return;
  }

  if (i > 0)
  {
    i = i - 1;
  }
  // 保存电台最大索引
  CONF_WRITE_INDEX_MAX(i);
  // printf("CONF_WRITE_INDEX_MAX  %bu  \r\n", i);

  // //切换到第一个电台
  sys_radio_index = 0;
  LED_FRE_REAL = sys_freq = CONF_GET_FREQ_BY_INDEX(0);
  RDA5807M_Set_Freq(LED_FRE_REAL);

  CONF_WRITE();
}

void RDA5807M_Set_Volume(uint8_t vol)
{
  uint16_t temp_reg;
  // 限制音量范围 0-15
  if (vol > 15)
  {
    vol = 15;
  }

  // 优先解除静音
  if (!MUTE_STATUS)
  {
    temp_reg = RDA5807M_Read_Reg(0x02);
    temp_reg |= (1 << 14); // 设置14位1 解除静音
    RDA5807M_Write_Reg(0x02, temp_reg);
    MUTE_STATUS = 1;
  }

  temp_reg = RDA5807M_Read_Reg(0x05);
  temp_reg &= 0xFFF0;
  temp_reg |= (vol & 0x0F);
  RDA5807M_Write_Reg(0x05, temp_reg);
  sys_vol = vol;
}

/**
 * @brief
 *
 */
void RDA5807M_SET_MUTE()
{
  uint16_t temp_reg;
  temp_reg = RDA5807M_Read_Reg(0x02);
  temp_reg &= ~(1 << 14); // 14位设置为0 开启静音
  MUTE_STATUS = 0;
  RDA5807M_Write_Reg(0x02, temp_reg);
}

/**
 * @brief 获取当前频率的信号强度
 * @param 无
 * @return 信号强度(0-127)
 */
uint8_t RDA5807M_Read_RSSI(void)
{
  uint16_t temp_rssi;
  temp_rssi = RDA5807M_Read_Reg(0x0B);
  temp_rssi >>= 9;
  return (uint8_t)temp_rssi;
}

void RDA5807M_OFF(void)
{
  RDA5807M_Write_Reg(0x02, RDA5807M_Read_Reg(0x02) & 0xFFFE);
}
