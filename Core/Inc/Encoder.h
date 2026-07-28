/**
 * @file    Encoder.h
 * @brief   双相正交编码器 — ISR 计数 + 主循环定时测速（参考 STM32 逻辑）
 */

#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>
#include <stdbool.h>

#define ENCODER_PPR      265U
#define ENCODER_TIMEOUT  2000U

#define WHEEL_DIAMETER_M 0.048f
#define WHEEL_CIRCUM_M   (3.1415926f * WHEEL_DIAMETER_M)

typedef enum { ENC_A = 0, ENC_B = 1 } Enc_ID;

void     Encoder_Init(void);
void     Encoder_Update(Enc_ID id);        /* 主循环调用，计算速度 */
void     Encoder_SetInvert(Enc_ID id, bool invert);
float    Encoder_GetRPM(Enc_ID id);
float    Encoder_GetSpeed(Enc_ID id);
float    Encoder_GetMPS(Enc_ID id);
int32_t  Encoder_GetPulse(Enc_ID id);
bool     Encoder_IsRunning(Enc_ID id);

extern volatile uint32_t g_diag_cc1A;  /* 电机A CCP1 中断次数 */
extern volatile uint32_t g_diag_cc1B;  /* 电机B CCP1 中断次数 */

#endif
