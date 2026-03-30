# FreeRTOS堆内存管理 - 教学文档索引

## 📚 项目概述

这是STM32-FreeRTOS渐进式学习的第五个专题，通过这个项目你将学会FreeRTOS的堆内存管理机制，从最简单的heap_1开始。

## 🎯 学习目标

通过学习这个专题，你将能够：

1. ✅ 理解FreeRTOS的内存管理架构
2. ✅ 掌握heap_1分配器的工作原理
3. ✅ 学会使用 `pvPortMalloc()` 分配内存
4. ✅ 学会获取内存使用统计信息
5. ✅ 理解不同heap分配器的区别
6. ✅ 能够选择合适的内存分配方案

## 📖 文档导航

按照以下顺序阅读文档，效果最佳：

### 1. 概念入门
- **[01_堆内存管理概念介绍.md](01_堆内存管理概念介绍.md)** - 理解什么是堆内存管理
  - 为什么需要堆内存管理
  - FreeRTOS的5种heap分配器
  - 静态 vs 动态内存分配

- **[02_heap_1分配器详解.md](02_heap_1分配器详解.md)** - heap_1的工作原理
  - heap_1的实现原理
  - 分配算法
  - 优缺点分析

### 2. API详解
- **[03_pvPortMalloc_API详解.md](03_pvPortMalloc_API详解.md)** - 内存分配函数
  - `pvPortMalloc()` 使用方法
  - 参数说明
  - 返回值处理

- **[04_内存统计API详解.md](04_内存统计API详解.md)** - 获取内存使用情况
  - `xPortGetFreeHeapSize()`
  - `xPortGetMinimumEverFreeHeapSize()`（部分heap支持）

### 3. 实践指南
- **[05_heap_1使用实践指南.md](05_heap_1使用实践指南.md)** - heap_1的实际应用
  - 适用场景
  - 配置建议
  - 最佳实践

### 4. 问题排查
- **[06_常见问题FAQ.md](06_常见问题FAQ.md)** - 遇到问题看这里
  - 最常见的内存问题及解决方案
  - 调试技巧
  - 如何避免常见错误

## 🚀 快速开始

### 编译项目

```bash
cd projects/stm32-learning/02-core-kernel/03-memory-management/01-heap-1

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
  FreeRTOS heap_1 内存分配器示例
====================================

学习要点：
1. heap_1 内存分配器工作原理
2. pvPortMalloc() 分配内存
3. xPortGetFreeHeapSize() 获取统计
4. heap_1 的特点和限制

====================================
  heap_1 的特点：
====================================
✓ 实现简单，代码量小
✓ 没有内存碎片
✓ 执行时间确定

✗ 不支持内存释放
✗ 内存使用后无法回收
✗ 适合静态分配场景
====================================

正在创建任务...
所有任务创建成功！

启动FreeRTOS调度器...

[Demo] 内存演示任务启动！
[Monitor] 监控任务启动！
...
```

## 📂 项目结构

```
05-memory-management/
├── README.md                          # 本文档
├── 01_堆内存管理概念介绍.md
├── 02_heap_1分配器详解.md
├── 03_pvPortMalloc_API详解.md
├── 04_内存统计API详解.md
├── 05_heap_1使用实践指南.md
└── 06_常见问题FAQ.md
```

## 🔗 相关链接

- **项目代码**: [../../../../projects/stm32-learning/02-core-kernel/03-memory-management/01-heap-1/](../../../../projects/stm32-learning/02-core-kernel/03-memory-management/01-heap-1/)
- **上一项目**: [04-semaphore](../04-semaphore/) - 信号量使用
- **FreeRTOS官方文档**: [内存管理](https://www.freertos.org/a00111.html)
- **Mastering-the-FreeRTOS-Kernel**: [第3章 堆内存管理](../../Mastering-the-FreeRTOS-Kernel/03_堆内存管理.md)

## 💡 学习建议

1. **按顺序阅读**：文档是按渐进式设计的，建议按顺序阅读
2. **边读边试**：阅读API文档时，可以修改代码试试看
3. **动手实验**：尝试分配不同大小的内存，观察变化
4. **思考对比**：思考heap_1和其他heap分配器的区别
5. **遇到问题**：先看[06_常见问题FAQ.md](06_常见问题FAQ.md)，还是解决不了再搜索

## FreeRTOS的5种Heap分配器

| 分配器 | 特点 | 适用场景 |
|--------|------|---------|
| **heap_1** | 最简单，不支持释放 | 启动时静态分配所有内存 |
| **heap_2** | 支持释放，有碎片 | 简单的动态分配 |
| **heap_3** | 封装标准库malloc/free | 需要使用标准库 |
| **heap_4** | 支持释放，合并相邻块 | 通用场景，推荐 |
| **heap_5** | 支持多个内存区域 | 非连续内存的系统 |

本专题从heap_1开始学习，后续会逐步介绍其他分配器。

祝你学习愉快！🎉

---

*参考资料：FreeRTOS官方文档 v10.x - 内存管理*
