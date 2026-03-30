/**
 * @file main.c
 * @brief FreeRTOS任务调度示例
 *
 * 本项目演示FreeRTOS的多任务调度和优先级管理。
 * 学习要点：
 * 1. 创建多个任务
 * 2. 任务优先级的作用
 * 3. 抢占式调度的工作原理
 * 4. 时间片轮转（相同优先级任务）
 *
 * 参考资料：
 * - FreeRTOS官方文档: https://www.freertos.org/RTOS-task-priority.html
 * - Mastering-the-FreeRTOS-Kernel 第4章
 */

#include <stdint.h>
#include <stdbool.h>

/* FreeRTOS 头文件 */
#include "FreeRTOS.h"
#include "task.h"

/* 平台相关的串口输出函数声明 */
void vUARTInit(void);
void vPrintString(const char *pcString);
void vPrintNumber(uint32_t ulNumber);

/* ==============================================
 *              任务函数定义
 * ==============================================
 *
 * 本项目创建3个任务，演示不同优先级的调度效果：
 * - HighPriorityTask: 高优先级 (3)
 * - MediumPriorityTask: 中优先级 (2)
 * - LowPriorityTask: 低优先级 (1)
 */

/**
 * @brief 高优先级任务
 * @param pvParameters 任务参数
 *
 * 优先级：3（最高）
 * 特点：运行时间短，频率高
 */
