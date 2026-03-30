# 06 - 常见问题FAQ

## FAQ列表

### FAQ1：任务创建失败怎么办？

**问题**：
```c
if(xTaskCreate(...) != pdPASS)
{
    // 任务创建失败！
}
```

**原因分析**：

| 可能原因 | 检查方法 | 解决方法 |
|---------|---------|---------|
| 堆内存不足 | 检查 `configTOTAL_HEAP_SIZE` | 增加堆大小 |
| 栈大小太大 | 检查 `usStackDepth` 参数 | 减小栈大小 |
| 优先级太高 | 检查 `configMAX_PRIORITIES` | 降低优先级或增加最大优先级 |

**解决步骤**：
```c
// 1. 检查堆大小
#define configTOTAL_HEAP_SIZE    (10 * 1024)  // 至少10KB

// 2. 减小栈大小
xTaskCreate(vTask, "Task", 128, ...);  // 不要用太大

// 3. 检查优先级
#define configMAX_PRIORITIES    5  // 优先级不要超过这个值
xTaskCreate(vTask, "Task", 128, NULL, 1, ...);  // 用1就好
```

---

### FAQ2：调度器启动失败怎么办？

**问题**：
```c
vTaskStartScheduler();
// 执行到这里了！说明启动失败！
```

**最常见原因：内存不足，无法创建空闲任务**

**解决方法**：
```c
// 在 FreeRTOSConfig.h 中增加堆大小
#define configTOTAL_HEAP_SIZE    (10 * 1024)  // 改成至少10KB

// 确保使用了正确的内存分配器
// 检查 Makefile 中是否包含了 heap_2.c 或 heap_4.c
```

**调试技巧**：
```c
// 启用内存分配失败钩子
#define configUSE_MALLOC_FAILED_HOOK    1

void vApplicationMallocFailedHook(void)
{
    // 内存分配失败时会调用这个函数
    输出错误("内存分配失败！\r\n");
    for(;;);
}
```

---

### FAQ3：任务运行了但没有输出？

**问题**：
- 调度器启动成功
- 但串口没有任何输出

**可能原因**：

| 原因 | 检查方法 |
|-----|---------|
| 串口未初始化 | 检查是否调用了串口初始化函数 |
| 优先级太低 | 检查任务优先级是否为0（和空闲任务一样） |
| 栈溢出 | 启用栈溢出检测 |
| 任务卡在某处 | 检查任务代码 |

**解决方法**：
```c
// 1. 确保串口初始化了
vUARTInit();

// 2. 任务优先级不要是0
xTaskCreate(vTask, "Task", 128, NULL, 1, ...);  // 用1或更高

// 3. 启用栈溢出检测
#define configCHECK_FOR_STACK_OVERFLOW    1

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    输出错误("栈溢出：");
    输出字符串(pcTaskName);
    输出字符串("\r\n");
    for(;;);
}
```

---

### FAQ4：任务只运行一次就不运行了？

**问题**：
- 任务运行了一次
- 然后就不运行了

**最常见原因：任务函数返回了！**

```c
// ❌ 错误！
void vBadTask(void *pvParameters)
{
    做某事();
    // 没有无限循环！函数返回了！
}

// 结果：任务运行一次就被删除了！
```

**解决方法**：
```c
// ✅ 正确
void vGoodTask(void *pvParameters)
{
    for(;;)  // 必须有无限循环！
    {
        做某事();
        vTaskDelay(100);
    }
}
```

---

### FAQ5：其他任务无法运行？

**问题**：
- 只有一个任务在运行
- 其他任务都不运行

**最常见原因：一个任务不让出CPU！**

```c
// ❌ 错误！
void vBadTask(void *pvParameters)
{
    for(;;)
    {
        做某事();
        // 没有延时！一直占用CPU！
    }
}
```

**解决方法**：
```c
// ✅ 正确
void vGoodTask(void *pvParameters)
{
    for(;;)
    {
        做某事();
        vTaskDelay(10);  // 必须让出CPU！
    }
}
```

---

### FAQ6：如何选择任务栈大小？

**问题**：
- 栈太小：栈溢出
- 栈太大：浪费内存

**推荐方法**：

