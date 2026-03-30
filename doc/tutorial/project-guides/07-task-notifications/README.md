# 任务通知（Task Notifications）学习指南

## 章节概述

本章节学习FreeRTOS任务通知机制，这是一种轻量级的同步和通信方式，相比传统信号量和队列具有显著的性能优势。

## 学习目标

- 理解任务通知的工作原理
- 掌握二值通知的使用（替代二值信号量）
- 掌握计数通知的使用（替代计数信号量）
- 掌握邮箱通知的使用（替代轻量级队列）
- 了解任务通知的性能优势

## 对应项目

- **项目位置**: `projects/stm32-learning/02-core-kernel/06-task-notifications/`
- **项目README**: [项目说明文档](../../../../projects/stm32-learning/02-core-kernel/06-task-notifications/README.md)

## 学习路径

1. **先阅读项目README**，了解项目整体结构和运行方法
2. **编译并运行项目**，观察实际输出
3. **阅读main.c源码**，结合注释理解API使用
4. **修改代码实验**，尝试不同的通知方式
5. **参考官方文档**，深入理解原理

## 关键知识点

### 1. 任务通知基础

每个FreeRTOS任务都有：
- 一个32位的通知值
- 一个通知状态（pending/non-pending）

### 2. 二值通知模式

```c
// 发送通知（类似给出信号量）
xTaskNotifyGive(xTaskToNotify);

// 等待通知（类似获取信号量）
ulTaskNotifyTake(pdTRUE, xTicksToWait);
```

### 3. 计数通知模式

```c
// pdFALSE表示不清零，返回累计计数
ulTaskNotifyTake(pdFALSE, xTicksToWait);
```

### 4. 邮箱通知模式

```c
// 发送32位数据
xTaskNotify(xTaskToNotify, ulValue, eSetValueWithOverwrite);

// 等待并接收数据
xTaskNotifyWait(0x00, 0xFFFFFFFF, &ulValue, xTicksToWait);
```

## 参考资料

- [FreeRTOS官方文档 - 任务通知](https://www.freertos.org/RTOS-task-notifications.html)
- [Mastering the FreeRTOS Kernel - 第10章](../Mastering-the-FreeRTOS-Kernel/10_任务通知.md)
- [FreeRTOS Demo - TaskNotify.c](https://github.com/FreeRTOS/FreeRTOS/blob/main/FreeRTOS/Demo/Common/Minimal/TaskNotify.c)

## 常见问题

**Q: 任务通知可以替代所有信号量吗？**
A: 不能。任务通知只能发给一个特定任务，需要多接收者时仍需用信号量。

**Q: 任务通知的内存占用更小吗？**
A: 是的。任务通知不需要创建额外的内核对象，内存占用更小。

**Q: 任务通知更快吗？**
A: 是的。实测表明任务通知比二值信号量快约45%。
