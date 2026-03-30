/**
 * @file main.c
 * @brief FreeRTOS综合应用学习项目
 *
 * 学习要点：
 * 1. 传感器数据采集任务（软件定时器定时采样）
 * 2. 数据处理任务（队列通信）
 * 3. 显示/输出任务（任务通知触发更新）
 * 4. 命令处理任务（事件组控制各任务状态）
 * 5. 多任务协调与同步
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* FreeRTOS 头文件 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "event_groups.h"

/* 平台相关的串口输出函数声明 */
void vUARTInit(void);
void vPrintString(const char *pcString);
void vPrintNumber(uint32_t ulNumber);
void vPrintChar(char c);

/* ========================================
 * 系统配置宏定义
 * ======================================== */

/* 任务优先级 */
#define tskPRIORITY_CMD_HANDLER        5
#define tskPRIORITY_DISPLAY             4
#define tskPRIORITY_DATA_PROC           3
#define tskPRIORITY_SENSOR              3
#define tskPRIORITY_WORKER              2

/* 任务堆栈大小 */
#define tskSTACK_SIZE                   256

/* 队列配置 */
#define DATA_QUEUE_LENGTH               10
#define DATA_QUEUE_ITEM_SIZE            sizeof(SensorData_t)

/* 软件定时器配置 */
#define SENSOR_TIMER_PERIOD_MS          500
#define DISPLAY_UPDATE_TIMER_PERIOD_MS  2000

/* 事件组位定义 */
#define EVENT_BIT_SENSOR_ENABLE         (1 << 0)
#define EVENT_BIT_DATA_PROC_ENABLE      (1 << 1)
#define EVENT_BIT_DISPLAY_ENABLE        (1 << 2)
#define EVENT_BIT_SYSTEM_RUNNING        (1 << 3)
#define EVENT_BIT_ALL_TASKS_READY       (EVENT_BIT_SENSOR_ENABLE | \
                                           EVENT_BIT_DATA_PROC_ENABLE | \
                                           EVENT_BIT_DISPLAY_ENABLE)

/* 延迟常量 */
#define DELAY_SHORT_MS                  100
#define DELAY_MEDIUM_MS                 500
#define DELAY_LONG_MS                   1000

/* ========================================
 * 数据结构定义
 * ======================================== */

/* 传感器数据结构体 */
typedef struct {
    uint32_t ulTimestamp;      /* 时间戳 */
    int32_t  lTemperature;      /* 温度（模拟值，单位0.1度） */
    int32_t  lHumidity;         /* 湿度（模拟值，单位0.1%） */
    int32_t  lPressure;         /* 气压（模拟值，单位hPa） */
} SensorData_t;

/* 处理后的数据结构体 */
typedef struct {
    uint32_t ulTimestamp;
    int32_t  lAvgTemp;          /* 平均温度 */
    int32_t  lAvgHumidity;      /* 平均湿度 */
    int32_t  lAvgPressure;      /* 平均气压 */
    uint32_t ulSampleCount;     /* 采样计数 */
} ProcessedData_t;

/* ========================================
 * 全局变量
 * ======================================== */

/* 队列句柄 */
static QueueHandle_t xSensorDataQueue = NULL;

/* 事件组句柄 */
static EventGroupHandle_t xSystemEventGroup = NULL;

/* 软件定时器句柄 */
static TimerHandle_t xSensorTimer = NULL;
static TimerHandle_t xDisplayTimer = NULL;

/* 任务句柄 */
static TaskHandle_t xDisplayTaskHandle = NULL;

/* 处理后的数据缓存 */
static ProcessedData_t xLatestProcessedData = {0};

/* 模拟传感器状态 */
static int32_t lSensorBaseTemp = 250;    /* 25.0度 */
static int32_t lSensorBaseHumidity = 600; /* 60.0% */
static int32_t lSensorBasePressure = 1013;/* 1013 hPa */

/* ========================================
 * 辅助函数
 * ======================================== */

void vPrintSeparator(void)
{
    vPrintString("\r\n========================================\r\n");
}

