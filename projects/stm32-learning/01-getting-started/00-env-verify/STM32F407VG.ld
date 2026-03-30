/*
 * STM32F407VG 链接脚本
 * 适用于QEMU STM32F4-discovery仿真
 */

MEMORY
{
    FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = 1024K
    RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 192K
    CCMRAM (rw)     : ORIGIN = 0x10000000, LENGTH = 64K
}

/* 入口点 - 复位中断向量 */
ENTRY(Reset_Handler)

SECTIONS
{
    /* 向量表 - 放在Flash最开头 */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    /* 代码段 */
    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
        _etext = .;
    } > FLASH

    /* 只读数据段 */
    .rodata :
    {
        . = ALIGN(4);
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    /* 可读写数据段 - 初始化的数据 */
    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT> FLASH

    /* BSS段 - 未初始化的数据 */
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        __bss_start__ = _sbss;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
        __bss_end__ = _ebss;
    } > RAM

    /* 堆 - FreeRTOS需要使用堆内存 */
    .heap :
    {
        . = ALIGN(8);
        __heap_start__ = .;
        . += 0x1000; /* 预留最小堆空间，实际由FreeRTOS配置 */
        __heap_end__ = .;
        . = ALIGN(8);
    } > RAM

    /* 栈 */
    .stack :
    {
        . = ALIGN(8);
        __stack_top__ = .;
        . += 0x4000; /* 16KB栈空间 */
        __stack_bottom__ = .;
        . = ALIGN(8);
    } > RAM

    /* 提供给链接器的必要符号 */
    PROVIDE(heap_end = __heap_end__);
    PROVIDE(stack_end = __stack_bottom__);
    PROVIDE(_end = _ebss);
    PROVIDE(end = _end);
}
