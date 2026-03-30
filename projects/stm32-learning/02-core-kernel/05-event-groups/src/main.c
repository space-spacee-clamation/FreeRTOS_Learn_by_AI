/**
 * @file main.c
 * @brief FreeRTOS事件组（Event Groups）学习项目
 *
 * 本项目演示FreeRTOS事件组的使用方法。
 * 学习要点：
 * 1. 事件组的概念和工作原理
 * 2. 事件位的设置、清除和等待
 * 3. AND/OR条件等待机制
 * 4. xEventGroupCreate() / xEventGroupSetBits() / xEventGroupClearBits() API
 * 5. xEventGroupWaitBits() - 等待事件位
 * 6. xEventGroupSync() - 任务同步（会合点）
 * 7. 超时处理机制
 * 8. 多条件触发场景的实际应用
 *
 * 参考资料：
 * - FreeRTOS官方文档: https://www.freertos.org/FreeRTOS-Event-Groups.html
 * - FreeRTOS Demo项目: Common/Minimal/EventGroupsDemo.c
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* FreeRTOS 头文件 */
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

/* 平台相关的串口输出函数声明 */
void vUARTInit(void);
void vPrintString(const char *pcString);
void vPrintNumber(uint32_t ulNumber);

/* ==============================================
 *              宏定义 - 事件位定义
 * ==============================================
 */

/* 事件位定义 - 每个位代表一个事件 */
#define evtBIT_SENSOR_TEMP_READY     ( 1 << 0 )  /* 温度传感器就绪 */
#define evtBIT_SENSOR_HUMID_READY    ( 1 << 1 )  /* 湿度传感器就绪 */
#define evtBIT_SENSOR_PRESS_READY    ( 1 << 2 )  /* 压力传感器就绪 */
#define evtBIT_DATA_PROCESSED         ( 1 << 3 )  /* 数据处理完成 */
#define evtBIT_ALARM_TRIGGERED        ( 1 << 4 )  /* 报警触发 */

/* 组合事件位 - 用于多条件等待 */
#define evtALL_SENSORS_READY          ( evtBIT_SENSOR_TEMP_READY | \
                                         evtBIT_SENSOR_HUMID_READY | \
                                         evtBIT_SENSOR_PRESS_READY )

/* 任务优先级 */
#define tskPRIORITY_SENSOR            3
#define tskPRIORITY_PROCESSOR         2
#define tskPRIORITY_MONITOR           1
#define tskPRIORITY_SYNC_TASK         2

/* 任务堆栈大小 */
#define tskSTACK_SIZE_BASIC           256
#define tskSTACK_SIZE_LARGE           384

/* 延时时间（毫秒） */
#define DELAY_SHORT_MS                500
#define DELAY_MEDIUM_MS               1000
#define DELAY_LONG_MS                 2000
#define DELAY_TIMEOUT_MS              5000

/* ==============================================
 *              全局变量
 * ==============================================
 */

/* 事件组句柄 */
static EventGroupHandle_t xSensorEventGroup = NULL;
static EventGroupHandle_t xSyncEventGroup = NULL;

/* ==============================================
 *              辅助函数
 * ==============================================
 */

/**
 * @brief 打印事件组当前状态
 * @param xEventGroup 事件组句柄
 * @param pcName 事件组名称
 */
void vPrintEventGroupStatus(EventGroupHandle_t xEventGroup, const char *pcName)
{
    EventBits_t uxBits;

    if(xEventGroup == NULL)
    {
        return;
    }

    uxBits = xEventGroupGetBits(xEventGroup);

    vPrintString("\r\n---------- ");
    vPrintString(pcName);
    vPrintString(" 状态 ----------\r\n");
    vPrintString("当前事件位: 0x");
    vPrintNumber((uint32_t)uxBits);
    vPrintString("\r\n");
    vPrintString("  温度就绪: ");
    vPrintString((uxBits & evtBIT_SENSOR_TEMP_READY) ? "✓" : "✗");
    vPrintString("\r\n");
    vPrintString("  湿度就绪: ");
    vPrintString((uxBits & evtBIT_SENSOR_HUMID_READY) ? "✓" : "✗");
    vPrintString("\r\n");
    vPrintString("  压力就绪: ");
    vPrintString((uxBits & evtBIT_SENSOR_PRESS_READY) ? "✓" : "✗");
    vPrintString("\r\n");
    vPrintString("  处理完成: ");
    vPrintString((uxBits & evtBIT_DATA_PROCESSED) ? "✓" : "✗");
    vPrintString("\r\n");
    vPrintString("  报警触发: ");
    vPrintString((uxBits & evtBIT_ALARM_TRIGGERED) ? "✓" : "✗");
    vPrintString("\r\n");
    vPrintString("----------------------------------------\r\n\r\n");
}

