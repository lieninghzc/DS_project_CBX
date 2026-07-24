/**
 * @file    Motor.h
 * @brief   双路直流电机 PWM 驱动
 *
 * 硬件连接（SysConfig）：
 *   PWM_A:  TIMG6 CCP0 → PB26
 *   PWM_B:  TIMG6 CCP1 → PB27
 *   A 方向: AIN1=PA4, AIN2=PA3（A_DIRECT）
 *   B 方向: BIN1=PB21, BIN2=PB22（B_DIRECT）
 *
 * 方向控制逻辑：
 *   AIN1=H, AIN2=L  → 正转
 *   AIN1=L, AIN2=H  → 反转
 *   AIN1=H, AIN2=H  → 锁死（刹车）
 *   AIN1=L, AIN2=L  → 空挡（自由滑行）
 */

#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>
#include <stdbool.h>

typedef enum { MOTOR_A = 0, MOTOR_B = 1 } Motor_ID;

typedef enum {
    MOTOR_STOP  = 0,   /* 空挡 */
    MOTOR_FWD   = 1,   /* 正转 */
    MOTOR_REV   = 2,   /* 反转 */
    MOTOR_BRAKE = 3,   /* 锁死 */
} Motor_Dir;

/** 初始化（SysConfig 已配置 PWM 和方向引脚） */
void Motor_Init(void);

/** 设置 PWM 占空比（0~1000，即 0.0% ~ 100.0%） */
void Motor_SetDuty(Motor_ID id, uint16_t duty);

/** 设置方向 */
void Motor_SetDir(Motor_ID id, Motor_Dir dir);

/** 同时设置方向和占空比 */
void Motor_Set(Motor_ID id, Motor_Dir dir, uint16_t duty);

/** 停止（空挡） */
void Motor_Stop(Motor_ID id);

/** 刹车（锁死） */
void Motor_Brake(Motor_ID id);

/** 获取当前占空比 */
uint16_t Motor_GetDuty(Motor_ID id);

/** 获取当前方向 */
Motor_Dir Motor_GetDir(Motor_ID id);

#endif /* MOTOR_H */
