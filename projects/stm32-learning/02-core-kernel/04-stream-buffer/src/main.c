/**
 * @file main.c
 * @brief FreeRTOS流缓冲区（Stream Buffer）学习项目
 *
 * 本项目演示FreeRTOS流缓冲区的使用方法。
 * 学习要点：
 * 1. 流缓冲区的概念和工作原理
 * 2. 流缓冲区与队列的区别
 * 3. 水线（Trigger Level）机制
 * 4. xStreamBufferCreate() / xStreamBufferSend() / xStreamBufferReceive() API
 * 5. 生产者-消费者模型
 * 6. 流缓冲区状态查询
 *
 * 参考资料：
 * - FreeRTOS官方文档: https://www.freertos.org/RTOS-stream-buffers.html
 * - FreeRTOS Demo项目: Common/Minimal/StreamBufferDemo.c
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* FreeRTOS 头文件 */
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"

/* 平台相关的串口输出函数声明 */
void vUARTInit(void);
void vPrintString(const char *pcString);
void vPrintNumber(uint32_t ulNumber);

/* ==============================================
 *              宏定义
 * ==============================================
 */

/* 流缓冲区大小 - 用于基础示例 */
#define sbSTREAM_BUFFER_SIZE_BASIC       64

/* 流缓冲区大小 - 用于大数据传输测试 */
#define sbSTREAM_BUFFER_SIZE_LARGE       256

/* 水线（Trigger Level）设置 */
#define sbTRIGGER_LEVEL_1                1   /* 1字节触发 */
#define sbTRIGGER_LEVEL_8                8   /* 8字节触发 */
#define sbTRIGGER_LEVEL_16               16  /* 16字节触发 */

/* 任务优先级 */
#define tskPRIORITY_PRODUCER             2
#define tskPRIORITY_CONSUMER             2
#define tskPRIORITY_MONITOR              1

/* 任务堆栈大小 */
#define tskSTACK_SIZE_BASIC               256
#define tskSTACK_SIZE_LARGE               384

/* 延时时间（毫秒） */
#define DELAY_SHORT_MS                    500
#define DELAY_MEDIUM_MS                   1000
#define DELAY_LONG_MS                     2000

/* ==============================================
 *              全局变量
 * ==============================================
 */

/* 流缓冲区句柄 */
static StreamBufferHandle_t xStreamBufferBasic = NULL;
static StreamBufferHandle_t xStreamBufferLarge = NULL;

/* 测试数据字符串 */
static const char *pcTestString1 = "Hello FreeRTOS Stream Buffer!";
static const char *pcTestString2 = "This is a longer test string to demonstrate stream buffer operations.";

/* ==============================================
 *              辅助函数
 * ==============================================
 */

/**
 * @brief 打印流缓冲区状态
 * @param xStreamBuffer 流缓冲区句柄
 * @param pcName 缓冲区名称
 */
void vPrintStreamBufferStatus(StreamBufferHandle_t xStreamBuffer, const char *pcName)
{
    size_t xSpacesAvailable, xBytesAvailable;
    BaseType_t xIsEmpty, xIsFull;

    if(xStreamBuffer == NULL)
    {
        return;
    }

    xSpacesAvailable = xStreamBufferSpacesAvailable(xStreamBuffer);
    xBytesAvailable = xStreamBufferBytesAvailable(xStreamBuffer);
    xIsEmpty = xStreamBufferIsEmpty(xStreamBuffer);
    xIsFull = xStreamBufferIsFull(xStreamBuffer);

    vPrintString("\r\n---------- ");
    vPrintString(pcName);
    vPrintString(" 状态 ----------\r\n");
    vPrintString("可用空间: ");
    vPrintNumber(xSpacesAvailable);
    vPrintString(" 字节\r\n");
    vPrintString("已用数据: ");
    vPrintNumber(xBytesAvailable);
    vPrintString(" 字节\r\n");
    vPrintString("是否为空: ");
    vPrintString(xIsEmpty == pdTRUE ? "是" : "否");
    vPrintString("\r\n");
    vPrintString("是否已满: ");
    vPrintString(xIsFull == pdTRUE ? "是" : "否");
    vPrintString("\r\n");
    vPrintString("----------------------------------------\r\n\r\n");
}