/* ==============================================
 *              任务函数定义
 * ==============================================
 */

/**
 * @brief 温度传感器任务 - 模拟温度传感器读取
 * @param pvParameters 任务参数
 */
void vTempSensorTask(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulReadCount = 0;

    vPrintString("[TempSensor] 温度传感器任务启动！\r\n");

    for(;;)
    {
        ulReadCount++;

        vPrintString("[TempSensor] 读取温度数据 #");
        vPrintNumber(ulReadCount);
        vPrintString("...\r\n");

        /* 模拟传感器读取延迟 */
        vTaskDelay(pdMS_TO_TICKS(DELAY_SHORT_MS));

        /* 设置温度就绪事件位 */
        xEventGroupSetBits(xSensorEventGroup, evtBIT_SENSOR_TEMP_READY);

        vPrintString("[TempSensor] ✓ 温度数据就绪！\r\n");
        vPrintEventGroupStatus(xSensorEventGroup, "传感器事件组");

        /* 等待一段时间后清除位，准备下一次读取 */
        vTaskDelay(pdMS_TO_TICKS(DELAY_LONG_MS));
        xEventGroupClearBits(xSensorEventGroup, evtBIT_SENSOR_TEMP_READY);
    }
}

/**
 * @brief 湿度传感器任务 - 模拟湿度传感器读取
 * @param pvParameters 任务参数
 */
void vHumidSensorTask(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulReadCount = 0;

    vPrintString("[HumidSensor] 湿度传感器任务启动！\r\n");

    for(;;)
    {
        ulReadCount++;

        vPrintString("[HumidSensor] 读取湿度数据 #");
        vPrintNumber(ulReadCount);
        vPrintString("...\r\n");

        /* 模拟传感器读取延迟 - 比温度稍慢 */
        vTaskDelay(pdMS_TO_TICKS(DELAY_MEDIUM_MS));

        /* 设置湿度就绪事件位 */
        xEventGroupSetBits(xSensorEventGroup, evtBIT_SENSOR_HUMID_READY);

        vPrintString("[HumidSensor] ✓ 湿度数据就绪！\r\n");
        vPrintEventGroupStatus(xSensorEventGroup, "传感器事件组");

        vTaskDelay(pdMS_TO_TICKS(DELAY_LONG_MS));
        xEventGroupClearBits(xSensorEventGroup, evtBIT_SENSOR_HUMID_READY);
    }
}

/**
 * @brief 压力传感器任务 - 模拟压力传感器读取
 * @param pvParameters 任务参数
 */
void vPressSensorTask(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulReadCount = 0;

    vPrintString("[PressSensor] 压力传感器任务启动！\r\n");

    for(;;)
    {
        ulReadCount++;

        vPrintString("[PressSensor] 读取压力数据 #");
        vPrintNumber(ulReadCount);
        vPrintString("...\r\n");

        /* 模拟传感器读取延迟 - 最慢 */
        vTaskDelay(pdMS_TO_TICKS(DELAY_LONG_MS));

        /* 设置压力就绪事件位 */
        xEventGroupSetBits(xSensorEventGroup, evtBIT_SENSOR_PRESS_READY);

        vPrintString("[PressSensor] ✓ 压力数据就绪！\r\n");
        vPrintEventGroupStatus(xSensorEventGroup, "传感器事件组");

        vTaskDelay(pdMS_TO_TICKS(DELAY_LONG_MS));
        xEventGroupClearBits(xSensorEventGroup, evtBIT_SENSOR_PRESS_READY);
    }
}

