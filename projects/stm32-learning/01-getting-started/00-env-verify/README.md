# 环境验证测试项目 - 00-env-verify

## 项目介绍

这是STM32-FreeRTOS渐进式学习的第一个项目，**目的是验证开发环境是否能够正常工作**，包括：

1. ✅ 交叉编译工具链 `arm-none-eabi-gcc` 是否正确安装
2. ✅ FreeRTOS源码是否正确获取
3. ✅ 链接器是否能够正确链接生成可执行文件
4. ✅ QEMU是否能够正确仿真并运行程序

**支持的平台：**
- **LM3S6965** (QEMU: `lm3s6965evb`) - 原始支持平台
- **STM32F103** (QEMU: `olimex-stm32-p103`) - 新增支持平台 ⭐

这个项目不涉及复杂的FreeRTOS功能，只验证最基本的编译、链接和运行流程。

## 学习目标

- 确认开发环境已经正确搭建完成
- 理解STM32程序从编译到运行的完整流程
- 验证QEMU仿真可以正常工作
- 观察第一个FreeRTOS任务的运行
- 了解多平台编译配置方法

## 项目结构

```
00-env-verify/
├── Makefile                    # 编译脚本（支持多平台）
├── LM3S6965.ld                 # LM3S6965链接脚本
├── STM32F103RB.ld              # STM32F103链接脚本
├── startup_lm3s6965.c          # LM3S6965启动文件
├── startup_stm32f103rb.s       # STM32F103启动文件
├── include/
│   ├── FreeRTOSConfig.h         # 默认LM3S6965 FreeRTOS配置
│   ├── FreeRTOSConfig_LM3S6965.h  # LM3S6965 FreeRTOS配置
│   └── FreeRTOSConfig_STM32F103.h # STM32F103 FreeRTOS配置
└── src/
    ├── main_lm3s6965.c         # LM3S6965主程序
    └── main_stm32f103.c        # STM32F103主程序
```

## 编译和运行方法

### 前置要求

确保已经运行过项目根目录的 `init.sh` 脚本完成环境初始化：

```bash
cd /path/to/FreeRTOSDemo
./init.sh
```

### 查看可用目标

```bash
make help
```

### 编译 LM3S6965 版本

```bash
cd projects/stm32-learning/01-getting-started/00-env-verify
make all-lm3s6965
```

### 运行 LM3S6965（使用QEMU）

```bash
make run-lm3s6965
```

### 编译 STM32F103 版本 ⭐

```bash
make all-stm32f103
```

### 运行 STM32F103（使用QEMU）⭐

```bash
make run-stm32f103
```

### 清理编译产物

```bash
make clean
```

### GDB调试（可选）

**调试 LM3S6965:**

终端1：启动QEMU等待连接
```bash
make debug-lm3s6965
```

终端2：启动GDB连接
```bash
arm-none-eabi-gdb env-verify-lm3s6965.elf
(gdb) target remote :1234
(gdb) continue
```

**调试 STM32F103:**

终端1：启动QEMU等待连接
```bash
make debug-stm32f103
```

终端2：启动GDB连接
```bash
arm-none-eabi-gdb env-verify-stm32f103.elf
(gdb) target remote :1234
(gdb) continue
```

## 预期输出结果

### STM32F103 输出

如果环境配置正确，运行 STM32F103 版本应该看到类似下面的输出：

```
====================================
  STM32F103 FreeRTOS 环境验证测试
====================================

系统初始化完成，创建测试任务...
任务创建成功，启动FreeRTOS调度器...

Blink: 0 - FreeRTOS运行正常!
Blink: 1 - FreeRTOS运行正常!
Blink: 2 - FreeRTOS运行正常!
Blink: 3 - FreeRTOS运行正常!
...
```

### LM3S6965 输出

运行 LM3S6965 版本应该看到类似下面的输出：

```
====================================
  LM3S6965 FreeRTOS 环境验证测试
====================================

系统初始化完成，创建测试任务...
任务创建成功，启动FreeRTOS调度器...

Blink: 0 - FreeRTOS运行正常!
Blink: 1 - FreeRTOS运行正常!
...
```

