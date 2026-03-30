# heap_2分配器详解

## heap_2是什么？

heap_2是FreeRTOS提供的支持内存释放的内存分配器。

### 核心特点

1. ✓ **支持内存释放** - vPortFree() 真正工作
2. ✓ **最佳适配算法** - 找刚好够用的最小块
3. ✓ **空闲块按大小排序** - 小块在前，大块在后
4. ✗ **不合并相邻空闲块** - 会产生外部碎片！
5. ✗ **没有最小历史统计** - 不支持xPortGetMinimumEverFreeHeapSize()

## heap_2的工作原理

### 基本思路

heap_2使用**链表**来管理空闲块，空闲块按**大小排序**：

```
初始化：
堆：[                  一个大块                  ]
      ↑
   空闲链表：xStart -> [大块] -> xEnd

分配100字节（最佳适配）：
堆：[100字节已用][         剩余大块         ]
                   ↑
              空闲链表：xStart -> [剩余大块] -> xEnd

再分配200字节：
堆：[100字节][200字节][      剩余      ]
                               ↑
                          空闲链表

释放中间的200字节：
堆：[100字节][  空闲200  ][      剩余      ]
                                    ↑
              空闲链表：xStart -> [200字节] -> [剩余] -> xEnd
                              （按大小排序，小块在前！）
```

### 数据结构

heap_2的核心数据结构：

```c
/* 堆内存数组 */
static uint8_t ucHeap[configTOTAL_HEAP_SIZE];

/* 块链接结构体 */
typedef struct A_BLOCK_LINK
{
    struct A_BLOCK_LINK *pxNextFreeBlock;  /* 下一个空闲块 */
    size_t xBlockSize;                      /* 块大小 */
} BlockLink_t;

/* 链表头和尾 */
static BlockLink_t xStart, xEnd;

/* 剩余字节数 */
static size_t xFreeBytesRemaining;
```

### 关键常量

```c
/* 结构体大小（对齐后） */
xHeapStructSize = (sizeof(BlockLink_t) + alignment) & ~alignment_mask;

/* 最小块大小 = 结构体大小的2倍 */
#define heapMINIMUM_BLOCK_SIZE (xHeapStructSize * 2)

/* 块分配状态位（用最高位标记） */
#define heapBLOCK_ALLOCATED_BITMASK  (1 << 31)  /* 假设32位系统 */
```

## 分配算法详解

### pvPortMalloc() 的工作步骤

当调用 `pvPortMalloc(xWantedSize)` 时：

```
1. 检查需要的大小是否为0
   ↓
2. 加上块结构体大小，字节对齐
   xWantedSize += xHeapStructSize + alignment
   ↓
3. 首次调用？初始化堆
   ↓
4. 遍历空闲链表（按大小排序）找合适的块
   - 从小块开始找
   - 找第一个 >= xWantedSize 的块
   ↓
5. 找到合适的块：
   a) 从空闲链表中移除这个块
   b) 如果块比需要的大很多（> heapMINIMUM_BLOCK_SIZE）
      - 分裂成两块
      - 一块返回给用户
      - 另一块插回空闲链表
   c) 标记块为"已分配"（设置最高位）
   d) 更新 xFreeBytesRemaining
   e) 返回指针（跳过块结构体）
   ↓
6. 没找到：返回NULL
```

### 图解分配过程

```
初始状态：
空闲链表：xStart -> [400字节] -> xEnd

调用 pvPortMalloc(100):

1. 计算实际需要 = 100 + 结构体大小 + 对齐 = 120
2. 遍历空闲链表，找到400字节块
3. 检查：400 - 120 = 280 > 最小块大小 ✓
4. 分裂：
   - 块1: 120字节（标记为已分配）
   - 块2: 280字节（保持空闲）
5. 把块2插回空闲链表
6. 返回块1的用户数据指针

结果：
堆：[块头][100字节用户数据][块头][280字节空闲]
空闲链表：xStart -> [280字节] -> xEnd
```

### 最佳适配算法的好处

```
假设空闲链表：[50][100][200][300]（按大小排序）

请求80字节：
- 先看50字节 → 不够
- 再看100字节 → 够了！
- 分配这个100字节的块（而不是更大的块）

结果：
- 大的块保留下来给后续的大请求
- 减少了"大材小用"的情况
```

## 释放算法详解

### vPortFree() 的工作步骤

当调用 `vPortFree(pv)` 时：

```
1. 检查指针是否为NULL
   ↓
2. 指针回退到块结构体位置
   pxBlock = (BlockLink_t *)((uint8_t *)pv - xHeapStructSize)
   ↓
3. 验证块确实已分配
   （检查最高位是否设置）
   ↓
4. 清除"已分配"标记
   ↓
5. 将块插入空闲链表
   - 按大小排序插入
   - 小块在前，大块在后
   ↓
6. 更新 xFreeBytesRemaining
```

### 关键：不合并相邻块！

```
释放前的堆布局：
[已分配100][已分配200][已分配150]

释放中间的200字节：
[已分配100][  空闲200  ][已分配150]
                ↑
           这个块不与左右合并！

结果：
即使这三个块在物理上是连续的，
heap_2也不会把它们合并成一个大块！
```