/**
 * @brief 数据处理任务 - 等待所有传感器就绪后处理数据
 * @param pvParameters 任务参数
 */
void vDataProcessorTask(void *pvParameters)
{
    (void)pvParameters;
    EventBits_t uxBits;
    uint32_t ulProcessCount = 0;

    vPrintString("[Processor] 数据处理任务启动！\r\n");
    vPrintString("[Processor] 等待所有传感器数据就绪...\r\n");

    for(;;)
    {
        /* ==============================================
         *  学习重点1: AND条件等待 - 等待所有位都设置
         *  xEventGroupWaitBits参数说明：
         *  - xEventGroup: 事件组句柄
         *  - uxBitsToWaitFor: 要等待的事件位
         *  - xClearOnExit: 退出时是否清除等待的位
         *  - xWaitForAllBits: pdTRUE=AND(等待所有位), pdFALSE=OR(等待任意位)
         *  - xTicksToWait: 超时时间
         * ==============================================
         */
        vPrintString("\r\n[Processor] ========== 等待所有传感器 (AND条件) ==========\r\n");

        uxBits = xEventGroupWaitBits(
            xSensorEventGroup,          /* 事件组 */
            evtALL_SENSORS_READY,       /* 等待所有传感器就绪位 */
            pdTRUE,                      /* 退出时清除这些位 */
            pdTRUE,                      /* AND条件 - 所有位都必须设置 */
            pdMS_TO_TICKS(DELAY_TIMEOUT_MS)  /* 超时时间 */
        );

        /* 检查返回值 */
        if((uxBits & evtALL_SENSORS_READY) == evtALL_SENSORS_READY)
        {
            /* 所有传感器都就绪了 */
            ulProcessCount++;
            vPrintString("[Processor] ✓ 所有传感器数据就绪！开始处理 #");
            vPrintNumber(ulProcessCount);
            vPrintString("\r\n");

            /* 模拟数据处理 */
            vTaskDelay(pdMS_TO_TICKS(DELAY_SHORT_MS));

            /* 设置处理完成事件位 */
            xEventGroupSetBits(xSensorEventGroup, evtBIT_DATA_PROCESSED);
            vPrintString("[Processor] ✓ 数据处理完成！\r\n");
            vPrintEventGroupStatus(xSensorEventGroup, "传感器事件组");

            /* 清除处理完成位 */
            vTaskDelay(pdMS_TO_TICKS(DELAY_MEDIUM_MS));
            xEventGroupClearBits(xSensorEventGroup, evtBIT_DATA_PROCESSED);
        }
        else
        {
            /* 超时 */
            vPrintString("[Processor] ✗ 等待超时！当前状态:\r\n");
            vPrintEventGroupStatus(xSensorEventGroup, "传感器事件组");
        }
    }
}

/**
 * @brief 监控任务 - 演示OR条件等待（任意事件触发）
 * @param pvParameters 任务参数
 */
void vMonitorTask(void *pvParameters)
{
    (void)pvParameters;
    EventBits_t uxBits;
    uint32_t ulAlertCount = 0;

    vPrintString("[Monitor] 监控任务启动！\r\n");
    vPrintString("[Monitor] 监控任意传感器事件 (OR条件)...\r\n");

    for(;;)
    {
        /* ==============================================
         *  学习重点2: OR条件等待 - 等待任意位设置
         * ==============================================
         */
        uxBits = xEventGroupWaitBits(
            xSensorEventGroup,          /* 事件组 */
            evtALL_SENSORS_READY | evtBIT_DATA_PROCESSED,  /* 等待任意传感器或处理完成 */
            pdFALSE,                     /* 不清除位 */
            pdFALSE,                     /* OR条件 - 任意位设置即可 */
            pdMS_TO_TICKS(DELAY_MEDIUM_MS)
        );

        if(uxBits != 0)
        {
            ulAlertCount++;
            vPrintString("\r\n[Monitor] ========== 事件触发 #");
            vPrintNumber(ulAlertCount);
            vPrintString(" ==========\r\n");

            if(uxBits & evtBIT_SENSOR_TEMP_READY)
            {
                vPrintString("[Monitor] → 温度传感器就绪\r\n");
            }
            if(uxBits & evtBIT_SENSOR_HUMID_READY)
            {
                vPrintString("[Monitor] → 湿度传感器就绪\r\n");
            }
            if(uxBits & evtBIT_SENSOR_PRESS_READY)
            {
                vPrintString("[Monitor] → 压力传感器就绪\r\n");
            }
            if(uxBits & evtBIT_DATA_PROCESSED)
            {
                vPrintString("[Monitor] → 数据处理完成\r\n");
            }
        }
    }
}