/* ==============================================
 *              任务函数定义
 * ==============================================
 */

/**
 * @brief 生产者任务 - 向流缓冲区发送数据
 * @param pvParameters 任务参数
 */
void vProducerTask(void *pvParameters)
{
    (void)pvParameters;
    size_t xBytesSent;
    uint32_t ulCounter = 0;
    const char *pcMsgPrefix = "Msg#";

    vPrintString("[Producer] 生产者任务启动！\r\n");

    for(;;)
    {
        ulCounter++;

        vPrintString("[Producer] 发送: ");
        vPrintString(pcMsgPrefix);
        vPrintNumber(ulCounter);
        vPrintString(": \r\n");

        /* 发送数据到流缓冲区 - 发送固定消息 */
        xBytesSent = xStreamBufferSend(
            xStreamBufferBasic,
            pcTestString1,
            strlen(pcTestString1),
            pdMS_TO_TICKS(100)
        );

        if(xBytesSent > 0)
        {
            vPrintString("[Producer] ✓ 成功发送 ");
            vPrintNumber(xBytesSent);
            vPrintString(" 字节\r\n");
        }
        else
        {
            vPrintString("[Producer] ✗ 发送失败（缓冲区可能已满）\r\n");
        }

        /* 打印状态 */
        vPrintStreamBufferStatus(xStreamBufferBasic, "基础缓冲区");

        vTaskDelay(pdMS_TO_TICKS(DELAY_MEDIUM_MS));
    }
}

/**
 * @brief 消费者任务 - 从流缓冲区接收数据
 * @param pvParameters 任务参数
 */
void vConsumerTask(void *pvParameters)
{
    (void)pvParameters;
    size_t xBytesReceived;
    char cReceiveBuffer[64];

    vPrintString("[Consumer] 消费者任务启动！\r\n");

    for(;;)
    {
        /* 从流缓冲区接收数据 */
        xBytesReceived = xStreamBufferReceive(
            xStreamBufferBasic,
            cReceiveBuffer,
            sizeof(cReceiveBuffer) - 1,
            pdMS_TO_TICKS(DELAY_LONG_MS)
        );

        if(xBytesReceived > 0)
        {
            cReceiveBuffer[xBytesReceived] = '\0';
            vPrintString("[Consumer] ✓ 接收到: ");
            vPrintString(cReceiveBuffer);
            vPrintString(" (");
            vPrintNumber(xBytesReceived);
            vPrintString(" 字节)\r\n");
        }
        else
        {
            vPrintString("[Consumer] 等待数据...\r\n");
        }

        /* 打印状态 */
        vPrintStreamBufferStatus(xStreamBufferBasic, "基础缓冲区");

        vTaskDelay(pdMS_TO_TICKS(DELAY_MEDIUM_MS));
    }
}

/**
 * @brief 监控任务 - 定期打印流缓冲区状态
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

        vPrintStreamBufferStatus(xStreamBufferBasic, "基础缓冲区");
        if(xStreamBufferLarge != NULL)
        {
            vPrintStreamBufferStatus(xStreamBufferLarge, "大数据缓冲区");
        }

        vTaskDelay(pdMS_TO_TICKS(DELAY_LONG_MS));
    }
}

/**
 * @brief 流缓冲区演示任务 - 展示各种流缓冲区操作
 * @param pvParameters 任务参数
 */
