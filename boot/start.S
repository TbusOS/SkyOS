/*
 * SkyOS ARM32 启动代码
 * 文件: boot/start.S
 * 
 * 这是SkyOS的第一个执行的代码，负责：
 * 1. 设置异常向量表
 * 2. 初始化堆栈
 * 3. 跳转到C语言main函数
 */

.section .vectors, "ax"
.global _vectors
_vectors:
    ldr pc, =reset_handler      @ 0x00: Reset
    ldr pc, =undef_handler      @ 0x04: Undefined Instruction
    ldr pc, =swi_handler        @ 0x08: Software Interrupt (SVC)
    ldr pc, =prefetch_handler   @ 0x0C: Prefetch Abort
    ldr pc, =data_handler       @ 0x10: Data Abort
    nop                         @ 0x14: Reserved
    ldr pc, =irq_handler        @ 0x18: IRQ
    ldr pc, =fiq_handler        @ 0x1C: FIQ

.section .text
.global _start

_start:
reset_handler:
    @ 禁用中断
    cpsid if
    
    @ 设置各种模式下的栈指针
    @ SVC模式 (Supervisor)
    msr cpsr, #0x13     @ SVC mode, IRQ/FIQ disabled
    ldr sp, =svc_stack_top
    
    @ IRQ模式
    msr cpsr, #0x12     @ IRQ mode, IRQ/FIQ disabled  
    ldr sp, =irq_stack_top
    
    @ FIQ模式
    msr cpsr, #0x11     @ FIQ mode, IRQ/FIQ disabled
    ldr sp, =fiq_stack_top
    
    @ Abort模式
    msr cpsr, #0x17     @ Abort mode, IRQ/FIQ disabled
    ldr sp, =abort_stack_top
    
    @ Undefined模式
    msr cpsr, #0x1B     @ Undefined mode, IRQ/FIQ disabled
    ldr sp, =undef_stack_top
    
    @ 回到SVC模式
    msr cpsr, #0x13     @ SVC mode, IRQ/FIQ disabled
    
    @ 清空BSS段
    ldr r0, =__bss_start
    ldr r1, =__bss_end
    mov r2, #0
bss_clear_loop:
    cmp r0, r1
    bge bss_clear_done
    str r2, [r0], #4
    b bss_clear_loop
bss_clear_done:

    @ 调用C语言main函数
    bl main
    
    @ 如果main返回，进入无限循环
hang:
    wfi                 @ Wait For Interrupt
    b hang

/*
 * 异常处理程序
 * 在教学的早期阶段，这些只是简单的死循环
 * 后续课程会逐步完善这些处理程序
 */
undef_handler:
    @ TODO: 实现未定义指令异常处理
    b hang

swi_handler:
    @ TODO: 实现系统调用处理
    b hang

prefetch_handler:
    @ TODO: 实现预取指令异常处理
    b hang

data_handler:
    @ TODO: 实现数据访问异常处理
    b hang

irq_handler:
    @ TODO: 实现IRQ中断处理
    b hang

fiq_handler:
    @ TODO: 实现FIQ中断处理
    b hang

/*
 * 堆栈空间定义
 * 为各种ARM处理器模式分配独立的堆栈空间
 */
.section .bss
.align 3

    .space 4096
svc_stack_top:      @ Supervisor模式堆栈

    .space 1024  
irq_stack_top:      @ IRQ模式堆栈

    .space 1024
fiq_stack_top:      @ FIQ模式堆栈

    .space 1024
abort_stack_top:    @ Abort模式堆栈

    .space 1024
undef_stack_top:    @ Undefined模式堆栈 