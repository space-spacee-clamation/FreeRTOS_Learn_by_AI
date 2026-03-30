/**
 * @file main.c
 * @brief FreeRTOS heap_5内存分配器示例
 *
 * 本项目演示FreeRTOS heap_5内存分配器的使用方法。
 * 学习要点：
 * 1. heap_5内存分配器的工作原理
 * 2. vPortDefineHeapRegions() 函数使用
 * 3. 多非连续内存区域管理
 *
 * 参考资料：
 * - FreeRTOS官方文档: https://www.freertos.org/a00111.html
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
 *              全局变量 - heap_5专用
 * ==============================================
 */

/* 定义多个独立的内存区域用于heap_5 */
static uint8_t ucHeapRegion1[10 * 1024];  /* 区域1: 10KB */
static uint8_t ucHeapRegion2[15 * 1024];  /* 区域2: 15KB */
static uint8_t ucHeapRegion3[10 * 1024];  /* 区域3: 10KB */

/* 用于存储分配的内存块指针 */
void *g_pvAllocatedBlock1 = NULL;

/* ==============================================
 *              辅助函数
 * ==============================================
 */

void vPrintMemoryStats(void)
{
    size_t xFreeSize = xPortGetFreeHeapSize();
    size_t xMinEver = xPortGetMinimumEverFreeHeapSize();

    vPrintString("\r\n---------- 内存状态 ----------\r\n");
    vPrintString("总堆大小: ");
    vPrintNumber(10*1024 + 15*1024 + 10*1024);
    vPrintString(" 字节 (3个区域)\r\n");
    vPrintString("剩余大小: ");
    vPrintNumber(xFreeSize);
    vPrintString(" 字节\r\n");
    vPrintString("-------------------------------\r\n\r\n");
}

void vInitializeHeapRegions(void)
{
    HeapRegion_t xHeapRegions[] =
    {
        { (uint8_t *)ucHeapRegion1, sizeof(ucHeapRegion1) },
        { (uint8_t *)ucHeapRegion2, sizeof(ucHeapRegion2) },
        { (uint8_t *)ucHeapRegion3, sizeof(ucHeapRegion3) },
        { NULL, 0 }
    };
    
    vPrintString("[Init] 正在初始化 heap_5 内存区域...\r\n");
    vPrintString("[Init] 区域1: 地址=");
    vPrintNumber((uint32_t)ucHeapRegion1);
    vPrintString(" 大小=");
    vPrintNumber(sizeof(ucHeapRegion1));
    vPrintString(" 字节\r\n");
    vPrintString("[Init] 区域2: 地址=");
    vPrintNumber((uint32_t)ucHeapRegion2);
    vPrintString(" 大小=");
    vPrintNumber(sizeof(ucHeapRegion2));
    vPrintString(" 字节\r\n");
    vPrintString("[Init] 区域3: 地址=");
    vPrintNumber((uint32_t)ucHeapRegion3);
    vPrintString(" 大小=");
    vPrintNumber(sizeof(ucHeapRegion3));
    vPrintString(" 字节\r\n");
    
    vPortDefineHeapRegions(xHeapRegions);
    
    vPrintString("[Init] ✓ heap_5 初始化完成！\r\n\r\n");
}

/* ==============================================
 *              任务函数定义
 * ==============================================
 */

void vDemoTask(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulStep = 0;

    vPrintString("[Demo] 演示任务启动！\r\n");
    vPrintMemoryStats();
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
                vPrintString("[Demo] 分配块1: 5000 字节...\r\n");
                g_pvAllocatedBlock1 = pvPortMalloc(5000);
                if(g_pvAllocatedBlock1 != NULL)
                {
                    vPrintString("[Demo] ✓ 分配成功！地址: ");
                    vPrintNumber((uint32_t)g_pvAllocatedBlock1);
                    vPrintString("\r\n");
                    memset(g_pvAllocatedBlock1, 0xAA, 5000);
                }
                break;

            case 2:
                vPrintString("[Demo] 释放块1...\r\n");
                if(g_pvAllocatedBlock1 != NULL)
                {
                    vPortFree(g_pvAllocatedBlock1);
                    g_pvAllocatedBlock1 = NULL;
                    vPrintString("[Demo] ✓ 已释放！\r\n");
                }
                break;

            default:
                vPrintString("[Demo] 演示完成！\r\n");
                break;
        }

        vPrintMemoryStats();
        vTaskDelay(pdMS_TO_TICKS(2000));

        if(ulStep >= 3)
        {
            vPrintString("[Demo] 进入空闲循环\r\n");
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
    vPrintString("  FreeRTOS heap_5 内存分配器示例\r\n");
    vPrintString("====================================\r\n");
    vPrintString("\r\n");

    vPrintString("学习要点：\r\n");
    vPrintString("1. heap_5 内存分配器工作原理\r\n");
    vPrintString("2. vPortDefineHeapRegions() 使用\r\n");
    vPrintString("3. 多非连续内存区域管理\r\n");
    vPrintString("\r\n");

    /* 关键：必须在任何内存分配之前初始化heap_5！ */
    vInitializeHeapRegions();

    vPrintString("正在创建任务...\r\n");
    xTaskCreate(vDemoTask, "DemoTask", 256, NULL, 2, NULL);
    vPrintString("任务创建成功！\r\n");
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