1. **先给一个合理的初始值**：
```c
// 简单任务：128字（512字节）
#define TASK_STACK_SIZE_SMALL    128

// 普通任务：256字（1024字节）
#define TASK_STACK_SIZE_NORMAL    256

// 复杂任务：512字（2048字节）
#define TASK_STACK_SIZE_LARGE    512
```

2. **运行时检查实际使用量**：
```c
void vMyTask(void *pvParameters)
{
    UBaseType_t uxHighWaterMark;
    
    for(;;)
    {
        // 任务代码...
        
        // 检查栈使用情况
        uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
        // 返回值是剩余的栈空间（字）
        // 如果这个值很小（比如<20），说明栈快用完了
    }
}
```

---

### FAQ7：vTaskDelay() 和 vTaskDelayUntil() 有什么区别？

**区别**：

| 特性 | vTaskDelay() | vTaskDelayUntil() |
|-----|-------------|------------------|
| 延时类型 | 相对延时 | 绝对延时 |
| 精度 | 可能漂移 | 精确周期 |
| 适用场景 | 简单延时 | 定期执行 |

**vTaskDelay() - 相对延时**：
```c
// 从调用时刻开始延时
vTaskDelay(pdMS_TO_TICKS(1000));
// 执行时间 + 1000ms
```

**vTaskDelayUntil() - 绝对延时**：
```c
// 精确的周期执行
TickType_t xLastWakeTime = xTaskGetTickCount();

for(;;)
{
    做某事();
    // 从上一次唤醒开始，精确延时1000ms
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
}
```

---

### FAQ8：如何调试任务问题？

**调试工具和方法**：

1. **启用栈溢出检测**：
```c
#define configCHECK_FOR_STACK_OVERFLOW    1

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    // 栈溢出了！
}
```

2. **启用MALLOC失败钩子**：
```c
#define configUSE_MALLOC_FAILED_HOOK    1

void vApplicationMallocFailedHook(void)
{
    // 内存分配失败！
}
```

3. **打印任务状态**：
```c
// 打印所有任务的状态
vTaskList(pcBuffer);
输出字符串(pcBuffer);
```

4. **LED指示**：
```c
// 不同阶段点亮不同LED
vApplicationStackOverflowHook()
{
    LED_RED_ON();  // 红灯表示栈溢出
    for(;;);
}
```

---

### FAQ9：任务优先级如何选择？

**优先级规则**：

| 优先级 | 用途 | 示例 |
|-------|------|------|
| 高 (3-4) | 紧急任务 | 安全检测、通信中断处理 |
| 中 (2) | 普通任务 | 数据处理、UI更新 |
| 低 (1) | 后台任务 | 日志记录、统计 |
| 0 | 空闲任务 | （系统使用，不要用） |

**注意**：
- 不要让太多任务都是高优先级
- 相同优先级的任务会轮流执行（时间片）
- 高优先级任务会抢占低优先级任务

---

## 快速问题排查流程

### 遇到问题时，按这个顺序检查：

1. **任务是否创建成功？**
  ```c
  if(xTaskCreate(...) != pdPASS)
  {
      // 检查堆内存、栈大小、优先级
  }
  ```

2. **调度器是否启动成功？**
  ```c
  vTaskStartScheduler();
  // 如果到这里，说明失败了
  // 检查堆内存大小
  ```

3. **任务函数是否正确？**
  - 有无限循环 `for(;;)` 吗？
  - 有让出CPU的操作吗？（`vTaskDelay()` 等）
  - 函数返回了吗？

4. **有没有串口输出？**
  - 串口初始化了吗？
  - 任务优先级是0吗？（太低了）
  - 栈溢出了吗？

5. **启用调试钩子**
  - 栈溢出钩子
  - Malloc失败钩子
  - 空闲钩子

---

## 总结

### 最常见问题Top 3

1. **任务创建/调度器启动失败**
  - 👉 检查 `configTOTAL_HEAP_SIZE`，至少10KB

2. **任务只运行一次**
  - 👉 检查任务函数是否有无限循环 `for(;;)`

3. **其他任务无法运行**
  - 👉 检查是否有任务一直占用CPU（没有 `vTaskDelay()`）

### 遇到问题时

1. 先看本文档的FAQ
2. 启用调试钩子
3. 用LED或串口输出调试信息
4. 检查FreeRTOS配置

---

*参考资料：FreeRTOS官方文档 - 常见问题*