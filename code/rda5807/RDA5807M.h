#ifndef _RDA5807M_H_
#define _RDA5807M_H_

#include "config/Config.h"

#define BAND_87_108 1
#define BAND_76_91 2
#define BAND_76_108 3
#define BAND_65_76 4
#define BAND_50_76 5

#define Space_100kHz 0
#define Space_200kHz 1
#define Space_50KHz 2
#define Space_25KHz 3

/**
 * @brief 初始化
 */
void RDA5807M_init(void);

/**
 * 设置频率 读取当前频率
 */
void RDA5807M_Set_Freq(uint16t Freq);

/**
 * @brief 读取当前频率
 * @param 无
 * @return 频率(以MHz为单位*100)(如108MHz=>10800)
 */
uint16t RDA5807M_Read_Freq(void);

/**
 * @brief 读取SNR
 * RDA5807M_Read_SNR
 */
uint8t RDA5807M_Read_SNR(void);
void RDA5807M_Set_SNR(uint8t snr);

/**
 * @brief 手动搜索电台（搜索完成后会设置当前频率为搜到的频率）
 * @param direction 搜索方向（1 频率增加 0 频率减）
 * @return 电台频率
 */
uint16t RDA5807M_Seek(uint8t direction);

/**
 * @brief 点前是否是电台
 * @return 1 = 是   0 = 否
 */
uint8t RDA5807M_Radio_TRUE(void);

/**
 * 搜索全部电台
 */
void RDA5807M_Search_Automatic(void);

/**
 * 设置音量大小
 * @param 1-15
 */
void RDA5807M_Set_Volume(uint8t vol);

/**
 * @brief 设置静音
 * @param Bool：0是静音，1是解除静音
 * @return 无
 */
void RDA5807M_SetMUTE(uint8t mute);

/**
 * 静音模式和非静音模式来回切换，静音不持久化
 */
void RDA5807M_CHANGE_MUTE();

/**
 * @brief 设置静音
 * @param mute：1是静音，0是解除静音
 * @return 无
 */
uint8t RDA5807M_SetMutea_status();

/**
 * 读取RSSI
 * RDA5807M_Read_RSSI
 */
uint8t RDA5807M_Read_RSSI(void);


/**
 * 关闭芯片
 */
void RDA5807M_OFF(void);

#endif
