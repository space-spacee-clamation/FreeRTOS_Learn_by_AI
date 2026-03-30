/**
 * @file main.c
 * @brief FreeRTOS中断管理（Interrupt Management）学习项目
 *
 * 本项目演示FreeRTOS中断管理的使用方法。
 * 学习要点：
 * 1. 中断优先级配置
 * 2. ISR安全API的使用
 * 3. 二值信号量同步（任务-ISR）
 * 4. 队列ISR通信
 * 5. 延迟中断处理（deferred interrupt）
 * 6. 使用任务模拟ISR机制
 *
 * 参考资料：
 * - FreeRTOS官方文档: https://www.freertos.org/RTOS-Cortex-M3-M4.html
 * - FreeRTOS官方文档: https://www.freertos.org/FAQISR.html
 * - Mastering the FreeRTOS Kernel - 第7章
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* FreeRTOS 头文件 */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

/* 平台相关的串口输出函数声明 */
void vUARTInit(void);
void vPrintString(const char *pcString);
void vPrintNumber(uint32_t ulNumber);

/* ==============================================
 *              宏定义
 * ==============================================
 */

/* 任务优先级 */
#define tskPRIORITY_HANDLER_TASK        4   /* 延迟中断处理任务 - 高优先级 */
#define tskPRIORITY_ISR_SIMULATOR        3   /* ISR模拟任务 */
#define tskPRIORITY_WORKER_TASK          2   /* 工作任务 */
#define tskPRIORITY_LOW                   1   /* 低优先级任务 */

/* 任务堆栈大小 */
#define tskSTACK_SIZE_BASIC              256

/* 延时时间（毫秒） */
#define DELAY_SHORT_MS                   200
#define DELAY_MEDIUM_MS                  500
#define DELAY_LONG_MS                    1000

/* 队列长度 */
#define QUEUE_LENGTH                     5
#define ITEM_SIZE                        sizeof(uint32_t)

/* ==============================================
 *              全局变量
 * ==============================================
 */

/* 二值信号量 - 用于ISR-任务同步 */
static SemaphoreHandle_t xBinarySemaphore = NULL;

/* 队列 - 用于ISR向任务发送数据 */
static QueueHandle_t xISRQueue = NULL;

/* 计数信号量 - 用于统计中断次数 */
static SemaphoreHandle_t xCountingSemaphore = NULL;

/* 任务句柄 */
static TaskHandle_t xHandlerTask = NULL;

/* ==============================================
 *              辅助函数
 * ==============================================
 */

void vPrintSeparator(void)
{
    vPrintString("\r\n========================================\r\n");
}

/* ==============================================
 *      测试1：二值信号量 - ISR到任务同步
 * ==============================================
 */

/**
 * @brief ISR模拟器任务 - 模拟中断触发
 *
 * 这个任务模拟硬件中断的行为，定期"触发中断"
 * 并使用ISR安全的API与其他任务通信
 */
static void vISRSimulatorTask(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulInterruptCount = 0;
    BaseType_t xHigherPriorityTaskWoken;

    vPrintString("  [ISR模拟器] 任务启动，准备模拟中断...\r\n");
    vTaskDelay(pdMS_TO_TICKS(DELAY_MEDIUM_MS));

    for(;;)
    {
        ulInterruptCount++;
        vPrintString("\r\n  [ISR模拟器] 模拟中断触发 #");
        vPrintNumber(ulInterruptCount);
        vPrintString("\r\n");

        /* ==============================================
         * 学习重点：在ISR中使用FreeRTOS API
         *
         * 注意：在真实的ISR中，必须使用FromISR版本的API！
         * 1. xSemaphoreGiveFromISR() - 而不是 xSemaphoreGive()
         * 2. xQueueSendFromISR() - 而不是 xQueueSend()
         * 3. portYIELD_FROM_ISR() - 用于请求上下文切换
         * ==============================================
         */

        xHigherPriorityTaskWoken = pdFALSE;

        /* 1. 给出二值信号量 - 同步延迟处理任务 */
        if(xBinarySemaphore != NULL)
        {
            vPrintString("  [ISR] 给出二值信号量...\r\n");
            xSemaphoreGiveFromISR(xBinarySemaphore, &xHigherPriorityTaskWoken);
        }

        /* 2. 发送数据到队列 */
        if(xISRQueue != NULL)
        {
            vPrintString("  [ISR] 发送数据到队列: ");
            vPrintNumber(ulInterruptCount);
            vPrintString("\r\n");
            xQueueSendFromISR(xISRQueue, &ulInterruptCount, &xHigherPriorityTaskWoken);
        }

        /* 3. 给出计数信号量 - 统计中断次数 */
        if(xCountingSemaphore != NULL)
        {
            xSemaphoreGiveFromISR(xCountingSemaphore, &xHigherPriorityTaskWoken);
        }

        /* 注意：这里我们用任务模拟ISR，所以不需要portYIELD_FROM_ISR()
         * 在真实的ISR中才需要使用portYIELD_FROM_ISR() */

        /* 模拟下一次中断前的延时 */
        vTaskDelay(pdMS_TO_TICKS(DELAY_LONG_MS));

        if(ulInterruptCount >= 5)
        {
            vPrintString("\r\n  [ISR模拟器] 中断模拟完成，程序继续运行...\r\n");
            break;
        }
    }

    vTaskDelete(NULL);
}

/**
 * @brief 延迟中断处理任务
 *
 * 这个任务接收来自ISR的信号量，执行"中断下半部"处理
 * 这种模式称为"延迟中断处理"或"deferred interrupt processing"
 */
