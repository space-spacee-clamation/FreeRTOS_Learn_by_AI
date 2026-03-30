/**
 * @file main.c
 * @brief FreeRTOS heap_2内存分配器示例
 *
 * 本项目演示FreeRTOS heap_2内存分配器的使用方法。
 * 学习要点：
 * 1. heap_2内存分配器的工作原理
 * 2. pvPortMalloc() 内存分配
 * 3. vPortFree() 内存释放
 * 4. 内存碎片问题演示
 * 5. heap_2与heap_1的区别
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
void *g_pvAllocatedBlock4 = NULL;

/* ==============================================
 *              辅助函数
 * ==============================================
 */

/**
 * @brief 打印当前内存状态
 *
 * 这个函数打印堆内存的使用情况。
 * 注意：heap_2不支持内存碎片统计，但可以通过分配和释放
 *      观察到碎片问题的存在。
 */
void vPrintMemoryStats(void)
{
    size_t xFreeSize = xPortGetFreeHeapSize();

    vPrintString("\r\n---------- 内存状态 ----------\r\n");
    vPrintString("总堆大小: ");
    vPrintNumber(configTOTAL_HEAP_SIZE);
    vPrintString(" 字节\r\n");

    vPrintString("剩余大小: ");
    vPrintNumber(xFreeSize);
    vPrintString(" 字节\r\n");

    vPrintString("已分配: ");
    vPrintNumber(configTOTAL_HEAP_SIZE - xFreeSize);
    vPrintString(" 字节\r\n");
    vPrintString("-------------------------------\r\n\r\n");
}

/* ==============================================
 *              任务函数定义
 * ==============================================
 */

/**
 * @brief 内存演示任务 - 演示heap_2的使用
 * @param pvParameters 任务参数
 */
void vMemoryDemoTask(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulStep = 0;

    vPrintString("[Demo] 内存演示任务启动！\r\n");

    vPrintString("\r\n[Demo] ========== 步骤1: 初始内存状态 ==========\r\n");
    vPrintMemoryStats();

    vTaskDelay(pdMS_TO_TICKS(1000));

    for(;;)
    {
        ulStep++;

        vPrintString("\r\n[Demo] ========== 步骤");
        vPrintNumber(ulStep + 1);
        vPrintString(" ==========\r\n");

        switch(ulStep)
        {
            case 1:
                vPrintString("[Demo] 分配块1: 100 字节...\r\n");
                g_pvAllocatedBlock1 = pvPortMalloc(100);

                if(g_pvAllocatedBlock1 != NULL)
                {
                    vPrintString("[Demo] ✓ 块1分配成功！地址: ");
                    vPrintNumber((uint32_t)g_pvAllocatedBlock1);
                    vPrintString("\r\n");
                    memset(g_pvAllocatedBlock1, 0xAA, 100);
                }
                break;

            case 2:
                vPrintString("[Demo] 分配块2: 200 字节...\r\n");
                g_pvAllocatedBlock2 = pvPortMalloc(200);

                if(g_pvAllocatedBlock2 != NULL)
                {
                    vPrintString("[Demo] ✓ 块2分配成功！地址: ");
                    vPrintNumber((uint32_t)g_pvAllocatedBlock2);
                    vPrintString("\r\n");
                    memset(g_pvAllocatedBlock2, 0xBB, 200);
                }
                break;

            case 3:
                vPrintString("[Demo] 分配块3: 150 字节...\r\n");
                g_pvAllocatedBlock3 = pvPortMalloc(150);

                if(g_pvAllocatedBlock3 != NULL)
                {
                    vPrintString("[Demo] ✓ 块3分配成功！地址: ");
                    vPrintNumber((uint32_t)g_pvAllocatedBlock3);
                    vPrintString("\r\n");
                    memset(g_pvAllocatedBlock3, 0xCC, 150);
                }
                break;

            case 4:
                vPrintString("[Demo] 释放块2（中间块）...\r\n");
                if(g_pvAllocatedBlock2 != NULL)
                {
                    vPortFree(g_pvAllocatedBlock2);
                    g_pvAllocatedBlock2 = NULL;
                    vPrintString("[Demo] ✓ 块2已释放！\r\n");
                    vPrintString("[Demo] 注意：现在堆中有碎片了！\r\n");
                }
                break;

            case 5:
                vPrintString("[Demo] 尝试分配块4: 300 字节...\r\n");
                vPrintString("[Demo] 提示：由于碎片，可能分配失败！\r\n");

                g_pvAllocatedBlock4 = pvPortMalloc(300);

                if(g_pvAllocatedBlock4 != NULL)
                {
                    vPrintString("[Demo] ✓ 块4分配成功！\r\n");
                    vPrintString("[Demo] 说明：还有足够的连续空间\r\n");
                    memset(g_pvAllocatedBlock4, 0xDD, 300);
                }
                else
                {
                    vPrintString("[Demo] ✗ 块4分配失败！\r\n");
                    vPrintString("[Demo] 原因：内存碎片，没有足够大的连续块！\r\n");
                }
                break;

            case 6:
                vPrintString("[Demo] 释放所有剩余块...\r\n");

                if(g_pvAllocatedBlock1 != NULL)
                {
                    vPortFree(g_pvAllocatedBlock1);
                    g_pvAllocatedBlock1 = NULL;
                }

                if(g_pvAllocatedBlock3 != NULL)
                {
                    vPortFree(g_pvAllocatedBlock3);
                    g_pvAllocatedBlock3 = NULL;
                }

                if(g_pvAllocatedBlock4 != NULL)
                {
                    vPortFree(g_pvAllocatedBlock4);
                    g_pvAllocatedBlock4 = NULL;
                }

                vPrintString("[Demo] ✓ 所有块已释放！\r\n");
                vPrintString("[Demo] 注意：虽然都释放了，但堆中有多个不连续的空闲块！\r\n");
                break;

            case 7:
                vPrintString("[Demo] 尝试分配小块: 80 字节（应该成功）...\r\n");
                g_pvAllocatedBlock1 = pvPortMalloc(80);

                if(g_pvAllocatedBlock1 != NULL)
                {
                    vPrintString("[Demo] ✓ 小块分配成功！\r\n");
                    vPrintString("[Demo] 说明：可以利用碎片空间分配小块\r\n");
                }
                break;

            default:
                vPrintString("[Demo] 演示完成！\r\n");
                break;
        }

        vPrintMemoryStats();
        vTaskDelay(pdMS_TO_TICKS(2000));

        if(ulStep >= 8)
        {
            vPrintString("[Demo] 演示任务结束，进入空闲循环\r\n");

            if(g_pvAllocatedBlock1 != NULL) { vPortFree(g_pvAllocatedBlock1); g_pvAllocatedBlock1 = NULL; }
            if(g_pvAllocatedBlock2 != NULL) { vPortFree(g_pvAllocatedBlock2); g_pvAllocatedBlock2 = NULL; }
            if(g_pvAllocatedBlock3 != NULL) { vPortFree(g_pvAllocatedBlock3); g_pvAllocatedBlock3 = NULL; }
            if(g_pvAllocatedBlock4 != NULL) { vPortFree(g_pvAllocatedBlock4); g_pvAllocatedBlock4 = NULL; }

            for(;;)
            {
                vTaskDelay(pdMS_TO_TICKS(10000));
            }
        }
    }
}

