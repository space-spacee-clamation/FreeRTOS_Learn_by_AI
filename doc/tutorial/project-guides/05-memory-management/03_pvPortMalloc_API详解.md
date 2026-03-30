# pvPortMalloc API详解

## pvPortMalloc() - 分配内存

### 函数原型

```c
void *pvPortMalloc( size_t xWantedSize );
```

### 头文件

```c
#include "FreeRTOS.h"
```

### 参数说明

| 参数 | 类型 | 说明 |
|------|------|------|
| `xWantedSize` | `size_t` | 需要分配的字节数 |

### 返回值

| 返回值 | 说明 |
|--------|------|
| 非NULL | 分配成功，返回指向分配内存的指针 |
| NULL | 分配失败，内存不足 |

## 基本用法

### 最简单的例子

```c
void vSimpleExample(void)
{
    char *pcBuffer;

    /* 分配100字节 */
    pcBuffer = pvPortMalloc(100);

    if(pcBuffer != NULL)
    {
        /* 分配成功，使用内存 */
        strcpy(pcBuffer, "Hello FreeRTOS!");

        /* ... 使用pcBuffer ... */

        /* 注意：heap_1不支持释放！*/
        /* vPortFree(pcBuffer); - 这个调用没用 */
    }
    else
    {
        /* 分配失败 */
        /* 处理错误 */
    }
}
```

### 分配不同类型的数据

```c
void vDifferentTypes(void)
{
    /* 分配整数 */
    uint32_t *pulValue;
    pulValue = pvPortMalloc(sizeof(uint32_t));

    if(pulValue != NULL)
    {
        *pulValue = 12345;
    }

    /* 分配数组 */
    uint16_t *pusArray;
    pusArray = pvPortMalloc(10 * sizeof(uint16_t));

    if(pusArray != NULL)
    {
        for(int i = 0; i < 10; i++)
        {
            pusArray[i] = i;
        }
    }

    /* 分配结构体 */
    typedef struct {
        char name[20];
        uint32_t id;
        float value;
    } Data_t;

    Data_t *pxData;
    pxData = pvPortMalloc(sizeof(Data_t));

    if(pxData != NULL)
    {
        strcpy(pxData->name, "Sensor");
        pxData->id = 1;
        pxData->value = 25.5f;
    }
}
```

## 分配失败的处理

### 检查返回值

```c
void vCheckReturnValue(void)
{
    void *pvBuffer;

    pvBuffer = pvPortMalloc(1000);

    if(pvBuffer == NULL)
    {
        /* 分配失败！*/
        /* 这里要小心：不能使用pvPortMalloc来分配错误处理的内存 */

        /* 选项1：使用预先分配的缓冲区 */
        /* 选项2：进入安全状态 */
        /* 选项3：重置系统 */

        vPrintString("ERROR: Out of memory!\r\n");

        for(;;)
        {
            /* 死循环或其他错误处理 */
        }
    }
    else
    {
        /* 分配成功 */
        useBuffer(pvBuffer);
    }
}
```

### 使用 malloc failed 钩子

在FreeRTOSConfig.h中启用：

```c
#define configUSE_MALLOC_FAILED_HOOK    1
```

然后实现钩子函数：

```c
void vApplicationMallocFailedHook(void)
{
    /* 内存分配失败时会调用这个函数 */

    vPrintString("MALLOC FAILED!\r\n");
    vPrintString("Free heap size: ");
    vPrintNumber(xPortGetFreeHeapSize());
    vPrintString("\r\n");

    /* 可以在这里：
     * - 记录错误
     * - 切换到安全模式
     * - 触发系统复位
     */

    for(;;)
    {
        /* 死循环，防止继续运行 */
    }
}
```

## 实际示例：在任务中分配

### 示例1：启动时一次性分配

```c
/* 全局指针（heap_1的典型用法） */
char *g_pcTxBuffer = NULL;
char *g_pcRxBuffer = NULL;
uint8_t *g_puWorkArea = NULL;

void vInitSystem(void)
{
    /* 系统启动时，一次性分配所有需要的内存 */

    g_pcTxBuffer = pvPortMalloc(256);
    if(g_pcTxBuffer == NULL)
    {
        /* 处理错误 */
    }

    g_pcRxBuffer = pvPortMalloc(256);
    if(g_pcRxBuffer == NULL)
    {
        /* 处理错误 */
    }

    g_puWorkArea = pvPortMalloc(1024);
    if(g_puWorkArea == NULL)
    {
        /* 处理错误 */
    }

    /* 分配完成，打印状态 */
    vPrintMemoryStats();
}

void vTaskFunction(void *pvParameters)
{
    /* 任务中使用已经分配好的内存 */
    for(;;)
    {
        /* 使用 g_pcTxBuffer */
        /* 使用 g_pcRxBuffer */
        /* 使用 g_puWorkArea */

        /* 注意：不要在任务循环中调用 pvPortMalloc！*/
        /* heap_1不支持释放，内存会耗尽 */

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
```

