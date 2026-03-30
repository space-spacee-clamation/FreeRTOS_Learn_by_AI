# 02 - xTaskCreate() API详解

## 函数原型

```c
BaseType_t xTaskCreate(
    TaskFunction_t pvTaskCode,        /* 任务函数指针 */
    const char * const pcName,         /* 任务名称 */
    const uint16_t usStackDepth,       /* 栈大小 */
    void * const pvParameters,          /* 任务参数 */
    UBaseType_t uxPriority,            /* 任务优先级 */
    TaskHandle_t * const pxCreatedTask  /* 任务句柄 */
);
```

## 参数详细说明

### 参数1：pvTaskCode - 任务函数指针

**类型**：`TaskFunction_t`（本质是 `void (*)(void *)`）

**作用**：指定任务要执行的函数

**要求**：
- 函数必须是 `void func(void *pvParameters)` 格式
- 函数必须包含无限循环
- 函数不能返回（如果返回会被调度器删除）

**示例**：
```c
// 正确的任务函数
void vMyTask(void *pvParameters)
{
    for(;;)
    {
        // 任务代码
        vTaskDelay(100);
    }
}

// 创建任务时传入函数名
xTaskCreate(vMyTask, ...);
```

**常见错误**：
```c
// ❌ 错误：函数格式不对
void vMyTaskWrong1()  // 缺少参数
{
}

// ❌ 错误：返回值不是void
int vMyTaskWrong2(void *pvParameters)
{
    return 0;
}

// ❌ 错误：创建时加括号
xTaskCreate(vMyTask(), ...);  // 这样是调用函数，不是传递指针！
```

---

### 参数2：pcName - 任务名称

**类型**：`const char * const`

**作用**：给任务起个名字，用于调试

**特点**：
- 只用于调试，不影响功能
- FreeRTOS内部不会修改这个字符串
- 通常用简短的描述性名称

**示例**：
```c
xTaskCreate(vMyTask, "MyTask", ...);       // 简洁
xTaskCreate(vUARTTask, "UARTTask", ...);   // 描述功能
xTaskCreate(vLEDTask, "LEDTask", ...);     // 清晰明了
```

**调试用途**：
```c
// 在调试器中，可以通过任务名称识别任务
// FreeRTOS也提供API获取任务名称
char *pcTaskName = pcTaskGetTaskName(xTaskHandle);
```

---

### 参数3：usStackDepth - 栈大小

**类型**：`uint16_t`

**作用**：指定任务栈的大小

**重要**：单位是**字（word）**，不是字节！

- 32位系统（Cortex-M3/M4）：1字 = 4字节
- 16位系统：1字 = 2字节

**示例**：
```c
// 对于Cortex-M3（32位）：
usStackDepth = 128;  // 128字 × 4字节 = 512字节
usStackDepth = 256;  // 256字 × 4字节 = 1024字节
usStackDepth = 512;  // 512字 × 4字节 = 2048字节
```

**如何确定栈大小？**

1. **从小开始**：先给一个合理的初始值
2. **使用栈溢出检测**：FreeRTOS提供栈溢出检测
3. **监控实际使用**：使用 `uxTaskGetStackHighWaterMark()`

```c
// 初始设置
#define 任务栈大小  128

// 运行时检查实际使用的栈大小
UBaseType_t uxHighWaterMark;
uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
// 返回值是剩余的栈空间（字），越小说明栈使用越多
```

**栈大小建议**：

| 任务类型 | 推荐栈大小 | 说明 |
|---------|-----------|------|
| 简单任务 | 128-256字 | 只有基本操作 |
| 普通任务 | 256-512字 | 有函数调用、局部变量 |
| 复杂任务 | 512-1024字 | 有大数组、深度调用 |
| 非常复杂 | 1024+字 | 有大量局部数据 |

**常见错误**：
```c
// ❌ 栈太小
xTaskCreate(vMyTask, "Task", 16, ...);  // 很容易溢出！

// ❌ 栈太大（浪费内存）
xTaskCreate(vMyTask, "Task", 4096, ...);  // 简单任务不需要这么大！
```

---

### 参数4：pvParameters - 任务参数

**类型**：`void *`

**作用**：给任务传递参数

**特点**：
- 可以是任意类型的指针
- 如果不需要参数，传 `NULL`
- 任务可以通过这个指针访问外部数据

**示例1：不传参数**
```c
// 不需要传参数
xTaskCreate(vMyTask, "Task", 128, NULL, 1, NULL);

// 任务函数中
void vMyTask(void *pvParameters)
{
    (void)pvParameters;  // 防止编译器警告（未使用参数）
    
    for(;;)
    {
        // ...
    }
}
```

