/*
 * SkymOS ARM64启动代码
 * 文件: boot/start.S
 * 
 * ARM64 (AArch64) 启动汇编代码
 * 功能：
 * 1. 设置异常向量表
 * 2. 初始化各异常级别的堆栈
 * 3. 清零BSS段
 * 4. 跳转到C语言main函数
 */

.section .vectors, "ax"
.global _vectors

/*
 * ARM64异常向量表
 * 每个向量表项占128字节 (0x80)
 * 支持4种异常类型，每种有4个不同的来源
 */
_vectors:
    /* Current EL with SP_EL0 */
    .align 7
    b sync_current_el_sp0      // 0x000: Synchronous
    .align 7  
    b irq_current_el_sp0       // 0x080: IRQ
    .align 7
    b fiq_current_el_sp0       // 0x100: FIQ
    .align 7
    b serror_current_el_sp0    // 0x180: SError
    
    /* Current EL with SP_ELx */
    .align 7
    b sync_current_el_spx      // 0x200: Synchronous
    .align 7
    b irq_current_el_spx       // 0x280: IRQ
    .align 7
    b fiq_current_el_spx       // 0x300: FIQ
    .align 7
    b serror_current_el_spx    // 0x380: SError
    
    /* Lower EL using AArch64 */
    .align 7
    b sync_lower_el_aarch64    // 0x400: Synchronous
    .align 7
    b irq_lower_el_aarch64     // 0x480: IRQ
    .align 7
    b fiq_lower_el_aarch64     // 0x500: FIQ
    .align 7
    b serror_lower_el_aarch64  // 0x580: SError
    
    /* Lower EL using AArch32 */
    .align 7
    b sync_lower_el_aarch32    // 0x600: Synchronous
    .align 7
    b irq_lower_el_aarch32     // 0x680: IRQ
    .align 7
    b fiq_lower_el_aarch32     // 0x700: FIQ
    .align 7
    b serror_lower_el_aarch32  // 0x780: SError

.section .text
.global _start

_start:
    /* 禁用中断 */
    msr daifset, #0xf
    
    /* 设置异常向量表基址 */
    adr x0, _vectors
    msr vbar_el1, x0
    
    /* 设置堆栈指针 */
    adr x0, el1_stack_top
    mov sp, x0
    
    /* 清零BSS段 */
    adr x0, __bss_start
    adr x1, __bss_end
    mov x2, #0
    
bss_clear_loop:
    cmp x0, x1
    bhs bss_clear_done
    str x2, [x0], #8
    b bss_clear_loop
    
bss_clear_done:
    /* 跳转到C语言main函数 */
    bl main
    
    /* 如果main返回，进入死循环 */
hang:
    wfi
    b hang

/* 异常处理程序（简化版本） */
sync_current_el_sp0:
    b hang

irq_current_el_sp0:
    b hang

fiq_current_el_sp0:
    b hang

serror_current_el_sp0:
    b hang

sync_current_el_spx:
    b hang

irq_current_el_spx:
    b hang

fiq_current_el_spx:
    b hang

serror_current_el_spx:
    b hang

sync_lower_el_aarch64:
    b hang

irq_lower_el_aarch64:
    b hang

fiq_lower_el_aarch64:
    b hang

serror_lower_el_aarch64:
    b hang

sync_lower_el_aarch32:
    b hang

irq_lower_el_aarch32:
    b hang

fiq_lower_el_aarch32:
    b hang

serror_lower_el_aarch32:
    b hang

/* 堆栈区域 */
.section .bss
.align 4
el1_stack_bottom:
    .space 4096  // 4KB堆栈
el1_stack_top: 