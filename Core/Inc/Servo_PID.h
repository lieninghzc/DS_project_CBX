/**
 * Servo_PID.h — PD 控球, 10fps 相机
 *   err = target - pos → KP*err
 *   制动: KD * (pos - last_pos) / dt
 *   >3cm bang-bang, <3cm PD, <2mm 锁死
 */
#ifndef SERVO_PID_H
#define SERVO_PID_H

#include <stdint.h>

void ServoPID_Init(void);
void ServoPID_SetTarget(int16_t mm);       /* 目标位置, mm */
void ServoPID_Update(int16_t pos_mm);      /* 新一帧相机数据 (100ms/次) */
int8_t ServoPID_GetDir(void);              /* 读 Servo_DIR */

#endif
