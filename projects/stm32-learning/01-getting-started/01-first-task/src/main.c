/**
 * @file main.c
 * @brief 第一个FreeRTOS任务示例
 *
 * 本项目演示如何创建和运行第一个FreeRTOS任务。
 * 学习要点：
 * 1. 使用 xTaskCreate() 创建任务
 * 2. 任务函数的编写方法
 * 3. vTaskDelay() 任务延时
 * 4. 启动调度器 vTaskStartScheduler()
 *
 * 参考资料：
 * - FreeRTOS官方文档: https://www.freertos.org/a00125.html
 * - Mastering-the-FreeRTOS-Kernel 第4章
 */

#include <stdint.h>
#include <stdbool.h>

/* FreeRTOS 头文件 */
#include "FreeRTOS.h"
#include "task.h"

/* 平台相关的串口输出函数声明 */
void vUARTInit(void);
void vPrintString(const char *pcString);
void vPrintNumber(uint32_t ulNumber);

/* ==============================================
 *              学习重点1：任务函数
 * ==============================================
 *
 * FreeRTOS任务函数必须符合以下规范：
 * 1. 返回类型：void
 * 2. 参数：void *（可以传递任意类型的数据）
 * 3. 必须是无限循环
 * 4. 不能返回（如果返回会被调度器删除）
 */

/**
 * @brief 第一个FreeRTOS任务函数
 * @param pvParameters 传递给任务的参数（本例中未使用）
 *
 * 这是一个简单的任务，每隔500毫秒输出一次计数器值。
 * 注意：
 * - 使用 vTaskDelay() 让任务进入阻塞状态
 * - 任务必须是无限循环 for(;;) 或 while(1)
 * - 不要使用普通的延时循环，那会浪费CPU时间
 */
