# heap_4分配器详解

## heap_4是什么？

heap_4是FreeRTOS提供的功能最强大、最通用的内存分配器。它在heap_2的基础上增加了相邻空闲块自动合并机制，大幅提高了抗碎片化能力。

### 核心特点

1. ✓ **支持内存释放** - vPortFree() 真正工作
2. ✓ **首次适配算法** - 找第一个够用的块（快速）
3. ✓ **空闲块按地址排序** - 方便合并相邻块
4. ✓ **自动合并相邻空闲块** - 大幅减少外部碎片！
5. ✓ **完整的内存统计** - 支持vPortGetHeapStats()获取详细信息
6. ✓ **最小历史统计** - 支持xPortGetMinimumEverFreeHeapSize()
7. ✓ **pvPortCalloc支持** - 分配并清零内存

## heap_4的工作原理

### 基本思路

heap_4使用链表来管理空闲块，空闲块按**地址排序**（而不是大小排序）：

```
初始化：
堆：[                  一个大块                  ]
      ↑
   空闲链表：xStart -> [大块] -> xEnd

分配100字节（首次适配）：
堆：[100字节已用][         剩余大块         ]
                   ↑
              空闲链表：xStart -> [剩余大块] -> xEnd

再分配200字节：
堆：[100字节][200字节][      剩余      ]
                              ↑
                         空闲链表

释放中间的200字节：
堆：[100字节][  空闲200  ][      剩余      ]
                                    
空闲链表：xStart -> [200字节] -> [剩余] -> xEnd
          （按地址排序，方便检查相邻块！）

关键：现在释放100字节块！
堆：[  空闲100  ][  空闲200  ][      剩余      ]
                                    
heap_4检测到这两个空闲块相邻，自动合并！
结果：
堆：[          空闲300          ][      剩余      ]
                                    
空闲链表：xStart -> [300字节] -> [剩余] -> xEnd
          ✓ 碎片被消除了！
```

### 数据结构

heap_4的核心数据结构与heap_2类似，但增加了更多统计变量：

```c
/* 堆内存数组 */
#if ( configAPPLICATION_ALLOCATED_HEAP == 1 )
    extern uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
#else
    static uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
#endif

/* 块链接结构体 */
typedef struct A_BLOCK_LINK
{
    struct A_BLOCK_LINK *pxNextFreeBlock;  /* 下一个空闲块 */
    size_t xBlockSize;                      /* 块大小 */
} BlockLink_t;

/* 链表头和尾 */
static BlockLink_t xStart;
static BlockLink_t *pxEnd = NULL;

/* 剩余字节数 */
static size_t xFreeBytesRemaining = 0U;

/* 历史最小剩余字节数（新增！） */
static size_t xMinimumEverFreeBytesRemaining = 0U;

/* 统计信息（新增！） */
static size_t xNumberOfSuccessfulAllocations = 0U;
static size_t xNumberOfSuccessfulFrees = 0U;
```

### 关键常量

```c
/* 结构体大小（对齐后） */
static const size_t xHeapStructSize = ( sizeof( BlockLink_t ) + 
    ( ( size_t ) ( portBYTE_ALIGNMENT - 1 ) ) ) & 
    ~( ( size_t ) portBYTE_ALIGNMENT_MASK );

/* 最小块大小 = 结构体大小的2倍 */
#define heapMINIMUM_BLOCK_SIZE    ( ( size_t ) ( xHeapStructSize << 1 ) )

/* 块分配状态位（用最高位标记） */
#define heapBLOCK_ALLOCATED_BITMASK    ( ( ( size_t ) 1 ) << 
    ( ( sizeof( size_t ) * heapBITS_PER_BYTE ) - 1 ) )
#define heapBLOCK_IS_ALLOCATED( pxBlock )        ( ( ( pxBlock->xBlockSize ) & heapBLOCK_ALLOCATED_BITMASK ) != 0 )
#define heapALLOCATE_BLOCK( pxBlock )            ( ( pxBlock->xBlockSize ) |= heapBLOCK_ALLOCATED_BITMASK )
#define heapFREE_BLOCK( pxBlock )                ( ( pxBlock->xBlockSize ) &= ~heapBLOCK_ALLOCATED_BITMASK )
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
4. 遍历空闲链表（按地址排序）找合适的块
   - 从头开始找
   - 找第一个 >= xWantedSize 的块（首次适配）
   ↓
5. 找到合适的块：
   a) 从空闲链表中移除这个块
   b) 如果块比需要的大很多（> heapMINIMUM_BLOCK_SIZE）
      - 分裂成两块
      - 一块返回给用户
      - 另一块插回空闲链表
   c) 标记块为"已分配"（设置最高位）
   d) 更新 xFreeBytesRemaining
   e) 更新 xMinimumEverFreeBytesRemaining（如果需要）
   f) 增加 xNumberOfSuccessfulAllocations
   g) 返回指针（跳过块结构体）
   ↓
6. 没找到：返回NULL
```