**示例2：传整数**
```c
// 注意：这里用了类型转换技巧
// 把整数转换成指针传递（不推荐用于大整数）
int 任务ID = 1;
xTaskCreate(vMyTask, "Task", 128, (void *)任务ID, 1, NULL);

// 任务函数中
void vMyTask(void *pvParameters)
{
    int 我的ID = (int)pvParameters;
    
    for(;;)
    {
        // 使用 我的ID
    }
}
```

**示例3：传结构体指针（推荐）**
```c
// 定义一个结构体
typedef struct
{
    int 任务ID;
    char 任务名称[20];
    int 延时时间;
} TaskParams_t;

// 创建参数实例
TaskParams_t xParams = {
    .任务ID = 1,
    .任务名称 = "MyTask",
    .延时时间 = 100
};

// 传递结构体指针
xTaskCreate(vMyTask, "Task", 128, &xParams, 1, NULL);

// 任务函数中
void vMyTask(void *pvParameters)
{
    TaskParams_t *pxParams = (TaskParams_t *)pvParameters;
    
    for(;;)
    {
        // 使用 pxParams->任务ID, pxParams->延时时间 等
        vTaskDelay(pxParams->延时时间);
    }
}
```

**重要提醒**：
- ⚠️ 确保传递的数据在任务运行期间一直有效
- ⚠️ 不要传递局部变量的指针（除非保证变量生命周期）
- ⚠️ 传递全局变量或动态分配的内存更安全

---

### 参数5：uxPriority - 任务优先级

**类型**：`UBaseType_t`

**作用**：设置任务的优先级

**优先级规则**：
- 数值越大，优先级越高
- 空闲任务优先级是0（最低）
- 相同优先级的任务轮流执行（时间片）
- 高优先级任务可以抢占低优先级任务

**示例**：
```c
// 优先级定义
#define 优先级_空闲     0   // 空闲任务（系统使用）
#define 优先级_低       1   // 低优先级
#define 优先级_普通     2   // 普通优先级
#define 优先级_高       3   // 高优先级
#define 优先级_最高     4   // 最高优先级（注意不要超过配置的最大值）

// 创建不同优先级的任务
xTaskCreate(vLowTask, "Low", 128, NULL, 优先级_低, NULL);
xTaskCreate(vNormalTask, "Normal", 128, NULL, 优先级_普通, NULL);
xTaskCreate(vHighTask, "High", 128, NULL, 优先级_高, NULL);
```

**优先级抢占示例**：
```
时间轴 →
[低优先级任务运行中...]
    ↑
高优先级任务就绪
    ↓
[高优先级任务立即运行...]  ← 抢占！
[高优先级任务完成...]
    ↓
[低优先级任务继续运行...]
```

**优先级配置**：
```c
// 在 FreeRTOSConfig.h 中配置
#define configMAX_PRIORITIES  5  // 支持0-4共5个优先级
```

---

### 参数6：pxCreatedTask - 任务句柄

**类型**：`TaskHandle_t *`

**作用**：返回创建的任务的句柄，用于后续操作这个任务

**特点**：
- 如果不需要操作任务，传 `NULL`
- 如果需要操作任务（删除、挂起、恢复等），需要传一个指针
- 句柄就像任务的"身份证"

**示例1：不需要句柄**
```c
// 简单任务，创建后不需要操作它
xTaskCreate(vMyTask, "Task", 128, NULL, 1, NULL);
```

**示例2：需要句柄**
```c
// 定义句柄变量
TaskHandle_t xMyTaskHandle;

// 创建任务，获取句柄
xTaskCreate(vMyTask, "Task", 128, NULL, 1, &xMyTaskHandle);

// 后续可以使用句柄操作任务
vTaskDelete(xMyTaskHandle);      // 删除任务
vTaskSuspend(xMyTaskHandle);     // 挂起任务
vTaskResume(xMyTaskHandle);      // 恢复任务
```

**使用句柄的好处**：
```c
// 有了句柄，你可以精确控制任务
void vSomeFunction(void)
{
    // 暂停某个特定任务
    vTaskSuspend(xMyTaskHandle);
    
    // 做一些需要独占资源的操作...
    
    // 恢复任务
    vTaskResume(xMyTaskHandle);
}
```

---

## 返回值

**类型**：`BaseType_t`

**返回值**：
- `pdPASS` - 任务创建成功
- `errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY` - 内存不足，创建失败

**检查返回值的示例**：
```c
TaskHandle_t xMyTaskHandle;
BaseType_t xReturn;

// 创建任务并检查结果
xReturn = xTaskCreate(
    vMyTask,
    "MyTask",
    128,
    NULL,
    1,
    &xMyTaskHandle
);

if(xReturn == pdPASS)
{
    // 任务创建成功
    printf("任务创建成功！\n");
}
else
{
    // 任务创建失败（通常是内存不足）
    printf("任务创建失败！\n");
    // 这里可以做错误处理
}
```

