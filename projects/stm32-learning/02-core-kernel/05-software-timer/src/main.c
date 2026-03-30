/**
 * @file main.c
 * @brief FreeRTOS 软件定时器示例
 *
 * 本项目演示FreeRTOS软件定时器的使用方法。
 * 学习要点：
 * 1. 软件定时器基础概念
 * 2. 单次定时器（One-shot Timer）
 * 3. 周期定时器（Periodic Timer）
 * 4. 定时器回调函数
 * 5. 定时器启动、停止、复位操作
 * 6. 定时器服务任务
 *
 * 参考资料：
 * - FreeRTOS官方文档: https://www.freertos.org/RTOS-software-timer.html
 * - Mastering-the-FreeRTOS-Kernel 相关章节
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* FreeRTOS 头文件 */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

/* 平台相关的串口输出函数声明 */
void vUARTInit(void);
void vPrintString(const char *pcString);
void vPrintNumber(uint32_t ulNumber);

/* ==============================================
 *              全局变量
 * ==============================================
 */

/* 定时器句柄 */
TimerHandle_t g_xOneShotTimer = NULL;
TimerHandle_t g_xPeriodicTimer1 = NULL;
TimerHandle_t g_xPeriodicTimer2 = NULL;

/* 定时器回调计数 */
uint32_t g_ulOneShotCount = 0;
uint32_t g_ulPeriodic1Count = 0;
uint32_t g_ulPeriodic2Count = 0;

/* ==============================================
 *              定时器回调函数
 * ==============================================
 */

/**
 * @brief 单次定时器回调函数
 * @param xTimer 定时器句柄
 */
void vOneShotTimerCallback(TimerHandle_t xTimer)
{
    (void)xTimer;
    g_ulOneShotCount++;
    
    vPrintString("[Timer] ✓ 单次定时器回调！计数: ");
    vPrintNumber(g_ulOneShotCount);
    vPrintString("\r\n");
    vPrintString("[Timer] 注意：单次定时器只会触发一次！\r\n");
}

/**
 * @brief 周期定时器1回调函数（1秒周期）
 * @param xTimer 定时器句柄
 */
void vPeriodicTimer1Callback(TimerHandle_t xTimer)
{
    (void)xTimer;
    g_ulPeriodic1Count++;
    
    vPrintString("[Timer1] 周期定时器1回调（1秒）！计数: ");
    vPrintNumber(g_ulPeriodic1Count);
    vPrintString("\r\n");
}

/**
 * @brief 周期定时器2回调函数（2秒周期）
 * @param xTimer 定时器句柄
 */
void vPeriodicTimer2Callback(TimerHandle_t xTimer)
{
    (void)xTimer;
    g_ulPeriodic2Count++;
    
    vPrintString("[Timer2] 周期定时器2回调（2秒）！计数: ");
    vPrintNumber(g_ulPeriodic2Count);
    vPrintString("\r\n");
}

/* ==============================================
 *              任务函数定义
 * ==============================================
 */

/**
 * @brief 演示任务 - 控制定时器操作
 * @param pvParameters 任务参数
 */
void vDemoTask(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulStep = 0;
    BaseType_t xResult;

    vPrintString("[Demo] 演示任务启动！\r\n");
    vTaskDelay(pdMS_TO_TICKS(1000));

    for(;;)
    {
        ulStep++;

        vPrintString("\r\n[Demo] ========== 步骤");
        vPrintNumber(ulStep);
        vPrintString(" ==========\r\n");

        switch(ulStep)
        {
            case 1:
                vPrintString("[Demo] 启动周期定时器1（1秒周期）...\r\n");
                xResult = xTimerStart(g_xPeriodicTimer1, 0);
                if(xResult == pdPASS)
                {
                    vPrintString("[Demo] ✓ 周期定时器1启动成功！\r\n");
                }
                break;

            case 2:
                vPrintString("[Demo] 启动周期定时器2（2秒周期）...\r\n");
                xResult = xTimerStart(g_xPeriodicTimer2, 0);
                if(xResult == pdPASS)
                {
                    vPrintString("[Demo] ✓ 周期定时器2启动成功！\r\n");
                }
                break;

            case 3:
                vPrintString("[Demo] 启动单次定时器（3秒后触发）...\r\n");
                xResult = xTimerStart(g_xOneShotTimer, 0);
                if(xResult == pdPASS)
                {
                    vPrintString("[Demo] ✓ 单次定时器启动成功！\r\n");
                }
                break;

            case 4:
                vPrintString("[Demo] 停止周期定时器2...\r\n");
                xResult = xTimerStop(g_xPeriodicTimer2, 0);
                if(xResult == pdPASS)
                {
                    vPrintString("[Demo] ✓ 周期定时器2已停止！\r\n");
                }
                break;

            case 5:
                vPrintString("[Demo] 复位周期定时器1...\r\n");
                xResult = xTimerReset(g_xPeriodicTimer1, 0);
                if(xResult == pdPASS)
                {
                    vPrintString("[Demo] ✓ 周期定时器1已复位！\r\n");
                }
                break;

            case 6:
                vPrintString("[Demo] 再次启动周期定时器2...\r\n");
                xResult = xTimerStart(g_xPeriodicTimer2, 0);
                if(xResult == pdPASS)
                {
                    vPrintString("[Demo] ✓ 周期定时器2重新启动成功！\r\n");
                }
                break;

            default:
                vPrintString("[Demo] 演示完成！观察定时器继续运行...\r\n");
                break;
        }

        vPrintString("\r\n[Demo] 当前统计：\r\n");
        vPrintString("[Demo]   单次定时器: ");
        vPrintNumber(g_ulOneShotCount);
        vPrintString(" 次\r\n");
        vPrintString("[Demo]   周期定时器1: ");
        vPrintNumber(g_ulPeriodic1Count);
        vPrintString(" 次\r\n");
        vPrintString("[Demo]   周期定时器2: ");
        vPrintNumber(g_ulPeriodic2Count);
        vPrintString(" 次\r\n");

        vTaskDelay(pdMS_TO_TICKS(3000));

        if(ulStep >= 8)
        {
            vPrintString("[Demo] 进入空闲循环，定时器继续运行...\r\n");
            for(;;)
            {
                vTaskDelay(pdMS_TO_TICKS(10000));
            }
        }
    }
}

