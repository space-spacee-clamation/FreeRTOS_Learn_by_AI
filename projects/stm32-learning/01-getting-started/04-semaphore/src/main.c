/**
 * @file main.c
 * @brief FreeRTOS信号量使用示例
 *
 * 本项目演示如何使用FreeRTOS信号量进行任务同步和资源保护。
 * 学习要点：
 * 1. 二值信号量 - 用于任务同步
 * 2. 互斥信号量 - 用于共享资源保护
 * 3. xSemaphoreTake() 和 xSemaphoreGive() 的使用
 * 4. 优先级继承（互斥信号量）
 *
 * 参考资料：
 * - FreeRTOS官方文档: https://www.freertos.org/a00113.html
 * - Mastering-the-FreeRTOS-Kernel 第7章
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* FreeRTOS 头文件 */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* 平台相关的串口输出函数声明 */
void vUARTInit(void);
void vPrintString(const char *pcString);
void vPrintNumber(uint32_t ulNumber);

/* ==============================================
 *              信号量句柄（全局变量）
 * ==============================================
 *
 * 信号量句柄需要在多个任务中访问，所以声明为全局变量。
 */

/* 二值信号量：用于任务同步（类似事件标志） */
SemaphoreHandle_t xBinarySemaphore = NULL;

/* 互斥信号量：用于保护共享资源 */
SemaphoreHandle_t xMutex = NULL;

/* ==============================================
 *              共享资源（模拟）
 * ==============================================
 */

/* 共享资源：模拟一个需要保护的全局变量 */
uint32_t g_ulSharedCounter = 0;

/* 共享资源：模拟一个需要保护的字符串缓冲区 */
char g_cSharedBuffer[64] = {0};

/* ==============================================
 *              任务函数定义
 * ==============================================
 *
 * 本项目创建5个任务：
 * - vTask1: 触发同步事件
 * - vTask2: 等待同步事件
 * - vTask3: 使用互斥锁修改共享资源
 * - vTask4: 使用互斥锁修改共享资源
 * - vMonitorTask: 监控共享资源状态
 */

/**
 * @brief 任务1 - 触发同步事件
 * @param pvParameters 任务参数
 *
 * 这个任务定期"Give"二值信号量，模拟事件发生。
 */
