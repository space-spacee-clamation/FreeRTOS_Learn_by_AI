# heap_2使用实践指南

## 什么时候选择heap_2？

### 适合heap_2的场景

| 场景 | 说明 |
|------|------|
| **需要内存释放** | 需要动态分配和释放内存 |
| **简单动态模式** | 分配模式相对简单可预测 |
| **最佳适配需求** | 需要高效利用小空闲块 |
| **短期运行系统** | 系统运行时间不长，碎片来不及累积 |
| **比heap_1灵活** | 需要释放但不需要块合并 |

### 不适合heap_2的场景

| 场景 | 原因 |
|------|------|
| **频繁分配释放** | 会产生严重碎片 |
| **长期运行系统** | 碎片会累积，最终耗尽内存 |
| **内存受限系统** | 碎片会浪费宝贵内存 |
| **需要块合并** | heap_4更适合 |
| **需要最小统计** | heap_4支持xPortGetMinimumEverFreeHeapSize() |

## 与heap_1的对比选择

```
heap_1 ────────────────────────────────────────── heap_2 ────────────────────────────────────────── heap_4
  ↓                                                  ↓                                                  ↓
最简单，不释放                    支持释放，但有碎片               支持释放，合并块，无碎片
适合：启动时分配所有             适合：简单动态场景              适合：复杂动态场景
```

**选择指南：**

1. 如果**完全不需要释放** → 选heap_1
2. 如果**需要释放但模式简单** → 选heap_2
3. 如果**需要频繁分配释放** → 选heap_4

## heap_2的最佳实践

### 1. 理解碎片问题

```c
/* 这种模式会产生碎片 */
void vBadFragmentationPattern(void)
{
    void *pv1 = pvPortMalloc(100);  /* [块1] */
    void *pv2 = pvPortMalloc(200);  /* [块1][块2] */
    void *pv3 = pvPortMalloc(150);  /* [块1][块2][块3] */

    vPortFree(pv2);  /* 释放中间块 */
    /* 现在：[块1][空闲][块3] */
    /* 有碎片！*/

    /* 尝试分配250字节 */
    void *pv4 = pvPortMalloc(250);
    /* 可能失败，即使总空闲足够！*/
}
```

### 2. 推荐的分配释放顺序

```c
/* ✓ 好：按分配的逆序释放（类似栈）*/
void vGoodFreeOrder(void)
{
    void *pv1 = pvPortMalloc(100);
    void *pv2 = pvPortMalloc(200);
    void *pv3 = pvPortMalloc(150);

    /* 逆序释放：后分配的先释放 */
    vPortFree(pv3);  /* 先释放最后分配的 */
    vPortFree(pv2);  /* 然后中间的 */
    vPortFree(pv1);  /* 最后第一个 */
}
```

### 3. 使用固定大小的内存池

```c
/* 固定大小分配可以减少碎片 */
#define POOL_SIZE_64   64
#define POOL_SIZE_256  256
#define POOL_SIZE_1024 1024

/* 只用这几个大小分配 */
void *pvAlloc64(void)  { return pvPortMalloc(POOL_SIZE_64); }
void *pvAlloc256(void) { return pvPortMalloc(POOL_SIZE_256); }
void *pvAlloc1024(void){ return pvPortMalloc(POOL_SIZE_1024); }
```

### 4. 重用内存而不是释放重分配

```c
/* ✓ 好：重用内存 */
typedef struct {
    uint8_t *puData;
    size_t xSize;
    bool xInUse;
} Buffer_t;

Buffer_t g_xBuffers[4];  /* 缓冲区池 */

void vInitBufferPool(void)
{
    for(int i = 0; i < 4; i++)
    {
        g_xBuffers[i].puData = pvPortMalloc(256);
        g_xBuffers[i].xSize = 256;
        g_xBuffers[i].xInUse = false;
    }
}

uint8_t *pvGetBuffer(void)
{
    for(int i = 0; i < 4; i++)
    {
        if(g_xBuffers[i].xInUse == false)
        {
            g_xBuffers[i].xInUse = true;
            return g_xBuffers[i].puData;
        }
    }
    return NULL;
}

void vReleaseBuffer(uint8_t *puBuffer)
{
    for(int i = 0; i < 4; i++)
    {
        if(g_xBuffers[i].puData == puBuffer)
        {
            g_xBuffers[i].xInUse = false;
            return;
        }
    }
}
```

### 5. 定期监控内存状态

```c
void vMemoryMonitorTask(void *pvParameters)
{
    (void)pvParameters;
    size_t xFreeSize, xPreviousFreeSize;

    xPreviousFreeSize = xPortGetFreeHeapSize();

    for(;;)
    {
        xFreeSize = xPortGetFreeHeapSize();

        /* 检查内存是否在减少（可能泄漏）*/
        if(xFreeSize < xPreviousFreeSize)
        {
            vPrintString("[WARNING] Memory decreasing!\r\n");
            vPrintString("Previous: ");
            vPrintNumber(xPreviousFreeSize);
            vPrintString(", Now: ");
            vPrintNumber(xFreeSize);
            vPrintString("\r\n");
        }

        xPreviousFreeSize = xFreeSize;

        /* 低内存警告 */
        if(xFreeSize < 2048)
        {
            vPrintString("[CRITICAL] Low memory!\r\n");
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
```

