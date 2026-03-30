# 04-semaphore - FreeRTOS信号量使用示例

## 📚 项目概述

本项目演示如何在FreeRTOS中使用信号量进行任务同步和共享资源保护。

**学习要点：**
- 二值信号量 - 用于任务间同步
- 互斥信号量 - 用于共享资源保护
- `xSemaphoreTake()` 和 `xSemaphoreGive()` API的使用
- 优先级继承机制（互斥信号量）

## 🎯 学习目标

完成本项目后，你将能够：
1. 理解信号量的概念和用途
2. 创建和使用二值信号量进行任务同步
3. 创建和使用互斥信号量保护共享资源
4. 理解优先级继承机制的作用

## 📖 文档导航

完整的教学文档位于 `doc/tutorial/project-guides/04-semaphore/` 目录：

### 1. 概念入门
- [01_信号量概念介绍.md](../../../doc/tutorial/project-guides/04-semaphore/01_信号量概念介绍.md) - 信号量的基本概念、类型和应用场景

### 2. API详解
- [02_二值信号量API详解.md](../../../doc/tutorial/project-guides/04-semaphore/02_二值信号量API详解.md) - `xSemaphoreCreateBinary()`、`xSemaphoreTake()`、`xSemaphoreGive()`
- [03_互斥信号量API详解.md](../../../doc/tutorial/project-guides/04-semaphore/03_互斥信号量API详解.md) - `xSemaphoreCreateMutex()` 及优先级继承

### 3. 实践指南
- [04_信号量使用实践指南.md](../../../doc/tutorial/project-guides/04-semaphore/04_信号量使用实践指南.md) - 实际应用中的最佳实践

### 4. 问题排查
- [05_常见问题FAQ.md](../../../doc/tutorial/project-guides/04-semaphore/05_常见问题FAQ.md) - 常见问题及解决方案

## 🚀 快速开始

### 编译项目

```bash
# 编译 LM3S6965 版本（推荐用于QEMU）
make all-lm3s6965

# 或者编译 STM32F103 版本
make all-stm32f103
```

### 运行项目

```bash
# 在QEMU中运行 LM3S6965
make run-lm3s6965

# 运行 STM32F103（需要硬件或QEMU支持）
make run-stm32f103
```

### 预期输出

程序启动后，你将看到：

```
====================================
  FreeRTOS信号量使用示例
====================================

学习要点：
1. 二值信号量 - 任务同步
2. 互斥信号量 - 资源保护
3. xSemaphoreTake() / xSemaphoreGive()
4. 优先级继承（互斥信号量）

正在创建二值信号量...
二值信号量创建成功！

正在创建互斥信号量...
互斥信号量创建成功！

正在创建任务...
所有任务创建成功！

启动FreeRTOS调度器...

[Task1] 同步事件触发任务启动！
[Task2] 同步事件等待任务启动！
[Task3] 共享资源修改任务A启动！
[Task4] 共享资源修改任务B启动！
[Monitor] 监控任务启动！
...
```

## 📂 项目结构

```
04-semaphore/
├── Makefile              # 构建脚本
├── LM3S6965.ld          # LM3S6965链接脚本
├── STM32F103RB.ld       # STM32F103链接脚本
├── startup_lm3s6965.c   # LM3S6965启动文件
├── startup_stm32f103.c  # STM32F103启动文件
├── include/
│   ├── FreeRTOSConfig.h          # FreeRTOS配置（自动选择）
│   ├── FreeRTOSConfig_LM3S6965.h # LM3S6965专用配置
│   └── FreeRTOSConfig_STM32F103.h # STM32F103专用配置
└── src/
    └── main.c           # 主程序（信号量示例）
```

## 🔗 相关链接

- [FreeRTOS官方文档 - 信号量](https://www.freertos.org/a00113.html)
- [Mastering-the-FreeRTOS-Kernel 第8章 - 资源管理](../../../doc/tutorial/Mastering-the-FreeRTOS-Kernel/08_资源管理.md)
- [03-queue-comm 队列通信项目](../03-queue-comm/README.md) - 上一个学习项目
- [项目索引](../../../../项目索引.md) - 查看所有学习项目

## 💡 学习建议

1. **先理解概念**：阅读教学文档中的概念介绍，理解信号量的作用
2. **运行看效果**：先编译运行项目，观察输出，建立感性认识
3. **逐行读代码**：配合代码中的注释，理解每个API的使用方法
4. **动手修改**：尝试修改代码，观察行为变化
5. **做实验**：
   - 去掉互斥锁，观察共享资源被破坏的情况
   - 改变任务优先级，观察优先级继承的效果
   - 尝试使用计数信号量（拓展练习）