/* 模拟传感器数据采集 */
void vSimulateSensorReading(SensorData_t *pxData)
{
    static uint32_t ulSampleCount = 0;
    ulSampleCount++;

    pxData->ulTimestamp = xTaskGetTickCount();

    /* 生成带波动的模拟数据 */
    pxData->lTemperature = lSensorBaseTemp + ((int32_t)(ulSampleCount % 20) - 10);
    pxData->lHumidity = lSensorBaseHumidity + ((int32_t)(ulSampleCount % 15) - 7);
    pxData->lPressure = lSensorBasePressure + ((int32_t)(ulSampleCount % 10) - 5);
}

/* ========================================
 * 软件定时器回调函数
 * ======================================== */

/* 传感器采样定时器回调 */
static void vSensorTimerCallback(TimerHandle_t xTimer)
{
    (void)xTimer;
    SensorData_t xSensorData;

    /* 检查传感器是否启用 */
    if((xEventGroupGetBits(xSystemEventGroup) & EVENT_BIT_SENSOR_ENABLE) == 0)
    {
        return;
    }

    /* 模拟传感器读数 */
    vSimulateSensorReading(&xSensorData);

    /* 发送到队列 */
    xQueueSend(xSensorDataQueue, &xSensorData, 0);
}

/* 显示更新定时器回调 */
static void vDisplayTimerCallback(TimerHandle_t xTimer)
{
    (void)xTimer;

    /* 检查显示是否启用 */
    if((xEventGroupGetBits(xSystemEventGroup) & EVENT_BIT_DISPLAY_ENABLE) == 0)
    {
        return;
    }

    /* 发送任务通知触发显示更新 */
    xTaskNotifyGive(xDisplayTaskHandle);
}

/* ========================================
 * 任务1：传感器监控任务
 * ======================================== */
static void vSensorTask(void *pvParameters)
{
    (void)pvParameters;

    vPrintString("  [Sensor] Task started\r\n");

    /* 标记传感器任务就绪 */
    xEventGroupSetBits(xSystemEventGroup, EVENT_BIT_SENSOR_ENABLE);

    for(;;)
    {
        /* 任务主要通过软件定时器触发采样，这里只做状态监控 */
        vTaskDelay(pdMS_TO_TICKS(DELAY_LONG_MS));
    }
}

/* ========================================
 * 任务2：数据处理任务
 * ======================================== */
static void vDataProcessingTask(void *pvParameters)
{
    (void)pvParameters;
    SensorData_t xReceivedData;
    static int32_t lTempSum = 0;
    static int32_t lHumiditySum = 0;
    static int32_t lPressureSum = 0;
    static uint32_t ulSampleCount = 0;

    vPrintString("  [DataProc] Task started\r\n");

    /* 标记数据处理任务就绪 */
    xEventGroupSetBits(xSystemEventGroup, EVENT_BIT_DATA_PROC_ENABLE);

    for(;;)
    {
        /* 等待传感器数据 */
        if(xQueueReceive(xSensorDataQueue, &xReceivedData, portMAX_DELAY) == pdPASS)
        {
            /* 检查数据处理是否启用 */
            if((xEventGroupGetBits(xSystemEventGroup) & EVENT_BIT_DATA_PROC_ENABLE) == 0)
            {
                continue;
            }

            /* 累加数据 */
            lTempSum += xReceivedData.lTemperature;
            lHumiditySum += xReceivedData.lHumidity;
            lPressureSum += xReceivedData.lPressure;
            ulSampleCount++;

            /* 每5个样本计算一次平均值 */
            if(ulSampleCount >= 5)
            {
                xLatestProcessedData.ulTimestamp = xReceivedData.ulTimestamp;
                xLatestProcessedData.lAvgTemp = lTempSum / ulSampleCount;
                xLatestProcessedData.lAvgHumidity = lHumiditySum / ulSampleCount;
                xLatestProcessedData.lAvgPressure = lPressureSum / ulSampleCount;
                xLatestProcessedData.ulSampleCount = ulSampleCount;

                /* 重置统计 */
                lTempSum = 0;
                lHumiditySum = 0;
                lPressureSum = 0;
                ulSampleCount = 0;

                /* 通知显示任务有新数据 */
                xTaskNotifyGive(xDisplayTaskHandle);
            }
        }
    }
}

/* ========================================
 * 任务3：显示/输出任务
 * ======================================== */
