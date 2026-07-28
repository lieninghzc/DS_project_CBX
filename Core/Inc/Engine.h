/**
 * @file    Engine.h
 * @brief   双电机运动控制（前馈 + PID 闭环）
 *
 * 控制策略：
 *   duty = K_FF × target_mps + PID(target_mps, actual_mps)
 *   前馈项直接给定大致占空比，PID 仅修正误差 → 响应更快更稳
 *
 * 坐标系（俯视）：
 *   左轮 = Motor A, 右轮 = Motor B
 *   前进 = 两轮正转, 后退 = 两轮反转
 *   左转 = 左轮慢右轮快, 右转 = 左轮快右轮慢
 */

#ifndef ENGINE_H
#define ENGINE_H

#include <stdint.h>
#include <stdbool.h>

/* PID 参数 */
#define ENGINE_KP 50.0f
#define ENGINE_KI 400.0f
#define ENGINE_KD 0.0f
#define ENGINE_DT 0.02f /* 20ms 控制周期 */
#define ENGINE_OUT_MIN -999
#define ENGINE_OUT_MAX 999

/* 前馈系数：duty = target_mps × K_FF */
#define ENGINE_KFF 500.0f /* 100→0.14, 200→0.38, 300→0.63, 400→0.81, 500→0.95 */

/** 初始化 Engine（Motor + Encoder + PID） */
void Engine_Init (void);

/** 设定左右轮目标线速度 (m/s)，自动 PID 闭环 */
void Engine_SetSpeed (float left_mps, float right_mps);

/** 直走 */
void Engine_GoStraight (float mps);

/** 差速转向（正=前进，负=后退，差值越大转弯越急） */
void Engine_Turn (float left_mps, float right_mps);

/** 空挡滑行 */
void Engine_Stop (void);

/** 刹车锁死 */
void Engine_Brake (void);

/** 获取当前左轮实际速度 */
float Engine_GetLeftSpeed (void);

/** 获取当前右轮实际速度 */
float Engine_GetRightSpeed (void);

/** PID 控制更新（需以 20ms 周期在主循环调用） */
void Engine_Update (void);

/** 打印双轮速度 */
void Engine_Print (void);

/** 陀螺仪闭环旋转（°），正=逆时针，负=顺时针，阻塞直到完成 */
void Engine_Rotate (float angle_deg);

#endif
