# FreeRTOS目录结构详解

## 📚 概述

本文档详细解释FreeRTOS源代码的目录结构，帮助你理解各个文件和目录的作用。

## 🌳 完整目录树

```
FreeRTOS/                           # FreeRTOS根目录
├── FreeRTOS/                       # FreeRTOS内核主目录
│   ├── Source/                     # 内核源代码
│   │   ├── include/                # 内核头文件
│   │   │   ├── FreeRTOS.h          # 主头文件
│   │   │   ├── task.h              # 任务管理
│   │   │   ├── queue.h             # 队列管理
│   │   │   ├── semphr.h            # 信号量
│   │   │   ├── timers.h            # 软件定时器
│   │   │   ├── event_groups.h      # 事件组
│   │   │   └── ...
│   │   ├── portable/               # 平台移植代码
│   │   │   ├── GCC/                # GCC编译器移植
│   │   │   │   └── ARM_CM3/       # ARM Cortex-M3移植
│   │   │   ├── MemMang/            # 内存管理
│   │   │   │   ├── heap_1.c
│   │   │   │   ├── heap_2.c
│   │   │   │   ├── heap_3.c
│   │   │   │   ├── heap_4.c
│   │   │   │   └── heap_5.c
│   │   ├── tasks.c                 # 任务实现
│   │   ├── queue.c                 # 队列实现
│   │   ├── list.c                  # 链表实现
│   │   ├── timers.c                # 定时器实现
│   │   ├── event_groups.c          # 事件组实现
│   │   ├── croutine.c              # 协程实现
│   │   └── stream_buffer.c         # 流缓冲区实现
│   ├── Demo/                       # 示例项目
│   │   └── CORTEX_LM3S6965_GCC_QEMU/  # 我们使用的示例
│   ├── License/                    # 许可证文件
│   └── Test/                       # 测试代码
├── FreeRTOS-Plus/                  # FreeRTOS扩展组件
└── tools/                          # 工具脚本
```

## 📂 核心目录详解

### 1. FreeRTOS/Source/ - 内核源代码

这是FreeRTOS内核的核心，包含所有操作系统功能的实现。

#### Source/include/ - 头文件

| 文件 | 作用 |
|------|------|
| `FreeRTOS.h` | 主头文件，包含基本类型和宏定义 |
| `task.h` | 任务管理API声明 |
| `queue.h` | 队列和信号量API声明 |
| `semphr.h` | 信号量API（实际包含queue.h） |
| `timers.h` | 软件定时器API声明 |
| `event_groups.h` | 事件组API声明 |
| `portable.h` | 移植层接口声明 |

**使用方法**：在你的代码中只需包含需要的头文件：
```c
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
```

#### Source/portable/ - 平台移植代码

这个目录包含与特定硬件平台和编译器相关的代码。

**目录结构**：
```
portable/
├── [编译器]/          # GCC, IAR, Keil等
│   └── [处理器架构]/  # ARM_CM3, ARM_CM4等
│       ├── port.c     # 移植实现
│       └── portmacro.h # 移植宏定义
└── MemMang/           # 内存管理实现
    ├── heap_1.c
    ├── heap_2.c
    ├── heap_3.c
    ├── heap_4.c
    └── heap_5.c
```

**我们项目使用的移植**：
- 编译器：GCC
- 处理器：ARM Cortex-M3 (LM3S6965) 和 Cortex-M3 (STM32F103)

**内存管理方案**：
| 方案 | 特点 | 适用场景 |
|------|------|---------|
| heap_1 | 最简单，只能分配不能释放 | 不需要删除任务的系统 |
| heap_2 | 支持分配和释放，不合并碎片 | 频繁创建删除相同大小任务 |
| heap_3 | 使用标准库malloc/free | 需要标准库兼容性 |
| heap_4 | 支持分配释放，合并碎片 | 通用场景（推荐） |
| heap_5 | 支持多内存区域 | 有多个不连续内存块 |

**本项目使用 heap_4.c**

#### Source/ 根目录文件

| 文件 | 功能 |
|------|------|
| `tasks.c` | 任务调度、任务创建、任务状态管理 |
| `queue.c` | 队列、信号量、互斥锁实现 |
| `list.c` | 双向链表实现（内核数据结构基础） |
| `timers.c` | 软件定时器实现 |
| `event_groups.c` | 事件组实现 |
| `croutine.c` | 协程实现（轻量级任务） |
| `stream_buffer.c` | 流缓冲区实现 |

### 2. FreeRTOS/Demo/ - 示例项目

包含各种硬件平台和编译器的示例项目。

**我们参考的示例**：`CORTEX_LM3S6965_GCC_QEMU/`

这个示例专门为QEMU仿真的LM3S6965开发板设计。

### 3. FreeRTOS-Plus/ - 扩展组件

包含FreeRTOS的官方扩展组件：
- FreeRTOS+TCP - TCP/IP协议栈
- FreeRTOS+FAT - 文件系统
- FreeRTOS+CLI - 命令行接口
- 等等

## 🔗 我们项目中的使用方式

在我们的学习项目中，FreeRTOS的引用方式如下：

### Makefile中的配置

```makefile
# FreeRTOS根目录
FREERTOS_ROOT = ../../../../FreeRTOS/FreeRTOS

# 内核源代码路径
FREERTOS_SRC = $(FREERTOS_ROOT)/Source

# 头文件搜索路径
INCLUDES += -I$(FREERTOS_SRC)/include
INCLUDES += -I$(FREERTOS_SRC)/portable/GCC/ARM_CM3

# 需要编译的源文件
SOURCES += $(FREERTOS_SRC)/tasks.c
SOURCES += $(FREERTOS_SRC)/queue.c
SOURCES += $(FREERTOS_SRC)/list.c
SOURCES += $(FREERTOS_SRC)/timers.c
SOURCES += $(FREERTOS_SRC)/portable/GCC/ARM_CM3/port.c
SOURCES += $(FREERTOS_SRC)/portable/MemMang/heap_4.c
```

### 编译后的文件

编译时，FreeRTOS源文件会被编译成目标文件：
```
tasks.o
queue.o
list.o
timers.o
port.o
heap_4.o
```

然后与你的应用程序代码链接在一起。

## 💡 学习建议

1. **初学者**：不需要深入阅读内核源码，先学会使用API
2. **进阶学习**：阅读 `tasks.c` 了解任务调度原理
3. **深入理解**：阅读 `queue.c` 理解同步机制
4. **移植学习**：阅读 `port.c` 了解如何移植到新平台

## 📖 相关链接

- [FreeRTOS官方文档](https://www.freertos.org/RTOS-source-code.html)
- [Mastering-the-FreeRTOS-Kernel](../Mastering-the-FreeRTOS-Kernel/)
- [Makefile构建原理](./02_Makefile构建原理.md)
