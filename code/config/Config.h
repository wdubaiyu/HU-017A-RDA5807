#ifndef _CONFIG_5807M_
#define _CONFIG_5807M_

#include "../type.h"

// 全局变量
extern uint8_t sys_band;
extern uint8_t sys_vol;
extern uint16_t sys_freq;
extern uint8_t sys_radio_index;
extern uint8_t sys_radio_index_max;
extern bit sys_sleep_mode;
// 轮询显示SNR和RSSI
extern bit cycle_in_freq_rssi;



/**
 * @brief 复位配置
 * 
 * 复位所有配置到默认值
 */
void CONF_RESET(void);

/**
 * 开机初始化读取配置
 */
uint8_t CONF_SYS_INIT(void);

/**
 * @brief 读取band
    Band Select. 
    0 = 87–108 MHz (US/Europe)
    1 = 76–91 MHz (Japan) 
    2 = 76–108 MHz (world wide) 
    3 = 65 –76 MHz（If 0x07h_bit<9> ( band )=1, 65-76MHz; =0, 50-76MHz）
    4 = 50-65MHz （If 0x07h_bit<9> ( band )=1, 65-76MHz; =0, 50-76MHz） 0100=4
 * 
 * @return uint8_t 
 */
uint8_t CONF_READ_BAND(void);

/**
 * @brief 读取频率间隔,需要先设置系统band
 * Channel Spacing. 
    0 = 100 kHz 
    1 = 200 kHz 
    2 = 50kHz 
    3 = 25KHz 
 * @return uint8_t 
 */
uint8_t CONF_READ_SPACE(void);


/**
 * 触发写配置
 */
void CONF_WRITE(void);


/**
 * @brief 通过索引获取电台频率
 * 
 * @param index 0~254
 * @return uint16_t 频率值
 */
uint16_t CONF_GET_FREQ_BY_INDEX(uint8_t index);


/**
 * @brief 擦除电台eeprom
 * 
 * @param band 频段 0~4 
 */
void CONF_RADIO_ERASE();

/**
 * 追加一个电台
 */
void CONF_RADIO_PUT(uint8_t index, uint16_t freq);

/**
 * 搜台完成,保存频道总数
 */
void CONF_WRITE_INDEX_MAX(uint8_t index);

#endif
