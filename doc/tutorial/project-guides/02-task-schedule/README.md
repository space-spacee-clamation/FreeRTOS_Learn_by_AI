# FreeRTOS任务调度示例 - 教学文档索引

## 📚 项目概述

这是STM32-FreeRTOS渐进式学习的第三个项目，通过这个项目你将学会FreeRTOS的多任务调度和优先级管理。

## 🎯 学习目标

通过学习这个项目，你将能够：

1. ✅ 理解任务优先级的概念
2. ✅ 掌握抢占式调度的工作原理
3. ✅ 观察不同优先级任务的运行顺序
4. ✅ 理解时间片轮转调度
5. ✅ 学会创建和管理多个任务

## 📖 文档导航

按照以下顺序阅读文档，效果最佳：

### 1. 概念入门
- **[01_调度器概念介绍.md](01_调度器概念介绍.md)** - 理解调度器的作用
  - 什么是调度器
  - 为什么需要调度器
  - 抢占式 vs 协作式调度

### 2. 优先级详解
- **[02_任务优先级详解.md](02_任务优先级详解.md)** - 深入理解任务优先级
  - 优先级的数值含义
  - 如何设置优先级
  - 优先级继承概念

### 3. 调度策略
- **[03_调度策略详解.md](03_调度策略详解.md)** - FreeRTOS的调度算法
  - 抢占式调度
  - 时间片轮转
  - 调度器工作流程

### 4. 实践指南
- **[04_多任务创建实践.md](04_多任务创建实践.md)** - 实际创建多个任务
  - 任务创建步骤
  - 任务参数配置
  - 调试技巧

### 5. 问题排查
- **[05_常见问题FAQ.md](05_常见问题FAQ.md)** - 遇到问题看这里
  - 最常见的调度问题及解决方案
  - 调试技巧
  - 如何避免常见错误

## 🚀 快速开始

### 编译项目

```bash
cd projects/stm32-learning/01-getting-started/02-task-schedule

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
  FreeRTOS任务调度示例
====================================

学习要点：
1. 创建多个不同优先级的任务
2. 观察抢占式调度效果
3. 理解任务优先级的作用

任务配置：
- HighTask:   优先级 3, 周期 200ms
- MediumTask: 优先级 2, 周期 300ms
- LowTask:    优先级 1, 周期 500ms

正在创建任务...
所有任务创建成功！
启动FreeRTOS调度器...

[High] 高优先级任务启动！
[High] 运行 #0
[Medium] 中优先级任务启动！
[Medium] 运行 #0
[Low] 低优先级任务启动！
[Low] 运行 #0
[High] 运行 #1
[Medium] 运行 #1
[High] 运行 #2
[Low] 运行 #1
...
```

## 📂 项目结构

```
02-task-schedule/
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

- **项目代码**: [../../../../projects/stm32-learning/01-getting-started/02-task-schedule/](../../../../projects/stm32-learning/01-getting-started/02-task-schedule/)
- **上一项目**: [01-first-task](../01-first-task/) - 第一个任务
- **下一项目**: [03-queue-comm](../03-queue-comm/) - 队列通信
- **FreeRTOS官方文档**: [调度器配置](https://www.freertos.org/a00110.html)
- **Mastering-the-FreeRTOS-Kernel**: [第4章 任务管理](../../Mastering-the-FreeRTOS-Kernel/04_任务管理.md)

## 💡 学习建议

1. **按顺序阅读**：文档是按渐进式设计的，建议按顺序阅读
2. **观察输出**：运行项目时仔细观察输出顺序，理解调度器行为
3. **动手实验**：尝试修改任务优先级和延时时间，观察变化
4. **思考原理**：思考为什么高优先级任务会先运行
5. **遇到问题**：先看[05_常见问题FAQ.md](05_常见问题FAQ.md)，还是解决不了再搜索

祝你学习愉快！🎉

---

*参考资料：FreeRTOS官方文档 v10.x*