static void vDisplayTask(void *pvParameters)
{
    (void)pvParameters;

    vPrintString("  [Display] Task started\r\n");

    /* 标记显示任务就绪 */
    xEventGroupSetBits(xSystemEventGroup, EVENT_BIT_DISPLAY_ENABLE);

    for(;;)
    {
        /* 等待任务通知 */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        /* 检查显示是否启用 */
        if((xEventGroupGetBits(xSystemEventGroup) & EVENT_BIT_DISPLAY_ENABLE) == 0)
        {
            continue;
        }

        /* 显示处理后的数据 */
        vPrintSeparator();
        vPrintString("  [Display] Sensor Data Update\r\n");
        vPrintSeparator();
        vPrintString("  Timestamp: ");
        vPrintNumber(xLatestProcessedData.ulTimestamp);
        vPrintString("\r\n");
        vPrintString("  Temperature: ");
        vPrintNumber(xLatestProcessedData.lAvgTemp / 10);
        vPrintString(".");
        vPrintNumber(xLatestProcessedData.lAvgTemp % 10);
        vPrintString(" C\r\n");
        vPrintString("  Humidity: ");
        vPrintNumber(xLatestProcessedData.lAvgHumidity / 10);
        vPrintString(".");
        vPrintNumber(xLatestProcessedData.lAvgHumidity % 10);
        vPrintString(" %\r\n");
        vPrintString("  Pressure: ");
        vPrintNumber(xLatestProcessedData.lAvgPressure);
        vPrintString(" hPa\r\n");
        vPrintString("  Samples: ");
        vPrintNumber(xLatestProcessedData.ulSampleCount);
        vPrintString("\r\n");
        vPrintSeparator();
    }
}

/* ========================================
 * 任务4：命令处理任务
 * ======================================== */
static void vCommandHandlerTask(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulCommandCount = 0;
    EventBits_t uxBits;

    vPrintString("  [CmdHandler] Task started\r\n");

    /* 标记系统运行，启用所有任务 */
    xEventGroupSetBits(xSystemEventGroup, EVENT_BIT_SYSTEM_RUNNING | EVENT_BIT_ALL_TASKS_READY);

    for(;;)
    {
        ulCommandCount++;

        vPrintString("\r\n  [CmdHandler] Processing command cycle #");
        vPrintNumber(ulCommandCount);
        vPrintString("\r\n");

        /* 根据命令周期切换任务状态 */
        switch(ulCommandCount % 4)
        {
            case 0:
                /* 全部启用 */
                vPrintString("  [CmdHandler] Enabling all tasks\r\n");
                xEventGroupSetBits(xSystemEventGroup, EVENT_BIT_ALL_TASKS_READY);
                break;

            case 1:
                /* 暂停显示 */
                vPrintString("  [CmdHandler] Pausing display\r\n");
                xEventGroupClearBits(xSystemEventGroup, EVENT_BIT_DISPLAY_ENABLE);
                break;

            case 2:
                /* 恢复显示，暂停数据处理 */
                vPrintString("  [CmdHandler] Resuming display, pausing data processing\r\n");
                xEventGroupSetBits(xSystemEventGroup, EVENT_BIT_DISPLAY_ENABLE);
                xEventGroupClearBits(xSystemEventGroup, EVENT_BIT_DATA_PROC_ENABLE);
                break;

            case 3:
                /* 恢复全部 */
                vPrintString("  [CmdHandler] Resuming all tasks\r\n");
                xEventGroupSetBits(xSystemEventGroup, EVENT_BIT_ALL_TASKS_READY);
                break;
        }

        /* 显示当前状态 */
        uxBits = xEventGroupGetBits(xSystemEventGroup);
        vPrintString("  [CmdHandler] Current state: ");
        if(uxBits & EVENT_BIT_SENSOR_ENABLE)
            vPrintString("SENSOR ");
        if(uxBits & EVENT_BIT_DATA_PROC_ENABLE)
            vPrintString("DATAPROC ");
        if(uxBits & EVENT_BIT_DISPLAY_ENABLE)
            vPrintString("DISPLAY");
        vPrintString("\r\n");

        /* 等待一段时间再处理下一个命令 */
        vTaskDelay(pdMS_TO_TICKS(DELAY_LONG_MS * 3));
    }
}

/* ========================================
 * 任务5：工作任务
 * ======================================== */
