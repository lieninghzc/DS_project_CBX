/**
 * @file    Encoder.h
 * @brief   双相正交编码器测速（A/B 相捕获 + 方向检测）
 *
 * 硬件：
 *   TIMG0 CCP0 (PB10): 电机 A 编码器 A 相
 *   TIMG0 CCP1 (PB11): 电机 A 编码器 B 相
 *   TIMG12 CCP0 (PA14): 电机 B 编码器 A 相
 *   TIMG12 CCP1 (PA31): 电机 B 编码器 B 相
 *
 * 原理：
 *   A 相上升沿 → 读 B 相电平：B=HIGH 正向(+1), B=LOW 反向(-1)
 *   B 相上升沿 → 读 A 相电平：A=LOW 正向(+1), A=HIGH 反向(-1)
 *   每 N 个脉冲（绝对值）计算一次 RPM，正负号表示方向
 *
 * 公式：RPM = (MEAS_N / PPR) / dt_sec × 60 × sign
 */

#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>
#include <stdbool.h>

/* ── 参数配置 ── */
#define ENCODER_PPR 265U      /* 编码器每转脉冲数（物理线数） */
#define ENCODER_MEAS_N 10U    /* 每 N 个脉冲（2x 解码后为 20 边沿/转？no，2x=530 边沿/转）测一次速 */
#define ENCODER_TIMEOUT 2000U /* 超时 ms：超时未收到脉冲 → 转速归零 */

/* ── 轮子参数 ── */
#define WHEEL_DIAMETER_M 0.048f                        /* 轮子直径 (m) */
#define WHEEL_CIRCUM_M (3.1415926f * WHEEL_DIAMETER_M) /* 周长 ≈ 0.1508m */

/* 编码器选择 */
typedef enum
{
    ENC_A = 0,
    ENC_B = 1
} Enc_ID;

/** 初始化并启动 */
void Encoder_Init (void);

/** 设置方向反转（true=反转方向，用于编码器物理接线反了） */
void Encoder_SetInvert (Enc_ID id, bool invert);

/** 诊断：读取某编码器 B 相 GPIO 电平（0/1） */
uint32_t Encoder_DiagBPhase (Enc_ID id);

/** 诊断：电机A/B 各自 CCP1 ISR 触发次数 */
extern volatile uint32_t g_diag_cc1A;
extern volatile uint32_t g_diag_cc1B;

/** 获取当前转速（RPM），正=前进，负=后退 */
float Encoder_GetRPM (Enc_ID id);

/** 获取当前转速（Hz，转/秒），正=前进，负=后退 */
float Encoder_GetSpeed (Enc_ID id);

/** 获取原始带符号脉冲计数 */
int32_t Encoder_GetPulse (Enc_ID id);

/** 线速度 (m/s)，正=前进，负=后退 */
float Encoder_GetMPS (Enc_ID id);

/** 是否正在转动 */
bool Encoder_IsRunning (Enc_ID id);

#endif /* ENCODER_H */
