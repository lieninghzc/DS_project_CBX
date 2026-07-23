/**
    ******************************************************************************
    * @file      startup_mspm0g3507xx.s
    * @author    FELIX LIAO
    * @brief     MSPM0G3507xx Devices vector table for GCC based toolchains. 
    *            This module performs:
    *                - Set the initial SP
    *                - Set the initial PC == Reset_Handler,
    *                - Set the vector table entries with the exceptions ISR address
    *                - Branches to main in the C library (which eventually
    *                  calls main()).
    *            After Reset the Cortex-M4 processor is in Thread mode,
    *            priority is Privileged, and the Stack is set to Main.
    ******************************************************************************
    * @attention
    *
    * Copyright (c) 2026 FELIX LIAO.
    * All rights reserved.
    *
    * This software is licensed under terms that can be found in the LICENSE file
    * in the root directory of this software component.
    * If no LICENSE file comes with this software, it is provided AS-IS.
    *
    ******************************************************************************
    */
        
    .syntax unified
    .cpu cortex-m0
    .fpu softvfp
    .thumb

.global  g_pfnVectors
.global  Default_Handler

/* start address for the initialization values of the .data section. 
defined in linker script */
.word  _sidata
/* start address for the .data section. defined in linker script */  
.word  _sdata
/* end address for the .data section. defined in linker script */
.word  _edata
/* start address for the .bss section. defined in linker script */
.word  _sbss
/* end address for the .bss section. defined in linker script */
.word  _ebss
/* stack used for SystemInit_ExtMemCtl; always internal RAM used */

/**
 * @brief  This is the code that gets called when the processor first
 *          starts execution following a reset event. Only the absolutely
 *          necessary set is performed, after which the application
 *          supplied main() routine is called. 
 * @param  None
 * @retval : None
*/

    .section  .text.Reset_Handler
    .weak  Reset_Handler
    .type  Reset_Handler, %function
Reset_Handler:

/* Copy the data segment initializers from flash to SRAM */  
    ldr r0, =_sdata
    ldr r1, =_edata
    ldr r2, =_sidata
    movs r3, #0
    b LoopCopyDataInit

CopyDataInit:
    ldr r4, [r2, r3]
    str r4, [r0, r3]
    adds r3, r3, #4

LoopCopyDataInit:
    adds r4, r0, r3
    cmp r4, r1
    bcc CopyDataInit
    
/* Zero fill the bss segment. */
    ldr r2, =_sbss
    ldr r4, =_ebss
    movs r3, #0
    b LoopFillZerobss

FillZerobss:
    str  r3, [r2]
    adds r2, r2, #4

LoopFillZerobss:
    cmp r2, r4
    bcc FillZerobss

/* Call the clock system initialization function.*/
    bl  SYSCFG_DL_init
/* Call static constructors */
    bl  __libc_init_array
/* Call the application's entry point.*/
    bl  main
    bx  lr    
.size  Reset_Handler, .-Reset_Handler

/**
 * @brief  This is the code that gets called when the processor receives an 
 *         unexpected interrupt.  This simply enters an infinite loop, preserving
 *         the system state for examination by a debugger.
 * @param  None     
 * @retval None       
*/
    .section  .text.Default_Handler,"ax",%progbits
Default_Handler:
Infinite_Loop:
    b  Infinite_Loop
    .size  Default_Handler, .-Default_Handler
/******************************************************************************
*
* The minimal vector table for a Cortex M4. Note that the proper constructs
* must be placed on this to ensure that it ends up at physical address
* 0x0000.0000.
* 
*******************************************************************************/
    .section  .isr_vector,"a",%progbits
    .type  g_pfnVectors, %object
        
