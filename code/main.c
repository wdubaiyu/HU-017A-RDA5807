#include "config/Config.h"
#include "delay/Delay.h"
#include "key/key.h"
#include "led/myLed.h"
#include "rda5807/I2C.h"
#include "rda5807/RDA5807M.h"
#include "stc15.h"
#include "stdio.h"
#include "time/time.h"
#include "uart/UART.h"

// （自动搜台设置==11）
// （定时关机设置==21）
// （设置band==31）
// （设置重置EEPROM==41）
uint8_t key_function_flag;

// 电源状态（0正常，1关机倒计时，2已关机）
uint8_t POWER_STATUS = 0x00;
uint16_t timed_stanby_count;
// 写动态配置(是否需要保存配置到EEPROM) =0x0FD0 时触发写操作，<0x0FD0时计数加一
uint16_t timed_config_write = 0xFFFF;

bit rssi_read_flag;

// 触发写配置（延迟约4秒后执行）
//重置计数器为0 从头开始计数
void trigger_write() { timed_config_write = 0x0000; }
/**
 * 启动收音函数
 */
void InitSystem() {
  uint8_t conf_select = 0;
  POWER_STATUS = 0x00;
  Timer2_Stop();
  // 读取系统持久化配置，返回是否需要自动搜台
  conf_select = CONF_SYS_INIT();
  key_function_flag = 0x00;
  LED_TIMED_STANDBY = 0x1E;

  // printf("conf_select %bu \r\n", conf_select);
  // 打开数码管显示和按键扫描
  Timer0_Init();
  EA = 1;

  if (conf_select == 1) {
    // 配置band
    key_function_flag = 31;
    LED_SET_DISPLY_TYPE(104);
    return;
  }

  // 初始化收音机
  Delay(300); // 等待收音机芯片上电稳定后再初始化
  RDA5807M_init();

  LED_RESET_SLEEP_TIME();

  if (conf_select == 2) { // 自动收台
    RDA5807M_Search_Automatic();
  }
  // 播放上次关机时的电台
  LED_FRE_REAL = sys_freq;
  LED_SET_DISPLY_TYPE(10);
}

// 按键触发功能
void userInput(uint8_t Key_num) {
  // 有按键操作时重置睡眠时间
  LED_RESET_SLEEP_TIME();

  if (key_function_flag > 10) // 功能菜单触发后不响应基础操作
  {
    // 触发功能后按了V+
    if (Key_num == 1) {
      if (key_function_flag == 11) {
        LED_SNR = RDA5807M_Read_SNR();
        if (LED_SNR == 15) {
          LED_SNR = 0;
        } else {
          LED_SNR++;
        }
        RDA5807M_Set_SNR(LED_SNR);
      }

      if (key_function_flag == 21) {
        LED_TIMED_STANDBY_U();
      }

      if (key_function_flag == 31) {
        if (sys_band < 4) {
          sys_band++;
        }
      }

      return;
    }

    // 触发功能后按了V-
    if (Key_num == 2) {
      if (key_function_flag == 11) {
        LED_SNR = RDA5807M_Read_SNR();
        if (LED_SNR == 0) {
          LED_SNR = 15;
        } else {
          LED_SNR--;
        }
        RDA5807M_Set_SNR(LED_SNR);
      }

      if (key_function_flag == 21) {
        LED_TIMED_STANDBY_D();
      }

      if (key_function_flag == 31) {
        if (sys_band > 0) {
          sys_band--;
        }
      }
      return;
    }

    // 触发功能后按了F+  (取消)
    if (Key_num == 3) {
      //		    自动收台不需要调整
      //			if (key_function_flag == 11)
      //			{
      //			}

      if (key_function_flag == 21) {
        POWER_STATUS = 0;
      }

      if (key_function_flag == 31) {
        return;
      }

      // 取消功能直接展示频率
      LED_SET_DISPLY_TYPE(0);
      key_function_flag = 0;
      return;
    }

    // 触发功能后按了F- （确认）
    if (Key_num == 4) {
      if (key_function_flag == 11) {
        RDA5807M_Search_Automatic();
      }

      if (key_function_flag == 21) {
        // 定时关机大于0才能确认
        if (LED_TIMED_STANDBY > 0) {
          POWER_STATUS = 1;
          timed_stanby_count = 0;
          Timer2_Init(); // 定时器开始计时
          LED_SET_DISPLY_TYPE(1);
        } else {
          POWER_STATUS = 0;

          LED_SET_DISPLY_TYPE(0);
        }
      }

      if (key_function_flag == 31) {
        //  band设置
        CONF_RADIO_ERASE();
        InitSystem();
        return;
      }

      if (key_function_flag == 41) {
        //  重置eeprom
        RDA5807M_OFF();
        CONF_RESET();
        InitSystem();
        return;
      }

      key_function_flag = 0;
      return;
    }

    // 如果触发了功能则不进行下面的 操作
    return;
  }

  ////////////////////功能触发区///////////////////

  // K11 设置定时关机
  if (Key_num == 11) {
    key_function_flag = 21;
    // 设置定时关机时，取消上一次设定
    POWER_STATUS = 0;
    Timer2_Stop(); // 定时器停止计时
    LED_SET_DISPLY_TYPE(102);
    return;
  }

  // K12 设置省电模式（一定时间后关闭数码管）
  if (Key_num == 12) {
    LED_CHANGE_SLEEP_MODE();
    trigger_write();
    return;
  }

  // K13 自动搜台 触发
  if (Key_num == 13) {
    LED_SNR = RDA5807M_Read_SNR();
    key_function_flag = 11;
    LED_SET_DISPLY_TYPE(101); // 显示snr设置
    return;
  }

  // K14 切换POLL显示
  if (Key_num == 14) {
    cycle_in_freq_rssi = ~cycle_in_freq_rssi;
    LED_SET_DISPLY_TYPE(5);
    trigger_write();
    return;
  }

  // K22 V-长按
  if (Key_num == 22) {
    LED_SET_DISPLY_TYPE(4); // 数码管显示音量
    RDA5807M_SET_MUTE();
    return;
  }

  /////////////////常用操作区域
  // K33 手动搜下一个台
  if (Key_num == 33) {
    sys_freq = RDA5807M_Seek(1);
    return;
  }

  // K44 手动搜上一个台
  if (Key_num == 44) {
    sys_freq = RDA5807M_Seek(0);
    return;
  }

  // K34 回复出厂设置
  if (Key_num == 34) {
    key_function_flag = 41;
    LED_SET_DISPLY_TYPE(103); // 显示恢复出厂设置确认
    return;
  }

  // K3 V+
  if (Key_num == 1) {
    LED_SET_DISPLY_TYPE(4); // 数码管显示音量
    // 最大音量15
    if (sys_vol < 15) {
      RDA5807M_Set_Volume(sys_vol + 1);
      trigger_write();
    }
    return;
  }
  // K4 V-
  if (Key_num == 2) {
    LED_SET_DISPLY_TYPE(4); // 数码管显示音量
    // 最小音量1
    if (sys_vol > 0) {
      RDA5807M_Set_Volume(sys_vol - 1);
      trigger_write();
    }
    return;
  }

  // k2 F+
  if (Key_num == 3) {
    if (sys_radio_index_max == 0) {
      return;
    }

    if (sys_radio_index == sys_radio_index_max) {
      sys_radio_index = 0;
    } else {
      ++sys_radio_index;
    }

    LED_FRE_REAL = CONF_GET_FREQ_BY_INDEX(sys_radio_index);
    RDA5807M_Set_Freq(LED_FRE_REAL);
    trigger_write();
    // printf("sys_freq  %bu  %d\r\n", sys_radio_index, sys_freq);
    return;
  }
  // k1 F-
  if (Key_num == 4) {
    if (sys_radio_index_max == 0) {
      return;
    }

    if (sys_radio_index == 0) {
      sys_radio_index = sys_radio_index_max;
    } else {
      --sys_radio_index;
    }

    LED_FRE_REAL = CONF_GET_FREQ_BY_INDEX(sys_radio_index);
    RDA5807M_Set_Freq(LED_FRE_REAL);
    trigger_write();
    // printf("sys_freq  %bu  %d\r\n", sys_radio_index, sys_freq);
    return;
  }
}

