# 消息缓冲区（Message Buffers）学习指南

## 章节概述

本章节学习FreeRTOS消息缓冲区（Message Buffers）机制，这是一种用于传递变长消息的高级通信机制。消息缓冲区在流缓冲区的基础上增加了消息边界标识，特别适合需要保持消息完整性的场景。

## 学习目标

- 理解消息缓冲区与流缓冲区的区别
- 掌握变长消息的发送和接收
- 掌握任务间消息传递
- 掌握中断-任务消息通信（FromISR API）
- 学会消息缓冲区与流缓冲区的对比使用
- 理解消息长度前缀的工作原理

## 对应项目

- **项目位置**: `projects/stm32-learning/02-core-kernel/08-message-buffers/`
- **项目README**: [项目说明文档](../../../../projects/stm32-learning/02-core-kernel/08-message-buffers/README.md)

## 学习路径

1. **先阅读项目README**，了解项目整体结构和运行方法
2. **编译并运行项目**，观察实际输出
3. **阅读main.c源码**，结合注释理解API使用
4. **理解消息缓冲区机制**，对比流缓冲区的差异
5. **修改代码实验**，尝试不同的消息长度和场景
6. **参考官方文档**，深入理解原理

## 关键知识点

### 1. 消息缓冲区与流缓冲区对比

| 特性 | 消息缓冲区（Message Buffer） | 流缓冲区（Stream Buffer） |
|------|----------------------------|-------------------------|
| 消息边界 | 保持消息边界 | 字节流，无边界 |
| 存储方式 | 每条消息带长度前缀 | 连续字节流 |
| 读取单位 | 一次读取一条完整消息 | 可读取任意字节数 |
| 适用场景 | 命令、数据包传输 | 数据流、串口数据 |

```
消息缓冲区存储格式:
+--------+----------------+--------+----------------+
| 长度(4)| 消息1数据      | 长度(4)| 消息2数据      |
+--------+----------------+--------+----------------+

流缓冲区存储格式:
+--+--+--+--+--+--+--+--+--+--+
| A| B| C| D| E| F| G| H| I| J|  (字节流)
+--+--+--+--+--+--+--+--+--+--+
```

### 2. 消息缓冲区创建

```c
#include "message_buffer.h"

/* 创建消息缓冲区 */
/* 参数: 缓冲区总大小（字节） */
/* 返回: 消息缓冲区句柄，失败返回NULL */
xMessageBuffer = xMessageBufferCreate(MESSAGE_BUFFER_SIZE);

if(xMessageBuffer == NULL)
{
    /* 创建失败处理 */
}
```

**注意**: 缓冲区大小需要考虑消息长度前缀（每个消息额外4字节）。

### 3. 发送消息

```c
/* 发送消息 */
/* 参数1: 消息缓冲区句柄 */
/* 参数2: 指向要发送的数据的指针 */
/* 参数3: 数据长度（字节） */
/* 参数4: 阻塞时间（如果缓冲区满） */
/* 返回: 实际发送的字节数 */
size_t xSent = xMessageBufferSend(xMessageBuffer,
                                   pvData,
                                   xDataLength,
                                   xBlockTime);

if(xSent == xDataLength)
{
    /* 发送成功 */
}
```

### 4. 接收消息

```c
uint8_t ucBuffer[MAX_MESSAGE_SIZE];
size_t xReceivedLength;

/* 接收消息 */
/* 参数1: 消息缓冲区句柄 */
/* 参数2: 接收缓冲区指针 */
/* 参数3: 接收缓冲区大小 */
/* 参数4: 阻塞时间（如果缓冲区空） */
/* 返回: 接收到的消息长度 */
xReceivedLength = xMessageBufferReceive(xMessageBuffer,
                                         ucBuffer,
                                         sizeof(ucBuffer),
                                         xBlockTime);

if(xReceivedLength > 0)
{
    /* 处理接收到的消息 */
    /* ucBuffer包含完整消息，长度为xReceivedLength */
}
```

**注意**: 如果接收缓冲区小于消息长度，消息会被丢弃！

### 5. 中断中发送消息（FromISR）

```c
BaseType_t xHigherPriorityTaskWoken = pdFALSE;
size_t xSent;

/* 在ISR中发送消息 */
xSent = xMessageBufferSendFromISR(xMessageBuffer,
                                   pvData,
                                   xDataLength,
                                   &xHigherPriorityTaskWoken);

/* 如果有更高优先级任务被唤醒，请求上下文切换 */
if(xHigherPriorityTaskWoken == pdTRUE)
{
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
```

### 6. 其他有用的API

```c
/* 查询缓冲区中的空闲空间 */
size_t xSpacesAvailable = xMessageBufferSpacesAvailable(xMessageBuffer);

/* 查询下一条消息的长度（不读取） */
size_t xNextMessageLength = xMessageBufferNextLengthBytes(xMessageBuffer);

/* 复位消息缓冲区（清空所有数据） */
xMessageBufferReset(xMessageBuffer);

/* 删除消息缓冲区 */
vMessageBufferDelete(xMessageBuffer);
```

### 7. 流缓冲区的使用（对比演示）

```c
#include "stream_buffer.h"

/* 创建流缓冲区 */
/* 参数1: 缓冲区总大小 */
/* 参数2: 触发级别（发送字节数>=此值时唤醒接收任务） */
xStreamBuffer = xStreamBufferCreate(STREAM_BUFFER_SIZE, xTriggerLevel);

/* 发送字节流 */
xStreamBufferSend(xStreamBuffer, pvData, xDataLength, xBlockTime);

/* 接收字节流（可读取任意数量） */
xStreamBufferReceive(xStreamBuffer, pvBuffer, xBufferSize, xBlockTime);
```

## 参考资料

- [FreeRTOS官方文档 - Message Buffers](https://www.freertos.org/RTOS-message-buffer-API.html)
- [FreeRTOS官方文档 - Stream Buffers](https://www.freertos.org/RTOS-stream-buffer-API.html)
- [FreeRTOS Demo - MessageBufferDemo.c](https://github.com/FreeRTOS/FreeRTOS/blob/main/FreeRTOS/Demo/Common/Minimal/MessageBufferDemo.c)

## 常见问题

**Q: 消息缓冲区和队列有什么区别？**
A: 队列传递固定大小的数据项，消息缓冲区传递变长消息。队列更适合结构化数据，消息缓冲区更适合字符串、数据包等变长数据。

**Q: 每条消息的额外开销是多少？**
A: 每条消息有4字节的长度前缀开销。

**Q: 如果接收缓冲区比消息小会怎样？**
A: 整条消息会被丢弃！接收前可以用xMessageBufferNextLengthBytes()查询长度。

**Q: 消息缓冲区是线程安全的吗？**
A: 是的，FreeRTOS API都是线程安全的，但FromISR版本只能在ISR中使用。

**Q: 什么时候用消息缓冲区，什么时候用流缓冲区？**
A: 需要保持消息边界用消息缓冲区，处理连续字节流用流缓冲区。
