#include "Delay.h"
#include "ti_msp_dl_config.h"   // 提供 CPUCLK_FREQ 宏

// ============================================================
// 全局毫秒时间戳 — 每 1ms 自增一次
// ============================================================
volatile uint32_t g_msTick = 0;

// ============================================================
// delay_init — 初始化 SysTick，main 开头调用一次
// ============================================================
void delay_init(void)
{
    // CPUCLK_FREQ 是 SysConfig 生成的头文件里的宏：
    //   32MHz 内部振荡器  →  32000000
    //   80MHz PLL         →  80000000
    //
    // SysTick 是 ARM 内核自带的 24 位硬件递减计数器。
    // LOAD = 每毫秒的时钟周期数 - 1
    // 比如 80MHz：80000000 / 1000 - 1 = 79999
    SysTick->LOAD = (uint32_t)(CPUCLK_FREQ / 1000) - 1;
    SysTick->VAL  = 0;

    // CTRL 寄存器位说明：
    //   CLKSOURCE(bit2) = 1  → 用 CPU 时钟（不用外部参考时钟）
    //   TICKINT(bit1)   = 1  → 计数器到 0 时触发 SysTick_Handler 中断
    //   ENABLE(bit0)    = 1  → 启动计数器
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk
                  | SysTick_CTRL_TICKINT_Msk
                  | SysTick_CTRL_ENABLE_Msk;
}

// ============================================================
// delay_ms — 毫秒级阻塞延迟
// ============================================================
void delay_ms(uint32_t ms)
{
    if (ms == 0) return;

    uint32_t target = g_msTick + ms;

    // 空转，等 SysTick 中断把 g_msTick 推到目标值
    while (g_msTick < target) {
        // CPU 在这里等待，但中断照常触发
    }
}

// ============================================================
// delay_us — 微秒级阻塞延迟
// ============================================================
void delay_us(uint32_t us)
{
    if (us == 0) return;

    // 将微秒换算为 SysTick 计数值
    // 例：80MHz 时 1us = 80 个周期，10us = 800 个周期
    uint32_t ticks = (CPUCLK_FREQ / 1000000UL) * us;

    uint32_t start = SysTick->VAL;

    while (1) {
        uint32_t now = SysTick->VAL;

        if (now < start) {
            // 情况1：正常递减 —— 从 start 数到 now
            // elapsed = 已经走过的周期数
            if ((start - now) >= ticks) return;
        } else if (now > start) {
            // 情况2：计数器跨过 0，重新装载了 LOAD 值
            // 走过的周期 = 从 start 到 0 + 从 LOAD 到 now
            if ((start + (SysTick->LOAD + 1 - now)) >= ticks) return;
        }
        // now == start：刚读完还没开始走，再读一次
    }
}

// ============================================================
// SysTick 中断服务函数
// 覆盖启动文件 startup_mspm0g350x_gcc.c 里的弱定义 __attribute__((weak))
// 每 1ms 硬件自动调用一次
// ============================================================
void SysTick_Handler(void)
{
    g_msTick++;
}
