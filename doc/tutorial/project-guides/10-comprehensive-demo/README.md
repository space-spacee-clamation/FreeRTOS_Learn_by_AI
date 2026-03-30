# FreeRTOS 综合应用学习指南

## 章节概述

本章节是FreeRTOS核心内核功能的综合应用，通过构建一个完整的传感器数据采集与处理系统，整合之前学习的所有知识点。

## 学习目标

完成本章节后，你应该能够：

1. ✅ 设计完整的多任务系统架构
2. ✅ 合理选择和组合使用FreeRTOS的各种通信机制
3. ✅ 使用软件定时器实现周期性任务
4. ✅ 使用队列进行任务间数据传递
5. ✅ 使用任务通知实现轻量级同步
6. ✅ 使用事件组管理系统状态
7. ✅ 协调多个任务的执行顺序和优先级

## 项目结构

```
10-comprehensive-demo/
└── README.md                    # 本学习指南

对应项目代码位置：
projects/stm32-learning/02-core-kernel/09-comprehensive-demo/
```

## 系统设计

### 应用场景

我们设计一个环境监测系统，具有以下功能：

- 定期采集温度、湿度、气压数据
- 对采集的数据进行处理和统计
- 实时显示处理后的结果
- 可通过命令控制系统各模块状态

### 任务划分

| 任务 | 优先级 | 功能 | 通信机制 |
|------|--------|------|----------|
| 命令处理 | 5 | 控制系统状态，切换任务模式 | 事件组 |
| 显示任务 | 4 | 显示处理后的传感器数据 | 任务通知 |
| 数据处理 | 3 | 处理原始传感器数据，计算统计值 | 队列接收 |
| 传感器任务 | 3 | 监控传感器状态 | 软件定时器 |
| 工作任务 | 2 | 后台空闲任务 | - |

### 数据流向

```
软件定时器(500ms) → 传感器数据 → 队列 → 数据处理任务
                                                       ↓
任务通知 ← 计算完成 ← 平均值计算 ← 数据累加
    ↓
显示任务 → 串口输出
```

## 关键代码解析

### 1. 数据结构定义

```c
/* 传感器数据结构体 */
typedef struct {
    uint32_t ulTimestamp;      /* 时间戳 */
    int32_t  lTemperature;      /* 温度（模拟值） */
    int32_t  lHumidity;         /* 湿度（模拟值） */
    int32_t  lPressure;         /* 气压（模拟值） */
} SensorData_t;

/* 处理后的数据结构体 */
typedef struct {
    uint32_t ulTimestamp;
    int32_t  lAvgTemp;          /* 平均温度 */
    int32_t  lAvgHumidity;      /* 平均湿度 */
    int32_t  lAvgPressure;      /* 平均气压 */
    uint32_t ulSampleCount;     /* 采样计数 */
} ProcessedData_t;
```

**学习要点：**
- 使用结构体封装相关数据
- 区分原始数据和处理后数据
- 合理选择数据类型

### 2. 事件组位定义

```c
#define EVENT_BIT_SENSOR_ENABLE         (1 << 0)
#define EVENT_BIT_DATA_PROC_ENABLE      (1 << 1)
#define EVENT_BIT_DISPLAY_ENABLE        (1 << 2)
#define EVENT_BIT_SYSTEM_RUNNING        (1 << 3)
#define EVENT_BIT_ALL_TASKS_READY       (EVENT_BIT_SENSOR_ENABLE | \
                                           EVENT_BIT_DATA_PROC_ENABLE | \
                                           EVENT_BIT_DISPLAY_ENABLE)
```

**学习要点：**
- 使用位掩码定义事件标志
- 组合位标志表示复合状态
- 清晰的命名规范

### 3. 软件定时器回调

```c
static void vSensorTimerCallback(TimerHandle_t xTimer)
{
    (void)xTimer;
    SensorData_t xSensorData;

    /* 检查传感器是否启用 */
    if((xEventGroupGetBits(xSystemEventGroup) & EVENT_BIT_SENSOR_ENABLE) == 0)
    {
        return;
    }

    /* 模拟传感器读数 */
    vSimulateSensorReading(&xSensorData);

    /* 发送到队列 */
    xQueueSend(xSensorDataQueue, &xSensorData, 0);
}
```

