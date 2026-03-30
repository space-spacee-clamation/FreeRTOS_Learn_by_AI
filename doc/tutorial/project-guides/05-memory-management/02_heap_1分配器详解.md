# heap_1分配器详解

## heap_1是什么？

heap_1是FreeRTOS提供的最简单的内存分配器。

### 核心特点

1. ✓ **实现极其简单** - 代码量最少
2. ✓ **没有内存碎片** - 从不释放内存
3. ✓ **执行时间确定** - 分配时间总是一样的
4. ✗ **不支持内存释放** - vPortFree() 什么都不做

## heap_1的工作原理

### 基本思路

heap_1使用一个简单的**指针**来跟踪下一个可用的内存位置：

```
初始化：
堆：[ ][ ][ ][ ][ ][ ][ ][ ]
      ↑
   pxNextFreeByte（指向开始）

分配100字节：
堆：[100字节已用][ ][ ][ ][ ]
                    ↑
                 pxNextFreeByte

再分配200字节：
堆：[100字节][200字节][ ][ ]
                              ↑
                           pxNextFreeByte
```

### 数据结构

heap_1的核心数据结构非常简单：

```c
/* 堆内存数组 - 在FreeRTOSConfig.h中定义大小 */
static uint8_t ucHeap[configTOTAL_HEAP_SIZE];

/* 指向下一个可用字节的指针 */
static size_t xNextFreeByte = (size_t)0;
```

就这么简单！只有一个数组和一个指针。

## 分配算法详解

### pvPortMalloc() 的工作步骤

当调用 `pvPortMalloc(xWantedSize)` 时：

```
1. 检查需要的大小是否为0
   ↓
2. 字节对齐（通常是8字节对齐）
   ↓
3. 检查是否有足够的空间
   (xNextFreeByte + xWantedSize) <= configTOTAL_HEAP_SIZE ?
   ↓
4. 如果有空间：
   - 记录分配位置：pvReturn = &(ucHeap[xNextFreeByte])
   - 移动指针：xNextFreeByte += xWantedSize
   - 返回指针
   ↓
5. 如果没有空间：
   - 返回NULL
   - 调用vApplicationMallocFailedHook()（如果启用）
```

### 图解分配过程

```
初始状态：
ucHeap:  [ ][ ][ ][ ][ ][ ][ ][ ][ ][ ]  (10字节)
           0  1  2  3  4  5  6  7  8  9
           ↑
        xNextFreeByte = 0

调用 pvPortMalloc(3):

1. 对齐（假设不需要对齐）
2. 检查：0 + 3 <= 10 ✓
3. 分配：
   pvReturn = &ucHeap[0]
   xNextFreeByte = 0 + 3 = 3

结果：
ucHeap:  [###][ ][ ][ ][ ][ ][ ]
           0  1  2  3  4  5  6  7  8  9
                       ↑
                    xNextFreeByte = 3

再调用 pvPortMalloc(4):

1. 检查：3 + 4 <= 10 ✓
2. 分配：
   pvReturn = &ucHeap[3]
   xNextFreeByte = 3 + 4 = 7

结果：
ucHeap:  [###][####][ ][ ]
           0  1  2  3  4  5  6  7  8  9
                                   ↑
                                xNextFreeByte = 7
```

### 字节对齐

**为什么需要对齐？**

- 处理器访问对齐的数据更快
- 某些处理器要求数据必须对齐

**heap_1的对齐处理：**

```c
/* 确保分配的地址是8字节对齐的 */
#define portBYTE_ALIGNMENT    8
#define portBYTE_ALIGNMENT_MASK (0x0007)

xWantedSize += (portBYTE_ALIGNMENT - 1);
xWantedSize &= ~portBYTE_ALIGNMENT_MASK;
```

**例子：**

```
请求3字节：
3 + 7 = 10
10 & ~7 = 8
实际分配8字节（有5字节的内部碎片）

请求9字节：
9 + 7 = 16
16 & ~7 = 16
实际分配16字节（有7字节的内部碎片）
```

## vPortFree() - 什么都不做！

### heap_1的释放函数

```c
void vPortFree(void *pv)
{
    /* heap_1: 什么都不做！*/
    (void)pv;  /* 抑制未使用参数警告 */
}
```

### 为什么不支持释放？

heap_1的设计目标是**简单**和**确定**：

1. **不需要跟踪已分配的块** - 不需要复杂的数据结构
2. **不会产生碎片** - 不释放就不会有空隙
3. **代码量最小** - 易于理解和验证

### 这意味着什么？

