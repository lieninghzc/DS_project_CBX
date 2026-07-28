/**
 * @file    Encoder.c
 * @brief   双相正交编码器 — A/B 相捕获 + 方向检测（N 脉冲窗口法）
 *
 * 硬件：
 *   TIMG0 CCP0 (PB10): 电机 A 编码器 A 相 → CC0_DN 中断
 *   TIMG0 CCP1 (PB11): 电机 A 编码器 B 相 → CC1_DN 中断
 *   TIMG12 CCP0 (PA14): 电机 B 编码器 A 相 → CC0_DN 中断
 *   TIMG12 CCP1 (PA31): 电机 B 编码器 B 相 → CC1_DN 中断
 *
 * 方向判断（2x 解码，仅上升沿）：
 *   A 相上升沿 → 读 B 相 GPIO：B=HIGH 正转(+1), B=LOW 反转(-1)
 *   B 相上升沿 → 读 A 相 GPIO：A=LOW 正转(+1), A=HIGH 反转(-1)
 *
 * 速度计算：
 *   累计 |Δpulse| >= MEAS_N → RPM = signed_Δpulse / PPR / dt × 60
 *   超时无脉冲 → 转速归零
 */

#include "main.h"
#include "Encoder.h"

/* ── 每通道状态 ── */
typedef struct {
    volatile int32_t pulse;      /* ISR 递增的有符号脉冲计数 */
    int32_t last_pulse;          /* 上次计算时的脉冲快照 */
    uint32_t t_last;             /* 上次计算完成的时间戳 (g_msTick) */
    uint32_t t_recent;           /* 最近一次脉冲的时间戳 */
    float    rpm;                /* 当前转速（正=前进，负=后退） */
    bool     running;            /* 是否在转 */
    bool     invert;             /* 方向反转标志 */
} Enc_Ch;

static Enc_Ch g_enc[2];

/* ── ISR 诊断计数器 ── */
volatile uint32_t g_diag_cc1A = 0;  /* 电机A CCP1 中断次数 */
volatile uint32_t g_diag_cc1B = 0;  /* 电机B CCP1 中断次数 */

/* ── 方向反转 ── */
void Encoder_SetInvert(Enc_ID id, bool invert)
{
    g_enc[id].invert = invert;
}

/* ── 内部计算 ── */
static void Enc_Compute(Enc_ID id)
{
    Enc_Ch *ch = &g_enc[id];
    uint32_t now = ch->t_recent;

    int32_t dpulse = ch->pulse - ch->last_pulse;

    /* 需累计满 MEAS_N 个脉冲（绝对值）后才计算 */
    if (dpulse < 0) dpulse = -dpulse;
    if ((uint32_t)dpulse < ENCODER_MEAS_N) return;

    /* dt = 当前时间 - 上次完成时间 (秒) */
    float dt = (now - ch->t_last) / 1000.0f;

    /* 防除零 */
    if (dt <= 0.001f) return;

    /* 带符号的净脉冲变化 */
    int32_t net = ch->pulse - ch->last_pulse;

    /* RPM = (净脉冲数 / PPR) / dt_sec × 60 */
    ch->rpm = ((float)net / ENCODER_PPR) * 60.0f / dt;

    /* 记录本轮完成位置 */
    ch->last_pulse = ch->pulse;
    ch->t_last     = now;
}

/* ── 超时检查 ── */
static void Enc_Timeout(Enc_ID id)
{
    Enc_Ch *ch = &g_enc[id];
    uint32_t now = g_msTick;
    uint32_t elapsed = now - ch->t_recent;

    /* 100ms 无脉冲 → 瞬归零（保底，不改变 running 状态） */
    if (elapsed > 100) {
        ch->rpm = 0.0f;
    }

    /* 超 ENCODER_TIMEOUT ms 无脉冲 → 彻底停止 */
    if (elapsed > ENCODER_TIMEOUT && ch->running) {
        ch->running = false;
        /* 同步脉冲计数，避免下次突然跳变 */
        ch->last_pulse = ch->pulse;
        ch->t_last     = now;
    }
}

/* ── 正交解码查表 (4x, 参考 STM32 实现) ── */
static const int8_t qdec_table[16] = {
    0, -1,  1,  0,
    1,  0,  0, -1,
   -1,  0,  0,  1,
    0,  1, -1,  0
};

/* 电机 A 的 A/B 相软件电平 (bit1=A, bit0=B) */
static volatile uint8_t g_stateA = 0;
static volatile uint8_t g_stateB = 0;