程序会每500毫秒输出一行信息，说明FreeRTOS正在正常运行，任务调度工作正常。

按下 `Ctrl+A` 然后按 `X` 可以退出QEMU。

## STM32F103 配置说明 ⭐

基于分析报告和CORTEX_STM32F103_Keil项目配置：

| 配置项 | 值 | 说明 |
|--------|-----|------|
| CPU主频 | 72MHz | STM32F103最大主频 |
| 堆大小 | 10KB | 适配20KB RAM |
| 最小栈 | 80字 | 适度减小以节省内存 |
| 最大优先级 | 5级 | 参考Keil项目 |
| 串口地址 | 0x40013804 | USART1 DR寄存器 |
| QEMU机器 | olimex-stm32-p103 | Olimex STM32-P103开发板 |

## 故障排查指南

### 1. 编译错误：arm-none-eabi-gcc: not found

**问题**：找不到交叉编译工具链

**解决**：
```bash
sudo apt update
sudo apt install gcc-arm-none-eabi
```

或者重新运行根目录的 `init.sh` 脚本。

### 2. 编译错误：No such file or directory: FreeRTOS/Source/include/FreeRTOS.h

**问题**：找不到FreeRTOS源码

**解决**：
- 确认项目根目录下存在 `FreeRTOS` 目录
- 如果不存在，运行 `./init.sh` 自动克隆FreeRTOS源码
- 如果已经手动下载，确认放置在项目根目录

### 3. 链接错误：undefined reference to `xxx'

**问题**：通常是FreeRTOS源码路径配置错误

**解决**：
- 检查Makefile中的FreeRTOS路径是否正确
- 当前Makefile使用相对路径 `../../../FreeRTOS/`，如果项目结构没改变应该没问题
- 确认FreeRTOS-Kernel已经正确克隆到 `FreeRTOS/FreeRTOS/Source/`

### 4. QEMU运行没有输出

**问题**：QEMU启动后卡住，没有任何输出

**解决**：
- 确认QEMU版本支持相应的机器类型
- 检查QEMU安装：`qemu-system-arm --version`
- 如果没安装：`sudo apt install qemu-system-arm`
- STM32F103使用 `olimex-stm32-p103` 机器
- LM3S6965使用 `lm3s6965evb` 机器

### 5. 编译成功但QEMU立即退出

**问题**：QEMU启动后立即退出，没有错误信息

**解决**：
- 检查ELF文件是否正确生成
- 确认链接脚本中的存储器地址正确
  - STM32F103/LM3S6965 Flash从0x08000000开始
  - LM3S6965 Flash从0x00000000开始
- 确认中断向量表放在正确位置（Flash开头）

### 6. 输出乱码

**问题**：能看到输出但是内容乱码

**解决**：QEMU中串口波特率由硬件配置决定，本项目直接输出数据不需要配置波特率，如果出现乱码通常是因为QEMU版本问题，尝试升级QEMU到最新版本。

### 7. STM32F103编译出错

**问题**：STM32F103版本编译失败

**解决**：
- 确保使用 `make all-stm32f103` 而不是 `make all`
- 确认 `include/FreeRTOSConfig_STM32F103.h` 文件存在
- 确认 `src/main_stm32f103.c` 文件存在

## 下一步

如果项目能够正常编译运行并且输出正确，恭喜你！开发环境已经准备就绪，可以进入下一个学习项目。

## 参考资料

- [FreeRTOS官方网站](https://www.freertos.org/)
- [QEMU ARM模拟器文档](https://www.qemu.org/docs/master/system/target-arm.html)
- [STM32F103参考手册](https://www.st.com/resource/en/reference_manual/cd00171190-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf)
- [Olimex STM32-P103开发板](https://www.olimex.com/Products/ARM/ST/STM32-P103/)
- [STM32F103 QEMU支持分析报告](../../../doc/experiences/qemu/STM32F103_QEMU支持分析报告.md)
- [项目根目录README](../../../../Readme.md)
