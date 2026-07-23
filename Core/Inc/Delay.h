#ifndef DELAY_H
#define DELAY_H

#include <stdint.h>

// ============================================================
// 精确延迟库 — 基于 SysTick（Cortex-M 内核自带硬件定时器）
//
// 用法:
//   delay_init();          // 初始化（main 开头调用一次）
//   delay_ms(500);         // 阻塞等待 500 毫秒
//   delay_us(10);          // 阻塞等待 10 微秒
//   uint32_t t = timer_ms; // 读取全局毫秒时间戳
// ============================================================

/** 全局毫秒计数器，SysTick 中断每 1ms 自增 */
extern volatile uint32_t g_msTick;

/** 初始化 SysTick，每 1ms 中断一次。main 开头调用一次即可。 */
void delay_init(void);

/** 毫秒级阻塞延迟 */
void delay_ms(uint32_t ms);

/** 微秒级阻塞延迟（最大约 200ms，由 CPUCLK_FREQ 决定） */
void delay_us(uint32_t us);

#endif
