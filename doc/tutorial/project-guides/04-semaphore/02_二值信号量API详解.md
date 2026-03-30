# 02 - xQueueCreate() API详解

## 函数原型

```c
QueueHandle_t xQueueCreate(
    UBaseType_t uxQueueLength,  /* 队列长度 */
    UBaseType_t uxItemSize       /* 每个数据项的大小 */
);
```

## 参数详细说明

### 参数1：uxQueueLength - 队列长度

**类型**：`UBaseType_t`

**作用**：指定队列最多能存储多少个数据项

**理解要点**：
- 这是队列的"容量"，表示队列中同时最多能存多少个数据
- 数据项是按"个"计算的，不是按字节
- 每个数据项的大小由第二个参数决定

**示例**：
```c
// 创建能存储10个整数的队列
xQueueCreate(10, sizeof(uint32_t));  // 最多存10个整数

// 创建能存储5个结构体的队列
xQueueCreate(5, sizeof(SensorData_t));  // 最多存5个SensorData

// 创建能存储20个字符的队列
xQueueCreate(20, sizeof(char));  // 最多存20个字符
```

**如何确定队列长度？**

1. **根据生产者-消费者速度差**：
   - 如果生产者比消费者快，队列需要更长
   - 如果速度相当，队列可以短一些

2. **考虑突发情况**：
   - 有时生产者会突然产生大量数据
   - 队列需要能容纳这些突发数据

3. **内存限制**：
   - 队列越长，占用内存越多
   - `总内存 = 队列长度 × 每个数据项大小`

**队列长度建议**：

| 场景 | 推荐长度 | 说明 |
|------|---------|------|
| 简单通信 | 2-5 | 基本够用，节省内存 |
| 普通应用 | 5-10 | 平衡性能和内存 |
| 高速数据 | 10-50 | 防止数据丢失 |
| 突发数据 | 50-100+ | 根据突发量确定 |

**常见错误**：
```c
// ❌ 队列太短（容易满）
xQueueCreate(1, sizeof(uint32_t));  // 只能存1个！

// ❌ 队列太长（浪费内存）
xQueueCreate(1000, sizeof(uint32_t));  // 简单应用不需要这么长！
```

---

### 参数2：uxItemSize - 数据项大小

**类型**：`UBaseType_t`

**作用**：指定每个数据项的大小（字节数）

**重要**：使用 `sizeof()` 运算符获取大小！

**示例**：
```c
// 存储整数
xQueueCreate(10, sizeof(uint32_t));      // 每个数据4字节

// 存储字符
xQueueCreate(20, sizeof(char));           // 每个数据1字节

// 存储浮点数
xQueueCreate(5, sizeof(float));           // 每个数据4字节

// 存储结构体
typedef struct {
    uint32_t id;
    float value;
    char name[20];
} SensorData_t;
xQueueCreate(5, sizeof(SensorData_t));    // 每个数据的大小是结构体总大小
```

**重要提醒**：队列存储的是数据的**拷贝**！

```c
// ✅ 正确：直接传递数据
uint32_t ulValue = 123;
xQueueSend(xQueue, &ulValue, 0);  // 队列会拷贝ulValue的值

// ✅ 正确：传递结构体
SensorData_t xData = {.id = 1, .value = 25.5};
xQueueSend(xQueue, &xData, 0);    // 队列会拷贝整个结构体

// ⚠️ 注意：如果传递指针，队列拷贝的是指针本身！
char *pcString = "Hello";
xQueueSend(xQueue, &pcString, 0);  // 队列存储的是指针值，不是字符串内容！
```

---

## 返回值

**类型**：`QueueHandle_t`

**返回值**：
- **成功**：返回有效的队列句柄（非NULL）
- **失败**：返回 `NULL`（通常是内存不足）

**检查返回值的示例**：
```c
QueueHandle_t xIntegerQueue;

// 创建队列
xIntegerQueue = xQueueCreate(10, sizeof(uint32_t));

if(xIntegerQueue == NULL)
{
    // 队列创建失败！
    // 通常是因为堆内存不足
    printf("队列创建失败！内存不足？\n");
    
    // 错误处理
    for(;;);  // 或者其他错误处理逻辑
}
else
{
    // 队列创建成功，可以使用了
    printf("队列创建成功！\n");
}
```

---

## 完整使用示例

### 示例1：创建整数队列

```c
#include "FreeRTOS.h"
#include "queue.h"
#include <stdio.h>

int main(void)
{
    QueueHandle_t xIntQueue;
    
    // 创建能存储10个整数的队列
    xIntQueue = xQueueCreate(10, sizeof(uint32_t));
    
    if(xIntQueue == NULL)
    {
        printf("队列创建失败！\n");
        return -1;
    }
    
    printf("整数队列创建成功！\n");
    printf("队列长度：10个整数\n");
    printf("每个数据大小：%d字节\n", sizeof(uint32_t));
    
    // ... 使用队列 ...
    
    return 0;
}
```

### 示例2：创建结构体队列

