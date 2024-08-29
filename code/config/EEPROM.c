#include <stc15.h>
#include <intrins.h>
#include "EEPROM.H"
#include "Delay.H"

#define CMD_READ 1    // IAP字节读命令
#define CMD_PROGRAM 2 // IAP字节编程命令
#define CMD_ERASE 3   // IAP扇区擦除命令

#define ENABLE_IAP 0x80 // if SYSCLK<30MHz
// #define ENABLE_IAP 0x81 //if SYSCLK<24MHz
// #define ENABLE_IAP 0x82 //if SYSCLK<20MHz
// #define ENABLE_IAP 0x83 //if SYSCLK<12MHz
// #define ENABLE_IAP 0x84 //if SYSCLK<6MHz
// #define ENABLE_IAP 0x85 //if SYSCLK<3MHz
// #define ENABLE_IAP 0x86 //if SYSCLK<2MHz
// #define ENABLE_IAP 0x87 //if SYSCLK<1MH

/*----------------------------
关闭IAP
----------------------------*/
void IapIdle()
{
    IAP_CONTR = 0;    // 关闭IAP功能
    IAP_CMD = 0;      // 清除命令寄存器
    IAP_TRIG = 0;     // 清除触发寄存器
    IAP_ADDRH = 0x80; // 将地址设置到非IAP区域
    IAP_ADDRL = 0;
}

/*----------------------------
扇区擦除
----------------------------*/
void IapEraseSector(uint16t addr)
{
    IAP_CONTR = ENABLE_IAP; // 使能IAP
    IAP_CMD = CMD_ERASE;    // 设置IAP命令
    IAP_ADDRL = addr;       // 设置IAP低地址
    IAP_ADDRH = addr >> 8;  // 设置IAP高地址
    IAP_TRIG = 0x5a;        // 写触发命令(0x5a)
    IAP_TRIG = 0xa5;        // 写触发命令(0xa5)
    _nop_();                // 等待ISP/IAP/EEPROM操作完成
    IapIdle();
}

/**
 * 读一个字节
 * @param  地址
 */
uint8t IapReadByte(uint16t addr)
{
    uint8t dat;             // 数据缓冲区
    IAP_CONTR = ENABLE_IAP; // 使能IAP
    IAP_CMD = CMD_READ;     // 设置IAP命令
    IAP_ADDRL = addr;       // 设置IAP低地址
    IAP_ADDRH = addr >> 8;  // 设置IAP高地址
    IAP_TRIG = 0x5a;        // 写触发命令(0x5a)
    IAP_TRIG = 0xa5;        // 写触发命令(0xa5)
    _nop_();                // 等待ISP/IAP/EEPROM操作完成
    dat = IAP_DATA;         // 读ISP/IAP/EEPROM数据
    IapIdle();              // 关闭IAP功能
    return dat;             // 返回
}

void IapReadArrayByte(uint16t addr, uint8t *dat)
{
    uint8t i;
    uint16t temp_addr;
    // EA = 0;

    IAP_CONTR = ENABLE_IAP; // 使能IAP
    IAP_CMD = CMD_READ;     // 设置IAP命令 读
    for (i = 0; i < sizeof(dat); i++)
    {
        temp_addr = addr + i;       // 计算偏移量
        IAP_ADDRL = temp_addr;      // 设置IAP低地址
        IAP_ADDRH = temp_addr >> 8; // 设置IAP高地址
        IAP_TRIG = 0x5a;            // 写触发命令(0x5a)
        IAP_TRIG = 0xa5;            // 写触发命令(0xa5)
        _nop_();                    // 等待ISP/IAP/EEPROM操作完成
        _nop_();
        dat[i] = IAP_DATA; // 读ISP/IAP/EEPROM数据
    }

    IapIdle(); // 关闭IAP功能

    // EA = 1;
}

/*----------------------------
写一字节数据到ISP/IAP/EEPROM区域
----------------------------*/
void IapProgramByte(uint16t addr, uint8t dat)
{
    IAP_CONTR = ENABLE_IAP; // 使能IAP
    IAP_CMD = CMD_PROGRAM;  // 设置IAP命令
    IAP_ADDRL = addr;       // 设置IAP低地址
    IAP_ADDRH = addr >> 8;  // 设置IAP高地址
    IAP_DATA = dat;         // 写ISP/IAP/EEPROM数据
    IAP_TRIG = 0x5a;        // 写触发命令(0x5a)
    IAP_TRIG = 0xa5;        // 写触发命令(0xa5)
    _nop_();                // 等待ISP/IAP/EEPROM操作完成
    IapIdle();
}


/*----------------------------
写一字节数据到ISP/IAP/EEPROM区域
----------------------------*/
//不知道为什么一次性写多个字节回出问题(写完后高8位全都一样)。只能多次调用写一个字节，且不能在这个文件里

// void IapProgramArrayByte(uint16t addr, uint8t *dat)
// {
//     uint8t i;
//     uint16t temp_addr;

//     EA = 0;
//     IAP_CONTR = ENABLE_IAP; // 使能IAP
//     IAP_CMD = CMD_PROGRAM;  // 设置IAP命令

//     for (i = 0; i < sizeof(dat); i++)
//     {
//         temp_addr = addr + i;
//         IAP_ADDRL = temp_addr;      // 设置IAP低地址
//         IAP_ADDRH = temp_addr >> 8; // 设置IAP高地址
//         IAP_DATA = dat[i];          // 写ISP/IAP/EEPROM数据
//         IAP_TRIG = 0x5a;            // 写触发命令(0x5a)
//         IAP_TRIG = 0xa5;            // 写触发命令(0xa5)
//         _nop_();                    // 等待ISP/IAP/EEPROM操作完成
//         _nop_();                    // 等待ISP/IAP/EEPROM操作完成
//     }

//     IapIdle();
//     EA = 1;
// }

