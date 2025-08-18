/*
 * SkyOS 异常处理程序
 * 文件: kernel/exception.c
 * 
 * 实现ARM32各种异常的处理程序
 */

#include <stdint.h>

/* 外部函数声明 */
extern void uart_puts(const char *str);
extern void uart_put_hex(uint32_t value);

/* 异常信息结构 */
struct exception_frame {
    uint32_t r0, r1, r2, r3, r4, r5, r6, r7;
    uint32_t r8, r9, r10, r11, r12;
    uint32_t lr;
};

/* 异常统计计数器 */
static uint32_t undef_count = 0;
static uint32_t swi_count = 0;
static uint32_t prefetch_abort_count = 0;
static uint32_t data_abort_count = 0;
static uint32_t irq_count = 0;
static uint32_t fiq_count = 0;

/* 读取ARM协处理器寄存器的函数 */
static inline uint32_t read_dfsr(void) {
    uint32_t val;
    asm volatile("mrc p15, 0, %0, c5, c0, 0" : "=r"(val));
    return val;
}

static inline uint32_t read_far(void) {
    uint32_t val;
    asm volatile("mrc p15, 0, %0, c6, c0, 0" : "=r"(val));
    return val;
}

static inline uint32_t read_ifsr(void) {
    uint32_t val;
    asm volatile("mrc p15, 0, %0, c5, c0, 1" : "=r"(val));
    return val;
}

static inline uint32_t read_ifar(void) {
    uint32_t val;
    asm volatile("mrc p15, 0, %0, c6, c0, 2" : "=r"(val));
    return val;
}

/* 未定义指令异常处理 */
void handle_undefined_instruction(struct exception_frame *frame) {
    undef_count++;
    
    uart_puts("\r\n*** UNDEFINED INSTRUCTION EXCEPTION ***\r\n");
    uart_puts("Exception count: ");
    uart_put_hex(undef_count);
    uart_puts("\r\n");
    
    uart_puts("Registers at exception:\r\n");
    uart_puts("  R0 = "); uart_put_hex(frame->r0); uart_puts("\r\n");
    uart_puts("  R1 = "); uart_put_hex(frame->r1); uart_puts("\r\n");
    uart_puts("  R2 = "); uart_put_hex(frame->r2); uart_puts("\r\n");
    uart_puts("  R3 = "); uart_put_hex(frame->r3); uart_puts("\r\n");
    uart_puts("  PC = "); uart_put_hex(frame->lr); uart_puts("\r\n");
    
    /* 读取指令故障状态寄存器 */
    uint32_t ifsr = read_ifsr();
    uart_puts("  IFSR = "); uart_put_hex(ifsr); uart_puts("\r\n");
    
    uart_puts("System halted due to undefined instruction.\r\n");
    uart_puts("******************************************\r\n");
    
    /* 停止系统 */
    while(1) {
        asm volatile("wfi");
    }
}

/* 数据访问异常处理 */
void handle_data_abort(struct exception_frame *frame) {
    data_abort_count++;
    
    /* 读取故障地址寄存器和故障状态寄存器 */
    uint32_t far = read_far();    /* Fault Address Register */
    uint32_t dfsr = read_dfsr();   /* Data Fault Status Register */
    
    uart_puts("\r\n*** DATA ABORT EXCEPTION ***\r\n");
    uart_puts("Exception count: ");
    uart_put_hex(data_abort_count);
    uart_puts("\r\n");
    
    uart_puts("Fault information:\r\n");
    uart_puts("  Fault Address (FAR): "); uart_put_hex(far); uart_puts("\r\n");
    uart_puts("  Data Fault Status (DFSR): "); uart_put_hex(dfsr); uart_puts("\r\n");
    uart_puts("  PC at fault: "); uart_put_hex(frame->lr); uart_puts("\r\n");
    
    /* 解析故障状态 */
    uint32_t fault_status = dfsr & 0xF;
    uart_puts("  Fault type: ");
    switch (fault_status) {
        case 0x1:
            uart_puts("Alignment fault");
            break;
        case 0x3:
            uart_puts("Access flag fault");
            break;
        case 0x5:
            uart_puts("Translation fault (section)");
            break;
        case 0x7:
            uart_puts("Translation fault (page)");
            break;
        case 0x9:
            uart_puts("Domain fault (section)");
            break;
        case 0xB:
            uart_puts("Domain fault (page)");
            break;
        case 0xD:
            uart_puts("Permission fault (section)");
            break;
        case 0xF:
            uart_puts("Permission fault (page)");
            break;
        default:
            uart_puts("Unknown fault (");
            uart_put_hex(fault_status);
            uart_puts(")");
            break;
    }
    uart_puts("\r\n");
    
    uart_puts("Registers at exception:\r\n");
    uart_puts("  R0 = "); uart_put_hex(frame->r0); uart_puts("\r\n");
    uart_puts("  R1 = "); uart_put_hex(frame->r1); uart_puts("\r\n");
    uart_puts("  R2 = "); uart_put_hex(frame->r2); uart_puts("\r\n");
    
    uart_puts("System halted due to data abort.\r\n");
    uart_puts("********************************\r\n");
    
    /* 停止系统 */
    while(1) {
        asm volatile("wfi");
    }
}

