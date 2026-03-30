# STM32F103 FreeRTOS QEMU 支持分析报告

## 一、概述

本报告对FreeRTOS Demo项目中的STM32F103配置进行了深入分析，对比了已在QEMU中成功运行的LM3S6965项目，并研究了STM32F103在QEMU中运行的可行性。

## 二、CORTEX_STM32F103_Keil 项目分析

### 2.1 关键配置参数 (FreeRTOSConfig.h)

| 配置项 | 值 | 说明 |
|--------|-----|------|
| `configCPU_CLOCK_HZ` | 72000000 | CPU主频72MHz |
| `configTICK_RATE_HZ` | 1000 | 系统时钟节拍1kHz |
| `configMAX_PRIORITIES` | 5 | 最大优先级数 |
| `configMINIMAL_STACK_SIZE` | 128 | 最小堆栈大小(字) |
| `configTOTAL_HEAP_SIZE` | 17KB | 总堆大小 |
| `configUSE_PREEMPTION` | 1 | 抢占式调度 |
| `configKERNEL_INTERRUPT_PRIORITY` | 255 | 内核中断优先级 |
| `configMAX_SYSCALL_INTERRUPT_PRIORITY` | 191 | 系统调用中断优先级 |

### 2.2 main.c 结构分析

**硬件初始化流程 (`prvSetupHardware`):**
1. 配置外部高速晶振 (HSE) 8MHz
2. 配置PLL: 8MHz * 9 = 72MHz
3. 配置Flash等待状态
4. 配置AHB/APB总线分频
5. 使能GPIO、SPI等外设时钟
6. 配置中断向量表 (0x08000000)
7. 配置NVIC优先级分组
8. 配置SysTick时钟源

**任务创建:**
- 阻塞队列任务
- 块时间测试任务
- 信号量测试任务
- 轮询队列任务
- 整数数学任务
- LED闪烁任务
- 串口测试任务
- Check任务 (高优先级，5秒周期)
- LCD任务 (看门守任务)

### 2.3 硬件依赖

该项目使用STM32F10x标准外设库，依赖：
- STM32F10x硬件寄存器
- LCD显示驱动
- SPI Flash
- 定时器中断

## 三、CORTEX_LM3S6965_GCC_QEMU 项目分析

### 3.1 QEMU启动方式

**启动命令 (StartQEMU.bat):**
```batch
qemu-system-arm -machine lm3s6965evb -s -S -kernel ./debug/RTOSDemo.elf
```

**关键参数:**
- `-machine lm3s6965evb`: 指定机器类型为LM3S6965评估板
- `-s`: 开启GDB服务器 (端口1234)
- `-S`: 启动时暂停，等待GDB连接
- `-kernel`: 指定内核ELF文件

### 3.2 关键配置参数 (FreeRTOSConfig.h)

| 配置项 | 值 | 说明 |
|--------|-----|------|
| `configCPU_CLOCK_HZ` | 50000000 | CPU主频50MHz |
| `configTICK_RATE_HZ` | 1000 | 系统时钟节拍1kHz |
| `configMINIMAL_STACK_SIZE` | 90 | 最小堆栈大小 |
| `configTOTAL_HEAP_SIZE` | 50KB | 总堆大小 |
| `configUSE_TIMERS` | 1 | 启用软件定时器 |
| `configUSE_MUTEXES` | 1 | 启用互斥量 |
| `configSUPPORT_STATIC_ALLOCATION` | 1 | 支持静态分配 |
| `configKERNEL_INTERRUPT_PRIORITY` | 255 | 所有8位都使用 |

### 3.3 内存布局 (standalone.ld)

```
MEMORY
{
    FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 256K
    SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 64K
}
```

**注意:** LM3S6965的Flash从0x00000000开始，而STM32从0x08000000开始。

### 3.4 LM3S6965 QEMU成功要素

1. **完整的机器模型**: QEMU官方支持`lm3s6965evb`
2. **简化的硬件初始化**: QEMU中无需复杂的时钟配置
3. **UART0映射**: 串口0直接映射到标准输出
4. **标准Cortex-M3端口**: 与STM32相同的ARM_CM3移植
5. **完整的中断向量表**: 包含所有必要的异常处理

## 四、当前 00-env-verify 项目分析

### 4.1 项目结构

```
00-env-verify/
├── Makefile                    # 编译脚本
├── LM3S6965.ld                 # LM3S6965链接脚本
├── STM32F103RB.ld              # STM32F103链接脚本(已存在)
├── STM32F407VG.ld              # STM32F407链接脚本
├── startup_lm3s6965.c          # LM3S6965启动文件
├── startup_stm32f103rb.s       # STM32F103启动文件(已存在)
├── startup_stm32f407vgt6.s     # STM32F407启动文件
├── include/
│   └── FreeRTOSConfig.h         # 当前使用LM3S6965配置
└── src/
    └── main.c                   # 当前使用LM3S6965串口地址
```

### 4.2 Makefile分析

**当前配置:**
- 目标芯片: LM3S6965
- 编译器: arm-none-eabi-gcc
- CPU: Cortex-M3
- 链接脚本: LM3S6965.ld
- 启动文件: startup_lm3s6965.o
- QEMU运行: `qemu-system-arm -machine lm3s6965evb`

**已有STM32F103相关文件:**
- `STM32F103RB.ld` - STM32F103链接脚本
- `startup_stm32f103rb.s` - STM32F103启动文件

### 4.3 内存配置对比

**LM3S6965:**
- Flash: 256KB @ 0x00000000
- RAM: 64KB @ 0x20000000