**学习要点：**
- 定时器回调函数的结构
- 在回调中检查系统状态
- 通过队列传递数据
- 回调函数应简短不阻塞

### 4. 数据处理任务

```c
static void vDataProcessingTask(void *pvParameters)
{
    SensorData_t xReceivedData;
    static int32_t lTempSum = 0;
    static int32_t lHumiditySum = 0;
    static int32_t lPressureSum = 0;
    static uint32_t ulSampleCount = 0;

    for(;;)
    {
        /* 等待传感器数据 */
        if(xQueueReceive(xSensorDataQueue, &xReceivedData, portMAX_DELAY) == pdPASS)
        {
            /* 检查数据处理是否启用 */
            if((xEventGroupGetBits(xSystemEventGroup) & EVENT_BIT_DATA_PROC_ENABLE) == 0)
            {
                continue;
            }

            /* 累加数据 */
            lTempSum += xReceivedData.lTemperature;
            lHumiditySum += xReceivedData.lHumidity;
            lPressureSum += xReceivedData.lPressure;
            ulSampleCount++;

            /* 每5个样本计算一次平均值 */
            if(ulSampleCount >= 5)
            {
                /* 计算平均值 */
                xLatestProcessedData.lAvgTemp = lTempSum / ulSampleCount;
                /* ... */

                /* 重置统计 */
                lTempSum = 0;
                ulSampleCount = 0;

                /* 通知显示任务有新数据 */
                xTaskNotifyGive(xDisplayTaskHandle);
            }
        }
    }
}
```

**学习要点：**
- 使用静态变量保持状态
- 队列接收数据
- 事件组检查任务状态
- 数据处理算法
- 任务通知触发后续处理

### 5. 命令处理任务

```c
static void vCommandHandlerTask(void *pvParameters)
{
    uint32_t ulCommandCount = 0;
    EventBits_t uxBits;

    for(;;)
    {
        ulCommandCount++;

        /* 根据命令周期切换任务状态 */
        switch(ulCommandCount % 4)
        {
            case 0:
                /* 全部启用 */
                xEventGroupSetBits(xSystemEventGroup, EVENT_BIT_ALL_TASKS_READY);
                break;
            case 1:
                /* 暂停显示 */
                xEventGroupClearBits(xSystemEventGroup, EVENT_BIT_DISPLAY_ENABLE);
                break;
            /* ... */
        }

        /* 等待一段时间再处理下一个命令 */
        vTaskDelay(pdMS_TO_TICKS(DELAY_LONG_MS * 3));
    }
}
```

**学习要点：**
- 使用事件组控制系统状态
- 动态切换任务启用/禁用
- 状态机设计模式
- 周期性命令处理

## 编译与运行

### 编译项目

```bash
cd projects/stm32-learning/02-core-kernel/09-comprehensive-demo
make all-lm3s6965
```

### 运行仿真

```bash
make run-lm3s6965
```

按 `Ctrl+A` 然后 `X` 退出QEMU。

## 预期输出分析

运行程序后，你会观察到以下阶段：

### 1. 系统初始化阶段

```
========================================
  FreeRTOS Comprehensive Demo
========================================

Learning objectives:
1. Sensor data collection (software timers)
2. Data processing (queue communication)
3. Display updates (task notifications)
4. Command handling (event groups)
5. Multi-task coordination

Creating queues...
  - Sensor data queue OK

Creating event groups...
  - System event group OK

Creating software timers...
  - Sensor timer OK (500ms)
  - Display timer OK (2000ms)

Creating tasks...
  - Sensor task (prio 3)
  - Data processing task (prio 3)
  - Display task (prio 4)
  - Command handler task (prio 5)
  - Worker task (prio 2)

All tasks created!

Starting timers...
  - Sensor timer started
  - Display timer started

Starting scheduler...
```

### 2. 任务启动阶段