void vFirstTask(void *pvParameters)
{
    /* 防止编译器警告：参数未使用 */
    (void)pvParameters;

    /* 任务局部变量 */
    uint32_t ulCounter = 0;

    vPrintString("FirstTask: 任务开始运行！\r\n");

    /* ==============================================
     *         学习重点2：任务无限循环
     * ==============================================
     *
     * 所有FreeRTOS任务都必须是无限循环！
     * 如果任务函数返回，调度器会自动删除该任务。
     */
    for(;;)
    {
        /* 输出计数器值 */
        vPrintString("FirstTask: 计数器 = ");
        vPrintNumber(ulCounter);
        vPrintString("\r\n");

        /* 计数器递增 */
        ulCounter++;

        /* ==============================================
         *       学习重点3：任务延时 vTaskDelay()
         * ==============================================
         *
         * vTaskDelay() 让任务进入阻塞状态指定的时间。
         * 在阻塞期间，任务不占用CPU时间，其他任务可以运行。
         *
         * pdMS_TO_TICKS() 宏将毫秒转换为系统时钟节拍数。
         * 例如：pdMS_TO_TICKS(500) = 500毫秒
         *
         * 重要提示：
         * - 不要使用 for 循环空转延时，那会浪费CPU！
         * - vTaskDelay() 是相对延时，从调用时刻开始计算
         */
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    /* 注意：任务永远不应该到达这里！ */
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
    vPrintString("  第一个FreeRTOS任务示例\r\n");
    vPrintString("====================================\r\n");
    vPrintString("\r\n");

    vPrintString("学习要点：\r\n");
    vPrintString("1. 使用 xTaskCreate() 创建任务\r\n");
    vPrintString("2. 任务函数的编写方法\r\n");
    vPrintString("3. vTaskDelay() 任务延时\r\n");
    vPrintString("4. 启动调度器 vTaskStartScheduler()\r\n");
    vPrintString("\r\n");

    /* ==============================================
     *       学习重点4：创建任务 xTaskCreate()
     * ==============================================
     *
     * BaseType_t xTaskCreate(
     *     TaskFunction_t pvTaskCode,        // 任务函数指针
     *     const char * const pcName,         // 任务名称（调试用）
     *     const uint16_t usStackDepth,       // 栈大小（以字为单位）
     *     void * const pvParameters,          // 传递给任务的参数
     *     UBaseType_t uxPriority,            // 任务优先级
     *     TaskHandle_t * const pxCreatedTask  // 返回的任务句柄
     * );
     *
     * 参数说明：
     * 1. pvTaskCode: 任务函数的地址 &vFirstTask
     * 2. pcName: 任务名称 "FirstTask"（调试用，不影响功能）
     * 3. usStackDepth: 栈大小 128（单位：字，32位系统中=512字节）
     * 4. pvParameters: 任务参数 NULL（本例中不需要）
     * 5. uxPriority: 优先级 1（数值越大优先级越高，0是空闲任务）
     * 6. pxCreatedTask: 任务句柄 NULL（不需要获取句柄）
     *
     * 返回值：
     * - pdPASS: 任务创建成功
     * - errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY: 内存不足
     */

    vPrintString("正在创建第一个任务...\r\n");

    BaseType_t xResult = xTaskCreate(
        vFirstTask,              /* 任务函数 */
        "FirstTask",             /* 任务名称（用于调试） */
        128,                     /* 栈大小（128字 = 512字节） */
        NULL,                    /* 任务参数（不需要） */
        1,                       /* 优先级（1 = 高于空闲任务） */
        NULL                     /* 任务句柄（不需要） */
    );

    /* 检查任务是否创建成功 */
    if(xResult == pdPASS)
    {
        vPrintString("任务创建成功！\r\n");
    }
    else
    {
        vPrintString("错误：任务创建失败！（内存不足？）\r\n");
        /* 任务创建失败，进入死循环 */
        for(;;);
    }

    /* ==============================================
     *      学习重点5：启动调度器 vTaskStartScheduler()
     * ==============================================
     *
     * vTaskStartScheduler() 启动FreeRTOS调度器。
     *
     * 重要说明：
     * - 调用此函数后，调度器开始运行已创建的任务
     * - 如果调度器启动成功，此函数永远不会返回！
     * - 只有在内存不足等错误情况下才会返回
     *
     * 调度器启动后会：
     * 1. 创建空闲任务（优先级0）
     * 2. 如果启用了定时器服务，创建定时器任务
     * 3. 启动系统时钟节拍中断
     * 4. 开始任务调度
     */

    vPrintString("启动FreeRTOS调度器...\r\n");
    vPrintString("\r\n");

    vTaskStartScheduler();

    /* ==============================================
     *      注意：下面的代码只有在调度器启动失败时才会执行！
     * ==============================================
     */

    vPrintString("错误：调度器启动失败！\r\n");
    vPrintString("通常原因：内存不足，无法创建空闲任务\r\n");
    vPrintString("请检查 FreeRTOSConfig.h 中的 configTOTAL_HEAP_SIZE\r\n");

    /* 进入死循环 */
    for(;;);

    /* 永远不会到达这里 */
    return 0;
}

/* ==============================================
 *           平台相关的串口输出函数
 * ==============================================
 *
 * 这部分代码根据不同的硬件平台有不同的实现。
 * 下面同时提供了 LM3S6965 和 STM32F103 的实现。
 */

/* 根据平台选择合适的实现 */
#if defined(PLATFORM_LM3S6965)

/* LM3S6965 串口实现 */
#define UART0_DR (*((volatile unsigned long *)0x4000C000))
#define UART0_FR (*((volatile unsigned long *)0x4000C018))
#define UART_FR_TXFF (1 << 5) /* 发送FIFO满 */

void vUARTInit(void)
{
    /* LM3S6965 UART0 已经由QEMU初始化，这里不需要额外操作 */
}

void vPrintChar(char c)
{
    /* 等待发送FIFO有空 */
    while(UART0_FR & UART_FR_TXFF);
    /* 发送字符 */
    UART0_DR = c;
}

#elif defined(PLATFORM_STM32F103)

/* STM32F103 串口实现 */
/* USART1 数据寄存器地址 */
#define USART1_DR (*((volatile unsigned long *)0x40013804))
/* USART1 状态寄存器 */
#define USART1_SR (*((volatile unsigned long *)0x40013800))
/* 发送数据寄存器空标志 */
#define USART_SR_TXE (1 << 7)

void vUARTInit(void)
{
    /* STM32F103 USART1 不需要特殊初始化，QEMU会处理 */
}

void vPrintChar(char c)
{
    /* 等待发送数据寄存器空 */
    while(!(USART1_SR & USART_SR_TXE));
    /* 发送字符 */
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

    /* 处理0的特殊情况 */
    if(ulNumber == 0)
    {
        vPrintChar('0');
        return;
    }

    /* 将数字转换为字符串（逆序） */
    while(ulNumber > 0 && iIndex < 15)
    {
        cBuffer[iIndex] = '0' + (ulNumber % 10);
        ulNumber = ulNumber / 10;
        iIndex++;
    }

    /* 逆序输出 */
    for(iIndex--; iIndex >= 0; iIndex--)
    {
        vPrintChar(cBuffer[iIndex]);
    }
}

/* ==============================================
 *           FreeRTOS 钩子函数（可选）
 * ==============================================
 */

/**
 * @brief 栈溢出钩子函数
 *
 * 当检测到任务栈溢出时调用。
 * 启用此钩子需要在 FreeRTOSConfig.h 中设置：
 * #define configCHECK_FOR_STACK_OVERFLOW 1 或 2
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

/**
 * @brief 空闲钩子函数
 *
 * 空闲任务运行时调用。
 * 启用此钩子需要在 FreeRTOSConfig.h 中设置：
 * #define configUSE_IDLE_HOOK 1
 *
 * 注意：空闲钩子函数中不能调用会阻塞的函数！
 */
void vApplicationIdleHook(void)
{
    /* 可以在这里放一些低优先级的后台处理 */
    /* 但不能调用阻塞函数！ */
}

/**
 * @brief malloc失败钩子函数
 *
 * 当 pvPortMalloc() 失败时调用。
 * 启用此钩子需要在 FreeRTOSConfig.h 中设置：
 * #define configUSE_MALLOC_FAILED_HOOK 1
 */
void vApplicationMallocFailedHook(void)
{
    vPrintString("\r\n!!! 内存分配失败！\r\n");

    for(;;);
}