## 实际应用示例

### 示例1：简单的消息缓冲

```c
/* 消息结构 */
typedef struct {
    uint32_t ulMessageId;
    uint32_t ulTimestamp;
    char pcData[64];
} Message_t;

/* 发送消息 - 分配 */
void vSendMessage(uint32_t ulId, const char *pcData)
{
    Message_t *pxMsg;

    pxMsg = pvPortMalloc(sizeof(Message_t));
    if(pxMsg != NULL)
    {
        pxMsg->ulMessageId = ulId;
        pxMsg->ulTimestamp = xTaskGetTickCount();
        strncpy(pxMsg->pcData, pcData, 63);
        pxMsg->pcData[63] = '\0';

        /* 发送到队列 */
        xQueueSend(g_xMessageQueue, &pxMsg, 0);
    }
}

/* 处理消息 - 释放 */
void vProcessMessagesTask(void *pvParameters)
{
    Message_t *pxMsg;

    for(;;)
    {
        if(xQueueReceive(g_xMessageQueue, &pxMsg, portMAX_DELAY) == pdPASS)
        {
            /* 处理消息 */
            vPrintString("Received message ID: ");
            vPrintNumber(pxMsg->ulMessageId);
            vPrintString("\r\n");

            /* 处理完释放！*/
            vPortFree(pxMsg);
            pxMsg = NULL;
        }
    }
}
```

### 示例2：临时工作缓冲区

```c
/* 在任务中使用临时缓冲区 */
void vDataProcessingTask(void *pvParameters)
{
    uint8_t *puWorkBuffer;
    size_t xBufferSize = 512;

    for(;;)
    {
        /* 等待需要处理的数据 */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        /* 分配临时缓冲区 */
        puWorkBuffer = pvPortMalloc(xBufferSize);

        if(puWorkBuffer != NULL)
        {
            /* 使用缓冲区 */
            vReadData(puWorkBuffer, xBufferSize);
            vProcessData(puWorkBuffer, xBufferSize);
            vWriteResults(puWorkBuffer, xBufferSize);

            /* 释放！*/
            vPortFree(puWorkBuffer);
            puWorkBuffer = NULL;
        }
        else
        {
            vPrintString("Failed to allocate work buffer!\r\n");
        }
    }
}
```

### 示例3：链表节点管理

```c
/* 链表节点 */
typedef struct Node {
    uint32_t ulData;
    struct Node *pxNext;
} Node_t;

Node_t *g_pvListHead = NULL;

/* 添加节点 */
void vAddNode(uint32_t ulValue)
{
    Node_t *pxNewNode;

    pxNewNode = pvPortMalloc(sizeof(Node_t));
    if(pxNewNode != NULL)
    {
        pxNewNode->ulData = ulValue;
        pxNewNode->pxNext = g_pvListHead;
        g_pvListHead = pxNewNode;
    }
}

/* 删除节点 */
void vRemoveNode(uint32_t ulValue)
{
    Node_t *pxCurrent = g_pvListHead;
    Node_t *pxPrevious = NULL;

    while(pxCurrent != NULL)
    {
        if(pxCurrent->ulData == ulValue)
        {
            /* 从链表移除 */
            if(pxPrevious == NULL)
            {
                g_pvListHead = pxCurrent->pxNext;
            }
            else
            {
                pxPrevious->pxNext = pxCurrent->pxNext;
            }

            /* 释放！*/
            vPortFree(pxCurrent);
            break;
        }

        pxPrevious = pxCurrent;
        pxCurrent = pxCurrent->pxNext;
    }
}

/* 清空整个链表 */
void vClearList(void)
{
    Node_t *pxCurrent = g_pvListHead;
    Node_t *pxNext;

    while(pxCurrent != NULL)
    {
        pxNext = pxCurrent->pxNext;
        vPortFree(pxCurrent);  /* 释放每个节点 */
        pxCurrent = pxNext;
    }

    g_pvListHead = NULL;
}
```

## 减少碎片的策略

### 策略1：预分配主要对象

```c
/* 启动时预分配主要对象 */
#define MAX_CONNECTIONS 10

typedef struct {
    uint32_t ulId;
    char pcName[32];
    bool xActive;
} Connection_t;

Connection_t *g_pxConnections[MAX_CONNECTIONS];

void vPreallocateConnections(void)
{
    for(int i = 0; i < MAX_CONNECTIONS; i++)
    {
        g_pxConnections[i] = pvPortMalloc(sizeof(Connection_t));
        if(g_pxConnections[i] != NULL)
        {
            g_pxConnections[i]->xActive = false;
        }
    }
}

/* 运行时只获取，不分配 */
Connection_t *pxGetConnection(void)
{
    for(int i = 0; i < MAX_CONNECTIONS; i++)
    {
        if((g_pxConnections[i] != NULL) &&
           (g_pxConnections[i]->xActive == false))
        {
            g_pxConnections[i]->xActive = true;
            return g_pxConnections[i];
        }
    }
    return NULL;
}
```