### 图解释放过程

```
释放前：
堆：[块A:100已用][块B:200已用][块C:150已用]
空闲链表：xStart -> [其他空闲块...] -> xEnd

调用 vPortFree(块B的用户指针):

1. 回退指针到块B的块头
2. 清除已分配标记
3. 按大小插入空闲链表

释放后：
堆：[块A:100已用][块B:200空闲][块C:150已用]
空闲链表：xStart -> [200字节] -> [其他...] -> xEnd
                                  ↑
                           按大小排序！

关键：块B没有与相邻块合并！
```

## 内存碎片问题演示

### 什么是内存碎片？

```
场景：频繁分配和释放不同大小的块

时间线：
1. 分配A(100), B(200), C(150)
   堆：[A][B][C][空闲]

2. 释放B
   堆：[A][空闲200][C][空闲]
         ↑ 碎片！

3. 尝试分配250字节
   总空闲 = 200 + 剩余空闲 = 可能足够
   但没有连续的250字节！
   分配失败！😭
```

### heap_2中的碎片演示

```c
/* 这会产生碎片 */
void vFragmentationExample(void)
{
    void *pv1 = pvPortMalloc(100);  /* 分配100 */
    void *pv2 = pvPortMalloc(200);  /* 分配200 */
    void *pv3 = pvPortMalloc(150);  /* 分配150 */

    vPortFree(pv2);  /* 释放中间的块 */

    /* 现在尝试分配250字节 */
    void *pv4 = pvPortMalloc(250);
    /* 可能失败！即使总空闲空间足够 */
}
```

### 碎片的后果

| 问题 | 说明 |
|------|------|
| **分配失败** | 总空间足够，但没有连续块 |
| **内存浪费** | 碎片空间无法有效利用 |
| **性能下降** | 需要遍历更多小块 |
| **系统不稳定** | 长时间运行后可能无法分配 |

## heap_2的源代码分析

### 核心数据结构

```c
typedef struct A_BLOCK_LINK
{
    struct A_BLOCK_LINK *pxNextFreeBlock;
    size_t xBlockSize;
} BlockLink_t;

/* 最高位用作分配标记 */
#define heapBLOCK_ALLOCATED_BITMASK  ((size_t)1 << ((sizeof(size_t) * 8) - 1))
#define heapBLOCK_IS_ALLOCATED(pxBlock)  (((pxBlock->xBlockSize) & heapBLOCK_ALLOCATED_BITMASK) != 0)
#define heapALLOCATE_BLOCK(pxBlock)      ((pxBlock->xBlockSize) |= heapBLOCK_ALLOCATED_BITMASK)
#define heapFREE_BLOCK(pxBlock)          ((pxBlock->xBlockSize) &= ~heapBLOCK_ALLOCATED_BITMASK)
```

### 插入空闲块（按大小排序）

```c
#define prvInsertBlockIntoFreeList(pxBlockToInsert)                          \
{                                                                              \
    BlockLink_t *pxIterator;                                                  \
    size_t xBlockSize = pxBlockToInsert->xBlockSize;                         \
                                                                              \
    /* 找到合适的位置（按大小排序） */                                        \
    for(pxIterator = &xStart;                                                \
        pxIterator->pxNextFreeBlock->xBlockSize < xBlockSize;               \
        pxIterator = pxIterator->pxNextFreeBlock)                            \
    {                                                                          \
        /* 继续找 */                                                          \
    }                                                                          \
                                                                              \
    /* 插入 */                                                                \
    pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock;        \
    pxIterator->pxNextFreeBlock = pxBlockToInsert;                           \
}
```

### 分配函数（简化版）

```c
void *pvPortMalloc(size_t xWantedSize)
{
    BlockLink_t *pxBlock, *pxPreviousBlock, *pxNewBlockLink;
    void *pvReturn = NULL;

    vTaskSuspendAll();
    {
        /* 首次调用？初始化 */
        if(xHeapHasBeenInitialised == pdFALSE)
        {
            prvHeapInit();
            xHeapHasBeenInitialised = pdTRUE;
        }

        if(xWantedSize > 0)
        {
            /* 加上块头大小 */
            xWantedSize += xHeapStructSize;

            /* 字节对齐 */
            if((xWantedSize & portBYTE_ALIGNMENT_MASK) != 0)
            {
                xWantedSize += (portBYTE_ALIGNMENT - (xWantedSize & portBYTE_ALIGNMENT_MASK));
            }

            /* 找合适的块 */
            if((xWantedSize > 0) && (xWantedSize <= xFreeBytesRemaining))
            {
                pxPreviousBlock = &xStart;
                pxBlock = xStart.pxNextFreeBlock;

                /* 遍历（从小块开始） */
                while((pxBlock->xBlockSize < xWantedSize) && 
                      (pxBlock->pxNextFreeBlock != NULL))
                {
                    pxPreviousBlock = pxBlock;
                    pxBlock = pxBlock->pxNextFreeBlock;
                }

                if(pxBlock != &xEnd)
                {
                    /* 找到块，返回用户数据指针 */
                    pvReturn = (void *)(((uint8_t *)pxPreviousBlock->pxNextFreeBlock) + xHeapStructSize);

                    /* 从链表移除 */
                    pxPreviousBlock->pxNextFreeBlock = pxBlock->pxNextFreeBlock;

                    /* 块太大？分裂 */
                    if((pxBlock->xBlockSize - xWantedSize) > heapMINIMUM_BLOCK_SIZE)
                    {
                        pxNewBlockLink = (void *)(((uint8_t *)pxBlock) + xWantedSize);
                        pxNewBlockLink->xBlockSize = pxBlock->xBlockSize - xWantedSize;
                        pxBlock->xBlockSize = xWantedSize;

                        /* 插入剩余部分 */
                        prvInsertBlockIntoFreeList(pxNewBlockLink);
                    }

                    xFreeBytesRemaining -= pxBlock->xBlockSize;

                    /* 标记为已分配 */
                    heapALLOCATE_BLOCK(pxBlock);
                    pxBlock->pxNextFreeBlock = NULL;
                }
            }
        }
    }
    xTaskResumeAll();

    return pvReturn;
}
```