### 图解分配过程

```
初始状态：
空闲链表：xStart -> [400字节] -> xEnd

调用 pvPortMalloc(100):

1. 计算实际需要 = 100 + 结构体大小 + 对齐 = 120
2. 遍历空闲链表，找到400字节块（首次适配）
3. 检查：400 - 120 = 280 > 最小块大小 ✓
4. 分裂：
   - 块1: 120字节（标记为已分配）
   - 块2: 280字节（保持空闲）
5. 把块2插回空闲链表
6. 更新统计信息
7. 返回块1的用户数据指针

结果：
堆：[块头][100字节用户数据][块头][280字节空闲]
空闲链表：xStart -> [280字节] -> xEnd
```

### 首次适配算法的优势

```
假设空闲链表：[100][50][200][300]（按地址排序）

请求80字节：
- 先看100字节 → 够了！
- 立即分配这个块（首次适配）
- 不需要继续遍历

对比最佳适配：
- 需要遍历整个链表找最小的够用的块
- 首次适配更快，特别是链表很长时

结果：
- 分配速度更快
- 大的块可能保留下来
- 在实际应用中效果往往很好！
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
5. 将块插入空闲链表（按地址排序）
   ↓
6. **关键：尝试合并相邻块！**
   a) 检查与前一个块是否相邻（通过地址判断）
   b) 如果相邻，合并
   c) 检查与后一个块是否相邻
   d) 如果相邻，合并
   ↓
7. 更新 xFreeBytesRemaining
8. 增加 xNumberOfSuccessfulFrees
```

### 关键：自动合并相邻块！

```
释放前的堆布局：
[已分配100][已分配200][已分配150]

释放中间的200字节：
[已分配100][  空闲200  ][已分配150]

现在释放100字节块：
[  空闲100  ][  空闲200  ][已分配150]
                ↑
       这两个块地址相邻！

heap_4检测到相邻，自动合并！
结果：
[          空闲300          ][已分配150]
       ↑
   一个大块，碎片被消除了！

✓ 即使这三个块在物理上是连续的，
✓ heap_4也会自动把它们合并成一个大块！
```

### 图解释放过程（带合并）

```
释放前：
堆：[块A:100已用][块B:200已用][块C:150已用]
空闲链表：xStart -> [其他空闲块...] -> xEnd

第一步：释放块B
调用 vPortFree(块B的用户指针):

1. 回退指针到块B的块头
2. 清除已分配标记
3. 按地址插入空闲链表
4. 检查相邻块：
   - 前一个是块A（已分配）→ 不合并
   - 后一个是块C（已分配）→ 不合并
   
中间状态：
堆：[块A:100已用][块B:200空闲][块C:150已用]
空闲链表：xStart -> [200字节] -> [其他...] -> xEnd

第二步：释放块A
调用 vPortFree(块A的用户指针):

1. 回退指针到块A的块头
2. 清除已分配标记
3. 按地址插入空闲链表
4. 检查相邻块：
   - 前一个：无 → 不合并
   - 后一个：块B（空闲）→ ✓ 地址相邻！
5. 合并块A和块B！
   - 新块大小 = 100 + 200 = 300字节
   - 从链表移除块B
   - 更新块A的大小

最终状态：
堆：[块A+B:300空闲][块C:150已用]
空闲链表：xStart -> [300字节] -> [其他...] -> xEnd
          ✓ 碎片被消除了！
```

## 块合并的详细机制

### 如何判断相邻？

