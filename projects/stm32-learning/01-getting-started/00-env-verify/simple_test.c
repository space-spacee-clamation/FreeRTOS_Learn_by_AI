/*
 * 最简单的测试程序 - 不依赖任何硬件
 * 只是通过写入特定内存地址来验证程序执行
 */

#include <stdint.h>

/* 测试用的内存地址 - SRAM区域 */
#define TEST_ADDR 0x20001000

/* 链接脚本定义的栈 */
extern uint32_t _estack;

/* 简单的延时函数 */
void delay(volatile int count)
{
    while (count--) {
        __asm__ volatile ("nop");
    }
}

/* 主函数 */
int main(void)
{
    volatile uint32_t *test_ptr = (volatile uint32_t *)TEST_ADDR;
    
    /* 写入测试模式 */
    *test_ptr = 0x12345678;
    delay(100000);
    
    *test_ptr = 0x87654321;
    delay(100000);
    
    *test_ptr = 0xDEADBEEF;
    delay(100000);
    
    /* 循环 */
    while (1) {
        *test_ptr = *test_ptr + 1;
        delay(50000);
    }
    
    return 0;
}

/* 中断向量表 - 最小版本 */
__attribute__((section(".isr_vector")))
void (*const g_pfnVectors[])(void) =
{
    (void (*)(void))(&_estack),
    (void (*)(void))main,
};
