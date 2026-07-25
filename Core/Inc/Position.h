/**
 * @file    Position.h
 * @brief   里程计 + 姿态估计（相对于起点）
 *
 * 数据源：
 *   编码器 → 左右轮位移 → 合位移
 *   陀螺仪 Z 轴 → 积分 → 偏航角 (Yaw)
 *
 * 差速底盘运动模型：
 *   Δd = (Δd_L + Δd_R) / 2         合位移
 *   θ  = ∫ω_z dt                    偏航角（陀螺 Z 积分）
 *   Δx = Δd × cos(θ)               X 轴增量
 *   Δy = Δd × sin(θ)               Y 轴增量
 *
 * 坐标系（右手系，俯视）：
 *   X 轴 = 车头正前方
 *   Y 轴 = 车体左侧
 *   θ   = 逆时针为正（0° = 车头方向）
 */

#ifndef POSITION_H
#define POSITION_H

#include <stdint.h>
#include <stdbool.h>

/** 当前位置与姿态 */
typedef struct {
    float x;       /* X 坐标 (m), 前方为正 */
    float y;       /* Y 坐标 (m), 左方为正 */
    float yaw;     /* 偏航角 (rad), 0=初始朝向, +CCW */
} Pose;

/** 初始化：记录起点 */
void Position_Init(void);

/** 增量更新里程计（每 10~20ms 调用一次） */
void Position_Update(void);

/** 获取当前位置 */
Pose Position_Get(void);

/** 获取当前位置 (x, y, yaw_deg) */
void Position_GetDeg(float *x, float *y, float *yaw_deg);

/** 距离起点直线距离 */
float Position_Distance(void);

/** 偏航角 (°) */
float Position_YawDeg(void);

/** 重置到原点 */
void Position_Reset(void);

#endif
