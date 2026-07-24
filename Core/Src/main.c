#include "main.h"

int main(void)
{
    SYSCFG_DL_init();

    /* GPIOA reset 会干掉 SWD，补回 PA19(SWDIO) + PA20(SWCLK) */
    DL_GPIO_initPeripheralInputFunctionFeatures(
        IOMUX_PINCM41, IOMUX_PINCM41_PF_DEBUGSS_SWDIO,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initPeripheralInputFunctionFeatures(
        IOMUX_PINCM42, IOMUX_PINCM42_PF_DEBUGSS_SWCLK,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    delay_init();
    SEGGER_RTT_Init();

    printf("\n=== MSPM0G3507 Startup ===\n");
    printf("CPUCLK: %lu Hz\n\n", (unsigned long)CPUCLK_FREQ);

    Encoder_Init();

    while (1) {
        delay_ms(20);

        /* LED 500ms */
        static uint32_t blink = 0;
        if (++blink >= 25) { blink = 0;
            DL_GPIO_togglePins(GPIO_LEDS_PORT,
                GPIO_LEDS_USER_LED_1_PIN | GPIO_LEDS_USER_LED_2_PIN);
        }

        /* 编码器每秒一次 */
        static uint32_t sec = 0;
        if (++sec >= 50) { sec = 0;
            uint32_t pA = Encoder_GetPulse(ENC_A);
            uint32_t pB = Encoder_GetPulse(ENC_B);
            int rpmA = (int)Encoder_GetRPM(ENC_A);
            int rpmB = (int)Encoder_GetRPM(ENC_B);
            SEGGER_RTT_printf(0, "ENC A:%lu pulse %d RPM  B:%lu pulse %d RPM  run:%d %d\n",
                pA, rpmA, pB, rpmB, Encoder_IsRunning(ENC_A), Encoder_IsRunning(ENC_B));
        }
    }
}