/* ==============================================
 *              主函数
 * ==============================================
 */

int main(void)
{
    vUARTInit();

    vPrintString("\r\n");
    vPrintString("====================================\r\n");
    vPrintString("  FreeRTOS 软件定时器示例\r\n");
    vPrintString("====================================\r\n");
    vPrintString("\r\n");

    vPrintString("学习要点：\r\n");
    vPrintString("1. 软件定时器基础概念\r\n");
    vPrintString("2. 单次定时器（One-shot）\r\n");
    vPrintString("3. 周期定时器（Periodic）\r\n");
    vPrintString("4. 定时器回调函数\r\n");
    vPrintString("5. 定时器启动、停止、复位\r\n");
    vPrintString("\r\n");

    vPrintString("====================================\r\n");
    vPrintString("  软件定时器特点：\r\n");
    vPrintString("====================================\r\n");
    vPrintString("✓ 由定时器服务任务管理\r\n");
    vPrintString("✓ 支持单次和周期模式\r\n");
    vPrintString("✓ 回调函数在定时器任务中执行\r\n");
    vPrintString("✓ 可以动态启动、停止、复位\r\n");
    vPrintString("\r\n");
    vPrintString("✗ 回调函数不能阻塞\r\n");
    vPrintString("✗ 精度依赖系统滴答\r\n");
    vPrintString("====================================\r\n");
    vPrintString("\r\n");

    /* 创建定时器 */
    vPrintString("正在创建定时器...\r\n");

    /* 创建单次定时器：3秒周期，单次模式 */
    g_xOneShotTimer = xTimerCreate(
        "OneShotTimer",           /* 定时器名称 */
        pdMS_TO_TICKS(3000),      /* 周期：3秒 */
        pdFALSE,                  /* 单次模式（非自动重载） */
        (void *)0,                /* 定时器ID */
        vOneShotTimerCallback     /* 回调函数 */
    );

    /* 创建周期定时器1：1秒周期，周期模式 */
    g_xPeriodicTimer1 = xTimerCreate(
        "PeriodicTimer1",         /* 定时器名称 */
        pdMS_TO_TICKS(1000),      /* 周期：1秒 */
        pdTRUE,                   /* 周期模式（自动重载） */
        (void *)1,                /* 定时器ID */
        vPeriodicTimer1Callback   /* 回调函数 */
    );

    /* 创建周期定时器2：2秒周期，周期模式 */
    g_xPeriodicTimer2 = xTimerCreate(
        "PeriodicTimer2",         /* 定时器名称 */
        pdMS_TO_TICKS(2000),      /* 周期：2秒 */
        pdTRUE,                   /* 周期模式（自动重载） */
        (void *)2,                /* 定时器ID */
        vPeriodicTimer2Callback   /* 回调函数 */
    );

    if(g_xOneShotTimer == NULL || g_xPeriodicTimer1 == NULL || g_xPeriodicTimer2 == NULL)
    {
        vPrintString("错误：定时器创建失败！\r\n");
        for(;;);
    }

    vPrintString("✓ 所有定时器创建成功！\r\n");
    vPrintString("\r\n");

    vPrintString("正在创建演示任务...\r\n");
    xTaskCreate(vDemoTask, "DemoTask", 256, NULL, 2, NULL);
    vPrintString("✓ 演示任务创建成功！\r\n");
    vPrintString("\r\n");

    vPrintString("启动FreeRTOS调度器...\r\n");
    vPrintString("\r\n");

    vTaskStartScheduler();

    vPrintString("错误：调度器启动失败！\r\n");
    for(;;);

    return 0;
}

/* ==============================================
 *           平台相关的串口输出函数
 * ==============================================
 */

#if defined(PLATFORM_LM3S6965)

#define UART0_DR (*((volatile unsigned long *)0x4000C000))
#define UART0_FR (*((volatile unsigned long *)0x4000C018))
#define UART_FR_TXFF (1 << 5)

void vUARTInit(void)
{
}

void vPrintChar(char c)
{
    while(UART0_FR & UART_FR_TXFF);
    UART0_DR = c;
}

#elif defined(PLATFORM_STM32F103)

#define USART1_DR (*((volatile unsigned long *)0x40013804))
#define USART1_SR (*((volatile unsigned long *)0x40013800))
#define USART_SR_TXE (1 << 7)

void vUARTInit(void)
{
}

void vPrintChar(char c)
{
    while(!(USART1_SR & USART_SR_TXE));
    USART1_DR = c;
}

#else
#error "未定义的平台！请定义 PLATFORM_LM3S6965 或 PLATFORM_STM32F103"
#endif

void vPrintString(const char *pcString)
{
    while(*pcString != '\0')
    {
        vPrintChar(*pcString);
        pcString++;
    }
}

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
