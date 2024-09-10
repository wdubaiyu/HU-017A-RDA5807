#ifndef __MY_LED__
#define __MY_LED__

#include "Config/Config.h"
#define LED_REC_TIME 0x7D0

extern uint16t LED_FRE_REAL;
extern uint16t LED_DISPLAY_REC_COUNT;
extern uint8t LED_RSSI;
extern uint8t LED_SNR;
extern uint8t LED_TIMED_STANDBY;
extern bit LED_SEEK_D;

// 显示数码管
void Led_Loop();

// les 睡眠时间重置
void LED_RESET_SLEEP_TIME();
void LED_CHANGE_SLEEP_MODE();

//修改显示类型
void LED_SET_DISPLY_TYPE(uint8t display_type);
uint8t LED_GET_DISPLY_TYPE();

// 省电模式
void LED_TIMED_STANDBY_U(); //+
void LED_TIMED_STANDBY_D(); //-
#endif
