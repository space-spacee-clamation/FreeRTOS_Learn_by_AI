# 04-heap-4 - FreeRTOS heap_4内存分配器示例

## 📚 项目概述

本项目演示FreeRTOS中最强大的内存分配器 - heap_4的使用方法，展示其相邻空闲块自动合并机制和优秀的抗碎片化能力。

**学习要点：**
- heap_4内存分配器的工作原理
- 首次适配算法（First Fit）
- 相邻空闲块自动合并机制
- vPortGetHeapStats() 详细统计
- 内存碎片化管理
- heap_4与heap_2的区别

## 🎯 学习目标

完成本项目后，你将能够：
1. 理解heap_4的工作原理
2. 掌握首次适配算法的特点
3. 理解相邻空闲块合并机制
4. 使用vPortGetHeapStats()获取详细统计
5. 评估heap_4的抗碎片化能力
6. 对比heap_2和heap_4的差异

## 📖 文档导航

完整的教学文档位于 `doc/tutorial/project-guides/05-memory-management/` 目录：

### 1. 概念入门
- [01_堆内存管理概念介绍.md](../../../../doc/tutorial/project-guides/05-memory-management/01_堆内存管理概念介绍.md) - 理解FreeRTOS的内存管理
- [07_heap_2分配器详解.md](../../../../doc/tutorial/project-guides/05-memory-management/07_heap_2分配器详解.md) - heap_2的工作原理
- [10_heap_3分配器详解.md](../../../../doc/tutorial/project-guides/05-memory-management/10_heap_3分配器详解.md) - heap_3的工作原理

### 2. API详解
- [03_pvPortMalloc_API详解.md](../../../../doc/tutorial/project-guides/05-memory-management/03_pvPortMalloc_API详解.md) - 内存分配函数
- [08_vPortFree_API详解.md](../../../../doc/tutorial/project-guides/05-memory-management/08_vPortFree_API详解.md) - 内存释放函数
- [04_内存统计API详解.md](../../../../doc/tutorial/project-guides/05-memory-management/04_内存统计API详解.md) - 内存统计函数

### 3. 实践指南
- [12_heap_4分配器详解.md](../../../../doc/tutorial/project-guides/05-memory-management/12_heap_4分配器详解.md) - heap_4的工作原理
- [13_heap_4使用实践指南.md](../../../../doc/tutorial/project-guides/05-memory-management/13_heap_4使用实践指南.md) - heap_4的实际应用

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

程序启动后，你将看到heap_4的内存分配和释放演示，特别注意观察：
1. 详细的内存统计信息（vPortGetHeapStats）
2. 释放相邻块时的自动合并
3. 空闲块数量的变化
4. heap_4的特点介绍
5. 与heap_2的对比

## 📂 项目结构

```
04-heap-4/
├── Makefile              # 构建脚本（已修改为使用heap_4.c）
├── LM3S6965.ld          # LM3S6965链接脚本
├── STM32F103RB.ld       # STM32F103链接脚本
├── README.md             # 本文件
├── startup_lm3s6965.c   # LM3S6965启动文件（已移除_sbrk）
├── startup_stm32f103.c  # STM32F103启动文件（已移除_sbrk）
├── include/
│   ├── FreeRTOSConfig.h          # FreeRTOS配置
│   ├── FreeRTOSConfig_LM3S6965.h # LM3S6965专用配置
│   └── FreeRTOSConfig_STM32F103.h # STM32F103专用配置
└── src/
    └── main.c           # 主程序（heap_4演示）
```

## 🔗 相关链接

- **项目文档**: [../../../../doc/tutorial/project-guides/05-memory-management/](../../../../doc/tutorial/project-guides/05-memory-management/)
- **上一项目**: [03-heap-3](../03-heap-3/README.md) - heap_3内存分配器
- **下一项目**: [05-heap-5](../05-heap-5/README.md) - heap_5内存分配器
- **FreeRTOS官方文档**: [内存管理](https://www.freertos.org/a00111.html)
- **Mastering-the-FreeRTOS-Kernel**: [第3章 堆内存管理](../../../../doc/tutorial/Mastering-the-FreeRTOS-Kernel/03_堆内存管理.md)

## 💡 学习建议

1. **先理解概念**：阅读教学文档中的heap_4工作原理
2. **对比分析**：理解heap_4与heap_2的区别
3. **观察合并效果**：特别注意释放相邻块时空闲块数量的变化
4. **查看详细统计**：理解vPortGetHeapStats()返回的各个字段含义
5. **做实验**：
   - 尝试不同的分配-释放模式
   - 观察空闲块合并的效果
   - 对比heap_2和heap_4在相同测试下的表现

## heap_4 的适用场景

heap_4适合以下场景：

✓ **通用内存管理** - 大多数FreeRTOS应用的首选
✓ **需要抗碎片化** - 频繁分配和释放不同大小的内存
✓ **需要详细统计** - 需要监控内存使用情况
✓ **长期运行系统** - 系统需要长时间稳定运行
✓ **复杂内存模式** - 内存分配释放模式不确定

heap_4不适合：

✗ **极致性能要求** - 比heap_2稍慢（合并开销）
✗ **极简系统** - 只需要简单的内存管理
✗ **确定执行时间** - 合并操作时间不确定

## heap_4 与其他分配器关键区别

| 特性 | heap_1 | heap_2 | heap_3 | heap_4 |
|------|--------|--------|--------|--------|
| 内存释放 | ✗ 不支持 | ✓ 支持 | ✓ 支持 | ✓ 支持 |
| 分配算法 | 线性分配 | 最佳适配 | 标准库 | 首次适配 |
| 块合并 | - | ✗ 不合并 | 取决于库 | ✓ 自动合并 |
| 抗碎片化 | ✗ 无 | 弱 | 取决于库 | 强 |
| 详细统计 | ✓ 基础 | ✓ 基础 | ✗ 无 | ✓ 完整 |
| 历史最小 | ✗ 无 | ✗ 无 | ✗ 无 | ✓ 有 |
| 执行时间 | ✓ 确定 | ✓ 相对确定 | ✗ 不确定 | ⚠️ 多数确定 |
| 复杂度 | 简单 | 中等 | 最简单 | 较高 |