```c
#include "FreeRTOS.h"
#include "queue.h"
#include <stdio.h>
#include <string.h>

// 定义传感器数据结构体
typedef struct
{
    uint32_t ulSensorId;      // 传感器ID
    float fTemperature;        // 温度值
    float fHumidity;           // 湿度值
    char cStatus[16];          // 状态字符串
} SensorData_t;

int main(void)
{
    QueueHandle_t xSensorQueue;
    
    printf("结构体大小：%d字节\n", sizeof(SensorData_t));
    printf("  - ulSensorId:  %d字节\n", sizeof(uint32_t));
    printf("  - fTemperature:%d字节\n", sizeof(float));
    printf("  - fHumidity:   %d字节\n", sizeof(float));
    printf("  - cStatus:     %d字节\n", sizeof(char[16]));
    
    // 创建能存储5个传感器数据的队列
    xSensorQueue = xQueueCreate(5, sizeof(SensorData_t));
    
    if(xSensorQueue == NULL)
    {
        printf("传感器队列创建失败！\n");
        return -1;
    }
    
    printf("传感器队列创建成功！\n");
    printf("队列长度：5个数据项\n");
    printf("总内存使用：%d字节\n", 5 * sizeof(SensorData_t));
    
    // ... 使用队列 ...
    
    return 0;
}
```

### 示例3：创建多个队列

```c
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include <stdio.h>

// 队列句柄（全局变量，多个任务需要访问）
QueueHandle_t xIntegerQueue = NULL;
QueueHandle_t xStringQueue = NULL;
QueueHandle_t xStructQueue = NULL;

typedef struct {
    int id;
    char name[20];
} DataItem_t;

int main(void)
{
    printf("开始创建队列...\n");
    
    // 创建整数队列
    xIntegerQueue = xQueueCreate(10, sizeof(uint32_t));
    if(xIntegerQueue == NULL)
    {
        printf("整数队列创建失败！\n");
        return -1;
    }
    printf("✓ 整数队列创建成功\n");
    
    // 创建字符串队列（每个字符串32字节）
    xStringQueue = xQueueCreate(5, 32);
    if(xStringQueue == NULL)
    {
        printf("字符串队列创建失败！\n");
        return -1;
    }
    printf("✓ 字符串队列创建成功\n");
    
    // 创建结构体队列
    xStructQueue = xQueueCreate(3, sizeof(DataItem_t));
    if(xStructQueue == NULL)
    {
        printf("结构体队列创建失败！\n");
        return -1;
    }
    printf("✓ 结构体队列创建成功\n");
    
    printf("\n所有队列创建完成！\n");
    
    // ... 创建任务，使用队列 ...
    
    return 0;
}
```

---

## 工作原理图解

### 队列创建时的内存分配

```
调用 xQueueCreate(5, sizeof(uint32_t))

FreeRTOS堆内存：
┌─────────────────────────────────────┐
│                                     │
│  队列控制块（Queue_t）              │ ← 管理队列的元数据
│  - 队列长度：5                       │
│  - 数据项大小：4字节                 │
│  - 读写指针等                        │
├─────────────────────────────────────┤
│                                     │
│  数据缓冲区                          │ ← 实际存储数据的地方
│  ┌───┬───┬───┬───┬───┐            │
│  │   │   │   │   │   │  5个位置    │
│  └───┴───┴───┴───┴───┘            │
│  每个位置4字节，共20字节             │
│                                     │
└─────────────────────────────────────┘

返回值：指向这个队列的句柄
```

### 队列创建失败的原因

```c
// 可能失败的情况：
xQueueCreate(10000, 100);  // ❌ 需要 10000×100 = 1,000,000 字节！
                            // 如果堆只有几KB，肯定失败

// 解决方法：
// 1. 减小队列长度
xQueueCreate(100, 100);   // 100×100 = 10,000 字节

// 2. 减小数据项大小
xQueueCreate(10000, 10);   // 10000×10 = 100,000 字节

// 3. 增加堆大小（修改FreeRTOSConfig.h）
#define configTOTAL_HEAP_SIZE  (20 * 1024)  // 从10KB增加到20KB
```

---

## 常见错误总结

| 错误 | 原因 | 解决方法 |
|------|------|---------|
| 返回NULL | 内存不足 | 减小队列长度或数据大小，或增加堆空间 |
| 队列很快满 | 队列长度太短 | 增加队列长度，或加快消费者速度 |
| 数据丢失 | 生产者比消费者快太多 | 增加队列长度，或调整任务优先级 |
| 内存浪费 | 队列太长 | 减小队列长度到合理值 |
| 数据错乱 | 数据项大小设置错误 | 确保使用 `sizeof()` 获取正确大小 |

---

## 记忆口诀

```
队列创建两参数，
长度大小要清楚。
长度是指数据数，
大小用 sizeof 测度。

返回值，要检查，
NULL 就是失败啦。
成功得到句柄后，
收发数据就用它。
```

---

## 参考资料

- **FreeRTOS官方文档**：[xQueueCreate()](https://www.freertos.org/a00116.html)
- **Mastering-the-FreeRTOS-Kernel**：第5章 - 队列
- **项目代码示例**：[main.c](../../../../projects/stm32-learning/01-getting-started/03-queue-comm/src/main.c)

---

## 下一步

现在你已经了解了 `xQueueCreate()` 的详细用法，接下来学习：
- [如何发送和接收数据](./03_发送接收API详解.md)
- [队列的实际应用](./04_队列使用实践指南.md)
