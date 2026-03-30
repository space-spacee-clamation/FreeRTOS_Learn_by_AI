# FreeRTOS队列通信示例 - 教学文档索引

## 📚 项目概述

这是STM32-FreeRTOS渐进式学习的第四个项目，通过这个项目你将学会使用FreeRTOS队列进行任务间通信。

## 🎯 学习目标

通过学习这个项目，你将能够：

1. ✅ 理解队列的概念和作用
2. ✅ 掌握 `xQueueCreate()` API的使用
3. ✅ 学会使用 `xQueueSend()` 发送数据
4. ✅ 学会使用 `xQueueReceive()` 接收数据
5. ✅ 理解队列的阻塞机制
6. ✅ 掌握任务间同步的方法

## 📖 文档导航

按照以下顺序阅读文档，效果最佳：

### 1. 概念入门
- **[01_队列概念介绍.md](01_队列概念介绍.md)** - 理解什么是队列
  - 为什么需要队列
  - 队列的工作原理
  - FIFO（先进先出）概念

### 2. API详解
- **[02_xQueueCreate_API详解.md](02_xQueueCreate_API详解.md)** - 队列创建函数详解
  - 函数参数说明
  - 队列长度和数据大小
  - 返回值分析

- **[03_发送接收API详解.md](03_发送接收API详解.md)** - 数据收发API
  - xQueueSend() 使用方法
  - xQueueReceive() 使用方法
  - 阻塞时间参数

### 3. 实践指南
- **[04_队列使用实践指南.md](04_队列使用实践指南.md)** - 队列的实际应用
  - 常见使用模式
  - 数据传递技巧
  - 队列与任务同步

### 4. 问题排查
- **[05_常见问题FAQ.md](05_常见问题FAQ.md)** - 遇到问题看这里
  - 最常见的队列问题及解决方案
  - 调试技巧
  - 如何避免常见错误

## 🚀 快速开始

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

## 📂 项目结构

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

## 🔗 相关链接

- **项目代码**: [../../../../projects/stm32-learning/01-getting-started/03-queue-comm/](../../../../projects/stm32-learning/01-getting-started/03-queue-comm/)
- **上一项目**: [02-task-schedule](../02-task-schedule/) - 任务调度
- **FreeRTOS官方文档**: [队列API](https://www.freertos.org/a00018.html)
- **Mastering-the-FreeRTOS-Kernel**: [第5章 队列](../../Mastering-the-FreeRTOS-Kernel/05_队列.md)

## 💡 学习建议

1. **按顺序阅读**：文档是按渐进式设计的，建议按顺序阅读
2. **边读边试**：阅读API文档时，可以修改代码试试看
3. **动手实验**：尝试修改队列长度、发送频率，观察变化
4. **遇到问题**：先看[05_常见问题FAQ.md](05_常见问题FAQ.md)，还是解决不了再搜索

祝你学习愉快！🎉

---

*参考资料：FreeRTOS官方文档 v10.x*
