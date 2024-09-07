#ifndef __MY_LED__
#define __MY_LED__

#include "Config/Config.h"

extern uint16t LED_FRE_REAL;
extern uint16t LED_DISPLAY_TYPE_REC;
extern uint8t LED_RSSI;
extern uint8t LED_SNR;
extern uint8t LED_TIMED_STANDBY;
extern bit LED_SEEK_D;

// 显示数码管
void Led_Loop();

// les 睡眠时间重置
void resetSleepTime();
void LED_CHANGE_SLEEP_MODE();
bit LED_NEET_DISPLY_REC();
void LED_SET_DISPLY_TYPE(uint8t display_type);

// 省电模式
void LED_TIMED_STANDBY_U(); //+
void LED_TIMED_STANDBY_D(); //-
#endif
