#include "main.h"

void SYSCFG_DL_init (void)
{
    DL_GPIO_enablePower(GPIOA);
    DL_GPIO_enablePower(GPIOB);
    DL_TimerG_reset(PWM_A_B_INST);
    DL_TimerG_reset(CAP_A_INST);
    DL_TimerG_reset(CAP_B_INST);
    DL_I2C_reset(I2C_0_INST);
    DL_UART_Main_reset(UART_0_INST);
    DL_UART_Main_reset(UART_1_CAM_INST);
    DL_TimerG_enablePower(PWM_A_B_INST);
    DL_TimerG_enablePower(CAP_A_INST);
    DL_TimerG_enablePower(CAP_B_INST);
    DL_I2C_enablePower(I2C_0_INST);
    DL_UART_Main_enablePower(UART_0_INST);
    DL_UART_Main_enablePower(UART_1_CAM_INST);
    delay_cycles(POWER_STARTUP_DELAY);
    SYSCFG_DL_GPIO_init();
    SYSCFG_DL_SYSCTL_init();
    SYSCFG_DL_PWM_A_B_init();
    SYSCFG_DL_CAP_A_init();
    SYSCFG_DL_CAP_B_init();
    SYSCFG_DL_CAP_B_B_init();
    SYSCFG_DL_CAP_A_A_init();
    DL_TimerG_setCCPDirection(CAP_A_INST, DL_TIMER_CC0_INPUT | DL_TIMER_CC1_INPUT);
    DL_TimerG_setCCPDirection(CAP_B_INST, DL_TIMER_CC0_INPUT | DL_TIMER_CC1_INPUT);
    SYSCFG_DL_I2C_0_init();
    SYSCFG_DL_UART_0_init();
    SYSCFG_DL_UART_1_CAM_init();
}

int main (void)
{
    SYSCFG_DL_init();
    delay_init();
    SEGGER_RTT_Init();

    Motor_Init();
    Encoder_Init();

    printf("pulseA cc1A pulseB cc1B\n");

    while (1) {
        delay_ms(200);
        printf("%6ld %5lu  %6ld %5lu\n",
            (long)Encoder_GetPulse(ENC_A), (unsigned long)g_diag_cc1A,
            (long)Encoder_GetPulse(ENC_B), (unsigned long)g_diag_cc1B);
    }
}
