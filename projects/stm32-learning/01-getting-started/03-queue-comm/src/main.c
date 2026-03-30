/**
 * @file main.c
 * @brief FreeRTOS队列通信示例
 *
 * 本项目演示如何使用FreeRTOS队列进行任务间通信。
 * 学习要点：
 * 1. 创建队列
 * 2. 发送数据到队列
 * 3. 从队列接收数据
 * 4. 使用队列进行任务同步
 *
 * 参考资料：
 * - FreeRTOS官方文档: https://www.freertos.org/a00018.html
 * - Mastering-the-FreeRTOS-Kernel 第5章
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* FreeRTOS 头文件 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* 平台相关的串口输出函数声明 */
void vUARTInit(void);
void vPrintString(const char *pcString);
void vPrintNumber(uint32_t ulNumber);

/* ==============================================
 *              队列句柄（全局变量）
 * ==============================================
 *
 * 队列句柄需要在多个任务中访问，所以声明为全局变量。
 */

/* 队列1：传输整数数据 */
QueueHandle_t xIntegerQueue = NULL;

/* 队列2：传输字符串数据 */
QueueHandle_t xStringQueue = NULL;

/* ==============================================
 *              任务函数定义
 * ==============================================
 *
 * 本项目创建4个任务：
 * - SenderTask1: 发送整数到队列
 * - SenderTask2: 发送字符串到队列
 * - ReceiverTask1: 接收整数数据
 * - ReceiverTask2: 接收字符串数据
 */

/**
 * @brief 发送者任务1 - 发送整数
 * @param pvParameters 任务参数
 *
 * 这个任务定期生成递增的整数，并发送到整数队列。
 */
