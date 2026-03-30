/**
 * @file main.c
 * @brief FreeRTOS heap_4内存分配器示例
 *
 * 本项目演示FreeRTOS heap_4内存分配器的使用方法。
 * 学习要点：
 * 1. heap_4内存分配器的工作原理
 * 2. 首次适配算法（First Fit）
 * 3. 相邻空闲块自动合并机制
 * 4. 内存碎片化管理
 * 5. vPortGetHeapStats() 详细统计
 * 6. heap_4与heap_2的区别
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
 * @brief 打印详细的内存统计信息（使用vPortGetHeapStats）
 *
 * 这个函数演示heap_4提供的详细内存统计功能。
 */
void vPrintDetailedMemoryStats(void)
{
    HeapStats_t xHeapStats;
    
    vPortGetHeapStats(&xHeapStats);
    
    vPrintString("\r\n========== 详细内存统计 ==========\r\n");
    vPrintString("总堆大小:       ");
    vPrintNumber(configTOTAL_HEAP_SIZE);
    vPrintString(" 字节\r\n");
    
    vPrintString("当前剩余空间:   ");
    vPrintNumber(xHeapStats.xAvailableHeapSpaceInBytes);
    vPrintString(" 字节\r\n");
    
    vPrintString("历史最小剩余:   ");
    vPrintNumber(xHeapStats.xMinimumEverFreeBytesRemaining);
    vPrintString(" 字节\r\n");
    
    vPrintString("空闲块数量:     ");
    vPrintNumber(xHeapStats.xNumberOfFreeBlocks);
    vPrintString("\r\n");
    
    vPrintString("最大空闲块:     ");
    vPrintNumber(xHeapStats.xSizeOfLargestFreeBlockInBytes);
    vPrintString(" 字节\r\n");
    
    vPrintString("最小空闲块:     ");
    vPrintNumber(xHeapStats.xSizeOfSmallestFreeBlockInBytes);
    vPrintString(" 字节\r\n");
    
    vPrintString("分配成功次数:   ");
    vPrintNumber(xHeapStats.xNumberOfSuccessfulAllocations);
    vPrintString("\r\n");
    
    vPrintString("释放成功次数:   ");
    vPrintNumber(xHeapStats.xNumberOfSuccessfulFrees);
    vPrintString("\r\n");
    vPrintString("===================================\r\n\r\n");
}

/**
 * @brief 打印简单的内存状态
 */
void vPrintMemoryStats(void)
{
    size_t xFreeSize = xPortGetFreeHeapSize();
    size_t xMinEver = xPortGetMinimumEverFreeHeapSize();

    vPrintString("\r\n---------- 内存状态 ----------\r\n");
    vPrintString("总堆大小: ");
    vPrintNumber(configTOTAL_HEAP_SIZE);
    vPrintString(" 字节\r\n");

    vPrintString("剩余大小: ");
    vPrintNumber(xFreeSize);
    vPrintString(" 字节\r\n");
    
    vPrintString("历史最小: ");
    vPrintNumber(xMinEver);
    vPrintString(" 字节\r\n");

    vPrintString("已分配:   ");
    vPrintNumber(configTOTAL_HEAP_SIZE - xFreeSize);
    vPrintString(" 字节\r\n");
    vPrintString("-------------------------------\r\n\r\n");
}

/* ==============================================
 *              任务函数定义
 * ==============================================
 */

/**
 * @brief 内存演示任务 - 演示heap_4的使用
 * @param pvParameters 任务参数
 */
