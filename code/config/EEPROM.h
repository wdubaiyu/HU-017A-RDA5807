#ifndef __EEP_ROM__
#define __EEP_ROM__
#include "Config.h"

//位 (bit) ——8个——> 字节 (Byte) ——1024个——> 千字节 (KB)
//每个扇区512字节（扇区一起写）   总空间 5K = 5120 字节，
// 地址值应该是 0x0000 0x0200 0x0400 0x0600 0x0800 0x1000 0x1200 
// 最后一个地址是 0x13FF (5119)


/**
 * @brief 第一个扇区 0x0000-0x0200 存放动态数据
 *    最少占用一个字节
 * 
 */
// 存放音量（0-15 uint8_t占用1字节）0x0000
#define addr_vol 0x0000
// 存放当前index （uint8_t占用1字节） 0x0003
#define addr_freq_index 0x0003
// 模式存放 存放睡眠模式、存放POLL模式 （占用 uint8_t占用1字节 睡眠使用0bit。poll使用1bit）
#define addr_mode 0x0004



// 存放电台最大数量（0~254）
#define addr_radio 0x0600
// 存放SPACE BAND  (uint8_t占用1字节SPACE 低2位，BAND 高6位)
#define addr_h03 0x0601
// 存放电台频率开始（两个byte一个台，16位）
#define addr_radio_list 0x0602


/**
 * 擦除一个扇区
 */
void IapEraseSector(uint16_t addr);

/**
 * 读取一个字节
 * @param addr 读取地址
 */

uint8_t IapReadByte(uint16_t addr);

/**
 * 读取 指定连续sizeOf(dat)字节
 * @param addr 读取地址
 */

void IapReadArrayByte(uint16_t addr, uint8_t *dat);

/**
 * 不擦除 写一字节
 * @param addr 写入地址
 * @param dat 8位 一字节数据
 */
void IapProgramByte(uint16_t addr, uint8_t dat);

#endif
