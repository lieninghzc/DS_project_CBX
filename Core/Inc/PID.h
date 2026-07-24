/**
 * @file    PID.h
 * @brief   PID 闭环控制器框架
 *
 * 离散 PID 公式（位置式）：
 *   u[k] = Kp·e[k] + Ki·Σe[i]·dt + Kd·(e[k]-e[k-1])/dt
 *
 * 输出 = u[k]（直接作为 PWM 占空比）
 */

#ifndef PID_H
#define PID_H

#include <stdint.h>
#include <stdbool.h>

/* PID 参数 */
typedef struct {
    float Kp, Ki, Kd;       /* 比例/积分/微分系数 */
    float setpoint;          /* 目标值 */
    float output;            /* 控制器输出 */
    float out_min, out_max;  /* 输出限幅（防积分饱和） */
    float integral;          /* 累计积分 */
    float prev_error;        /* 上一次误差 */
    float dt;                /* 控制周期（秒） */
} PID_Ctrl;

/** 初始化 PID 控制器 */
void PID_Init(PID_Ctrl *pid, float Kp, float Ki, float Kd,
              float dt, float out_min, float out_max);

/** 设置目标值 */
void PID_Setpoint(PID_Ctrl *pid, float sp);

/** 单步计算：输入当前测量值，返回控制量 */
float PID_Compute(PID_Ctrl *pid, float measured);

/** 重置 PID（清积分和上次误差） */
void PID_Reset(PID_Ctrl *pid);

/** 整定模式：设置系数 */
void PID_SetTunings(PID_Ctrl *pid, float Kp, float Ki, float Kd);

#endif /* PID_H */
