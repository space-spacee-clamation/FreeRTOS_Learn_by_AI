# vPortFree API详解

## vPortFree() - 释放内存

### 函数原型

```c
void vPortFree( void *pv );
```

### 头文件

```c
#include "FreeRTOS.h"
```

### 参数说明

| 参数 | 类型 | 说明 |
|------|------|------|
| `pv` | `void *` | 要释放的内存指针（必须是pvPortMalloc()返回的指针） |

### 返回值

无返回值（void）

## 重要提示：不是所有分配器都支持！

### heap_1 中

```c
/* heap_1的vPortFree实现 */
void vPortFree(void *pv)
{
    /* 什么都不做！*/
    (void)pv;  /* 只是为了避免编译器警告 */
}
```

**在heap_1中调用vPortFree()是安全的，但没有任何效果！**

### heap_2 中

```c
/* heap_2的vPortFree实现 - 真正工作！*/
void vPortFree(void *pv)
{
    /* 真正释放内存，把块放回空闲链表 */
    /* ... 实际代码 ... */
}
```

**在heap_2中vPortFree()真正工作！**

## 基本用法（heap_2专用）

### 最简单的例子

```c
void vSimpleExample(void)
{
    char *pcBuffer;

    /* 分配内存 */
    pcBuffer = pvPortMalloc(100);

    if(pcBuffer != NULL)
    {
        /* 使用内存 */
        strcpy(pcBuffer, "Hello FreeRTOS!");

        /* ... 使用pcBuffer ... */

        /* 使用完后释放！*/
        vPortFree(pcBuffer);
        pcBuffer = NULL;  /* 好习惯：避免悬空指针 */
    }
}
```

### 分配-使用-释放模式

```c
void vAllocateUseFree(void)
{
    uint8_t *puData;

    /* 1. 分配 */
    puData = pvPortMalloc(256);

    if(puData != NULL)
    {
        /* 2. 使用 */
        for(int i = 0; i < 256; i++)
        {
            puData[i] = i;
        }

        processData(puData, 256);

        /* 3. 释放 */
        vPortFree(puData);
        puData = NULL;
    }
}
```

## 常见使用模式

### 模式1：任务中的临时缓冲区

```c
void vTaskFunction(void *pvParameters)
{
    for(;;)
    {
        uint8_t *puTempBuffer;

        /* 每次循环分配临时缓冲区 */
        puTempBuffer = pvPortMalloc(512);

        if(puTempBuffer != NULL)
        {
            /* 使用临时缓冲区 */
            fillBuffer(puTempBuffer);
            processBuffer(puTempBuffer);

            /* 释放！*/
            vPortFree(puTempBuffer);
            puTempBuffer = NULL;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
```

### 模式2：可变大小的数据处理

```c
void vProcessVariableSizeData(uint32_t ulDataSize)
{
    uint8_t *puData;

    /* 根据需要分配大小 */
    puData = pvPortMalloc(ulDataSize);

    if(puData != NULL)
    {
        /* 处理数据 */
        receiveData(puData, ulDataSize);
        parseData(puData, ulDataSize);
        storeResult(puData, ulDataSize);

        /* 释放 */
        vPortFree(puData);
    }
}
```

### 模式3：链表节点管理

```c
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
            /* 找到节点，从链表移除 */
            if(pxPrevious == NULL)
            {
                g_pvListHead = pxCurrent->pxNext;
            }
            else
            {
                pxPrevious->pxNext = pxCurrent->pxNext;
            }

            /* 释放节点内存！*/
            vPortFree(pxCurrent);
            break;
        }

        pxPrevious = pxCurrent;
        pxCurrent = pxCurrent->pxNext;
    }
}
```

## 常见错误

### 错误1：释放NULL指针

```c
/* ✓ 安全：vPortFree(NULL) 是安全的 */
void vSafeExample(void)
{
    void *pv = NULL;
    vPortFree(pv);  /* 没问题，什么都不做 */
}
```

### 错误2：重复释放

```c
/* ✗ 危险：重复释放同一块内存 */
void vDoubleFreeExample(void)
{
    void *pv = pvPortMalloc(100);

    vPortFree(pv);   /* 第一次释放 - 没问题 */
    vPortFree(pv);   /* 第二次释放 - 危险！可能损坏堆 */

    /* ✓ 好习惯：释放后置NULL */
    pv = NULL;
    vPortFree(pv);   /* 现在安全了 */
}
```

### 错误3：释放栈上的内存

```c
/* ✗ 错误：释放不是pvPortMalloc分配的内存 */
void vBadStackFree(void)
{
    char cBuffer[100];  /* 栈上的数组 */

    vPortFree(cBuffer);  /* 错误！这不是堆内存！*/
}
```

### 错误4：释放后继续使用

