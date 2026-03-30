# 内存统计API详解

## xPortGetFreeHeapSize() - 获取剩余堆大小

### 函数原型

```c
size_t xPortGetFreeHeapSize( void );
```

### 头文件

```c
#include "FreeRTOS.h"
```

### 返回值

| 返回值 | 说明 |
|--------|------|
| `size_t` | 当前堆中剩余的字节数 |

### 基本用法

```c
void vExample(void)
{
    size_t xFreeSize;

    /* 获取剩余堆大小 */
    xFreeSize = xPortGetFreeHeapSize();

    vPrintString("Free heap: ");
    vPrintNumber(xFreeSize);
    vPrintString(" bytes\r\n");
}
```

## heap_1的统计功能

### heap_1只支持xPortGetFreeHeapSize()

```
heap_1 支持的统计：
✓ xPortGetFreeHeapSize() - 当前剩余大小

heap_1 不支持的统计：
✗ xPortGetMinimumEverFreeHeapSize() - 历史最小剩余（heap_2/4/5支持）
```

### 为什么heap_1不支持历史最小值？

因为heap_1**从不释放内存**，剩余大小只会减少，不会增加：

```
时间轴：
T0: [剩余: 50000]  ← 最大值
T1: [剩余: 49000]  ← 分配了1000
T2: [剩余: 48000]  ← 又分配了1000
T3: [剩余: 47500]  ← 又分配了500
...
（只会越来越小）
```

所以历史最小值就是当前值，不需要单独统计。

## 完整示例：监控内存使用

### 示例1：简单的内存状态打印

```c
void vPrintMemoryStats(void)
{
    size_t xFreeSize;
    size_t xUsedSize;

    /* 获取剩余大小 */
    xFreeSize = xPortGetFreeHeapSize();

    /* 计算已使用大小 */
    xUsedSize = configTOTAL_HEAP_SIZE - xFreeSize;

    vPrintString("\r\n========== 内存状态 ==========\r\n");
    vPrintString("总大小:    ");
    vPrintNumber(configTOTAL_HEAP_SIZE);
    vPrintString(" 字节\r\n");

    vPrintString("已使用:    ");
    vPrintNumber(xUsedSize);
    vPrintString(" 字节\r\n");

    vPrintString("剩余:      ");
    vPrintNumber(xFreeSize);
    vPrintString(" 字节\r\n");

    vPrintString("使用率:    ");
    vPrintNumber((xUsedSize * 100) / configTOTAL_HEAP_SIZE);
    vPrintString("%\r\n");
    vPrintString("===============================\r\n\r\n");
}
```

### 示例2：在任务中定期监控

```c
void vMemoryMonitorTask(void *pvParameters)
{
    (void)pvParameters;

    for(;;)
    {
        /* 打印内存状态 */
        vPrintMemoryStats();

        /* 每5秒监控一次 */
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
```

### 示例3：分配前后对比

```c
void vDemoAllocation(void)
{
    size_t xFreeBefore, xFreeAfter;
    void *pvBuffer;

    /* 分配前 */
    xFreeBefore = xPortGetFreeHeapSize();
    vPrintString("分配前剩余: ");
    vPrintNumber(xFreeBefore);
    vPrintString("\r\n");

    /* 分配内存 */
    pvBuffer = pvPortMalloc(1000);

    if(pvBuffer != NULL)
    {
        /* 分配后 */
        xFreeAfter = xPortGetFreeHeapSize();
        vPrintString("分配后剩余: ");
        vPrintNumber(xFreeAfter);
        vPrintString("\r\n");

        /* 计算实际分配大小（包括对齐开销） */
        vPrintString("实际分配:   ");
        vPrintNumber(xFreeBefore - xFreeAfter);
        vPrintString(" 字节\r\n");
    }
}
```

## 实际项目中的监控策略

### 启动时检查

```c
void vInitCheck(void)
{
    size_t xFree;

    /* 系统初始化后检查 */
    xFree = xPortGetFreeHeapSize();

    vPrintString("系统启动完成，剩余堆: ");
    vPrintNumber(xFree);
    vPrintString(" 字节\r\n");

    /* 检查是否满足最小要求 */
    if(xFree < 5000)  /* 假设至少需要5000字节余量 */
    {
        vPrintString("警告：堆余量不足！\r\n");
    }
}
```

### 低内存警告

