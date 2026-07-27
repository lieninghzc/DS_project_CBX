/**
 * @file    StateMachine.h
 * @brief   比赛状态机框架
 *
 * 状态转移:
 *   INIT  → CALIB → WAIT → RUN → STOP
 *                      ↑       │
 *                      └───────┘ (紧急制动)
 */

#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <stdint.h>
#include <stdbool.h>

/* ── 状态枚举 ── */
typedef enum {
    SM_INIT = 0,   /* 上电初始化        */
    SM_CALIB,      /* 传感器校准        */
    SM_WAIT,       /* 等待发车信号      */
    SM_RUN,        /* 比赛中            */
    SM_STOP,       /* 停车              */
    SM_ERROR,      /* 异常/紧急制动     */
} SM_State;

/* ── 发车信号来源 ── */
typedef enum {
    TRIG_NONE = 0,
    TRIG_BUTTON,      /* 按键 */
    TRIG_UART,        /* 串口/蓝牙指令   */
    TRIG_TIMER,       /* 倒计时          */
    TRIG_AUTO,        /* 自动检测        */
} SM_Trigger;

/** 获取当前状态 */
SM_State SM_GetState(void);

/** 获取状态名称字符串 */
const char *SM_GetStateName(void);

/** 初始化状态机 */
void SM_Init(void);

/**
 * 状态转移（每轮主循环调用一次）
 * @param trigger  触发源
 * @return         状态是否发生了变化
 */
bool SM_Update(SM_Trigger trigger);

/** 强制跳转到指定状态 */
void SM_Force(SM_State state);

/** 手动发车（RUN 时调用） */
void SM_Start(void);

/** 紧急制动 */
void SM_EmergencyStop(void);

/** 校准完成通知 */
void SM_CalibDone(void);

#endif /* STATEMACHINE_H */
