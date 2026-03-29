---
name: stm32-freertos-qemu-run
description: QEMU模拟运行STM32 FreeRTOS专用skill，提供渐进式引导，指导如何验证编译产物、使用QEMU仿真、查看输出和排查问题
---
# STM32 FreeRTOS QEMU仿真运行指南

## 何时使用
- 需要在QEMU中运行编译好的STM32 FreeRTOS项目时
- 需要调试运行中的FreeRTOS代码时
- 需要验证代码功能正确性但没有硬件开发板时
- 学习过程中快速验证代码修改效果时

## 何时不使用
- 已经有物理开发板并且想要测试实际硬件特性时
- 需要测试硬件相关外设操作（比如真实SPI/I2C设备交互）时
- 需要评估实际性能功耗比时

## 基本原则

本项目用户不熟悉虚拟机/仿真环境，需要遵循以下引导原则：

1. **渐进式引导**：每一步只做一件事，确认每一步成功后再进行下一步
2. **明确验证点**：每一步都告诉用户应该看到什么输出，如何判断成功/失败
3. **保持简单**：使用最简单的QEMU命令行参数，避免复杂配置
4. **问题沉淀**：遇到的问题都要记录到经验沉淀目录，方便后续快速排查

## 前置检查

在运行之前，需要验证以下条件：

1. **QEMU已安装**：
   ```bash
   qemu-system-arm --version
   ```
   如果输出版本信息，说明安装成功。

2. **交叉编译工具链已安装**：
   ```bash
   arm-none-eabi-gcc --version
   ```
   如果输出版本信息，说明安装成功。

3. **项目已编译**：确保已经执行过`make all`并且没有错误

## 第一步：验证编译产物

编译完成后，需要检查生成的文件：

```bash
ls -la build/
```

应该看到以下文件：
- `project-name.elf` - ELF格式可执行文件（包含调试信息）
- `project-name.bin` - 二进制镜像文件

验证ELF文件格式：
```bash
file build/*.elf
```

输出应该包含：`ARM executable, version 1 (SYSV)` 之类的信息。

## 第二步：使用QEMU仿真STM32

### 默认运行方式（STM32F4-Discovery）

本项目默认使用STM32F4-Discovery开发板仿真，在项目目录下直接运行：

```bash
make run
```

对应的QEMU命令：
```bash
qemu-system-arm -machine stm32f4-discovery -kernel build/your-project.elf -nographic -serial mon:stdio
```

参数说明：
- `-machine stm32f4-discovery`：指定仿真STM32F4发现板
- `-kernel`：指定内核镜像（ELF格式）
- `-nographic`：不使用图形界面
- `-serial mon:stdio`：串口输出重定向到标准输入输出

### 退出QEMU

退出方式：
1. 按 `Ctrl+A` 然后按 `X` 退出QEMU
2. 如果程序正常运行到结束，会自动退出

## 第三步：查看运行输出和调试

### 查看运行输出

程序中的串口输出会直接打印到当前终端，例如：
```
Starting FreeRTOS example...
Task 1 running
Task 2 running
...
```

如果没有任何输出，参考「常见问题」部分排查。

### GDB调试

1. 在项目目录启动QEMU调试模式：
   ```bash
   make debug
   ```
   这会启动QEMU并在端口1234等待GDB连接，程序暂停在入口点。

2. 在另一个终端启动GDB：
   ```bash
   arm-none-eabi-gdb build/your-project.elf
   ```

3. 在GDB中连接QEMU：
   ```gdb
   target remote localhost:1234
   ```

4. 现在可以正常使用GDB命令调试：
   ```gdb
   break main
   continue
   info registers
   ```

## 常见问题排查经验沉淀

所有QEMU相关问题排查经验都沉淀在：[../../../doc/experiences/qemu/README.md](../../../doc/experiences/qemu/README.md)

常见问题速查：

| 问题现象 | 可能原因 | 排查方法 |
|---------|---------|---------|
| QEMU命令找不到 | QEMU未安装 | 使用包管理器安装qemu-system-arm |
| 启动后立即退出 | 链接脚本错误，或入口点不对 | 检查linker.ld是否正确，检查中断向量表配置 |
| 编译找不到头文件 | FreeRTOS路径配置错误 | 检查Makefile中FREERTOS_ROOT路径 |
| 没有任何输出 | 串口配置错误 | 检查USART初始化代码，检查QEMU串口参数 |
| 程序崩溃 | 栈溢出或内存错误 | 使用GDB断点单步调试，检查堆配置 |

遇到新问题时，按照以下步骤处理：
1. 记录问题现象、环境信息、解决过程
2. 添加到 `doc/experiences/qemu/README.md` 中
3. 使用清晰的标题，方便后续搜索

## 与实际硬件开发的对应关系

QEMU仿真和实际硬件开发有以下对应关系：

| 特性 | QEMU仿真 | 实际硬件 | 学习说明 |
|-----|---------|---------|---------|
| FreeRTOS内核调度 | ✅ 完全一致 | ✅ 完全一致 | 学习核心API完全相同 |
| 中断处理 | ✅ 基本一致 | ✅ 一致 | 学习中断使用方法相同 |
| 外设寄存器 | ⚠️ 部分支持 | ✅ 完全支持 | 核心学习不受影响 |
| 时序特性 | ❌ 不完全一致 | ✅ 真实时序 | 学习逻辑和API使用足够 |
| 调试体验 | ✅ 非常方便 | ⚠️ 需要仿真器 | 适合学习阶段使用 |

## 学习建议

1. **初学阶段**：优先使用QEMU仿真，快速验证代码逻辑，不需要等待烧录
2. **验证概念**：FreeRTOS核心概念（任务、调度、同步、通信）在QEMU中学习完全足够
3. **外设学习**：学习完核心概念后，再到实际硬件上验证外设操作
4. **调试能力**：QEMU+GDB组合非常适合学习FreeRTOS内核调试技巧

## 参考资料

- [../../../doc/experiences/qemu/README.md](../../../doc/experiences/qemu/README.md) - QEMU问题经验沉淀
- [../../../doc/experiences/build-system/README.md](../../../doc/experiences/build-system/README.md) - 编译系统经验
- [../stm32-freertos-coding/SKILL.md](../stm32-freertos-coding/SKILL.md) - 代码编写规范

## 命令速查

| 操作 | 命令 |
|-----|-----|
| 编译项目 | `make all` |
| 清理编译 | `make clean` |
| 直接运行 | `make run` |
| 调试启动 | `make debug` |
| 退出QEMU | `Ctrl+A` 然后 `X` |
| GDB连接 | `target remote localhost:1234` |