void vIntegerSenderTask(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulCounter = 0;
    BaseType_t xStatus;

    vPrintString("[Sender1] 整数发送任务启动！\r\n");

    for(;;)
    {
        /* 生成要发送的数据 */
        ulCounter++;

        vPrintString("[Sender1] 准备发送: ");
        vPrintNumber(ulCounter);
        vPrintString("\r\n");

        /* ==============================================
         *       学习重点1：发送数据到队列
         * ==============================================
         *
         * BaseType_t xQueueSend(
         *     QueueHandle_t xQueue,           // 队列句柄
         *     const void * const pvItemToQueue, // 要发送的数据指针
         *     TickType_t xTicksToWait         // 等待时间（如果队列满）
         * );
         *
         * 参数说明：
         * 1. xQueue: 目标队列的句柄
         * 2. pvItemToQueue: 指向要发送的数据的指针
         * 3. xTicksToWait: 如果队列满，等待的时间（0=不等待，portMAX_DELAY=无限等待）
         *
         * 返回值：
         * - pdPASS: 发送成功
         * - errQUEUE_FULL: 队列满，发送失败
         */

        /* 发送数据到队列，等待最多10个时钟节拍 */
        xStatus = xQueueSend(
            xIntegerQueue,          /* 队列句柄 */
            &ulCounter,             /* 指向要发送的数据 */
            pdMS_TO_TICKS(10)       /* 等待时间：10ms */
        );

        if(xStatus != pdPASS)
        {
            vPrintString("[Sender1] 错误：队列满，发送失败！\r\n");
        }
        else
        {
            vPrintString("[Sender1] 发送成功！\r\n");
        }

        /* 延时200ms */
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

/**
 * @brief 发送者任务2 - 发送字符串
 * @param pvParameters 任务参数
 *
 * 这个任务定期发送字符串消息到字符串队列。
 */
void vStringSenderTask(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulCounter = 0;
    BaseType_t xStatus;
    char cMessage[32];

    vPrintString("[Sender2] 字符串发送任务启动！\r\n");

    for(;;)
    {
        /* 生成消息字符串 */
        ulCounter++;
        strcpy(cMessage, "Hello #");

        /* 把计数器转换成字符串（简化版） */
        if(ulCounter < 10)
        {
            cMessage[7] = '0' + ulCounter;
            cMessage[8] = '\0';
        }
        else
        {
            cMessage[7] = '0' + (ulCounter / 10);
            cMessage[8] = '0' + (ulCounter % 10);
            cMessage[9] = '\0';
        }

        vPrintString("[Sender2] 准备发送: ");
        vPrintString(cMessage);
        vPrintString("\r\n");

        /* 发送字符串到队列 */
        xStatus = xQueueSend(
            xStringQueue,
            cMessage,
            pdMS_TO_TICKS(10)
        );

        if(xStatus != pdPASS)
        {
            vPrintString("[Sender2] 错误：队列满，发送失败！\r\n");
        }
        else
        {
            vPrintString("[Sender2] 发送成功！\r\n");
        }

        /* 延时300ms */
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}

/**
 * @brief 接收者任务1 - 接收整数
 * @param pvParameters 任务参数
 *
 * 这个任务从整数队列接收数据并处理。
 */
void vIntegerReceiverTask(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulReceivedValue;
    BaseType_t xStatus;

    vPrintString("[Receiver1] 整数接收任务启动！\r\n");

    for(;;)
    {
        /* ==============================================
         *       学习重点2：从队列接收数据
         * ==============================================
         *
         * BaseType_t xQueueReceive(
         *     QueueHandle_t xQueue,      // 队列句柄
         *     void * const pvBuffer,      // 接收数据的缓冲区
         *     TickType_t xTicksToWait    // 等待时间（如果队列空）
         * );
         *
         * 参数说明：
         * 1. xQueue: 要读取的队列句柄
         * 2. pvBuffer: 用于存储接收数据的缓冲区
         * 3. xTicksToWait: 如果队列空，等待的时间
         *
         * 返回值：
         * - pdPASS: 接收成功
         * - errQUEUE_EMPTY: 队列空，接收失败
         *
         * 注意：xQueueReceive() 会从队列中移除数据！
         * 如果只想查看数据不移除，使用 xQueuePeek()
         */

        /* 等待队列中的数据，最多等待100ms */
        xStatus = xQueueReceive(
            xIntegerQueue,          /* 队列句柄 */
            &ulReceivedValue,        /* 接收缓冲区 */
            pdMS_TO_TICKS(100)       /* 等待时间：100ms */
        );

        if(xStatus == pdPASS)
        {
            vPrintString("[Receiver1] 收到数据: ");
            vPrintNumber(ulReceivedValue);
            vPrintString("\r\n");

            /* 可以在这里处理接收到的数据 */
            uint32_t ulResult = ulReceivedValue * 2;
            vPrintString("[Receiver1] 处理结果: ");
            vPrintNumber(ulResult);
            vPrintString("\r\n");
        }
        else
        {
            vPrintString("[Receiver1] 等待数据超时...\r\n");
        }
    }
}

/**
 * @brief 接收者任务2 - 接收字符串
 * @param pvParameters 任务参数
 *
 * 这个任务从字符串队列接收消息。
 */
void vStringReceiverTask(void *pvParameters)
{
    (void)pvParameters;
    char cReceivedMessage[32];
    BaseType_t xStatus;

    vPrintString("[Receiver2] 字符串接收任务启动！\r\n");

    for(;;)
    {
        /* 接收字符串数据 */
        xStatus = xQueueReceive(
            xStringQueue,
            cReceivedMessage,
            pdMS_TO_TICKS(100)
        );

        if(xStatus == pdPASS)
        {
            vPrintString("[Receiver2] 收到消息: ");
            vPrintString(cReceivedMessage);
            vPrintString("\r\n");
        }
        else
        {
            vPrintString("[Receiver2] 等待消息超时...\r\n");
        }
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
    vPrintString("  FreeRTOS队列通信示例\r\n");
    vPrintString("====================================\r\n");
    vPrintString("\r\n");

    vPrintString("学习要点：\r\n");
    vPrintString("1. 创建队列 xQueueCreate()\r\n");
    vPrintString("2. 发送数据 xQueueSend()\r\n");
    vPrintString("3. 接收数据 xQueueReceive()\r\n");
    vPrintString("4. 任务间通信与同步\r\n");
    vPrintString("\r\n");

    /* ==============================================
     *       学习重点3：创建队列
     * ==============================================
     *
     * QueueHandle_t xQueueCreate(
     *     UBaseType_t uxQueueLength,  // 队列长度（能存储多少个数据项）
     *     UBaseType_t uxItemSize       // 每个数据项的大小（字节）
     * );
     *
     * 参数说明：
     * 1. uxQueueLength: 队列最多能存储多少个数据
     * 2. uxItemSize: 每个数据的大小（使用 sizeof() 获取）
     *
     * 返回值：
     * - 成功：返回队列句柄
     * - 失败：返回NULL（内存不足）
     */

    vPrintString("正在创建队列...\r\n");

    /* 创建整数队列：长度5，每个数据是uint32_t */
    xIntegerQueue = xQueueCreate(
        5,                      /* 队列长度：5个数据项 */
        sizeof(uint32_t)        /* 每个数据项的大小 */
    );

    /* 创建字符串队列：长度3，每个数据是32字节的字符串 */
    xStringQueue = xQueueCreate(
        3,                      /* 队列长度：3个数据项 */
        32                      /* 每个数据项的大小：32字节 */
    );

    /* 检查队列是否创建成功 */
    if((xIntegerQueue == NULL) || (xStringQueue == NULL))
    {
        vPrintString("错误：队列创建失败！（内存不足？）\r\n");
        for(;;);
    }

    vPrintString("队列创建成功！\r\n");
    vPrintString("\r\n");

    /* ==============================================
     *       创建任务
     * ==============================================
     *
     * 注意：
     * - 接收任务优先级稍高，确保能及时处理数据
     * - 发送任务优先级稍低
     */

    vPrintString("正在创建任务...\r\n");

    /* 创建整数发送任务（优先级1） */
    xTaskCreate(vIntegerSenderTask, "IntSender", 128, NULL, 1, NULL);

    /* 创建字符串发送任务（优先级1） */
    xTaskCreate(vStringSenderTask, "StrSender", 128, NULL, 1, NULL);

    /* 创建整数接收任务（优先级2 - 稍高） */
    xTaskCreate(vIntegerReceiverTask, "IntReceiver", 128, NULL, 2, NULL);

    /* 创建字符串接收任务（优先级2 - 稍高） */
    xTaskCreate(vStringReceiverTask, "StrReceiver", 128, NULL, 2, NULL);

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
