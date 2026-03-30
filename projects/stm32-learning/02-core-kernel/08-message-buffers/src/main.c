/**
 * @file main.c
 * @brief FreeRTOS消息缓冲区（Message Buffers）学习项目
 *
 * 学习要点：
 * 1. 变长消息发送接收
 * 2. 任务间消息传递
 * 3. 中断-任务消息通信（使用任务模拟ISR）
 * 4. 与流缓冲区对比演示
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* FreeRTOS 头文件 */
#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"
#include "stream_buffer.h"

/* 平台相关的串口输出函数声明 */
void vUARTInit(void);
void vPrintString(const char *pcString);
void vPrintNumber(uint32_t ulNumber);
void vPrintChar(char c);

/* 宏定义 */
#define tskPRIORITY_RECEIVER        4
#define tskPRIORITY_SENDER          3
#define tskPRIORITY_ISR_SIM         3
#define tskPRIORITY_WORKER          2
#define tskSTACK_SIZE               256
#define DELAY_SHORT_MS              200
#define DELAY_MEDIUM_MS             500
#define DELAY_LONG_MS               1000
#define MESSAGE_BUFFER_SIZE         200
#define STREAM_BUFFER_SIZE          200

/* 全局变量 */
static MessageBufferHandle_t xTaskMsgBuf = NULL;
static MessageBufferHandle_t xISRMsgBuf = NULL;
static StreamBufferHandle_t xStreamBuf = NULL;

/* 辅助函数 */
void vPrintSeparator(void)
{
    vPrintString("\r\n========================================\r\n");
}

/* 辅助函数：构造ISR消息字符串 */
void vBuildISRMessage(char *pcBuf, size_t uBufSize, uint32_t ulMsgNum)
{
    const char *pcPrefix = "ISR Msg #";
    size_t uPrefixLen = strlen(pcPrefix);
    size_t uIndex = 0;
    char cNumBuf[16];
    int iNumIndex = 0;
    int i;

    /* 复制前缀 */
    while(uIndex < uPrefixLen && uIndex < uBufSize - 1)
    {
        pcBuf[uIndex] = pcPrefix[uIndex];
        uIndex++;
    }

    /* 转换数字为字符串 */
    if(ulMsgNum == 0)
    {
        cNumBuf[iNumIndex++] = '0';
    }
    else
    {
        while(ulMsgNum > 0 && iNumIndex < 15)
        {
            cNumBuf[iNumIndex++] = '0' + (ulMsgNum % 10);
            ulMsgNum = ulMsgNum / 10;
        }
    }

    /* 反转数字字符串并复制到结果 */
    for(i = iNumIndex - 1; i >= 0 && uIndex < uBufSize - 1; i--)
    {
        pcBuf[uIndex] = cNumBuf[i];
        uIndex++;
    }

    /* 字符串结束符 */
    pcBuf[uIndex] = '\0';
}

/* 任务1：消息发送者 */
static void vSenderTask(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulCount = 0;
    size_t xSent;
    const char *pcMsgs[] = {"Hello", "FreeRTOS Message Buffer", "Longer message for testing!"};

    vPrintString("  [Sender] Task started\r\n");
    vTaskDelay(pdMS_TO_TICKS(DELAY_MEDIUM_MS));

    for(;;)
    {
        ulCount++;
        vPrintString("\r\n  [Sender] Sending message #");
        vPrintNumber(ulCount);
        vPrintString("\r\n");

        xSent = xMessageBufferSend(xTaskMsgBuf,
                                   (void *)pcMsgs[ulCount % 3],
                                   strlen(pcMsgs[ulCount % 3]),
                                   portMAX_DELAY);

        if(xSent > 0)
        {
            vPrintString("  [Sender] Sent ");
            vPrintNumber(xSent);
            vPrintString(" bytes\r\n");
        }

        vTaskDelay(pdMS_TO_TICKS(DELAY_LONG_MS));

        if(ulCount >= 6)
        {
            vPrintString("\r\n  [Sender] Done sending\r\n");
            break;
        }
    }

    vTaskDelete(NULL);
}