### 释放函数（简化版）

```c
void vPortFree(void *pv)
{
    uint8_t *puc = (uint8_t *)pv;
    BlockLink_t *pxLink;

    if(pv != NULL)
    {
        /* 回退到块头 */
        puc -= xHeapStructSize;
        pxLink = (void *)puc;

        /* 确认已分配 */
        configASSERT(heapBLOCK_IS_ALLOCATED(pxLink) != 0);

        if(heapBLOCK_IS_ALLOCATED(pxLink) != 0)
        {
            /* 清除分配标记 */
            heapFREE_BLOCK(pxLink);

            vTaskSuspendAll();
            {
                /* 插入空闲链表（按大小排序） */
                prvInsertBlockIntoFreeList(((BlockLink_t *)pxLink));
                xFreeBytesRemaining += pxLink->xBlockSize;
            }
            xTaskResumeAll();
        }
    }
}
```

## heap_2的优缺点分析

### 优点

| 优点 | 说明 |
|------|------|
| **支持释放** | 比heap_1更灵活 |
| **最佳适配** | 有效利用小空闲块 |
| **按大小排序** | 找块效率较高 |
| **实现中等复杂度** | 比heap_4简单 |
| **线程安全** | 有临界区保护 |

### 缺点

| 缺点 | 说明 |
|------|------|
| **产生碎片** | 不合并相邻块 |
| **没有块合并** | 碎片无法自愈 |
| **长期运行问题** | 碎片累积可能导致失败 |
| **无最小历史统计** | 无法看到最坏情况 |
| **碎片利用率低** | 小碎片难以利用 |

## heap_2 与 heap_1 对比

| 特性 | heap_1 | heap_2 |
|------|--------|--------|
| 内存释放 | ✗ 不支持 | ✓ 支持 |
| 内存碎片 | ✗ 无 | ✗ 有 |
| 块合并 | - | ✗ 不合并 |
| 分配算法 | 线性分配 | 最佳适配 |
| 空闲块管理 | 单个指针 | 排序链表 |
| 最小历史统计 | ✗ | ✗ |
| 适用场景 | 静态分配 | 简单动态 |
| 复杂度 | 简单 | 中等 |

## heap_2的适用场景

### 适合使用heap_2

✓ **需要释放内存** - 比heap_1灵活
✓ **分配模式简单** - 大小相对固定
✓ **短期运行系统** - 碎片来不及累积
✓ **可以接受碎片** - 碎片问题不严重
✓ **需要最佳适配** - 高效利用小块

### 不适合使用heap_2

✗ **频繁分配释放** - 会产生严重碎片
✗ **长期运行系统** - 碎片会累积
✗ **内存受限系统** - 碎片浪费宝贵内存
✗ **需要块合并** - 用heap_4
✗ **需要最小统计** - 用heap_4

## 调试技巧

### 跟踪空闲块

虽然heap_2的内部变量是静态的，但你可以：

```c
/* 在调试器中监视这些变量 */
xFreeBytesRemaining  /* 剩余字节数 */
xStart.pxNextFreeBlock  /* 空闲链表头 */
```

### 检查碎片程度

```c
/* 估算碎片（粗略方法） */
void vCheckFragmentation(void)
{
    size_t xFree = xPortGetFreeHeapSize();

    /* 尝试分配一个大块看是否成功 */
    void *pvTest = pvPortMalloc(xFree / 2);
    if(pvTest == NULL)
    {
        printf("可能有碎片问题！\r\n");
    }
    else
    {
        vPortFree(pvTest);
    }
}
```

## 减少heap_2碎片的技巧

### 1. 固定分配大小

```c
/* 好：只用几个固定大小 */
#define SIZE_SMALL  64
#define SIZE_MEDIUM 256
#define SIZE_LARGE  1024

void *pvAllocSmall(void)  { return pvPortMalloc(SIZE_SMALL); }
void *pvAllocMedium(void)