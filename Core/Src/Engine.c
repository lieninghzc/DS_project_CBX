/**
 * @file    Engine.c
 * @brief   双电机运动控制实现（前馈 + PID）
 *
 * 控制公式：
 *   duty = K_FF × target + PID(target, actual)
 *
 *   K_FF = 1000（1 m/s → duty ≈ 1000，具体值需实测校准）
 *   PID 仅在编码器有信号时介入修正
 */

#include "main.h"
#include "Engine.h"

/* ── 左右轮 PID + 目标 ── */
static PID_Ctrl g_pidL, g_pidR;
static float    g_targetL = 0, g_targetR = 0;

/* ================================================================
 * 初始化
 * ================================================================ */
void Engine_Init(void)
{
    Motor_Init();
    Encoder_Init();

    PID_Init(&g_pidL, ENGINE_KP, ENGINE_KI, ENGINE_KD,
             ENGINE_DT, ENGINE_OUT_MIN, ENGINE_OUT_MAX);
    PID_Init(&g_pidR, ENGINE_KP, ENGINE_KI, ENGINE_KD,
             ENGINE_DT, ENGINE_OUT_MIN, ENGINE_OUT_MAX);
}

/* ================================================================
 * 设定目标速度并立即输出前馈占空比
 * ================================================================ */
void Engine_SetSpeed(float left_mps, float right_mps)
{
    g_targetL = left_mps;
    g_targetR = right_mps;

    PID_Setpoint(&g_pidL, left_mps);
    PID_Setpoint(&g_pidR, right_mps);
    PID_Reset(&g_pidL);
    PID_Reset(&g_pidR);

    /* 前馈：直接给占空比，不等 PID 慢慢积 */
    int16_t dutyL = (int16_t)(left_mps  * ENGINE_KFF);
    int16_t dutyR = (int16_t)(right_mps * ENGINE_KFF);

    /* 限幅 */
    if (dutyL >  999) dutyL =  999;
    if (dutyL < -999) dutyL = -999;
    if (dutyR >  999) dutyR =  999;
    if (dutyR < -999) dutyR = -999;

    Motor_Set(MOTOR_A, (dutyL >= 0) ? MOTOR_FWD : MOTOR_REV,
              (uint16_t)(dutyL >= 0 ? dutyL : -dutyL));
    Motor_Set(MOTOR_B, (dutyR >= 0) ? MOTOR_FWD : MOTOR_REV,
              (uint16_t)(dutyR >= 0 ? dutyR : -dutyR));
}

/* ================================================================
 * 直走
 * ================================================================ */
void Engine_GoStraight(float mps)
{
    Engine_SetSpeed(mps, mps);
}

/* ================================================================
 * 差速转向
 * ================================================================ */
void Engine_Turn(float left_mps, float right_mps)
{
    Engine_SetSpeed(left_mps, right_mps);
}

void Engine_Stop(void)  { Motor_Set(MOTOR_A, MOTOR_STOP, 0);
                          Motor_Set(MOTOR_B, MOTOR_STOP, 0); }
void Engine_Brake(void) { Motor_Set(MOTOR_A, MOTOR_BRAKE, 0);
                          Motor_Set(MOTOR_B, MOTOR_BRAKE, 0); }

float Engine_GetLeftSpeed(void)  { return Encoder_GetMPS(ENC_A); }
float Engine_GetRightSpeed(void) { return Encoder_GetMPS(ENC_B); }

/* ================================================================
 * PID 更新（每 20ms 调一次）
 *
 * 前馈 + PID 叠加：
 *   ff   = target × K_FF
 *   pid  = PID_Compute(target, actual)
 *   duty = clamp(ff + pid, 0, 999)
 * ================================================================ */
void Engine_Update(void)
{
    float spdL = Encoder_GetMPS(ENC_A);
    float spdR = Encoder_GetMPS(ENC_B);

    /* 左轮 */
    float outL = 0;
    float ffL  = g_targetL * ENGINE_KFF;
    if (Encoder_IsRunning(ENC_A)) {
        outL = PID_Compute(&g_pidL, spdL);
    } else {
        PID_Reset(&g_pidL);
    }
    float dutyL = ffL + outL;
    if (dutyL >  999) dutyL =  999;
    if (dutyL < -999) dutyL = -999;
    Motor_Set(MOTOR_A, (dutyL >= 0) ? MOTOR_FWD : MOTOR_REV,
              (uint16_t)(dutyL >= 0 ? dutyL : -dutyL));

    /* 右轮 */
    float outR = 0;
    float ffR  = g_targetR * ENGINE_KFF;
    if (Encoder_IsRunning(ENC_B)) {
        outR = PID_Compute(&g_pidR, spdR);
    } else {
        PID_Reset(&g_pidR);
    }
    float dutyR = ffR + outR;
    if (dutyR >  999) dutyR =  999;
    if (dutyR < -999) dutyR = -999;
    Motor_Set(MOTOR_B, (dutyR >= 0) ? MOTOR_FWD : MOTOR_REV,
              (uint16_t)(dutyR >= 0 ? dutyR : -dutyR));
}

/* ================================================================
 * 打印
 * ================================================================ */
void Engine_Print(void)
{
    SEGGER_RTT_printf(0,
        "ENG L:%.2f→%.2f  R:%.2f→%.2f m/s\n",
        Engine_GetLeftSpeed(), g_targetL,
        Engine_GetRightSpeed(), g_targetR);
}
