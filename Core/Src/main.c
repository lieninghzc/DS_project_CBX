#include "main.h"

int main (void)
{
    SYSCFG_DL_init();
    delay_init();
    SEGGER_RTT_Init();
    //XJ_I2C_Init();
    //printf("XJ I2C Init OK\n");
    MPU6050_Init();
    printf("Gyroscope Init OK\n");

    printf("\n=== MSPM0G3507 Startup ===\n");
    printf("CPUCLK: %lu Hz\n", (unsigned long)CPUCLK_FREQ);

    //uint32_t count = 0;

    while (1)
    {
        delay_ms(500);
        //DL_GPIO_togglePins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_1_PIN);
        //DL_GPIO_togglePins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_2_PIN);
        //XJ_PrintSensors();
        MPU6050_Print();
    }
}