void vTask1(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulCounter = 0;

    vPrintString("[Task1] 同步事件触发任务启动！\r\n");

    for(;;)
    {
        ulCounter++;

        vPrintString("\r\n[Task1] ============= 事件发生 #");
        vPrintNumber(ulCounter);
        vPrintString(" =============\r\n");

        /* ==============================================
         *       学习重点1：Give二值信号量
         * ==============================================
         *
         * BaseType_t xSemaphoreGive(
         *     SemaphoreHandle_t xSemaphore
         * );
         *
         * 二值信号量就像一个"标志"：
         * - Give：设置标志（让等待的任务可以运行）
         * - Take：清除标志（等待事件发生）
         */

        vPrintString("[Task1] 发送信号量...\r\n");
        xSemaphoreGive(xBinarySemaphore);
        vPrintString("[Task1] 信号量已发送！\r\n");

        /* 延时500ms */
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/**
 * @brief 任务2 - 等待同步事件
 * @param pvParameters 任务参数
 *
 * 这个任务"Take"二值信号量，等待事件发生。
 */
void vTask2(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulEventCount = 0;

    vPrintString("[Task2] 同步事件等待任务启动！\r\n");

    for(;;)
    {
        /* ==============================================
         *       学习重点2：Take二值信号量
         * ==============================================
         *
         * BaseType_t xSemaphoreTake(
         *     SemaphoreHandle_t xSemaphore,
         *     TickType_t xTicksToWait
         * );
         *
         * 参数：
         * 1. xSemaphore: 信号量句柄
         * 2. xTicksToWait: 等待时间（0=不等待，portMAX_DELAY=无限等待）
         *
         * 返回值：
         * - pdPASS: 成功获取信号量
         * - pdFAIL: 超时
         */

        vPrintString("[Task2] 等待信号量...\r\n");

        /* 无限等待信号量 */
        if(xSemaphoreTake(xBinarySemaphore, portMAX_DELAY) == pdPASS)
        {
            ulEventCount++;
            vPrintString("[Task2] ✓ 收到事件！总数: ");
            vPrintNumber(ulEventCount);
            vPrintString("\r\n");

            /* 模拟处理事件 */
            vPrintString("[Task2] 正在处理事件...\r\n");
            vTaskDelay(pdMS_TO_TICKS(100));
            vPrintString("[Task2] 事件处理完成！\r\n");
        }
    }
}

/**
 * @brief 任务3 - 使用互斥锁修改共享资源
 * @param pvParameters 任务参数
 *
 * 这个任务使用互斥信号量保护共享资源的访问。
 */
void vTask3(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulLocalValue;

    vPrintString("[Task3] 共享资源修改任务A启动！\r\n");

    for(;;)
    {
        /* ==============================================
         *       学习重点3：获取互斥锁
         * ==============================================
         *
         * 互斥信号量用于保护共享资源：
         * - Take：获取锁（如果已被占用则等待）
         * - Give：释放锁（让其他任务可以使用）
         *
         * 互斥信号量有优先级继承机制，防止优先级反转！
         */

        /* 获取互斥锁 */
        vPrintString("[Task3] 尝试获取互斥锁...\r\n");
        if(xSemaphoreTake(xMutex, pdMS_TO_TICKS(100)) == pdPASS)
        {
            vPrintString("[Task3] ✓ 获取互斥锁成功！\r\n");

            /* ==============================================
             *       临界区：访问共享资源
             * ==============================================
             *
             * 这里的代码是"临界区"，同一时间只有一个任务能执行。
             */

            /* 读取-修改-写回共享计数器 */
            ulLocalValue = g_ulSharedCounter;
            vPrintString("[Task3] 读取共享计数器: ");
            vPrintNumber(ulLocalValue);
            vPrintString("\r\n");

            ulLocalValue += 10;  /* 修改 */

            /* 模拟耗时操作 */
            vTaskDelay(pdMS_TO_TICKS(50));

            g_ulSharedCounter = ulLocalValue;  /* 写回 */
            vPrintString("[Task3] 写回共享计数器: ");
            vPrintNumber(g_ulSharedCounter);
            vPrintString("\r\n");

            /* 修改共享字符串缓冲区 */
            strcpy(g_cSharedBuffer, "Task3 was here!");
            vPrintString("[Task3] 修改共享缓冲区: ");
            vPrintString(g_cSharedBuffer);
            vPrintString("\r\n");

            /* ==============================================
             *       学习重点4：释放互斥锁
             * ==============================================
             *
             * 必须配对使用Take和Give！
             * 忘记Give会导致死锁！
             */

            vPrintString("[Task3] 释放互斥锁...\r\n");
            xSemaphoreGive(xMutex);
            vPrintString("[Task3] ✓ 互斥锁已释放！\r\n");
        }
        else
        {
            vPrintString("[Task3] ✗ 获取互斥锁超时！\r\n");
        }

        /* 延时一段时间 */
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}

/**
 * @brief 任务4 - 使用互斥锁修改共享资源
 * @param pvParameters 任务参数
 *
 * 这个任务也使用互斥信号量保护共享资源的访问。
 */
void vTask4(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulLocalValue;

    vPrintString("[Task4] 共享资源修改任务B启动！\r\n");

    for(;;)
    {
        /* 获取互斥锁 */
        vPrintString("[Task4] 尝试获取互斥锁...\r\n");
        if(xSemaphoreTake(xMutex, pdMS_TO_TICKS(100)) == pdPASS)
        {
            vPrintString("[Task4] ✓ 获取互斥锁成功！\r\n");

            /* 临界区：访问共享资源 */
            ulLocalValue = g_ulSharedCounter;
            vPrintString("[Task4] 读取共享计数器: ");
            vPrintNumber(ulLocalValue);
            vPrintString("\r\n");

            ulLocalValue += 1;  /* 每次加1，和Task3不同 */

            /* 模拟耗时操作 */
            vTaskDelay(pdMS_TO_TICKS(30));

            g_ulSharedCounter = ulLocalValue;
            vPrintString("[Task4] 写回共享计数器: ");
            vPrintNumber(g_ulSharedCounter);
            vPrintString("\r\n");

            /* 修改共享字符串缓冲区 */
            strcpy(g_cSharedBuffer, "Task4 was here!");
            vPrintString("[Task4] 修改共享缓冲区: ");
            vPrintString(g_cSharedBuffer);
            vPrintString("\r\n");

            /* 释放互斥锁 */
            vPrintString("[Task4] 释放互斥锁...\r\n");
            xSemaphoreGive(xMutex);
            vPrintString("[Task4] ✓ 互斥锁已释放！\r\n");
        }
        else
        {
            vPrintString("[Task4] ✗ 获取互斥锁超时！\r\n");
        }

        /* 延时一段时间 */
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}

/**
 * @brief 监控任务 - 监控共享资源状态
 * @param pvParameters 任务参数
 *
 * 这个任务定期显示共享资源的状态。
 */
void vMonitorTask(void *pvParameters)
{
    (void)pvParameters;

    vPrintString("[Monitor] 监控任务启动！\r\n");

    for(;;)
    {
        vPrintString("\r\n[Monitor] ---------- 资源状态 ----------\r\n");
        vPrintString("[Monitor] 共享计数器 = ");
        vPrintNumber(g_ulSharedCounter);
        vPrintString("\r\n");
        vPrintString("[Monitor] 共享缓冲区 = ");
        vPrintString(g_cSharedBuffer);
        vPrintString("\r\n");
        vPrintString("[Monitor] -----------------------------\r\n\r\n");

        /* 延时1秒 */
        vTaskDelay(pdMS_TO_TICKS(1000));
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
    vPrintString("  FreeRTOS信号量使用示例\r\n");
    vPrintString("====================================\r\n");
    vPrintString("\r\n");

    vPrintString("学习要点：\r\n");
    vPrintString("1. 二值信号量 - 任务同步\r\n");
    vPrintString("2. 互斥信号量 - 资源保护\r\n");
    vPrintString("3. xSemaphoreTake() / xSemaphoreGive()\r\n");
    vPrintString("4. 优先级继承（互斥信号量）\r\n");
    vPrintString("\r\n");

    /* ==============================================
     *       学习重点5：创建二值信号量
     * ==============================================
     *
     * SemaphoreHandle_t xSemaphoreCreateBinary(void);
     *
     * 注意：
     * - 创建后信号量是"空"的（需要先Give才能Take）
     * - 二值信号量只有0和1两种状态
     * - 主要用于任务同步
     */

    vPrintString("正在创建二值信号量...\r\n");
    xBinarySemaphore = xSemaphoreCreateBinary();

    if(xBinarySemaphore == NULL)
    {
        vPrintString("错误：二值信号量创建失败！\r\n");
        for(;;);
    }

    vPrintString("二值信号量创建成功！\r\n");
    vPrintString("\r\n");

    /* ==============================================
     *       学习重点6：创建互斥信号量
     * ==============================================
     *
     * SemaphoreHandle_t xSemaphoreCreateMutex(void);
     *
     * 注意：
     * - 创建后互斥锁是"可用"的（可以直接Take）
     * - 互斥信号量有优先级继承机制
     * - 主要用于共享资源保护
     */

    vPrintString("正在创建互斥信号量...\r\n");
    xMutex = xSemaphoreCreateMutex();

    if(xMutex == NULL)
    {
        vPrintString("错误：互斥信号量创建失败！\r\n");
        for(;;);
    }

    vPrintString("互斥信号量创建成功！\r\n");
    vPrintString("\r\n");

    /* ==============================================
     *       创建任务
     * ==============================================
     *
     * 注意：
     * - Task3和Task4使用互斥锁，优先级设为相同
     * - Task1和Task2演示二值信号量同步
     * - Monitor任务优先级稍低
     */

    vPrintString("正在创建任务...\r\n");

    /* Task1: 触发同步事件（优先级2） */
    xTaskCreate(vTask1, "Task1", 128, NULL, 2, NULL);

    /* Task2: 等待同步事件（优先级2） */
    xTaskCreate(vTask2, "Task2", 128, NULL, 2, NULL);

    /* Task3: 修改共享资源A（优先级3） */
    xTaskCreate(vTask3, "Task3", 128, NULL, 3, NULL);

    /* Task4: 修改共享资源B（优先级3） */
    xTaskCreate(vTask4, "Task4", 128, NULL, 3, NULL);

    /* Monitor: 监控资源状态（优先级1） */
    xTaskCreate(vMonitorTask, "Monitor", 128, NULL, 1, NULL);

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