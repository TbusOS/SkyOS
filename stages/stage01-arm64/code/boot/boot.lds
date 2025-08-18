/*
 * SkymOS ARM64 链接脚本
 * 文件: boot/boot.lds
 * 
 * 定义ARM64内核的内存布局：
 * - 异常向量表在0x40000000 (QEMU virt machine的入口)
 * - 代码段、数据段、BSS段的安排
 */

ENTRY(_start)

/* 内存布局 - QEMU virt machine ARM64 */
MEMORY
{
    /* RAM: 256MB starting at 0x40000000 */
    RAM (rwx) : ORIGIN = 0x40000000, LENGTH = 256M
}

/* 段定义 */
SECTIONS
{
    /* 内核加载地址 */
    . = 0x40000000;
    
    /* 异常向量表 - 必须在最开始 */
    .vectors : {
        KEEP(*(.vectors))
    } > RAM
    
    /* 代码段 */
    .text : {
        *(.text*)
        *(.rodata*)
        . = ALIGN(8);
    } > RAM
    
    /* 数据段 */
    .data : {
        *(.data*)
        . = ALIGN(8);
    } > RAM
    
    /* BSS段 (未初始化数据) */
    .bss : {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(8);
        __bss_end = .;
    } > RAM
    
    /* 堆栈和堆区域 */
    .heap : {
        __heap_start = .;
        . += 1M;  /* 1MB堆空间 */
        __heap_end = .;
    } > RAM
    
    /* 内核结束位置 */
    __kernel_end = .;
    
    /* 丢弃不需要的段 */
    /DISCARD/ : {
        *(.comment)
        *(.note*)
        *(.ARM.attributes)
        *(.eh_frame*)
    }
} 