```c
/* 简化版合并逻辑 */
static void prvInsertBlockIntoFreeList( BlockLink_t * pxBlockToInsert )
{
    BlockLink_t * pxIterator;
    uint8_t * puc;
    
    /* 找到合适的位置（按地址排序） */
    for( pxIterator = &xStart; 
         heapPROTECT_BLOCK_POINTER( pxIterator->pxNextFreeBlock ) < pxBlockToInsert; 
         pxIterator = heapPROTECT_BLOCK_POINTER( pxIterator->pxNextFreeBlock ) )
    {
        /* 继续找 */
    }
    
    /* 检查与前一个块是否相邻 */
    puc = ( uint8_t * ) pxIterator;
    if( ( puc + pxIterator->xBlockSize ) == ( uint8_t * ) pxBlockToInsert )
    {
        /* 相邻！合并 */
        pxIterator->xBlockSize += pxBlockToInsert->xBlockSize;
        pxBlockToInsert = pxIterator;
    }
    
    /* 检查与后一个块是否相邻 */
    puc = ( uint8_t * ) pxBlockToInsert;
    if( ( puc + pxBlockToInsert->xBlockSize ) == 
        ( uint8_t * ) heapPROTECT_BLOCK_POINTER( pxIterator->pxNextFreeBlock ) )
    {
        if( heapPROTECT_BLOCK_POINTER( pxIterator->pxNextFreeBlock ) != pxEnd )
        {
            /* 相邻！合并 */
            pxBlockToInsert->xBlockSize += 
                heapPROTECT_BLOCK_POINTER( pxIterator->pxNextFreeBlock )->xBlockSize;
            pxBlockToInsert->pxNextFreeBlock = 
                heapPROTECT_BLOCK_POINTER( pxIterator->pxNextFreeBlock )->pxNextFreeBlock;
        }
    }
    
    /* 插入链表 */
    if( pxIterator != pxBlockToInsert )
    {
        pxIterator->pxNextFreeBlock = heapPROTECT_BLOCK_POINTER( pxBlockToInsert );
    }
}
```

### 合并的三种情况

```
情况1：只与后一个块相邻
[空闲A][空闲B][已分配]
    ↓ 合并
[     空闲A+B     ][已分配]

情况2：只与前一个块相邻
[已分配][空闲A][空闲B]
              ↓ 合并
[已分配][     空闲A+B     ]

情况3：与前后都相邻（最佳情况！）
[空闲A][空闲B][空闲C]
       ↓ 合并
[         空闲A+B+C         ]
```

## 内存统计功能详解

### HeapStats_t 结构体

```c
typedef struct xHeapStats
{
    size_t xAvailableHeapSpaceInBytes;      /* 当前剩余空间 */
    size_t xSizeOfLargestFreeBlockInBytes;  /* 最大空闲块 */
    size_t xSizeOfSmallestFreeBlockInBytes; /* 最小空闲块 */
    size_t xNumberOfFreeBlocks;              /* 空闲块数量 */
    size_t xMinimumEverFreeBytesRemaining;   /* 历史最小剩余 */
    size_t xNumberOfSuccessfulAllocations;    /* 成功分配次数 */
    size_t xNumberOfSuccessfulFrees;         /* 成功释放次数 */
} HeapStats_t;
```

### vPortGetHeapStats() 的使用

```c
void vPrintHeapStats(void)
{
    HeapStats_t xStats;
    
    vPortGetHeapStats(&xStats);
    
    printf("当前剩余: %u 字节\r\n", xStats.xAvailableHeapSpaceInBytes);
    printf("最大空闲块: %u 字节\r\n", xStats.xSizeOfLargestFreeBlockInBytes);
    printf("最小空闲块: %u 字节\r\n", xStats.xSizeOfSmallestFreeBlockInBytes);
    printf("空闲块数量: %u\r\n", xStats.xNumberOfFreeBlocks);
    printf("历史最小: %u 字节\r\n", xStats.xMinimumEverFreeBytesRemaining);
    printf("分配成功: %u 次\r\n", xStats.xNumberOfSuccessfulAllocations);
    printf("释放成功: %u 次\r\n", xStats.xNumberOfSuccessfulFrees);
}
```

### 统计信息的实时更新

```
分配时更新：
- xAvailableHeapSpaceInBytes -= 分配大小
- 如果 xAvailableHeapSpaceInBytes < xMinimumEverFreeBytesRemaining
  - 更新 xMinimumEverFreeBytesRemaining
- xNumberOfSuccessfulAllocations++

释放时更新：
- xAvailableHeapSpaceInBytes += 释放大小
- xNumberOfSuccessfulFrees++

获取统计时遍历计算：
- xSizeOfLargestFreeBlockInBytes
- xSizeOfSmallestFreeBlockInBytes
- xNumberOfFreeBlocks
```