void vStreamBufferDemoTask(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulStep = 0;
    size_t xBytesSent, xBytesReceived;
    char cBuffer[128];

    vPrintString("[Demo] 流缓冲区演示任务启动！\r\n");

    vPrintString("\r\n[Demo] ========== 步骤1: 创建大数据流缓冲区 ==========\r\n");
    xStreamBufferLarge = xStreamBufferCreate(sbSTREAM_BUFFER_SIZE_LARGE, sbTRIGGER_LEVEL_8);
    
    if(xStreamBufferLarge != NULL)
    {
        vPrintString("[Demo] ✓ 大数据流缓冲区创建成功！\r\n");
        vPrintString("[Demo] 大小: ");
        vPrintNumber(sbSTREAM_BUFFER_SIZE_LARGE);
        vPrintString(" 字节, 水线: ");
        vPrintNumber(sbTRIGGER_LEVEL_8);
        vPrintString(" 字节\r\n");
    }
    else
    {
        vPrintString("[Demo] ✗ 缓冲区创建失败！\r\n");
    }

    vPrintStreamBufferStatus(xStreamBufferLarge, "大数据缓冲区");
    vTaskDelay(pdMS_TO_TICKS(DELAY_MEDIUM_MS));

    for(;;)
    {
        ulStep++;

        vPrintString("\r\n[Demo] ========== 步骤");
        vPrintNumber(ulStep + 1);
        vPrintString(" ==========\r\n");

        switch(ulStep)
        {
            case 1:
                vPrintString("[Demo] 发送短字符串到大数据缓冲区...\r\n");
                xBytesSent = xStreamBufferSend(
                    xStreamBufferLarge,
                    pcTestString1,
                    strlen(pcTestString1),
                    0
                );
                
                if(xBytesSent > 0)
                {
                    vPrintString("[Demo] ✓ 发送成功: ");
                    vPrintString(pcTestString1);
                    vPrintString("\r\n");
                }
                break;

            case 2:
                vPrintString("[Demo] 接收并打印数据...\r\n");
                xBytesReceived = xStreamBufferReceive(
                    xStreamBufferLarge,
                    cBuffer,
                    sizeof(cBuffer) - 1,
                    0
                );
                
                if(xBytesReceived > 0)
                {
                    cBuffer[xBytesReceived] = '\0';
                    vPrintString("[Demo] ✓ 接收到: ");
                    vPrintString(cBuffer);
                    vPrintString("\r\n");
                }
                break;

            case 3:
                vPrintString("[Demo] 测试水线机制 - 先发送少量数据...\r\n");
                xBytesSent = xStreamBufferSend(
                    xStreamBufferLarge,
                    "12345",
                    5,
                    0
                );
                vPrintString("[Demo] 已发送 5 字节（水线设置为 8 字节）\r\n");
                vPrintString("[Demo] 尝试接收（应该超时，因为未达到水线）...\r\n");
                
                xBytesReceived = xStreamBufferReceive(
                    xStreamBufferLarge,
                    cBuffer,
                    sizeof(cBuffer) - 1,
                    pdMS_TO_TICKS(500)
                );
                
                if(xBytesReceived == 0)
                {
                    vPrintString("[Demo] ✓ 接收超时（符合预期 - 未达到水线）\r\n");
                }
                break;

            case 4:
                vPrintString("[Demo] 再发送一些数据以达到水线...\r\n");
                xBytesSent = xStreamBufferSend(
                    xStreamBufferLarge,
                    "67890",
                    5,
                    0
                );
                vPrintString("[Demo] 现在缓冲区有 10 字节（超过水线 8 字节）\r\n");
                vPrintString("[Demo] 再次接收...\r\n");
                
                xBytesReceived = xStreamBufferReceive(
                    xStreamBufferLarge,
                    cBuffer,
                    sizeof(cBuffer) - 1,
                    0
                );
                
                if(xBytesReceived > 0)
                {
                    cBuffer[xBytesReceived] = '\0';
                    vPrintString("[Demo] ✓ 成功接收: ");
                    vPrintString(cBuffer);
                    vPrintString("\r\n");
                }
                break;

            case 5:
                vPrintString("[Demo] 发送长字符串测试...\r\n");
                xBytesSent = xStreamBufferSend(
                    xStreamBufferLarge,
                    pcTestString2,
                    strlen(pcTestString2),
                    0
                );
                
                if(xBytesSent > 0)
                {
                    vPrintString("[Demo] ✓ 发送了 ");
                    vPrintNumber(xBytesSent);
                    vPrintString(" 字节\r\n");
                }
                break;

            case 6:
                vPrintString("[Demo] 分块接收长字符串...\r\n");
                do
                {
                    xBytesReceived = xStreamBufferReceive(
                        xStreamBufferLarge,
                        cBuffer,
                        16,
                        0
                    );
                    
                    if(xBytesReceived > 0)
                    {
                        cBuffer[xBytesReceived] = '\0';
                        vPrintString("[Demo] 块接收: ");
                        vPrintString(cBuffer);
                        vPrintString("\r\n");
                    }
                } while(xBytesReceived > 0);
                break;

            case 7:
                vPrintString("[Demo] 演示缓冲区满的情况...\r\n");
                vPrintString("[Demo] 快速填充缓冲区...\r\n");
                
                for(int i = 0; i < 10; i++)
                {
                    xBytesSent = xStreamBufferSend(
                        xStreamBufferLarge,
                        "ABCDEFGHIJ",
                        10,
                        0
                    );
                    
                    if(xBytesSent == 0)
                    {
                        vPrintString("[Demo] ✓ 缓冲区已满（符合预期）\r\n");
                        break;
                    }
                }
                break;

            case 8:
                vPrintString("[Demo] 清空缓冲区...\r\n");
                xStreamBufferReset(xStreamBufferLarge);
                vPrintString("[Demo] ✓ 缓冲区已重置！\r\n");
                break;

            default:
                vPrintString("[Demo] 演示完成！进入空闲循环\r\n");
                for(;;)
                {
                    vTaskDelay(pdMS_TO_TICKS(10000));
                }
        }

        vPrintStreamBufferStatus(xStreamBufferLarge, "大数据缓冲区");
        vTaskDelay(pdMS_TO_TICKS(DELAY_LONG_MS));

        if(ulStep >= 9)
        {
            vPrintString("[Demo] 演示结束，进入空闲循环\r\n");
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
    vPrintString("  FreeRTOS 流缓冲区学习示例\r\n");
    vPrintString("====================================\r\n");
    vPrintString("\r\n");

    vPrintString("学习要点：\r\n");
    vPrintString("1. 流缓冲区的概念和工作原理\r\n");
    vPrintString("2. 流缓冲区与队列的区别\r\n");
    vPrintString("3. 水线（Trigger Level）机制\r\n");
    vPrintString("4. xStreamBufferCreate() / xStreamBufferSend() / xStreamBufferReceive()\r\n");
    vPrintString("5. 生产者-消费者模型\r\n");
    vPrintString("6. 流缓冲区状态查询\r\n");
    vPrintString("\r\n");

    vPrintString("====================================\r\n");
    vPrintString("  流缓冲区 vs 队列：\r\n");
    vPrintString("====================================\r\n");
    vPrintString("特性              队列          流缓冲区\r\n");
    vPrintString("----------------------------------------\r\n");
    vPrintString("数据结构          离散项        字节流\r\n");
    vPrintString("收发单位          固定大小      可变大小\r\n");
    vPrintString("水线机制          ✗ 无         ✓ 有\r\n");
    vPrintString("适用场景          结构化数据    流数据\r\n");
    vPrintString("中断安全          ✓ 是         ✓ 是\r\n");
    vPrintString("====================================\r\n");
    vPrintString("\r\n");

    vPrintString("正在创建基础流缓冲区...\r\n");
    xStreamBufferBasic = xStreamBufferCreate(sbSTREAM_BUFFER_SIZE_BASIC, sbTRIGGER_LEVEL_1);
    
    if(xStreamBufferBasic != NULL)
    {
        vPrintString("✓ 基础流缓冲区创建成功！\r\n");
        vPrintString("大小: ");
        vPrintNumber(sbSTREAM_BUFFER_SIZE_BASIC);
        vPrintString(" 字节, 水线: ");
        vPrintNumber(sbTRIGGER_LEVEL_1);
        vPrintString(" 字节\r\n");
    }
    else
    {
        vPrintString("✗ 缓冲区创建失败！\r\n");
        for(;;);
    }

    vPrintString("\r\n正在创建任务...\r\n");

    xTaskCreate(vProducerTask, "Producer", tskSTACK_SIZE_BASIC, NULL, tskPRIORITY_PRODUCER, NULL);
    xTaskCreate(vConsumerTask, "Consumer", tskSTACK_SIZE_BASIC, NULL, tskPRIORITY_CONSUMER, NULL);
    xTaskCreate(vMonitorTask, "Monitor", tskSTACK_SIZE_BASIC, NULL, tskPRIORITY_MONITOR, NULL);
    xTaskCreate(vStreamBufferDemoTask, "DemoTask", tskSTACK_SIZE_LARGE, NULL, tskPRIORITY_MONITOR + 1, NULL);

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