### 示例2：创建内核对象

```c
/* 注意：创建任务、队列等时，FreeRTOS内部会调用pvPortMalloc */

void vCreateKernelObjects(void)
{
    TaskHandle_t xTaskHandle;
    QueueHandle_t xQueue;

    /* 创建任务 - 内部会分配任务堆栈和TCB */
    xTaskCreate(vTaskFunc, "Task", 128, NULL, 1, &xTaskHandle);

    /* 创建队列 - 内部会分配队列存储 */
    xQueue = xQueueCreate(10, sizeof(uint32_t));

    /* 这些分配都使用heap_1，创建后不要删除！*/
}
```

## 常见错误

### 错误1：不检查返回值

```c
/* ✗ 错误：不检查返回值 */
void vBadExample1(void)
{
    char *pcBuffer = pvPortMalloc(100);
    strcpy(pcBuffer, "Hello");  /* 如果分配失败，这里会崩溃！*/
}

/* ✓ 正确：总是检查返回值 */
void vGoodExample1(void)
{
    char *pcBuffer = pvPortMalloc(100);

    if(pcBuffer != NULL)
    {
        strcpy(pcBuffer, "Hello");
    }
    else
    {
        /* 处理错误 */
    }
}
```

### 错误2：在heap_1中频繁分配释放

```c
/* ✗ 错误：heap_1中这样做会耗尽内存 */
void vBadExample2(void)
{
    for(;;)
    {
        void *pv = pvPortMalloc(100);  /* 分配 */
        use(pv);
        vPortFree(pv);  /* heap_1中这个调用没用！*/
        /* 下次循环又分配，内存越来越少 */
    }
}

/* ✓ 正确：heap_1中只在启动时分配一次 */
void *g_pvBuffer = NULL;

void vGoodExample2_Init(void)
{
    g_pvBuffer = pvPortMalloc(100);  /* 启动时分配一次 */
}

void vGoodExample2_Runtime(void)
{
    for(;;)
    {
        use(g_pvBuffer);  /* 重复使用 */
        vTaskDelay(100);
    }
}
```

### 错误3：分配大小计算错误

```c
/* ✗ 错误：忘记乘以元素大小 */
uint32_t *pulArray = pvPortMalloc(10);  /* 只分配了10字节！*/

/* ✓ 正确：使用sizeof */
uint32_t *pulArray = pvPortMalloc(10 * sizeof(uint32_t));  /* 正确 */
```

## 使用sizeof的最佳实践

```c
/* ✓ 好：使用变量类型 */
uint32_t *pulValue;
pulValue = pvPortMalloc(sizeof(*pulValue));

/* ✓ 好：数组 */
uint16_t *pusArray;
pusArray = pvPortMalloc(10 * sizeof(*pusArray));

/* ✓ 好：结构体 */
typedef struct {
    int a;
    float b;
} MyStruct_t;

MyStruct_t *pxStruct;
pxStruct = pvPortMalloc(sizeof(*pxStruct));
```

## 线程安全

pvPortMalloc() 是线程安全的！

内部实现：

```c
void *pvPortMalloc(size_t xWantedSize)
{
    void *pvReturn = NULL;

    /* 挂起所有任务（进入临界区） */
    vTaskSuspendAll();
    {
        /* 执行分配... */
    }
    /* 恢复调度 */
    xTaskResumeAll();

    return pvReturn;
}
```

这意味着：
- ✓ 可以在多个任务中同时调用
- ✓ 不需要额外的互斥锁
- ✓ 线程安全

## 总结

- **pvPortMalloc()** = 分配内存
- **总是检查返回值** = 不要假设分配成功
- **heap_1中只在启动时分配** = 不要运行时频繁分配
- **使用sizeof** = 避免大小计算错误
- **线程安全** = 可以在多任务中使用

## 思考问题

1. 为什么要检查pvPortMalloc()的返回值？
2. sizeof(*p)和sizeof(type)有什么区别？哪个更好？
3. pvPortMalloc()是线程安全的吗？为什么？
4. 在heap_1中，应该在什么时候分配内存？

---

*参考资料：FreeRTOS官方文档 v10.x - 内存分配API*
