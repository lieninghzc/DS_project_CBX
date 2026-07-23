#include "main.h"

int main(void)
{
    SYSCFG_DL_init();
    delay_init();
    SEGGER_RTT_Init();

    printf("\n=== MSPM0G3507 启动 ===\n");
    printf("CPUCLK: %lu Hz\n", (unsigned long)CPUCLK_FREQ);

    uint32_t count = 0;

    while (1) {
        delay_ms(500);
        DL_GPIO_togglePins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_1_PIN);
        printf("[%lu]\n", (unsigned long)++count);
    }
}
