/**
 * @file    PID.c
 * @brief   PID 闭环控制器实现
 *
 * 离散位置式 PID：
 *   error = setpoint - measured
 *   P = Kp × error
 *   I = Ki × Σ(error × dt), 带积分限幅（防饱和）
 *   D = Kd × (error - prev_error) / dt
 *   output = clamp(P + I + D, out_min, out_max)
 *
 * 典型应用：
 *   车速控制：setpoint=目标RPM, measured=实际RPM, output=PWM占空比
 */

#include "main.h"
#include "PID.h"
#include <string.h>

/* ================================================================
 * 初始化
 * ================================================================ */
void PID_Init(PID_Ctrl *pid, float Kp, float Ki, float Kd,
              float dt, float out_min, float out_max)
{
    memset(pid, 0, sizeof(*pid));
    pid->Kp      = Kp;
    pid->Ki      = Ki;
    pid->Kd      = Kd;
    pid->dt      = dt;
    pid->out_min = out_min;
    pid->out_max = out_max;
}

/* ================================================================
 * 设置目标值
 * ================================================================ */
void PID_Setpoint(PID_Ctrl *pid, float sp)
{
    pid->setpoint = sp;
}

/* ================================================================
 * 单步计算（位置式）
 *
 * @param measured  当前测量值（传感器反馈）
 * @return          控制输出 u（已限幅）
 * ================================================================ */
float PID_Compute(PID_Ctrl *pid, float measured)
{
    /* 误差 */
    float error = pid->setpoint - measured;

    /* 比例项 */
    float P = pid->Kp * error;

    /* 积分项（带限幅防饱和） */
    pid->integral += error * pid->dt;

    /* 积分限幅 */
    if (pid->integral >  pid->out_max) pid->integral = pid->out_max;
    if (pid->integral < -pid->out_max) pid->integral = -pid->out_max;

    float I = pid->Ki * pid->integral;

    /* 微分项（测量值微分，避免 setpoint 突变带来的微分冲击） */
    float deriv = (error - pid->prev_error) / pid->dt;
    pid->prev_error = error;

    float D = pid->Kd * deriv;

    /* 合成输出 */
    pid->output = P + I + D;

    /* 输出限幅 */
    if (pid->output > pid->out_max) pid->output = pid->out_max;
    if (pid->output < pid->out_min) pid->output = pid->out_min;

    return pid->output;
}

/* ================================================================
 * 重置（清积分和上次误差，保留系数）
 * ================================================================ */
void PID_Reset(PID_Ctrl *pid)
{
    pid->integral   = 0;
    pid->prev_error = 0;
    pid->output     = 0;
}

/* ================================================================
 * 运行时修改 PID 系数
 * ================================================================ */
void PID_SetTunings(PID_Ctrl *pid, float Kp, float Ki, float Kd)
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
}
