# 04 - vTaskDelay() 使用说明

## 为什么需要任务延时？

### 问题：空循环延时

**裸机编程中常见的写法**：
```c
// ❌ 空循环延时（不好！）
void delay_ms(uint32_t ms)
{
    for(uint32_t i = 0; i < ms * 1000; i++)
    {
        // 什么都不做，只是循环
        __NOP();
    }
}

void main(void)
{
    while(1)
    {
        LED_ON();
        delay_ms(500);  // ❌ 这500ms内CPU一直在空转！
        LED_OFF();
        delay_ms(500);
    }
}
```

**问题**：
- ❌ CPU一直在空转，浪费电力
- ❌ 其他任务无法运行
- ❌ 延时时间不精确（取决于编译器优化）

---

### FreeRTOS的解决方案：vTaskDelay()

**FreeRTOS中的正确写法**：
```c
// ✅ 使用 vTaskDelay()
void vLEDTask(void *pvParameters)
{
    for(;;)
    {
        LED_ON();
        vTaskDelay(pdMS_TO_TICKS(500));  // ✅ 任务进入阻塞状态
        LED_OFF();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
```

**优点**：
- ✅ 任务进入阻塞状态，不占用CPU
- ✅ 其他任务可以运行
- ✅ 延时时间精确（基于系统时钟节拍）

---

## vTaskDelay() 函数详解

### 函数原型

```c
void vTaskDelay(const TickType_t xTicksToDelay);
```

### 参数说明

| 参数 | 类型 | 说明 |
|-----|------|------|
| xTicksToDelay | TickType_t | 延时的系统时钟节拍数 |

---

## 系统时钟节拍

### 什么是时钟节拍？

**类比**：就像音乐的节拍器
- 每隔固定时间"滴答"一次
- FreeRTOS用这个"滴答"来计时

**典型配置**：
```c
// 在 FreeRTOSConfig.h 中配置
#define configTICK_RATE_HZ    1000  // 每秒1000个节拍 = 1ms一个节拍
```

| configTICK_RATE_HZ | 节拍间隔 | 说明 |
|-------------------|---------|------|
| 100 | 10ms | 低精度，省电 |
| 1000 | 1ms | 常用，平衡 |
| 10000 | 0.1ms | 高精度，耗电 |

---

## pdMS_TO_TICKS() 宏

### 作用

将**毫秒**转换为**系统时钟节拍数**。

### 用法

```c
// 延时500毫秒
vTaskDelay(pdMS_TO_TICKS(500));

// 延时1秒
vTaskDelay(pdMS_TO_TICKS(1000));

// 延时10毫秒
vTaskDelay(pdMS_TO_TICKS(10));
```

### 原理

```c
// pdMS_TO_TICKS() 的定义大概是这样的
#define pdMS_TO_TICKS( xTimeInMs ) \
    ( ( TickType_t ) ( ( ( TickType_t ) ( xTimeInMs ) * \
    ( TickType_t ) configTICK_RATE_HZ ) / ( TickType_t ) 1000 ) )
```

**例子**（假设 configTICK_RATE_HZ = 1000）：
```c
pdMS_TO_TICKS(500)  = (500 * 1000) / 1000 = 500个节拍
pdMS_TO_TICKS(1000) = (1000 * 1000) / 1000 = 1000个节拍
```

---

## vTaskDelay() 的工作原理

### 状态转换

```
调用 vTaskDelay(500)
    ↓
任务从"运行态" → "阻塞态"
    ↓
其他任务可以运行
    ↓
500个时钟节拍后
    ↓
任务从"阻塞态" → "就绪态"
    ↓
等待调度器选中运行
```

### 时间线示例

```
时间轴 →
[任务A运行]                    [任务A继续运行]
     ↓                              ↑
调用 vTaskDelay(500)              延时结束
     ↓                              ↑
    [任务B运行][任务C运行]...  [任务A就绪]
```

---

## 使用示例

### 示例1：LED闪烁

