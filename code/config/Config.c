#include <stc15.h>
#include <stdio.h>
#include "Delay.h"
#include "Config.h"
#include "EEPROM.h"

uint8t sys_vol = 0x05;
// 0一段时间后休眠 1一直显示
bit sys_sleep_mode;
uint16t sys_freq = 0x21FC; // 1017

// 当前频率对应电台的序号
uint8t sys_radio_index = 0x00;
uint8t sys_radio_index_max = 0x00;

bit sys_write_freq_flag = 0;
bit sys_write_vol_flag = 0;
bit sys_write_sleep_flag = 0;

/**
 * 从EEPROM中读取存储的电台频率
 * @param EEPROM地址
 */
uint16t CONF_READ_RAIDO_FREQ(uint16t addr)
{
    uint8t freq_array_read[2] = {0x00};
    IapReadArrayByte(addr, freq_array_read);
    return ((uint16t)freq_array_read[0]) << 8 | freq_array_read[1];
}

/**
 * 通过频道号从EEPROM中读取存储的电台频率
 * @param radio_index
 */
uint16t CONF_GET_RADIO_INDEX(uint8t index)
{
    uint16t temp_addr = addr_radio_list + (index * 2);
    uint16t freq = CONF_READ_RAIDO_FREQ(temp_addr);
    // 修改系统频率  tips 只有需要播放时才会通过index读取频率这里直接设置了
    sys_freq = freq;
    sys_radio_index = index;
    return freq;
}

/**
 * 持久化音量
 */
void CONF_SET_VOL(uint8t vol)
{
    IapEraseSector(addr_vol);
    IapProgramByte(addr_vol, vol & 0x00FF);
    sys_vol = vol & 0x00FF;
}

/**
 * 持久化当前电台（频率和索引）
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
    IapProgramByte(addr_freq + 1, freq_array[1]);

    if (sys_radio_index != 0xFF)
    {
        IapProgramByte(addr_freq_index, sys_radio_index);
    }

    sys_freq = freq;
}

/**
 * 持久化睡眠模式
 */
void CONF_SET_SLEEP_RSSI()
{
    IapEraseSector(addr_sleep_mode);
    IapProgramByte(addr_sleep_mode, 0x00 | sys_sleep_mode);
}

void CONF_WRITE(void)
{
    if (sys_write_freq_flag)
    {
        CONF_SET_FREQ(sys_freq);
        sys_write_freq_flag = 0;
    }

    if (sys_write_vol_flag)
    {
        CONF_SET_VOL(sys_vol);
        sys_write_vol_flag = 0;
    }

    if (sys_write_sleep_flag)
    {
        CONF_SET_SLEEP_RSSI();
        sys_write_sleep_flag = 0;
    }
}

/**
 * 清空电台(包括频道号，和频率列表)
 */
void CONF_RADIO_ERASE(void)
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
    if (sys_vol < 0 | sys_vol > 15)
    {
        sys_vol = 5;
    }

    // 从eeprom获取睡眠模式纠正
    sys_sleep_mode = IapReadByte(addr_sleep_mode);
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
    if (sys_radio_index_max == 0xFF) // 没有搜过台
    {
        return 1; // 需要自动搜台
    }

    // 加载电台配置
    sys_freq = CONF_READ_RAIDO_FREQ(addr_freq);
    sys_radio_index = IapReadByte(addr_freq_index);
    // printf("read config sys_freq %d index %bu\r\n", sys_freq, sys_radio_index);

    return 0;
}
