/*
 * Startup file for STM32F407VGT6
 * 中断向量表和复位处理
 */

    .syntax unified
    .arch armv7e-m
    .thumb

/* 外部引用链接脚本定义的符号 */
    .extern __stack_top__
    .extern __bss_start__
    .extern __bss_end__

/* 外部引用 */
    .extern main
    .extern SystemInit

/* 中断向量表 - 放在Flash起始地址 */
    .section .isr_vector, "a", %progbits
    .align 2
    .global g_pfnVectors
g_pfnVectors:
    .word __stack_top__               /* 初始栈指针 */
    .word Reset_Handler               /* 复位中断 */
    .word NMI_Handler                 /* NMI中断 */
    .word HardFault_Handler           /* 硬件错误中断 */
    .word MemManage_Handler           /* 内存管理错误 */
    .word BusFault_Handler            /* 总线错误 */
    .word UsageFault_Handler          /* 使用错误 */
    .word 0                           /* 保留 */
    .word 0                           /* 保留 */
    .word 0                           /* 保留 */
    .word 0                           /* 保留 */
    .word SVC_Handler                 /* SVCall */
    .word DebugMon_Handler            /* 调试监控 */
    .word 0                           /* 保留 */
    .word PendSV_Handler              /* PendSV */
    .word SysTick_Handler             /* SysTick */
    /* 外部中断 */
    .word WWDG_IRQHandler
    .word PVD_IRQHandler
    .word TAMP_STAMP_IRQHandler
    .word RTC_WKUP_IRQHandler
    .word FLASH_IRQHandler
    .word RCC_IRQHandler
    .word EXTI0_IRQHandler
    .word EXTI1_IRQHandler
    .word EXTI2_IRQHandler
    .word EXTI3_IRQHandler
    .word EXTI4_IRQHandler
    .word DMA1_Stream0_IRQHandler
    .word DMA1_Stream1_IRQHandler
    .word DMA1_Stream2_IRQHandler
    .word DMA1_Stream3_IRQHandler
    .word DMA1_Stream4_IRQHandler
    .word DMA1_Stream5_IRQHandler
    .word DMA1_Stream6_IRQHandler
    .word ADC_IRQHandler
    .word CAN1_TX_IRQHandler
    .word CAN1_RX0_IRQHandler
    .word CAN1_RX1_IRQHandler
    .word CAN1_SCE_IRQHandler
    .word EXTI9_5_IRQHandler
    .word TIM1_BRK_TIM9_IRQHandler
    .word TIM1_UP_TIM10_IRQHandler
    .word TIM1_TRG_COM_TIM11_IRQHandler
    .word TIM1_CC_IRQHandler
    .word TIM2_IRQHandler
    .word TIM3_IRQHandler
    .word TIM4_IRQHandler
    .word I2C1_EV_IRQHandler
    .word I2C1_ER_IRQHandler
    .word I2C2_EV_IRQHandler
    .word I2C2_ER_IRQHandler
    .word SPI1_IRQHandler
    .word SPI2_IRQHandler
    .word USART1_IRQHandler
    .word USART2_IRQHandler
    .word USART3_IRQHandler
    .word EXTI15_10_IRQHandler
    .word RTC_Alarm_IRQHandler
    .word OTG_FS_WKUP_IRQHandler
    .word 0
    .word 0
    .word 0
    .word 0
    .word DMA1_Stream7_IRQHandler
    .word FSMC_IRQHandler
    .word SDIO_IRQHandler
    .word TIM5_IRQHandler
    .word SPI3_IRQHandler
    .word UART4_IRQHandler
    .word UART5_IRQHandler
    .word TIM6_DAC_IRQHandler
    .word TIM7_IRQHandler
    .word DMA2_Stream0_IRQHandler
    .word DMA2_Stream1_IRQHandler
    .word DMA2_Stream2_IRQHandler
    .word DMA2_Stream3_IRQHandler
    .word DMA2_Stream4_IRQHandler
    .word ETH_IRQHandler
    .word ETH_WKUP_IRQHandler
    .word CAN2_TX_IRQHandler
    .word CAN2_RX0_IRQHandler
    .word CAN2_RX1_IRQHandler
    .word CAN2_SCE_IRQHandler
    .word OTG_FS_IRQHandler
    .word OTG_HS_IRQHandler
    .word DCMI_IRQHandler
    .word 0
    .word 0
    .word FPU_IRQHandler
    .word 0
    .word 0
    .word 0
    .word 0
    .word DMA2_Stream5_IRQHandler
    .word DMA2_Stream6_IRQHandler
    .word DMA2_Stream7_IRQHandler
    .word USART6_IRQHandler
    .word I2C3_EV_IRQHandler
    .word I2C3_ER_IRQHandler
    .word OTG_HS_EP1_OUT_IRQHandler
    .word OTG_HS_EP1_IN_IRQHandler
    .word OTG_HS_WKUP_IRQHandler
    .word OTG_HS_IRQHandler
    .word HASH_RNG_IRQHandler
    .word FPU_IRQHandler
    /* 向量表结束 */
    .size g_pfnVectors, . - g_pfnVectors

/* 复位处理 - 程序入口 */
    .text
    .align 2
    .global Reset_Handler
    .type Reset_Handler, %function
Reset_Handler:
    /* 清除BSS段 */
    ldr     r0, =__bss_start__
    ldr     r1, =__bss_end__
    mov     r2, #0
Loop:
    cmp     r0, r1
    bcc     Clear
    b       LoopDone
