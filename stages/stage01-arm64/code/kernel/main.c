#include <stdint.h>

/*
 * SkyOS ARM64版本 - 适配Apple M3芯片
 * 这是一个可以在M3 Mac上作为用户程序运行的教学操作系统
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

void skyos_putc(char c) {
    putchar(c);
    fflush(stdout);
}

void skyos_puts(const char* str) {
    printf("%s", str);
    fflush(stdout);
}

void skyos_put_hex(uint64_t value) {
    printf("0x%016llX", value);
}

void skyos_delay(uint32_t ms) {
    usleep(ms * 1000);
}

int main(void) {
    uint32_t heartbeat = 0;
    
    skyos_puts("======================================\r\n");
    skyos_puts("    SkyOS ARM64 - Apple M3版本\r\n");
    skyos_puts("======================================\r\n");
    skyos_puts("🚀 SkyOS ARM64教学操作系统启动中...\r\n");
    skyos_puts("\r\n");
    
    // 显示处理器信息
    skyos_puts("📱 处理器信息:\r\n");
    skyos_puts("   处理器ID (MIDR_EL1): ");
    skyos_put_hex(get_processor_id());
    skyos_puts("\r\n");
    
    skyos_puts("   当前异常级别: EL");
    skyos_put_hex(get_current_el());
    skyos_puts("\r\n");
    
    skyos_puts("   多处理器ID (MPIDR): ");
    skyos_put_hex(get_mpidr());
    skyos_puts("\r\n");
    
    skyos_puts("\r\n");
    skyos_puts("🎯 ARM64特性演示:\r\n");
    skyos_puts("   - 64位寄存器访问 ✅\r\n");
    skyos_puts("   - 异常级别检查 ✅\r\n");
    skyos_puts("   - 多核处理器检测 ✅\r\n");
    skyos_puts("   - 内联汇编调用 ✅\r\n");
    skyos_puts("\r\n");
    
    skyos_puts("✅ SkyOS ARM64初始化完成！\r\n");
    skyos_puts("🔄 进入心跳循环...\r\n");
    skyos_puts("\r\n");
    
    // 心跳循环 - 模拟操作系统运行
    while (1) {
        skyos_puts("💓 心跳 #");
        skyos_put_hex(heartbeat++);
        skyos_puts(" - SkyOS ARM64在Apple M3上运行! 🍎\r\n");
        
        skyos_delay(1000); // 1秒延时
        
        // 每10次心跳显示一次系统状态
        if (heartbeat % 10 == 0) {
            skyos_puts("📊 系统状态: 运行正常, 当前EL=");
            skyos_put_hex(get_current_el());
            skyos_puts("\r\n");
        }
        
        // 运行60次后退出，避免无限循环
        if (heartbeat >= 60) {
            skyos_puts("\r\n");
            skyos_puts("🎉 SkyOS ARM64演示完成!\r\n");
            skyos_puts("📚 这展示了在M3上运行ARM64汇编的可能性\r\n");
            break;
        }
    }
    
    return 0;
} 