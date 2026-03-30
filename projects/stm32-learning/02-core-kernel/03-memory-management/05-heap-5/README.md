# 05-heap-5 - FreeRTOS heap_5内存分配器示例

## 📚 项目概述

本项目演示FreeRTOS中最灵活的内存分配器 - heap_5的使用方法，展示其管理多个非连续内存区域的能力。

**学习要点：**
- heap_5内存分配器的工作原理
- vPortDefineHeapRegions() 函数使用
- 多非连续内存区域管理
- 首次适配算法（First Fit）
- 相邻空闲块自动合并机制
- heap_5与其他heap方案的区别

## 🎯 学习目标

完成本项目后，你将能够：
1. 理解heap_5的工作原理
2. 掌握vPortDefineHeapRegions()的使用方法
3. 管理多个非连续内存区域
4. 理解heap_5的区域内合并机制
5. 评估heap_5的适用场景
6. 对比heap_5和其他heap分配器的差异

## 📖 文档导航

完整的教学文档位于 `doc/tutorial/project-guides/05-memory-management/` 目录：

### 1. 概念入门
- [01_堆内存管理概念介绍.md](../../../../doc/tutorial/project-guides/05-memory-management/01_堆内存管理概念介绍.md) - 理解FreeRTOS的内存管理
- [14_heap_5分配器详解.md](../../../../doc/tutorial/project-guides/05-memory-management/14_heap_5分配器详解.md) - heap_5的工作原理

### 2. API详解
- [03_pvPortMalloc_API详解.md](../../../../doc/tutorial/project-guides/05-memory-management/03_pvPortMalloc_API详解.md) - 内存分配函数
- [08_vPortFree_API详解.md](../../../../doc/tutorial/project-guides/05-memory-management/08_vPortFree_API详解.md) - 内存释放函数
- [15_vPortDefineHeapRegions_API详解.md](../../../../doc/tutorial/project-guides/05-memory-management/15_vPortDefineHeapRegions_API详解.md) - heap_5专用API

### 3. 实践指南
- [16_heap_5使用实践指南.md](../../../../doc/tutorial/project-guides/05-memory-management/16_heap_5使用实践指南.md) - heap_5的实际应用

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

程序启动后，你将看到heap_5的内存区域初始化和分配演示，特别注意观察：
1. vPortDefineHeapRegions()的调用
2. 多个内存区域的定义和初始化
3. 不同区域间的内存分配
4. 区域内的空闲块合并
5. heap_5的特点介绍

## 📂 项目结构

```
05-heap-5/
├── Makefile              # 构建脚本（已修改为使用heap_5.c）
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
    └── main.c           # 主程序（heap_5演示）
```

## 🔗 相关链接

- **项目文档**: [../../../../doc/tutorial/project-guides/05-memory-management/](../../../../doc/tutorial/project-guides/05-memory-management/)
- **上一项目**: [04-heap-4](../04-heap-4/README.md) - heap_4内存分配器
- **FreeRTOS官方文档**: [内存管理](https://www.freertos.org/a00111.html)
- **Mastering-the-FreeRTOS-Kernel**: [第3章 堆内存管理](../../../../doc/tutorial/Mastering-the-FreeRTOS-Kernel/03_堆内存管理.md)

## 💡 学习建议

1. **先理解概念**：阅读教学文档中的heap_5工作原理
2. **关注初始化**：理解vPortDefineHeapRegions()的重要性
3. **观察区域分配**：注意内存块分配在哪个区域
4. **理解限制**：heap_5不能跨区域合并空闲块
5. **做实验**：
   - 尝试定义不同大小的多个区域
   - 观察分配失败的情况
   - 理解heap_5的适用场景

## heap_5 的适用场景

heap_5适合以下场景：

✓ **多区域内存** - 系统有多个非连续的内存区域
✓ **内存映射** - 需要使用特定地址范围的内存
✓ **混合内存** - 同时使用内部SRAM和外部RAM
✓ **灵活配置** - 需要在运行时定义内存布局
✓ **复杂内存架构** - 有多个内存bank或段

heap_5不适合：

✗ **单区域系统** - 只有一个连续内存区域时用heap_4更简单
✗ **极简系统** - 不需要多区域支持
✗ **不需要灵活性** - 内存布局固定且简单

## heap_5 与其他分配器关键区别

| 特性 | heap_1 | heap_2 | heap_3 | heap_4 | heap_5 |
|------|--------|--------|--------|--------|--------|
| 内存释放 | ✗ 不支持 | ✓ 支持 | ✓ 支持 | ✓ 支持 | ✓ 支持 |
| 分配算法 | 线性分配 | 最佳适配 | 标准库 | 首次适配 | 首次适配 |
| 块合并 | - | ✗ 不合并 | 取决于库 | ✓ 自动合并 | ✓ 区域内合并 |
| 多区域 | ✗ 否 | ✗ 否 | ✗ 否 | ✗ 否 | ✓ 是 |
| 抗碎片化 | ✗ 无 | 弱 | 取决于库 | 强 | 区域内强 |
| 详细统计 | ✓ 基础 | ✓ 基础 | ✗ 无 | ✓ 完整 | ✓ 完整 |
| 复杂度 | 简单 | 中等 | 最简单 | 较高 | 最高 |
