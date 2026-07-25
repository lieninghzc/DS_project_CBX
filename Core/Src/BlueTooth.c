/**
 * @file    BlueTooth.c
 * @brief   HC-05 蓝牙模块 UART 通信实现
 *
 * HC-05 工作模式：
 *   透传模式：EN/KEY 悬空或低电平，所有 UART 数据直接通过蓝牙收发
 *   AT 模式：  上电前 EN/KEY 拉高，UART 发送 AT 指令配置模块
 *
 * 常用 AT 指令：
 *   AT            → 测试连接（返回 OK）
 *   AT+NAME=xxx   → 设置蓝牙名称
 *   AT+UART=9600,0,0 → 设置波特率,停止位,校验位
 *   AT+ROLE=0     → 从机模式（0=从机, 1=主机）
 *
 * 数据流：
 *   发送：MCU → UART0(TX PA0) → HC-05(RXD) → 蓝牙无线 → 手机/电脑
 *   接收：手机/电脑 → 蓝牙无线 → HC-05(TXD) → UART0(RX PA1) → MCU
 */

#include "main.h"
#include "BlueTooth.h"
#include <stdarg.h>
#include <string.h>

/* ── printf 缓冲（BT_Printf 格式化用） ── */
#define BT_PRINTF_BUF  128

/* ================================================================
 * 初始化（UART 硬件已由 SYSCFG_DL_init → SYSCFG_DL_UART_0_init 完成）
 * ================================================================ */
void BT_Init(void)
{
    /* UART0 已在 SYSCFG_DL_init() 中初始化，无需额外操作 */
}

/* ================================================================
 * 发送单字节（阻塞，等 TX FIFO 有空位）
 * ================================================================ */
void BT_SendByte(uint8_t byte)
{
    DL_UART_Main_transmitDataBlocking(UART_0_INST, byte);
}

/* ================================================================
 * 发送字节数组
 * ================================================================ */
void BT_SendData(const uint8_t *data, uint16_t len)
{
    while (len--) {
        BT_SendByte(*data++);
    }
}

/* ================================================================
 * 发送字符串
 * ================================================================ */
void BT_SendStr(const char *str)
{
    while (*str) {
        BT_SendByte((uint8_t)*str++);
    }
}

/* ================================================================
 * 格式化发送（类似 printf，但输出到蓝牙而非 RTT）
 * ================================================================ */
void BT_Printf(const char *fmt, ...)
{
    char buf[BT_PRINTF_BUF];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    BT_SendStr(buf);
}

/* ================================================================
 * 检查 RX 缓冲区是否有未读数据
 * ================================================================ */
bool BT_Available(void)
{
    return !DL_UART_Main_isRXFIFOEmpty(UART_0_INST);
}

/* ================================================================
 * 读取单字节（阻塞，直到收到数据）
 * ================================================================ */
uint8_t BT_ReadByte(void)
{
    /* 阻塞等待 RX FIFO 收到数据 */
    while (DL_UART_Main_isRXFIFOEmpty(UART_0_INST)) {
        /* 空转等待，可改为低功耗或 RTOS 挂起 */
    }
    return DL_UART_Main_receiveDataBlocking(UART_0_INST);
}

/* ================================================================
 * 读取一行（以 \n 或 \r\n 结束）
 * 返回值：实际读取字节数（含 \\0），超时未收到任何数据返回 0
 * ================================================================ */
uint16_t BT_ReadLine(char *buf, uint16_t maxLen)
{
    if (maxLen < 2) return 0;
    uint16_t idx = 0;
    maxLen--;  /* 留一位给 \\0 */

    while (idx < maxLen) {
        /* 简单超时机制：等待约 1 秒 */
        uint32_t timeout = 1000000;
        while (DL_UART_Main_isRXFIFOEmpty(UART_0_INST)) {
            if (--timeout == 0) {
                buf[idx] = '\0';
                return idx;  /* 超时，返回已收到的部分 */
            }
        }

        uint8_t ch = DL_UART_Main_receiveDataBlocking(UART_0_INST);

        /* 遇到 \n 结束（兼容 \r\n） */
        if (ch == '\n') {
            /* 如果前一个字节是 \r，覆盖它 */
            if (idx > 0 && buf[idx - 1] == '\r') idx--;
            buf[idx] = '\0';
            return idx;
        }

        buf[idx++] = ch;
    }

    buf[idx] = '\0';
    return idx;
}

/* ================================================================
 * 发送 AT 指令并打印响应（调试用）
 * 同时把响应输出到 RTT 方便 Ozone 查看
 * ================================================================ */
void BT_SendAT(const char *cmd)
{
    printf("[BT] TX: %s\n", cmd);
    BT_SendStr(cmd);
    BT_SendStr("\r\n");

    /* 等响应 */
    delay_ms(200);

    printf("[BT] RX: ");
    while (BT_Available()) {
        uint8_t ch = BT_ReadByte();
        printf("%c", ch);
    }
    printf("\n");
}
