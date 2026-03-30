# heap_5 内存分配器学习项目规划

## 项目概述

heap_5是FreeRTOS提供的最灵活的内存分配器，支持跨多个非连续内存块的堆管理。本项目将深入学习heap_5的工作原理、配置方法和使用场景。

**核心特点：**
- 支持多个非连续的内存区域作为堆
- 继承heap_4的所有功能（首次适配、块合并等）
- 需要手动调用vPortDefineHeapRegions()初始化
- 适合内存地址不连续的嵌入式系统
- 可以将内部RAM和外部RAM组合使用

**与其他分配器的关键区别：**
- heap_1-4都使用单个连续的内存数组
- heap_5使用vPortDefineHeapRegions()定义多个区域
- heap_5可以利用分散的内存空间
- heap_5初始化更复杂但更灵活

## 学习目标

1. 理解heap_5的设计理念和多区域管理
2. 掌握HeapRegion_t结构体的使用
3. 学习vPortDefineHeapRegions()的调用时机和方法
4. 理解heap_5如何在多个区域间分配内存
5. 掌握heap_5的适用场景和配置要点
6. 学会在实际项目中合理划分内存区域

## 项目结构

```
projects/stm32-learning/02-core-kernel/03-memory-management/
└── 05-heap-5/
    ├── Makefile
    ├── LM3S6965.ld
    ├── STM32F103RB.ld
    ├── README.md
    ├── startup_lm3s6965.c
    ├── startup_stm32f103.c
    ├── include/
    │   ├── FreeRTOSConfig.h
    │   ├── FreeRTOSConfig_LM3S6965.h
    │   └── FreeRTOSConfig_STM32F103.h
    └── src/
        └── main.c
```

## 教学文档计划

在 `doc/tutorial/project-guides/05-memory-management/` 目录下创建：

1. `14_heap_5分配器详解.md`
   - heap_5的设计理念
   - HeapRegion_t结构体详解
   - vPortDefineHeapRegions()工作原理
   - 多区域内存管理机制
   - 与heap_4的继承关系
   - 初始化顺序的重要性

2. `15_heap_5使用实践指南.md`
   - 基本使用方法
   - 内存区域定义示例
   - 实际应用场景
   - 初始化顺序和注意事项
   - 与链接脚本的配合
   - 调试技巧

## 代码实现要点

### main.c 核心功能

1. **heap_5特点展示**
   - 打印heap_5的核心特性
   - 与其他分配器的对比表格
   - 多区域管理的优势说明

2. **多区域定义演示**
   - 定义多个模拟的内存区域
   - 使用HeapRegion_t结构体
   - 演示vPortDefineHeapRegions()调用
   - 强调必须在任何分配前调用

3. **跨区域分配测试**
   - 在不同区域分配内存
   - 验证分配器能正确使用所有区域
   - 展示分配顺序和策略
   - 验证区域边界的正确处理

4. **块合并验证**
   - 在单个区域内测试块合并
   - 验证跨区域不能合并（物理不连续）
   - 展示heap_4特性在heap_5中的继承

5. **详细内存统计**
   - 使用vPortGetHeapStats()获取统计
   - 展示总可用空间和各区域使用情况
   - 验证历史最小统计功能

6. **错误处理演示**
   - 演示未调用vPortDefineHeapRegions()的后果
   - 演示区域定义顺序错误的问题
   - 演示NULL结束标记的重要性

### 关键演示步骤

```
步骤1：展示heap_5特点和多区域概念
步骤2：定义多个内存区域（模拟）
步骤3：调用vPortDefineHeapRegions()初始化
步骤4：在各区域分配内存，验证跨区域分配
步骤5：释放内存，验证块合并（单区域内）
步骤6：展示详细统计信息
步骤7：演示常见错误和注意事项
步骤8：总结heap_5的适用场景
```

## 配置要求

### FreeRTOSConfig.h

```c
#define configTOTAL_HEAP_SIZE              ( ( size_t ) ( 20 * 1024 ) )  // 总堆空间
#define configSUPPORT_DYNAMIC_ALLOCATION   1
#define configUSE_MALLOC_FAILED_HOOK       1
```

### 内存分配器选择

在Makefile中指定使用heap_5.c：
```makefile
HEAP_SRC = $(FREERTOS_PATH)/Source/portable/MemMang/heap_5.c
```

### 内存区域定义示例