static void vHandlerTask(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulReceivedValue;
    uint32_t ulInterruptCount;

    vPrintString("  [延迟处理] 任务启动，等待中断...\r\n");

    for(;;)
    {
        /* ==============================================
         * 学习重点：等待二值信号量
         *
         * 这里使用无限期等待（portMAX_DELAY）
         * 当ISR给出信号量时，这个任务会被立即唤醒
         * ==============================================
         */
        if(xSemaphoreTake(xBinarySemaphore, portMAX_DELAY) == pdTRUE)
        {
            vPrintString("\r\n  [延迟处理] 收到信号量，开始处理中断...\r\n");

            /* 从队列接收数据 */
            if(xQueueReceive(xISRQueue, &ulReceivedValue, 0) == pdTRUE)
            {
                vPrintString("  [延迟处理] 从队列收到数据: ");
                vPrintNumber(ulReceivedValue);
                vPrintString("\r\n");
            }

            /* 从计数信号量获取中断统计 */
            ulInterruptCount = uxSemaphoreGetCount(xCountingSemaphore);
            vPrintString("  [延迟处理] 当前累计中断数: ");
            vPrintNumber(ulInterruptCount);
            vPrintString("\r\n");

            /* 模拟中断处理工作 */
            vPrintString("  [延迟处理] 中断处理完成！\r\n");
        }
    }
}

/**
 * @brief 工作任务 - 低优先级后台任务
 */
static void vWorkerTask(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulLoopCount = 0;

    vPrintString("  [工作任务] 后台任务启动...\r\n");

    for(;;)
    {
        ulLoopCount++;

        /* 每10次循环打印一次 */
        if((ulLoopCount % 10) == 0)
        {
            vPrintString("  [工作任务] 运行中... 循环 #");
            vPrintNumber(ulLoopCount);
            vPrintString("\r\n");
        }

        vTaskDelay(pdMS_TO_TICKS(DELAY_SHORT_MS));
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
    vPrintSeparator();
    vPrintString("  FreeRTOS 中断管理学习示例\r\n");
    vPrintSeparator();
    vPrintString("\r\n");

    vPrintString("学习要点：\r\n");
    vPrintString("1. 中断优先级配置（configMAX_SYSCALL_INTERRUPT_PRIORITY）\r\n");
    vPrintString("2. ISR安全API的使用（FromISR后缀）\r\n");
    vPrintString("3. 二值信号量 - ISR到任务同步\r\n");
    vPrintString("4. 队列 - ISR到任务数据传输\r\n");
    vPrintString("5. 延迟中断处理模式\r\n");
    vPrintString("6. portYIELD_FROM_ISR()的使用\r\n");
    vPrintString("\r\n");

    vPrintSeparator();
    vPrintString("  中断管理关键概念：\r\n");
    vPrintSeparator();
    vPrintString("- ISR要尽可能短，只做必要工作\r\n");
    vPrintString("- 复杂处理延迟到任务中完成\r\n");
    vPrintString("- 使用FromISR版本的API\r\n");
    vPrintString("- 注意中断优先级配置\r\n");
    vPrintString("\r\n");

    vPrintString("正在创建同步对象...\r\n");

    /* ==============================================
     * 学习重点：创建用于ISR通信的对象
     *
     * 1. 二值信号量 - 用于同步
     * 2. 队列 - 用于传递数据
     * 3. 计数信号量 - 用于统计
     * ==============================================
     */

    /* 创建二值信号量 */
    xBinarySemaphore = xSemaphoreCreateBinary();
    if(xBinarySemaphore == NULL)
    {
        vPrintString("错误：创建二值信号量失败！\r\n");
        for(;;);
    }
    vPrintString("  - 二值信号量创建成功\r\n");

    /* 创建队列 */
    xISRQueue = xQueueCreate(QUEUE_LENGTH, ITEM_SIZE);
    if(xISRQueue == NULL)
    {
        vPrintString("错误：创建队列失败！\r\n");
        for(;;);
    }
    vPrintString("  - 队列创建成功\r\n");

    /* 创建计数信号量（最大计数10，初始计数0） */
    xCountingSemaphore = xSemaphoreCreateCounting(10, 0);
    if(xCountingSemaphore == NULL)
    {
        vPrintString("错误：创建计数信号量失败！\r\n");
        for(;;);
    }
    vPrintString("  - 计数信号量创建成功\r\n");

    vPrintString("\r\n正在创建任务...\r\n");

    /* 创建延迟中断处理任务 - 较高优先级 */
    xTaskCreate(vHandlerTask, "Handler", tskSTACK_SIZE_BASIC,
                NULL, tskPRIORITY_HANDLER_TASK, &xHandlerTask);
    vPrintString("  - 延迟处理任务创建（优先级: 4）\r\n");

    /* 创建ISR模拟器任务 */
    xTaskCreate(vISRSimulatorTask, "ISRSim", tskSTACK_SIZE_BASIC,
                NULL, tskPRIORITY_ISR_SIMULATOR, NULL);
    vPrintString("  - ISR模拟器任务创建（优先级: 3）\r\n");

    /* 创建工作任务 - 低优先级 */
    xTaskCreate(vWorkerTask, "Worker", tskSTACK_SIZE_BASIC,
                NULL, tskPRIORITY_WORKER_TASK, NULL);
    vPrintString("  - 工作任务创建（优先级: 2）\r\n");

    vPrintString("\r\n所有任务创建成功！\r\n");
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