void CAP_A_INST_IRQHandler(void)
{
    uint32_t iidx;
    while ((iidx = DL_TimerG_getPendingInterrupt(CAP_A_INST)) != DL_TIMER_IIDX_ZERO) {
        uint8_t old = g_stateA;
        uint8_t cur;

        if (iidx == DL_TIMER_IIDX_CC0_DN) {
            DL_Timer_clearInterruptStatus(CAP_A_INST, DL_TIMER_INTERRUPT_CC0_DN_EVENT);
            g_stateA ^= 2;                    /* A 相翻转 bit1 */
        } else if (iidx == DL_TIMER_IIDX_CC1_DN) {
            DL_Timer_clearInterruptStatus(CAP_A_INST, DL_TIMER_INTERRUPT_CC1_DN_EVENT);
            g_diag_cc1A++;
            g_stateA ^= 1;                    /* B 相翻转 bit0 */
        } else {
            DL_Timer_clearInterruptStatus(CAP_A_INST, 0xFFFFFFFF);
            return;
        }

        cur = g_stateA;
        g_enc[ENC_A].pulse += qdec_table[((old & 3) << 2) | (cur & 3)];
        g_enc[ENC_A].t_recent = g_msTick;
        g_enc[ENC_A].running  = true;
    }
    Enc_Compute(ENC_A);
}

void CAP_B_INST_IRQHandler(void)
{
    uint32_t iidx;
    while ((iidx = DL_TimerG_getPendingInterrupt(CAP_B_INST)) != DL_TIMER_IIDX_ZERO) {
        uint8_t old = g_stateB;
        uint8_t cur;

        if (iidx == DL_TIMER_IIDX_CC0_DN) {
            DL_Timer_clearInterruptStatus(CAP_B_INST, DL_TIMER_INTERRUPT_CC0_DN_EVENT);
            g_stateB ^= 2;                    /* A 相翻转 bit1 */
        } else if (iidx == DL_TIMER_IIDX_CC1_DN) {
            DL_Timer_clearInterruptStatus(CAP_B_INST, DL_TIMER_INTERRUPT_CC1_DN_EVENT);
            g_diag_cc1B++;
            g_stateB ^= 1;                    /* B 相翻转 bit0 */
        } else {
            DL_Timer_clearInterruptStatus(CAP_B_INST, 0xFFFFFFFF);
            return;
        }

        cur = g_stateB;
        g_enc[ENC_B].pulse += qdec_table[((old & 3) << 2) | (cur & 3)];
        g_enc[ENC_B].t_recent = g_msTick;
        g_enc[ENC_B].running  = true;
    }
    Enc_Compute(ENC_B);
}

/* ================================================================
 * 初始化
 * ================================================================ */
void Encoder_Init(void)
{
    Enc_Ch *a = &g_enc[ENC_A];
    Enc_Ch *b = &g_enc[ENC_B];

    a->t_last = a->t_recent = g_msTick;
    b->t_last = b->t_recent = g_msTick;

    DL_TimerG_startCounter(CAP_A_INST);
    DL_TimerG_startCounter(CAP_B_INST);
    NVIC_EnableIRQ(CAP_A_INST_INT_IRQN);
    NVIC_EnableIRQ(CAP_B_INST_INT_IRQN);
}

/* ================================================================
 * 获取转速（超时自动归零）
 * ================================================================ */
float Encoder_GetRPM(Enc_ID id)
{
    Enc_Timeout(id);
    float rpm = g_enc[id].rpm;
    return g_enc[id].invert ? -rpm : rpm;
}

float Encoder_GetSpeed(Enc_ID id)
{
    return Encoder_GetRPM(id) / 60.0f;  /* Hz = RPM / 60 */
}

/*
 * 线速度 (m/s) = 转速 (rev/s) × 轮子周长 (m)
 *              = RPM / 60 × π × D
 * 正=前进，负=后退
 */
float Encoder_GetMPS(Enc_ID id)
{
    return Encoder_GetRPM(id) / 60.0f * WHEEL_CIRCUM_M;
}

int32_t Encoder_GetPulse(Enc_ID id)
{
    int32_t p = g_enc[id].pulse;
    return g_enc[id].invert ? -p : p;
}

bool Encoder_IsRunning(Enc_ID id)
{
    Enc_Timeout(id);
    return g_enc[id].running;
}

uint32_t Encoder_DiagBPhase(Enc_ID id)
{
    if (id == ENC_A)
        return DL_GPIO_readPins(GPIO_CAP_A_A_C1_PORT, GPIO_CAP_A_A_C1_PIN);
    else
        return DL_GPIO_readPins(GPIO_CAP_B_B_C1_PORT, GPIO_CAP_B_B_C1_PIN);
}
