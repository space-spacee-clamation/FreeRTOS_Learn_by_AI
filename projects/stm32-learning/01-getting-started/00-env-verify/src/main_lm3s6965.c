/*
 * 环境验证测试项目 - 主程序 (LM3S6965版本)
 *
 * 学习重点：
 * 1. 验证开发环境（编译工具链、FreeRTOS源码、QEMU仿真）是否正常工作
 * 2. 创建第一个FreeRTOS任务
 * 3. 通过串口输出验证程序运行
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>

/* FreeRTOS头文件 */
#include <FreeRTOS.h>
#include <task.h>

/* LM3S6965 配置 */
uint32_t SystemCoreClock = 50000000;  /* 50MHz */
#define UART_DR_ADDR   0x4000C000UL    /* UART0 DR寄存器地址 */

/* 任务配置 */
#define BLINK_TASK_STACK_SIZE    configMINIMAL_STACK_SIZE
#define BLINK_TASK_PRIORITY      (tskIDLE_PRIORITY + 1)

/* 函数声明 */
void SystemInit(void);
static void vBlinkTask(void *pvParameters);
void uart_send_string(const char *str);
void vAssertCalled(const char *pcFile, unsigned long ulLine);

/*
 * 系统初始化
 * 这个函数在启动文件中被调用，做基本的系统初始化
 */
void SystemInit(void)
{
    /* 对于QEMU仿真，我们不需要复杂的初始化 */
    /* QEMU已经处理了时钟配置和向量表 */
}

/*
 * 简单的串口输出函数
 */
void uart_send_string(const char *str)
{
    volatile uint32_t *const uart_dr = (volatile uint32_t *)UART_DR_ADDR;

    while (*str) {
        *uart_dr = (uint32_t)(*str++);
    }
}

/*
 * 闪烁任务
 * 因为QEMU中没有实际LED，我们通过串口输出来模拟闪烁效果
 */
static void vBlinkTask(void *pvParameters)
{
    (void)pvParameters;
    int count = 0;

    for (;;) {
        /* 输出闪烁信息 */
        char msg[64];
        sprintf(msg, "Blink: %d - FreeRTOS运行正常!\r\n", count++);
        uart_send_string(msg);

        /* 延迟500ms，相当于每秒闪烁两次 */
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/*
 * 主函数 - 程序入口
 */
int main(void)
{
    /* 输出欢迎信息 */
    uart_send_string("\r\n====================================\r\n");
    uart_send_string("  LM3S6965 FreeRTOS 环境验证测试\r\n");
    uart_send_string("====================================\r\n\r\n");

    uart_send_string("系统初始化完成，创建测试任务...\r\n");

    /* 创建闪烁任务 */
    BaseType_t xReturn = xTaskCreate(
        vBlinkTask,          /* 任务函数 */
        "Blink",             /* 任务名称，用于调试 */
        BLINK_TASK_STACK_SIZE, /* 栈大小，以字为单位 */
        NULL,                /* 任务参数 */
        BLINK_TASK_PRIORITY, /* 任务优先级 */
        NULL                 /* 不需要保存任务句柄 */
    );

    if (xReturn != pdPASS) {
        uart_send_string("错误：任务创建失败!\r\n");
        while (1);
    }

    uart_send_string("任务创建成功，启动FreeRTOS调度器...\r\n\r\n");

    /* 启动调度器，这不会返回 */
    vTaskStartScheduler();

    /* 如果启动失败会到这里 */
    uart_send_string("错误：无法启动调度器!\r\n");

    /* 永远循环 */
    while (1);
}

/* 可选：如果需要堆统计，可以在这里添加 */
#if (configUSE_MALLOC_FAILED_HOOK == 1)
void vApplicationMallocFailedHook(void)
{
    uart_send_string("内存分配失败!\r\n");
    while (1);
}
#endif

#if (configCHECK_FOR_STACK_OVERFLOW > 0)
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
    (void)pxTask;
    uart_send_string("栈溢出检测: ");
    uart_send_string(pcTaskName);
    uart_send_string("\r\n");
    while (1);
}
#endif

/*
 * FreeRTOS断言失败处理函数
 */
void vAssertCalled(const char *pcFile, unsigned long ulLine)
{
    char msg[128];
    sprintf(msg, "断言失败: 文件 %s, 行 %lu\r\n", pcFile, ulLine);
    uart_send_string(msg);
    while (1);
}

/*
 * 系统调用存根 - 用于满足newlib C库要求
 */
#include <sys/types.h>
#include <sys/stat.h>
int _sbrk(int incr);
int _close(int file);
int _fstat(int file, struct stat *st);
int _isatty(int file);
int _lseek(int file, int ptr, int dir);
int _read(int file, char *ptr, int len);
int _write(int file, char *ptr, int len);
void _exit(int status);
int _kill(int pid, int sig);
int _getpid(void);

/* 堆结束位置，由链接脚本定义 */
extern int end;
static int *heap_end = (int *)&end;

int _sbrk(int incr)
{
    int *prev_heap_end = heap_end;
    heap_end += incr;
    return (int)prev_heap_end;
}

int _close(int file)
{
    (void)file;
    return -1;
}

int _fstat(int file, struct stat *st)
{
    (void)file;
    (void)st;
    return 0;
}

int _isatty(int file)
{
    (void)file;
    return 1;
}

int _lseek(int file, int ptr, int dir)
{
    (void)file;
    (void)ptr;
    (void)dir;
    return 0;
}

int _read(int file, char *ptr, int len)
{
    (void)file;
    (void)ptr;
    (void)len;
    return 0;
}

int _write(int file, char *ptr, int len)
{
    (void)file;
    volatile uint32_t *const uart_dr = (volatile uint32_t *)UART_DR_ADDR;
    for (int i = 0; i < len; i++) {
        *uart_dr = (uint32_t)ptr[i];
    }
    return len;
}

void _exit(int status)
{
    (void)status;
    while (1);
}

int _kill(int pid, int sig)
{
    (void)pid;
    (void)sig;
    return -1;
}

int _getpid(void)
{
    return 1;
}
