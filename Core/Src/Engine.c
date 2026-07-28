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
static float g_targetL = 0, g_targetR = 0;

/* ================================================================
 * 初始化
 * ================================================================ */
void Engine_Init (void)
{
    Motor_Init();
    Encoder_Init();

    PID_Init(&g_pidL, ENGINE_KP, ENGINE_KI, ENGINE_KD, ENGINE_DT, ENGINE_OUT_MIN, ENGINE_OUT_MAX);
    PID_Init(&g_pidR, ENGINE_KP, ENGINE_KI, ENGINE_KD, ENGINE_DT, ENGINE_OUT_MIN, ENGINE_OUT_MAX);
}

/* ================================================================
 * 设定目标速度并立即输出前馈占空比
 * ================================================================ */
void Engine_SetSpeed (float left_mps, float right_mps)
{
    g_targetL = left_mps;
    g_targetR = right_mps;
    PID_Reset(&g_pidL);
    PID_Reset(&g_pidR);
    PID_Setpoint(&g_pidL, left_mps);
    PID_Setpoint(&g_pidR, right_mps);

    /* 前馈：直接给占空比，不等 PID 慢慢积 */
    int16_t dutyL = (int16_t)(left_mps * ENGINE_KFF);
    int16_t dutyR = (int16_t)(right_mps * ENGINE_KFF);

    /* 限幅 */
    if (dutyL > 999)
        dutyL = 999;
    if (dutyL < -999)
        dutyL = -999;
    if (dutyR > 999)
        dutyR = 999;
    if (dutyR < -999)
        dutyR = -999;

    Motor_Set(MOTOR_A, (dutyL >= 0) ? MOTOR_FWD : MOTOR_REV, (uint16_t)(dutyL >= 0 ? dutyL : -dutyL));
    Motor_Set(MOTOR_B, (dutyR >= 0) ? MOTOR_FWD : MOTOR_REV, (uint16_t)(dutyR >= 0 ? dutyR : -dutyR));
}

/* ================================================================
 * 直走
 * ================================================================ */
void Engine_GoStraight (float mps)
{
    Engine_SetSpeed(mps, mps);
}

/* ================================================================
 * 差速转向
 * ================================================================ */
void Engine_Turn (float left_mps, float right_mps)
{
    Engine_SetSpeed(left_mps, right_mps);
}

void Engine_Stop (void)
{
    g_targetL = 0;
    g_targetR = 0;
    Motor_Set(MOTOR_A, MOTOR_STOP, 0);
    Motor_Set(MOTOR_B, MOTOR_STOP, 0);
}

void Engine_Brake (void)
{
    g_targetL = 0;
    g_targetR = 0;
    Motor_Set(MOTOR_A, MOTOR_BRAKE, 999);
    Motor_Set(MOTOR_B, MOTOR_BRAKE, 999);
}

float Engine_GetLeftSpeed (void)
{
    return Encoder_GetMPS(ENC_A);
}

float Engine_GetRightSpeed (void)
{
    return Encoder_GetMPS(ENC_B);
}

/* ================================================================
 * PID 更新（每 20ms 调一次）
 *
 * 前馈 + PID 叠加：
 *   ff   = target × K_FF
 *   pid  = PID_Compute(target, actual)
 *   duty = clamp(ff + pid, 0, 999)
 * ================================================================ */
