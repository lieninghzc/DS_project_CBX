/**
 * @file    StateMachine.h
 * @brief   比赛状态机 —— 封装全部比赛流程
 *
 * 状态:
 *   INIT → CALIB → WAIT → RUN → STOP
 *
 * 用法: SM_Init() 初始化, while(1) SM_Run() 循环
 */

#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <stdint.h>

typedef enum {
    SM_INIT = 0,
    SM_CALIB,
    SM_WAIT,
    SM_RUN,
    SM_STOP,
    SM_ERROR,
} SM_State;

/** 获取当前状态 */
SM_State SM_GetState(void);

/** 初始化 */
void SM_Init(void);

/** 每轮主循环调用一次 */
void SM_Run(void);

#endif
