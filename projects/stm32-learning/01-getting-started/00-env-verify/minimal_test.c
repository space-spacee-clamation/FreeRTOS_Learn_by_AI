/*
 * 最小测试程序 - 仅测试串口输出
 */

#include <stdint.h>

/* 链接脚本定义的栈 */
extern uint32_t _estack;

/* STM32 串口和时钟寄存器 */
#define RCC_APB2ENR     (*(volatile uint32_t *)0x40021018)
#define USART1_SR       (*(volatile uint32_t *)0x40013800)
#define USART1_DR       (*(volatile uint32_t *)0x40013804)
#define USART1_BRR      (*(volatile uint32_t *)0x40013808)
#define USART1_CR1      (*(volatile uint32_t *)0x4001380C)
#define USART1_CR2      (*(volatile uint32_t *)0x40013810)
#define USART1_CR3      (*(volatile uint32_t *)0x40013814)

#define TXE_FLAG        (1 << 7)
#define UE_FLAG         (1 << 13)
#define TE_FLAG         (1 << 3)

void uart_init(void)
{
    /* 使能 GPIOA 和 USART1 时钟 */
    RCC_APB2ENR |= (1 << 2) | (1 << 14);
    
    /* 配置串口：115200 8N1 */
    USART1_BRR = 0x271;  /* 72MHz / 115200 = 625 = 0x271 */
    USART1_CR1 = UE_FLAG | TE_FLAG;
}

void uart_send_char(char c)
{
    while (!(USART1_SR & TXE_FLAG));
    USART1_DR = (uint32_t)c;
}

void uart_send_string(const char *str)
{
    while (*str) {
        uart_send_char(*str++);
    }
}

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
    uart_init();
    
    uart_send_string("\r\n=== STM32F103 最小测试 ===\r\n");
    uart_send_string("串口初始化成功！\r\n");
    
    int count = 0;
    while (1) {
        char msg[32];
        msg[0] = 'C';
        msg[1] = 'o';
        msg[2] = 'u';
        msg[3] = 'n';
        msg[4] = 't';
        msg[5] = ':';
        msg[6] = ' ';
        msg[7] = '0' + (count / 100) % 10;
        msg[8] = '0' + (count / 10) % 10;
        msg[9] = '0' + count % 10;
        msg[10] = '\r';
        msg[11] = '\n';
        msg[12] = '\0';
        
        uart_send_string(msg);
        count++;
        
        delay(1000000);
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
