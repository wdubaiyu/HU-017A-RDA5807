#include "led/myLed.h"
#include "../config/Config.h"
#include "../delay/Delay.h"
#include "../led/74HC595.h"
#include "stc15.h"
#include "stdio.h"

//,0xC7 -->L11  0x89-->H12
xdata uint8_t NixieTable[] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, // (0)0,1,2,3,4
                              0x92, 0x82, 0xF8, 0x80, 0x90,
                              0x7F, // (5)5,6,7,8,9,点(10)
                              0x88, 0x83, 0xC6, 0xA1, 0x86, // (11)A,b,C,d,E
                              0x8E, 0x8C, 0xC1, 0xCE, 0x91, // (16)F,P,U,T,y
                              0x89, 0xC7, 0x12, 0xC8, 0xAB, // (21)L,H,S,N,n
                              0xF7, 0xBF};                  // (26)_ -

xdata uint8_t NixieTableDp[] = {0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02,
                                0x78, 0x00, 0x10, 0x7F, 0xC7, 0x89};

uint16_t LED_FRE_REAL = 8700;
uint16_t LED_DISPLAY_REC_COUNT = 0x00;
uint8_t LED_DISPLAY_TYPE = 0x06;
uint8_t LED_RSSI = 0x00;
uint8_t LED_SNR = 0x00;
uint8_t LED_TIMED_STANDBY = 0x1E; // 默认30分钟
// 手动搜台方向
bit LED_SEEK_D = 1;
// 睡眠模式剩余时间
uint16_t LED_SHOW_TIME = 0x1F40; // 统计睡眠模式剩余时间8s 8000

/**
 内部使用 根据dpf判断是否需要小数点
**/
char getData(uint8_t a, dpf) {
  if (dpf) {
    return NixieTableDp[a];
  } else {
    return NixieTable[a];
  }
}

/**
 *显示数字 内部使用
 */
void CallNixieTube(uint8_t a, b, c, d, dp) {
  // 数码管显示位数轮询（0~3）
  static uint8_t LED_POLLING_POSTITION = 0;
  uint8_t sizeOfNixie = sizeof(NixieTable);
  uint8_t dat;
  bit show;

  P20 = P21 = P22 = P23 = 1; // 先消隐

  if (LED_POLLING_POSTITION == 0) {
    dat = a;
    show = (dat < sizeOfNixie);
  } else if (LED_POLLING_POSTITION == 1) {
    dat = b;
    show = (dat < sizeOfNixie);
  } else if (LED_POLLING_POSTITION == 2) {
    dat = c;
    show = (dat < sizeOfNixie);
  } else if (LED_POLLING_POSTITION == 3) {
    dat = d;
    show = (dat < sizeOfNixie);
  } else {
    show = 0;
  }

  if (show) {
    _74HC595_WriteByte(getData(dat, dp == (LED_POLLING_POSTITION + 1)));
    // 写完数据后再打开对应位选
    if (LED_POLLING_POSTITION == 0) { P20 = 0; }
    else if (LED_POLLING_POSTITION == 1) { P21 = 0; }
    else if (LED_POLLING_POSTITION == 2) { P22 = 0; }
    else if (LED_POLLING_POSTITION == 3) { P23 = 0; }
  }

  if (++LED_POLLING_POSTITION > 3) {
    LED_POLLING_POSTITION = 0;
  }
}

// 显示睡眠模式
void DispaySELLP() {
  if (sys_sleep_mode) {
    CallNixieTube(0x17, 0xFF, 0xFF, 21, 0xFF);
  } else {
    CallNixieTube(0x17, 0xFF, 0xFF, 22, 0xFF);
  }
}

// 显示频率
void DispayF(uint16_t temp) {
  uint8_t NUM_GE, NUM_SHI, NUM_BAI, NUM_DEC;
  if (temp == 0) {
    CallNixieTube(0x10, 0x10, 0x10, 0x10, 0xFF);
    return;
  }
  NUM_BAI = temp / 10000;
  NUM_SHI = (temp % 10000) / 1000;
  NUM_GE = (temp % 1000) / 100;
  NUM_DEC = (temp % 100) / 10;
  if (NUM_BAI < 1) {
    CallNixieTube(100, NUM_SHI, NUM_GE, NUM_DEC, 3);
  } else {
    CallNixieTube(NUM_BAI, NUM_SHI, NUM_GE, NUM_DEC, 3);
  }
}
// 显示频率(loop调用时候显示为递增效果)
void DispayFRE(void) {
  if (LED_FRE_REAL == sys_freq) {
    DispayF(LED_FRE_REAL);
    return;
  }

  // 正向  数码管频率在增加的效果
  if (LED_SEEK_D) {
    ++LED_FRE_REAL;
    if (LED_FRE_REAL > 10800) {
      LED_FRE_REAL = 8700;
    }
  } else {
    --LED_FRE_REAL;
    if (LED_FRE_REAL < 8700) {
      LED_FRE_REAL = 10800;
    }
  }

  DispayF(LED_FRE_REAL);
}

// 显示音量
void DispayVl() { CallNixieTube(0xFF, sys_vol / 10, sys_vol % 10, 0xFF, 0xFF); }

// 显示信号质量
void DispayRSSI() {
  uint8_t NUM_GE, NUM_SHI, NUM_BAI;
  NUM_BAI = LED_RSSI / 100;
  NUM_SHI = (LED_RSSI % 100) / 10;
  NUM_GE = (LED_RSSI % 10);
  if (NUM_BAI) {
    CallNixieTube(0xFF, NUM_BAI, NUM_SHI, NUM_GE, 0xFF);
  } else { // 百位不显示0，大多数情况下百位都达不到1
    CallNixieTube(0xFF, 0xFF, NUM_SHI, NUM_GE, 0xFF);
  }
}

