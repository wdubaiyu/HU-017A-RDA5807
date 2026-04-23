#ifndef _RDA5807M_H_
#define _RDA5807M_H_

#include "../type.h"

/**
 * @brief 初始化
 */
void RDA5807M_init(void);

/**
 * 设置频率
 */
void RDA5807M_Set_Freq(uint16_t Freq);

/**
 * @brief 读取当前频率
 * @param 无
 * @return 频率(以MHz为单位*100)(如108MHz=>10800)
 */
uint16_t RDA5807M_Read_Freq(void);

/**
 * @brief 读取SNR
 * RDA5807M_Read_SNR
 */
uint8_t RDA5807M_Read_SNR(void);

void RDA5807M_Set_SNR(uint8_t snr);

uint16_t SEEK(uint8_t direction, bit round);
/**
 * @brief 手动搜索电台（搜索完成后会设置当前频率为搜到的频率）
 * @param direction 搜索方向（1 频率增加 0 频率减）
 * @return 电台频率
 */
uint16_t RDA5807M_Seek(uint8_t direction);

/**
 * @brief 点前是否是电台
 * @return 1 = 是   0 = 否
 */
bit RDA5807M_Radio_TRUE(void);

/**
 * 搜索全部电台
 */
void RDA5807M_Search_Automatic(void);

/**
 * @brief 设置音量/解除静音 解除静音时不调整音量
 * @param Val:音量值(0-15)
 * @return 无
 */
void RDA5807M_Set_Volume(uint8_t vol);

/**
 * 打开静音模式
 */
void RDA5807M_SET_MUTE();


/**
 * 读取RSSI
 * RDA5807M_Read_RSSI
 */
uint8_t RDA5807M_Read_RSSI(void);


/**
 * 关闭芯片
 */
void RDA5807M_OFF(void);

#endif