---

## 完整使用示例

### 示例1：最简单的任务创建
```c
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

// 任务函数
void vSimpleTask(void *pvParameters)
{
    (void)pvParameters;
    
    for(;;)
    {
        printf("简单任务运行中...\n");
        vTaskDelay(1000);  // 延时1秒
    }
}

int main(void)
{
    // 创建任务
    xTaskCreate(
        vSimpleTask,      // 任务函数
        "SimpleTask",     // 任务名称
        128,              // 栈大小（字）
        NULL,             // 任务参数
        1,                // 优先级
        NULL              // 任务句柄（不需要）
    );
    
    // 启动调度器
    vTaskStartScheduler();
    
    // 永远不会到这里
    for(;;);
    return 0;
}
```

### 示例2：带参数和句柄的任务
```c
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

// 任务参数结构体
typedef struct
{
    int 任务ID;
    const char *任务名称;
    int 延时毫秒;
} TaskConfig_t;

// 任务句柄
TaskHandle_t xLEDTaskHandle = NULL;
TaskHandle_t xUARTTaskHandle = NULL;

// LED任务
void vLEDTask(void *pvParameters)
{
    TaskConfig_t *pxConfig = (TaskConfig_t *)pvParameters;
    
    printf("LED任务启动：ID=%d, 名称=%s\n", 
           pxConfig->任务ID, pxConfig->任务名称);
    
    for(;;)
    {
        printf("LED闪烁...\n");
        vTaskDelay(pdMS_TO_TICKS(pxConfig->延时毫秒));
    }
}

// UART任务
void vUARTTask(void *pvParameters)
{
    TaskConfig_t *pxConfig = (TaskConfig_t *)pvParameters;
    
    printf("UART任务启动：ID=%d, 名称=%s\n", 
           pxConfig->任务ID, pxConfig->任务名称);
    
    for(;;)
    {
        printf("UART发送数据...\n");
        vTaskDelay(pdMS_TO_TICKS(pxConfig->延时毫秒));
    }
}

int main(void)
{
    BaseType_t xReturn;
    
    // 配置参数
    static TaskConfig_t xLEDConfig = {
        .任务ID = 1,
        .任务名称 = "LEDTask",
        .延时毫秒 = 500
    };
    
    static TaskConfig_t xUARTConfig = {
        .任务ID = 2,
        .任务名称 = "UARTTask",
        .延时毫秒 = 1000
    };
    
    // 创建LED任务
    xReturn = xTaskCreate(
        vLEDTask,
        xLEDConfig.任务名称,
        128,
        &xLEDConfig,
        2,
        &xLEDTaskHandle
    );
    
    if(xReturn != pdPASS)
    {
        printf("LED任务创建失败！\n");
        return -1;
    }
    
    // 创建UART任务
    xReturn = xTaskCreate(
        vUARTTask,
        xUARTConfig.任务名称,
        128,
        &xUARTConfig,
        1,
        &xUARTTaskHandle
    );
    
    if(xReturn != pdPASS)
    {
        printf("UART任务创建失败！\n");
        return -1;
    }
    
    printf("所有任务创建成功，启动调度器...\n");
    
    // 启动调度器
    vTaskStartScheduler();
    
    // 永远不会到这里
    for(;;);
    return 0;
}
```

---

## 常见错误总结

| 错误 | 原因 | 解决方法 |
|------|------|---------|
| 栈溢出 | `usStackDepth` 太小 | 增大栈大小，使用栈溢出检测 |
| 任务不运行 | 优先级太低，或没有启动调度器 | 检查优先级，确保调用 `vTaskStartScheduler()` |
| 编译错误 | 函数参数类型不对 | 确保任务函数格式正确：`void func(void *)` |
| 内存不足 | 堆空间太小 | 增加 `configTOTAL_HEAP_SIZE` |
| 任务立即返回 | 任务函数中没有无限循环 | 在任务函数中添加 `for(;;)` 循环 |

---

## 记忆口诀

```
任务创建六参数，
函数名称栈大小，
参数优先级句柄，
按序传递别忘掉。

返回值，要检查，
pdPASS 是成功啦。
任务函数无限循，
调度启动靠主程。
```

---

## 下一步

现在你已经了解了 `xTaskCreate()` 的详细用法，接下来学习：
- [如何编写任务函数](./03_任务函数编写指南.md)
- [如何使用 vTaskDelay()](./04_vTaskDelay使用说明.md)