```c
/* 低内存阈值 */
#define LOW_MEMORY_THRESHOLD    2048  /* 2KB */

void vCheckLowMemory(void)
{
    size_t xFree = xPortGetFreeHeapSize();

    if(xFree < LOW_MEMORY_THRESHOLD)
    {
        vPrintString("警告：低内存！剩余: ");
        vPrintNumber(xFree);
        vPrintString(" 字节\r\n");

        /* 可以在这里：
         * - 发出警告
         * - 进入安全模式
         * - 关闭非必要功能
         */
    }
}
```

## 理解实际分配大小

### 对齐开销

heap_1会进行字节对齐，实际分配的可能比请求的多：

```c
void vShowAlignmentOverhead(void)
{
    size_t xFree1, xFree2, xFree3;
    void *pv1, *pv2, *pv3;

    /* 分配1字节 */
    xFree1 = xPortGetFreeHeapSize();
    pv1 = pvPortMalloc(1);
    xFree2 = xPortGetFreeHeapSize();

    vPrintString("请求 1 字节，实际分配 ");
    vPrintNumber(xFree1 - xFree2);
    vPrintString(" 字节\r\n");

    /* 分配3字节 */
    xFree2 = xPortGetFreeHeapSize();
    pv2 = pvPortMalloc(3);
    xFree3 = xPortGetFreeHeapSize();

    vPrintString("请求 3 字节，实际分配 ");
    vPrintNumber(xFree2 - xFree3);
    vPrintString(" 字节\r\n");

    /* 分配9字节 */
    xFree3 = xPortGetFreeHeapSize();
    pv3 = pvPortMalloc(9);
    xFree1 = xPortGetFreeHeapSize();  /* 复用变量 */

    vPrintString("请求 9 字节，实际分配 ");
    vPrintNumber(xFree3 - xFree1);
    vPrintString(" 字节\r\n");
}
```

可能的输出（假设8字节对齐）：

```
请求 1 字节，实际分配 8 字节
请求 3 字节，实际分配 8 字节
请求 9 字节，实际分配 16 字节
```

## 调试技巧

### 跟踪内存使用

```c
/* 在关键位置打印内存状态 */
void vDebugMemoryCheck(const char *pcLocation)
{
    vPrintString("[");
    vPrintString(pcLocation);
    vPrintString("] Free heap: ");
    vPrintNumber(xPortGetFreeHeapSize());
    vPrintString("\r\n");
}

/* 使用示例 */
void vMyFunction(void)
{
    vDebugMemoryCheck("Start");

    doSomething();
    vDebugMemoryCheck("After doSomething");

    pvBuffer = pvPortMalloc(100);
    vDebugMemoryCheck("After malloc");

    /* ... */
}
```

### 检测泄漏（heap_1特有的"泄漏"）

在heap_1中，任何分配都是永久性的：

```c
/* 在heap_1中，这个函数每次调用都会"泄漏"100字节 */
void vHeap1LeakExample(void)
{
    void *pv = pvPortMalloc(100);
    /* 没有vPortFree，或者vPortFree没用 */
    /* 这个内存永远不会回来 */
}
```

所以在heap_1中：

```c
/* ✓ 好：只在初始化时调用一次 */
void vInitOnce(void)
{
    g_pBuffer = pvPortMalloc(100);
}

/* ✗ 坏：在循环中调用 */
void vLoopFunction(void)
{
    for(;;)
    {
        void *pv = pvPortMalloc(100);  /* 每次都分配新的！*/
        vTaskDelay(100);
    }
}
```

## 其他heap分配器的统计功能（对比）

| 功能 | heap_1 | heap_2 | heap_4 | heap_5 |
|------|--------|--------|--------|--------|
| `xPortGetFreeHeapSize()` | ✓ | ✓ | ✓ | ✓ |
| `xPortGetMinimumEverFreeHeapSize()` | ✗ | ✓ | ✓ | ✓ |

这也是选择更高级heap分配器的原因之一。

## 总结

- **xPortGetFreeHeapSize()** = 获取当前剩余堆大小
- **heap_1只支持这个统计** = 没有历史最小值
- **用它来监控内存使用** = 定期检查，避免耗尽
- **注意对齐开销** = 实际分配可能比请求多
- **heap_1中所有分配都是永久的** = 小心设计

## 思考问题

1. heap_1为什么不支持xPortGetMinimumEverFreeHeapSize()？
2. 如何计算实际分配的内存（包括对齐开销）？
3. 为什么要监控内存使用情况？
4. 在heap_1中，什么是"内存泄漏"？

---

*参考资料：FreeRTOS官方文档 v10.x - 内存统计API*