```
  [Sensor] Task started
  [DataProc] Task started
  [Display] Task started
  [CmdHandler] Task started
  [Worker] Background task started
```

### 3. 数据采集与显示阶段

```
  [CmdHandler] Processing command cycle #1
  [CmdHandler] Enabling all tasks
  [CmdHandler] Current state: SENSOR DATAPROC DISPLAY

========================================
  [Display] Sensor Data Update
========================================
  Timestamp: 250
  Temperature: 25.0 C
  Humidity: 60.0 %
  Pressure: 1013 hPa
  Samples: 5
========================================
```

### 4. 状态切换阶段

```
  [CmdHandler] Processing command cycle #2
  [CmdHandler] Pausing display
  [CmdHandler] Current state: SENSOR DATAPROC

  [Worker] Running... loop #20

  [CmdHandler] Processing command cycle #3
  [CmdHandler] Resuming display, pausing data processing
  [CmdHandler] Current state: SENSOR DISPLAY
```

## 练习与思考

### 基础练习

1. **修改采样周期**
   - 将传感器采样周期从500ms改为200ms
   - 观察数据更新频率的变化
   - 思考：采样周期对系统负载的影响

2. **调整统计窗口**
   - 将平均值计算从5个样本改为10个样本
   - 观察输出变化
   - 思考：统计窗口大小对数据平滑的影响

3. **添加新的传感器**
   - 在SensorData_t中添加光照强度字段
   - 修改模拟数据生成函数
   - 更新显示输出

### 进阶练习

1. **实现数据缓存**
   - 添加一个队列存储历史数据
   - 实现查询历史数据的功能
   - 通过命令控制查询

2. **添加报警功能**
   - 设置温度阈值
   - 当温度超过阈值时触发报警
   - 使用不同的通信机制通知

3. **优化任务设计**
   - 分析当前任务优先级是否合理
   - 尝试调整优先级观察系统行为
   - 思考：优先级设计的原则

### 挑战练习

1. **实现低功耗模式**
   - 在系统空闲时进入低功耗
   - 使用tickless idle功能
   - 测量功耗降低效果

2. **添加存储功能**
   - 模拟Flash存储
   - 定期保存数据到"Flash"
   - 实现数据持久化

3. **设计更复杂的命令系统**
   - 定义命令协议
   - 实现命令解析器
   - 支持更丰富的控制功能

## 常见问题

### Q1: 为什么我的数据显示不更新？

可能原因：
- 任务通知没有正确发送
- 显示任务被阻塞
- 事件组标志被清除

排查方法：
- 检查xTaskNotifyGive()是否被调用
- 检查事件组DISPLAY_ENABLE位状态
- 在关键位置添加调试输出

### Q2: 队列满了怎么办？

原因分析：
- 生产者速度 > 消费者速度
- 队列长度设置不足

解决方案：
- 增加队列长度
- 优化消费者任务处理速度
- 考虑使用流缓冲区

### Q3: 如何调试多任务系统？

调试技巧：
- 在关键位置添加串口输出
- 使用RTOS感知的调试器
- 检查任务状态和堆栈使用
- 验证通信机制是否正常工作

## 总结

通过本综合项目，我们学习了：

1. ✅ **系统架构设计**：如何将一个应用分解为多个任务
2. ✅ **通信机制选择**：根据需求选择队列、通知、事件组
3. ✅ **任务协调**：使用事件组管理系统状态
4. ✅ **数据处理流程**：从采集到处理再到显示的完整流程
5. ✅ **优先级管理**：合理配置任务优先级
6. ✅ **资源共享**：通过合适的机制传递和共享数据

这标志着FreeRTOS核心内核学习阶段的完成！接下来可以进入扩展功能和实际应用阶段。

## 参考资料

- [项目代码](../../../../projects/stm32-learning/02-core-kernel/09-comprehensive-demo/)
- [Mastering the FreeRTOS Kernel](https://www.freertos.org/Documentation/161204_Mastering_the_FreeRTOS_Real_Time_Kernel-A_Hands-On_Tutorial_Guide.pdf)
- [FreeRTOS官方文档](https://www.freertos.org/)