## heap_4的源代码分析

### 核心数据结构（完整）

```c
typedef struct A_BLOCK_LINK
{
    struct A_BLOCK_LINK * pxNextFreeBlock;
    size_t xBlockSize;
} BlockLink_t;

/* 最高位用作分配标记 */
#define heapBLOCK_ALLOCATED_BITMASK    ( ( ( size_t ) 1 ) << 
    ( ( sizeof( size_t ) * heapBITS_PER_BYTE ) - 1 ) )
#define heapBLOCK_SIZE_IS_VALID( xBlockSize )    ( ( ( xBlockSize ) & heapBLOCK_ALLOCATED_BITMASK ) == 0 )
#define heapBLOCK_IS_ALLOCATED( pxBlock )        ( ( ( pxBlock->xBlockSize ) & heapBLOCK_ALLOCATED_BITMASK ) != 0 )
#define heapALLOCATE_BLOCK( pxBlock )            ( ( pxBlock->xBlockSize ) |= heapBLOCK_ALLOCATED_BITMASK )
#define heapFREE_BLOCK( pxBlock )                ( ( pxBlock->xBlockSize ) &= ~heapBLOCK_ALLOCATED_BITMASK )
```

### 插入空闲块（按地址排序 + 自动合并）

```c
static void prvInsertBlockIntoFreeList( BlockLink_t * pxBlockToInsert )
{
    BlockLink_t * pxIterator;
    uint8_t * puc;

    /* 找到合适的位置（按地址排序） */
    for( pxIterator = &xStart; 
         heapPROTECT_BLOCK_POINTER( pxIterator->pxNextFreeBlock ) < pxBlockToInsert; 
         pxIterator = heapPROTECT_BLOCK_POINTER( pxIterator->pxNextFreeBlock ) )
    {
        /* 继续找 */
    }

    /* 检查与前一个块是否相邻 */
    puc = ( uint8_t * ) pxIterator;
    if( ( puc + pxIterator->xBlockSize ) == ( uint8_t * ) pxBlockToInsert )
    {
        /* 相邻！合并 */
        pxIterator->xBlockSize += pxBlockToInsert->xBlockSize;
        pxBlockToInsert = pxIterator;
    }

    /* 检查与后一个块是否相邻 */
    puc = ( uint8_t * ) pxBlockToInsert;
    if( ( puc + pxBlockToInsert->xBlockSize ) ==
        ( uint8_t * ) heapPROTECT_BLOCK_POINTER( pxIterator->pxNextFreeBlock ) )
    {
        if( heapPROTECT_BLOCK_POINTER( pxIterator->pxNextFreeBlock ) != pxEnd )
        {
            /* 相邻！合并 */
            pxBlockToInsert->xBlockSize +=
                heapPROTECT_BLOCK_POINTER( pxIterator->pxNextFreeBlock )->xBlockSize;
            pxBlockToInsert->pxNextFreeBlock =
                heapPROTECT_BLOCK_POINTER( pxIterator->pxNextFreeBlock )->pxNextFreeBlock;
        }
    }

    /* 插入链表 */
    if( pxIterator != pxBlockToInsert )
    {
        pxIterator->pxNextFreeBlock = heapPROTECT_BLOCK_POINTER( pxBlockToInsert );
    }
}
```

### 分配函数（简化版）

