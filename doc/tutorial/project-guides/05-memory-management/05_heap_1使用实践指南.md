# heap_1使用实践指南

## 什么时候选择heap_1？

### 适合heap_1的场景

| 场景 | 说明 |
|------|------|
| **简单系统** | 只有几个任务，创建后永不删除 |
| **静态设计** | 所有资源都在启动时创建 |
| **高确定性** | 需要分配时间完全确定 |
| **长期运行** | 系统运行数月甚至数年不重启 |
| **内存充足** | 有足够的内存可以一次性分配 |

### 不适合heap_1的场景

| 场景 | 原因 |
|------|------|
| **需要动态分配释放** | heap_1不支持释放 |
| **内存受限** | 无法充分利用内存 |
| **复杂应用** | 需要灵活的内存管理 |
| **经常创建删除对象** | 内存会快速耗尽 |

## 最佳实践

### 1. 启动时分配所有内存

```c
/* 全局指针，指向所有需要的内存 */
static char *g_pcTxBuffer = NULL;
static char *g_pcRxBuffer = NULL;
static uint8_t *g_puWorkArea = NULL;
static QueueHandle_t g_xDataQueue = NULL;
static SemaphoreHandle_t g_xMutex = NULL;

void vSystemInit(void)
{
    /* ==============================================
     * 在系统启动时，一次性分配所有需要的内存
     * ==============================================
     */

    vPrintString("正在初始化系统...\r\n");

    /* 1. 分配通信缓冲区 */
    g_pcTxBuffer = pvPortMalloc(256);
    if(g_pcTxBuffer == NULL) { vErrorHandler(); }

    g_pcRxBuffer = pvPortMalloc(256);
    if(g_pcRxBuffer == NULL) { vErrorHandler(); }

    /* 2. 分配工作区 */
    g_puWorkArea = pvPortMalloc(1024);
    if(g_puWorkArea == NULL) { vErrorHandler(); }

    /* 3. 创建FreeRTOS对象（内部使用pvPortMalloc） */
    g_xDataQueue = xQueueCreate(10, sizeof(uint32_t));
    if(g_xDataQueue == NULL) { vErrorHandler(); }

    g_xMutex = xSemaphoreCreateMutex();
    if(g_xMutex == NULL) { vErrorHandler(); }

    /* 4. 创建任务（堆栈也是分配的） */
    xTaskCreate(vTask1, "Task1", 128, NULL, 2, NULL);
    xTaskCreate(vTask2, "Task2", 128, NULL, 2, NULL);
    xTaskCreate(vTask3, "Task3", 128, NULL, 1, NULL);

    /* 打印内存使用情况 */
    vPrintString("系统初始化完成！\r\n");
    vPrintMemoryStats();

    /* 检查剩余内存是否足够 */
    if(xPortGetFreeHeapSize() < 2048)
    {
        vPrintString("警告：剩余内存不足！\r\n");
    }
}

void vErrorHandler(void)
{
    vPrintString("系统初始化失败！\r\n");
    for(;;);
}
```

### 2. 运行时只使用，不分配

```c
void vTaskFunction(void *pvParameters)
{
    /* 任务启动前，所有需要的内存已经分配好了 */

    for(;;)
    {
        /* ==============================================
         * 任务循环中：
         * ✓ 可以使用已经分配的内存
         * ✗ 不要调用 pvPortMalloc()
         * ✗ 不要创建新任务/队列
         * ==============================================
         */

        /* ✓ 好：使用预分配的缓冲区 */
        processData(g_puWorkArea);
        sendMessage(g_pcTxBuffer);

        /* ✓ 好：使用预创建的队列 */
        xQueueSend(g_xDataQueue, &ulValue, 0);

        /* ✗ 坏：不要在循环中分配！*/
        /* void *pv = pvPortMalloc(100); */

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
```

### 3. 合理设置堆大小

在FreeRTOSConfig.h中：

