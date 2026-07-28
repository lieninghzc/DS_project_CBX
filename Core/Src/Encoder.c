/**
 * Encoder.c — A相CCP0捕获计数, B相GPIO读电平判方向
 *   A↑ + B=LOW  → 正转+1
 *   A↑ + B=HIGH → 反转-1
 */

#include "main.h"
#include "Encoder.h"

typedef struct {
    volatile int32_t pulse;
    int32_t          last_pulse;
    uint32_t         t_last, t_recent;
    float            rpm;
    bool             running, invert;
} Enc_Ch;

static Enc_Ch g_enc[2];

volatile uint32_t g_diag_cc1A = 0;
volatile uint32_t g_diag_cc1B = 0;

void Encoder_SetInvert(Enc_ID id, bool v) { g_enc[id].invert = v; }

/* ── 读B相GPIO ── */
static inline int read_B(Enc_ID id) {
    if (id == ENC_A)
        return DL_GPIO_readPins(GPIO_CAP_A_A_C1_PORT, GPIO_CAP_A_A_C1_PIN);
    else
        return DL_GPIO_readPins(GPIO_CAP_B_B_C1_PORT, GPIO_CAP_B_B_C1_PIN);
}

/* ── ISR: 仅CCP0(A相)上升沿 ── */
void CAP_A_INST_IRQHandler(void)
{
    uint32_t iidx;
    while ((iidx = DL_TimerG_getPendingInterrupt(CAP_A_INST)) != DL_TIMER_IIDX_ZERO) {
        if (iidx == DL_TIMER_IIDX_CC0_DN) {
            DL_Timer_clearInterruptStatus(CAP_A_INST, DL_TIMER_INTERRUPT_CC0_DN_EVENT);
            g_enc[ENC_A].pulse += read_B(ENC_A) ? -1 : 1;
            g_enc[ENC_A].t_recent = g_msTick;
            g_enc[ENC_A].running  = true;
        } else {
            DL_Timer_clearInterruptStatus(CAP_A_INST, 0xFFFFFFFF);
            return;
        }
    }
    Encoder_Update(ENC_A);
}

void CAP_B_INST_IRQHandler(void)
{
    uint32_t iidx;
    while ((iidx = DL_TimerG_getPendingInterrupt(CAP_B_INST)) != DL_TIMER_IIDX_ZERO) {
        if (iidx == DL_TIMER_IIDX_CC0_DN) {
            DL_Timer_clearInterruptStatus(CAP_B_INST, DL_TIMER_INTERRUPT_CC0_DN_EVENT);
            g_enc[ENC_B].pulse += read_B(ENC_B) ? 1 : -1;  /* B反转 */
            g_enc[ENC_B].t_recent = g_msTick;
            g_enc[ENC_B].running  = true;
        } else {
            DL_Timer_clearInterruptStatus(CAP_B_INST, 0xFFFFFFFF);
            return;
        }
    }
    Encoder_Update(ENC_B);
}

void Encoder_Init(void)
{
    g_enc[ENC_A].t_last = g_enc[ENC_A].t_recent = g_msTick;
    g_enc[ENC_B].t_last = g_enc[ENC_B].t_recent = g_msTick;
    DL_TimerG_startCounter(CAP_A_INST);
    DL_TimerG_startCounter(CAP_B_INST);
    NVIC_EnableIRQ(CAP_A_INST_INT_IRQN);
    NVIC_EnableIRQ(CAP_B_INST_INT_IRQN);
}

void Encoder_Update(Enc_ID id)
{
    Enc_Ch *ch = &g_enc[id];
    uint32_t now = g_msTick;
    if (now - ch->t_recent > 100) { ch->rpm = 0; ch->running = 0; ch->last_pulse = ch->pulse; ch->t_last = now; return; }
    float dt = (now - ch->t_last) / 1000.0f;
    if (dt < 0.01f) return;
    int32_t d = ch->pulse - ch->last_pulse;
    ch->rpm = ((float)d / ENCODER_PPR) * 60.0f / dt;
    ch->last_pulse = ch->pulse;
    ch->t_last = now;
    ch->running = (d != 0);
}

float    Encoder_GetRPM(Enc_ID id)   { Encoder_Update(id); float r = g_enc[id].rpm; return g_enc[id].invert ? -r : r; }
float    Encoder_GetMPS(Enc_ID id)   { return Encoder_GetRPM(id) / 60.0f * WHEEL_CIRCUM_M; }
int32_t  Encoder_GetPulse(Enc_ID id) { int32_t p = g_enc[id].pulse; return g_enc[id].invert ? -p : p; }