```c
void vLEDTask(void *pvParameters)
{
    (void)pvParameters;
    
    for(;;)
    {
        // 点亮LED
        LED_ON();
        
        // 延时500ms
        vTaskDelay(pdMS_TO_TICKS(500));
        
        // 熄灭LED
        LED_OFF();
        
        // 再延时500ms
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
```

---

### 示例2：周期性任务

```c
void vSensorReadTask(void *pvParameters)
{
   (void)pvParameters;
   
   for(;;)
   {
       // 读取传感器
       读取传感器();
       
       // 处理数据
       处理数据();
       
       // 延时100ms再读下一次
       vTaskDelay(pdMS_TO_TICKS(100));
   }
}
```

---

## 注意事项

### 1. 最小延时时间

```c
// ❌ 不要这样：延时太短
vTaskDelay(1);  // 只延时1个节拍，可能几乎立即返回

// ✅ 推荐：用合理的延时时间
vTaskDelay(pdMS_TO_TICKS(10));  // 至少10ms
```

### 2. 延时精度

- `vTaskDelay()` 基于系统时钟节拍
- 精度取决于 `configTICK_RATE_HZ`
- 如果configTICK_RATE_HZ = 1000，则精度为±1ms

### 3. 不要在中断中调用

```c
// ❌ 错误：中断中不能调用 vTaskDelay()
void 某中断处理函数(void)
{
   vTaskDelay(10);  // 不行！
}

// ✅ 正确：中断中用 xQueueSendFromISR() 等
```

### 4. 空闲任务也需要运行

```c
// ❌ 不要让所有任务都一直延时很短
for(;;)
{
   做某事();
   vTaskDelay(1);  // 太短了！
}

// ✅ 合理的延时
for(;;)
{
   做某事();
   vTaskDelay(pdMS_TO_TICKS(10));  // 10ms或更长
}
```

---

## vTaskDelay() vs vTaskDelayUntil()

### 两个延时函数的区别

| 特性 | vTaskDelay() | vTaskDelayUntil() |
|-----|-------------|------------------|
| 延时类型 | 相对延时 | 绝对延时 |
| 精度 | 可能漂移 | 精确周期 |
| 适用场景 | 简单延时 | 定期执行 |

### vTaskDelay() - 相对延时

```c
// 从调用时刻开始计算延时
vTaskDelay(pdMS_TO_TICKS(1000));
// 执行时间 + 1000ms
```

**可能有漂移**：
```
第1次：执行1ms + 延时1000ms = 1001ms
第2次：执行2ms + 延时1000ms = 1002ms  ← 漂移了！
第3次：执行1ms + 延时1000ms = 1001ms
```

---

### vTaskDelayUntil() - 绝对延时

```c
// 精确的周期性执行
TickType_t xLastWakeTime = xTaskGetTickCount();

for(;;)
{
   做某事();
   
   // 从上一次唤醒开始，精确延时1000ms
   vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
}
```

**没有漂移**：
```
第1次：从0开始，到1000
第2次：从1000开始，到2000  ← 精确！
第3次：从2000开始，到3000  ← 精确！
```

---

## 总结

### 关键点

1. ✅ `vTaskDelay()` 让任务进入阻塞状态
2. ✅ 任务阻塞时，其他任务可以运行
3. ✅ 使用 `pdMS_TO_TICKS()` 转换毫秒为节拍
4. ✅ 不要用空循环延时！
5. ✅ 如果需要精确周期，用 `vTaskDelayUntil()`
6. ✅ 不要在中断中调用 `vTaskDelay()`

### 记忆口诀

> 空转延时太浪费，FreeRTOS有办法
> vTaskDelay来帮你，阻塞期间别人跑
> 毫秒转换用宏包，pdMS_TO_TICKS要记牢
> 定期执行要精确，vTaskDelayUntil更好

### 下一步

现在你会用延时了，接下来学习：
- **[05_调度器启动指南.md](05_调度器启动指南.md)** - 如何启动调度器

---

*参考资料：FreeRTOS官方文档 - 任务延时*