```c
/* 根据你的需求计算需要的大小 */

/* 估算：
 * - 空闲任务堆栈:  128字 = 512字节
 * - 任务1堆栈:      128字 = 512字节
 * - 任务2堆栈:      128字 = 512字节
 * - 任务3堆栈:      128字 = 512字节
 * - 队列存储:      10*4  = 40字节
 * - TCB等内核对象:  ~500字节
 * - 应用缓冲区:    256+256+1024 = 1536字节
 * - 余量:           2048字节
 * ---------------------------------------
 * 总计:           ~6000字节
 */

/* 设置为10KB，留出足够余量 */
#define configTOTAL_HEAP_SIZE    ( ( size_t ) ( 10 * 1024 ) )
```

### 4. 考虑字节对齐的开销

```c
/* 实际分配的会比请求的多，因为对齐 */

/* 假设8字节对齐 */

/* 请求1字节，实际分配8字节（浪费7字节）*/
void *pv1 = pvPortMalloc(1);

/* 请求7字节，实际分配8字节（浪费1字节）*/
void *pv2 = pvPortMalloc(7);

/* 请求8字节，实际分配8字节（无浪费）*/
void *pv3 = pvPortMalloc(8);

/* 建议：分配时考虑对齐，或者接受一些浪费 */
```

### 5. 监控内存使用

```c
/* 定期检查内存使用情况 */

void vMemoryMonitorTask(void *pvParameters)
{
    (void)pvParameters;
    size_t xFreeSize;

    for(;;)
    {
        xFreeSize = xPortGetFreeHeapSize();

        /* 打印状态 */
        vPrintString("[Monitor] Free heap: ");
        vPrintNumber(xFreeSize);
        vPrintString("\r\n");

        /* 低内存警告 */
        if(xFreeSize < 1024)
        {
            vPrintString("[WARNING] Low memory!\r\n");
        }

        /* 每分钟检查一次 */
        vTaskDelay(pdMS_TO_TICKS(60000));
    }
}
```

## 实际项目模板

### 项目结构建议

```
project/
├── include/
│   ├── FreeRTOSConfig.h
│   └── system_config.h
├── src/
│   ├── main.c
│   ├── system_init.c    # 只在这里分配内存
│   ├── task1.c         # 只使用，不分配
│   └── task2.c         # 只使用，不分配
└── Makefile
```

### system_init.c 模板

```c
/* system_init.c - 系统初始化，一次性分配所有内存 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* ==============================================
 * 全局资源（都在这里初始化）
 * ==============================================
 */

/* 缓冲区 */
char *g_pcTxBuffer = NULL;
char *g_pcRxBuffer = NULL;
uint8_t *g_puWorkBuffer = NULL;

/* FreeRTOS对象 */
QueueHandle_t g_xSensorQueue = NULL;
QueueHandle_t g_xCommandQueue = NULL;
SemaphoreHandle_t g_xUARTMutex = NULL;

/* ==============================================
 * 初始化函数
 * ==============================================
 */

BaseType_t xSystemInit(void)
{
    /* 1. 分配缓冲区 */
    g_pcTxBuffer = pvPortMalloc(256);
    if(g_pcTxBuffer == NULL) return pdFAIL;

    g_pcRxBuffer = pvPortMalloc(256);
    if(g_pcRxBuffer == NULL) return pdFAIL;

    g_puWorkBuffer = pvPortMalloc(512);
    if(g_puWorkBuffer == NULL) return pdFAIL;

    /* 2. 创建队列 */
    g_xSensorQueue = xQueueCreate(20, sizeof(uint16_t));
    if(g_xSensorQueue == NULL) return pdFAIL;

    g_xCommandQueue = xQueueCreate(10, 32);
    if(g_xCommandQueue == NULL) return pdFAIL;

    /* 3. 创建互斥锁 */
    g_xUARTMutex = xSemaphoreCreateMutex();
    if(g_xUARTMutex == NULL) return pdFAIL;

    /* 4. 创建任务 */
    xTaskCreate(vSensorTask, "Sensor", 128, NULL, 3, NULL);
    xTaskCreate(vProcessTask, "Process", 128, NULL, 2, NULL);
    xTaskCreate(vUITask, "UI", 128, NULL, 1, NULL);

    return pdPASS;
}
```