/* 任务2：消息接收者 */
static void vReceiverTask(void *pvParameters)
{
    (void)pvParameters;
    uint8_t ucBuf[256];
    size_t xReceived;
    uint32_t ulCount = 0;

    vPrintString("  [Receiver] Task started, waiting for messages...\r\n");

    for(;;)
    {
        xReceived = xMessageBufferReceive(xTaskMsgBuf, ucBuf, sizeof(ucBuf), portMAX_DELAY);

        if(xReceived > 0)
        {
            ulCount++;
            ucBuf[xReceived] = '\0';
            vPrintString("\r\n  [Receiver] Got message #");
            vPrintNumber(ulCount);
            vPrintString("\r\n");
            vPrintString("  [Receiver] Content: ");
            vPrintString((char *)ucBuf);
            vPrintString("\r\n");
        }
    }
}

/* 任务3：ISR模拟器 */
static void vISRSimTask(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulCount = 0;
    BaseType_t xHigherPriWoken;
    char cBuf[64];
    size_t xSent;

    vPrintString("\r\n  [ISR-Sim] Task started\r\n");
    vTaskDelay(pdMS_TO_TICKS(DELAY_MEDIUM_MS));

    for(;;)
    {
        ulCount++;
        vBuildISRMessage(cBuf, sizeof(cBuf), ulCount);

        vPrintString("\r\n  [ISR-Sim] Simulating interrupt #");
        vPrintNumber(ulCount);
        vPrintString("\r\n");

        xHigherPriWoken = pdFALSE;
        xSent = xMessageBufferSendFromISR(xISRMsgBuf, cBuf, strlen(cBuf), &xHigherPriWoken);

        if(xSent > 0)
        {
            vPrintString("  [ISR-Sim] Sent ");
            vPrintNumber(xSent);
            vPrintString(" bytes\r\n");
        }

        vTaskDelay(pdMS_TO_TICKS(DELAY_LONG_MS));

        if(ulCount >= 4)
        {
            vPrintString("\r\n  [ISR-Sim] Done\r\n");
            break;
        }
    }

    vTaskDelete(NULL);
}

/* 任务4：ISR消息处理 */
static void vISRHandlerTask(void *pvParameters)
{
    (void)pvParameters;
    uint8_t ucBuf[128];
    size_t xReceived;
    uint32_t ulCount = 0;

    vPrintString("  [ISR-Handler] Task started\r\n");

    for(;;)
    {
        xReceived = xMessageBufferReceive(xISRMsgBuf, ucBuf, sizeof(ucBuf), portMAX_DELAY);

        if(xReceived > 0)
        {
            ulCount++;
            ucBuf[xReceived] = '\0';
            vPrintString("\r\n  [ISR-Handler] Got ISR message #");
            vPrintNumber(ulCount);
            vPrintString("\r\n");
            vPrintString("  [ISR-Handler] Content: ");
            vPrintString((char *)ucBuf);
            vPrintString("\r\n");
        }
    }
}

/* 任务5：流缓冲区演示 */
static void vStreamDemoTask(void *pvParameters)
{
    (void)pvParameters;
    uint8_t ucData;
    uint8_t ucRxBuf[10];
    size_t xSent, xReceived;
    uint32_t i;
    const uint8_t ucTestStr[] = "StreamBuffer test";

    vPrintString("\r\n  [Stream-Demo] Task started\r\n");
    vTaskDelay(pdMS_TO_TICKS(DELAY_MEDIUM_MS));

    vPrintSeparator();
    vPrintString("  Message Buffer vs Stream Buffer\r\n");
    vPrintSeparator();
    vPrintString("- Message Buffer: Each message has length prefix\r\n");
    vPrintString("- Stream Buffer: Byte stream, no message boundaries\r\n");
    vPrintSeparator();

    /* 流缓冲区测试 */
    vPrintString("\r\n  [Stream-Demo] Sending bytes one by one:\r\n");
    for(i = 0; i < 5; i++)
    {
        ucData = 'A' + i;
        xSent = xStreamBufferSend(xStreamBuf, &ucData, 1, 0);
        if(xSent == 1)
        {
            vPrintString("  [Stream] Sent: ");
            vPrintChar(ucData);
            vPrintString("\r\n");
        }
    }

    xReceived = xStreamBufferReceive(xStreamBuf, ucRxBuf, sizeof(ucRxBuf), 0);
    vPrintString("  [Stream] Read ");
    vPrintNumber(xReceived);
    vPrintString(" bytes at once: ");
    for(i = 0; i < xReceived; i++)
    {
        vPrintChar(ucRxBuf[i]);
    }
    vPrintString("\r\n");

    vPrintString("\r\n  [Stream-Demo] Demo complete\r\n");
    vTaskDelete(NULL);
}