void vHighPriorityTask(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulCounter = 0;

    vPrintString("[High] 高优先级任务启动！\r\n");

    for(;;)
    {
        vPrintString("[High] 运行 #");
        vPrintNumber(ulCounter);
        vPrintString("\r\n");

        ulCounter++;

        /* 高优先级任务短时间运行后阻塞 */
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

/**
 * @brief 中优先级任务
 * @param pvParameters 任务参数
 *
 * 优先级：2（中等）
 * 特点：运行时间中等
 */
void vMediumPriorityTask(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulCounter = 0;

    vPrintString("[Medium] 中优先级任务启动！\r\n");

    for(;;)
    {
        vPrintString("[Medium] 运行 #");
        vPrintNumber(ulCounter);
        vPrintString("\r\n");

        ulCounter++;

        /* 中优先级任务阻塞时间中等 */
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}

/**
 * @brief 低优先级任务
 * @param pvParameters 任务参数
 *
 * 优先级：1（最低）
 * 特点：作为后台任务
 */
void vLowPriorityTask(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulCounter = 0;

    vPrintString("[Low] 低优先级任务启动！\r\n");

    for(;;)
    {
        vPrintString("[Low] 运行 #");
        vPrintNumber(ulCounter);
        vPrintString("\r\n");

        ulCounter++;

        /* 低优先级任务阻塞时间较长 */
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/**
 * @brief 主函数
 * @return 永远不会返回（如果调度器启动成功）
 */
int main(void)
{
    /* 初始化串口 */
    vUARTInit();

    /* 输出欢迎信息 */
    vPrintString("\r\n");
    vPrintString("====================================\r\n");
    vPrintString("  FreeRTOS任务调度示例\r\n");
    vPrintString("====================================\r\n");
    vPrintString("\r\n");

    vPrintString("学习要点：\r\n");
    vPrintString("1. 创建多个不同优先级的任务\r\n");
    vPrintString("2. 观察抢占式调度效果\r\n");
    vPrintString("3. 理解任务优先级的作用\r\n");
    vPrintString("\r\n");

    vPrintString("任务配置：\r\n");
    vPrintString("- HighTask:   优先级 3, 周期 200ms\r\n");
    vPrintString("- MediumTask: 优先级 2, 周期 300ms\r\n");
    vPrintString("- LowTask:    优先级 1, 周期 500ms\r\n");
    vPrintString("\r\n");

    /* ==============================================
     *       创建多个任务 - 演示优先级
     * ==============================================
     *
     * 注意优先级的数值：
     * - 数值越大，优先级越高
     * - 高优先级任务可以抢占低优先级任务
     * - 相同优先级任务之间使用时间片轮转
     */

    vPrintString("正在创建任务...\r\n");

    /* 创建低优先级任务 */
    BaseType_t xResult1 = xTaskCreate(
        vLowPriorityTask,
        "LowTask",
        128,
        NULL,
        1,      /* 优先级 1 */
        NULL
    );

    /* 创建中优先级任务 */
    BaseType_t xResult2 = xTaskCreate(
        vMediumPriorityTask,
        "MediumTask",
        128,
        NULL,
        2,      /* 优先级 2 */
        NULL
    );

    /* 创建高优先级任务 */
    BaseType_t xResult3 = xTaskCreate(
        vHighPriorityTask,
        "HighTask",
        128,
        NULL,
        3,      /* 优先级 3 */
        NULL
    );

    /* 检查任务是否创建成功 */
    if((xResult1 == pdPASS) && (xResult2 == pdPASS) && (xResult3 == pdPASS))
    {
        vPrintString("所有任务创建成功！\r\n");
    }
    else
    {
        vPrintString("错误：任务创建失败！\r\n");
        for(;;);
    }

    /* ==============================================
     *      启动调度器
     * ==============================================
     *
     * 观察输出，你会看到：
     * 1. 高优先级任务最先运行
     * 2. 高优先级任务运行频率最高
     * 3. 低优先级任务只有在高优先级任务阻塞时才能运行
     */

    vPrintString("启动FreeRTOS调度器...\r\n");
    vPrintString("\r\n");

    vTaskStartScheduler();

    /* 只有在调度器启动失败时才会执行到这里 */
    vPrintString("错误：调度器启动失败！\r\n");
    for(;;);

    return 0;
}

/* ==============================================
 *           平台相关的串口输出函数
 * ==============================================
 */

/* 根据平台选择合适的实现 */
#if defined(PLATFORM_LM3S6965)

/* LM3S6965 串口实现 */
#define UART0_DR (*((volatile unsigned long *)0x4000C000))
#define UART0_FR (*((volatile unsigned long *)0x4000C018))
#define UART_FR_TXFF (1 << 5) /* 发送FIFO满 */

void vUARTInit(void)
{
    /* LM3S6965 UART0 已经由QEMU初始化 */
}

void vPrintChar(char c)
{
    while(UART0_FR & UART_FR_TXFF);
    UART0_DR = c;
}

#elif defined(PLATFORM_STM32F103)

/* STM32F103 串口实现 */
#define USART1_DR (*((volatile unsigned long *)0x40013804))
#define USART1_SR (*((volatile unsigned long *)0x40013800))
#define USART_SR_TXE (1 << 7)

void vUARTInit(void)
{
    /* STM32F103 USART1 不需要特殊初始化 */
}

void vPrintChar(char c)
{
    while(!(USART1_SR & USART_SR_TXE));
    USART1_DR = c;
}

#else
#error "未定义的平台！请定义 PLATFORM_LM3S6965 或 PLATFORM_STM32F103"
#endif

/* 通用的字符串输出函数 */
void vPrintString(const char *pcString)
{
    while(*pcString != '\0')
    {
        vPrintChar(*pcString);
        pcString++;
    }
}

/* 通用的数字输出函数（十进制） */
void vPrintNumber(uint32_t ulNumber)
{
    char cBuffer[16];
    int iIndex = 0;

    if(ulNumber == 0)
    {
        vPrintChar('0');
        return;
    }

    while(ulNumber > 0 && iIndex < 15)
    {
        cBuffer[iIndex] = '0' + (ulNumber % 10);
        ulNumber = ulNumber / 10;
        iIndex++;
    }

    for(iIndex--; iIndex >= 0; iIndex--)
    {
        vPrintChar(cBuffer[iIndex]);
    }
}

/* ==============================================
 *           FreeRTOS 钩子函数
 * ==============================================
 */

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    vPrintString("\r\n!!! 栈溢出检测：");
    vPrintString(pcTaskName);
    vPrintString("\r\n");
    for(;;);
}

void vApplicationMallocFailedHook(void)
{
    vPrintString("\r\n!!! 内存分配失败！\r\n");
    for(;;);
}