```c
void * pvPortMalloc( size_t xWantedSize )
{
    BlockLink_t * pxBlock, * pxPreviousBlock, * pxNewBlockLink;
    void * pvReturn = NULL;

    if( xWantedSize > 0 )
    {
        /* 加上块头大小 */
        xWantedSize += xHeapStructSize;

        /* 字节对齐 */
        if( ( xWantedSize & portBYTE_ALIGNMENT_MASK ) != 0x00 )
        {
            xWantedSize += portBYTE_ALIGNMENT - ( xWantedSize & portBYTE_ALIGNMENT_MASK );
        }

        vTaskSuspendAll();
        {
            /* 首次调用？初始化 */
            if( pxEnd == NULL )
            {
                prvHeapInit();
            }

            if( heapBLOCK_SIZE_IS_VALID( xWantedSize ) != 0 )
            {
                if( ( xWantedSize > 0 ) && ( xWantedSize <= xFreeBytesRemaining ) )
                {
                    /* 遍历找块（首次适配） */
                    pxPreviousBlock = &xStart;
                    pxBlock = heapPROTECT_BLOCK_POINTER( xStart.pxNextFreeBlock );

                    while( ( pxBlock->xBlockSize < xWantedSize ) &&
                           ( pxBlock->pxNextFreeBlock != heapPROTECT_BLOCK_POINTER( NULL ) ) )
                    {
                        pxPreviousBlock = pxBlock;
                        pxBlock = heapPROTECT_BLOCK_POINTER( pxBlock->pxNextFreeBlock );
                    }

                    if( pxBlock != pxEnd )
                    {
                        /* 找到块 */
                        pvReturn = ( void * ) ( ( ( uint8_t * )
                            heapPROTECT_BLOCK_POINTER( pxPreviousBlock->pxNextFreeBlock ) ) +
                            xHeapStructSize );

                        /* 从链表移除 */
                        pxPreviousBlock->pxNextFreeBlock = pxBlock->pxNextFreeBlock;

                        /* 块太大？分裂 */
                        if( ( pxBlock->xBlockSize - xWantedSize ) > heapMINIMUM_BLOCK_SIZE )
                        {
                            pxNewBlockLink = ( void * ) ( ( ( uint8_t * ) pxBlock ) + xWantedSize );
                            pxNewBlockLink->xBlockSize = pxBlock->xBlockSize - xWantedSize;
                            pxBlock->xBlockSize = xWantedSize;

                            /* 插入剩余部分 */
                            pxNewBlockLink->pxNextFreeBlock = pxPreviousBlock->pxNextFreeBlock;
                            pxPreviousBlock->pxNextFreeBlock = heapPROTECT_BLOCK_POINTER( pxNewBlockLink );
                        }

                        xFreeBytesRemaining -= pxBlock->xBlockSize;

                        /* 更新最小历史 */
                        if( xFreeBytesRemaining < xMinimumEverFreeBytesRemaining )
                        {
                            xMinimumEverFreeBytesRemaining = xFreeBytesRemaining;
                        }

                        /* 标记为已分配 */
                        heapALLOCATE_BLOCK( pxBlock );
                        pxBlock->pxNextFreeBlock = heapPROTECT_BLOCK_POINTER( NULL );
                        xNumberOfSuccessfulAllocations++;
                    }
                }
            }
        }
        ( void ) xTaskResumeAll();
    }

    return pvReturn;
}
```

### 释放函数（简化版）

```c
void vPortFree( void * pv )
{
    uint8_t * puc = ( uint8_t * ) pv;
    BlockLink_t * pxLink;

    if( pv != NULL )
    {
        /* 回退到块头 */
        puc -= xHeapStructSize;
        pxLink = ( void * ) puc;

        /* 确认已分配 */
        if( heapBLOCK_IS_ALLOCATED( pxLink ) != 0 )
        {
            /* 清除分配标记 */
            heapFREE_BLOCK( pxLink );

            vTaskSuspendAll();
            {
                /* 插入空闲链表（会自动合并！） */
                xFreeBytesRemaining += pxLink->xBlockSize;
                prvInsertBlockIntoFreeList( ( ( BlockLink_t * ) pxLink ) );
                xNumberOfSuccessfulFrees++;
            }
            ( void ) xTaskResumeAll();
        }
    }
}
```

## heap_4的优缺点分析

### 优点

| 优点 | 说明 |
|------|------|
| **支持释放** | 完全支持内存释放 |
| **自动合并块** | 大幅减少碎片 |
| **首次适配** | 分配速度快 |
| **完整统计** | 支持详细的堆统计 |
| **历史最小统计** | 可以看到最坏情况 |
| **碎片自愈** | 长期运行稳定 |
| **pvPortCalloc** | 支持清零分配 |
| **线程安全** | 有临界区保护 |

### 缺点

| 缺点 | 说明 |
|------|------|
| **实现较复杂** | 比heap_1/heap_2复杂 |
| **首次适配** | 可能不如最佳适配节省空间 |
| **按地址排序** | 插入需要遍历 |
| **合并开销** | 释放时有合并操作 |

## heap_4 与其他分配器对比

