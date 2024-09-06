#ifndef _CONFIG_5807M_
#define _CONFIG_5807M_

#define ERROR 0
#define OK 1

// 自定义全局类型 uint8t 一个字节
typedef unsigned char uint8t;
// 自定义全局类型 uint16t 两个字节 高位在前，低位在后
typedef unsigned short int uint16t;

// 全局变量
extern uint8t sys_vol;
extern bit sys_sleep_mode;
extern uint16t sys_freq;
extern uint8t sys_radio_index;
extern uint8t sys_radio_index_max;

/**
 * 开机初始化读取配置
 */
uint8t CONF_SYS_INIT(void);
/**
 * 写音量到eeprom
 */
void CONF_SET_VOL(uint8t vol);
/**
 * 写频率到eeprom
 */
void CONF_SET_FREQ(uint16t freq);

/**
 * 保存睡眠模式
 */
void CONF_CHANGE_SLEEP_MODE();

/**
 * 清空电台(包括频道号，和频率列表)
 */
void CONF_RADIO_ERASE();

/**
 * 追加一个电台
 */
void CONF_RADIO_PUT(uint8t index, uint16t freq);

/**
 * 搜台完成,保存频道总数
 */
void CONF_SET_INDEX_MAX(uint8t index);

/**
 *  通过频道号 查询电台频率
 */
uint16t CONF_GET_RADIO_INDEX(uint8t index);

#endif
