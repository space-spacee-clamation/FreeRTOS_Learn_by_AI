# 03 - 发送接收API详解

## 目录

1. [发送数据：xQueueSend()](#发送数据xqueuesend)
2. [从队尾发送：xQueueSendToBack()](#从队尾发送xqueuesendtoback)
3. [从队首发送：xQueueSendToFront()](#从队首发送xqueuesendtofront)
4. [接收数据：xQueueReceive()](#接收数据xqueuereceive)
5. [查看数据（不移除）：xQueuePeek()](#查看数据不移除xqueuepeek)
6. [查询队列状态](#查询队列状态)
7. [完整示例](#完整示例)

---

## 发送数据：xQueueSend()

### 函数原型

```c
BaseType_t xQueueSend(
    QueueHandle_t xQueue,           /* 队列句柄 */
    const void * const pvItemToQueue, /* 要发送的数据指针 */
    TickType_t xTicksToWait         /* 等待时间（如果队列满） */
);
```

### 参数详细说明

#### 参数1：xQueue - 队列句柄

**类型**：`QueueHandle_t`

**作用**：指定要发送到哪个队列

**要求**：
- 必须是之前通过 `xQueueCreate()` 创建的有效句柄
- 不能是 `NULL`

**示例**：
```c
QueueHandle_t xIntQueue = xQueueCreate(10, sizeof(uint32_t));

// 发送数据到这个队列
xQueueSend(xIntQueue, &ulValue, 0);
```

---

#### 参数2：pvItemToQueue - 数据指针

**类型**：`const void * const`

**作用**：指向要发送的数据

**重要**：
- 队列会**拷贝**这个数据，不是存储指针
- 数据拷贝的大小由创建队列时的 `uxItemSize` 决定
- 数据发送后，原数据可以立即修改或销毁

**示例1：发送基本类型**
```c
// 发送整数
uint32_t ulValue = 123;
xQueueSend(xQueue, &ulValue, 0);  // 队列拷贝ulValue的值

// 发送浮点数
float fValue = 3.14f;
xQueueSend(xQueue, &fValue, 0);

// 发送字符
char cValue = 'A';
xQueueSend(xQueue, &cValue, 0);
```

**示例2：发送结构体**
```c
typedef struct {
    uint32_t id;
    float value;
} SensorData_t;

SensorData_t xData = {
    .id = 1,
    .value = 25.5f
};

// 发送整个结构体
xQueueSend(xQueue, &xData, 0);  // 队列拷贝整个结构体
```

**示例3：发送指针（注意！）**
```c
// ⚠️ 注意：队列拷贝的是指针本身，不是指针指向的数据！

char *pcString = "Hello";

// 这样发送的是指针值（地址）
xQueueSend(xQueue, &pcString, 0);

// 如果pcString指向的内存被修改或释放，接收方会出问题！
```

---

#### 参数3：xTicksToWait - 等待时间

**类型**：`TickType_t`

**作用**：如果队列满了，任务等待的时间

**可选值**：
- `0`：不等待，立即返回
- `pdMS_TO_TICKS(ms)`：等待指定毫秒数
- `portMAX_DELAY`：无限等待（直到队列有空间）

**示例**：
```c
// 不等待，如果队列满立即返回错误
xQueueSend(xQueue, &ulValue, 0);

// 等待100毫秒
xQueueSend(xQueue, &ulValue, pdMS_TO_TICKS(100));

// 无限等待（直到队列有空间）
xQueueSend(xQueue, &ulValue, portMAX_DELAY);
```

**等待时的状态**：
```
调用 xQueueSend(xQueue, &data, pdMS_TO_TICKS(100))

    ↓
队列是否有空间？
    ├─ 有 → 立即拷贝数据，返回 pdPASS ✓
    │
    └─ 无 → 任务进入阻塞态，等待...
           ↓
       100ms内是否有空间？
           ├─ 有 → 被唤醒，拷贝数据，返回 pdPASS ✓
           │
           └─ 无 → 超时返回 errQUEUE_FULL ✗
```

---

### 返回值

**类型**：`BaseType_t`

**返回值**：
- `pdPASS`：发送成功
- `errQUEUE_FULL`：队列满，发送失败

**检查返回值的示例**：
```c
BaseType_t xStatus;
uint32_t ulValue = 123;

// 发送数据
xStatus = xQueueSend(xQueue, &ulValue, pdMS_TO_TICKS(100));

if(xStatus == pdPASS)
{
    // 发送成功
    printf("数据发送成功！\n");
}
else
{
    // 发送失败（队列满）
    printf("数据发送失败：队列满！\n");
}
```

---

## 从队尾发送：xQueueSendToBack()

### 函数原型

```c
BaseType_t xQueueSendToBack(
    QueueHandle_t xQueue,
    const void * const pvItemToQueue,
    TickType_t xTicksToWait
);
```

### 说明

- **作用**：把数据添加到队列的尾部（后端）
- **特点**：这是标准的FIFO行为
- **等同**：`xQueueSend()` 就是 `xQueueSendToBack()` 的宏定义！

**示例**：
```c
// 这两个函数是完全一样的！
xQueueSend(xQueue, &ulValue, 0);           // ← 推荐使用这个
xQueueSendToBack(xQueue, &ulValue, 0);     // ← 这个也可以
```

---

## 从队首发送：xQueueSendToFront()

### 函数原型

```c
BaseType_t xQueueSendToFront(
    QueueHandle_t xQueue,
    const void * const pvItemToQueue,
    TickType_t xTicksToWait
);
```

### 说明

- **作用**：把数据添加到队列的头部（前端）
- **特点**：数据会被优先接收（LIFO行为）
- **用途**：紧急数据需要优先处理时

**图解**：
```
队列状态：[A][B][C][ ][ ]
           ↑     ↑
          队首   队尾

xQueueSendToFront(xQueue, &X, 0);
结果：[X][A][B][C][ ]  ← X插队到最前面！

xQueueSendToBack(xQueue, &Y, 0);
结果：[X][A][B][C][Y]  ← Y添加到最后面
```

**示例**：
```c
// 普通数据发送到队尾
xQueueSend(xQueue, &普通数据, 0);

// 紧急数据发送到队首，优先处理
xQueueSendToFront(xQueue, &紧急数据, 0);
```

---

## 接收数据：xQueueReceive()

### 函数原型

```c
BaseType_t xQueueReceive(
    QueueHandle_t xQueue,      /* 队列句柄 */
    void * const pvBuffer,      /* 接收数据的缓冲区 */
    TickType_t xTicksToWait    /* 等待时间（如果队列空） */
);
```

### 参数详细说明

#### 参数1：xQueue - 队列句柄

**类型**：`QueueHandle_t`

**作用**：指定从哪个队列接收数据

---

#### 参数2：pvBuffer - 接收缓冲区

**类型**：`void *`

**作用**：指向存储接收数据的缓冲区

**重要**：
- 缓冲区必须足够大，能容纳一个数据项
- 队列会把数据**拷贝**到这个缓冲区
- 数据会从队列中**移除**！

**示例1：接收基本类型**
```c
uint32_t ulReceivedValue;  // 接收缓冲区

// 从队列接收数据
xQueueReceive(xQueue, &ulReceivedValue, 0);

// 使用接收到的数据
printf("收到：%d\n", ulReceivedValue);
```

**示例2：接收结构体**
```c
typedef struct {
    uint32_t id;
    float value;
} SensorData_t;

SensorData_t xReceivedData;  // 接收缓冲区

// 接收结构体
xQueueReceive(xQueue, &xReceivedData, 0);

// 使用接收到的数据
printf("ID: %d, Value: %.1f\n", 
       xReceivedData.id, xReceivedData.value);
```

---

#### 参数3：xTicksToWait - 等待时间

**类型**：`TickType_t`

**作用**：如果队列空，任务等待的时间

**可选值**：
- `0`：不等待，立即返回
- `pdMS_TO_TICKS(ms)`：等待指定毫秒数
- `portMAX_DELAY`：无限等待（直到有数据）

**示例**：
```c
uint32_t ulValue;

// 不等待，如果队列空立即返回
xQueueReceive(xQueue, &ulValue, 0);

// 等待100毫秒
xQueueReceive(xQueue, &ulValue, pdMS_TO_TICKS(100));

// 无限等待（直到有数据）
xQueueReceive(xQueue, &ulValue, portMAX_DELAY);
```

---

### 返回值

**类型**：`BaseType_t`

**返回值**：
- `pdPASS`：接收成功
- `errQUEUE_EMPTY`：队列空，接收失败

**检查返回值的示例**：
```c
BaseType_t xStatus;
uint32_t ulValue;

// 接收数据
xStatus = xQueueReceive(xQueue, &ulValue, pdMS_TO_TICKS(100));

if(xStatus == pdPASS)
{
    // 接收成功
    printf("收到数据：%d\n", ulValue);
}
else
{
    // 接收失败（队列空）
    printf("接收超时，队列空！\n");
}
```

---

## 查看数据（不移除）：xQueuePeek()

### 函数原型

```c
BaseType_t xQueuePeek(
    QueueHandle_t xQueue,
    void * const pvBuffer,
    TickType_t xTicksToWait
);
```

### 与 xQueueReceive() 的区别

| 函数 | 是否移除数据 | 用途 |
|------|------------|------|
| `xQueueReceive()` | ✅ 是 | 获取并消耗数据 |
| `xQueuePeek()` | ❌ 否 | 只查看数据 |

**图解**：
```
队列：[A][B][C][ ][ ]

调用 xQueueReceive()
结果：[B][C][ ][ ][ ]  ← A被移除了！
       返回：A

对比：
队列：[A][B][C][ ][ ]

调用 xQueuePeek()
结果：[A][B][C][ ][ ]  ← A还在！
       返回：A
```

**示例**：
```c
uint32_t ulValue;

// 先看看队列里有什么（不移除）
if(xQueuePeek(xQueue, &ulValue, 0) == pdPASS)
{
    printf("队列里有：%d\n", ulValue);
}

// 数据还在队列里，现在真正接收
xQueueReceive(xQueue, &ulValue, 0);
printf("现在取出了：%d\n", ulValue);
```

---

## 查询队列状态

### 1. 查询队列中有多少个数据：uxQueueMessagesWaiting()

```c
UBaseType_t uxQueueMessagesWaiting(QueueHandle_t xQueue);
```

**示例**：
```c
UBaseType_t uxCount;

// 查询队列中有多少个数据
uxCount = uxQueueMessagesWaiting(xQueue);

printf("队列中有 %d 个数据\n", uxCount);
```

### 2. 查询队列中还剩多少空间：uxQueueSpacesAvailable()

```c
UBaseType_t uxQueueSpacesAvailable(QueueHandle_t xQueue);
```

**示例**：
```c
UBaseType_t uxSpaces;

// 查询队列中还能存多少个数据
uxSpaces = uxQueueSpacesAvailable(xQueue);

printf("队列还能存 %d 个数据\n", uxSpaces);
```

---

## 完整示例

### 示例1：简单的发送和接收

```c
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include <stdio.h>

QueueHandle_t xIntegerQueue = NULL;

// 发送任务
void vSenderTask(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulCounter = 0;
    BaseType_t xStatus;

    for(;;)
    {
        ulCounter++;
        
        printf("准备发送：%d\n", ulCounter);
        
        // 发送数据，等待最多100ms
        xStatus = xQueueSend(
            xIntegerQueue,
            &ulCounter,
            pdMS_TO_TICKS(100)
        );
        
        if(xStatus == pdPASS)
        {
            printf("发送成功！\n");
        }
        else
        {
            printf("发送失败：队列满！\n");
        }
        
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// 接收任务
void vReceiverTask(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulReceivedValue;
    BaseType_t xStatus;

    for(;;)
    {
        // 接收数据，等待最多200ms
        xStatus = xQueueReceive(
            xIntegerQueue,
            &ulReceivedValue,
            pdMS_TO_TICKS(200)
        );
        
        if(xStatus == pdPASS)
        {
            printf("收到：%d\n", ulReceivedValue);
            
            // 处理数据
            uint32_t ulResult = ulReceivedValue * 2;
            printf("处理结果：%d\n", ulResult);
        }
        else
        {
            printf("等待超时...\n");
        }
    }
}

int main(void)
{
    // 创建队列
    xIntegerQueue = xQueueCreate(5, sizeof(uint32_t));
    
    if(xIntegerQueue == NULL)
    {
        printf("队列创建失败！\n");
        return -1;
    }
    
    // 创建任务
    xTaskCreate(vSenderTask, "Sender", 128, NULL, 1, NULL);
    xTaskCreate(vReceiverTask, "Receiver", 128, NULL, 2, NULL);
    
    // 启动调度器
    vTaskStartScheduler();
    
    for(;;);
    return 0;
}
```

### 示例2：使用xQueuePeek()查看数据

```c
#include "FreeRTOS.h"
#include "queue.h"
#include <stdio.h>

int main(void)
{
    QueueHandle_t xQueue;
    uint32_t ulValue;
    BaseType_t xStatus;
    
    // 创建队列
    xQueue = xQueueCreate(3, sizeof(uint32_t));
    
    // 添加一些数据
    ulValue = 10;
    xQueueSend(xQueue, &ulValue, 0);
    
    ulValue = 20;
    xQueueSend(xQueue, &ulValue, 0);
    
    ulValue = 30;
    xQueueSend(xQueue, &ulValue, 0);
    
    printf("队列中有 %d 个数据\n", 
           uxQueueMessagesWaiting(xQueue));
    
    // Peek：查看数据（不移除）
    xStatus = xQueuePeek(xQueue, &ulValue, 0);
    printf("Peek看到：%d\n", ulValue);
    printf("队列中还有 %d 个数据\n", 
           uxQueueMessagesWaiting(xQueue));
    
    // Receive：真正接收（移除）
    xStatus = xQueueReceive(xQueue, &ulValue, 0);
    printf("Receive收到：%d\n", ulValue);
    printf("队列中还有 %d 个数据\n", 
           uxQueueMessagesWaiting(xQueue));
    
    return 0;
}
```

**输出**：
```
队列中有 3 个数据
Peek看到：10
队列中还有 3 个数据
Receive收到：10
队列中还有 2 个数据
```

---

## 常见错误总结

| 错误 | 原因 | 解决方法 |
|------|------|---------|
| 数据总是发送失败 | 队列满且等待时间太短 | 增加等待时间，或增加队列长度 |
| 数据总是接收失败 | 队列空且等待时间太短 | 增加等待时间，或加快生产者速度 |
| 收到的数据不对 | 数据项大小设置错误 | 确保创建队列时 uxItemSize 正确 |
| 内存崩溃 | 接收缓冲区太小 | 确保缓冲区足够大 |
| 数据丢失 | 多个发送者同时发送 | 考虑使用互斥锁，或增加队列长度 |

---

## 记忆口诀

```
队列发送三参数，
句柄数据等待值。
成功返回 pdPASS，
队列满了会等待。

接收也是三参数，
数据必须拷出去。
Receive 会移除，
Peek 只看不拿去。

发送队首或队尾，
紧急数据插队先。
查询消息有多少，
空闲空间也能算。
```

---

## 参考资料

- **FreeRTOS官方文档**：[队列发送API](https://www.freertos.org/a00117.html)
- **FreeRTOS官方文档**：[队列接收API](https://www.freertos.org/a00118.html)
- **Mastering-the-FreeRTOS-Kernel**：第5章 - 队列
- **项目代码示例**：[main.c](../../../../projects/stm32-learning/01-getting-started/03-queue-comm/src/main.c)

---

## 下一步

现在你已经了解了队列发送和接收API的详细用法，接下来学习：
- [队列的实际应用](./04_队列使用实践指南.md)
- [常见问题排查](./05_常见问题FAQ.md)
