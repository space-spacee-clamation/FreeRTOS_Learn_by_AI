/*
 * STM32F4xx HAL 基础配置文件
 * 用于QEMU仿真环境验证
 */

#ifndef STM32F4xx_HAL_CONF_H
#define STM32F4xx_HAL_CONF_H

/* 配置HAL模块 - 我们只需要基础功能，不需要很多外设驱动 */
#define HAL_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED

/* 系统时钟配置 - STM32F4-Discovery在QEMU中使用168MHz */
#if !defined  (HSE_VALUE)
#define HSE_VALUE    ((uint32_t)8000000) /* 外部晶振8MHz */
#endif /* HSE_VALUE */

#if !defined  (HSI_VALUE)
  #define HSI_VALUE      ((uint32_t)16000000)
#endif /* HSI_VALUE */

/* 系统核心频率 */
#define VECT_TAB_OFFSET  0x00

/* 配置断言 */
#ifdef  USE_FULL_ASSERT
#define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))
void assert_failed(uint8_t* file, uint32_t line);
#else
#define assert_param(expr) ((void)0)
#endif /* USE_FULL_ASSERT */

#endif /* STM32F4xx_HAL_CONF_H */
