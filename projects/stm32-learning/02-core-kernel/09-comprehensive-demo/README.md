# FreeRTOS 综合应用学习项目

## 项目概述

本项目是FreeRTOS核心内核功能的综合应用示例，整合了之前学习的所有知识点，构建一个完整的多任务传感器数据采集与处理系统。

## 学习要点

### 1. 传感器数据采集（软件定时器）
- 使用软件定时器实现周期性采样
- 定时器回调函数的正确使用
- 模拟传感器数据生成

### 2. 数据处理（队列通信）
- 队列用于传递传感器数据
- 生产者-消费者模型实现
- 数据累加与平均值计算

### 3. 显示更新（任务通知）
- 使用任务通知触发显示更新
- 轻量级任务同步机制
- 数据格式化与输出

### 4. 命令控制（事件组）
- 事件组用于控制各任务状态
- 位标志管理任务启用/禁用
- 系统状态监控与切换

### 5. 多任务协调
- 任务优先级配置
- 资源共享与同步
- 完整的系统架构设计

## 系统架构

```
┌─────────────────────────────────────────────────────────────┐
│                     命令处理任务 (Priority 5)               │
│                    (事件组控制各任务状态)                    │
└────────────────────────┬────────────────────────────────────┘
                         │
         ┌───────────────┼───────────────┐
         │               │               │
         ▼               ▼               ▼
┌──────────────┐ ┌──────────────┐ ┌──────────────┐
│  传感器任务  │ │ 数据处理任务 │ │  显示任务    │
│ (Priority 3) │ │ (Priority 3) │ │ (Priority 4) │
└──────┬───────┘ └───────┬──────┘ └───────┬──────┘
       │                 │                  │
       │ 定时器回调      │ 队列接收         │ 任务通知
       ▼                 ▼                  ▼
┌─────────────────────────────────────────────────────┐
│              软件定时器 (500ms, 2000ms)           │
└─────────────────────────────────────────────────────┘
```

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
   - FreeRTOS综合应用示例标题
   - 学习要点列表
   - 系统组件创建过程

2. **系统运行状态**：
   - 各任务启动确认
   - 软件定时器启动
   - 调度器启动

3. **传感器数据采集**：
   - 每500ms采集一次传感器数据
   - 数据通过队列传递给处理任务

4. **数据处理**：
   - 每5个样本计算一次平均值
   - 处理完成后通知显示任务

5. **显示更新**：
   - 显示处理后的传感器数据
   - 包括温度、湿度、气压
   - 采样计数统计

6. **命令控制**：
   - 每3秒切换一次系统状态
   - 演示各任务的启用/禁用
   - 显示当前系统状态

7. **后台任务**：
   - 低优先级工作任务在后台运行
   - 展示多任务并发执行

## 事件组位定义

| 位标志 | 宏定义 | 用途 |
|--------|--------|------|
| Bit 0 | `EVENT_BIT_SENSOR_ENABLE` | 传感器采集任务启用 |
| Bit 1 | `EVENT_BIT_DATA_PROC_ENABLE` | 数据处理任务启用 |
| Bit 2 | `EVENT_BIT_DISPLAY_ENABLE` | 显示任务启用 |
| Bit 3 | `EVENT_BIT_SYSTEM_RUNNING` | 系统运行状态 |

## 关键API总结

| API函数 | 用途 |
|---------|------|
| `xTimerCreate()` | 创建软件定时器 |
| `xTimerStart()` | 启动定时器 |
| `xQueueCreate()` | 创建队列 |
| `xQueueSend()` | 发送数据到队列 |
| `xQueueReceive()` | 从队列接收数据 |
| `xEventGroupCreate()` | 创建事件组 |
| `xEventGroupSetBits()` | 设置事件组位 |
| `xEventGroupClearBits()` | 清除事件组位 |
| `xEventGroupGetBits()` | 获取事件组状态 |
| `xTaskNotifyGive()` | 发送任务通知 |
| `ulTaskNotifyTake()` | 等待任务通知 |

## 项目结构

```
09-comprehensive-demo/
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

- [FreeRTOS官方文档 - 软件定时器](https://www.freertos.org/RTOS-software-timer.html)
- [FreeRTOS官方文档 - 队列](https://www.freertos.org/Embedded-RTOS-Queues.html)
- [FreeRTOS官方文档 - 事件组](https://www.freertos.org/FreeRTOS-Event-Groups.html)
- [FreeRTOS官方文档 - 任务通知](https://www.freertos.org/RTOS-task-notifications.html)
- [Mastering the FreeRTOS Kernel](https://www.freertos.org/Documentation/161204_Mastering_the_FreeRTOS_Real_Time_Kernel-A_Hands-On_Tutorial_Guide.pdf)

## 注意事项

1. **软件定时器回调**：
   - 回调函数在定时器服务任务中执行
   - 回调函数不能阻塞
   - 回调函数应尽可能简短

2. **队列使用**：
   - 注意队列项大小的定义
   - 考虑队列长度与数据产生速率的匹配
   - 发送和接收时的超时设置

3. **任务通知**：
   - 比二值信号量更轻量级
   - 每个任务只有一个通知值
   - 适合一对一的任务同步

4. **事件组**：
   - 适合管理多个状态标志
   - 可同时等待多个事件
   - 注意位标志的定义与使用

5. **任务优先级**：
   - 根据任务重要性合理分配优先级
   - 避免优先级反转问题
   - 考虑任务间的依赖关系
