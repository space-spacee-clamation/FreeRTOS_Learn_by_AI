/*
 * STM32F103 启动文件
 * 简化版本用于QEMU仿真
 */

#include <stdint.h>

/* 函数声明 */
void Reset_Handler(void);
static void Default_Handler(void);
void xPortPendSVHandler(void);
void xPortSysTickHandler(void);
void vPortSVCHandler(void);

/* 主函数声明 */
extern int main(void);

/* 链接脚本定义的符号 */
extern uint32_t _etext;
extern uint32_t _sdata;
extern uint32_t _data;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _bss;
extern uint32_t _ebss;
extern uint32_t _estack;

/* 中断向量表 */
__attribute__((section(".isr_vector")))
void (*const g_pfnVectors[])(void) =
{
    (void (*)(void))(&_estack),     /* 初始栈指针 */
    Reset_Handler,                    /* 复位处理 */
    Default_Handler,                  /* NMI */
    Default_Handler,                  /* 硬故障 */
    Default_Handler,                  /* 内存管理 */
    Default_Handler,                  /* 总线故障 */
    Default_Handler,                  /* 使用故障 */
    0,                                /* 保留 */
    0,                                /* 保留 */
    0,                                /* 保留 */
    0,                                /* 保留 */
    vPortSVCHandler,                  /* SVC调用 */
    Default_Handler,                  /* 调试监控 */
    0,                                /* 保留 */
    xPortPendSVHandler,               /* PendSV */
    xPortSysTickHandler,              /* SysTick */
    Default_Handler,                  /* WWDG */
    Default_Handler,                  /* PVD */
    Default_Handler,                  /* TAMPER */
    Default_Handler,                  /* RTC */
    Default_Handler,                  /* FLASH */
    Default_Handler,                  /* RCC */
    Default_Handler,                  /* EXTI0 */
    Default_Handler,                  /* EXTI1 */
    Default_Handler,                  /* EXTI2 */
    Default_Handler,                  /* EXTI3 */
    Default_Handler,                  /* EXTI4 */
    Default_Handler,                  /* DMA1_Channel1 */
    Default_Handler,                  /* DMA1_Channel2 */
    Default_Handler,                  /* DMA1_Channel3 */
    Default_Handler,                  /* DMA1_Channel4 */
    Default_Handler,                  /* DMA1_Channel5 */
    Default_Handler,                  /* DMA1_Channel6 */
    Default_Handler,                  /* DMA1_Channel7 */
    Default_Handler,                  /* ADC1_2 */
    Default_Handler,                  /* USB_HP_CAN1_TX */
    Default_Handler,                  /* USB_LP_CAN1_RX0 */
    Default_Handler,                  /* CAN1_RX1 */
    Default_Handler,                  /* CAN1_SCE */
    Default_Handler,                  /* EXTI9_5 */
    Default_Handler,                  /* TIM1_BRK */
    Default_Handler,                  /* TIM1_UP */
    Default_Handler,                  /* TIM1_TRG_COM */
    Default_Handler,                  /* TIM1_CC */
    Default_Handler,                  /* TIM2 */
    Default_Handler,                  /* TIM3 */
    Default_Handler,                  /* TIM4 */
    Default_Handler,                  /* I2C1_EV */
    Default_Handler,                  /* I2C1_ER */
    Default_Handler,                  /* I2C2_EV */
    Default_Handler,                  /* I2C2_ER */
    Default_Handler,                  /* SPI1 */
    Default_Handler,                  /* SPI2 */
    Default_Handler,                  /* USART1 */
    Default_Handler,                  /* USART2 */
    Default_Handler,                  /* USART3 */
    Default_Handler,                  /* EXTI15_10 */
    Default_Handler,                  /* RTCAlarm */
    Default_Handler,                  /* USBWakeUp */
    0,                                /* 保留 */
    0,                                /* 保留 */
    0,                                /* 保留 */
    0,                                /* 保留 */
    0,                                /* 保留 */
    0,                                /* 保留 */
    0,                                /* 保留 */
    Default_Handler                   /* 末尾 */
};

/*
 * 复位处理函数
 */
void Reset_Handler(void)
{
    uint32_t *pulSrc, *pulDest;

    /* 复制数据段从Flash到RAM */
    pulSrc = &_etext;
    for (pulDest = &_data; pulDest < &_edata; ) {
        *pulDest++ = *pulSrc++;
    }

    /* 清零BSS段 */
    for (pulDest = &_bss; pulDest < &_ebss; ) {
        *pulDest++ = 0;
    }

    /* 调用主函数 */
    main();

    /* 如果main返回，进入无限循环 */
    while (1);
}

/*
 * 默认中断处理函数
 */
static void Default_Handler(void)
{
    while (1);
}