/* 任务6：工作任务 */
static void vWorkerTask(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulCount = 0;

    vPrintString("  [Worker] Background task started\r\n");

    for(;;)
    {
        ulCount++;
        if((ulCount % 10) == 0)
        {
            vPrintString("  [Worker] Running... loop #");
            vPrintNumber(ulCount);
            vPrintString("\r\n");
        }
        vTaskDelay(pdMS_TO_TICKS(DELAY_SHORT_MS));
    }
}

/* 主函数 */
int main(void)
{
    vUARTInit();

    vPrintString("\r\n");
    vPrintSeparator();
    vPrintString("  FreeRTOS Message Buffers Demo\r\n");
    vPrintSeparator();
    vPrintString("\r\n");

    vPrintString("Learning objectives:\r\n");
    vPrintString("1. Variable length messages\r\n");
    vPrintString("2. Task-to-task messaging\r\n");
    vPrintString("3. ISR-to-task messaging\r\n");
    vPrintString("4. Message vs Stream buffer comparison\r\n");
    vPrintString("\r\n");

    /* 创建缓冲区 */
    vPrintString("Creating buffers...\r\n");

    xTaskMsgBuf = xMessageBufferCreate(MESSAGE_BUFFER_SIZE);
    if(xTaskMsgBuf == NULL)
    {
        vPrintString("Error creating task message buffer!\r\n");
        for(;;);
    }
    vPrintString("  - Task message buffer OK\r\n");

    xISRMsgBuf = xMessageBufferCreate(MESSAGE_BUFFER_SIZE);
    if(xISRMsgBuf == NULL)
    {
        vPrintString("Error creating ISR message buffer!\r\n");
        for(;;);
    }
    vPrintString("  - ISR message buffer OK\r\n");

    xStreamBuf = xStreamBufferCreate(STREAM_BUFFER_SIZE, 1);
    if(xStreamBuf == NULL)
    {
        vPrintString("Error creating stream buffer!\r\n");
        for(;;);
    }
    vPrintString("  - Stream buffer OK\r\n");

    /* 创建任务 */
    vPrintString("\r\nCreating tasks...\r\n");

    xTaskCreate(vReceiverTask, "MsgRx", tskSTACK_SIZE, NULL, tskPRIORITY_RECEIVER, NULL);
    vPrintString("  - Receiver task (prio 4)\r\n");

    xTaskCreate(vSenderTask, "MsgTx", tskSTACK_SIZE, NULL, tskPRIORITY_SENDER, NULL);
    vPrintString("  - Sender task (prio 3)\r\n");

    xTaskCreate(vISRHandlerTask, "ISR-Rx", tskSTACK_SIZE, NULL, tskPRIORITY_RECEIVER, NULL);
    vPrintString("  - ISR handler task (prio 4)\r\n");

    xTaskCreate(vISRSimTask, "ISR-Sim", tskSTACK_SIZE, NULL, tskPRIORITY_ISR_SIM, NULL);
    vPrintString("  - ISR simulator task (prio 3)\r\n");

    xTaskCreate(vStreamDemoTask, "StreamDemo", tskSTACK_SIZE, NULL, tskPRIORITY_SENDER, NULL);
    vPrintString("  - Stream buffer demo task (prio 3)\r\n");

    xTaskCreate(vWorkerTask, "Worker", tskSTACK_SIZE, NULL, tskPRIORITY_WORKER, NULL);
    vPrintString("  - Worker task (prio 2)\r\n");

    vPrintString("\r\nAll tasks created!\r\n");
    vPrintString("\r\nStarting scheduler...\r\n\r\n");

    vTaskStartScheduler();

    vPrintString("Error: scheduler start failed!\r\n");
    for(;;);

    return 0;
}

/* 平台相关的串口输出函数 */
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
#error "Undefined platform! Define PLATFORM_LM3S6965 or PLATFORM_STM32F103"
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

/* FreeRTOS 钩子函数 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    vPrintString("\r\n!!! Stack overflow: ");
    vPrintString(pcTaskName);
    vPrintString("\r\n");
    for(;;);
}

void vApplicationMallocFailedHook(void)
{
    vPrintString("\r\n!!! Malloc failed!\r\n");
    for(;;);
}
