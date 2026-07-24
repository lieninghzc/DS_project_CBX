/**
 * @file    Encoder.h
 * @brief   编码器 A 相脉冲测速
 *
 * 原理：累计 N 个脉冲，记录耗时 → 速度 = N / dt × 系数
 *   CAP_A (TIMG0):  Motor A 编码器 A 相
 *   CAP_B (TIMG12): Motor B 编码器 A 相
 *
 * 公式：RPM = (MEAS_N / PPR) / dt_sec × 60
 *       其中 MEAS_N=每次测量脉冲数, PPR=编码器线数, dt=测量窗口耗时(秒)
 */

#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>
#include <stdbool.h>

/* ── 参数配置 ── */
#define ENCODER_PPR 265U      /* 编码器每转脉冲数 */
#define ENCODER_MEAS_N 5U     /* 每 N 个脉冲测一次速 */
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

/** 获取当前转速（RPM） */
float Encoder_GetRPM (Enc_ID id);

/** 获取当前转速（Hz，转/秒） */
float Encoder_GetSpeed (Enc_ID id);

/** 获取原始脉冲计数 */
uint32_t Encoder_GetPulse (Enc_ID id);

/** 线速度 (m/s) */
float Encoder_GetMPS (Enc_ID id);

/** 是否正在转动 */
bool Encoder_IsRunning (Enc_ID id);

#endif /* ENCODER_H */
