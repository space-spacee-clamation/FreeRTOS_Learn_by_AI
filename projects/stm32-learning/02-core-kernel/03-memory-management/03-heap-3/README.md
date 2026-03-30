# 03-heap-3 - FreeRTOS heap_3内存分配器示例

## 📚 项目概述

本项目演示FreeRTOS中使用标准库malloc/free包装器的内存分配器 - heap_3的使用方法。

**学习要点：**
- heap_3内存分配器的工作原理
- `pvPortMalloc()` 内存分配
- `vPortFree()` 内存释放
- 线程安全实现
- heap_3的特点和限制

## 🎯 学习目标

完成本项目后，你将能够：
1. 理解heap_3的工作原理
2. 掌握内存分配和释放API
3. 理解线程安全的实现方式
4. 了解heap_3的适用场景和限制
5. 对比heap_1、heap_2和heap_3的差异

## 📖 文档导航

完整的教学文档位于 `doc/tutorial/project-guides/05-memory-management/` 目录：

### 1. 概念入门
- [01_堆内存管理概念介绍.md](../../../../doc/tutorial/project-guides/05-memory-management/01_堆内存管理概念介绍.md) - 理解FreeRTOS的内存管理
- [02_heap_1分配器详解.md](../../../../doc/tutorial/project-guides/05-memory-management/02_heap_1分配器详解.md) - heap_1的工作原理
- [07_heap_2分配器详解.md](../../../../doc/tutorial/project-guides/05-memory-management/07_heap_2分配器详解.md) - heap_2的工作原理
- [10_heap_3分配器详解.md](../../../../doc/tutorial/project-guides/05-memory-management/10_heap_3分配器详解.md) - heap_3的工作原理

### 2. API详解
- [03_pvPortMalloc_API详解.md](../../../../doc/tutorial/project-guides/05-memory-management/03_pvPortMalloc_API详解.md) - 内存分配函数
- [08_vPortFree_API详解.md](../../../../doc/tutorial/project-guides/05-memory-management/08_vPortFree_API详解.md) - 内存释放函数

### 3. 实践指南
- [11_heap_3使用实践指南.md](../../../../doc/tutorial/project-guides/05-memory-management/11_heap_3使用实践指南.md) - heap_3的实际应用

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

程序启动后，你将看到heap_3的内存分配和释放演示，特别注意观察：
1. 内存分配成功
2. 内存可以释放
3. 多任务环境下的线程安全
4. heap_3的特点介绍
5. 与其他分配器的对比

## 📂 项目结构

```
03-heap-3/
├── Makefile              # 构建脚本（已修改为使用heap_3.c）
├── LM3S6965.ld          # LM3S6965链接脚本
├── STM32F103RB.ld       # STM32F103链接脚本
├── README.md             # 本文件
├── startup_lm3s6965.c   # LM3S6965启动文件
├── startup_stm32f103.c  # STM32F103启动文件
├── include/
│   ├── FreeRTOSConfig.h          # FreeRTOS配置
│   ├── FreeRTOSConfig_LM3S6965.h # LM3S6965专用配置
│   └── FreeRTOSConfig_STM32F103.h # STM32F103专用配置
└── src/
    └── main.c           # 主程序（heap_3演示）
```

## 🔗 相关链接

- **项目文档**: [../../../../doc/tutorial/project-guides/05-memory-management/](../../../../doc/tutorial/project-guides/05-memory-management/)
- **上一项目**: [02-heap-2](../02-heap-2/README.md) - heap_2内存分配器
- **下一项目**: [04-heap-4](../04-heap-4/README.md) - heap_4内存分配器
- **FreeRTOS官方文档**: [内存管理](https://www.freertos.org/a00111.html)
- **Mastering-the-FreeRTOS-Kernel**: [第3章 堆内存管理](../../../../doc/tutorial/Mastering-the-FreeRTOS-Kernel/03_堆内存管理.md)

## 💡 学习建议

1. **先理解概念**：阅读教学文档中的heap_3工作原理
2. **对比分析**：理解heap_3与heap_1、heap_2的区别
3. **观察线程安全**：注意vTaskSuspendAll()的使用
4. **做实验**：
   - 尝试在不同任务中同时分配内存
   - 观察分配失败的情况
   - 思考heap_3的适用场景

## heap_3 的适用场景

heap_3适合以下场景：

✓ **已有标准库堆配置** - 链接器已配置好堆区域
✓ **简单应用** - 不需要复杂的内存管理
✓ **快速原型** - 快速上手，不需要考虑内存碎片
✓ **标准库依赖** - 项目已经使用了标准库

heap_3不适合：

✗ **需要确定执行时间** - 实时性要求高的系统
✗ **需要内存统计** - 无法获取空闲内存等信息
✗ **内存受限系统** - 标准库实现可能不够高效
✗ **深度嵌入式** - 没有标准库支持的环境

## heap_3 与其他分配器关键区别

| 特性 | heap_1 | heap_2 | heap_3 |
|------|--------|--------|--------|
| 内存释放 | ✗ 不支持 | ✓ 支持 | ✓ 支持 |
| 内存统计 | ✓ 支持 | ✓ 支持 | ✗ 不支持 |
| 执行时间 | ✓ 确定 | ✓ 相对确定 | ✗ 不确定 |
| 实现方式 | 自实现 | 自实现 | 标准库包装 |
| 线程安全 | - | - | ✓ 是 |
| 内存碎片 | ✗ 无 | ✗ 有 | 取决于标准库 |
| 复杂度 | 简单 | 中等 | 最简单 |
