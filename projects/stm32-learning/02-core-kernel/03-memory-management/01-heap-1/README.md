# 01-heap-1 - FreeRTOS heap_1内存分配器示例

## 📚 项目概述

本项目演示FreeRTOS中最简单的内存分配器 - heap_1的使用方法。

**学习要点：**
- heap_1内存分配器的工作原理
- `pvPortMalloc()` 内存分配
- `xPortGetFreeHeapSize()` 内存统计
- heap_1的特点和限制

## 🎯 学习目标

完成本项目后，你将能够：
1. 理解heap_1的工作原理
2. 掌握基本的内存分配API
3. 了解heap_1的适用场景和限制
4. 学会监控内存使用情况

## 📖 文档导航

完整的教学文档位于 `doc/tutorial/project-guides/05-memory-management/` 目录：

### 1. 概念入门
- [01_堆内存管理概念介绍.md](../../../../doc/tutorial/project-guides/05-memory-management/01_堆内存管理概念介绍.md) - 理解FreeRTOS的内存管理
- [02_heap_1分配器详解.md](../../../../doc/tutorial/project-guides/05-memory-management/02_heap_1分配器详解.md) - heap_1的工作原理

### 2. API详解
- [03_pvPortMalloc_API详解.md](../../../../doc/tutorial/project-guides/05-memory-management/03_pvPortMalloc_API详解.md) - 内存分配函数
- [04_内存统计API详解.md](../../../../doc/tutorial/project-guides/05-memory-management/04_内存统计API详解.md) - 获取内存使用情况

### 3. 实践指南
- [05_heap_1使用实践指南.md](../../../../doc/tutorial/project-guides/05-memory-management/05_heap_1使用实践指南.md) - heap_1的实际应用

### 4. 问题排查
- [06_常见问题FAQ.md](../../../../doc/tutorial/project-guides/05-memory-management/06_常见问题FAQ.md) - 常见问题及解决方案

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

[Demo] ========== 步骤1: 初始内存状态 ==========

---------- 内存状态 ----------
总堆大小: 51200 字节
剩余大小: XXXXX 字节
已分配: XXXXX 字节
-------------------------------

...
```

## 📂 项目结构

```
01-heap-1/
├── Makefile              # 构建脚本（已修改为使用heap_1.c）
├── LM3S6965.ld          # LM3S6965链接脚本
├── STM32F103RB.ld       # STM32F103链接脚本
├── startup_lm3s6965.c   # LM3S6965启动文件
├── startup_stm32f103.c  # STM32F103启动文件
├── include/
│   ├── FreeRTOSConfig.h          # FreeRTOS配置
│   ├── FreeRTOSConfig_LM3S6965.h # LM3S6965专用配置
│   └── FreeRTOSConfig_STM32F103.h # STM32F103专用配置
└── src/
    └── main.c           # 主程序（heap_1演示）
```

## 🔗 相关链接

- **项目文档**: [../../../../doc/tutorial/project-guides/05-memory-management/](../../../../doc/tutorial/project-guides/05-memory-management/)
- **上一项目**: [04-semaphore](../../01-getting-started/04-semaphore/README.md) - 信号量使用
- **FreeRTOS官方文档**: [内存管理](https://www.freertos.org/a00111.html)
- **Mastering-the-FreeRTOS-Kernel**: [第3章 堆内存管理](../../../../doc/tutorial/Mastering-the-FreeRTOS-Kernel/03_堆内存管理.md)
- **项目索引**: [../../../../项目索引.md](../../../../项目索引.md) - 查看所有学习项目

## 💡 学习建议

1. **先理解概念**：阅读教学文档中的概念介绍，理解heap_1的工作原理
2. **运行看效果**：先编译运行项目，观察内存分配过程
3. **逐行读代码**：配合代码中的注释，理解每个步骤
4. **注意限制**：特别注意heap_1不支持内存释放这个重要限制
5. **做实验**：
   - 尝试分配更大的内存块，观察什么时候会失败
   - 修改configTOTAL_HEAP_SIZE，观察影响
   - 思考什么样的项目适合使用heap_1

## heap_1 的适用场景

heap_1适合以下场景：

✓ **系统启动时静态分配** - 所有内存都在启动时分配好
✓ **简单的嵌入式系统** - 不需要动态内存管理
✓ **对确定性要求高** - 分配时间确定，不会有碎片
✓ **永不删除任务/队列** - 所有对象创建后一直存在

heap_1不适合：

✗ **需要动态分配释放** - 无法回收内存
✗ **内存受限系统** - 无法充分利用内存
✗ **复杂的应用** - 需要灵活的内存管理
