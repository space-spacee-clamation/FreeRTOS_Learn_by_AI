# heap_3 使用实践指南

## 🚀 快速开始

### 基本使用步骤

1. **确保链接器配置正确**
   - 检查链接脚本中的堆配置
   - 确认堆大小满足需求

2. **配置FreeRTOS**
   ```c
   #define configSUPPORT_DYNAMIC_ALLOCATION    1
   #define configUSE_MALLOC_FAILED_HOOK         1  // 可选
   ```

3. **在Makefile中使用heap_3**
   ```makefile
   # 将heap_2.c改为heap_3.c
   $(FREERTOS_SRC)/portable/MemMang/heap_3.o
   ```

4. **使用内存分配API**
   ```c
   // 分配内存
   void *ptr = pvPortMalloc(size);
   if(ptr != NULL) {
       // 使用内存
   }

   // 释放内存
   vPortFree(ptr);
   ```

## 💻 实用示例

### 示例1：基础内存分配

```c
#include "FreeRTOS.h"

void vBasicExample(void)
{
    // 分配100字节
    void *pvBuffer = pvPortMalloc(100);

    if(pvBuffer != NULL)
    {
        // 使用缓冲区
        memset(pvBuffer, 0, 100);

        // 使用完毕后释放
        vPortFree(pvBuffer);
        pvBuffer = NULL;  // 避免悬空指针
    }
    else
    {
        // 处理分配失败
        // 可以记录错误、重试或采取其他措施
    }
}
```

### 示例2：多任务环境下使用

```c
#include "FreeRTOS.h"
#include "task.h"

// 任务1：分配内存
void vTask1(void *pvParameters)
{
    for(;;)
    {
        void *pvData = pvPortMalloc(50);
        if(pvData != NULL)
        {
            // 使用数据
            vTaskDelay(pdMS_TO_TICKS(100));
            vPortFree(pvData);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// 任务2：同时也分配内存
void vTask2(void *pvParameters)
{
    for(;;)
    {
        void *pvData = pvPortMalloc(80);
        if(pvData != NULL)
        {
            // 使用数据
            vTaskDelay(pdMS_TO_TICKS(150));
            vPortFree(pvData);
        }
        vTaskDelay(pdMS_TO_TICKS(800));
    }
}

// 创建任务
void vStartTasks(void)
{
    xTaskCreate(vTask1, "Task1", 256, NULL, 2, NULL);
    xTaskCreate(vTask2, "Task2", 256, NULL, 2, NULL);
}
```

### 示例3：结构体分配

```c
#include "FreeRTOS.h"
#include <string.h>

// 定义一个数据结构
typedef struct
{
    uint32_t ulID;
    char     pcName[20];
    float    fValue;
} DataItem_t;

// 创建数据项
DataItem_t *pxCreateDataItem(uint32_t ulID, const char *pcName, float fValue)
{
    // 分配结构体
    DataItem_t *pxItem = (DataItem_t *)pvPortMalloc(sizeof(DataItem_t));

    if(pxItem != NULL)
    {
        // 初始化结构体
        pxItem->ulID = ulID;
        strncpy(pxItem->pcName, pcName, sizeof(pxItem->pcName) - 1);
        pxItem->pcName[sizeof(pxItem->pcName) - 1] = '\0';
        pxItem->fValue = fValue;
    }

    return pxItem;
}

// 删除数据项
void vDeleteDataItem(DataItem_t *pxItem)
{
    if(pxItem != NULL)
    {
        vPortFree(pxItem);
    }
}

// 使用示例
void vStructExample(void)
{
    DataItem_t *pxItem = pxCreateDataItem(1, "Sensor", 25.5f);

    if(pxItem != NULL)
    {
        // 使用数据项
        // ...

        // 删除数据项
        vDeleteDataItem(pxItem);
    }
}
```

### 示例4：数组分配

```c
#include "FreeRTOS.h"

// 创建整数数组
int32_t *plAllocateIntArray(size_t xSize)
{
    return (int32_t *)pvPortMalloc(xSize * sizeof(int32_t));
}

// 使用数组
void vArrayExample(void)
{
    const size_t xArraySize = 10;
    int32_t *plArray = plAllocateIntArray(xArraySize);

    if(plArray != NULL)
    {
        // 初始化数组
        for(size_t i = 0; i < xArraySize; i++)
        {
            plArray[i] = (int32_t)i;
        }

        // 使用数组
        // ...

        // 释放数组
        vPortFree(plArray);
    }
}
```

## 🔧 最佳实践

### 1. 错误处理

**始终检查返回值：**
```c
void *pvBuffer = pvPortMalloc(100);
if(pvBuffer == NULL)
{
    // 处理错误
    vLoggingPrintf("内存分配失败！\n");
    return;
}
// 使用缓冲区
```

