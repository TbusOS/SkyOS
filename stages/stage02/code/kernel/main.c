/*
 * SkyOS ARM32 主函数 - 阶段2：异常处理与中断
 * 文件: kernel/main.c
 * 
 * 这是内核的C语言入口点，在ARM汇编启动代码完成后调用
 * 功能：
 * 1. 初始化UART串口
 * 2. 演示异常处理机制
 * 3. 测试系统调用功能
 * 4. 初始化定时器和GIC
 * 5. 基础的内核主循环
 */

#include <stdint.h>

/* QEMU virt machine UART0 基址 */
#define UART0_BASE      0x09000000
#define UART_DR         (UART0_BASE + 0x00)    /* 数据寄存器 */
#define UART_FR         (UART0_BASE + 0x18)    /* 标志寄存器 */
#define UART_FR_TXFF    (1 << 5)               /* 发送FIFO满 */

/* 简单的寄存器读写宏 */
#define REG(addr) (*(volatile uint32_t*)(addr))

/* 外部函数声明 */
extern void test_syscalls(void);
extern void test_exceptions(void);
extern void print_exception_stats(void);
extern void print_syscall_stats(void);
extern void enable_irq(void);
extern void disable_irq(void);

/* 定时器和GIC函数声明 */
extern void timer_init(void);
extern void gic_init(void);
extern void timer_print_status(void);
extern void gic_print_status(void);
extern void gic_print_interrupt_stats(void);
extern void gic_print_version_info(void);
extern void timer_delay_ms(uint32_t milliseconds);
extern uint32_t timer_get_interrupt_count(void);

/* UART输出字符函数 */
void uart_putc(char c) {
    /* 等待发送FIFO不满 */
    while (REG(UART_FR) & UART_FR_TXFF) {
        /* 空等待 */
    }
    
    /* 发送字符 */
    REG(UART_DR) = c;
}

/* UART输出字符串函数 */
void uart_puts(const char *str) {
    while (*str) {
        uart_putc(*str++);
    }
}

/* 输出十六进制数字 */
void uart_put_hex(uint32_t value) {
    const char hex_chars[] = "0123456789ABCDEF";
    
    uart_puts("0x");
    for (int i = 28; i >= 0; i -= 4) {
        uart_putc(hex_chars[(value >> i) & 0xF]);
    }
}

/* 获取ARM处理器ID */
uint32_t get_processor_id(void) {
    uint32_t id;
    asm volatile("mrc p15, 0, %0, c0, c0, 0" : "=r"(id));
    return id;
}

/* 获取当前程序状态寄存器(CPSR) */
uint32_t get_cpsr(void) {
    uint32_t cpsr;
    asm volatile("mrs %0, cpsr" : "=r"(cpsr));
    return cpsr;
}

/* 获取当前处理器模式 */
const char* get_processor_mode(uint32_t cpsr) {
    switch (cpsr & 0x1F) {
        case 0x10: return "User";
        case 0x11: return "FIQ";
        case 0x12: return "IRQ";
        case 0x13: return "Supervisor";
        case 0x17: return "Abort";
        case 0x1B: return "Undefined";
        case 0x1F: return "System";
        default:   return "Unknown";
    }
}

/* 延时函数 */
void delay(uint32_t count) {
    for (volatile uint32_t i = 0; i < count; i++) {
        /* 空循环 */
    }
}

/* 获取定时器滴答数 (定时器模块实现) */
uint32_t get_timer_ticks(void);

/* 演示中断控制 */
void demo_interrupt_control(void) {
    uart_puts("\r\n=== 中断控制演示 ===\r\n");
    
    uint32_t cpsr_before = get_cpsr();
    uart_puts("禁用前CPSR: ");
    uart_put_hex(cpsr_before);
    uart_puts(" (IRQ ");
    uart_puts((cpsr_before & 0x80) ? "禁用" : "启用");
    uart_puts(")\r\n");
    
    /* 禁用中断 */
    disable_irq();
    uint32_t cpsr_after_disable = get_cpsr();
    uart_puts("禁用后CPSR: ");
    uart_put_hex(cpsr_after_disable);
    uart_puts(" (IRQ ");
    uart_puts((cpsr_after_disable & 0x80) ? "禁用" : "启用");
    uart_puts(")\r\n");
    
    /* 启用中断 */
    enable_irq();
    uint32_t cpsr_after_enable = get_cpsr();
    uart_puts("启用后CPSR: ");
    uart_put_hex(cpsr_after_enable);
    uart_puts(" (IRQ ");
    uart_puts((cpsr_after_enable & 0x80) ? "禁用" : "启用");
    uart_puts(")\r\n");
    
    uart_puts("===================\r\n");
}

/* 演示处理器模式信息 */
void demo_processor_modes(void) {
    uart_puts("\r\n=== 处理器模式信息 ===\r\n");
    
    uint32_t processor_id = get_processor_id();
    uint32_t cpsr = get_cpsr();
    
    uart_puts("处理器ID: ");
    uart_put_hex(processor_id);
    uart_puts("\r\n");
    
    uart_puts("当前模式: ");
    uart_puts(get_processor_mode(cpsr));
    uart_puts(" (CPSR: ");
    uart_put_hex(cpsr);
    uart_puts(")\r\n");
    
    uart_puts("中断状态:\r\n");
    uart_puts("  IRQ: ");
    uart_puts((cpsr & (1 << 7)) ? "禁用" : "启用");
    uart_puts("\r\n");
    uart_puts("  FIQ: ");
    uart_puts((cpsr & (1 << 6)) ? "禁用" : "启用");
    uart_puts("\r\n");
    
    uart_puts("====================\r\n");
}

