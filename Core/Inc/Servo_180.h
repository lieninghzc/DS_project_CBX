/**
 * Servo_180.h — SG90 180° 舵机, PWM 位置控制
 *   PWM_Servo: TIMG8 CCP0 PA29, 50kHz, period=1000 (20ms)
 */
#ifndef SERVO_180_H
#define SERVO_180_H

#include <stdint.h>

void Servo180_Init(void);
void Servo180_SetAngle(uint8_t deg);    /* 0~180° */
void Servo180_SetUs(uint16_t us);       /* 500~2500us 直接设脉宽 */

#endif