**STM32F103RB:**
- Flash: 128KB @ 0x08000000
- RAM: 20KB @ 0x20000000

## 五、QEMU对STM32F103的支持情况

### 5.1 QEMU支持的STM32机器类型

让我们检查QEMU支持的机器类型：

```bash
qemu-system-arm -machine help
```

### 5.2 当前QEMU STM32支持现状

根据QEMU官方文档和社区资源：

**官方支持的STM32机器:**
- `stm32vldiscovery` - STM32VLDISCOVERY (STM32F100RB)
- `stm32f4-discovery` - STM32F4DISCOVERY (STM32F407VG)
- `netduino2` - Netduino 2 (STM32F205RF)
- `netduinoplus2` - Netduino Plus 2 (STM32F405RG)
- `olimex-stm32-h407` - Olimex STM32-H407 (STM32F407ZG)
- `olimex-stm32-p103` - Olimex STM32-P103 (STM32F103RBT6) ⭐

**重要发现:** QEMU支持`olimex-stm32-p103`，这是基于STM32F103RBT6的开发板！

### 5.3 STM32F103 vs STM32F100

| 特性 | STM32F103RB | STM32F100RB |
|------|-------------|-------------|
| 内核 | Cortex-M3 | Cortex-M3 |
| 主频 | 72MHz | 24MHz |
| Flash | 128KB | 128KB |
| RAM | 20KB | 8KB |
| USB | ✓ | ✗ |
| CAN | ✓ | ✗ |

## 六、STM32F103在QEMU中运行的可行性分析

### 6.1 可行性评估

**结论: 可行！**

理由：
1. ✅ QEMU支持`olimex-stm32-p103`机器类型（STM32F103RBT6）
2. ✅ 已有STM32F103链接脚本和启动文件
3. ✅ 可以复用LM3S6965项目的FreeRTOS配置框架
4. ✅ Cortex-M3内核移植代码通用

### 6.2 需要的关键配置

#### 6.2.1 修改Makefile

需要添加STM32F103编译目标：

```makefile
# 新增：STM32F103配置
MCU_STM32F103 = STM32F103
CFLAGS_STM32F103 = -mcpu=cortex-m3 -mthumb -DSTM32F103xB
LDFLAGS_STM32F103 = -TSTM32F103RB.ld
STARTUP_STM32F103 = startup_stm32f103rb.o

# 新增：QEMU运行STM32F103
run-stm32f103: all-stm32f103
	qemu-system-arm -machine olimex-stm32-p103 -kernel $(ELF_STM32F103) -nographic -serial mon:stdio
```

#### 6.2.2 FreeRTOSConfig.h调整

基于STM32F103_Keil项目调整：

```c
#define configCPU_CLOCK_HZ          ( ( unsigned long ) 72000000 )  // 72MHz
#define configTOTAL_HEAP_SIZE       ( ( size_t ) ( 10 * 1024 ) )     // 10KB (适配20KB RAM)
#define configMINIMAL_STACK_SIZE    ( ( unsigned short ) 80 )          // 适当减小
```

#### 6.2.3 串口输出调整

STM32F103 USART1地址：0x40013800 (数据寄存器DR)

```c
// STM32F103 USART1 DR寄存器地址
volatile uint32_t *const uart_dr = (volatile uint32_t *)0x40013804;
```

或者使用更简单的方式，直接写入QEMU的串口输出地址。

#### 6.2.4 简化硬件初始化

在QEMU中可以大幅简化初始化：

```c
void SystemInit(void)
{
    // QEMU已经处理了时钟配置
    // 无需配置PLL、Flash等待状态等
}
```

### 6.3 潜在挑战

1. **外设仿真限制**: QEMU可能不完全仿真所有STM32F103外设
2. **内存较小**: 只有20KB RAM，需要仔细配置FreeRTOS堆大小
3. **Flash起始地址**: 0x08000000 vs LM3S6965的0x00000000
4. **中断向量表**: 需要确保向量表放在正确位置

## 七、建议实施方案

### 7.1 阶段一：最小可行性验证

1. 创建简化的STM32F103配置
2. 使用QEMU的`olimex-stm32-p103`机器
3. 验证基本的FreeRTOS任务调度
4. 验证串口输出

### 7.2 阶段二：功能完善

1. 适配FreeRTOS配置到STM32F103
2. 添加更多演示任务
3. 测试各种FreeRTOS功能

### 7.3 推荐目录结构

```
00-env-verify/
├── Makefile                    # 支持多目标编译
├── config/
│   ├── lm3s6965/              # LM3S6965配置
│   ├── stm32f103/             # STM32F103配置
│   └── stm32f407/             # STM32F407配置
└── ...
```

## 八、结论

1. **STM32F103可以在QEMU中运行** - QEMU支持`olimex-stm32-p103`机器类型
2. **项目已有良好基础** - 已有STM32F103链接脚本和启动文件
3. **需要适度调整** - 主要是内存配置、串口地址、硬件初始化
4. **建议分阶段实施** - 先验证最小可行性，再逐步完善功能

**下一步行动:** 创建STM32F103的QEMU验证项目，证明可行性。

## 九、参考资料

- [FreeRTOS官方文档](https://www.freertos.org/)
- [QEMU ARM模拟器文档](https://www.qemu.org/docs/master/system/target-arm.html)
- [STM32F103参考手册](https://www.st.com/resource/en/reference_manual/cd00171190-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf)
- [Olimex STM32-P103开发板](https://www.olimex.com/Products/ARM/ST/STM32-P103/)

---

*报告生成日期: 2026-03-29*
*分析工具: VS Code + 文件分析*
