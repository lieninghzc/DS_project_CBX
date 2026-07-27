#include "main.h"

void SYSCFG_DL_init(void)
{
    DL_GPIO_enablePower(GPIOA); DL_GPIO_enablePower(GPIOB);
    DL_TimerG_reset(PWM_A_B_INST);
    DL_TimerG_reset(CAP_A_INST); DL_TimerG_reset(CAP_B_INST);
    DL_I2C_reset(I2C_0_INST);
    DL_UART_Main_reset(UART_0_INST); DL_UART_Main_reset(UART_1_CAM_INST);
    DL_TimerG_enablePower(PWM_A_B_INST);
    DL_TimerG_enablePower(CAP_A_INST); DL_TimerG_enablePower(CAP_B_INST);
    DL_I2C_enablePower(I2C_0_INST);
    DL_UART_Main_enablePower(UART_0_INST); DL_UART_Main_enablePower(UART_1_CAM_INST);
    delay_cycles(POWER_STARTUP_DELAY);

    SYSCFG_DL_GPIO_init();     SYSCFG_DL_SYSCTL_init();
    SYSCFG_DL_PWM_A_B_init();  SYSCFG_DL_CAP_A_init();
    SYSCFG_DL_CAP_B_init();    SYSCFG_DL_I2C_0_init();
    SYSCFG_DL_UART_0_init();   SYSCFG_DL_UART_1_CAM_init();
}

int main(void)
{
    SYSCFG_DL_init();
    delay_init();
    SEGGER_RTT_Init();
    SM_Init();

    DL_GPIO_initDigitalOutput(GPIO_LEDS_USER_LED_1_IOMUX);
    DL_GPIO_initDigitalOutput(GPIO_LEDS_USER_LED_2_IOMUX);
    DL_GPIO_setPins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_1_PIN | GPIO_LEDS_USER_LED_2_PIN);
    DL_GPIO_enableOutput(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_1_PIN | GPIO_LEDS_USER_LED_2_PIN);

    printf("\n=== MSPM0G3507 Startup ===\n");
    printf("CPUCLK: %lu Hz\n\n", (unsigned long)CPUCLK_FREQ);

    Encoder_Init();  Maix_Init();  BT_Init();
    Position_Init(); OLED_Init();  OLED_Clear();

    if (MPU6050_Init()) MPU6050_Calibrate();

    uint32_t last = g_msTick;
    while (1) {
        while (g_msTick == last);
        last = g_msTick;
        SM_Run();
    }
}
