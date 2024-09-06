#include <stc15.h>
#include <stdio.h>
#include "Delay.h"
#include "Config.h"
#include "EEPROM.h"

uint8t sys_vol = 0x05;
// 1 是关闭睡眠模式 0开启睡眠模式
bit sys_sleep_mode = 1;
uint16t sys_freq = 0x21FC; // 1017
// 当前频率对应电台的序号
uint8t sys_radio_index = 0x00;
uint8t sys_radio_index_max = 0x00;

/**
 * 频率最大值10800 占用两个字节
 */
uint16t CONF_READ_RAIDO_FREQ(uint16t addr)
{
    uint8t freq_array_read[2] = {0x00};
    IapReadArrayByte(addr, freq_array_read);
    return ((uint16t)freq_array_read[0]) << 8 | freq_array_read[1];
}

void CONF_SET_VOL(uint8t vol)
{
    if (sys_vol != vol)
    {
        IapEraseSector(addr_vol);
        IapProgramByte(addr_vol, vol & 0x00FF);
        sys_vol = vol & 0x00FF;
    }
}

/**
 * 当前电台数据保存（频率和索引）
 */
void CONF_SET_FREQ(uint16t freq)
{
    // 暂存数据
    uint8t freq_array[2] = {0x00};
    freq_array[0] = freq >> 8;
    freq_array[1] = freq;
    // 清空扇区
    IapEraseSector(addr_freq);
    IapProgramByte(addr_freq, freq_array[0]);
    Delay_Ms_(2);
    IapProgramByte(addr_freq + 1, freq_array[1]);

    if (sys_radio_index != 0xFF)
    {
        IapProgramByte(addr_freq_index, sys_radio_index);
    }
    sys_freq = freq; // 修改系统配频率
}

/**
 * 切换 占用一个字节
 */
void CONF_CHANGE_SLEEP_MODE()
{
    // 读取系统睡眠模式
    if (IapReadByte(addr_sleep_mode) & 0x01)
    {
        sys_sleep_mode = 0;
    }
    else
    {
        sys_sleep_mode = 1;
    }

    IapEraseSector(addr_sleep_mode);
    IapProgramByte(addr_sleep_mode, 0x00 | sys_sleep_mode);
}

/**
 * 清空电台(包括频道号，和频率列表)
 */
void CONF_RADIO_ERASE()
{
    IapEraseSector(addr_radio);
}

/**
 * 追加一个电台
 */
void CONF_RADIO_PUT(uint8t index, uint16t freq)
{
    uint16t temp_addr;
    uint8t freq_array_read[2] = {0x00};
    uint8t freq_array[2] = {0x00};
    freq_array[0] = freq >> 8;
    freq_array[1] = freq;
    temp_addr = addr_radio_list + index * 2;
    IapProgramByte(temp_addr, freq_array[0]);
    Delay_Ms_(2);
    IapProgramByte(temp_addr + 1, freq_array[1]);

    IapReadArrayByte(temp_addr, freq_array_read);

    // printf("CONF_RADIO_PUT GET %d   %bu\r\n", CONF_READ_RAIDO_FREQ(temp_addr), index);
}

/**
 * 搜台完成,radio_index_max
 * @param 电台总数
 */
void CONF_SET_INDEX_MAX(uint8t index)
{
    sys_radio_index_max = index;
    IapProgramByte(addr_radio, index);
}

uint8t CONF_SYS_INIT(void)
{
    // 从eeprom获取音量并纠正
    sys_vol = IapReadByte(addr_vol);
    // printf("read config vol %bu\r\n", sys_vol);
    if (sys_vol < 0 | sys_vol > 15)
    {
        sys_vol = 5;
    }

    // 从eeprom获取睡眠模式纠正
    sys_sleep_mode = IapReadByte(addr_sleep_mode);
    // printf("read config sleep mode %bu\r\n", sys_sleep_mode);
    if (sys_sleep_mode)
    {
        sys_sleep_mode = 1;
    }
    else
    {
        sys_sleep_mode = 0;
    }

    // 读取电台最大索引（0~254有效），255没搜索过
    sys_radio_index_max = IapReadByte(addr_radio);
    // printf("read config sys_radio_index_max %bu\r\n", sys_radio_index_max);
    if (sys_radio_index_max == 0xFF) // 没有搜过台
    {
        return 1; // 需要自动搜台
    }

    // 加载电台配置
    sys_freq = CONF_READ_RAIDO_FREQ(addr_freq);
    // printf("read config sys_freq %d\r\n", sys_freq);
    sys_radio_index = IapReadByte(addr_freq_index);
    // printf("read config sys_radio_index %bu\r\n", sys_radio_index);

    return 0;
}

/**
 *  通过频道号 查询电台频率
 */
uint16t CONF_GET_RADIO_INDEX(uint8t index)
{
    uint16t temp_addr = addr_radio_list + (index * 2);
    uint16t freq = CONF_READ_RAIDO_FREQ(temp_addr);
    // 修改系统频率
    sys_freq = freq;
    sys_radio_index = index;
    // printf("read radio %d %bu\r\n", freq, index);
    return freq;
}