### 策略2：按大小分类分配

```c
/* 小、中、大三种固定大小 */
#define SIZE_SMALL  64
#define SIZE_MEDIUM 256
#define SIZE_LARGE  1024

/* 根据需要选择合适的大小 */
void *pvAllocateSuitable(size_t xWantedSize)
{
    if(xWantedSize <= SIZE_SMALL)
    {
        return pvPortMalloc(SIZE_SMALL);
    }
    else if(xWantedSize <= SIZE_MEDIUM)
    {
        return pvPortMalloc(SIZE_MEDIUM);
    }
    else if(xWantedSize <= SIZE_LARGE)
    {
        return pvPortMalloc(SIZE_LARGE);
    }
    else
    {
        return NULL;
    }
}
```

### 策略3：避免频繁的小分配

```c
/* ✗ 坏：频繁小分配 */
void vBadFrequentAllocation(void)
{
    for(int i = 0; i < 100; i++)
    {
        char *pcBuf = pvPortMalloc(16);  /* 频繁小分配 */
        /* 使用... */
        vPortFree(pcBuf);  /* 频繁释放 */
    }
}

/* ✓ 好：一次分配大的 */
void vGoodBatchAllocation(void)
{
    char *pcBigBuf = pvPortMalloc(1600);  /* 一次分配大的 */

    if(pcBigBuf != NULL)
    {
        for(int i = 0; i < 100; i++)
        {
            char *pcBuf = pcBigBuf + (i * 16);  /* 分块使用 */
            /* 使用pcBuf... */
        }

        vPortFree(pcBigBuf);  /* 一次释放 */
    }
}
```

## heap_2的调试技巧

### 1. 跟踪分配释放配对

```c
/* 调试用：跟踪分配和释放 */
#ifdef DEBUG_HEAP
static uint32_t g_ulAllocCount = 0;
static uint32_t g_ulFreeCount = 0;

#define DEBUG_ALLOC()  g_ulAllocCount++
#define DEBUG_FREE()   g_ulFreeCount++
#else
#define DEBUG_ALLOC()
#define DEBUG_FREE()
#endif

void *pvDebugMalloc(size_t xSize)
{
    void *pv = pvPortMalloc(xSize);
    if(pv != NULL)
    {
        DEBUG_ALLOC();
    }
    return pv;
}

void vDebugFree(void *pv)
{
    if(pv != NULL)
    {
        DEBUG_FREE();
        vPortFree(pv);
    }
}

void vPrintHeapStats(void)
{
    #ifdef DEBUG_HEAP
    vPrintString("Alloc count: ");
    vPrintNumber(g_ulAllocCount);
    vPrintString(", Free count: ");
    vPrintNumber(g_ulFreeCount);
    vPrintString("\r\n");
    #endif
}
```

### 2. 检查内存模式

```c
/* 填充模式，检查是否越界 */
#define FILL_PATTERN 0xAA

void *pvCheckedMalloc(size_t xSize)
{
    size_t xActualSize = xSize + 4;  /* 额外4字节用于模式 */
    uint8_t *pu = pvPortMalloc(xActualSize);

    if(pu != NULL)
    {
        /* 在末尾填充模式 */
        pu[xSize] = FILL_PATTERN;
        pu[xSize + 1] = FILL_PATTERN;
        pu[xSize + 2] = FILL_PATTERN;
        pu[xSize + 3] = FILL_PATTERN;
    }

    return pu;
}

void vCheckAndFree(void *pv, size_t xSize)
{
    uint8_t *pu = (uint8_t *)pv;

    if(pu != NULL)
    {
        /* 检查模式是否被覆盖 */
        if((pu[xSize] != FILL_PATTERN) ||
           (pu[xSize + 1] != FILL_PATTERN) ||
           (pu[xSize + 2] != FILL_PATTERN) ||
           (pu[xSize + 3] != FILL_PATTERN))
        {
            vPrintString("Memory overwrite detected!\r\n");
        }

        vPortFree(pu);
    }
}
```

## 从heap_2迁移到heap_4

如果发现heap_2的碎片问题太严重，迁移到heap_4很简单：

### 步骤1：修改Makefile

```makefile
# 修改前（heap_2）
$(FREERTOS_SRC)/portable/MemMang/heap_2-lm3s6965.o

# 修改后（heap_4）
$(FREERTOS_SRC)/portable/MemMang/heap_4-lm3s6965.o
```

### 步骤2：代码几乎不需要修改

```c
/* heap_2的代码 */
void *pv = pvPortMalloc(100);
/* ... 使用 ... */
vPortFree(pv);

/* heap_4的代码 - 完全一样！*/
void *pv = pvPortMalloc(100);
/* ... 使用 ... */
vPortFree(p