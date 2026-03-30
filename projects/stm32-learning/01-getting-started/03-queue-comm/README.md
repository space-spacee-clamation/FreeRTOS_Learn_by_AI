# FreeRTOS队列通信示例

## 项目概述

这是STM32-FreeRTOS渐进式学习的第四个项目，演示如何使用FreeRTOS队列进行任务间通信。

## 学习目标

通过学习这个项目，你将能够：

1. 理解什么是队列以及为什么需要队列
2. 掌握创建队列的方法
3. 学会发送数据到队列
4. 学会从队列接收数据
5. 使用队列进行任务同步

## 项目结构

```
03-queue-comm/
├── Makefile                    # 编译脚本
├── LM3S6965.ld                 # LM3S6965链接脚本
├── STM32F103RB.ld              # STM32F103链接脚本
├── startup_lm3s6965.c          # LM3S6965启动文件
├── startup_stm32f103.c         # STM32F103启动文件
├── include/
│   ├── FreeRTOSConfig.h         # LM3S6965 FreeRTOS配置
│   ├── FreeRTOSConfig_LM3S6965.h
│   └── FreeRTOSConfig_STM32F103.h
└── src/
    └── main.c                 # 主程序（阅读学习重点）
```

## 学习要点

### 1. 队列的概念

- **队列**：任务间传递数据的缓冲区
- **FIFO**：先进先出（First In, First Out）
- **线程安全**：队列操作是原子的，自动处理同步

### 2. 本项目的四个任务

| 任务 | 功能 | 优先级 |
|-----|------|-------|
| IntSender | 发送整数到队列 | 1 |
| StrSender | 发送字符串到队列 | 1 |
| IntReceiver | 从队列接收整数 | 2 |
| StrReceiver | 从队列接收字符串 | 2 |

### 3. 主要API

- `xQueueCreate()` - 创建队列
- `xQueueSend()` - 发送数据到队列
- `xQueueReceive()` - 从队列接收数据
- `xQueuePeek()` - 查看数据（不移除）

## 快速开始

### 编译项目

```bash
cd projects/stm32-learning/01-getting-started/03-queue-comm

# 编译LM3S6965版本
make all-lm3s6965

# 或者编译STM32F103版本
make all-stm32f103
```

### 运行项目

```bash
# 运行LM3S6965（推荐初学者）
make run-lm3s6965

# 运行STM32F103
make run-stm32f103
```

**退出QEMU**：按 `Ctrl+A` 然后按 `X`

### 预期输出

```
====================================
  FreeRTOS队列通信示例
====================================

学习要点：
1. 创建队列 xQueueCreate()
2. 发送数据 xQueueSend()
3. 接收数据 xQueueReceive()
4. 任务间通信与同步

正在创建队列...
队列创建成功！

正在创建任务...
所有任务创建成功！

启动FreeRTOS调度器...

[Sender1] 整数发送任务启动！
[Sender2] 字符串发送任务启动！
[Receiver1] 整数接收任务启动！
[Receiver2] 字符串接收任务启动！
[Sender1] 准备发送: 1
[Sender1] 发送成功！
[Receiver1] 收到数据: 1
[Receiver1] 处理结果: 2
[Sender2] 准备发送: Hello #1
[Sender2] 发送成功！
[Receiver2] 收到消息: Hello #1
...
```

## 观察重点

运行时注意观察：

1. **发送-接收配对**：Sender发送数据后，Receiver会收到
2. **队列缓冲**：如果Receiver处理慢，数据会在队列中排队
3. **同步机制**：队列自动处理任务间的同步

## 思考问题

1. 如果发送者比接收者快很多，会发生什么？
2. 队列长度设为1和设为10有什么区别？
3. 除了传递数据，队列还能用来做什么？

## 相关链接

- **上一项目**: [02-task-schedule](../02-task-schedule/) - 任务调度
- **FreeRTOS官方文档**: [队列API](https://www.freertos.org/a00018.html)
- **Mastering-the-FreeRTOS-Kernel**: [第5章 队列](../../../../doc/tutorial/Mastering-the-FreeRTOS-Kernel/05_队列.md)
- **教学文档**: [../../../../doc/tutorial/project-guides/03-queue-comm/](../../../../doc/tutorial/project-guides/03-queue-comm/)

## 学习建议

1. **先运行观察**：编译运行项目，观察数据是如何在任务间传递的
2. **修改队列长度**：尝试修改队列长度，观察对系统的影响
3. **修改发送频率**：调整发送任务的延时时间，观察队列满的情况
4. **阅读代码**：重点理解队列创建、发送、接收的API使用
