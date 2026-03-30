/**
 * @file main.c
 * @brief FreeRTOS任务通知（Task Notifications）学习项目
 *
 * 本项目演示FreeRTOS任务通知的使用方法。
 * 学习要点：
 * 1. 任务通知的概念和工作原理
 * 2. 二值通知 - 替代二值信号量
 * 3. 计数通知 - 替代计数信号量
 * 4. 邮箱通知 - 替代轻量级队列
 * 5. xTaskNotifyGive() / ulTaskNotifyTake() API
 * 6. xTaskNotify() / xTaskNotifyWait() API
 *
 * 参考资料：
 * - FreeRTOS官方文档: https://www.freertos.org/RTOS-task-notifications.html
 * - FreeRTOS Demo项目: Common/Minimal/TaskNotify.c
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
 *              宏定义
 * ==============================================
 */

/* 任务优先级 */
#define tskPRIORITY_NOTIFIED           3
#define tskPRIORITY_NOTIFIER           2

/* 任务堆栈大小 */
#define tskSTACK_SIZE_BASIC           256

/* 延时时间（毫秒） */
#define DELAY_SHORT_MS                500
#define DELAY_MEDIUM_MS               1000

/* ==============================================
 *              全局变量
 * ==============================================
 */

/* 任务句柄 */
static TaskHandle_t xNotifiedTask = NULL;

/* ==============================================
 *              辅助函数
 * ==============================================
 */

void vPrintSeparator(void)
{
    vPrintString("\r\n========================================\r\n");
}

/* ==============================================
 *           测试1：二值通知（替代二值信号量）
 * ==============================================
 */

static void vNotifiedTask(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulNotificationCount;

    vPrintString("  [任务通知] 等待通知...\r\n");

    for(;;)
    {
        /* 等待通知 - pdTRUE表示清零通知值（类似二值信号量） */
        ulNotificationCount = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        vPrintString("  [任务通知] 收到通知！计数: ");
        vPrintNumber(ulNotificationCount);
        vPrintString("\r\n");
    }
}

static void vNotifierTask(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulCount = 0;

    vTaskDelay(pdMS_TO_TICKS(DELAY_SHORT_MS));

    for(;;)
    {
        ulCount++;
        vPrintString("\r\n  [任务通知] 发送通知 #");
        vPrintNumber(ulCount);
        vPrintString("\r\n");

        /* 发送通知 - 类似给出二值信号量 */
        xTaskNotifyGive(xNotifiedTask);

        vTaskDelay(pdMS_TO_TICKS(DELAY_MEDIUM_MS));

        if(ulCount >= 5)
        {
            vPrintString("\r\n  [任务通知] 测试完成，程序继续运行...\r\n");
            break;
        }
    }

    vTaskDelete(NULL);
}

/* ==============================================
 *              主函数
 * ==============================================
 */

int main(void)
{
    vUARTInit();

    vPrintString("\r\n");
    vPrintSeparator();
    vPrintString("  FreeRTOS 任务通知学习示例\r\n");
    vPrintSeparator();
    vPrintString("\r\n");

    vPrintString("学习要点：\r\n");
    vPrintString("1. 任务通知的概念和工作原理\r\n");
    vPrintString("2. xTaskNotifyGive() - 发送通知\r\n");
    vPrintString("3. ulTaskNotifyTake() - 等待通知\r\n");
    vPrintString("4. 任务通知 vs 二值信号量\r\n");
    vPrintString("\r\n");

    vPrintSeparator();
    vPrintString("  任务通知优势：\r\n");
    vPrintSeparator();
    vPrintString("- 更快的执行速度\r\n");
    vPrintString("- 更少的内存占用\r\n");
    vPrintString("- 更简单的API使用\r\n");
    vPrintString("\r\n");

    vPrintString("正在创建任务...\r\n");

    /* 创建接收通知的任务 */
    xTaskCreate(vNotifiedTask, "Notified", tskSTACK_SIZE_BASIC,
                NULL, tskPRIORITY_NOTIFIED, &xNotifiedTask);

    /* 创建发送通知的任务 */
    xTaskCreate(vNotifierTask, "Notifier", tskSTACK_SIZE_BASIC,
                NULL, tskPRIORITY_NOTIFIER, NULL);

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
