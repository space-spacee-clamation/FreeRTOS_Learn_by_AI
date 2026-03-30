/*
 * LM3S6965 启动文件
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
    Default_Handler,                  /* MPU故障 */
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
    Default_Handler,                  /* GPIO Port A */
    Default_Handler,                  /* GPIO Port B */
    Default_Handler,                  /* GPIO Port C */
    Default_Handler,                  /* GPIO Port D */
    Default_Handler,                  /* GPIO Port E */
    Default_Handler,                  /* UART0 */
    Default_Handler,                  /* UART1 */
    Default_Handler,                  /* SSI */
    Default_Handler,                  /* I2C */
    Default_Handler,                  /* PWM Fault */
    Default_Handler,                  /* PWM Gen 0 */
    Default_Handler,                  /* PWM Gen 1 */
    Default_Handler,                  /* PWM Gen 2 */
    Default_Handler,                  /* QEI */
    Default_Handler,                  /* ADC Seq 0 */
    Default_Handler,                  /* ADC Seq 1 */
    Default_Handler,                  /* ADC Seq 2 */
    Default_Handler,                  /* ADC Seq 3 */
    Default_Handler,                  /* Watchdog */
    Default_Handler,                  /* Timer 0A */
    Default_Handler,                  /* Timer 0B */
    Default_Handler,                  /* Timer 1A */
    Default_Handler,                  /* Timer 1B */
    Default_Handler,                  /* Timer 2A */
    Default_Handler,                  /* Timer 2B */
    Default_Handler,                  /* Comp 0 */
    Default_Handler,                  /* Comp 1 */
    Default_Handler,                  /* Comp 2 */
    Default_Handler,                  /* System Control */
    Default_Handler,                  /* Flash Control */
    Default_Handler,                  /* GPIO Port F */
    Default_Handler,                  /* GPIO Port G */
    Default_Handler,                  /* GPIO Port H */
    Default_Handler,                  /* UART2 */
    Default_Handler,                  /* SSI1 */
    Default_Handler,                  /* Timer 3A */
    Default_Handler,                  /* Timer 3B */
    Default_Handler,                  /* I2C1 */
    Default_Handler,                  /* QEI1 */
    Default_Handler,                  /* CAN0 */
    Default_Handler,                  /* CAN1 */
    0,                                /* 保留 */
    Default_Handler,                  /* Ethernet */
    Default_Handler                   /* Hibernate */
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

/*
 * _sbrk - 堆内存扩展函数（用于标准库malloc）
 */
extern uint32_t end;  // 链接脚本定义的BSS段结束地址
static uint32_t *heap_end = &end;

void *_sbrk(int incr)
{
    uint32_t *prev_heap_end;

    prev_heap_end = heap_end;
    heap_end += incr;

    return (void *)prev_heap_end;
}
