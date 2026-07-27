/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_MSPM0G350X
#define CONFIG_MSPM0G3507

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)


#define GPIO_HFXT_PORT                                                     GPIOA
#define GPIO_HFXIN_PIN                                             DL_GPIO_PIN_5
#define GPIO_HFXIN_IOMUX                                         (IOMUX_PINCM10)
#define GPIO_HFXOUT_PIN                                            DL_GPIO_PIN_6
#define GPIO_HFXOUT_IOMUX                                        (IOMUX_PINCM11)
#define CPUCLK_FREQ                                                     80000000
/* Defines for SYSPLL_ERR_01 Workaround */
/* Represent 1.000 as 1000 */
#define FLOAT_TO_INT_SCALE                                               (1000U)
#define FCC_EXPECTED_RATIO                                                  2000
#define FCC_UPPER_BOUND                       (FCC_EXPECTED_RATIO * (1 + 0.003))
#define FCC_LOWER_BOUND                       (FCC_EXPECTED_RATIO * (1 - 0.003))

bool SYSCFG_DL_SYSCTL_SYSPLL_init(void);


/* Defines for PWM_A_B */
#define PWM_A_B_INST                                                       TIMG6
#define PWM_A_B_INST_IRQHandler                                 TIMG6_IRQHandler
#define PWM_A_B_INST_INT_IRQN                                   (TIMG6_INT_IRQn)
#define PWM_A_B_INST_CLK_FREQ                                           80000000
/* GPIO defines for channel 0 */
#define GPIO_PWM_A_B_C0_PORT                                               GPIOB
#define GPIO_PWM_A_B_C0_PIN                                       DL_GPIO_PIN_26
#define GPIO_PWM_A_B_C0_IOMUX                                    (IOMUX_PINCM57)
#define GPIO_PWM_A_B_C0_IOMUX_FUNC                   IOMUX_PINCM57_PF_TIMG6_CCP0
#define GPIO_PWM_A_B_C0_IDX                                  DL_TIMER_CC_0_INDEX
/* GPIO defines for channel 1 */
#define GPIO_PWM_A_B_C1_PORT                                               GPIOB
#define GPIO_PWM_A_B_C1_PIN                                       DL_GPIO_PIN_27
#define GPIO_PWM_A_B_C1_IOMUX                                    (IOMUX_PINCM58)
#define GPIO_PWM_A_B_C1_IOMUX_FUNC                   IOMUX_PINCM58_PF_TIMG6_CCP1
#define GPIO_PWM_A_B_C1_IDX                                  DL_TIMER_CC_1_INDEX



/* Defines for CAP_A */
#define CAP_A_INST                                                       (TIMG0)
#define CAP_A_INST_IRQHandler                                   TIMG0_IRQHandler
#define CAP_A_INST_INT_IRQN                                     (TIMG0_INT_IRQn)
#define CAP_A_INST_LOAD_VALUE                                           (25007U)
/* GPIO defines for channel 0 */
#define GPIO_CAP_A_C0_PORT                                                 GPIOB
#define GPIO_CAP_A_C0_PIN                                         DL_GPIO_PIN_10
#define GPIO_CAP_A_C0_IOMUX                                      (IOMUX_PINCM27)
#define GPIO_CAP_A_C0_IOMUX_FUNC                     IOMUX_PINCM27_PF_TIMG0_CCP0

/* Defines for CAP_B */
#define CAP_B_INST                                                      (TIMG12)
#define CAP_B_INST_IRQHandler                                  TIMG12_IRQHandler
#define CAP_B_INST_INT_IRQN                                    (TIMG12_INT_IRQn)
#define CAP_B_INST_LOAD_VALUE                                           (50015U)
/* GPIO defines for channel 0 */
#define GPIO_CAP_B_C0_PORT                                                 GPIOA
#define GPIO_CAP_B_C0_PIN                                         DL_GPIO_PIN_14
#define GPIO_CAP_B_C0_IOMUX                                      (IOMUX_PINCM36)
#define GPIO_CAP_B_C0_IOMUX_FUNC                    IOMUX_PINCM36_PF_TIMG12_CCP0






/* Defines for I2C_0 */
#define I2C_0_INST                                                          I2C0
#define I2C_0_INST_IRQHandler                                    I2C0_IRQHandler
#define I2C_0_INST_INT_IRQN                                        I2C0_INT_IRQn
#define I2C_0_BUS_SPEED_HZ                                                100000
#define GPIO_I2C_0_SDA_PORT                                                GPIOA
#define GPIO_I2C_0_SDA_PIN                                        DL_GPIO_PIN_10
#define GPIO_I2C_0_IOMUX_SDA                                     (IOMUX_PINCM21)
#define GPIO_I2C_0_IOMUX_SDA_FUNC                      IOMUX_PINCM21_PF_I2C0_SDA
#define GPIO_I2C_0_SCL_PORT                                                GPIOA
#define GPIO_I2C_0_SCL_PIN                                        DL_GPIO_PIN_11
#define GPIO_I2C_0_IOMUX_SCL                                     (IOMUX_PINCM22)
#define GPIO_I2C_0_IOMUX_SCL_FUNC                      IOMUX_PINCM22_PF_I2C0_SCL


