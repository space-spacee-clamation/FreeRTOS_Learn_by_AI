# FreeRTOS 中断管理（Interrupt Management）学习项目

## 项目概述

本项目演示FreeRTOS中断管理的使用方法。中断是嵌入式系统中响应外部事件的关键机制，FreeRTOS提供了完整的中断管理支持，包括ISR安全API、中断优先级配置和延迟中断处理等。

## 学习要点

1. **中断优先级配置**
   - `configMAX_SYSCALL_INTERRUPT_PRIORITY` - 可调用FreeRTOS API的最高中断优先级
   - `configKERNEL_INTERRUPT_PRIORITY` - 内核使用的中断优先级
   - Cortex-M3的中断优先级：数值越低优先级越高

2. **ISR安全API的使用**
   - 必须使用`FromISR`后缀的API函数
   - `xSemaphoreGiveFromISR()` - 而不是`xSemaphoreGive()`
   - `xQueueSendFromISR()` - 而不是`xQueueSend()`

3. **二值信号量 - ISR到任务同步**
   - ISR给出信号量唤醒任务
   - 任务等待信号量执行延迟处理
   - 实现简单的中断-任务同步

4. **队列 - ISR到任务数据传输**
   - ISR向队列发送数据
   - 任务从队列接收数据
   - 支持传递任意类型的数据

5. **延迟中断处理（Deferred Interrupt Processing）**
   - ISR只做必要的快速工作
   - 复杂处理延迟到任务中完成
   - 缩短中断关闭时间

6. **任务模拟ISR机制**
   - 使用高优先级任务模拟硬件中断
   - 便于在QEMU仿真环境中学习
   - 演示FromISR API的正确用法

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
   - FreeRTOS中断管理学习示例标题
   - 学习要点列表
   - 中断管理关键概念介绍

2. **ISR模拟演示**：
   - ISR模拟器任务每1000ms模拟一次中断
   - ISR给出二值信号量并发送数据到队列
   - 延迟处理任务被唤醒并处理中断
   - 演示5次中断后完成

3. **后台任务**：
   - 低优先级工作任务在后台运行
   - 展示中断处理不阻塞其他任务

## 中断管理关键概念

### 延迟中断处理模式

```
硬件中断 → ISR（快速处理）→ 信号量/队列 → 任务（复杂处理）
         ↑                              ↑
      必须快                        可以慢
      关中断                       开中断
```

### ISR中可以做什么

| 操作 | 推荐 | 说明 |
|------|------|------|
| 给出信号量 | ✓ | 快速，不阻塞 |
| 发送到队列 | ✓ | 快速，不阻塞 |
| 读取硬件寄存器 | ✓ | 必须做的 |
| 复杂计算 | ✗ | 延迟到任务 |
| 字符串操作 | ✗ | 延迟到任务 |
| 调用printf | ✗ | 延迟到任务 |

### FromISR API对照表

| 普通API | ISR安全API |
|---------|-----------|
| `xSemaphoreGive()` | `xSemaphoreGiveFromISR()` |
| `xSemaphoreTake()` | 不推荐在ISR中使用 |
| `xQueueSend()` | `xQueueSendFromISR()` |
| `xQueueReceive()` | 不推荐在ISR中使用 |
| `xTaskNotifyGive()` | `xTaskNotifyGiveFromISR()` |
| - | `portYIELD_FROM_ISR()` |

## 项目结构

```
07-interrupt-management/
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

- [FreeRTOS官方文档 - Cortex-M3/M4中断](https://www.freertos.org/RTOS-Cortex-M3-M4.html)
- [FreeRTOS官方文档 - ISR FAQ](https://www.freertos.org/FAQISR.html)
- [FreeRTOS Demo项目 - IntSemTest.c](https://github.com/FreeRTOS/FreeRTOS/blob/main/FreeRTOS/Demo/Common/Minimal/IntSemTest.c)
- [Mastering the FreeRTOS Kernel - 第7章 中断管理](../../../../doc/tutorial/Mastering-the-FreeRTOS-Kernel/07_中断管理.md)

## 关键API总结

| API函数 | 用途 |
|---------|------|
| `xSemaphoreCreateBinary()` | 创建二值信号量 |
| `xSemaphoreGiveFromISR()` | ISR中给出信号量 |
| `xSemaphoreTake()` | 任务中获取信号量 |
| `xQueueCreate()` | 创建队列 |
| `xQueueSendFromISR()` | ISR中发送到队列 |
| `xQueueReceive()` | 任务中从队列接收 |
| `portYIELD_FROM_ISR()` | ISR中请求上下文切换 |

## 中断优先级配置说明

### Cortex-M3中断优先级

- **优先级数值越小，优先级越高**
- 0 = 最高优先级
- 255 = 最低优先级

### FreeRTOS配置参数

```c
// 可调用FreeRTOS API的最高中断优先级
#define configMAX_SYSCALL_INTERRUPT_PRIORITY  0x40

// 内核使用的中断优先级（SysTick, PendSV）
#define configKERNEL_INTERRUPT_PRIORITY         255
```

### 中断优先级分组

| 优先级范围 | 可调用API | 说明 |
|-----------|----------|------|
| 0 - 0x3F | ✗ 不能 | 最高优先级，不被FreeRTOS管理 |
| 0x40 - 254 | ✓ 可以 | 可以使用FromISR API |
| 255 | ✓ 可以 | 内核使用的优先级 |

## 注意事项

1. **ISR要尽可能短**：
   - 只做必要的硬件操作
   - 复杂处理延迟到任务
   - 减少中断关闭时间

2. **必须使用FromISR版本API**：
   - 在ISR中绝对不能调用普通API
   - FromISR版本有特殊的中断安全保护
   - 编译器不会检查，使用错误会导致系统崩溃

3. **portYIELD_FROM_ISR()的使用**：
   - 如果`xHigherPriorityTaskWoken`被设为`pdTRUE`
   - 表示有更高优先级任务被唤醒
   - 调用`portYIELD_FROM_ISR()`请求上下文切换

4. **任务模拟ISR**：
   - 本项目用任务模拟ISR便于学习
   - 真实硬件中要写真正的ISR函数
   - FromISR API用法完全相同

5. **中断优先级配置错误的后果**：
   - 优先级过高可能导致系统崩溃
   - 优先级过低可能导致中断延迟
   - 仔细阅读官方文档关于中断配置的说明