void vMemoryDemoTask(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulStep = 0;

    vPrintString("[Demo] 内存演示任务启动！\r\n");

    vPrintString("\r\n[Demo] ========== 步骤1: 初始内存状态 ==========\r\n");
    vPrintDetailedMemoryStats();

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
                    vPrintString("[Demo] 注意：在heap_4中，空闲块会与相邻空闲块合并（但这里块2两边都被占用）\r\n");
                }
                break;

            case 5:
                vPrintString("[Demo] 释放块1...\r\n");
                if(g_pvAllocatedBlock1 != NULL)
                {
                    vPortFree(g_pvAllocatedBlock1);
                    g_pvAllocatedBlock1 = NULL;
                    vPrintString("[Demo] ✓ 块1已释放！\r\n");
                    vPrintString("[Demo] 关键：现在块1和块2的空闲空间相邻了！\r\n");
                    vPrintString("[Demo] heap_4会自动将它们合并成一个大块！\r\n");
                }
                break;

            case 6:
                vPrintString("[Demo] 尝试分配块4: 300 字节...\r\n");
                vPrintString("[Demo] 提示：由于heap_4的合并机制，应该能成功分配！\r\n");

                g_pvAllocatedBlock4 = pvPortMalloc(300);

                if(g_pvAllocatedBlock4 != NULL)
                {
                    vPrintString("[Demo] ✓ 块4分配成功！地址: ");
                    vPrintNumber((uint32_t)g_pvAllocatedBlock4);
                    vPrintString("\r\n");
                    vPrintString("[Demo] 说明：heap_4成功合并了相邻空闲块！\r\n");
                    vPrintString("[Demo] 对比heap_2：heap_2无法做到这点！\r\n");
                    memset(g_pvAllocatedBlock4, 0xDD, 300);
                }
                else
                {
                    vPrintString("[Demo] ✗ 块4分配失败！\r\n");
                }
                break;

            case 7:
                vPrintString("[Demo] 释放所有剩余块...\r\n");

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
                vPrintString("[Demo] 注意：heap_4会把所有相邻空闲块都合并！\r\n");
                vPrintString("[Demo] 最终应该只有一个大空闲块！\r\n");
                break;

            case 8:
                vPrintString("[Demo] 验证：尝试分配一个很大的块（3000字节）...\r\n");
                g_pvAllocatedBlock1 = pvPortMalloc(3000);

                if(g_pvAllocatedBlock1 != NULL)
                {
                    vPrintString("[Demo] ✓ 大块分配成功！\r\n");
                    vPrintString("[Demo] 证明：所有空闲块被成功合并！\r\n");
                    vPortFree(g_pvAllocatedBlock1);
                    g_pvAllocatedBlock1 = NULL;
                }
                else
                {
                    vPrintString("[Demo] ✗ 大块分配失败！\r\n");
                }
                break;

            default:
                vPrintString("[Demo] 演示完成！\r\n");
                break;
        }

        vPrintDetailedMemoryStats();
        vTaskDelay(pdMS_TO_TICKS(2000));

        if(ulStep >= 9)
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
    vPrintString("  FreeRTOS heap_4 内存分配器示例\r\n");
    vPrintString("====================================\r\n");
    vPrintString("\r\n");

    vPrintString("学习要点：\r\n");
    vPrintString("1. heap_4 内存分配器工作原理\r\n");
    vPrintString("2. 首次适配算法（First Fit）\r\n");
    vPrintString("3. 相邻空闲块自动合并\r\n");
    vPrintString("4. vPortGetHeapStats() 详细统计\r\n");
    vPrintString("5. 抗碎片化能力演示\r\n");
    vPrintString("6. heap_4 与 heap_2 的区别\r\n");
    vPrintString("\r\n");

    vPrintString("====================================\r\n");
    vPrintString("  heap_4 的特点：\r\n");
    vPrintString("====================================\r\n");
    vPrintString("✓ 支持内存分配和释放\r\n");
    vPrintString("✓ 使用首次适配算法\r\n");
    vPrintString("✓ 自动合并相邻空闲块\r\n");
    vPrintString("✓ 优秀的抗碎片化能力\r\n");
    vPrintString("✓ 提供详细的统计信息\r\n");
    vPrintString("\r\n");
    vPrintString("✗ 比heap_2稍慢（合并开销）\r\n");
    vPrintString("====================================\r\n");
    vPrintString("\r\n");

    vPrintString("====================================\r\n");
    vPrintString("  heap_4 与 heap_2 对比：\r\n");
    vPrintString("====================================\r\n");
    vPrintString("特性          heap_2      heap_4\r\n");
    vPrintString("------------------------------------\r\n");
    vPrintString("分配算法      最佳适配    首次适配\r\n");
    vPrintString("块合并        ✗ 不合并    ✓ 自动合并\r\n");
    vPrintString("抗碎片化      弱          强\r\n");
    vPrintString("详细统计      ✗ 无        ✓ 有\r\n");
    vPrintString("历史最小      ✗ 无        ✓ 有\r\n");
    vPrintString("适用场景      简单动态    通用场景\r\n");
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
