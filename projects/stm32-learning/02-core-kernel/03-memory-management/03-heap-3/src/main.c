/**
 * @file main.c
 * @brief FreeRTOS heap_3内存分配器示例
 *
 * 本项目演示FreeRTOS heap_3内存分配器的使用方法。
 * 学习要点：
 * 1. heap_3内存分配器的工作原理
 * 2. pvPortMalloc() 内存分配
 * 3. vPortFree() 内存释放
 * 4. 线程安全实现
 * 5. heap_3的特点和限制
 *
 * 参考资料：
 * - FreeRTOS官方文档: https://www.freertos.org/a00111.html
 * - Mastering-the-FreeRTOS-Kernel 第3章
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* FreeRTOS 头文件 */
#include "FreeRTOS.h"
#include "task.h"

/* 平台相关的串口输出函数声明 */
void vUARTInit(void);
void vPrintString(const char *pcString);
void vPrintNumber(uint32_t ulNumber);

/* ==============================================
 *              全局变量
 * ==============================================
 */

/* 用于存储分配的内存块指针 */
void *g_pvAllocatedBlock1 = NULL;
void *g_pvAllocatedBlock2 = NULL;
void *g_pvAllocatedBlock3 = NULL;

/* 任务同步标志 */
volatile bool g_bTask1Done = false;
volatile bool g_bTask2Done = false;

/* ==============================================
 *              辅助函数
 * ==============================================
 */

/**
 * @brief 打印heap_3的特点介绍
 */
void vPrintHeap3Features(void)
{
    vPrintString("\r\n");
    vPrintString("====================================\r\n");
    vPrintString("  heap_3 的特点：\r\n");
    vPrintString("====================================\r\n");
    vPrintString("✓ 标准库 malloc/free 包装器\r\n");
    vPrintString("✓ 线程安全（挂起调度器）\r\n");
    vPrintString("✓ 实现最简单\r\n");
    vPrintString("\r\n");
    vPrintString("✗ 执行时间不确定\r\n");
    vPrintString("✗ 不提供内存统计\r\n");
    vPrintString("✗ 依赖链接器配置堆\r\n");
    vPrintString("✗ 效率相对较低\r\n");
    vPrintString("====================================\r\n");
    vPrintString("\r\n");
}

/**
 * @brief 打印heap_3与其他分配器的对比
 */
void vPrintComparison(void)
{
    vPrintString("====================================\r\n");
    vPrintString("  heap_3 与 heap_1/2 对比：\r\n");
    vPrintString("====================================\r\n");
    vPrintString("特性          heap_1      heap_2      heap_3\r\n");
    vPrintString("------------------------------------------------\r\n");
    vPrintString("内存释放      ✗ 不支持    ✓ 支持      ✓ 支持\r\n");
    vPrintString("内存统计      ✓ 支持      ✓ 支持      ✗ 不支持\r\n");
    vPrintString("执行时间      ✓ 确定      ✓ 相对确定  ✗ 不确定\r\n");
    vPrintString("实现方式      自实现      自实现      标准库\r\n");
    vPrintString("线程安全      -           -           ✓ 是\r\n");
    vPrintString("====================================\r\n");
    vPrintString("\r\n");
}

/* ==============================================
 *              任务函数定义
 * ==============================================
 */

/**
 * @brief 任务1 - 分配内存
 * @param pvParameters 任务参数
 */
