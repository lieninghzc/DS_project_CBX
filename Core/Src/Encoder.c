/**
 * @file    Encoder.c
 * @brief   编码器脉冲累加测速（N 脉冲窗口法）
 *
 * 流程：
 *   ISR 每收到一个脉冲 → pulse++，累计计数 + 时间戳
 *   累计到 MEAS_N 个脉冲 → 计算 dt = 当前时间 - 上次完成时间
 *                       → RPM = (MEAS_N / PPR) / dt × 60
 *                       → 计数器清零，重新开始
 *   超时未收到脉冲 → 转速归零
 */

#include "main.h"
#include "Encoder.h"

/* ── 每通道状态 ── */
typedef struct {
    volatile uint32_t pulse;      /* ISR 递增的脉冲计数 */
    uint32_t last_pulse;          /* 上次完成时的脉冲总数 */
    uint32_t t_last;              /* 上次完成的时间戳 (g_msTick) */
    uint32_t t_recent;            /* 最近一次脉冲的时间戳 */
    float    rpm;                 /* 当前转速 */
    bool     running;             /* 是否在转 */
} Enc_Ch;

static Enc_Ch g_enc[2];

/* ── 内部计算 ── */
static void Enc_Compute(Enc_ID id)
{
    Enc_Ch *ch = &g_enc[id];
    uint32_t now = ch->t_recent;

    /* 需累计满 MEAS_N 个脉冲后才计算 */
    if (ch->pulse - ch->last_pulse < ENCODER_MEAS_N) return;

    /* dt = 当前时间 - 上次完成时间 (秒) */
    float dt = (now - ch->t_last) / 1000.0f;

    /* 防除零（首次测量或时间溢出） */
    if (dt <= 0.001f) return;

    /* RPM = (脉冲数 / PPR) / dt_sec × 60
     *     = (MEAS_N / PPR) × 60 / dt_sec */
    ch->rpm = ((float)ENCODER_MEAS_N / ENCODER_PPR) * 60.0f / dt;

    /* 记录本轮完成位置 */
    ch->last_pulse = ch->pulse;
    ch->t_last     = now;
}

/* ── 超时检查（主循环或定时器周期性调用） ── */
static void Enc_Timeout(Enc_ID id)
{
    Enc_Ch *ch = &g_enc[id];
    uint32_t now = g_msTick;
    uint32_t elapsed = now - ch->t_recent;

    /* 超 ENCODER_TIMEOUT ms 无脉冲 → 视为停止 */
    if (elapsed > ENCODER_TIMEOUT && ch->running) {
        ch->running = false;
        ch->rpm     = 0.0f;
        /* 同步脉冲计数，避免下次突然跳变 */
        ch->last_pulse = ch->pulse;
        ch->t_last     = now;
    }
}

/* ================================================================
 * ISR — 编码器 A 捕获（TIMG0）
 * ================================================================ */
void CAP_A_INST_IRQHandler(void)
{
    /* CC0_DN 捕获 → 脉冲+1 */
    uint32_t iidx = DL_TimerG_getPendingInterrupt(CAP_A_INST);
    switch (iidx) {
        case DL_TIMER_IIDX_CC0_DN:
        case DL_TIMER_IIDX_CC0_UP:
            DL_Timer_clearInterruptStatus(CAP_A_INST, DL_TIMER_INTERRUPT_CC0_DN_EVENT);
            g_enc[ENC_A].pulse++;
            g_enc[ENC_A].t_recent = g_msTick;
            g_enc[ENC_A].running  = true;
            Enc_Compute(ENC_A);
            break;
        default:
            /* 清所有未预期的中断标志 */
            DL_Timer_clearInterruptStatus(CAP_A_INST, 0xFFFFFFFF);
            break;
    }
}

void CAP_B_INST_IRQHandler(void)
{
    uint32_t iidx = DL_TimerG_getPendingInterrupt(CAP_B_INST);
    switch (iidx) {
        case DL_TIMER_IIDX_CC0_DN:
        case DL_TIMER_IIDX_CC0_UP:
            DL_Timer_clearInterruptStatus(CAP_B_INST, DL_TIMER_INTERRUPT_CC0_DN_EVENT);
            g_enc[ENC_B].pulse++;
            g_enc[ENC_B].t_recent = g_msTick;
            g_enc[ENC_B].running  = true;
            Enc_Compute(ENC_B);
            break;
        default:
            DL_Timer_clearInterruptStatus(CAP_B_INST, 0xFFFFFFFF);
            break;
    }
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
    return g_enc[id].rpm;
}

float Encoder_GetSpeed(Enc_ID id)
{
    return Encoder_GetRPM(id) / 60.0f;  /* Hz = RPM / 60 */
}

/*
 * 线速度 (m/s) = 转速 (rev/s) × 轮子周长 (m)
 *              = RPM / 60 × π × D
 */
float Encoder_GetMPS(Enc_ID id)
{
    return Encoder_GetRPM(id) / 60.0f * WHEEL_CIRCUM_M;
}

uint32_t Encoder_GetPulse(Enc_ID id)
{
    return g_enc[id].pulse;
}

bool Encoder_IsRunning(Enc_ID id)
{
    Enc_Timeout(id);
    return g_enc[id].running;
}
