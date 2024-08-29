#ifndef __MY_LED__
#define __MY_LED__

#include "Config/Config.h"

extern uint16t LED_FRE_REAL;
extern uint8t DISPLAY_type;
extern uint8t LED_RSSI;
extern uint8t LED_SNR;
extern bit LED_SEEK_D;
extern bit LED_HAND_MARK;

// 显示数码管
void Led_Loop();

// les 睡眠时间重置
void resetSleepTime();
void Led_CHANGE_SLEEP_MODE();
#endif