void vTask1(void *pvParameters)
{
    (void)pvParameters;

    vPrintString("[Task1] 任务1启动！\r\n");
    vPrintString("[Task1] 正在分配 150 字节...\r\n");

    g_pvAllocatedBlock1 = pvPortMalloc(150);

    if(g_pvAllocatedBlock1 != NULL)
    {
        vPrintString("[Task1] ✓ 分配成功！地址: ");
        vPrintNumber((uint32_t)g_pvAllocatedBlock1);
        vPrintString("\r\n");
        memset(g_pvAllocatedBlock1, 0xAA, 150);
    }
    else
    {
        vPrintString("[Task1] ✗ 分配失败！\r\n");
    }

    g_bTask1Done = true;

    vPrintString("[Task1] 任务结束，进入空闲循环\r\n");
    for(;;)
    {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

/**
 * @brief 任务2 - 分配更多内存
 * @param pvParameters 任务参数
 */
void vTask2(void *pvParameters)
{
    (void)pvParameters;

    vPrintString("[Task2] 任务2启动！\r\n");
    vPrintString("[Task2] 正在分配 200 字节...\r\n");

    g_pvAllocatedBlock2 = pvPortMalloc(200);

    if(g_pvAllocatedBlock2 != NULL)
    {
        vPrintString("[Task2] ✓ 分配成功！地址: ");
        vPrintNumber((uint32_t)g_pvAllocatedBlock2);
        vPrintString("\r\n");
        memset(g_pvAllocatedBlock2, 0xBB, 200);
    }
    else
    {
        vPrintString("[Task2] ✗ 分配失败！\r\n");
    }

    g_bTask2Done = true;

    vPrintString("[Task2] 任务结束，进入空闲循环\r\n");
    for(;;)
    {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

/**
 * @brief 内存演示任务 - 演示heap_3的使用
 * @param pvParameters 任务参数
 */
void vMemoryDemoTask(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulStep = 0;

    vPrintString("[Demo] 内存演示任务启动！\r\n");

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
                vPrintString("[Demo] 等待任务1和任务2完成...\r\n");
                while(!g_bTask1Done || !g_bTask2Done)
                {
                    vTaskDelay(pdMS_TO_TICKS(100));
                }
                vPrintString("[Demo] ✓ 所有任务分配完成！\r\n");
                break;

            case 2:
                vPrintString("[Demo] 现在分配块3: 100 字节...\r\n");
                g_pvAllocatedBlock3 = pvPortMalloc(100);

                if(g_pvAllocatedBlock3 != NULL)
                {
                    vPrintString("[Demo] ✓ 块3分配成功！地址: ");
                    vPrintNumber((uint32_t)g_pvAllocatedBlock3);
                    vPrintString("\r\n");
                    memset(g_pvAllocatedBlock3, 0xCC, 100);
                }
                break;

            case 3:
                vPrintString("[Demo] 释放块1...\r\n");
                if(g_pvAllocatedBlock1 != NULL)
                {
                    vPortFree(g_pvAllocatedBlock1);
                    g_pvAllocatedBlock1 = NULL;
                    vPrintString("[Demo] ✓ 块1已释放！\r\n");
                }
                break;

            case 4:
                vPrintString("[Demo] 重新分配块1: 80 字节...\r\n");
                g_pvAllocatedBlock1 = pvPortMalloc(80);

                if(g_pvAllocatedBlock1 != NULL)
                {
                    vPrintString("[Demo] ✓ 块1重新分配成功！地址: ");
                    vPrintNumber((uint32_t)g_pvAllocatedBlock1);
                    vPrintString("\r\n");
                    memset(g_pvAllocatedBlock1, 0xDD, 80);
                }
                break;

            case 5:
                vPrintString("[Demo] 释放所有内存块...\r\n");

                if(g_pvAllocatedBlock1 != NULL)
                {
                    vPortFree(g_pvAllocatedBlock1);
                    g_pvAllocatedBlock1 = NULL;
                }

                if(g_pvAllocatedBlock2 != NULL)
                {
                    vPortFree(g_pvAllocatedBlock2);
                    g_pvAllocatedBlock2 = NULL;
                }

                if(g_pvAllocatedBlock3 != NULL)
                {
                    vPortFree(g_pvAllocatedBlock3);
                    g_pvAllocatedBlock3 = NULL;
                }

                vPrintString("[Demo] ✓ 所有内存已释放！\r\n");
                break;

            default:
                vPrintString("[Demo] 演示完成！\r\n");
                vPrintString("\r\n[Demo] heap_3学习要点总结：\r\n");
                vPrintString("[Demo] 1. heap_3是标准库malloc/free的简单包装\r\n");
                vPrintString("[Demo] 2. 通过vTaskSuspendAll()保证线程安全\r\n");
                vPrintString("[Demo] 3. 不提供内存统计功能\r\n");
                vPrintString("[Demo] 4. 执行时间取决于标准库实现\r\n");
                vPrintString("[Demo] 5. 需要链接器配置堆区域\r\n");
                break;
        }

        vTaskDelay(pdMS_TO_TICKS(2000));

        if(ulStep >= 6)
        {
            vPrintString("[Demo] 演示任务结束，进入空闲循环\r\n");
            for(;;)
            {
                vTaskDelay(pdMS_TO_TICKS(10000));
            }
        }
    }
}

/**
 * @brief 主函数
 * @return 永远不会返回（如果调度器启动成功）
 */
int main(void)
{
    vUARTInit();

    vPrintString("\r\n");
    vPrintString("====================================\r\n");
    vPrintString("  FreeRTOS heap_3 内存分配器示例\r\n");
    vPrintString("====================================\r\n");
    vPrintString("\r\n");

    vPrintString("学习要点：\r\n");
    vPrintString("1. heap_3 内存分配器工作原理\r\n");
    vPrintString("2. pvPortMalloc() 分配内存\r\n");
    vPrintString("3. vPortFree() 释放内存\r\n");
    vPrintString("4. 线程安全实现\r\n");
    vPrintString("5. heap_3 的特点和限制\r\n");
    vPrintString("\r\n");

    vPrintHeap3Features();
    vPrintComparison();

    vPrintString("正在创建任务...\r\n");

    xTaskCreate(vTask1, "Task1", 256, NULL, 2, NULL);
    xTaskCreate(vTask2, "Task2", 256, NULL, 2, NULL);
    xTaskCreate(vMemoryDemoTask, "DemoTask", 256, NULL, 1, NULL);

    vPrintString("所有任务创建成功！\r\n");
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
