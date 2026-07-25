/**
 * @file    MaiXPro.c
 * @brief   MaixCAM/Pro UART 通信协议实现
 *
 * UART1: TX=PA8, RX=PA9, 115200-8-N-1
 *
 * 协议格式（每帧以 \n 结尾）：
 *   收到 "PING"   → 回复 "PONG"
 *   收到 "READY"  → 回复 "ACK"
 *   收到 其他     → 原样返回 (Echo)
 */

#include "main.h"
#include "MaiXPro.h"
#include <string.h>

/* 接收缓冲 */
#define RX_BUF_SIZE 128
static char g_rx_buf[RX_BUF_SIZE];
static uint8_t g_rx_idx = 0;

/* ================================================================ */
void Maix_Init (void)
{ /* UART 已由 SYSCFG_DL_init 初始化 */
}

/* ================================================================ */
void Maix_Send (const char* str)
{
    while (*str)
    {
        DL_UART_Main_transmitDataBlocking(MAIX_UART, (uint8_t)*str++);
    }
}

void Maix_SendLine (const char* str)
{
    Maix_Send(str);
    DL_UART_Main_transmitDataBlocking(MAIX_UART, '\n');
}

void Maix_SendData (const uint8_t* data, uint16_t len)
{
    while (len--)
    {
        DL_UART_Main_transmitDataBlocking(MAIX_UART, *data++);
    }
}

/* ================================================================ */
bool Maix_Available (void)
{
    return !DL_UART_Main_isRXFIFOEmpty(MAIX_UART);
}

uint8_t Maix_ReadByte (void)
{
    while (DL_UART_Main_isRXFIFOEmpty(MAIX_UART))
        ;
    return DL_UART_Main_receiveDataBlocking(MAIX_UART);
}

uint16_t Maix_ReadLine (char* buf, uint16_t maxLen)
{
    if (maxLen < 2)
        return 0;
    uint16_t idx = 0;
    maxLen--;

    while (idx < maxLen)
    {
        uint8_t ch = Maix_ReadByte();
        if (ch == '\n')
        {
            buf[idx] = '\0';
            return idx;
        }
        if (ch != '\r')
            buf[idx++] = ch;
    }
    buf[idx] = '\0';
    return idx;
}

/* ================================================================
 * 协议处理
 * ================================================================ */
void Maix_Process (void)
{
    if (!Maix_Available())
        return;

    uint8_t ch = DL_UART_Main_receiveDataBlocking(MAIX_UART);

    /* 收到换行 → 处理完整命令 */
    if (ch == '\n')
    {
        g_rx_buf[g_rx_idx] = '\0';
        g_rx_idx = 0;

        if (strcmp(g_rx_buf, "PING") == 0)
        {
            Maix_SendLine("PONG");
        }
        else if (strcmp(g_rx_buf, "READY") == 0)
        {
            Maix_SendLine("ACK");
        }
        else
        {
            /* Echo: 原样返回 */
            Maix_SendLine(g_rx_buf);
        }
    }
    /* 跳过 \r */
    else if (ch != '\r')
    {
        if (g_rx_idx < RX_BUF_SIZE - 1)
        {
            g_rx_buf[g_rx_idx++] = ch;
        }
    }
}

/* ================================================================
 * 非阻塞轮询：有数据就处理
 * ================================================================ */
bool Maix_Poll (void)
{
    bool handled = false;
    while (Maix_Available())
    {
        Maix_Process();
        handled = true;
    }
    return handled;
}