```c
void vBadExample(void)
{
    void *pvBuffer;

    for(;;)
    {
        pvBuffer = pvPortMalloc(100);  /* 分配 */

        useBuffer(pvBuffer);

        vPortFree(pvBuffer);  /* 这个调用什么都不做！*/

        /* 下次循环又分配100字节 */
        /* 堆会越来越满，最终耗尽！*/
    }
}
```

## heap_1的优缺点分析

### 优点

| 优点 | 说明 |
|------|------|
| **实现简单** | 代码量最少，易于理解和审计 |
| **没有碎片** | 不释放就不会有外部碎片 |
| **时间确定** | 分配时间总是O(1) |
| **线程安全** | 有临界区保护 |
| ** ROM占用小** | 代码量小，适合ROM受限的系统 |

### 缺点

| 缺点 | 说明 |
|------|------|
| **不支持释放** | 内存用了就不能回收 |
| **可能浪费** | 对齐会有内部碎片 |
| **不灵活** | 只适合特定场景 |
| **长期运行** | 长时间运行可能耗尽内存 |

## heap_1的源代码（简化版）

让我们看看heap_1的实际代码（简化注释版）：

```c
/* heap_1.c - 简化注释版 */

#include <stdlib.h>

/* 定义堆内存数组 */
#if( configAPPLICATION_ALLOCATED_HEAP == 1 )
    extern uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
#else
    static uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
#endif

/* 下一个可用字节的位置 */
static size_t xNextFreeByte = ( size_t ) 0;

void *pvPortMalloc( size_t xWantedSize )
{
    void *pvReturn = NULL;

    /* 进入临界区 */
    vTaskSuspendAll();
    {
        /* 字节对齐 */
        if( ( xWantedSize & portBYTE_ALIGNMENT_MASK ) != 0x00 )
        {
            xWantedSize += ( portBYTE_ALIGNMENT - ( xWantedSize & portBYTE_ALIGNMENT_MASK ) );
        }

        /* 检查空间 */
        if( ( xNextFreeByte + xWantedSize ) <= configTOTAL_HEAP_SIZE )
        {
            /* 分配成功 */
            pvReturn = &( ucHeap[ xNextFreeByte ] );
            xNextFreeByte += xWantedSize;
        }
    }
    ( void ) xTaskResumeAll();

    /* 失败钩子 */
    if( pvReturn == NULL )
    {
        vApplicationMallocFailedHook();
    }

    return pvReturn;
}

void vPortFree( void *pv )
{
    /* heap_1: 什么都不做！*/
    ( void ) pv;
}

size_t xPortGetFreeHeapSize( void )
{
    return ( configTOTAL_HEAP_SIZE - xNextFreeByte );
}
```

这就是完整的heap_1！非常简单。

## heap_1的初始化

heap_1不需要显式初始化。第一次调用`pvPortMalloc()`时：

1. `xNextFreeByte` 已经初始化为0
2. 直接开始分配

FreeRTOS启动时会为空闲任务分配堆栈，这通常是第一次使用堆。

## 实际应用中的内存布局

假设 `configTOTAL_HEAP_SIZE = 50*1024 = 51200` 字节：

```
系统启动后：
ucHeap[0..] = [空闲任务堆栈][...]
                                  ↑
                            xNextFreeByte

创建任务后：
ucHeap[0..] = [空闲任务][任务1堆栈][任务2堆栈][队列][...]
                                                              ↑
                                                         xNextFreeByte

你的应用分配后：
ucHeap[0..] = [内核对象...][你的分配1][你的分配2][...]
                                                                      ↑
                                                                 xNextFreeByte
```

## 调试技巧

### 查看内存使用情况

```c
void vPrintMemoryInfo(void)
{
    size_t xFree = xPortGetFreeHeapSize();
    size_t xUsed = configTOTAL_HEAP_SIZE - xFree;

    printf("Total: %u\r\n", (unsigned int)configTOTAL_HEAP_SIZE);
    printf("Used:  %u\r\n", (unsigned int)xUsed);
    printf("Free:  %u\r\n", (unsigned int)xFree);
}
```

### 跟踪xNextFreeByte

虽然xNextFreeByte是静态变量，但你可以在调试器中监视它。

## 总结

- **heap_1** = 最简单的分配器
- **核心思想** = 一个指针，线性分配
- **主要限制** = 不支持释放
- **适用场景** = 启动时分配所有内存
- **优点** = 简单、无碎片、确定

## 思考问题

1. heap_1使用什么数据结构来管理内存？
2. 为什么heap_1没有外部碎片？
3. 字节对齐的目的是什么？
4. 如果在heap_1中频繁分配和释放会发生什么？

---

*参考资料：FreeRTOS官方文档 v10.x - heap_1实现*