void main() {
  uint8_t Key_num;

  I2C_Init();
  // UartInit();
  // printf("UartInit...\r\n");
  InitSystem();

  while (1) {

    // 是否切换到显示RSSI
    if (rssi_read_flag) {
      LED_RSSI = RDA5807M_Read_RSSI();
      LED_SET_DISPLY_TYPE(2);
      LED_DISPLAY_REC_COUNT = LED_REC_TIME / 2;
      rssi_read_flag = 0; // 重置标记
    }

    // 读取用户按键输入
    Key_num = POP_KEY();

    // 在关机中
    if (POWER_STATUS == 2) {
      // 关机中长按F-开机
      if (Key_num == 44) {
        InitSystem();
      }

      continue;
    }

    // 关机时间到
    if (POWER_STATUS == 1 && LED_TIMED_STANDBY < 1) {
      POWER_STATUS = 2;
      Timer2_Stop(); // 定时器停止计时
      RDA5807M_OFF();
      P20 = P21 = P22 = P23 = 1; // 关闭数码管
      continue;
    }

    // 按键有效,响应用户操作
    if (Key_num) {
      // printf("Key_num.. %d \n", (int)Key_num);
      userInput(Key_num);
    }
  }
}

void Timer0_Isr(void) interrupt 1 {

  uint8_t led_type = LED_GET_DISPLY_TYPE();

  // 写动态配置计数
  if (timed_config_write < 0x0FD0) {
    timed_config_write += 1;
  } else if (timed_config_write == 0x0FD0) {
    // 已处理写配置
    CONF_WRITE();
    timed_config_write = 0xFFFF;
  }


  // 轮询按键
  Key_Loop();

  // 不是关机状态才显示数码管
  if (POWER_STATUS < 2) {
    // 数码管关闭不执行和数码管相关的操作
    if (Led_Loop()) {
      TL0 = 0x66; // 设置定时初始值
      TH0 = 0xFC; // 设置定时初始值
      return;
    }

    // 数码管处于显示中，需要切换为freq显示
    if (led_type != 10) // 10是频率显示
    {
      if (led_type < 100) // 是否需要显示恢复为频率
      {
        if (++LED_DISPLAY_REC_COUNT >= LED_REC_TIME) {
          LED_SET_DISPLY_TYPE(10);
          // 数码管恢复显示时触发持久化操作（不包括FREQ——index）
        }
      }
    } else if (cycle_in_freq_rssi) // 显示的是10频率，开启了rssi轮询显示
    {
      if (++LED_DISPLAY_REC_COUNT >= LED_REC_TIME) {
        // 打开读取rssi功能
        rssi_read_flag = 1;
      }
    }
  }

  TL0 = 0x66; // 设置定时初始值
  TH0 = 0xFC; // 设置定时初始值
}

// 定时关机功能开启,循环减时间
void Timer2_Isr(void) interrupt 12 {
  if (POWER_STATUS == 1) {
    if (++timed_stanby_count >= 3000) {
      LED_TIMED_STANDBY -= 1; // 减去一分钟
      timed_stanby_count = 0; // 重新计数
    }
  }
}