#ifndef __KEY_H__
#define __KEY_H__

#define KEY_DELAY_TIME 20                          // 短按消抖
#define KEY_LONG_TIME 1000                         // 长按触发
#define COMBO_TRIGGER_DELAY 400                    // 组合按键记次基准值（组合按键计时器默认值，当组合按键一直长按时获取键值后置为零）
#define COMBO_DELAY_TIME 100 + COMBO_TRIGGER_DELAY // 组合按键触发添加延迟

/**

 */

/**
 * @brief  获取按键键码(获取清零)
 * @return 按键键值
 * 1 V+
 * 2 V-
 * 3 F+
 * 4 F-
 * 支持组合按键（同时长按，建议先按下v+）
 * 12,13,14
 * 支持长按
 * 11,22,33,44
 */
unsigned char POP_KEY(void);

/**
 * 轮询查询键盘
 */
void Key_Loop(void);

#endif