Clear:
    strb    r2, [r0], #1
    b       Loop
LoopDone:

    /* 调用系统初始化 */
    bl      SystemInit

    /* 跳转到main函数 */
    bl      main
    bx      lr
    .size Reset_Handler, . - Reset_Handler

/* 默认中断处理 - 无限循环 */
    .section .text.Default_Handler,"ax",%progbits
    .align 2
    .global Default_Handler
    .type Default_Handler, %function
Default_Handler:
    b       Default_Handler
    .size Default_Handler, . - Default_Handler

/* 为所有未实现的中断指定默认处理 */
    .macro  def_irq_handler name
    .weak   \name
    .thumb_set \name, Default_Handler
    .endm

    def_irq_handler NMI_Handler
    def_irq_handler HardFault_Handler
    def_irq_handler MemManage_Handler
    def_irq_handler BusFault_Handler
    def_irq_handler UsageFault_Handler
    def_irq_handler SVC_Handler
    def_irq_handler DebugMon_Handler
    def_irq_handler PendSV_Handler
    def_irq_handler SysTick_Handler
    def_irq_handler WWDG_IRQHandler
    def_irq_handler PVD_IRQHandler
    def_irq_handler TAMP_STAMP_IRQHandler
    def_irq_handler RTC_WKUP_IRQHandler
    def_irq_handler FLASH_IRQHandler
    def_irq_handler RCC_IRQHandler
    def_irq_handler EXTI0_IRQHandler
    def_irq_handler EXTI1_IRQHandler
    def_irq_handler EXTI2_IRQHandler
    def_irq_handler EXTI3_IRQHandler
    def_irq_handler EXTI4_IRQHandler
    def_irq_handler DMA1_Stream0_IRQHandler
    def_irq_handler DMA1_Stream1_IRQHandler
    def_irq_handler DMA1_Stream2_IRQHandler
    def_irq_handler DMA1_Stream3_IRQHandler
    def_irq_handler DMA1_Stream4_IRQHandler
    def_irq_handler DMA1_Stream5_IRQHandler
    def_irq_handler DMA1_Stream6_IRQHandler
    def_irq_handler ADC_IRQHandler
    def_irq_handler CAN1_TX_IRQHandler
    def_irq_handler CAN1_RX0_IRQHandler
    def_irq_handler CAN1_RX1_IRQHandler
    def_irq_handler CAN1_SCE_IRQHandler
    def_irq_handler EXTI9_5_IRQHandler
    def_irq_handler TIM1_BRK_TIM9_IRQHandler
    def_irq_handler TIM1_UP_TIM10_IRQHandler
    def_irq_handler TIM1_TRG_COM_TIM11_IRQHandler
    def_irq_handler TIM1_CC_IRQHandler
    def_irq_handler TIM2_IRQHandler
    def_irq_handler TIM3_IRQHandler
    def_irq_handler TIM4_IRQHandler
    def_irq_handler I2C1_EV_IRQHandler
    def_irq_handler I2C1_ER_IRQHandler
    def_irq_handler I2C2_EV_IRQHandler
    def_irq_handler I2C2_ER_IRQHandler
    def_irq_handler SPI1_IRQHandler
    def_irq_handler SPI2_IRQHandler
    def_irq_handler USART1_IRQHandler
    def_irq_handler USART2_IRQHandler
    def_irq_handler USART3_IRQHandler
    def_irq_handler EXTI15_10_IRQHandler
    def_irq_handler RTC_Alarm_IRQHandler
    def_irq_handler OTG_FS_WKUP_IRQHandler
    def_irq_handler DMA1_Stream7_IRQHandler
    def_irq_handler FSMC_IRQHandler
    def_irq_handler SDIO_IRQHandler
    def_irq_handler TIM5_IRQHandler
    def_irq_handler SPI3_IRQHandler
    def_irq_handler UART4_IRQHandler
    def_irq_handler UART5_IRQHandler
    def_irq_handler TIM6_DAC_IRQHandler
    def_irq_handler TIM7_IRQHandler
    def_irq_handler DMA2_Stream0_IRQHandler
    def_irq_handler DMA2_Stream1_IRQHandler
    def_irq_handler DMA2_Stream2_IRQHandler
    def_irq_handler DMA2_Stream3_IRQHandler
    def_irq_handler DMA2_Stream4_IRQHandler
    def_irq_handler ETH_IRQHandler
    def_irq_handler ETH_WKUP_IRQHandler
    def_irq_handler CAN2_TX_IRQHandler
    def_irq_handler CAN2_RX0_IRQHandler
    def_irq_handler CAN2_RX1_IRQHandler
    def_irq_handler CAN2_SCE_IRQHandler
    def_irq_handler OTG_FS_IRQHandler
    def_irq_handler OTG_HS_IRQHandler
    def_irq_handler DCMI_IRQHandler
    def_irq_handler FPU_IRQHandler
    def_irq_handler DMA2_Stream5_IRQHandler
    def_irq_handler DMA2_Stream6_IRQHandler
    def_irq_handler DMA2_Stream7_IRQHandler
    def_irq_handler USART6_IRQHandler
    def_irq_handler I2C3_EV_IRQHandler
    def_irq_handler I2C3_ER_IRQHandler
    def_irq_handler OTG_HS_EP1_OUT_IRQHandler
    def_irq_handler OTG_HS_EP1_IN_IRQHandler
    def_irq_handler OTG_HS_WKUP_IRQHandler
    def_irq_handler OTG_HS_IRQHandler
    def_irq_handler HASH_RNG_IRQHandler
