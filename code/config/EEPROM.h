#ifndef __EEP_ROM__
#define __EEP_ROM__
#include "Config.h"

// 每个扇区512字节
#define addr_vol 0x0000
// 存放当前频率

#define addr_freq 0x0200
// 存放当前index
#define addr_freq_index 0x0202

// 存放睡眠模式
#define addr_sleep_mode 0x0400
// 存放POLL模式
#define addr_poll_mode 0x0402

// 存放电台最大数量
#define addr_radio 0x0600
// 存放电台频率开始（两个byte一个台）
#define addr_radio_list 0x0602

/**
 * 擦除一个扇区
 */
void IapEraseSector(uint16t addr);

/**
 * 读取一个字节
 * @param addr 读取地址
 */

uint8t IapReadByte(uint16t addr);

/**
 * 读取 指定连续sizeOf(dat)字节
 * @param addr 读取地址
 */

void IapReadArrayByte(uint16t addr, uint8t *dat);

/**
 * 不擦除 写一字节
 * @param addr 写入地址
 * @param dat 8位 一字节数据
 */
void IapProgramByte(uint16t addr, uint8t dat);

#endif
