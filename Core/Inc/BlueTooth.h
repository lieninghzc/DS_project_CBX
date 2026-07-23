/**
 * @file    BlueTooth.h
 * @brief   HC-05 蓝牙模块 UART 通信驱动
 *
 * 硬件连接（SysConfig）：
 *   UART0: TX=PA0, RX=PA1, 9600 8N1
 *   HC-05: RXD ← PA0(TX), TXD → PA1(RX), VCC=5V, GND=GND
 *   EN/KEY 引脚上电前拉高进入 AT 模式，悬空/拉低为透传模式
 */

#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <stdint.h>
#include <stdbool.h>

/* ================================================================
 * API
 * ================================================================ */

/** 初始化蓝牙（UART 已由 SYSCFG_DL_init 初始化）*/
void BT_Init(void);

/** 发送单字节 */
void BT_SendByte(uint8_t byte);

/** 发送字节数组 */
void BT_SendData(const uint8_t *data, uint16_t len);

/** 发送字符串（不含结尾 \\0） */
void BT_SendStr(const char *str);

/** 发送格式化字符串（类似 printf，通过 RTT 或 UART 输出） */
void BT_Printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

/** 检查是否有接收数据 */
bool BT_Available(void);

/** 读取一个字节（阻塞，直到有数据） */
uint8_t BT_ReadByte(void);

/** 读取一行（以 \\n 结束），maxLen 含结尾 \\0。返回实际读取的字节数 */
uint16_t BT_ReadLine(char *buf, uint16_t maxLen);

/** 发送 AT 指令并等待响应（调试用）*/
void BT_SendAT(const char *cmd);

#endif /* BLUETOOTH_H */
