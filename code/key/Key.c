#include <STC15.H>
#include <stdio.h>
#include "config/Config.h"
#include "Key.h"

// 按键引脚定义
sbit KEY1 = P1 ^ 6; // v+
sbit KEY2 = P1 ^ 7; // v-
sbit KEY3 = P1 ^ 5; // f+
sbit KEY4 = P1 ^ 4; // f-

// 按键状态枚举
typedef enum {
    KEY_STATE_IDLE = 0,      // 空闲状态
    KEY_STATE_PRESS,         // 按下状态（消抖中）
    KEY_STATE_SHORT,         // 短按确认
    KEY_STATE_LONG,          // 长按确认
    KEY_STATE_COMBINATION    // 组合按键
} KeyState;

// 按键数据结构
typedef struct {
    uint8t pin_num;          // 按键编号 1-4
    uint16t cnt;             // 计数器
    KeyState state;          // 当前状态
    uint8t key_num_short;    // 短按键值
    uint8t key_num_long;     // 长按键值
} Key_TypeDef;

// 读取按键引脚状态
static bit ReadKeyPin(uint8t pin_num)
{
    switch (pin_num) {
        case 1: return KEY1;
        case 2: return KEY2;
        case 3: return KEY3;
        case 4: return KEY4;
        default: return 1;
    }
}

// 按键表
static Key_TypeDef Keys[] = {
    {1, 0, KEY_STATE_IDLE, 1, 11},
    {2, 0, KEY_STATE_IDLE, 2, 22},
    {3, 0, KEY_STATE_IDLE, 3, 33},
    {4, 0, KEY_STATE_IDLE, 4, 44}
};

#define KEY_NUM  (sizeof(Keys) / sizeof(Keys[0]))

static volatile uint8t KeyNum = 0;       // 按键返回值
static volatile uint8t Key1LongPressed = 0;   // KEY1 长按标志（用于组合键）

/**
 * @brief  获取按键键码(获取清零)
 * @return 按键键值
 */
unsigned char POP_KEY(void)
{
    unsigned char temp = KeyNum;
    KeyNum = 0;
    return temp;
}

/**
 * @brief 检查组合按键
 * @return 组合键值 (12,13,14) 或 0
 */
static uint8t CheckCombinationKeys(void)
{
    if (Key1LongPressed) {
        if (!KEY2) return 12;
        if (!KEY3) return 13;
        if (!KEY4) return 14;
    }
    return 0;
}

/**
 * @brief 按键驱动函数，在中断中调用
 */
void Key_Loop(void)
{
    uint8t i;
    uint8t combo_key;
    
    // 先处理 KEY1 的特殊逻辑（支持组合键）
    if (Keys[0].state == KEY_STATE_LONG && Key1LongPressed) {
        // 检查是否有组合按键触发
        combo_key = CheckCombinationKeys();
        if (combo_key) {
            KeyNum = combo_key;
            Key1LongPressed = 0;
            // 锁定所有按键，等待释放
            for (i = 0; i < KEY_NUM; i++) {
                Keys[i].state = KEY_STATE_COMBINATION;
            }
            return;
        }
    }
    
    // 处理每个按键
    for (i = 0; i < KEY_NUM; i++) {
        Key_TypeDef *key = &Keys[i];
        bit pin_val = ReadKeyPin(key->pin_num);
        
        switch (key->state) {
            case KEY_STATE_IDLE:
                if (!pin_val) {  // 按键按下
                    key->cnt = 0;
                    key->state = KEY_STATE_PRESS;
                }
                break;
                
            case KEY_STATE_PRESS:
                if (pin_val) {  // 按键释放，消抖失败
                    key->state = KEY_STATE_IDLE;
                } else {
                    if (++key->cnt >= KEY_DELAY_TIME) {
                        key->state = KEY_STATE_SHORT;
                    }
                }
                break;
                
            case KEY_STATE_SHORT:
                if (pin_val) {  // 按键释放，短按确认
                    KeyNum = key->key_num_short;
                    key->state = KEY_STATE_IDLE;
                    if (i == 0) Key1LongPressed = 0;  // KEY1 短按，清除长按标志
                } else {
                    if (++key->cnt >= KEY_LONG_TIME) {
                        key->state = KEY_STATE_LONG;
                        if (i == 0) {
                            Key1LongPressed = 1;  // KEY1 长按，标记可组合
                        } else {
                            // 其他按键长按直接触发
                            KeyNum = key->key_num_long;
                            key->state = KEY_STATE_COMBINATION;  // 锁定，等待释放
                        }
                    }
                }
                break;
                
            case KEY_STATE_LONG:
                if (pin_val) {  // KEY1 长按后释放
                    if (Key1LongPressed && !CheckCombinationKeys()) {
                        // 没有组合键，触发 KEY1 长按
                        KeyNum = key->key_num_long;
                    }
                    Key1LongPressed = 0;
                    key->state = KEY_STATE_IDLE;
                }
                break;
                
            case KEY_STATE_COMBINATION:
                // 组合键状态，等待所有按键释放
                if (pin_val) {
                    // 检查是否所有按键都释放了
                    uint8t all_released = 1;
                    uint8t j;
                    for (j = 0; j < KEY_NUM; j++) {
                        if (!ReadKeyPin(Keys[j].pin_num)) {
                            all_released = 0;
                            break;
                        }
                    }
                    if (all_released) {
                        for (j = 0; j < KEY_NUM; j++) {
                            Keys[j].state = KEY_STATE_IDLE;
                        }
                        Key1LongPressed = 0;
                    }
                }
                break;
                
            default:
                key->state = KEY_STATE_IDLE;
                break;
        }
    }
}
