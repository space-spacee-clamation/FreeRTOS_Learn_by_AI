# FreeRTOS 事件组（Event Groups）学习项目

## 项目概述

本项目演示FreeRTOS事件组的使用方法，事件组是一种用于多任务同步和事件通知的强大机制。

## 学习要点

1. **事件组的概念和工作原理**
   - 事件组是一组二进制标志（事件位）的集合
   - 每个事件位代表一个特定的事件或条件
   - 适用于多任务之间的复杂同步场景

2. **事件位的设置、清除和等待**
   - `xEventGroupCreate()` - 创建事件组
   - `xEventGroupSetBits()` - 设置一个或多个事件位
   - `xEventGroupClearBits()` - 清除一个或多个事件位
   - `xEventGroupGetBits()` - 获取当前事件组状态

3. **AND/OR条件等待机制**
   - OR等待：等待任意一个事件位设置
   - AND等待：等待所有指定的事件位都设置

4. **xEventGroupWaitBits() API**
   - 参数说明：
     - `xClearOnExit`: 退出时是否清除等待的位
     - `xWaitForAllBits`: pdTRUE=AND, pdFALSE=OR
     - `xTicksToWait`: 超时时间

5. **xEventGroupSync() - 任务同步（会合点）**
   - 原子操作：设置自己的位 + 等待其他位
   - 所有任务到达后自动清除同步位
   - 适用于多任务会合场景

6. **超时处理机制**
   - 支持设置等待超时
   - 超时后返回当前事件组状态

## 硬件/仿真要求

- **LM3S6965**: QEMU仿真支持（推荐）
- **STM32F103**: QEMU仿真支持

## 运行方法

### 编译LM3S6965版本
```bash
make all-lm3s6965
```

### 在QEMU中运行LM3S6965版本
```bash
make run-lm3s6965
```

### 调试LM3S6965版本（等待GDB连接）
```bash
make debug-lm3s6965
```

### 编译STM32F103版本
```bash
make all-stm32f103
```

### 在QEMU中运行STM32F103版本
```bash
make run-stm32f103
```

### 清理编译产物
```bash
make clean
```

## 预期结果

运行后应该看到：

1. **传感器任务**：
   - 温度传感器每500ms读取一次数据
   - 湿度传感器每1000ms读取一次数据
   - 压力传感器每2000ms读取一次数据

2. **数据处理任务**：
   - 等待所有三个传感器就绪（AND条件）
   - 处理完成后设置处理完成事件位
   - 演示超时处理（5秒超时）

3. **监控任务**：
   - 监控任意传感器事件或处理完成（OR条件）
   - 实时显示触发的事件

4. **同步任务**：
   - 三个同步任务演示xEventGroupSync()
   - 任务1立即准备，任务2延迟500ms，任务3延迟1000ms
   - 所有任务到达会合点后同时继续执行

## 事件组 vs 其他同步机制对比

| 特性          | 队列      | 信号量    | 事件组    |
|---------------|-----------|-----------|-----------|
| 数据传递      | ✓ 有      | ✗ 无      | ✗ 无      |
| 多事件等待    | ✗ 无      | ✗ 无      | ✓ 有      |
| AND/OR逻辑    | ✗ 无      | ✗ 无      | ✓ 有      |
| 任务同步      | ✗ 无      | ✗ 无      | ✓ 有      |

## 项目结构

```
05-event-groups/
├── README.md              # 本文件
├── Makefile               # 编译脚本
├── LM3S6965.ld           # LM3S6965链接脚本
├── STM32F103RB.ld        # STM32F103链接脚本
├── src/
│   └── main.c            # 主程序
├── include/
│   ├── FreeRTOSConfig.h  # FreeRTOS配置
│   ├── FreeRTOSConfig_LM3S6965.h
│   └── FreeRTOSConfig_STM32F103.h
├── startup_lm3s6965.c    # LM3S6965启动文件
└── startup_stm32f103.c   # STM32F103启动文件
```

## 参考资料

- [FreeRTOS官方文档 - 事件组](https://www.freertos.org/FreeRTOS-Event-Groups.html)
- [FreeRTOS Demo项目 - EventGroupsDemo.c](https://github.com/FreeRTOS/FreeRTOS/blob/main/FreeRTOS/Demo/Common/Minimal/EventGroupsDemo.c)
- [Mastering the FreeRTOS Kernel - 第9章 事件组](../../../../doc/tutorial/Mastering-the-FreeRTOS-Kernel/09_事件组.md)

## 关键API总结

| API函数 | 用途 |
|---------|------|
| `xEventGroupCreate()` | 创建事件组 |
| `xEventGroupSetBits()` | 设置事件位 |
| `xEventGroupClearBits()` | 清除事件位 |
| `xEventGroupGetBits()` | 获取当前事件位 |
| `xEventGroupWaitBits()` | 等待事件位（支持AND/OR） |
| `xEventGroupSync()` | 任务同步（会合点） |
| `vEventGroupDelete()` | 删除事件组 |

## 注意事项

1. **事件位数限制**：
   - 8位或24位，取决于configUSE_16_BIT_TICKS配置
   - 本项目使用24位事件组

2. **清除位策略**：
   - 使用`xClearOnExit`参数自动清除
   - 或手动调用`xEventGroupClearBits()`

3. **超时处理**：
   - 总是检查返回值判断是否超时
   - 超时返回的是当前事件组状态

4. **ISR安全版本**：
   - `xEventGroupSetBitsFromISR()`
   - `xEventGroupGetBitsFromISR()`
   - `xEventGroupClearBitsFromISR()`