/* 测试定时器中断功能 */
void test_timer_interrupt(void) {
    uart_puts("\r\n=== 测试定时器中断 ===\r\n");
    
    uint32_t start_interrupts = timer_get_interrupt_count();
    uart_puts("开始时中断数: ");
    uart_put_hex(start_interrupts);
    uart_puts("\r\n");
    
    uart_puts("等待2秒 (200个10ms定时器中断)...\r\n");
    timer_delay_ms(2000);
    
    uint32_t end_interrupts = timer_get_interrupt_count();
    uart_puts("结束时中断数: ");
    uart_put_hex(end_interrupts);
    uart_puts("\r\n");
    
    uint32_t interrupt_diff = end_interrupts - start_interrupts;
    uart_puts("期间接收中断: ");
    uart_put_hex(interrupt_diff);
    uart_puts(" 个\r\n");
    
    if (interrupt_diff >= 180 && interrupt_diff <= 220) {
        uart_puts("✅ 定时器中断工作正常!\r\n");
    } else {
        uart_puts("❌ 定时器中断异常!\r\n");
    }
    
    uart_puts("====================\r\n");
}

/* 主函数 - 内核入口点 */
int main(void) {
    /* 输出启动信息 */
    uart_puts("\r\n");
    uart_puts("============================================\r\n");
    uart_puts("    SkyOS - 阶段2：异常处理与中断\r\n");
    uart_puts("============================================\r\n");
    uart_puts("版本: 0.2.0 (完整异常处理和中断版)\r\n");
    uart_puts("架构: ARM Cortex-A15 (ARMv7-A)\r\n");
    uart_puts("平台: QEMU virt machine\r\n");
    uart_puts("编译时间: " __DATE__ " " __TIME__ "\r\n");
    uart_puts("--------------------------------------------\r\n");
    
    /* 显示处理器模式信息 */
    demo_processor_modes();
    
    /* 演示中断控制 */
    demo_interrupt_control();
    
    /* 初始化GIC中断控制器 */
    uart_puts("🔧 初始化中断子系统...\r\n");
    gic_init();
    
    /* 初始化ARM Generic Timer */
    timer_init();
    
    /* 显示GIC版本信息 */
    gic_print_version_info();
    
    /* 启用IRQ中断 */
    uart_puts("🔓 启用IRQ中断...\r\n");
    enable_irq();
    
    uart_puts("✅ 中断子系统初始化完成!\r\n");
    uart_puts("--------------------------------------------\r\n");
    
    /* 测试异常处理机制 */
    uart_puts("🧪 测试异常处理机制:\r\n");
    test_exceptions();
    
    /* 测试系统调用 */
    uart_puts("🧪 测试系统调用机制:\r\n");
    test_syscalls();
    
    /* 测试定时器中断 */
    test_timer_interrupt();
    
    uart_puts("--------------------------------------------\r\n");
    uart_puts("🎉 阶段2核心功能演示完成！\r\n");
    uart_puts("============================================\r\n");
    
    /* 显示初始状态 */
    timer_print_status();
    gic_print_status();
    
    /* 主循环 */
    uart_puts("\r\n🚀 开始主程序循环 (按Ctrl+A X退出QEMU):\r\n");
    
    uint32_t counter = 0;
    
    while (1) {
        /* 使用定时器延时而不是忙等待 */
        timer_delay_ms(3000);  /* 3秒间隔 */
        
        counter++;
        
        uart_puts("\r\n💓 主程序心跳 #");
        uart_put_hex(counter);
        uart_puts("\r\n");
        
        /* 每5次心跳显示详细统计信息 */
        if (counter % 5 == 0) {
            print_exception_stats();
            print_syscall_stats();
            gic_print_interrupt_stats();
            timer_print_status();
        } else {
            /* 简单状态显示 */
            uart_puts("  定时器中断数: ");
            uart_put_hex(timer_get_interrupt_count());
            uart_puts("\r\n");
            uart_puts("  运行时间: ");
            uart_put_hex(get_timer_ticks() / 100);
            uart_puts(".");
            uart_put_hex((get_timer_ticks() % 100) / 10);
            uart_puts(" 秒\r\n");
        }
        
        /* 每10次心跳测试一次系统调用 */
        if (counter % 10 == 0) {
            uart_puts("\r\n--- 定期系统调用测试 ---\r\n");
            
            /* 测试获取时间系统调用 */
            uint32_t result;
            asm volatile(
                "mov r0, #4\n"      /* SYS_GETTIME */
                "mov r1, #0\n"
                "svc #0\n"
                "mov %0, r0\n"
                : "=r"(result)
                :
                : "r0", "r1"
            );
            
            uart_puts("当前系统时间: ");
            uart_put_hex(result);
            uart_puts(" 滴答\r\n");
            
            uart_puts("----------------------------\r\n");
        }
        
        /* 每20次心跳显示GIC状态 */
        if (counter % 20 == 0) {
            gic_print_status();
        }
    }
    
    /* 永远不会到达这里 */
    return 0;
} 