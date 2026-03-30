# 第一个FreeRTOS任务示例 - 教学文档索引

## 📚 项目概述

这是STM32-FreeRTOS渐进式学习的第二个项目，通过这个项目你将学会FreeRTOS任务的基本概念和使用方法。

## 🎯 学习目标

通过学习这个项目，你将能够：

1. ✅ 理解什么是FreeRTOS任务
2. ✅ 掌握 `xTaskCreate()` API的使用
3. ✅ 学会编写符合规范的任务函数
4. ✅ 正确使用 `vTaskDelay()` 进行任务延时
5. ✅ 理解如何启动FreeRTOS调度器
6. ✅ 排查常见的任务创建和运行问题

## 📖 文档导航

按照以下顺序阅读文档，效果最佳：

### 1. 概念入门
- **[01_任务概念介绍.md](01_任务概念介绍.md)** - 从零开始理解什么是任务
  - 任务与裸机编程的区别
  - 任务的工作原理
  - 为什么需要多任务

### 2. API详解
- **[02_xTaskCreate_API详解.md](02_xTaskCreate_API详解.md)** - 深入理解任务创建函数
  - 函数原型和每个参数的详细说明
  - 返回值分析
  - 使用示例
  - 常见错误

### 3. 实践指南
- **[03_任务函数编写指南.md](03_任务函数编写指南.md)** - 学会写任务函数
  - 任务函数的规范
  - 无限循环的重要性
  - 参数传递方法
  - 最佳实践

### 4. 辅助功能
- **[04_vTaskDelay使用说明.md](04_vTaskDelay使用说明.md)** - 任务延时的正确用法
  - 为什么不能用空循环延时
  - `vTaskDelay()` 的工作原理
  - 时间单位转换

### 5. 关键步骤
- **[05_调度器启动指南.md](05_调度器启动指南.md)** - 启动FreeRTOS的关键一步
  - `vTaskStartScheduler()` 做了什么
  - 启动前的准备工作
  - 启动失败的原因

### 6. 问题排查
- **[06_常见问题FAQ.md](06_常见问题FAQ.md)** - 遇到问题看这里
  - 最常见的8个问题及解决方案
  - 调试技巧
  - 如何避免常见错误

## 🚀 快速开始

### 编译项目

```bash
cd projects/stm32-learning/01-getting-started/01-first-task

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
  第一个FreeRTOS任务示例
====================================

学习要点：
1. 使用 xTaskCreate() 创建任务
2. 任务函数的编写方法
3. vTaskDelay() 任务延时
4. 启动调度器 vTaskStartScheduler()

正在创建第一个任务...
任务创建成功！
启动FreeRTOS调度器...

FirstTask: 任务开始运行！
FirstTask: 计数器 = 0
FirstTask: 计数器 = 1
FirstTask: 计数器 = 2
...
```

## 📂 项目结构

```
01-first-task/
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

## 🔗 相关链接

- **项目代码**: [../../../../projects/stm32-learning/01-getting-started/01-first-task/](../../../../projects/stm32-learning/01-getting-started/01-first-task/)
- **上一项目**: [00-env-verify](../00-env-verify/) - 环境验证
- **FreeRTOS官方文档**: [任务创建API](https://www.freertos.org/a00125.html)
- **Mastering-the-FreeRTOS-Kernel**: [第4章 任务管理](../../Mastering-the-FreeRTOS-Kernel/04_任务管理.md)

## 💡 学习建议

1. **按顺序阅读**：文档是按渐进式设计的，建议按顺序阅读
2. **边读边试**：阅读API文档时，可以修改代码试试看
3. **动手实验**：尝试修改任务优先级、栈大小、延时时间，观察变化
4. **遇到问题**：先看[06_常见问题FAQ.md](06_常见问题FAQ.md)，还是解决不了再搜索

祝你学习愉快！🎉

---

*参考资料：FreeRTOS官方文档 v10.x*
