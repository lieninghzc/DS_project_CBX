/**
 * Motor.c — FWD=AIN1高/AIN2低, A电机对向安装需反转
 */
#include "main.h"
#include "Motor.h"

#define PWM_MAX  999U
#define MOTOR_A_INVERT  1  /* A对向: Motor_Set(FWD)→物理REV→小车前进 */

static uint16_t  g_duty[2] = {0, 0};
static Motor_Dir g_dir[2]  = {MOTOR_STOP, MOTOR_STOP};

void Motor_Init(void)
{
    DL_TimerG_setCaptureCompareValue(PWM_A_B_INST, 1, DL_TIMERG_CAPTURE_COMPARE_0_INDEX);
    DL_TimerG_setCaptureCompareValue(PWM_A_B_INST, 1, DL_TIMERG_CAPTURE_COMPARE_1_INDEX);
    Motor_SetDir(MOTOR_A, MOTOR_STOP);
    Motor_SetDir(MOTOR_B, MOTOR_STOP);
    DL_TimerG_startCounter(PWM_A_B_INST);
}

void Motor_SetDuty(Motor_ID id, uint16_t duty)
{
    if (duty > PWM_MAX) duty = PWM_MAX;
    if (duty == 0) duty = 1;
    g_duty[id] = duty;
    uint16_t ccr = PWM_MAX - duty;
    uint32_t idx = (id == MOTOR_A) ? DL_TIMERG_CAPTURE_COMPARE_0_INDEX
                                   : DL_TIMERG_CAPTURE_COMPARE_1_INDEX;
    DL_TimerG_setCaptureCompareValue(PWM_A_B_INST, ccr, idx);
}

void Motor_SetDir(Motor_ID id, Motor_Dir dir)
{
    g_dir[id] = dir;
    GPIO_Regs *port;
    uint32_t p1, p2;

    if (id == MOTOR_A) {
        port = A_DIRECT_PORT; p1 = A_DIRECT_AIN_1_PIN; p2 = A_DIRECT_AIN_2_PIN;
#if MOTOR_A_INVERT
        if (dir == MOTOR_FWD)      dir = MOTOR_REV;
        else if (dir == MOTOR_REV) dir = MOTOR_FWD;
#endif
    } else {
        port = B_DIRECT_PORT; p1 = B_DIRECT_BIN_1_PIN; p2 = B_DIRECT_BIN_2_PIN;
    }

    switch (dir) {
    case MOTOR_FWD:  DL_GPIO_setPins(port, p1);   DL_GPIO_clearPins(port, p2); break;
    case MOTOR_REV:  DL_GPIO_clearPins(port, p1); DL_GPIO_setPins(port, p2);   break;
    case MOTOR_BRAKE:DL_GPIO_setPins(port, p1|p2);                              break;
    default:         DL_GPIO_clearPins(port, p1|p2);                            break;
    }
}

void Motor_Set(Motor_ID id, Motor_Dir dir, uint16_t duty) { Motor_SetDir(id,dir); Motor_SetDuty(id,duty); }
void Motor_Stop(Motor_ID id)  { Motor_Set(id, MOTOR_STOP, 0); }
void Motor_Brake(Motor_ID id) { Motor_Set(id, MOTOR_BRAKE, 0); }
uint16_t  Motor_GetDuty(Motor_ID id) { return g_duty[id]; }
Motor_Dir Motor_GetDir(Motor_ID id)  { return g_dir[id]; }
