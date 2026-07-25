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

    Engine_Init();
    Maix_Init();

    /* 测试：直走 0.3 m/s */
    Engine_GoStraight(0.3f);

    while (1) {
        delay_ms(1);

        /* PID 更新（每 20ms） */
        static uint32_t ctrl = 0;
        if (++ctrl >= 20) { ctrl = 0; Engine_Update(); }

        /* MaixCAM UART */
        Maix_Poll();

        /* LED 500ms */
        static uint32_t blink = 0;
        if (++blink >= 500) { blink = 0;
            DL_GPIO_togglePins(GPIO_LEDS_PORT,
                GPIO_LEDS_USER_LED_1_PIN | GPIO_LEDS_USER_LED_2_PIN);
        }

        /* 速度打印每秒一次 */
        static uint32_t sec = 0;
        if (++sec >= 1000) { sec = 0; Engine_Print(); }
    }
}
