/* 极简测试程序 */
#include <stdint.h>

#define UART_TEST_ADDR 0x4000C000  /* 试试LM3S6965的地址 */
#define UART_STM32F1  0x40013800  /* USART1 */
#define UART_STM32F2  0x40004400  /* USART2 */

void SystemInit(void) {}

int main(void)
{
    volatile uint32_t *uart;
    
    /* 尝试多个地址 */
    uart = (volatile uint32_t *)UART_TEST_ADDR;
    *uart = 'H';
    *uart = 'e';
    *uart = 'l';
    *uart = 'l';
    *uart = 'o';
    *uart = '\n';
    
    uart = (volatile uint32_t *)UART_STM32F1;
    *uart = 'A';
    *uart = 'B';
    *uart = 'C';
    *uart = '\n';
    
    uart = (volatile uint32_t *)UART_STM32F2;
    *uart = 'X';
    *uart = 'Y';
    *uart = 'Z';
    *uart = '\n';
    
    while(1);
    return 0;
}
