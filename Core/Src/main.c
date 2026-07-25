#include "main.h"

/*
 * 覆盖 SysConfig 生成的 SYSCFG_DL_init (它是 weak 函数)
 * 跳过 DL_GPIO_reset(GPIOA/B) — 它们会杀死 SWD 引脚和电机方向引脚
 */
void SYSCFG_DL_init(void)
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
    SYSCFG_DL_I2C_0_init();
    SYSCFG_DL_UART_0_init();
    SYSCFG_DL_UART_1_CAM_init();
}

int main(void)
{
    SYSCFG_DL_init();

    /* 确保 LED 初始为灭（拉高=灭） */
    DL_GPIO_initDigitalOutput(GPIO_LEDS_USER_LED_1_IOMUX);
    DL_GPIO_initDigitalOutput(GPIO_LEDS_USER_LED_2_IOMUX);
    DL_GPIO_setPins(GPIO_LEDS_PORT,
        GPIO_LEDS_USER_LED_1_PIN | GPIO_LEDS_USER_LED_2_PIN);
    DL_GPIO_enableOutput(GPIO_LEDS_PORT,
        GPIO_LEDS_USER_LED_1_PIN | GPIO_LEDS_USER_LED_2_PIN);

    delay_init();
    SEGGER_RTT_Init();

    printf("\n=== MSPM0G3507 Startup ===\n");
    printf("CPUCLK: %lu Hz\n\n", (unsigned long)CPUCLK_FREQ);

    Encoder_Init();
    Maix_Init();
    Position_Init();

    bool gyro_ok = MPU6050_Init();
    if (gyro_ok) MPU6050_Calibrate();
    printf("GYRO:%d ENC:1 XJ:1\n\n", gyro_ok);

    while (1) {
        delay_ms(1);

        static uint32_t p = 0;
        if (++p >= 20) { p = 0; if (gyro_ok) Position_Update(); }

        Maix_Poll();

        /* ── LED 短闪（拉低亮, 50ms） + 打印 1s ── */
        static uint32_t t = 0;
        if (++t >= 50) {
            DL_GPIO_setPins(GPIO_LEDS_PORT,
                GPIO_LEDS_USER_LED_1_PIN | GPIO_LEDS_USER_LED_2_PIN); /* 拉高→灭 */
        }
        if (t >= 1000) { t = 0;
            DL_GPIO_clearPins(GPIO_LEDS_PORT,
                GPIO_LEDS_USER_LED_1_PIN | GPIO_LEDS_USER_LED_2_PIN); /* 拉低→亮 */

            float x, y, yaw;
            Position_GetDeg(&x, &y, &yaw);
            printf("POS: X=%+.3f Y=%+.3f YAW=%+.1f\n", x, y, yaw);

            XJ_PrintSensors();

            printf("ENC: A=%lu %.1fRPM B=%lu %.1fRPM\n---\n",
                Encoder_GetPulse(ENC_A), Encoder_GetRPM(ENC_A),
                Encoder_GetPulse(ENC_B), Encoder_GetRPM(ENC_B));
        }
    }
}