```c
/* ✗ 错误：释放后使用（悬空指针）*/
void vUseAfterFree(void)
{
    char *pcBuffer = pvPortMalloc(100);

    if(pcBuffer != NULL)
    {
        strcpy(pcBuffer, "Hello");

        vPortFree(pcBuffer);  /* 释放 */

        /* ✗ 错误：释放后继续使用！*/
        strcpy(pcBuffer, "World");  /* 这里会崩溃或数据损坏！*/
    }
}

/* ✓ 正确：释放后置NULL */
void vGoodExample(void)
{
    char *pcBuffer = pvPortMalloc(100);

    if(pcBuffer != NULL)
    {
        strcpy(pcBuffer, "Hello");

        vPortFree(pcBuffer);
        pcBuffer = NULL;  /* 置NULL */

        /* 现在可以安全检查 */
        if(pcBuffer != NULL)
        {
            /* 不会执行到这里 */
            strcpy(pcBuffer, "World");
        }
    }
}
```

### 错误5：忘记释放（内存泄漏）

```c
/* ✗ 错误：内存泄漏 */
void vMemoryLeak(void)
{
    for(;;)
    {
        void *pv = pvPortMalloc(100);
        /* 使用 pv */
        /* 但是忘记释放！*/
        /* 内存会越来越少！*/
        vTaskDelay(100);
    }
}

/* ✓ 正确：配对分配和释放 */
void vNoLeak(void)
{
    for(;;)
    {
        void *pv = pvPortMalloc(100);
        if(pv != NULL)
        {
            /* 使用 pv */
            vPortFree(pv);  /* 释放！*/
        }
        vTaskDelay(100);
    }
}
```

## 最佳实践

### 1. 配对原则

```c
/* ✓ 好：分配和释放在同一个函数中 */
void vPairedFunction(void)
{
    void *pv = pvPortMalloc(100);
    if(pv != NULL)
    {
        /* 使用 */
        vPortFree(pv);  /* 配对释放 */
    }
}
```

### 2. 释放后置NULL

```c
/* ✓ 好：释放后立即置NULL */
void vFreeAndNull(void **ppv)
{
    if(*ppv != NULL)
    {
        vPortFree(*ppv);
        *ppv = NULL;  /* 置NULL */
    }
}

/* 使用 */
void *pvBuffer = pvPortMalloc(100);
/* ... 使用 ... */
vFreeAndNull(&pvBuffer);  /* 安全释放并置NULL */
```

### 3. 检查指针有效性

```c
/* ✓ 好：在释放前检查（虽然vPortFree(NULL)是安全的）*/
void vSafeFree(void *pv)
{
    if(pv != NULL)
    {
        vPortFree(pv);
    }
}
```

## heap_2中的特殊注意事项

### 碎片问题

```c
/* 这种模式会在heap_2中产生碎片 */
void vFragmentationPattern(void)
{
    void *pv1 = pvPortMalloc(100);
    void *pv2 = pvPortMalloc(200);
    void *pv3 = pvPortMalloc(150);

    vPortFree(pv2);  /* 释放中间的块 */

    /* 现在有碎片了！*/
    /* 尝试分配250字节可能失败！*/
    void *pv4 = pvPortMalloc(250);
}
```

### 推荐的释放顺序

```c
/* ✓ 好：按分配的逆序释放（减少碎片）*/
void vGoodFreeOrder(void)
{
    void *pv1 = pvPortMalloc(100);
    void *pv2 = pvPortMalloc(200);
    void *pv3 = pvPortMalloc(150);

    /* 按逆序释放 */
    vPortFree(pv3);  /* 先释放最后分配的 */
    vPortFree(pv2);  /* 然后中间的 */
    vPortFree(pv1);  /* 最后第一个 */
}
```

## 线程安全

vPortFree() 是线程安全的！

内部实现（heap_2）：

```c
void vPortFree(void *pv)
{
    /* ... 一些检查 ... */

    vTaskSuspendAll();  /* 挂起所有任务 */
    {
        /* 执行释放操作 */
        /* 插入空闲链表 */
    }
    xTaskResumeAll();  /* 恢复调度 */
}
```

这意味着：
- ✓ 可以在多个任务中同时调用
- ✓ 不需要额外的互斥锁
- ✓ 线程安全

## 总结

- **vPortFree()** = 释放内存（但heap_1中不工作！）
- **总是配对使用** = pvPortMalloc()和vPortFree()配对
- **释放后置NULL** = 避免悬空指针
- **heap_2专用** = 只有在heap_2及以上才真正工作
- **注意碎片** = heap_2会产生碎片
- **线程安全** = 可以在多任务中使用

## 思考问题

1. 在heap_1中调用vPortFree()会发生什么？
2. 为什么释放后要把指针置为NULL？
3. 什么是内存泄漏？如何避免？
4. 什么是悬空指针？有什么危险？
5. heap_2中，什么样的分配释放模式会产生较多碎片？

---

*参考资料：FreeRTOS官方文档 v10.x - 内存释放API*
