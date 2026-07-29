/**
 * Servo.h — 360° 连续旋转舵机 PWM 驱动
 *
 * 脉宽: 500~2500us, 周期 20ms (50Hz)
 *   1500us = 停止
 *   <1500  → 反转加速
 *   >1500  → 正转加速
 *
 * 用法:
 *   Servo_Init();
 *   Servo_SetSpeed(-50);  // 50% 反向
 *   Servo_SetSpeed(100);  // 100% 正向
 *   Servo_Stop();         // 停止
 */

#ifndef SERVO_H
#define SERVO_H

#include <stdint.h>

void Servo_Init(void);
void Servo_SetSpeed(int16_t speed);  /* -1000~+1000, 负=反转 正=正转, 0=停止 */
void Servo_Stop(void);

#endif