**使用malloc失败钩子：**
```c
void vApplicationMallocFailedHook(void)
{
    // 记录错误
    vLoggingPrintf("!!! 内存分配失败钩子被调用！\n");

    // 可以采取紧急措施
    // 例如：重置系统、进入安全模式等
    for(;;)
    {
        // 闪烁LED警告
        vToggleLED();
        vDelayMs(100);
    }
}
```

### 2. 内存管理

**避免内存泄漏：**
```c
// 错误示例 - 内存泄漏
void vLeakExample(void)
{
    void *pvBuffer = pvPortMalloc(100);
    // 忘记释放！
}

// 正确示例
void vNoLeakExample(void)
{
    void *pvBuffer = pvPortMalloc(100);
    if(pvBuffer != NULL)
    {
        // 使用缓冲区
        // ...

        // 释放缓冲区
        vPortFree(pvBuffer);
        pvBuffer = NULL;  // 清空指针
    }
}
```

**使用RAII模式（如果支持C++）：**
```cpp
class AutoFree
{
public:
    AutoFree(void *ptr) : m_ptr(ptr) {}
    ~AutoFree() { if(m_ptr) vPortFree(m_ptr); }
    void *get() { return m_ptr; }
private:
    void *m_ptr;
};

void vRAIIExample()
{
    AutoFree buffer(pvPortMalloc(100));
    if(buffer.get() != NULL)
    {
        // 使用缓冲区
        // ...
        // 自动释放，不需要手动调用vPortFree
    }
}
```

### 3. 指针安全

**避免悬空指针：**
```c
void *pvBuffer = pvPortMalloc(100);
// ... 使用 ...
vPortFree(pvBuffer);
pvBuffer = NULL;  // 立即清空指针

// 之后使用前检查
if(pvBuffer != NULL)
{
    // 安全使用
}
```

**双重释放防护：**
```c
void vSafeFree(void **ppvPtr)
{
    if(ppvPtr != NULL && *ppvPtr != NULL)
    {
        vPortFree(*ppvPtr);
        *ppvPtr = NULL;
    }
}

// 使用
void *pvBuffer = pvPortMalloc(100);
// ... 使用 ...
vSafeFree(&pvBuffer);  // 安全释放
vSafeFree(&pvBuffer);  // 再次调用也安全
```

### 4. 分配策略

**预分配常用大小：**
```c
// 在系统启动时预分配
#define COMMON_BUFFER_SIZE  64
static void *g_pvCommonBuffers[10];
static volatile size_t g_uxBufferIndex = 0;

void vPreallocateBuffers(void)
{
    for(size_t i = 0; i < 10; i++)
    {
        g_pvCommonBuffers[i] = pvPortMalloc(COMMON_BUFFER_SIZE);
    }
}

// 获取预分配的缓冲区
void *pvGetCommonBuffer(void)
{
    // 简单的循环分配
    void *pvBuffer = g_pvCommonBuffers[g_uxBufferIndex];
    g_uxBufferIndex = (g_uxBufferIndex + 1) % 10;
    return pvBuffer;
}
```

## 🐛 常见问题排查

### 问题1：分配总是失败

**可能原因：**
- 链接器堆配置太小
- 链接脚本堆区域配置错误
- 标准库没有正确初始化

**排查步骤：**
1. 检查链接脚本中的堆大小
2. 生成链接器映射文件确认堆位置
3. 尝试使用更大的堆大小
4. 检查标准库初始化代码

### 问题2：程序崩溃

**可能原因：**
- 访问已释放的内存
- 缓冲区溢出
- 未初始化的指针

**排查步骤：**
1. 使用调试器检查崩溃位置
2. 添加更多的NULL检查
3. 检查数组越界访问
4. 使用内存保护功能（如果有）

### 问题3：内存泄漏

**可能原因：**
- 忘记释放内存
- 异常路径没有释放
- 指针被覆盖

**排查步骤：**
1. 代码审查所有malloc/free配对
2. 添加日志跟踪分配和释放
3. 使用静态分析工具
4. 检查异常处理路径

## 📊 性能考虑

### 执行时间

heap_3的执行时间取决于标准库的实现，通常：
- 简单分配：几微秒到几十微秒
- 复杂情况：可能更长
- 最坏情况：不确定

### 内存开销

标准库malloc通常有一些内存开销：
- 每个分配块的管理信息（几个字节到十几个字节）
- 可能的内存对齐填充
- 取决于具体实现

### 优化建议

1. **减少分配次数**
   - 批量分配而不是多次小分配
   - 使用对象池

2. **避免频繁分配释放**
   - 重用已分配的内存
   - 考虑静态分配

3. **合理的分配顺序**
   - 先分配大对象，后分配小对象
   - 减少碎片（取决于标准库实现）

## 📚 参考资料

- [heap_3分配器详解](10_heap_3分配器详解.md)
- [pvPortMalloc_API详解](03_pvPortMalloc_API详解.md)
- [vPortFree_API详解](08_vPortFree_API详解.md)
- [常见问题FAQ](06_常见问题FAQ.md)
