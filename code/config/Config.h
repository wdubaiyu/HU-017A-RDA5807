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
extern uint16t sys_freq;
extern uint8t sys_radio_index;
extern uint8t sys_radio_index_max;
extern bit sys_sleep_mode;
// 轮询显示SNR和RSSI
extern bit sys_poll_mode;

// 延迟写类型标记
extern bit sys_write_freq_flag;
extern bit sys_write_vol_flag;
extern bit sys_write_sleep_flag;
extern bit sys_write_poll_flag;

/**
 * 开机初始化读取配置
 */
uint8t CONF_SYS_INIT(void);

/**
 * 持久化当前电台（频率和索引）
 */
void CONF_SET_FREQ(uint16t freq);

/**
 * 触发写配置
 */
void CONF_WRITE(void);

/**
 *  通过频道号 查询电台频率
 */
uint16t CONF_GET_RADIO_INDEX(uint8t index);

/**
 * 清空电台(包括频道号，和频率列表)
 */
void CONF_RADIO_ERASE(void);

/**
 * 追加一个电台
 */
void CONF_RADIO_PUT(uint8t index, uint16t freq);

/**
 * 搜台完成,保存频道总数
 */
void CONF_SET_INDEX_MAX(uint8t index);

#endif
