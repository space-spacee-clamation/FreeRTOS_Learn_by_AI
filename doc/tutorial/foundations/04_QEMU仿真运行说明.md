# QEMU仿真运行说明

## 📚 概述

QEMU是一个开源的机器仿真器和虚拟机。在本项目中，我们使用QEMU来仿真STM32和LM3S6965开发板，这样你不需要真实硬件就可以学习FreeRTOS。

## 🎯 学习目标

完成本指南后，你将能够：
1. 使用QEMU运行编译好的FreeRTOS程序
2. 理解QEMU的基本命令行参数
3. 调试运行中的程序
4. 解决常见的QEMU问题

## 🚀 快速开始

### 最简单的运行方式

在项目目录中直接运行：

```bash
cd projects/stm32-learning/01-getting-started/04-semaphore
make run-lm3s6965
```

### 退出QEMU

按 `Ctrl+A` 然后按 `X` 退出QEMU。

## 🔧 手动运行QEMU

让我们看看Makefile背后实际执行的命令。

### LM3S6965仿真命令

```bash
qemu-system-arm \
    -machine lm3s6965evb \
    -kernel semaphore-lm3s6965.elf \
    -nographic \
    -serial mon:stdio
```

### 参数详解

| 参数 | 作用 |
|------|------|
| `qemu-system-arm` | QEMU ARM系统仿真器 |
| `-machine lm3s6965evb` | 指定仿真的机器类型 |
| `-kernel filename.elf` | 指定内核/程序文件 |
| `-nographic` | 不使用图形界面 |
| `-serial mon:stdio` | 串口重定向到标准输入输出 |

## 📋 QEMU支持的开发板

### 查看可用的机器类型

```bash
qemu-system-arm -machine help
```

### 我们项目使用的开发板

| 开发板 | 芯片 | 架构 | 推荐用途 |
|--------|------|------|---------|
| `lm3s6965evb` | LM3S6965 | Cortex-M3 | ✅ 推荐学习使用 |
| `stm32vldiscovery` | STM32F100 | Cortex-M3 | ⚠️ 有限支持 |
| `netduino2` | STM32F205 | Cortex-M3 | 可用 |

**注意**：LM3S6965在QEMU中支持最好，推荐使用。

## 🎮 交互式使用

### 查看QEMU监视器

QEMU有一个内置的监视器，可以用来查看和控制仿真。

在运行QEMU时：
- 按 `Ctrl+A` 然后按 `C` 切换到监视器
- 输入 `help` 查看可用命令
- 按 `Ctrl+A` 然后按 `X` 退出

### 常用监视器命令

```
help           - 显示帮助
info registers - 显示CPU寄存器
info mtree     - 显示内存映射
xp /10wx 0x0   - 查看内存（10个字，十六进制）
stop           - 停止CPU
c              - 继续执行
q              - 退出QEMU
```

## 🔍 调试功能

### GDB调试模式

QEMU可以作为GDB远程调试服务器使用。

#### 第一步：启动QEMU调试模式

```bash
# 使用Makefile
make debug-lm3s6965

# 或者手动运行
qemu-system-arm \
    -machine lm3s6965evb \
    -kernel semaphore-lm3s6965.elf \
    -nographic \
    -serial mon:stdio \
    -gdb tcp::1234 \
    -S
```

参数说明：
- `-gdb tcp::1234` - 在1234端口等待GDB连接
- `-S` - 启动后暂停，等待GDB连接

#### 第二步：在另一个终端启动GDB

```bash
cd projects/stm32-learning/01-getting-started/04-semaphore
arm-none-eabi-gdb semaphore-lm3s6965.elf
```

#### 第三步：在GDB中连接QEMU

```gdb
# 连接到QEMU
target remote localhost:1234

# 设置断点
break main

# 继续执行
continue

# 单步执行
step

# 查看变量
print g_ulSharedCounter

# 查看 backtrace
bt

# 退出GDB
quit
```

## 📊 查看输出

### 串口输出

程序中的 `vPrintString()` 和 `vPrintNumber()` 输出会直接显示在终端。

**示例输出**：
```
====================================
  FreeRTOS信号量使用示例
====================================

学习要点：
1. 二值信号量 - 任务同步
2. 互斥信号量 - 资源保护
3. xSemaphoreTake() / xSemaphoreGive()
4. 优先级继承（互斥信号量）

正在创建二值信号量...
二值信号量创建成功！
...
```

### 保存输出到文件

```bash
# 保存输出到文件
make run-lm3s6965 2>&1 | tee output.log

# 只保存到文件，不显示
make run-lm3s6965 > output.log 2>&1
```

## ❓ 常见问题排查

### 问题1：没有任何输出

**可能原因**：
- 程序崩溃
- 串口配置错误
- 程序在等待什么

**排查步骤**：
```bash
# 1. 检查ELF文件是否正确
file semaphore-lm3s6965.elf

# 2. 检查链接脚本
cat LM3S6965.ld | head -20

# 3. 尝试用GDB调试
make debug-lm3s6965
```

### 问题2：QEMU命令找不到

**症状**：
```bash
bash: qemu-system-arm: command not found
```

**解决方案**：
```bash
# Ubuntu/Debian
sudo apt-get install qemu-system-arm

# 检查是否安装成功
qemu-system-arm --version
```

### 问题3：程序立即退出

**症状**：QEMU启动后立即退出，没有任何输出。

**可能原因**：
- 链接脚本错误
- 程序在main之前崩溃
- 中断向量表配置错误

**排查方法**：
```bash
# 用GDB调试看看程序在哪
make debug-lm3s6965

# 在另一个终端
arm-none-eabi-gdb semaphore-lm3s6965.elf
(gdb) target remote localhost:1234
(gdb) stepi
```

### 问题4：Timer警告

**症状**：看到 "Timer with period zero, disabling" 警告。

**说明**：这是QEMU的正常警告，不影响程序运行，可以忽略。

## 📝 经验沉淀

所有QEMU相关的问题和解决方案都记录在：
[../../doc/experiences/qemu/README.md](../../experiences/qemu/README.md)

## 🆚 QEMU vs 真实硬件

| 特性 | QEMU仿真 | 真实硬件 |
|------|---------|---------|
| FreeRTOS调度 | ✅ 完全一致 | ✅ 完全一致 |
| 任务API | ✅ 完全一致 | ✅ 完全一致 |
| 队列/信号量 | ✅ 完全一致 | ✅ 完全一致 |
| 串口输出 | ✅ 支持 | ✅ 支持 |
| 精确时序 | ❌ 不保证 | ✅ 真实时序 |
| 外设访问 | ⚠️ 有限支持 | ✅ 完全支持 |

**结论**：学习FreeRTOS核心功能，QEMU完全够用！

## 🎯 推荐学习流程

1. **先用QEMU快速验证**
   - 编译运行，观察输出
   - 修改代码，快速迭代
   - 理解API使用

2. **再上硬件验证**
   - 学习外设操作
   - 验证真实性能
   - 测试低功耗等特性

## 📖 相关链接

- [编译流程分步指南](./03_编译流程分步指南.md)
- [Makefile构建原理](./02_Makefile构建原理.md)
- [QEMU经验沉淀](../../experiences/qemu/README.md)
- [STM32F103 QEMU支持分析](../../experiences/qemu/STM32F103_QEMU支持分析报告.md)
- [QEMU官方文档](https://www.qemu.org/docs/master/)