/* 预取指令异常处理 */
void handle_prefetch_abort(struct exception_frame *frame) {
    prefetch_abort_count++;
    
    /* 读取指令故障状态寄存器 */
    uint32_t ifsr = read_ifsr();   /* Instruction Fault Status Register */
    uint32_t ifar = read_ifar();   /* Instruction Fault Address Register */
    
    uart_puts("\r\n*** PREFETCH ABORT EXCEPTION ***\r\n");
    uart_puts("Exception count: ");
    uart_put_hex(prefetch_abort_count);
    uart_puts("\r\n");
    
    uart_puts("Fault information:\r\n");
    uart_puts("  Instruction Fault Address (IFAR): "); uart_put_hex(ifar); uart_puts("\r\n");
    uart_puts("  Instruction Fault Status (IFSR): "); uart_put_hex(ifsr); uart_puts("\r\n");
    uart_puts("  PC at fault: "); uart_put_hex(frame->lr); uart_puts("\r\n");
    
    uart_puts("System halted due to prefetch abort.\r\n");
    uart_puts("************************************\r\n");
    
    /* 停止系统 */
    while(1) {
        asm volatile("wfi");
    }
}

/* FIQ处理程序 */
void handle_fiq(void) {
    fiq_count++;
    
    uart_puts("FIQ #");
    uart_put_hex(fiq_count);
    uart_puts(" received\r\n");
    
    /* FIQ通常用于高优先级、低延迟的中断处理 */
    /* 这里暂时只做简单的计数和打印 */
}

/* 获取异常统计信息 */
void print_exception_stats(void) {
    uart_puts("\r\n=== Exception Statistics ===\r\n");
    uart_puts("Undefined Instructions: "); uart_put_hex(undef_count); uart_puts("\r\n");
    uart_puts("System Calls (SWI): "); uart_put_hex(swi_count); uart_puts("\r\n");
    uart_puts("Prefetch Aborts: "); uart_put_hex(prefetch_abort_count); uart_puts("\r\n");
    uart_puts("Data Aborts: "); uart_put_hex(data_abort_count); uart_puts("\r\n");
    uart_puts("IRQ Interrupts: "); uart_put_hex(irq_count); uart_puts("\r\n");
    uart_puts("FIQ Interrupts: "); uart_put_hex(fiq_count); uart_puts("\r\n");
    uart_puts("============================\r\n");
}

/* 测试异常处理 */
void test_exceptions(void) {
    uart_puts("\r\n=== Testing Exception Handling ===\r\n");
    
    /* 测试未定义指令 (注释掉，避免系统崩溃) */
    /*
    uart_puts("Testing undefined instruction...\r\n");
    asm volatile(".word 0xFFFFFFFF");
    */
    
    /* 测试数据访问异常 (注释掉，避免系统崩溃) */
    /*
    uart_puts("Testing data abort...\r\n");
    volatile uint32_t *invalid_ptr = (uint32_t*)0xFFFFFFFF;
    *invalid_ptr = 0x12345678;
    */
    
    uart_puts("Exception tests are commented out to prevent system halt.\r\n");
    uart_puts("Uncomment in exception.c to test actual exceptions.\r\n");
    uart_puts("===================================\r\n");
} 