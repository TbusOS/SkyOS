/*
 * SkyOS ARM32 启动代码 - 阶段2：异常处理与中断
 * 文件: boot/start.S
 * 
 * 这是SkyOS的第一个执行的代码，负责：
 * 1. 设置异常向量表
 * 2. 初始化堆栈
 * 3. 实现完整的异常处理程序
 * 4. 跳转到C语言main函数
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
 * 完整的异常处理程序框架
 * 每个异常都有独立的处理程序
 */

undef_handler:
    @ 保存上下文到Undefined模式栈
    stmfd sp!, {r0-r12, lr}
    
    @ 调用C语言异常处理函数
    mov r0, sp          @ 传递寄存器帧指针
    bl handle_undefined_instruction
    
    @ 恢复上下文并返回
    ldmfd sp!, {r0-r12, pc}^

swi_handler:
    @ 保存上下文到SVC模式栈 (注意：SVC模式下lr指向返回地址)
    stmfd sp!, {r0-r12, lr}
    
    @ 获取SVC指令中的立即数
    ldr r0, [lr, #-4]
    bic r0, r0, #0xFF000000
    
    @ 从栈中获取寄存器参数
    mov r1, sp
    
    @ 调用系统调用处理函数
    bl handle_swi
    
    @ 恢复上下文并返回
    ldmfd sp!, {r0-r12, pc}^

prefetch_handler:
    @ 保存上下文到Abort模式栈
    stmfd sp!, {r0-r12, lr}
    
    @ 调用C语言异常处理函数
    mov r0, sp          @ 传递寄存器帧指针
    bl handle_prefetch_abort
    
    @ 恢复上下文并返回
    ldmfd sp!, {r0-r12, pc}^

data_handler:
    @ 保存上下文到Abort模式栈
    stmfd sp!, {r0-r12, lr}
    
    @ 调用C语言异常处理函数
    mov r0, sp          @ 传递寄存器帧指针
    bl handle_data_abort
    
    @ 恢复上下文并返回
    ldmfd sp!, {r0-r12, pc}^

irq_handler:
    @ 保存上下文到IRQ模式栈
    sub lr, lr, #4          @ 调整返回地址
    stmfd sp!, {r0-r12, lr}
    
    @ 调用C语言IRQ处理函数
    bl handle_irq
    
    @ 恢复上下文并返回
    ldmfd sp!, {r0-r12, pc}^

fiq_handler:
    @ FIQ有独立的寄存器组，处理更快
    stmfd sp!, {r0-r7, lr}
    
    @ 调用C语言FIQ处理函数
    bl handle_fiq
    
    @ 恢复上下文并返回
    ldmfd sp!, {r0-r7, pc}^

/*
 * 中断控制函数
 */
.global enable_irq
enable_irq:
    mrs r0, cpsr
    bic r0, r0, #0x80    @ 清除IRQ禁用位
    msr cpsr_c, r0
    bx lr

.global disable_irq
disable_irq:
    mrs r0, cpsr
    orr r0, r0, #0x80    @ 设置IRQ禁用位
    msr cpsr_c, r0
    bx lr

.global enable_fiq
enable_fiq:
    mrs r0, cpsr
    bic r0, r0, #0x40    @ 清除FIQ禁用位
    msr cpsr_c, r0
    bx lr

.global disable_fiq
disable_fiq:
    mrs r0, cpsr
    orr r0, r0, #0x40    @ 设置FIQ禁用位
    msr cpsr_c, r0
    bx lr

/*
 * 堆栈空间定义
 * 为各种ARM处理器模式分配独立的堆栈空间
 */
.section .bss
.align 3

    .space 4096
svc_stack_top:      @ Supervisor模式堆栈

    .space 2048  
irq_stack_top:      @ IRQ模式堆栈

    .space 1024
fiq_stack_top:      @ FIQ模式堆栈

    .space 1024
abort_stack_top:    @ Abort模式堆栈

    .space 1024
undef_stack_top:    @ Undefined模式堆栈 