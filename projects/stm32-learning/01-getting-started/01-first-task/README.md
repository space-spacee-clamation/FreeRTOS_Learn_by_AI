# 第一个FreeRTOS任务示例 - 01-first-task

## 项目介绍

这是STM32-FreeRTOS渐进式学习的第二个项目，**目的是学习如何创建和运行第一个FreeRTOS任务**。

在环境验证项目中，我们已经看到了FreeRTOS任务的运行，但这个项目将更加专注于：

1. 📚 理解FreeRTOS任务的基本概念
2. 🔧 学习 `xTaskCreate()` API的使用
3. ⏱️ 了解任务延时函数 `vTaskDelay()`
4. 🔄 观察FreeRTOS调度器如何工作

**支持的平台：**
- **LM3S6965** (QEMU: `lm3s6965evb`) - 原始支持平台
- **STM32F103** (QEMU: `olimex-stm32-p103`) - 新增支持平台 ⭐

## 学习目标

通过这个项目，你将学会：

- ✅ 什么是FreeRTOS任务
- ✅ 如何使用 `xTaskCreate()` 创建任务
- ✅ 任务函数的编写规范
- ✅ 如何使用 `vTaskDelay()` 进行任务延时
- ✅ 理解任务优先级的基本概念
- ✅ 启动FreeRTOS调度器 `vTaskStartScheduler()`

## 详细教学文档

本项目配套了详细的教学文档，按知识点拆分：

- 📖 [文档索引](../../../../doc/tutorial/project-guides/01-first-task/README.md)
- 📖 [任务概念介绍](../../../../doc/tutorial/project-guides/01-first-task/01_任务概念介绍.md)
- 📖 [xTaskCreate() API详解](../../../../doc/tutorial/project-guides/01-first-task/02_xTaskCreate_API详解.md)
- 📖 [任务函数编写指南](../../../../doc/tutorial/project-guides/01-first-task/03_任务函数编写指南.md)
- 📖 [vTaskDelay()使用说明](../../../../doc/tutorial/project-guides/01-first-task/04_vTaskDelay使用说明.md)
- 📖 [调度器启动指南](../../../../doc/tutorial/project-guides/01-first-task/05_调度器启动指南.md)
- 📖 [常见问题FAQ](../../../../doc/tutorial/project-guides/01-first-task/06_常见问题FAQ.md)

## 参考资料

这个项目的实现参考了FreeRTOS官方文档和Demo项目：