/**
 * @brief 同步任务1 - 用于演示xEventGroupSync()
 * @param pvParameters 任务参数
 */
void vSyncTask1(void *pvParameters)
{
    (void)pvParameters;
    EventBits_t uxBits;
    uint32_t ulSyncCount = 0;

    vPrintString("[SyncTask1] 同步任务1启动！\r\n");

    for(;;)
    {
        ulSyncCount++;

        vPrintString("[SyncTask1] 准备就绪 #");
        vPrintNumber(ulSyncCount);
        vPrintString("，等待其他任务...\r\n");

        /* ==============================================
         *  学习重点3: 任务同步（会合点）
         *  xEventGroupSync() 特点：
         *  1. 原子操作：设置自己的位 + 等待其他位
         *  2. 所有任务到达后自动清除同步位
         *  3. 适用于多任务会合场景
         * ==============================================
         */
        uxBits = xEventGroupSync(
            xSyncEventGroup,           /* 事件组 */
            (1 << 0),                  /* 设置自己的位（bit 0） */
            (1 << 0) | (1 << 1) | (1 << 2),  /* 等待所有3个任务的位 */
            portMAX_DELAY              /* 无限等待 */
        );

        if((uxBits & ((1 << 0) | (1 << 1) | (1 << 2))) == ((1 << 0) | (1 << 1) | (1 << 2)))
        {
            vPrintString("[SyncTask1] ✓ 所有任务已同步！执行同步操作 #");
            vPrintNumber(ulSyncCount);
            vPrintString("\r\n");
        }

        vTaskDelay(pdMS_TO_TICKS(DELAY_LONG_MS));
    }
}

/**
 * @brief 同步任务2 - 用于演示xEventGroupSync()
 * @param pvParameters 任务参数
 */
void vSyncTask2(void *pvParameters)
{
    (void)pvParameters;
    EventBits_t uxBits;
    uint32_t ulSyncCount = 0;

    vPrintString("[SyncTask2] 同步任务2启动！\r\n");

    for(;;)
    {
        ulSyncCount++;

        vPrintString("[SyncTask2] 准备就绪 #");
        vPrintNumber(ulSyncCount);
        vPrintString("，等待其他任务...\r\n");

        /* 模拟一些准备工作 */
        vTaskDelay(pdMS_TO_TICKS(DELAY_SHORT_MS));

        uxBits = xEventGroupSync(
            xSyncEventGroup,
            (1 << 1),                  /* 设置自己的位（bit 1） */
            (1 << 0) | (1 << 1) | (1 << 2),
            portMAX_DELAY
        );

        if((uxBits & ((1 << 0) | (1 << 1) | (1 << 2))) == ((1 << 0) | (1 << 1) | (1 << 2)))
        {
            vPrintString("[SyncTask2] ✓ 所有任务已同步！执行同步操作 #");
            vPrintNumber(ulSyncCount);
            vPrintString("\r\n");
        }

        vTaskDelay(pdMS_TO_TICKS(DELAY_LONG_MS));
    }
}

/**
 * @
 * @brief 同步任务3 - 用于演示xEventGroupSync()
 * @param pvParameters 任务参数
 */
