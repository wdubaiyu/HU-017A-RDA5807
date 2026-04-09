#include <STC15.H>
#include "config/Config.h"
#include <stdio.h>
#include "Key.h"

unsigned char KeyNum = 0;

sbit KEY1 = P1 ^ 6; // v+
sbit KEY2 = P1 ^ 7; // v-
sbit KEY3 = P1 ^ 5; // f+
sbit KEY4 = P1 ^ 4; // f-


// 按键状态结构体 (移到xdata节省DATA空间)
typedef struct
{
    uint8t state;           // 当前状态 (0=释放, 1=按下)
    uint8t lock;            // 锁定标志
    uint8t in_combo;        // 参与组合按键标志
    uint8t combo_triggered; // 组合按键已触发标志
    uint16t cnt;            // 按键计时
} KeyState;

xdata KeyState keys[4]; // 移至xdata区

// 组合按键计时器 (移至xdata)
xdata uint16t combo_timer;
// 组合按键标志
xdata uint8t combo_flags;

/**
 * @brief  获取按键键码(获取清零)
 * @param  无
 * @retval 按下按键的键码，范围：0,1~4,0表示无按键按下
 */
unsigned char POP_KEY(void)
{
	unsigned char temp = 0;
	temp = KeyNum;
	KeyNum = 0;
	return temp;
}


/**
 * @brief  按键扫描函数
 */
void Key_Loop(void)
{
    // 所有局部变量在函数开头声明
    uint8t i;             // 处理第几位的按键
    uint8t pin_state;     // 当前按键状态
    uint8t pressed_keys;  // 统计哪些按键参加了组合按键
    uint8t pressed_count; // 统计几个按键参与了组合按键
    uint8t key1;
    uint8t key2;
    uint8t any_key_released = 0;
    uint8t still_pressed;


    // 1. 更新按键状态
    for (i = 0; i < 4; i++)
    {
        // 读取按键状态(0按下，1释放)
        switch (i)
        {
        case 0:
            pin_state = KEY1;
            break;
        case 1:
            pin_state = KEY2;
            break;
        case 2:
            pin_state = KEY3;
            break;
        case 3:
            pin_state = KEY4;
            break;
        default:
            pin_state = 1;
            break;
        }

        if (pin_state) // 按键释放
        {
            if (keys[i].state) // 之前是按下状态（1）
            {
                keys[i].state = 0;

                // 如果按键参与了组合，清除标志但不触发短按
                if (keys[i].in_combo)
                {
                    keys[i].in_combo = 0;
                    keys[i].combo_triggered = 0; // 清除组合触发标志
                }
                // 短按检测 (未锁定且未参与组合)
                else if (keys[i].cnt > KEY_DELAY_TIME &&
                         keys[i].cnt <= KEY_LONG_TIME &&
                         !combo_flags)
                {
                    KeyNum = i + 1; // 短按键值 1,2,3,4
                }

                keys[i].lock = 0;
                any_key_released = 1; // 标记有按键释放
            }
            keys[i].cnt = 0;
        }
        else
        { // 按键按下
            if (!keys[i].state)
            { // 新按下
                keys[i].state = 1;
                keys[i].cnt = 0;
                keys[i].in_combo = 0; // 清除组合标志

                // 如果另一个键还在组合状态，准备重新检测组合
                if (combo_flags && !keys[i].combo_triggered)
                {
                    keys[i].in_combo = 1; // 标记参与组合
                }
            }

            // 更新按键计时
            if (keys[i].cnt < 0xFFFF)
                keys[i].cnt++;

            // 长按检测 (无组合时)
            if (keys[i].cnt > KEY_LONG_TIME && !keys[i].lock && !combo_flags)
            {
                KeyNum = (i + 1) * 10 + (i + 1); // 11,22,33,44
                keys[i].lock = 1;                // 锁定防止重复触发
            }
        }
    }

    // 2. 如果有按键释放，重置组合状态
    if (any_key_released)
    {
        combo_flags = 0;
        combo_timer = COMBO_TRIGGER_DELAY;

        // 检查是否还有按键处于组合状态
        still_pressed = 0;
        for (i = 0; i < 4; i++)
        {
            if (keys[i].in_combo)
            {
                still_pressed |= (1 << i);
            }
        }

        // 如果有按键仍然处于组合状态，重新开始组合检测
        if (still_pressed)
        {
            combo_flags = still_pressed;
            combo_timer = COMBO_TRIGGER_DELAY;
        }
    }

    // 3. 组合按键检测(统计组合按键)
    pressed_keys = 0;
    pressed_count = 0;
    for (i = 0; i < 4; i++)
    {
        if (keys[i].state)
        {
            pressed_keys |= (1 << i);
            pressed_count++;
        }
    }

    if (pressed_count >= 2)
    { // 至少两个按键按下
        if (!combo_flags)
        {
            // 开始组合按键检测
            combo_flags = pressed_keys;
            combo_timer = COMBO_TRIGGER_DELAY;
        }
        else if (combo_timer < 0xFFFF)
        {
            combo_timer += 1;
        }

        // 组合按键触发 (消抖后并添加延迟)
        if (combo_timer > KEY_DELAY_TIME + COMBO_DELAY_TIME)
        {
            // 找出两个按下的按键
            key1 = 0;
            key2 = 0;
            for (i = 0; i < 4; i++)
            {
                if (combo_flags & (1 << i))
                {
                    if (!key1)
                    {
                        key1 = i + 1;
                    }
                    else if (!key2)
                    {
                        key2 = i + 1;
                    }
                }
            }

            // 生成组合键值 (小索引在前)
            if (key1 && key2)
            {
                KeyNum = (key1 < key2) ? (key1 * 10 + key2) : (key2 * 10 + key1);

                // 标记参与组合的按键
                for (i = 0; i < 4; i++)
                {
                    if (combo_flags & (1 << i))
                    {
                        keys[i].in_combo = 1;        // 标记参与组合
                        keys[i].combo_triggered = 1; // 标记已触发
                        keys[i].lock = 1;            // 锁定防止长按重复
                    }
                }

                // 不清除组合标志，允许重新检测
                combo_timer = 0; // 重置计时器(组合按键连续触发频率降低)
            }
        }
    }
    else
    {
        // 少于两个按键按下，清除组合状态
        combo_flags = 0;
        combo_timer = COMBO_TRIGGER_DELAY;
    }
}

/**
 * 按键初始化
 */
void Key_Init(void)
{
    uint8t i;
    combo_timer = COMBO_TRIGGER_DELAY;
    combo_flags = 0;
    for (i = 0; i < 4; i++)
    {
        keys[i].state = 0;
        keys[i].lock = 0;
        keys[i].in_combo = 0;
        keys[i].combo_triggered = 0;
        keys[i].cnt = 0;
    }
}