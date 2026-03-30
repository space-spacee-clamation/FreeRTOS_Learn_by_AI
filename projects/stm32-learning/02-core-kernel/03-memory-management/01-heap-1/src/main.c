/**
 * @file main.c
 * @brief FreeRTOS heap_1内存分配器示例
 *
 * 本项目演示FreeRTOS heap_1内存分配器的使用方法。
 * 学习要点：
 * 1. heap_1内存分配器的工作原理
 * 2. pvPortMalloc() 内存分配
 * 3. 内存使用统计
 * 4. heap_1的特点和限制
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

/* ==============================================
 *              辅助函数
 * ==============================================
 */

/**
 * @brief 打印当前内存状态
 *
 * 这个函数打印堆内存的使用情况，
 * 包括总大小、已分配大小、剩余大小等。
 */
void vPrintMemoryStats(void)
{
    /* ==============================================
     *       学习重点1：获取空闲堆大小
     * ==============================================
     *
     * size_t xPortGetFreeHeapSize(void);
     *
     * 返回值：
     * - 当前堆中剩余的字节数
     *
     * 注意：
     * - heap_1不支持内存碎片统计
     * - 这个函数可以用来监控内存使用情况
     */

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
 *
 * 本项目创建2个任务：
 * - vMemoryDemoTask: 演示内存分配过程
 * - vMonitorTask: 定期监控内存状态
 */

/**
 * @brief 内存演示任务 - 演示heap_1的使用
 * @param pvParameters 任务参数
 *
 * 这个任务演示：
 * 1. 初始内存状态
 * 2. 多次分配内存
 * 3. 观察内存变化
 * 4. 演示heap_1不支持释放
 */
void vMemoryDemoTask(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulStep = 0;

    vPrintString("[Demo] 内存演示任务启动！\r\n");

    /* ==============================================
     *       学习重点2：观察初始内存状态
     * ==============================================
     *
     * heap_1在启动时会有一些初始分配：
     * - 空闲任务堆栈
     * - 可能还有其他内核对象
     */

    vPrintString("\r\n[Demo] ========== 步骤1: 初始内存状态 ==========\r\n");
    vPrintMemoryStats();

    vTaskDelay(pdMS_TO_TICKS(1000));

    for(;;)
    {
        ulStep++;

        vPrintString("\r\n[Demo] ========== 步骤");
        vPrintNumber(ulStep + 1);
        vPrintString(" ==========\r\n");

        /* ==============================================
         *       学习重点3：使用 pvPortMalloc() 分配内存
         * ==============================================
         *
         * void *pvPortMalloc( size_t xWantedSize );
         *
         * 参数：
         * - xWantedSize: 需要分配的字节数
         *
         * 返回值：
         * - 成功：指向分配内存的指针
         * - 失败：NULL
         *
         * heap_1的特点：
         * - 简单的线性分配
         * - 不支持内存释放！
         * - 没有内存碎片问题
         * - 适合静态分配场景
         */

        switch(ulStep)
        {
            case 1:
                /* 第一次分配：100字节 */
                vPrintString("[Demo] 分配 100 字节...\r\n");
                g_pvAllocatedBlock1 = pvPortMalloc(100);

                if(g_pvAllocatedBlock1 != NULL)
                {
                    vPrintString("[Demo] ✓ 分配成功！地址: ");
                    vPrintNumber((uint32_t)g_pvAllocatedBlock1);
                    vPrintString("\r\n");

                    /* 写入一些数据 */
                    memset(g_pvAllocatedBlock1, 0xAA, 100);
                    vPrintString("[Demo] 已填充数据 0xAA\r\n");
                }
                else
                {
                    vPrintString("[Demo] ✗ 分配失败！\r\n");
                }
                break;

            case 2:
                /* 第二次分配：200字节 */
                vPrintString("[Demo] 分配 200 字节...\r\n");
                g_pvAllocatedBlock2 = pvPortMalloc(200);

                if(g_pvAllocatedBlock2 != NULL)
                {
                    vPrintString("[Demo] ✓ 分配成功！地址: ");
                    vPrintNumber((uint32_t)g_pvAllocatedBlock2);
                    vPrintString("\r\n");

                    memset(g_pvAllocatedBlock2, 0xBB, 200);
                    vPrintString("[Demo] 已填充数据 0xBB\r\n");
                }
                else
                {
                    vPrintString("[Demo] ✗ 分配失败！\r\n");
                }
                break;

            case 3:
                /* 第三次分配：500字节 */
                vPrintString("[Demo] 分配 500 字节...\r\n");
                g_pvAllocatedBlock3 = pvPortMalloc(500);

                if(g_pvAllocatedBlock3 != NULL)
                {
                    vPrintString("[Demo] ✓ 分配成功！地址: ");
                    vPrintNumber((uint32_t)g_pvAllocatedBlock3);
                    vPrintString("\r\n");

                    memset(g_pvAllocatedBlock3, 0xCC, 500);
                    vPrintString("[Demo] 已填充数据 0xCC\r\n");
                }
                else
                {
                    vPrintString("[Demo] ✗ 分配失败！\r\n");
                }
                break;

            case 4:
                /* ==============================================
                 *       学习重点4：heap_1 不支持释放
                 * ==============================================
                 *
                 * void vPortFree( void *pv );
                 *
                 * 重要提示：
                 * - heap_1的 vPortFree() 什么都不做！
                 * - 内存一旦分配就无法回收
                 * - 这是heap_1的最大限制
                 *
                 * 为什么？
                 * - heap_1设计用于静态分配场景
                 * - 系统启动时分配所有需要的内存
                 * - 运行时不再分配/释放
                 */

                vPrintString("[Demo] 尝试释放内存...\r\n");
                vPrintString("[Demo] 注意：heap_1 不支持内存释放！\r\n");
                vPrintString("[Demo] vPortFree() 调用会被忽略\r\n");

                if(g_pvAllocatedBlock1 != NULL)
                {
                    vPortFree(g_pvAllocatedBlock1);
                    g_pvAllocatedBlock1 = NULL;
                    vPrintString("[Demo] 已调用 vPortFree() (但实际没有释放)\r\n");
                }
                break;

            default:
                vPrintString("[Demo] 演示完成！进入循环...\r\n");
                break;
        }

        /* 打印当前内存状态 */
        vPrintMemoryStats();

        /* 延时2秒 */
        vTaskDelay(pdMS_TO_TICKS(2000));

        if(ulStep >= 5)
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

        /* 延时3秒 */
        vTaskDelay(pdMS_TO_TICKS(3000));
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
    vPrintString("  FreeRTOS heap_1 内存分配器示例\r\n");
    vPrintString("====================================\r\n");
    vPrintString("\r\n");

    vPrintString("学习要点：\r\n");
    vPrintString("1. heap_1 内存分配器工作原理\r\n");
    vPrintString("2. pvPortMalloc() 分配内存\r\n");
    vPrintString("3. xPortGetFreeHeapSize() 获取统计\r\n");
    vPrintString("4. heap_1 的特点和限制\r\n");
    vPrintString("\r\n");

    vPrintString("====================================\r\n");
    vPrintString("  heap_1 的特点：\r\n");
    vPrintString("====================================\r\n");
    vPrintString("✓ 实现简单，代码量小\r\n");
    vPrintString("✓ 没有内存碎片\r\n");
    vPrintString("✓ 执行时间确定\r\n");
    vPrintString("\r\n");
    vPrintString("✗ 不支持内存释放\r\n");
    vPrintString("✗ 内存使用后无法回收\r\n");
    vPrintString("✗ 适合静态分配场景\r\n");
    vPrintString("====================================\r\n");
    vPrintString("\r\n");

    /* ==============================================
     *       学习重点5：创建任务
     * ==============================================
     */

    vPrintString("正在创建任务...\r\n");

    /* 演示任务：优先级2 */
    xTaskCreate(vMemoryDemoTask, "DemoTask", 256, NULL, 2, NULL);

    /* 监控任务：优先级1 */
    xTaskCreate(vMonitorTask, "Monitor", 256, NULL, 1, NULL);

    vPrintString("所有任务创建成功！\r\n");
    vPrintString("\r\n");

    /* ==============================================
     *      启动调度器
     * ==============================================
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