void vSyncTask3(void *pvParameters)
{
    (void)pvParameters;
    EventBits_t uxBits;
    uint32_t ulSyncCount = 0;

    vPrintString("[SyncTask3] 同步任务3启动！\r\n");

    for(;;)
    {
        ulSyncCount++;

        vPrintString("[SyncTask3] 准备就绪 #");
        vPrintNumber(ulSyncCount);
        vPrintString("，等待其他任务...\r\n");

        /* 模拟更多准备工作 */
        vTaskDelay(pdMS_TO_TICKS(DELAY_MEDIUM_MS));

        uxBits = xEventGroupSync(
            xSyncEventGroup,
            (1 << 2),                  /* 设置自己的位（bit 2） */
            (1 << 0) | (1 << 1) | (1 << 2),
            portMAX_DELAY
        );

        if((uxBits & ((1 << 0) | (1 << 1) | (1 << 2))) == ((1 << 0) | (1 << 1) | (1 << 2)))
        {
            vPrintString("[SyncTask3] ✓ 所有任务已同步！执行同步操作 #");
            vPrintNumber(ulSyncCount);
            vPrintString("\r\n");
        }

        vTaskDelay(pdMS_TO_TICKS(DELAY_LONG_MS));
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
    vPrintString("  FreeRTOS 事件组学习示例\r\n");
    vPrintString("====================================\r\n");
    vPrintString("\r\n");

    vPrintString("学习要点：\r\n");
    vPrintString("1. 事件组的概念和工作原理\r\n");
    vPrintString("2. 事件位的设置、清除和等待\r\n");
    vPrintString("3. AND/OR条件等待机制\r\n");
    vPrintString("4. xEventGroupCreate() / SetBits() / ClearBits()\r\n");
    vPrintString("5. xEventGroupWaitBits() - 等待事件位\r\n");
    vPrintString("6. xEventGroupSync() - 任务同步（会合点）\r\n");
    vPrintString("7. 超时处理机制\r\n");
    vPrintString("\r\n");

    vPrintString("====================================\r\n");
    vPrintString("  事件组 vs 其他同步机制：\r\n");
    vPrintString("====================================\r\n");
    vPrintString("特性          队列      信号量    事件组\r\n");
    vPrintString("----------------------------------------\r\n");
    vPrintString("数据传递      ✓ 有      ✗ 无      ✗ 无\r\n");
    vPrintString("多事件等待    ✗ 无      ✗ 无      ✓ 有\r\n");
    vPrintString("AND/OR逻辑    ✗ 无      ✗ 无      ✓ 有\r\n");
    vPrintString("任务同步      ✗ 无      ✗ 无      ✓ 有\r\n");
    vPrintString("====================================\r\n");
    vPrintString("\r\n");

    /* 创建事件组 */
    vPrintString("正在创建事件组...\r\n");
    xSensorEventGroup = xEventGroupCreate();
    xSyncEventGroup = xEventGroupCreate();

    if(xSensorEventGroup != NULL && xSyncEventGroup != NULL)
    {
        vPrintString("✓ 事件组创建成功！\r\n");
    }
    else
    {
        vPrintString("✗ 事件组创建失败！\r\n");
        for(;;);
    }

    vPrintString("\r\n正在创建任务...\r\n");

    /* 创建传感器相关任务 */
    xTaskCreate(vTempSensorTask, "TempSensor", tskSTACK_SIZE_BASIC, NULL, tskPRIORITY_SENSOR, NULL);
    xTaskCreate(vHumidSensorTask, "HumidSensor", tskSTACK_SIZE_BASIC, NULL, tskPRIORITY_SENSOR, NULL);
    xTaskCreate(vPressSensorTask, "PressSensor", tskSTACK_SIZE_BASIC, NULL, tskPRIORITY_SENSOR, NULL);
    xTaskCreate(vDataProcessorTask, "Processor", tskSTACK_SIZE_LARGE, NULL, tskPRIORITY_PROCESSOR, NULL);
    xTaskCreate(vMonitorTask, "Monitor", tskSTACK_SIZE_BASIC, NULL, tskPRIORITY_MONITOR, NULL);

    /* 创建同步任务 */
    xTaskCreate(vSyncTask1, "SyncTask1", tskSTACK_SIZE_BASIC, NULL, tskPRIORITY_SYNC_TASK, NULL);
    xTaskCreate(vSyncTask2, "SyncTask2", tskSTACK_SIZE_BASIC, NULL, tskPRIORITY_SYNC_TASK, NULL);
    xTaskCreate(vSyncTask3, "SyncTask3", tskSTACK_SIZE_BASIC, NULL, tskPRIORITY_SYNC_TASK, NULL);

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