/**
 * @brief 监控任务 - 定期打印内存状态
 * @param pvParameters 任务参数
 */
void vMonitorTask(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulTickCount = 0;

    vPrintString("[Monitor] 监控任务启动！\r\n");

    for(;;)
    {
        ulTickCount++;

        vPrintString("\r\n[Monitor] ========== 监控周期 #");
        vPrintNumber(ulTickCount);
        vPrintString(" ==========\r\n");

        vPrintMemoryStats();
        vTaskDelay(pdMS_TO_TICKS(3000));
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
    vPrintString("  FreeRTOS heap_2 内存分配器示例\r\n");
    vPrintString("====================================\r\n");
    vPrintString("\r\n");

    vPrintString("学习要点：\r\n");
    vPrintString("1. heap_2 内存分配器工作原理\r\n");
    vPrintString("2. pvPortMalloc() 分配内存\r\n");
    vPrintString("3. vPortFree() 释放内存\r\n");
    vPrintString("4. 内存碎片问题演示\r\n");
    vPrintString("5. heap_2 与 heap_1 的区别\r\n");
    vPrintString("\r\n");

    vPrintString("====================================\r\n");
    vPrintString("  heap_2 的特点：\r\n");
    vPrintString("====================================\r\n");
    vPrintString("✓ 支持内存分配和释放\r\n");
    vPrintString("✓ 使用最佳适配算法\r\n");
    vPrintString("✓ 比heap_1更灵活\r\n");
    vPrintString("\r\n");
    vPrintString("✗ 会产生内存碎片\r\n");
    vPrintString("✗ 不合并相邻空闲块\r\n");
    vPrintString("✗ 没有最小历史空闲统计\r\n");
    vPrintString("====================================\r\n");
    vPrintString("\r\n");

    vPrintString("====================================\r\n");
    vPrintString("  heap_2 与 heap_1 对比：\r\n");
    vPrintString("====================================\r\n");
    vPrintString("特性          heap_1      heap_2\r\n");
    vPrintString("------------------------------------\r\n");
    vPrintString("内存释放      ✗ 不支持    ✓ 支持\r\n");
    vPrintString("内存碎片      ✗ 无        ✗ 有\r\n");
    vPrintString("块合并        -           ✗ 不合并\r\n");
    vPrintString("分配算法      线性分配    最佳适配\r\n");
    vPrintString("适用场景      静态分配    简单动态\r\n");
    vPrintString("====================================\r\n");
    vPrintString("\r\n");

    vPrintString("正在创建任务...\r\n");

    xTaskCreate(vMemoryDemoTask, "DemoTask", 256, NULL, 2, NULL);
    xTaskCreate(vMonitorTask, "Monitor", 256, NULL, 1, NULL);

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
