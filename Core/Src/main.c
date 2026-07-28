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

    DL_GPIO_initDigitalOutput(GPIO_LEDS_USER_LED_1_IOMUX);
    DL_GPIO_initDigitalOutput(GPIO_LEDS_USER_LED_2_IOMUX);
    DL_GPIO_setPins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_1_PIN | GPIO_LEDS_USER_LED_2_PIN);
    DL_GPIO_enableOutput(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_1_PIN | GPIO_LEDS_USER_LED_2_PIN);

    Engine_Init();

    printf("\n===== Engine Test =====\n");

    printf("--- FWD 0.3 ---\n");
    Engine_GoStraight(0.3f);
    for (int i = 0; i < 100; i++) { delay_ms(20); Engine_Update();
        if (i % 10 == 0) printf("  t=%.1f L=%.2f(%ld) R=%.2f(%ld)\n", i*0.02f, (double)Engine_GetLeftSpeed(), (long)Encoder_GetPulse(ENC_A), (double)Engine_GetRightSpeed(), (long)Encoder_GetPulse(ENC_B)); }
    Engine_Brake(); delay_ms(300);

    printf("--- REV 0.3 ---\n");
    Engine_GoStraight(-0.3f);
    for (int i = 0; i < 100; i++) { delay_ms(20); Engine_Update();
        if (i % 10 == 0) printf("  t=%.1f L=%.2f(%ld) R=%.2f(%ld)\n", i*0.02f, (double)Engine_GetLeftSpeed(), (long)Encoder_GetPulse(ENC_A), (double)Engine_GetRightSpeed(), (long)Encoder_GetPulse(ENC_B)); }
    Engine_Brake(); delay_ms(300);

    printf("--- Turn L=-0.3 R=+0.3 ---\n");
    Engine_Turn(-0.3f, 0.3f);
    for (int i = 0; i < 100; i++) { delay_ms(20); Engine_Update();
        if (i % 10 == 0) printf("  t=%.1f L=%.2f(%ld) R=%.2f(%ld)\n", i*0.02f, (double)Engine_GetLeftSpeed(), (long)Encoder_GetPulse(ENC_A), (double)Engine_GetRightSpeed(), (long)Encoder_GetPulse(ENC_B)); }
    Engine_Brake(); delay_ms(300);

    printf("--- Turn L=+0.3 R=-0.3 ---\n");
    Engine_Turn(0.3f, -0.3f);
    for (int i = 0; i < 100; i++) { delay_ms(20); Engine_Update();
        if (i % 10 == 0) printf("  t=%.1f L=%.2f(%ld) R=%.2f(%ld)\n", i*0.02f, (double)Engine_GetLeftSpeed(), (long)Encoder_GetPulse(ENC_A), (double)Engine_GetRightSpeed(), (long)Encoder_GetPulse(ENC_B)); }
    Engine_Brake();

    printf("===== Done =====\n");
    while (1) { delay_ms(500); DL_GPIO_togglePins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_1_PIN); }
}