static void vWorkerTask(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulCount = 0;

    vPrintString("  [Worker] Background task started\r\n");

    for(;;)
    {
        ulCount++;
        if((ulCount % 20) == 0)
        {
            vPrintString("  [Worker] Running... loop #");
            vPrintNumber(ulCount);
            vPrintString("\r\n");
        }
        vTaskDelay(pdMS_TO_TICKS(DELAY_SHORT_MS));
    }
}

/* ========================================
 * 主函数
 * ======================================== */
int main(void)
{
    vUARTInit();

    vPrintString("\r\n");
    vPrintSeparator();
    vPrintString("  FreeRTOS Comprehensive Demo\r\n");
    vPrintSeparator();
    vPrintString("\r\n");

    vPrintString("Learning objectives:\r\n");
    vPrintString("1. Sensor data collection (software timers)\r\n");
    vPrintString("2. Data processing (queue communication)\r\n");
    vPrintString("3. Display updates (task notifications)\r\n");
    vPrintString("4. Command handling (event groups)\r\n");
    vPrintString("5. Multi-task coordination\r\n");
    vPrintString("\r\n");

    /* 创建队列 */
    vPrintString("Creating queues...\r\n");
    xSensorDataQueue = xQueueCreate(DATA_QUEUE_LENGTH, DATA_QUEUE_ITEM_SIZE);
    if(xSensorDataQueue == NULL)
    {
        vPrintString("Error creating sensor data queue!\r\n");
        for(;;);
    }
    vPrintString("  - Sensor data queue OK\r\n");

    /* 创建事件组 */
    vPrintString("\r\nCreating event groups...\r\n");
    xSystemEventGroup = xEventGroupCreate();
    if(xSystemEventGroup == NULL)
    {
        vPrintString("Error creating system event group!\r\n");
        for(;;);
    }
    vPrintString("  - System event group OK\r\n");

    /* 创建软件定时器 */
    vPrintString("\r\nCreating software timers...\r\n");
    xSensorTimer = xTimerCreate("SensorTimer",
                                  pdMS_TO_TICKS(SENSOR_TIMER_PERIOD_MS),
                                  pdTRUE,
                                  NULL,
                                  vSensorTimerCallback);
    if(xSensorTimer == NULL)
    {
        vPrintString("Error creating sensor timer!\r\n");
        for(;;);
    }
    vPrintString("  - Sensor timer OK (500ms)\r\n");

    xDisplayTimer = xTimerCreate("DisplayTimer",
                                   pdMS_TO_TICKS(DISPLAY_UPDATE_TIMER_PERIOD_MS),
                                   pdTRUE,
                                   NULL,
                                   vDisplayTimerCallback);
    if(xDisplayTimer == NULL)
    {
        vPrintString("Error creating display timer!\r\n");
        for(;;);
    }
    vPrintString("  - Display timer OK (2000ms)\r\n");

    /* 创建任务 */
    vPrintString("\r\nCreating tasks...\r\n");

    xTaskCreate(vSensorTask, "Sensor", tskSTACK_SIZE, NULL, tskPRIORITY_SENSOR, NULL);
    vPrintString("  - Sensor task (prio 3)\r\n");

    xTaskCreate(vDataProcessingTask, "DataProc", tskSTACK_SIZE, NULL, tskPRIORITY_DATA_PROC, NULL);
    vPrintString("  - Data processing task (prio 3)\r\n");

    xTaskCreate(vDisplayTask, "Display", tskSTACK_SIZE, NULL, tskPRIORITY_DISPLAY, &xDisplayTaskHandle);
    vPrintString("  - Display task (prio 4)\r\n");

    xTaskCreate(vCommandHandlerTask, "CmdHandler", tskSTACK_SIZE, NULL, tskPRIORITY_CMD_HANDLER, NULL);
    vPrintString("  - Command handler task (prio 5)\r\n");

    xTaskCreate(vWorkerTask, "Worker", tskSTACK_SIZE, NULL, tskPRIORITY_WORKER, NULL);
    vPrintString("  - Worker task (prio 2)\r\n");

    vPrintString("\r\nAll tasks created!\r\n");
    vPrintString("\r\nStarting timers...\r\n");

    /* 启动定时器 */
    xTimerStart(xSensorTimer, 0);
    vPrintString("  - Sensor timer started\r\n");

    xTimerStart(xDisplayTimer, 0);
    vPrintString("  - Display timer started\r\n");

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