```c
/* heap_5特有：定义多个内存区域 */
/* 注意：这些区域必须按地址从低到高排序 */
HeapRegion_t xHeapRegions[] =
{
    /* 区域1：内部RAM低地址 */
    { ( uint8_t * ) 0x20000000UL, 0x4000 },   /* 16KB */
    
    /* 区域2：内部RAM高地址 */
    { ( uint8_t * ) 0x20004000UL, 0x4000 },   /* 16KB */
    
    /* 区域3：外部RAM（如果有） */
    { ( uint8_t * ) 0x60000000UL, 0x10000 },  /* 64KB */
    
    /* 结束标记（必需！）*/
    { NULL, 0 }
};

/* 必须在任何分配前调用！*/
vPortDefineHeapRegions(xHeapRegions);
```

## 预期输出示例

```
====================================
  FreeRTOS heap_5 内存分配器示例
====================================

学习要点：
1. heap_5 多区域内存管理
2. HeapRegion_t 结构体使用
3. vPortDefineHeapRegions() 初始化
4. 跨区域内存分配
5. 适用场景和注意事项

====================================
  heap_5 的特点：
====================================
✓ 支持多个非连续内存区域
✓ 继承heap_4的块合并功能
✓ 使用首次适配算法
✓ 需要手动初始化内存区域
✓ 适合内存地址分散的系统
====================================

[Demo] 步骤1：定义内存区域...
[Demo] 区域1: 0x20000000, 大小: 16384 字节
[Demo] 区域2: 0x20004000, 大小: 16384 字节
[Demo] 区域3: 0x60000000, 大小: 65536 字节
[Demo] 总计: 98304 字节

[Demo] 步骤2：调用 vPortDefineHeapRegions()...
[Demo] heap_5 初始化成功！

[Demo] 步骤3：分配内存，测试跨区域...
[Demo] 分配 块A (4000字节)... 成功 (区域1)
[Demo] 分配 块B (5000字节)... 成功 (区域1)
[Demo] 分配 块C (8000字节)... 成功 (区域2)
[Demo] 分配 块D (20000字节)... 成功 (区域3)
[Demo] ✓ 跨区域分配正常工作！

[Demo] 步骤4：释放部分内存，测试合并...
[Demo] 释放块A
[Demo] 释放块B
[Demo] 空闲块数量: 1  (区域1内合并了！)
[Demo] ✓ 单区域内块合并正常工作！

[Demo] 步骤5：详细内存统计...
[Demo] 总剩余: 81304 字节
[Demo] 历史最小: 79304 字节
[Demo] 空闲块数量: 3
[Demo] 最大空闲块: 45536 字节

[Demo] 步骤6：演示错误处理（故意）...
[Demo] 注意：如果不调用vPortDefineHeapRegions()会断言失败！
[Demo] 注意：区域必须按地址从低到高排序！
[Demo] 注意：必须有NULL结束标记！

[Demo] heap_5学习要点总结：
[Demo] 1. 支持多个非连续内存区域
[Demo] 2. 必须先调用vPortDefineHeapRegions()
[Demo] 3. 区域按地址从低到高排序
[Demo] 4. 继承heap_4的块合并功能
[Demo] 5. 适合内存地址分散的嵌入式系统
[Demo] 6. 可以组合内部和外部RAM
```

## 与其他分配器的关键对比

| 特性 | heap_1-4 | heap_5 |
|------|----------|--------|
| 内存区域 | 单个连续数组 | 多个非连续区域 |
| 初始化 | 自动 | 需手动调用vPortDefineHeapRegions() |
| HeapRegion_t | 不使用 | 必须使用 |
| 块合并 | heap_2/4支持 | 支持（单区域内） |
| 灵活性 | 较低 | 最高 |
| 复杂度 | 简单 | 中等 |
| 适用场景 | 内存连续 | 内存分散 |

## 实施步骤

1. 从heap_4项目复制基础结构
2. 修改Makefile使用heap_5.c
3. 重写main.c实现heap_5演示
4. 创建README.md文档
5. 编写教学文档（2个文件）
6. 编译测试
7. QEMU验证
8. 更新进度文档

## 注意事项

1. **初始化顺序最重要** - vPortDefineHeapRegions()必须在任何pvPortMalloc()之前调用，包括任务创建等隐式分配
2. **区域排序** - HeapRegion_t数组必须按地址从低到高排序
3. **结束标记** - 数组必须以{NULL, 0}结束
4. **跨区域不合并** - 物理不连续的区域不能合并
5. **单区域内合并** - 同一区域内的块仍会像heap_4一样合并
6. **适合场景** - STM32等有内部SRAM和外部SRAM的系统，或内存映射分散的系统
7. **不使用静态数组** - heap_5不使用ucHeap[]静态数组，完全通过vPortDefineHeapRegions()定义
