/**
 * PID.c — 位置式 PI (+D可选), 条件积分抗饱和
 */

#include "main.h"
#include "PID.h"
#include <string.h>

void PID_Init(PID_Ctrl *p, float Kp, float Ki, float Kd,
              float dt, float out_min, float out_max, float i_max)
{
    memset(p, 0, sizeof(*p));
    p->Kp = Kp; p->Ki = Ki; p->Kd = Kd;
    p->dt = dt;
    p->out_min = out_min; p->out_max = out_max;
    p->i_max   = i_max;
}

void PID_SetTunings(PID_Ctrl *p, float Kp, float Ki, float Kd)
{ p->Kp = Kp; p->Ki = Ki; p->Kd = Kd; }

void PID_Reset(PID_Ctrl *p)
{ p->integral = 0; p->prev_measured = 0; p->prev_error = 0; }

float PID_Compute(PID_Ctrl *p, float setpoint, float measured)
{
    float error = setpoint - measured;
    float P = p->Kp * error;

    /* D — 测量值微分, 只算一次 */
    float D = 0;
    if (p->Kd > 0) {
        D = p->Kd * (p->prev_measured - measured) / p->dt;
        p->prev_measured = measured;
    }

    float I = p->Ki * p->integral;
    float out_raw = P + I + D;

    /* 条件积分: 输出饱和且误差同向 → 冻结积分 */
    if (!((out_raw >= p->out_max && error > 0) ||
          (out_raw <= p->out_min && error < 0))) {
        p->integral += error * p->dt;
    }

    if (p->integral >  p->i_max) p->integral =  p->i_max;
    if (p->integral < -p->i_max) p->integral = -p->i_max;

    float output = P + p->Ki * p->integral + D;
    if (output > p->out_max) output = p->out_max;
    if (output < p->out_min) output = p->out_min;

    p->prev_error = error;
    return output;
}
