# FreeRTOS任务调度示例

## 项目概述

这是STM32-FreeRTOS渐进式学习的第三个项目，演示FreeRTOS的多任务调度和优先级管理。

## 学习目标

通过学习这个项目，你将能够：

1. 理解什么是任务优先级
2. 掌握抢占式调度的工作原理
3. 观察不同优先级任务的运行顺序
4. 理解时间片轮转调度

## 项目结构

```
02-task-schedule/
├── Makefile                    # 编译脚本
├── LM3S6965.ld                 # LM3S6965链接脚本
├── STM32F103RB.ld              # STM32F103链接脚本
├── startup_lm3s6965.c          # LM3S6965启动文件
├── startup_stm32f103.c         # STM32F103启动文件
├── include/
│   ├── FreeRTOSConfig.h         # LM3S6965 FreeRTOS配置
│   ├── FreeRTOSConfig_LM3S6965.h
│   └── FreeRTOSConfig_STM32F103.h
└── src/
    └── main.c                 # 主程序（阅读学习重点）
```

## 学习要点

### 1. 任务优先级

- 优先级数值越大，优先级越高
- 高优先级任务可以抢占低优先级任务
- 空闲任务优先级为0

### 2. 本项目的三个任务

| 任务名称 | 优先级 | 运行周期 | 说明 |
|---------|--------|---------|------|
| HighTask | 3 | 200ms | 高优先级，运行频率最高 |
| MediumTask | 2 | 300ms | 中优先级 |
| LowTask | 1 | 500ms | 低优先级，只有在高优先级任务阻塞时运行 |

### 3. 调度器工作原理

- **抢占式调度**：高优先级任务就绪时立即抢占CPU
- **时间片轮转**：相同优先级任务之间平分CPU时间
- **阻塞状态**：任务调用vTaskDelay()时进入阻塞状态

## 快速开始

### 编译项目

```bash
cd projects/stm32-learning/01-getting-started/02-task-schedule

# 编译LM3S6965版本
make all-lm3s6965

# 或者编译STM32F103版本
make all-stm32f103
```

### 运行项目

```bash
# 运行LM3S6965（推荐初学者）
make run-lm3s6965

# 运行STM32F103
make run-stm32f103
```

**退出QEMU**：按 `Ctrl+A` 然后按 `X`

### 预期输出

```
====================================
  FreeRTOS任务调度示例
====================================

学习要点：
1. 创建多个不同优先级的任务
2. 观察抢占式调度效果
3. 理解任务优先级的作用

任务配置：
- HighTask:   优先级 3, 周期 200ms
- MediumTask: 优先级 2, 周期 300ms
- LowTask:    优先级 1, 周期 500ms

正在创建任务...
所有任务创建成功！
启动FreeRTOS调度器...

[High] 高优先级任务启动！
[High] 运行 #0
[Medium] 中优先级任务启动！
[Medium] 运行 #0
[Low] 低优先级任务启动！
[Low] 运行 #0
[High] 运行 #1
[Medium] 运行 #1
[High] 运行 #2
[Low] 运行 #1
...
```

## 观察重点

运行时注意观察：

1. **启动顺序**：高优先级任务最先启动
2. **运行频率**：HighTask运行最频繁
3. **阻塞行为**：每个任务运行后都会阻塞指定时间
4. **调度效果**：低优先级任务只有在高优先级任务都阻塞时才能运行

## 思考问题

1. 如果把LowTask的优先级改为3，会发生什么？
2. 如果HighTask不调用vTaskDelay()，其他任务能运行吗？
3. 如何验证任务确实在按预期调度？

## 相关链接

- **上一项目**: [01-first-task](../01-first-task/) - 第一个任务
- **下一项目**: [03-queue-comm](../03-queue-comm/) - 队列通信
- **FreeRTOS官方文档**: [任务优先级](https://www.freertos.org/RTOS-task-priority.html)
- **Mastering-the-FreeRTOS-Kernel**: [第4章 任务管理](../../../../doc/tutorial/Mastering-the-FreeRTOS-Kernel/04_任务管理.md)
- **教学文档**: [../../../../doc/tutorial/project-guides/02-task-schedule/](../../../../doc/tutorial/project-guides/02-task-schedule/)

## 学习建议

1. **先运行观察**：编译运行项目，观察输出顺序
2. **修改参数**：尝试修改优先级和延时时间，观察变化
3. **阅读代码**：重点看main.c中的任务创建和任务函数
4. **思考原理**：理解为什么输出是这样的顺序