void Engine_Update (void)
{
    /* 目标为 0 → 保持刹车/空挡，不跑 PID */
    if (g_targetL == 0 && g_targetR == 0)
        return;

    float spdL = Encoder_GetMPS(ENC_A);
    float spdR = Encoder_GetMPS(ENC_B);

    /* 左轮：前馈 + PID（始终运行，积分自动累加应对阻力） */
    float ffL = g_targetL * ENGINE_KFF;
    float pidL = PID_Compute(&g_pidL, spdL);
    float dutyL = ffL + pidL;
    if (dutyL > 999)
        dutyL = 999;
    if (dutyL < -999)
        dutyL = -999;
    Motor_Set(MOTOR_A, (dutyL >= 0) ? MOTOR_FWD : MOTOR_REV, (uint16_t)(dutyL >= 0 ? dutyL : -dutyL));

    /* 右轮 */
    float ffR = g_targetR * ENGINE_KFF;
    float pidR = PID_Compute(&g_pidR, spdR);
    float dutyR = ffR + pidR;
    if (dutyR > 999)
        dutyR = 999;
    if (dutyR < -999)
        dutyR = -999;
    Motor_Set(MOTOR_B, (dutyR >= 0) ? MOTOR_FWD : MOTOR_REV, (uint16_t)(dutyR >= 0 ? dutyR : -dutyR));
}

/* ================================================================
 * 打印
 * ================================================================ */
void Engine_Print (void)
{
    SEGGER_RTT_printf(0, "ENG L:%.2f→%.2f  R:%.2f→%.2f m/s\n", Engine_GetLeftSpeed(), g_targetL, Engine_GetRightSpeed(), g_targetR);
}

/* ================================================================
 * 陀螺仪闭环旋转
 * 正角度=逆时针（左轮反转，右轮正转）
 * 负角度=顺时针
 * ================================================================ */
void Engine_Rotate (float angle_deg)
{
    float start_yaw = Position_YawDeg();
    float target_yaw = start_yaw + angle_deg;
    /* 速度 PID 锁轮速 0.1m/s, 方向硬设, 陀螺积分到位刹车 */
    float spd = 0.3f;
    PID_Reset(&g_pidL);
    PID_Reset(&g_pidR);
    PID_Setpoint(&g_pidL, spd);
    PID_Setpoint(&g_pidR, spd);

    printf("Rotate: %.1f° @ %.2f m/s per wheel\n", angle_deg, spd);

    bool first_phase = true;   /* 粗调阶段：±10° 刹车 */
    while (1) {
        delay_ms(20);
        Position_Update();
        float remain = target_yaw - Position_YawDeg();

        /* 到位 ±2° → 完成 */
        if (remain < 2.0f && remain > -2.0f) break;

        /* 粗调阶段：±10° 刹车 + 等待惯性停止 */
        if (first_phase && remain < 10.0f && remain > -10.0f) {
            Engine_Brake();
            delay_ms(500);
            PID_Reset(&g_pidL); PID_Reset(&g_pidR);  /* ★ 清旧积分 */
            PID_Setpoint(&g_pidL, spd); PID_Setpoint(&g_pidR, spd);
            first_phase = false;
            continue;
        }

        /* 精调：根据剩余角度决定旋转方向，remain>0 继续原向, <0 反向纠正 */
        bool need_reverse = (remain < 0);  /* 过冲了 → 反向 */
        bool go_ccw = (angle_deg > 0) ^ need_reverse;  /* 原向CCW XOR 需要反向 = 当前实际方向 */

        float dutyL = PID_Compute(&g_pidL, Encoder_GetMPS(ENC_A));
        float dutyR = PID_Compute(&g_pidR, Encoder_GetMPS(ENC_B));
        uint16_t dL = (uint16_t)(dutyL > 0 ? dutyL : 1);
        uint16_t dR = (uint16_t)(dutyR > 0 ? dutyR : 1);

        if (go_ccw) {
            Motor_Set(MOTOR_A, MOTOR_REV, dL);   /* A后退 B前进 → CCW */
            Motor_Set(MOTOR_B, MOTOR_FWD, dR);
        } else {
            Motor_Set(MOTOR_A, MOTOR_FWD, dL);   /* A前进 B后退 → CW */
            Motor_Set(MOTOR_B, MOTOR_REV, dR);
        }
    }

    Engine_Brake();
    printf("Rotate done: %.1f°\n", Position_YawDeg());
}
