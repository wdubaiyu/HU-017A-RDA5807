#include "Config.h"
#include "../delay/Delay.h"
#include "EEPROM.h"
#include "stc15.h"
#include "stdio.h" //printf


uint8_t sys_band = 0x00;
uint8_t sys_vol = 0x05;
// 0一段时间后休眠 1一直显示
bit sys_sleep_mode = 1;
// 轮询展示freq 和 rssi 开关 1是轮训
bit cycle_in_freq_rssi = 0;
uint16_t sys_freq = 0x21FC; // 8700

// 当前频率对应电台的序号（最多255）
uint8_t sys_radio_index = 0x00;
uint8_t sys_radio_index_max = 0x00;

/**
 * 从EEPROM中读取存储的电台频率
 * @param EEPROM地址
 */
uint16_t CONF_READ_RAIDO_FREQ(uint16_t addr) {
  uint8_t freq_array_read[2] = {0x00};
  IapReadArrayByte(addr, freq_array_read);
  return ((uint16_t)freq_array_read[0]) << 8 | freq_array_read[1];
}

/**
 * 通过频道号从EEPROM中读取存储的电台频率
 * @param radio_index
 */
uint16_t CONF_GET_FREQ_BY_INDEX(uint8_t index) {
  uint16_t temp_addr = addr_radio_list + (index * 2);
  uint16_t freq = CONF_READ_RAIDO_FREQ(temp_addr);
  // 修改系统频率  tips 只有需要播放时才会通过index读取频率这里直接设置了
  sys_freq = freq;
  sys_radio_index = index;
  return freq;
}

void CONF_WRITE(void) {
  // 清空第0扇区0x0000~0x0200
  IapEraseSector(addr_vol);
  // 写入音量
  IapProgramByte(addr_vol, sys_vol & 0x00FF);
  // 写入索引
  IapProgramByte(addr_freq_index, sys_radio_index);

  // 写入休眠模式  // 写入轮询模式
  IapProgramByte(addr_mode, (sys_sleep_mode ? 0x01 : 0x00) |
                               (cycle_in_freq_rssi ? 0x00 : 0x02));
}

uint8_t CONF_READ_SPACE(void) {
  return 0x02;
  // switch (sys_band) {
  // case 0:
  //   return 0x00;
  // case 1:
  //   return 0x00;
  // case 2:
  //   return 0x00;
  // case 3:
  //   return 0x02;
  // case 4:
  //   return 0x02;
  // default:
  //   return 0x00;
  // }
}

uint8_t CONF_READ_BAND(void) {
  uint8_t h03band = IapReadByte(addr_h03);
  if (h03band == 0xFF) {
    return 0x00; // 默认 0 = 87–108 MHz (US/Europe)
  }
  return (h03band & 0x1C) >> 2;
}

/**
 * 清空电台(包括频道号，和频率列表)
 */
void CONF_RADIO_ERASE() {
  IapEraseSector(addr_radio);
  IapProgramByte(addr_h03, sys_band << 2 | CONF_READ_SPACE());
}

/**
 * 搜台完成,radio_index_max
 * @param 电台总数
 */
void CONF_WRITE_INDEX_MAX(uint8_t index) {
  sys_radio_index_max = index;
  IapProgramByte(addr_radio, index);
}

/**
 * 追加一个电台
 */
void CONF_RADIO_PUT(uint8_t index, uint16_t freq) {
  uint16_t temp_addr;
  uint8_t freq_array[2] = {0x00};
  freq_array[0] = freq >> 8;
  freq_array[1] = freq;
  temp_addr = addr_radio_list + index * 2;
  IapProgramByte(temp_addr, freq_array[0]);
  Delay(4);
  IapProgramByte(temp_addr + 1, freq_array[1]);
}

uint8_t CONF_SYS_INIT(void) {
  // 从eeprom获取音量并纠正
  uint8_t band = IapReadByte(addr_h03);
  uint8_t _mode = IapReadByte(addr_mode);
  if (band == 0xFF) {
    sys_band = 0x00; // 重置band
    return 0x01;
  }
  sys_band = (band & 0x1C) >> 2;
  //   printf("init band: %bu\n", sys_band);

  sys_vol = IapReadByte(addr_vol);
  if (sys_vol < 0 | sys_vol > 15) {
    sys_vol = 5;
  }
  // 从eeprom获取睡眠模式纠正
  sys_sleep_mode = (_mode & 0x01);
  // 从eeprom获取POLL模式纠正
  cycle_in_freq_rssi = (_mode & 0x02) ? 0 : 1;

  // printf("init mode: %bu %d  %d\r\n",
  // _mode,(int)sys_sleep_mode,(int)cycle_in_freq_rssi);

  // 读取电台最大索引（0~254有效），255没搜索过
  sys_radio_index_max = IapReadByte(addr_radio);

  if (sys_radio_index_max == 0xFF || sys_radio_index_max == 0) // 没有搜过台
  {
    return 0x02; // 需要自动搜台
  }

  // 加载上一次选择的电台索引
  sys_radio_index = IapReadByte(addr_freq_index);
  CONF_GET_FREQ_BY_INDEX(sys_radio_index);

  //   printf("read config %bu  %d  %bu\r\n", sys_vol, sys_freq,
  //   sys_radio_index);
  return 0;
}

void CONF_RESET(void) {
  sys_vol = 0x05;
  sys_sleep_mode = 1;
  cycle_in_freq_rssi = 1;
  IapEraseSector(addr_vol);
  IapEraseSector(addr_radio);
}