void DispaySNR() {
  uint8_t NUM_GE, NUM_SHI;
  NUM_GE = (LED_SNR % 10);
  NUM_SHI = (LED_SNR % 100) / 10;
  CallNixieTube(0x17, 25, NUM_SHI, NUM_GE, 0xFF);
}

void DispayTimedStandby() {
  uint8_t NUM_GE, NUM_SHI, NUM_BAI;
  NUM_BAI = LED_TIMED_STANDBY / 100;
  NUM_SHI = (LED_TIMED_STANDBY % 100) / 10;
  NUM_GE = (LED_TIMED_STANDBY % 10);
  if (NUM_BAI) {
    // 22 对应led显示屏幕 L
    CallNixieTube(22, NUM_BAI, NUM_SHI, NUM_GE, 0xFF);
  } else { // 22 对应led显示屏幕 L 0xFF(百位不显示0)
    CallNixieTube(22, 0xFF, NUM_SHI, NUM_GE, 0xFF);
  }
}

void DispayEEPROMReset() { CallNixieTube(0x1a, 0x1a, 0x1b, 0x1b, 0xFF); }

void DispayBand() {
  uint8_t NUM_GE, NUM_SHI;
  NUM_SHI = sys_band / 10;
  NUM_GE = sys_band % 10;
  CallNixieTube(12, 0xFF, NUM_SHI, NUM_GE, 0xFF);
}

void DispalyConfirm() { CallNixieTube(0xFF, 0xFF, 24, 0, 0xFF); }

void DispalCancel() { CallNixieTube(0xFF, 0xFF, 26, 26, 0xFF); }

// 展示 切换rssi轮询开关
void DispayPOLL() { CallNixieTube(0x17, 0xFF, 0xFF, cycle_in_freq_rssi, 0xFF); }

// 展示 ----
void DispayHyphen() { CallNixieTube(0x1B, 0x1B, 0x1B, 0x1B, 0xFF); }

// 显示内容
void DISPLY() {
  if (LED_DISPLAY_TYPE == 10)
    DispayFRE(); // 展示频率
  if (LED_DISPLAY_TYPE == 0)
    DispalCancel(); // 显示OFF
  if (LED_DISPLAY_TYPE == 1)
    DispalyConfirm(); // 显示NO
  if (LED_DISPLAY_TYPE == 2)
    DispayRSSI(); // 展示信号强度
  if (LED_DISPLAY_TYPE == 3)
    DispaySELLP(); // 展示睡眠模式
  if (LED_DISPLAY_TYPE == 4)
    DispayVl(); // 音量
  if (LED_DISPLAY_TYPE == 5)
    DispayPOLL(); // 展示POLL
  if (LED_DISPLAY_TYPE == 6)
    DispayHyphen(); // 展示hyphen ----

  if (LED_DISPLAY_TYPE == 101)
    DispaySNR(); // 展示SNR
  if (LED_DISPLAY_TYPE == 102)
    DispayTimedStandby(); // 展示定时待机
  if (LED_DISPLAY_TYPE == 103)
    DispayEEPROMReset(); // 展示EEPROM复位
  if (LED_DISPLAY_TYPE == 104)
    DispayBand(); // 展示band设置
}

void LED_CHANGE_SLEEP_MODE() {
  // 设置省电模式显示
  LED_SET_DISPLY_TYPE(3);
  sys_sleep_mode = ~sys_sleep_mode;
}

// 睡眠模式剩余时间重置
void LED_RESET_SLEEP_TIME() {
  if (sys_sleep_mode == 0) {
    LED_SHOW_TIME = 0x1F40; // 8秒8000
  }
}

// 判断是否显示
bit Led_Loop() {
  // return 1;
  // sys_sleep_mode =1 一直显示 （LED_DISPLAY_TYPE
  // 大于100时是出发了功能设置需要一直显示）
  if (sys_sleep_mode || LED_DISPLAY_TYPE > 100) {
    DISPLY();
  }
  // 睡眠倒计时
  else if (LED_SHOW_TIME > 10) {
    DISPLY();
    LED_SHOW_TIME--;
  }
  // 不显示关闭数量管电源
  else {
    if (LED_SHOW_TIME != 5) // 写一个特殊值5 用于一次性关闭数码管显示
    {
      LED_SHOW_TIME = 5;
      P20 = P21 = P22 = P23 = 1;
    }

    return 1;
  }
  return 0;
}

void LED_SET_DISPLY_TYPE(uint8_t display_type) {
  LED_DISPLAY_REC_COUNT = 0x00;
  if (LED_DISPLAY_TYPE == display_type) {
    return;
  }

  LED_DISPLAY_TYPE = display_type;
}

uint8_t LED_GET_DISPLY_TYPE() { return LED_DISPLAY_TYPE; }

void LED_TIMED_STANDBY_U() {
  if (LED_TIMED_STANDBY < 20) {
    LED_TIMED_STANDBY += 5;
  } else if (LED_TIMED_STANDBY < 60) {
    LED_TIMED_STANDBY += 10;
  } else if (LED_TIMED_STANDBY < 230) {
    LED_TIMED_STANDBY += 20;
  }
  LED_SET_DISPLY_TYPE(102);
}

void LED_TIMED_STANDBY_D() {

  if (LED_TIMED_STANDBY > 60) {
    LED_TIMED_STANDBY -= 20;
  } else if (LED_TIMED_STANDBY > 20) {
    LED_TIMED_STANDBY -= 10;
  } else if (LED_TIMED_STANDBY >= 5) {
    LED_TIMED_STANDBY -= 5;
  }
  LED_SET_DISPLY_TYPE(102);
}