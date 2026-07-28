/**
 * Engine.c — 小车坐标系: target+/duty+ = 小车前进
 *   Motor A: MOTOR_A_INVERT 使 FWD→物理REV→小车前进
 *   Encoder: 两电机 FWD→编码器读负, SetInvert 翻为正
 */

#include "main.h"
#include "Engine.h"

static PID_Ctrl g_pidL, g_pidR;
float g_targetL = 0, g_targetR = 0;  /* 外部可读 */

void Engine_Init(void)
{
    Motor_Init();
    Encoder_Init();
    PID_Init(&g_pidL, ENGINE_KP, ENGINE_KI, ENGINE_KD, ENGINE_DT, ENGINE_OUT_MIN, ENGINE_OUT_MAX, ENGINE_IMAX);
    PID_Init(&g_pidR, ENGINE_KP, ENGINE_KI, ENGINE_KD, ENGINE_DT, ENGINE_OUT_MIN, ENGINE_OUT_MAX, ENGINE_IMAX);
}

void Engine_SetSpeed(float left_mps, float right_mps)
{
    g_targetL = left_mps;
    g_targetR = right_mps;
    PID_Reset(&g_pidL);
    PID_Reset(&g_pidR);

    int16_t dutyL = (int16_t)(g_targetL * ENGINE_KFF);
    int16_t dutyR = (int16_t)(g_targetR * ENGINE_KFF);
    if (dutyL >  999) dutyL =  999;
    if (dutyL < -999) dutyL = -999;
    if (dutyR >  999) dutyR =  999;
    if (dutyR < -999) dutyR = -999;

    Motor_Set(MOTOR_A, (dutyL >= 0) ? MOTOR_FWD : MOTOR_REV, (uint16_t)(dutyL >= 0 ? dutyL : -dutyL));
    Motor_Set(MOTOR_B, (dutyR >= 0) ? MOTOR_FWD : MOTOR_REV, (uint16_t)(dutyR >= 0 ? dutyR : -dutyR));
}

void Engine_GoStraight(float mps) { Engine_SetSpeed(mps, mps); }
void Engine_Turn(float l, float r) { Engine_SetSpeed(l, r); }

/* 仅设目标, 不 Reset → 循迹连续调用不丢积分, Engine_Update自行取g_target传入PID */
void Engine_SetTarget(float left_mps, float right_mps)
{
    g_targetL = left_mps;
    g_targetR = right_mps;
}

void Engine_Stop(void)  { g_targetL=g_targetR=0; Motor_Set(MOTOR_A,MOTOR_STOP,0); Motor_Set(MOTOR_B,MOTOR_STOP,0); }
void Engine_Brake(void) { g_targetL=g_targetR=0; Motor_Set(MOTOR_A,MOTOR_BRAKE,999); Motor_Set(MOTOR_B,MOTOR_BRAKE,999); }

float Engine_GetLeftSpeed(void)  { return Encoder_GetMPS(ENC_A); }
float Engine_GetRightSpeed(void) { return Encoder_GetMPS(ENC_B); }

void Engine_Update(void)
{
    if (g_targetL == 0 && g_targetR == 0) return;

    float spdL = Encoder_GetMPS(ENC_A);
    float spdR = Encoder_GetMPS(ENC_B);

    float ffL   = g_targetL * ENGINE_KFF;
    float pidL  = PID_Compute(&g_pidL, g_targetL, spdL);
    float dutyL = ffL + pidL;
    if (dutyL >  999) dutyL =  999;
    if (dutyL < -999) dutyL = -999;
    Motor_Set(MOTOR_A, (dutyL >= 0) ? MOTOR_FWD : MOTOR_REV, (uint16_t)(dutyL >= 0 ? dutyL : -dutyL));

    float ffR   = g_targetR * ENGINE_KFF;
    float pidR  = PID_Compute(&g_pidR, g_targetR, spdR);
    float dutyR = ffR + pidR;
    if (dutyR >  999) dutyR =  999;
    if (dutyR < -999) dutyR = -999;
    Motor_Set(MOTOR_B, (dutyR >= 0) ? MOTOR_FWD : MOTOR_REV, (uint16_t)(dutyR >= 0 ? dutyR : -dutyR));
}

void Engine_Print(void)
{
    SEGGER_RTT_printf(0, "ENG L:%.2f->%.2f  R:%.2f->%.2f m/s\n",
        Engine_GetLeftSpeed(), g_targetL, Engine_GetRightSpeed(), g_targetR);
}

void Engine_Rotate(float angle_deg)
{
    float start_yaw  = Position_YawDeg();
    float target_yaw = start_yaw + angle_deg;
    float spd = 0.3f;
    float sign = (angle_deg > 0) ? 1.0f : -1.0f;
    g_targetL = -sign * spd;
    g_targetR =  sign * spd;
    PID_Reset(&g_pidL); PID_Reset(&g_pidR);
    printf("Rotate: %.1f @ %.2f m/s\n", angle_deg, spd);

    while (1) {
        delay_ms(20);
        Position_Update();
        float remain = target_yaw - Position_YawDeg();
        if (remain < 2.0f && remain > -2.0f) break;
        if ((angle_deg > 0 && remain < 0) || (angle_deg < 0 && remain > 0)) {
            g_targetL = -g_targetL; g_targetR = -g_targetR;
            PID_Reset(&g_pidL); PID_Reset(&g_pidR);
        }
        Engine_Update();
    }
    Engine_Brake();
    g_targetL = g_targetR = 0;
    printf("Rotate done: %.1f\n", Position_YawDeg());
}