## 常见陷阱

### 陷阱1：在运行时分配

```c
/* ✗ 错误：在任务循环中分配 */
void vBadTask(void *pvParameters)
{
    for(;;)
    {
        char *pcTemp = pvPortMalloc(100);  /* 每次都分配 */
        /* 使用... */
        vPortFree(pcTemp);  /* heap_1中没用！*/
        vTaskDelay(100);
        /* 内存越来越少... */
    }
}

/* ✓ 正确：启动时分配一次 */
char *g_pcTemp = NULL;

void vGoodInit(void)
{
    g_pcTemp = pvPortMalloc(100);  /* 只分配一次 */
}

void vGoodTask(void *pvParameters)
{
    for(;;)
    {
        /* 使用 g_pcTemp */
        vTaskDelay(100);
    }
}
```

### 陷阱2：忘记初始化所有指针

```c
/* ✗ 错误：可能使用NULL指针 */
void vBadExample(void)
{
    char *pcBuffer;

    /* 假设分配成功，但可能失败 */
    pcBuffer = pvPortMalloc(100);

    /* 直接使用！*/
    strcpy(pcBuffer, "Hello");  /* 如果分配失败会崩溃 */
}

/* ✓ 正确：检查返回值 */
void vGoodExample(void)
{
    char *pcBuffer;

    pcBuffer = pvPortMalloc(100);

    if(pcBuffer != NULL)
    {
        /* 分配成功，使用 */
        strcpy(pcBuffer, "Hello");
    }
    else
    {
        /* 分配失败，处理 */
        vErrorHandler();
    }
}
```

### 陷阱3：堆大小设置太小

```c
/* ✗ 错误：堆太小 */
#define configTOTAL_HEAP_SIZE    ( ( size_t ) ( 2 * 1024 ) )  /* 太小！*/

/* ✓ 正确：估算 + 余量 */
#define configTOTAL_HEAP_SIZE    ( ( size_t ) ( 15 * 1024 ) ) /* 足够 */
```

## 从heap_1迁移到其他heap

如果后来发现heap_1不够用，迁移到heap_4很简单：

### 步骤1：修改Makefile

```makefile
# 修改前（heap_1）
$(FREERTOS_SRC)/portable/MemMang/heap_1-lm3s6965.o

# 修改后（heap_4）
$(FREERTOS_SRC)/portable/MemMang/heap_4-lm3s6965.o
```

### 步骤2：代码修改很小

```c
/* heap_1的代码 */
void *pv = pvPortMalloc(100);
/* ... 使用 ... */
/* vPortFree(pv); - 这个调用heap_1中没用 */

/* heap_4的代码 */
void *pv = pvPortMalloc(100);
/* ... 使用 ... */
vPortFree(pv);  /* 现在这个调用有用了！*/
```

所以即使先用heap_1开始，后续迁移也很容易。

## 总结

- **启动时分配，运行时使用** = heap_1的核心模式
- **合理估算堆大小** = 预估 + 余量
- **监控内存使用** = 定期检查剩余大小
- **避免运行时分配** = 任务循环中不要调用pvPortMalloc
- **容易迁移** = 后续可以轻松切换到heap_4

## 思考问题

1. heap_1的"启动时分配"模式有什么优点？
2. 如何估算需要的堆大小？
3. 为什么要在任务循环中避免分配内存？
4. 如果heap_1不够用了，迁移到heap_4需要做什么？

---

*参考资料：FreeRTOS官方文档 v10.x - heap_1使用指南*
