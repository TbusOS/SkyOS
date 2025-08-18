/*
 * SkymOS ARM32 主函数
 * 这是内核的入口点，从boot.S调用
 */

#include <stdint.h>

/* QEMU virt machine UART0 基址 */
#define UART0_BASE      0x09000000
#define UART_DR         (UART0_BASE + 0x00)    /* 数据寄存器 */
#define UART_FR         (UART0_BASE + 0x18)    /* 标志寄存器 */
#define UART_FR_TXFF    (1 << 5)               /* 发送FIFO满 */

/* 简单的寄存器读写宏 */
#define REG(addr) (*(volatile uint32_t*)(addr))

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

/* 主函数 - 内核入口点 */
int main(void) {
    uint32_t processor_id;
    uint32_t cpsr;
    
    /* 输出启动信息 */
    uart_puts("\r\n");
    uart_puts("======================================\r\n");
    uart_puts("    SkymOS - ARM32 教学操作系统\r\n");
    uart_puts("======================================\r\n");
    uart_puts("版本: 0.1.0 (教学演示版)\r\n");
    uart_puts("架构: ARM Cortex-A15\r\n");
    uart_puts("编译时间: " __DATE__ " " __TIME__ "\r\n");
    uart_puts("--------------------------------------\r\n");
    
    /* 显示处理器信息 */
    processor_id = get_processor_id();
    uart_puts("处理器ID: ");
    uart_put_hex(processor_id);
    uart_puts("\r\n");
    
    /* 显示当前处理器模式 */
    cpsr = get_cpsr();
    uart_puts("当前模式: ");
    uart_puts(get_processor_mode(cpsr));
    uart_puts(" (CPSR: ");
    uart_put_hex(cpsr);
    uart_puts(")\r\n");
    
    /* 检查中断状态 */
    uart_puts("中断状态: ");
    if (cpsr & (1 << 7)) {
        uart_puts("IRQ禁用 ");
    } else {
        uart_puts("IRQ启用 ");
    }
    if (cpsr & (1 << 6)) {
        uart_puts("FIQ禁用");
    } else {
        uart_puts("FIQ启用");
    }
    uart_puts("\r\n");
    
    uart_puts("--------------------------------------\r\n");
    uart_puts("内核初始化完成！\r\n");
    uart_puts("======================================\r\n");
    
    /* 简单的心跳显示 */
    uart_puts("\r\n开始心跳显示 (按Ctrl+A X退出QEMU):\r\n");
    
    uint32_t counter = 0;
    while (1) {
        uart_puts("心跳 #");
        uart_put_hex(counter++);
        uart_puts(" - SkymOS 正在运行!\r\n");
        
        /* 延时约1秒 */
        delay(1000000);
    }
    
    /* 永远不会到达这里 */
    return 0;
} 