| 特性 | heap_1 | heap_2 | heap_3 | heap_4 |
|------|--------|--------|--------|--------|
| 内存释放 | ✗ 不支持 | ✓ 支持 | ✓ 支持 | ✓ 支持 |
| 内存碎片 | ✗ 无 | ✗ 有 | 取决于libc | ✓ 很少 |
| 块合并 | - | ✗ 不合并 | 取决于libc | ✓ 自动合并 |
| 分配算法 | 线性分配 | 最佳适配 | libc malloc | 首次适配 |
| 空闲块管理 | 单个指针 | 大小排序 | libc内部 | 地址排序 |
| 最小历史统计 | ✗ | ✗ | ✗ | ✓ |
| 详细统计 | ✗ | ✗ | ✗ | ✓ |
| 线程安全 | ✓ | ✓ | 取决于libc | ✓ |
| 适用场景 | 静态分配 | 简单动态 | 需要libc | 通用动态 |
| 复杂度 | 简单 | 中等 | 外部依赖 | 较复杂 |

## heap_4的适用场景

### 适合使用heap_4

✓ **通用动态分配** - 最灵活的分配器
✓ **频繁分配释放** - 块合并避免碎片
✓ **长期运行系统** - 碎片自愈能力
✓ **需要内存统计** - 详细的堆状态信息
✓ **内存受限系统** - 高效利用每一字节
✓ **不确定分配模式** - 自适应能力强
✓ **需要块合并** - 自动碎片整理

### 不适合使用heap_4

✗ **极简单系统** - heap_1/heap_2足够
✗ **追求极致简单** - heap_1更简单
✗ **需要最佳适配** - heap_2可能更适合
✗ **已有libc malloc** - 可以考虑heap_3

## 调试技巧

### 使用vPortGetHeapStats()监控

```c
void vMonitorHeap(void)
{
    HeapStats_t xStats;
    
    /* 定期调用查看堆状态 */
    vPortGetHeapStats(&xStats);
    
    /* 检查历史最小，判断是否接近耗尽 */
    if(xStats.xMinimumEverFreeBytesRemaining < 1024)
    {
        printf("警告：堆内存接近耗尽！\r\n");
    }
    
    /* 检查空闲块数量，判断碎片程度 */
    if(xStats.xNumberOfFreeBlocks > 10 &&
       xStats.xSizeOfLargestFreeBlockInBytes < 512)
    {
        printf("可能有碎片问题\r\n");
    }
}
```

### 检查内存泄漏

```c
/* 比较分配和释放次数 */
void vCheckForLeaks(void)
{
    HeapStats_t xStats;
    vPortGetHeapStats(&xStats);
    
    /* 如果分配次数远大于释放次数，可能有泄漏 */
    if(xStats.xNumberOfSuccessfulAllocations >
       xStats.xNumberOfSuccessfulFrees + 10)
    {
        printf("可能存在内存泄漏！\r\n");
        printf("分配: %u, 释放: %u\r\n",
               xStats.xNumberOfSuccessfulAllocations,
               xStats.xNumberOfSuccessfulFrees);
    }
}
```

## heap_4的额外功能

### pvPortCalloc - 清零分配

```c
void * pvPortCalloc( size_t xNum, size_t xSize )
{
    void * pv = NULL;

    if( heapMULTIPLY_WILL_OVERFLOW( xNum, xSize ) == 0 )
    {
        pv = pvPortMalloc( xNum * xSize );

        if( pv != NULL )
        {
            ( void ) memset( pv, 0, xNum * xSize );
        }
    }

    return pv;
}
```

### xPortResetHeapMinimumEverFreeHeapSize

```c
/* 重置历史最小记录 */
void xPortResetHeapMinimumEverFreeHeapSize( void )
{
    xMinimumEverFreeBytesRemaining = xFreeBytesRemaining;
}
```

## 最佳实践建议

1. **优先使用heap_4** - 除非有特殊理由，heap_4通常是最佳选择
2. **监控历史最小** - 用xMinimumEverFreeBytesRemaining设置安全余量
3. **定期检查统计** - 调用vPortGetHeapStats()了解堆状态
4. **合理设置堆大小** - 基于历史最小值留出安全余量
5. **使用pvPortCalloc** - 需要清零内存时用这个而不是手动memset
6. **避免极端碎片化操作** - 虽然heap_4能合并，但也尽量避免