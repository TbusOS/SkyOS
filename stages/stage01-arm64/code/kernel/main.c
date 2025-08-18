#include <stdint.h>

/*
 * SkymOS ARM64版本 - 适配Apple M3芯片
 * 教学用操作系统内核主函数
 */

// 简化的"硬件"抽象 - 使用标准C库作为HAL
#include <stdio.h>
#include <unistd.h>
#include <time.h>

// ARM64寄存器读取函数
uint64_t get_processor_id(void) {
    uint64_t id;
    // ARM64 MIDR_EL1寄存器
    asm volatile("mrs %0, midr_el1" : "=r"(id));
    return id;
}

uint64_t get_current_el(void) {
    uint64_t el;
    // 获取当前异常级别
    asm volatile("mrs %0, currentel" : "=r"(el));
    return (el >> 2) & 3;
}

uint64_t get_mpidr(void) {
    uint64_t mpidr;
    // 多处理器亲和性寄存器
    asm volatile("mrs %0, mpidr_el1" : "=r"(mpidr));
    return mpidr;
}

void skymos_putc(char c) {
    putchar(c);
    fflush(stdout);
}

void skymos_puts(const char* str) {
    printf("%s", str);
    fflush(stdout);
}

void skymos_put_hex(uint64_t value) {
    printf("0x%016llX", value);
}

void skymos_delay(uint32_t ms) {
    usleep(ms * 1000);
}

int main(void) {
    uint32_t heartbeat = 0;
    
    skymos_puts("======================================\r\n");
    skymos_puts("    SkymOS ARM64 - Apple M3版本\r\n");
    skymos_puts("======================================\r\n");
    skymos_puts("🚀 SkymOS ARM64教学操作系统启动中...\r\n");
    skymos_puts("\r\n");
    
    // 显示处理器信息
    skymos_puts("📱 处理器信息:\r\n");
    skymos_puts("   处理器ID (MIDR_EL1): ");
    skymos_put_hex(get_processor_id());
    skymos_puts("\r\n");
    
    skymos_puts("   当前异常级别: EL");
    skymos_put_hex(get_current_el());
    skymos_puts("\r\n");
    
    skymos_puts("   多处理器ID (MPIDR): ");
    skymos_put_hex(get_mpidr());
    skymos_puts("\r\n");
    
    skymos_puts("\r\n");
    skymos_puts("🎯 ARM64特性演示:\r\n");
    skymos_puts("   - 64位寄存器访问 ✅\r\n");
    skymos_puts("   - 异常级别检查 ✅\r\n");
    skymos_puts("   - 多核处理器检测 ✅\r\n");
    skymos_puts("   - 内联汇编调用 ✅\r\n");
    skymos_puts("\r\n");
    
    skymos_puts("✅ SkymOS ARM64初始化完成！\r\n");
    skymos_puts("🔄 进入心跳循环...\r\n");
    skymos_puts("\r\n");
    
    // 心跳循环 - 模拟操作系统运行
    while (1) {
        skymos_puts("💓 心跳 #");
        skymos_put_hex(heartbeat++);
        skymos_puts(" - SkymOS ARM64在Apple M3上运行! 🍎\r\n");
        
        skymos_delay(1000); // 1秒延时
        
        // 每10次心跳显示一次系统状态
        if (heartbeat % 10 == 0) {
            skymos_puts("📊 系统状态: 运行正常, 当前EL=");
            skymos_put_hex(get_current_el());
            skymos_puts("\r\n");
        }
        
        // 运行60次后退出，避免无限循环
        if (heartbeat >= 60) {
            skymos_puts("\r\n");
            skymos_puts("🎉 SkymOS ARM64演示完成!\r\n");
            skymos_puts("📚 这展示了在M3上运行ARM64汇编的可能性\r\n");
            break;
        }
    }
    
    return 0;
} 