/**
 * PID.h — 位置式 PID, 支持 ±速度
 *   P = Kp·e, I = Ki·∫e·dt (限幅), D = Kd·(prev_meas - meas)/dt
 */

#ifndef PID_H
#define PID_H

typedef struct {
    float Kp, Ki, Kd;
    float dt;
    float out_min, out_max;
    float i_max;            /* 积分绝对值上限, H_FF/ki_ratio合理值 */
    float integral;
    float prev_measured;
    float prev_error;
} PID_Ctrl;

void  PID_Init(PID_Ctrl *p, float Kp, float Ki, float Kd,
               float dt, float out_min, float out_max, float i_max);
void  PID_SetTunings(PID_Ctrl *p, float Kp, float Ki, float Kd);
void  PID_Reset(PID_Ctrl *p);
float PID_Compute(PID_Ctrl *p, float setpoint, float measured);

#endif
