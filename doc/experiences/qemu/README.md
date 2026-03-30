# QEMU 模拟器经验

此目录存放所有与QEMU相关的使用经验：
- 虚拟机配置
- 调试经验
- 网络配置
- 镜像制作
- 性能优化

---

## STM32F103 QEMU 配置经验 ⭐

### 关键发现

在为STM32F103配置QEMU仿真环境时，我们获得了以下重要经验：

#### 1. CORTEX_STM32F103_Keil 项目的参考价值

FreeRTOS官方源码中的 `CORTEX_STM32F103_Keil` 项目是极其重要的参考：

- 提供了STM32F103的标准FreeRTOS配置
- 包含合理的内存配置（72MHz主频，20KB RAM，64KB Flash）
- 展示了正确的启动文件和链接脚本结构
- 提供了经过验证的FreeRTOSConfig.h参数

**重要原则**：在开始任何新项目前，**优先查询FreeRTOS/Demo目录下是否存在相关的参考项目**，这能节省大量时间并避免配置错误。

#### 2. QEMU中STM32F103的实现方案

经过详细分析，我们确定了STM32F103在QEMU中的最佳实现方案：

| 方案 | 机器类型 | 说明 | 状态 |
|------|----------|------|------|
| 方案A | `lm3s6965evb` | LM3S6965（同Cortex-M3内核） | ✅ 可用 |
| 方案B | `olimex-stm32-p103` | Olimex STM32-P103开发板 | ✅ 推荐 |
| 方案C | `stm32vldiscovery` | STM32VLDISCOVERY | ⚠️ 有限支持 |

**最终选择**：使用 `olimex-stm32-p103` 机器类型，因为：
- 直接基于STM32F103RBT6芯片
- 有更准确的外设模拟
- 社区支持较好

#### 3. 配置参数总结

基于CORTEX_STM32F103_Keil项目的推荐配置：

| 配置项 | 值 | 说明 |
|--------|-----|------|
| CPU主频 | 72MHz | STM32F103最大主频 |
| Flash大小 | 64KB+ | STM32F103RB为128KB |
| RAM大小 | 20KB | STM32F103RB为20KB |
| 堆大小 | 10KB | configTOTAL_HEAP_SIZE |
| 最小栈 | 80字 | configMINIMAL_STACK_SIZE |
| 最大优先级 | 5级 | configMAX_PRIORITIES |
| 滴答频率 | 1000Hz | configTICK_RATE_HZ |
| 串口地址 | 0x40013804 | USART1_DR寄存器 |

#### 4. 实际使用的QEMU命令

```bash
# STM32F103 (Olimex STM32-P103)
qemu-system-arm \
  -machine olimex-stm32-p103 \
  -nographic \
  -kernel env-verify-stm32f103.elf \
  -serial mon:stdio

# LM3S6965 (作为备选方案)
qemu-system-arm \
  -machine lm3s6965evb \
  -nographic \
  -kernel env-verify-lm3s6965.elf \
  -serial mon:stdio
```

### 经验总结

1. **参考Demo项目的重要性**：FreeRTOS/Demo目录下的项目经过验证，是最佳参考
2. **芯片兼容性**：同架构（Cortex-M3）的芯片在QEMU中可以互用
3. **逐步验证**：先验证基础环境，再添加复杂功能
4. **多方案准备**：准备备选方案以防主要方案遇到问题

### 相关文档

- [STM32F103 QEMU支持分析报告](./STM32F103_QEMU支持分析报告.md)
- [环境验证项目](../../projects/stm32-learning/01-getting-started/00-env-verify/README.md)
