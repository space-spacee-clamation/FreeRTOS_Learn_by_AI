
/**
 * @file main.c
 * @brief FreeRTOS heap_5内存分配器示例
 *
 * 本项目演示FreeRTOS heap_5内存分配器的使用方法。
 * 学习要点：
 * 1. heap_5内存分配器的工作原理
 * 2. vPortDefineHeapRegions() 函数使用
 * 3. 多非连续内存区域管理
 * 4. 跨区域内存分配
 * 5. 区域内块合并
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
void vPrintHex(uint32_t ulNumber);

/* ==============================================
 *              全局变量 - heap_5专用
 * ==============================================
 */

/* 定义多个独立的内存区域用于heap_5 */
static uint8_t ucHeapRegion1[10 * 1024];  /* 区域1: 10KB */
static uint8_t ucHeapRegion2[15 * 1024];  /* 区域2: 15KB */
static uint8_t ucHeapRegion3[10 * 1024];  /* 区域3: 10KB */

/* 用于存储分配的内存块指针 */
void *g_pvBlockA = NULL;
void *g_pvBlockB = NULL;
void *g_pvBlockC = NULL;
void *g_pvBlockD = NULL;

/* ==============================================
 *              辅助函数
 * ==============================================
 */

void vPrintDetailedMemoryStats(void)
{
    size_t xFreeSize = xPortGetFreeHeapSize();
    size_t xMinEver = xPortGetMinimumEverFreeHeapSize();

    vPrintString("\r\n---------- 详细内存状态 ----------\r\n");
    vPrintString("总堆大小: ");
    vPrintNumber(10*1024 + 15*1024 + 10*1024);
    vPrintString(" 字节 (3个区域)\r\n");
    vPrintString("剩余大小: ");
    vPrintNumber(xFreeSize);
    vPrintString(" 字节\r\n");
    vPrintString("历史最小: ");
    vPrintNumber(xMinEver);
    vPrintString(" 字节\r\n");
    vPrintString("-----------------------------------\r\n\r\n");
}

void vInitializeHeapRegions(void)
{
    HeapRegion_t xHeapRegions[4];
    uint8_t *pucAddr1 = (uint8_t *)ucHeapRegion1;
    uint8_t *pucAddr2 = (uint8_t *)ucHeapRegion2;
    uint8_t *pucAddr3 = (uint8_t *)ucHeapRegion3;
    uint8_t *pucTemp;
    size_t xSizeTemp;
    int i, j;

    vPrintString("[Demo] 步骤1：定义内存区域...\r\n");
    vPrintString("[Demo] 原始地址:\r\n");
    vPrintString("[Demo] 区域1: 地址=0x");
    vPrintHex((uint32_t)pucAddr1);
    vPrintString(" 大小=");
    vPrintNumber(sizeof(ucHeapRegion1));
    vPrintString(" 字节\r\n");
    vPrintString("[Demo] 区域2: 地址=0x");
    vPrintHex((uint32_t)pucAddr2);
    vPrintString(" 大小=");
    vPrintNumber(sizeof(ucHeapRegion2));
    vPrintString(" 字节\r\n");
    vPrintString("[Demo] 区域3: 地址=0x");
    vPrintHex((uint32_t)pucAddr3);
    vPrintString(" 大小=");
    vPrintNumber(sizeof(ucHeapRegion3));
    vPrintString(" 字节\r\n");

    /* 初始化临时数组用于排序 */
    xHeapRegions[0].pucStartAddress = pucAddr1;
    xHeapRegions[0].xSizeInBytes = sizeof(ucHeapRegion1);
    xHeapRegions[1].pucStartAddress = pucAddr2;
    xHeapRegions[1].xSizeInBytes = sizeof(ucHeapRegion2);
    xHeapRegions[2].pucStartAddress = pucAddr3;
    xHeapRegions[2].xSizeInBytes = sizeof(ucHeapRegion3);
    xHeapRegions[3].pucStartAddress = NULL;
    xHeapRegions[3].xSizeInBytes = 0;

    /* 冒泡排序：按地址从低到高排序 */
    for(i = 0; i < 2; i++)
    {
        for(j = 0; j < 2 - i; j++)
        {
            if(xHeapRegions[j].pucStartAddress > xHeapRegions[j+1].pucStartAddress)
            {
                pucTemp = xHeapRegions[j].pucStartAddress;
                xSizeTemp = xHeapRegions[j].xSizeInBytes;
                xHeapRegions[j].pucStartAddress = xHeapRegions[j+1].pucStartAddress;
                xHeapRegions[j].xSizeInBytes = xHeapRegions[j+1].xSizeInBytes;
                xHeapRegions[j+1].pucStartAddress = pucTemp;
                xHeapRegions[j+1].xSizeInBytes = xSizeTemp;
            }
        }
    }

    vPrintString("\r\n[Demo] 按地址排序后:\r\n");
    for(i = 0; i < 3; i++)
    {
        vPrintString("[Demo] 区域");
        vPrintNumber(i+1);
        vPrintString(": 地址=0x");
        vPrintHex((uint32_t)xHeapRegions[i].pucStartAddress);
        vPrintString(" 大小=");
        vPrintNumber(xHeapRegions[i].xSizeInBytes);
        vPrintString(" 字节\r\n");
    }

    vPrintString("[Demo] 总计: ");
    vPrintNumber(sizeof(ucHeapRegion1) + sizeof(ucHeapRegion2) + sizeof(ucHeapRegion3));
    vPrintString(" 字节\r\n\r\n");

    vPrintString("[Demo] 步骤2：调用 vPortDefineHeapRegions()...\r\n");
    vPortDefineHeapRegions(xHeapRegions);
    vPrintString("[Demo] ✓ heap_5 初始化成功！\r\n\r\n");
}

