/**
 * @file    MaiXPro.h
 * @brief   MaixCAM/Pro UART 通信协议
 *
 * 硬件: UART1, TX=PA8, RX=PA9, 115200-8-N-1
 * 协议:
 *   PING  → PONG   握手探测
 *   READY → ACK    就绪确认
 *   其他   → 原样返回 (Echo)
 */

#ifndef MAIXPRO_H
#define MAIXPRO_H

#include <stdint.h>
#include <stdbool.h>

/* UART 宏（SysConfig 生成） */
#define MAIX_UART       UART_1_CAM_INST
#define MAIX_BAUD       UART_1_CAM_BAUD_RATE

/** 初始化（UART 硬件已由 SysConfig 配置） */
void Maix_Init(void);

/** 发送字符串 */
void Maix_Send(const char *str);

/** 发送字符串 + 换行 */
void Maix_SendLine(const char *str);

/** 发送原始数据 */
void Maix_SendData(const uint8_t *data, uint16_t len);

/** 检查是否有接收数据 */
bool Maix_Available(void);

/** 读一个字节（阻塞） */
uint8_t Maix_ReadByte(void);

/** 读一行（以 \n 结束），返回实际长度 */
uint16_t Maix_ReadLine(char *buf, uint16_t maxLen);

/** 处理收到的命令：PING→PONG, READY→ACK, 其他→Echo */
void Maix_Process(void);

/** 非阻塞检查：有完整行则处理并返回 true */
bool Maix_Poll(void);

#endif
