# 02-heap-2 - FreeRTOS heap_2内存分配器示例

## 📚 项目概述

本项目演示FreeRTOS中支持内存释放的内存分配器 - heap_2的使用方法。

**学习要点：**
- heap_2内存分配器的工作原理
- `pvPortMalloc()` 内存分配
- `vPortFree()` 内存释放
- 内存碎片问题演示
- heap_2与heap_1的区别

## 🎯 学习目标

完成本项目后，你将能够：
1. 理解heap_2的工作原理
2. 掌握内存分配和释放API
3. 理解内存碎片问题
4. 了解heap_2的适用场景和限制
5. 对比heap_1和heap_2的差异

## 📖 文档导航

完整的教学文档位于 `doc/tutorial/project-guides/05-memory-management/` 目录：

### 1. 概念入门
- [01_堆内存管理概念介绍.md](../../../../doc/tutorial/project-guides/05-memory-management/01_堆内存管理概念介绍.md) - 理解FreeRTOS的内存管理
- [02_heap_1分配器详解.md](../../../../doc/tutorial/project-guides/05-memory-management/02_heap_1分配器详解.md) - heap_1的工作原理
- [07_heap_2分配器详解.md](../../../../doc/tutorial/project-guides/05-memory-management/07_heap_2分配器详解.md) - heap_2的工作原理

### 2. API详解
- [03_pvPortMalloc_API详解.md](../../../../doc/tutorial/project-guides/05-memory-management/03_pvPortMalloc_API详解.md) - 内存分配函数
- [08_vPortFree_API详解.md](../../../../doc/tutorial/project-guides/05-memory-management/08_vPortFree_API详解.md) - 内存释放函数

### 3. 实践指南
- [09_heap_2使用实践指南.md](../../../../doc/tutorial/project-guides/05-memory-management/09_heap_2使用实践指南.md) - heap_2的实际应用

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

程序启动后，你将看到heap_2的内存分配和释放演示，特别注意观察：
1. 内存分配成功
2. 内存可以释放
3. 释放中间块后产生碎片
4. 碎片导致大内存分配可能失败
5. 小块仍可利用碎片空间分配

## 📂 项目结构

```
02-heap-2/
├── Makefile              # 构建脚本（已修改为使用heap_2.c）
├── LM3S6965.ld          # LM3S6965链接脚本
├── STM32F103RB.ld       # STM32F103链接脚本
├── startup_lm3s6965.c   # LM3S6965启动文件
├── startup_stm32f103.c  # STM32F103启动文件
├── include/
│   ├── FreeRTOSConfig.h          # FreeRTOS配置
│   ├── FreeRTOSConfig_LM3S6965.h # LM3S6965专用配置
│   └── FreeRTOSConfig_STM32F103.h # STM32F103专用配置
└── src/
    └── main.c           # 主程序（heap_2演示）
```

## 🔗 相关链接

- **项目文档**: [../../../../doc/tutorial/project-guides/05-memory-management/](../../../../doc/tutorial/project-guides/05-memory-management/)
- **上一项目**: [01-heap-1](../01-heap-1/README.md) - heap_1内存分配器
- **下一项目**: [03-heap-3](../03-heap-3/README.md) - heap_3内存分配器
- **FreeRTOS官方文档**: [内存管理](https://www.freertos.org/a00111.html)
- **Mastering-the-FreeRTOS-Kernel**: [第3章 堆内存管理](../../../../doc/tutorial/Mastering-the-FreeRTOS-Kernel/03_堆内存管理.md)

## 💡 学习建议

1. **先理解概念**：阅读教学文档中的heap_2工作原理
2. **对比heap_1**：先运行heap_1项目，再运行heap_2观察差异
3. **观察碎片**：特别注意步骤4-7的碎片产生和影响
4. **做实验**：
   - 尝试不同的分配和释放顺序
   - 修改configTOTAL_HEAP_SIZE观察影响
   - 思考如何避免或减少内存碎片

## heap_2 的适用场景

heap_2适合以下场景：

✓ **需要内存释放** - 比heap_1更灵活
✓ **分配模式简单** - 分配和释放大小相对固定
✓ **可以接受碎片** - 碎片问题不严重的场景
✓ **最佳适配算法** - 需要高效利用小空闲块

heap_2不适合：

✗ **频繁分配释放** - 会产生严重碎片
✗ **长期运行系统** - 碎片会逐渐累积
✗ **内存受限系统** - 碎片浪费宝贵内存
✗ **需要块合并** - heap_4更适合这种场景

## heap_2 与 heap_1 关键区别

| 特性 | heap_1 | heap_2 |
|------|--------|--------|
| 内存释放 | ✗ 不支持 | ✓ 支持 |
| 内存碎片 | ✗ 无 | ✗ 有 |
| 块合并 | - | ✗ 不合并 |
| 分配算法 | 线性分配 | 最佳适配 |
| 适用场景 | 静态分配 | 简单动态 |
| 复杂度 | 简单 | 中等 |