g_pfnVectors:
    .word  _estack
    .word  Reset_Handler
    .word  NMI_Handler
    .word  HardFault_Handler
    .word  MemManage_Handler
    .word  BusFault_Handler
    .word  UsageFault_Handler
    .word  0
    .word  0
    .word  0
    .word  0
    .word  SVC_Handler
    .word  DebugMon_Handler
    .word  0
    .word  PendSV_Handler
    .word  SysTick_Handler
    
    /* External Interrupts */
    .word  GROUP0_IRQHandler               // 1: GROUP0 interrupt handler
    .word  GROUP1_IRQHandler               // 2: GROUP1 interrupt handler
    .word  TIMG8_IRQHandler                // 3: TIMG8 interrupt handler
    .word  UART3_IRQHandler                // 4: UART3 interrupt handler
    .word  ADC0_IRQHandler                 // 5: ADC0 interrupt handler
    .word  ADC1_IRQHandler                 // 6: ADC1 interrupt handler
    .word  CANFD0_IRQHandler               // 7: CANFD0 interrupt handler
    .word  DAC0_IRQHandler                 // 8: DAC0 interrupt handler
    .word  0                               // 9: Reserved
    .word  SPI0_IRQHandler                 // 10: SPI0 interrupt handler
    .word  SPI1_IRQHandler                 // 11: SPI1 interrupt handler
    .word  0                               // 12: Reserved
    .word  0                               // 13: Reserved
    .word  UART1_IRQHandler                // 14: UART1 interrupt handler
    .word  UART2_IRQHandler                // 15: UART2 interrupt handler
    .word  UART0_IRQHandler                // 16: UART0 interrupt handler
    .word  TIMG0_IRQHandler                // 17: TIMG0 interrupt handler
    .word  TIMG6_IRQHandler                // 18: TIMG6 interrupt handler
    .word  TIMA0_IRQHandler                // 19: TIMA0 interrupt handler
    .word  TIMA1_IRQHandler                // 20: TIMA1 interrupt handler
    .word  TIMG7_IRQHandler                // 21: TIMG7 interrupt handler
    .word  TIMG12_IRQHandler               // 22: TIMG12 interrupt handler
    .word  0                               // 23: Reserved
    .word  0                               // 24: Reserved
    .word  I2C0_IRQHandler                 // 25: I2C0 interrupt handler
    .word  I2C1_IRQHandler                 // 26: I2C1 interrupt handler
    .word  0                               // 27: Reserved
    .word  0                               // 28: Reserved
    .word  AES_IRQHandler                  // 29: AES interrupt handler
    .word  0                               // 30: Reserved
    .word  RTC_IRQHandler                  // 31: RTC interrupt handler
    .word  DMA_IRQHandler                  // 32: DMA interrupt handler

    .size  g_pfnVectors, .-g_pfnVectors

/*******************************************************************************
*
* Provide weak aliases for each Exception handler to the Default_Handler. 
* As they are weak aliases, any function with the same name will override 
* this definition.
* 
*******************************************************************************/
    .weak      NMI_Handler
    .thumb_set NMI_Handler,Default_Handler

    .weak      HardFault_Handler
    .thumb_set HardFault_Handler,Default_Handler

    .weak      MemManage_Handler
    .thumb_set MemManage_Handler,Default_Handler

    .weak      BusFault_Handler
    .thumb_set BusFault_Handler,Default_Handler

    .weak      UsageFault_Handler
    .thumb_set UsageFault_Handler,Default_Handler

    .weak      SVC_Handler
    .thumb_set SVC_Handler,Default_Handler

    .weak      DebugMon_Handler
    .thumb_set DebugMon_Handler,Default_Handler

    .weak      PendSV_Handler
    .thumb_set PendSV_Handler,Default_Handler

    .weak      SysTick_Handler
    .thumb_set SysTick_Handler,Default_Handler              

    .weak      GROUP0_IRQHandler
    .thumb_set GROUP0_IRQHandler,Default_Handler

    .weak      GROUP1_IRQHandler
    .thumb_set GROUP1_IRQHandler,Default_Handler

    .weak      TIMG8_IRQHandler
    .thumb_set TIMG8_IRQHandler,Default_Handler

    .weak      UART3_IRQHandler
    .thumb_set UART3_IRQHandler,Default_Handler

    .weak      ADC0_IRQHandler
    .thumb_set ADC0_IRQHandler,Default_Handler

    .weak      ADC1_IRQHandler
    .thumb_set ADC1_IRQHandler,Default_Handler

    .weak      CANFD0_IRQHandler
    .thumb_set CANFD0_IRQHandler,Default_Handler

    .weak      DAC0_IRQHandler
    .thumb_set DAC0_IRQHandler,Default_Handler

    .weak      SPI0_IRQHandler
    .thumb_set SPI0_IRQHandler,Default_Handler

    .weak      SPI1_IRQHandler
    .thumb_set SPI1_IRQHandler,Default_Handler

    .weak      UART1_IRQHandler
    .thumb_set UART1_IRQHandler,Default_Handler

    .weak      UART2_IRQHandler
    .thumb_set UART2_IRQHandler,Default_Handler

    .weak      UART0_IRQHandler
    .thumb_set UART0_IRQHandler,Default_Handler

    .weak      TIMG0_IRQHandler
    .thumb_set TIMG0_IRQHandler,Default_Handler

    .weak      TIMG6_IRQHandler
    .thumb_set TIMG6_IRQHandler,Default_Handler

    .weak      TIMA0_IRQHandler
    .thumb_set TIMA0_IRQHandler,Default_Handler

    .weak      TIMA1_IRQHandler
    .thumb_set TIMA1_IRQHandler,Default_Handler

    .weak      TIMG7_IRQHandler
    .thumb_set TIMG7_IRQHandler,Default_Handler

    .weak      TIMG12_IRQHandler
    .thumb_set TIMG12_IRQHandler,Default_Handler

    .weak      I2C0_IRQHandler
    .thumb_set I2C0_IRQHandler,Default_Handler

    .weak      I2C1_IRQHandler
    .thumb_set I2C1_IRQHandler,Default_Handler

    .weak      AES_IRQHandler
    .thumb_set AES_IRQHandler,Default_Handler

    .weak      RTC_IRQHandler
    .thumb_set RTC_IRQHandler,Default_Handler

    .weak      DMA_IRQHandler
    .thumb_set DMA_IRQHandler,Default_Handler
