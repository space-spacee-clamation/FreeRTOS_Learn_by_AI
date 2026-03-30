# 04-stream-buffer - FreeRTOS流缓冲区示例

## 📚 项目概述

本项目演示FreeRTOS中流缓冲区（Stream Buffer）的使用方法，展示其与队列的区别、水线（Trigger Level）机制，以及生产者-消费者模型的实现。

**学习要点：**
- 流缓冲区的概念和工作原理
- 流缓冲区与队列的区别
- 水线（Trigger Level）机制
- xStreamBufferCreate() / xStreamBufferSend() / xStreamBufferReceive() API
- 生产者-消费者模型
- 中断与任务间通信
- 流缓冲区状态查询

## 🎯 学习目标

完成本项目后，你将能够：
1. 理解流缓冲区的工作原理
2. 掌握流缓冲区与队列的区别
3. 理解水线（Trigger Level）机制
4. 使用流缓冲区实现生产者-消费者模型
5. 在任务和中断中使用流缓冲区
6. 查询流缓冲区状态信息

## 📖 文档导航

完整的教学文档将位于 `doc/tutorial/project-guides/` 目录（待创建）：

### 1. 概念入门
- 流缓冲区概念介绍
- 流缓冲区与队列对比
- 水线机制详解

### 2. API详解
- xStreamBufferCreate() API详解
- xStreamBufferSend() / xStreamBufferReceive() API详解
- 流缓冲区状态查询API

### 3. 实践指南
- 流缓冲区使用实践指南
- 生产者-消费者模型实现
- 常见问题FAQ

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

程序启动后，你将看到流缓冲区的完整演示：
1. 简单测试 - 基本的发送和接收操作
2. 水线测试 - 演示水线机制的工作原理
3. 生产者-消费者 - 多任务间流式数据传输
4. 状态查询 - 查看流缓冲区的各种状态信息
5. 流缓冲区与队列的对比表

## 📂 项目结构

```
04-stream-buffer/
├── Makefile              # 构建脚本（已添加stream_buffer.c支持）
├── LM3S6965.ld          # LM3S6965链接脚本
├── STM32F103RB.ld       # STM32F103链接脚本
├── README.md             # 本文件
├── startup_lm3s6965.c   # LM3S6965启动文件
├── startup_stm32f103.c  # STM32F103启动文件
├── include/
│   ├── FreeRTOSConfig.h          # FreeRTOS配置
│   ├── FreeRTOSConfig_LM3S6965.h # LM3S6965专用配置
│   └── FreeRTOSConfig_STM32F103.h # STM32F103专用配置
└── src/
    └── main.c           # 主程序（流缓冲区演示）
```

## 🔗 相关链接

- **项目文档**: doc/tutorial/project-guides/（待创建）
- **上一项目**: [03-memory-management](../03-memory-management/) - 堆内存管理专题
- **下一项目**: [05-software-timer](../05-software-timer/) - 软件定时器专题
- **FreeRTOS官方文档**: [流缓冲区](https://www.freertos.org/RTOS-stream-buffers.html)
- **Mastering-the-FreeRTOS-Kernel**: 相关章节（待补充）

## 💡 学习建议

1. **先理解概念**：理解流缓冲区与队列的区别
2. **理解水线机制**：掌握Trigger Level的工作原理
3. **运行示例**：观察生产者-消费者模型的实际运行
4. **对比分析**：理解流缓冲区适用的场景
5. **做实验**：
   - 尝试不同的水线值
   - 测试不同的数据传输模式
   - 对比流缓冲区和队列的性能差异

## 流缓冲区的适用场景

流缓冲区适合以下场景：

✓ **流式数据传输** - 字节流而非固定大小数据项
✓ **生产者-消费者** - 一个生产者一个消费者的场景
✓ **中断到任务** - 中断服务程序向任务传输数据
✓ **可变长度数据** - 数据长度不固定的情况
✓ **字节级操作** - 需要逐字节处理数据

流缓冲区不适合：

✗ **多对多通信** - 多个生产者或多个消费者
✗ **固定大小数据项** - 队列更适合这种场景
✗ **需要数据拷贝** - 流缓冲区直接操作字节流
✗ **复杂同步** - 需要额外的同步机制

## 流缓冲区与队列关键区别

| 特性 | 队列 | 流缓冲区 |
|------|------|----------|
| 数据单位 | 固定大小数据项 | 字节流 |
| 生产者数量 | 支持多个 | 通常一个 |
| 消费者数量 | 支持多个 | 通常一个 |
| 水线机制 | 不支持 | 支持 |
| 数据拷贝 | 完整拷贝 | 流式操作 |
| 适用场景 | 多任务同步 | 流式数据传输 |
| 内存使用 | 按数据项分配 | 连续缓冲区 |