/* Defines for UART_0 */
#define UART_0_INST                                                        UART0
#define UART_0_INST_FREQUENCY                                           40000000
#define UART_0_INST_IRQHandler                                  UART0_IRQHandler
#define UART_0_INST_INT_IRQN                                      UART0_INT_IRQn
#define GPIO_UART_0_RX_PORT                                                GPIOA
#define GPIO_UART_0_TX_PORT                                                GPIOA
#define GPIO_UART_0_RX_PIN                                         DL_GPIO_PIN_1
#define GPIO_UART_0_TX_PIN                                         DL_GPIO_PIN_0
#define GPIO_UART_0_IOMUX_RX                                      (IOMUX_PINCM2)
#define GPIO_UART_0_IOMUX_TX                                      (IOMUX_PINCM1)
#define GPIO_UART_0_IOMUX_RX_FUNC                       IOMUX_PINCM2_PF_UART0_RX
#define GPIO_UART_0_IOMUX_TX_FUNC                       IOMUX_PINCM1_PF_UART0_TX
#define UART_0_BAUD_RATE                                                  (9600)
#define UART_0_IBRD_40_MHZ_9600_BAUD                                       (260)
#define UART_0_FBRD_40_MHZ_9600_BAUD                                        (27)
/* Defines for UART_1_CAM */
#define UART_1_CAM_INST                                                    UART1
#define UART_1_CAM_INST_FREQUENCY                                       40000000
#define UART_1_CAM_INST_IRQHandler                              UART1_IRQHandler
#define UART_1_CAM_INST_INT_IRQN                                  UART1_INT_IRQn
#define GPIO_UART_1_CAM_RX_PORT                                            GPIOA
#define GPIO_UART_1_CAM_TX_PORT                                            GPIOA
#define GPIO_UART_1_CAM_RX_PIN                                     DL_GPIO_PIN_9
#define GPIO_UART_1_CAM_TX_PIN                                     DL_GPIO_PIN_8
#define GPIO_UART_1_CAM_IOMUX_RX                                 (IOMUX_PINCM20)
#define GPIO_UART_1_CAM_IOMUX_TX                                 (IOMUX_PINCM19)
#define GPIO_UART_1_CAM_IOMUX_RX_FUNC                  IOMUX_PINCM20_PF_UART1_RX
#define GPIO_UART_1_CAM_IOMUX_TX_FUNC                  IOMUX_PINCM19_PF_UART1_TX
#define UART_1_CAM_BAUD_RATE                                              (9600)
#define UART_1_CAM_IBRD_40_MHZ_9600_BAUD                                   (260)
#define UART_1_CAM_FBRD_40_MHZ_9600_BAUD                                    (27)





/* Port definition for Pin Group GPIO_LEDS */
#define GPIO_LEDS_PORT                                                   (GPIOA)

/* Defines for USER_LED_1: GPIOA.2 with pinCMx 7 on package pin 42 */
#define GPIO_LEDS_USER_LED_1_PIN                                 (DL_GPIO_PIN_2)
#define GPIO_LEDS_USER_LED_1_IOMUX                                (IOMUX_PINCM7)
/* Defines for USER_LED_2: GPIOA.7 with pinCMx 14 on package pin 49 */
#define GPIO_LEDS_USER_LED_2_PIN                                 (DL_GPIO_PIN_7)
#define GPIO_LEDS_USER_LED_2_IOMUX                               (IOMUX_PINCM14)
/* Port definition for Pin Group A_DIRECT */
#define A_DIRECT_PORT                                                    (GPIOA)

/* Defines for AIN_1: GPIOA.4 with pinCMx 9 on package pin 44 */
#define A_DIRECT_AIN_1_PIN                                       (DL_GPIO_PIN_4)
#define A_DIRECT_AIN_1_IOMUX                                      (IOMUX_PINCM9)
/* Defines for AIN_2: GPIOA.3 with pinCMx 8 on package pin 43 */
#define A_DIRECT_AIN_2_PIN                                       (DL_GPIO_PIN_3)
#define A_DIRECT_AIN_2_IOMUX                                      (IOMUX_PINCM8)
/* Port definition for Pin Group B_DIRECT */
#define B_DIRECT_PORT                                                    (GPIOB)

/* Defines for BIN_1: GPIOB.21 with pinCMx 49 on package pin 20 */
#define B_DIRECT_BIN_1_PIN                                      (DL_GPIO_PIN_21)
#define B_DIRECT_BIN_1_IOMUX                                     (IOMUX_PINCM49)
/* Defines for BIN_2: GPIOB.22 with pinCMx 50 on package pin 21 */
#define B_DIRECT_BIN_2_PIN                                      (DL_GPIO_PIN_22)
#define B_DIRECT_BIN_2_IOMUX                                     (IOMUX_PINCM50)


/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);

bool SYSCFG_DL_SYSCTL_SYSPLL_init(void);
void SYSCFG_DL_PWM_A_B_init(void);
void SYSCFG_DL_CAP_A_init(void);
void SYSCFG_DL_CAP_B_init(void);
void SYSCFG_DL_I2C_0_init(void);
void SYSCFG_DL_UART_0_init(void);
void SYSCFG_DL_UART_1_CAM_init(void);


bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