- **FreeRTOS官方文档**: [Task Creation](https://www.freertos.org/a00125.html)
- **参考Demo项目**: `FreeRTOS/FreeRTOS/Demo/CORTEX_LM3S6965_GCC_QEMU/`
- **Mastering-the-FreeRTOS-Kernel**: 第4章 - 任务管理

## 项目结构

```
01-first-task/
├── Makefile                    # 编译脚本（支持多平台）
├── LM3S6965.ld                 # LM3S6965链接脚本
├── STM32F103RB.ld              # STM32F103链接脚本
├── startup_lm3s6965.c          # LM3S6965启动文件
├── startup_stm32f103.c         # STM32F103启动文件
├── include/
│   ├── FreeRTOSConfig_LM3S6965.h  # LM3S6965 FreeRTOS配置
│   └── FreeRTOSConfig_STM32F103.h # STM32F103 FreeRTOS配置
└── src/
    └── main.c                 # 主程序 - 包含第一个任务示例
```

## 核心知识点详解

### 1. 什么是FreeRTOS任务？

FreeRTOS任务是一个独立的执行线程，具有：
- 自己的栈空间
- 自己的程序计数器
- 自己的CPU寄存器状态

**关键点**：
- 每个任务都是一个无限循环函数
- 任务不会返回（如果返回会被调度器删除）
- 任务之间通过调度器分时共享CPU

### 2. 任务创建函数 `xTaskCreate()`

```c
BaseType_t xTaskCreate(
    TaskFunction_t pvTaskCode,        // 任务函数指针
    const char * const pcName,         // 任务名称（调试用）
    const uint16_t usStackDepth,       // 栈大小（以字为单位）
    void * const pvParameters,          // 传递给任务的参数
    UBaseType_t uxPriority,            // 任务优先级
    TaskHandle_t * const pxCreatedTask // 返回的任务句柄
);
```

**参数说明**：
- `pvTaskCode`: 任务函数的地址，任务函数必须是 `void func(void *pvParameters)` 格式
- `pcName`: 任务的文本名称，用于调试，不影响功能
- `usStackDepth`: 任务栈的大小，以字为单位（32位系统中1字=4字节）
- `pvParameters`: 传递给任务函数的参数，可以是NULL
- `uxPriority`: 任务的优先级，数值越大优先级越高
- `pxCreatedTask`: 用于返回任务句柄，如果不需要可以传NULL

**返回值**：
- `pdPASS`: 任务创建成功
- `errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY`: 内存不足，创建失败

### 3. 任务函数编写规范

```c
void vExampleTask(void *pvParameters)
{
    // 可选：使用传递进来的参数
    (void)pvParameters; // 防止未使用参数警告

    for(;;) // 任务必须是无限循环
    {
        // 任务代码
        vTaskDelay(pdMS_TO_TICKS(1000)); // 延时1秒
    }

    // 任务永远不应该到这里！
    // 如果到这里，任务会被调度器删除
}
```

**重要规则**：
1. 任务函数必须是 `void func(void *)` 格式
2. 任务函数必须包含无限循环
3. 任务函数不应该返回
4. 任务内使用 `vTaskDelay()` 让出CPU

### 4. 任务延时函数 `vTaskDelay()`

```c
void vTaskDelay(const TickType_t xTicksToDelay);
```

**功能**：让任务进入阻塞状态，延时指定的系统时钟节拍数。

**参数**：
- `xTicksToDelay`: 延时的系统时钟节拍数

**辅助宏**：
```c
// 将毫秒转换为系统时钟节拍数
pdMS_TO_TICKS(xTimeInMs)
```

**示例**：
```c
vTaskDelay(pdMS_TO_TICKS(500));  // 延时500毫秒
vTaskDelay(pdMS_TO_TICKS(1000)); // 延时1秒
```

### 5. 启动调度器 `vTaskStartScheduler()`

```c
void vTaskStartScheduler(void);
```

**功能**：启动FreeRTOS调度器，开始任务调度。

**重要说明**：
- 调用此函数后，调度器会开始运行创建的任务
- 如果调度器启动成功，此函数**永远不会返回**
- 只有在内存不足导致调度器启动失败时才会返回

## 代码学习重点

### 📍 重点1：创建第一个任务

在 `src/main.c` 中，我们创建了一个简单的任务：

```c
// 创建第一个任务
xTaskCreate(
    vFirstTask,              // 任务函数
    "FirstTask",             // 任务名称
    128,                     // 栈大小（128字 = 512字节）
    NULL,                    // 任务参数
    1,                       // 优先级（1 = 最低优先级）
    NULL                     // 任务句柄（不需要）
);
```

### 📍 重点2：任务函数实现

```c
void vFirstTask(void *pvParameters)
{
    (void)pvParameters; // 防止未使用参数警告
    uint32_t ulCounter = 0;

    for(;;)
    {
        // 输出消息
        vPrintString("FirstTask: 计数器 = ");
        vPrintNumber(ulCounter);
        vPrintString("\r\n");

        ulCounter++;

        // 延时500毫秒
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
```

### 📍 重点3：启动调度器

```c
// 启动FreeRTOS调度器
vPrintString("启动FreeRTOS调度器...\r\n");
vTaskStartScheduler();

// 如果到这里，说明调度器启动失败（通常是内存不足）
vPrintString("错误：调度器启动失败！\r\n");
for(;;);
```

## 编译和运行方法

### 前置要求

确保已经运行过项目根目录的 `init.sh` 脚本完成环境初始化。

### 查看可用目标

```bash
make help
```

### 编译 LM3S6965 版本

```bash
cd projects/stm32-learning/01-getting-started/01-first-task
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

## 预期输出结果

### STM32F103 输出

如果一切正常，运行 STM32F103 版本应该看到类似下面的输出：

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

FirstTask: 计数器 = 0
FirstTask: 计数器 = 1
FirstTask: 计数器 = 2
FirstTask: 计数器 = 3
FirstTask: 计数器 = 4
...
```

### LM3S6965 输出

运行 LM3S6965 版本应该看到类似的输出：

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

FirstTask: 计数器 = 0
FirstTask: 计数器 = 1
FirstTask: 计数器 = 2
...
```

程序会每500毫秒输出一行信息，计数器不断递增。

按下 `Ctrl+A` 然后按 `X` 可以退出QEMU。

## 学习思考问题

1. **任务栈大小**：我们使用了128字（512字节）的栈大小，这个大小合适吗？如何确定任务需要多少栈空间？

2. **任务优先级**：我们使用了优先级1，如果有多个任务，优先级如何影响它们的执行顺序？

3. **任务延时**：`vTaskDelay()` 和普通的延时循环有什么区别？为什么在FreeRTOS中必须使用 `vTaskDelay()`？

4. **无限循环**：为什么任务函数必须是无限循环？如果任务函数返回会发生什么？

## 常见问题排查

### 1. 编译错误：undefined reference to `xTaskCreate'

**问题**：找不到FreeRTOS函数

**解决**：
- 检查Makefile中是否正确包含了FreeRTOS源文件
- 确认FreeRTOS源码路径正确

### 2. 任务创建失败

**问题**：`xTaskCreate()` 返回失败

**可能原因**：
- 堆内存不足：检查 `FreeRTOSConfig.h` 中的 `configTOTAL_HEAP_SIZE`
- 栈大小设置过大：减小 `usStackDepth` 参数

### 3. 调度器启动失败

**问题**：`vTaskStartScheduler()` 返回了（这不应该发生）

**解决**：
- 通常是因为空闲任务创建失败
- 增加堆内存大小 `configTOTAL_HEAP_SIZE`
- 确认使用了正确的内存分配方案（heap_2、heap_4等）

### 4. 任务没有运行

**问题**：调度器启动了，但任务没有输出

**可能原因**：
- 任务优先级设置为0（空闲任务优先级）
- 任务函数中有错误导致崩溃
- 串口输出配置问题

## 下一步

恭喜你完成了第一个FreeRTOS任务的学习！

当你理解了这个项目后，可以进入下一个学习项目：
- **多任务调度示例**：学习如何创建多个任务并观察它们如何调度
- **任务优先级示例**：深入理解任务优先级如何影响执行顺序

## 参考资料

- [FreeRTOS官方任务创建文档](https://www.freertos.org/a00125.html)
- [FreeRTOS任务控制文档](https://www.freertos.org/a00112.html)
- [Mastering-the-FreeRTOS-Kernel 第4章](../../../../doc/tutorial/Mastering-the-FreeRTOS-Kernel/04_任务管理.md)
- [环境验证项目](../00-env-verify/README.md) - 上一个学习项目
- [项目根目录README](../../../../Readme.md)