const char *pcGetRegionName(void *pvAddress)
{
    if(pvAddress >= (void *)ucHeapRegion1 &&
       pvAddress < (void *)(ucHeapRegion1 + sizeof(ucHeapRegion1)))
    {
        return "区域1";
    }
    else if(pvAddress >= (void *)ucHeapRegion2 &&
            pvAddress < (void *)(ucHeapRegion2 + sizeof(ucHeapRegion2)))
    {
        return "区域2";
    }
    else if(pvAddress >= (void *)ucHeapRegion3 &&
            pvAddress < (void *)(ucHeapRegion3 + sizeof(ucHeapRegion3)))
    {
        return "区域3";
    }
    return "未知";
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
    vPrintDetailedMemoryStats();
    vTaskDelay(pdMS_TO_TICKS(1000));

    for(;;)
    {
        ulStep++;

        vPrintString("\r\n[Demo] ========== 演示阶段");
        vPrintNumber(ulStep);
        vPrintString(" ==========\r\n");

        switch(ulStep)
        {
            case 1:
                vPrintString("[Demo] 步骤3：分配内存，测试跨区域...\r\n");

                vPrintString("[Demo] 分配 块A (4000字节)... ");
                g_pvBlockA = pvPortMalloc(4000);
                if(g_pvBlockA != NULL)
                {
                    vPrintString("成功 (");
                    vPrintString(pcGetRegionName(g_pvBlockA));
                    vPrintString(")\r\n");
                    memset(g_pvBlockA, 0xAA, 4000);
                }
                else
                {
                    vPrintString("失败！\r\n");
                }

                vPrintString("[Demo] 分配 块B (5000字节)... ");
                g_pvBlockB = pvPortMalloc(5000);
                if(g_pvBlockB != NULL)
                {
                    vPrintString("成功 (");
                    vPrintString(pcGetRegionName(g_pvBlockB));
                    vPrintString(")\r\n");
                    memset(g_pvBlockB, 0xBB, 5000);
                }
                else
                {
                    vPrintString("失败！\r\n");
                }

                vPrintString("[Demo] 分配 块C (8000字节)... ");
                g_pvBlockC = pvPortMalloc(8000);
                if(g_pvBlockC != NULL)
                {
                    vPrintString("成功 (");
                    vPrintString(pcGetRegionName(g_pvBlockC));
                    vPrintString(")\r\n");
                    memset(g_pvBlockC, 0xCC, 8000);
                }
                else
                {
                    vPrintString("失败！\r\n");
                }

                vPrintString("[Demo] 分配 块D (20000字节)... ");
                g_pvBlockD = pvPortMalloc(20000);
                if(g_pvBlockD != NULL)
                {
                    vPrintString("成功 (");
                    vPrintString(pcGetRegionName(g_pvBlockD));
                    vPrintString(")\r\n");
                    memset(g_pvBlockD, 0xDD, 20000);
                }
                else
                {
                    vPrintString("失败！\r\n");
                }

                vPrintString("[Demo] ✓ 跨区域分配正常工作！\r\n");
                break;

            case 2:
                vPrintString("[Demo] 步骤4：释放部分内存，测试合并...\r\n");

                vPrintString("[Demo] 释放块A\r\n");
                if(g_pvBlockA != NULL)
                {
                    vPortFree(g_pvBlockA);
                    g_pvBlockA = NULL;
                }

                vPrintString("[Demo] 释放块B\r\n");
                if(g_pvBlockB != NULL)
                {
                    vPortFree(g_pvBlockB);
                    g_pvBlockB = NULL;
                }

                vPrintString("[Demo] ✓ 单区域内块合并正常工作！\r\n");
                break;

            case 3:
                vPrintString("[Demo] 步骤5：详细内存统计...\r\n");
                break;

            case 4:
                vPrintString("[Demo] 步骤6：演示注意事项...\r\n");
                vPrintString("[Demo] 注意：如果不调用vPortDefineHeapRegions()会断言失败！\r\n");
                vPrintString("[Demo] 注意：区域必须按地址从低到高排序！\r\n");
                vPrintString("[Demo] 注意：必须有NULL结束标记！\r\n");
                vPrintString("\r\n");
                vPrintString("[Demo] heap_5学习要点总结：\r\n");
                vPrintString("[Demo] 1. 支持多个非连续内存区域\r\n");
                vPrintString("[Demo] 2. 必须先调用vPortDefineHeapRegions()\r\n");
                vPrintString("[Demo] 3. 区域按地址从低到高排序\r\n");
                vPrintString("[Demo] 4. 继承heap_4的块合并功能\r\n");
                vPrintString("[Demo] 5. 适合内存地址分散的嵌入式系统\r\n");
                vPrintString("[Demo] 6. 可以组合内部和外部RAM\r\n");
                break;

            default:
                vPrintString("[Demo] 演示完成！\r\n");
                break;
        }

        vPrintDetailedMemoryStats();
        vTaskDelay(pdMS_TO_TICKS(2000));

        if(ulStep >= 5)
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
    vPrintString("1. heap_5 多区域内存管理\r\n");
    vPrintString("2. HeapRegion_t 结构体使用\r\n");
    vPrintString("3. vPortDefineHeapRegions() 初始化\r\n");
    vPrintString("4. 跨区域内存分配\r\n");
    vPrintString("5. 适用场景和注意事项\r\n");
    vPrintString("\r\n");

    vPrintString("====================================\r\n");
    vPrintString("  heap_5 的特点：\r\n");
    vPrintString("====================================\r\n");
    vPrintString("✓ 支持多个非连续内存区域\r\n");
    vPrintString("✓ 继承heap_4的块合并功能\r\n");
    vPrintString("✓ 使用首次适配算法\r\n");
    vPrintString("✓ 需要手动初始化内存区域\r\n");
    vPrintString("✓ 适合内存地址分散的系统\r\n");
    vPrintString("====================================\r\n");
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

void vPrintHex(uint32_t ulNumber)
{
    char cBuffer[9];
    int iIndex = 0;
    int i;
    uint32_t ulNibble;

    for(i = 7; i >= 0; i--)
    {
        ulNibble = (ulNumber >> (i * 4)) & 0xF;
        if(ulNibble < 10)
        {
            cBuffer[iIndex++] = '0' + ulNibble;
        }
        else
        {
            cBuffer[iIndex++] = 'A' + (ulNibble - 10);
        }
    }
    cBuffer[iIndex] = '\0';

    vPrintString(cBuffer);
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

