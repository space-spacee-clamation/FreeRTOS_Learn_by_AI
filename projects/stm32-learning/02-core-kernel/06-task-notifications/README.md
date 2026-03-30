# FreeRTOS 任务通知（Task Notifications）学习项目

## 项目概述

本项目演示FreeRTOS任务通知的使用方法。任务通知是FreeRTOS提供的一种轻量级同步和通信机制，相比传统的信号量和队列，具有更快的执行速度和更小的内存占用。

## 学习要点

1. **任务通知的概念和工作原理**
   - 每个任务都有一个32位的通知值和通知状态
   - 任务通知直接发送给目标任务，不需要中间对象
   - 内存占用更小，执行速度更快

2. **二值通知 - 替代二值信号量**
   - `xTaskNotifyGive()` - 发送通知（类似给出信号量）
   - `ulTaskNotifyTake(pdTRUE, ...)` - 等待通知（类似获取信号量）
   - pdTRUE表示接收后清零通知值

3. **计数通知 - 替代计数信号量**
   - `ulTaskNotifyTake(pdFALSE, ...)` - 累计通知值
   - pdFALSE表示不清零，返回累计计数
   - 适用于事件计数场景

4. **邮箱通知 - 替代轻量级队列**
   - `xTaskNotify()` - 发送32位数据
   - `xTaskNotifyWait()` - 等待并接收32位数据
   - 支持不同的通知动作（eSetValueWithOverwrite等）

5. **性能优势**
   - 比二值信号量快约45%
   - 比计数信号量占用内存更少
   - 不需要创建额外的内核对象

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

1. **程序启动信息**：
   - FreeRTOS任务通知学习示例标题
   - 学习要点列表
   - 任务通知优势介绍

2. **二值通知演示**：
   - 发送任务每1000ms发送一个通知
   - 接收任务等待并打印通知计数
   - 演示5次通知后完成

## 任务通知 vs 传统机制对比

| 特性          | 队列      | 信号量    | 任务通知  |
|---------------|-----------|-----------|-----------|
| 内存占用      | 大        | 中        | 小        |
| 执行速度      | 慢        | 中        | 快        |
| 数据传递      | ✓ 任意    | ✗ 无      | ✓ 32位    |
| 多接收者      | ✓ 支持    | ✓ 支持    | ✗ 单任务  |
| ISR安全       | ✓ 支持    | ✓ 支持    | ✓ 支持    |

## 项目结构

```
06-task-notifications/
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

- [FreeRTOS官方文档 - 任务通知](https://www.freertos.org/RTOS-task-notifications.html)
- [FreeRTOS Demo项目 - TaskNotify.c](https://github.com/FreeRTOS/FreeRTOS/blob/main/FreeRTOS/Demo/Common/Minimal/TaskNotify.c)
- [Mastering the FreeRTOS Kernel - 第8章 任务通知](../../../../doc/tutorial/Mastering-the-FreeRTOS-Kernel/08_任务通知.md)

## 关键API总结

| API函数 | 用途 |
|---------|------|
| `xTaskNotifyGive()` | 发送通知（给信号量） |
| `ulTaskNotifyTake()` | 等待通知（取信号量） |
| `xTaskNotify()` | 发送通知带32位数据 |
| `xTaskNotifyWait()` | 等待通知接收数据 |
| `xTaskNotifyAndQuery()` | 发送并查询原值 |

## 通知动作（eAction）

| 动作值 | 说明 |
|--------|------|
| `eNoAction` | 只更新通知状态，不修改值 |
| `eSetBits` | 按位或设置通知值 |
| `eIncrement` | 通知值加1（xTaskNotifyGive用） |
| `eSetValueWithOverwrite` | 设置值，覆盖原有值 |
| `eSetValueWithoutOverwrite` | 设置值，不覆盖 |

## 注意事项

1. **单接收者限制**：
   - 任务通知只能发给一个特定任务
   - 不支持多个任务等待同一个通知
   - 需要多接收者时请用信号量或队列

2. **通知值清零策略**：
   - `ulTaskNotifyTake(pdTRUE)` - 接收后清零
   - `ulTaskNotifyTake(pdFALSE)` - 不清零，返回累计
   - `xTaskNotifyWait()` - 通过参数控制

3. **ISR安全版本**：
   - `xTaskNotifyGiveFromISR()`
   - `xTaskNotifyFromISR()`
   - `vTaskNotifyGiveFromISR()`

4. **性能考虑**：
   - 简单同步场景优先使用任务通知
   - 复杂场景仍需用信号量/队列
   - 任务通知是FreeRTOS特有的优化机制
