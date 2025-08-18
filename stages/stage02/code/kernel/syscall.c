/*
 * SkyOS 系统调用实现
 * 文件: kernel/syscall.c
 * 
 * 实现SVC(软件中断)异常处理和系统调用机制
 */

#include <stdint.h>
#include <stddef.h>

/* 外部函数声明 */
extern void uart_puts(const char *str);
extern void uart_putc(char c);
extern void uart_put_hex(uint32_t value);
extern uint32_t get_timer_ticks(void);

/* 系统调用号定义 */
#define SYS_INVALID 0
#define SYS_WRITE   1
#define SYS_READ    2
#define SYS_EXIT    3
#define SYS_GETTIME 4
#define SYS_PRINT   5

/* 系统调用参数结构 */
struct syscall_regs {
    uint32_t r0, r1, r2, r3, r4, r5, r6, r7;
    uint32_t r8, r9, r10, r11, r12;
    uint32_t lr;
};

/* 系统调用统计 */
static uint32_t syscall_counts[16] = {0};
static uint32_t total_syscalls = 0;

/* 系统调用：写字符串到标准输出 */
static uint32_t sys_write(uint32_t fd, const char *buf, uint32_t count) {
    if (fd == 1) {  /* stdout */
        uint32_t written = 0;
        for (uint32_t i = 0; i < count; i++) {
            if (buf[i] == '\0') break;  /* 遇到字符串结束符停止 */
            uart_putc(buf[i]);
            written++;
        }
        return written;
    } else if (fd == 2) {  /* stderr */
        uart_puts("[STDERR] ");
        for (uint32_t i = 0; i < count; i++) {
            if (buf[i] == '\0') break;
            uart_putc(buf[i]);
        }
        return count;
    }
    return -1;  /* 无效的文件描述符 */
}

/* 系统调用：从标准输入读取数据 (简化实现) */
static uint32_t sys_read(uint32_t fd, char *buf, uint32_t count) {
    if (fd == 0) {  /* stdin */
        /* 简化实现：暂时不支持实际输入，返回模拟数据 */
        const char *msg = "Hello from kernel input!\n";
        uint32_t len = 0;
        while (msg[len] && len < count - 1) {
            buf[len] = msg[len];
            len++;
        }
        buf[len] = '\0';
        return len;
    }
    return -1;
}

/* 系统调用：退出程序 */
static uint32_t sys_exit(uint32_t exit_code) {
    uart_puts("\r\n=== Program Exit ===\r\n");
    uart_puts("Exit code: ");
    uart_put_hex(exit_code);
    uart_puts("\r\n");
    
    /* 打印系统调用统计 */
    uart_puts("System call statistics:\r\n");
    uart_puts("  Total syscalls: ");
    uart_put_hex(total_syscalls);
    uart_puts("\r\n");
    for (int i = 1; i < 16; i++) {
        if (syscall_counts[i] > 0) {
            uart_puts("  Syscall ");
            uart_put_hex(i);
            uart_puts(": ");
            uart_put_hex(syscall_counts[i]);
            uart_puts(" times\r\n");
        }
    }
    
    uart_puts("===================\r\n");
    
    /* 简单实现：进入死循环 */
    uart_puts("System halted by user exit.\r\n");
    while(1) {
        asm volatile("wfi");
    }
    return 0;
}

/* 系统调用：获取系统时间 */
static uint32_t sys_gettime(void) {
    return get_timer_ticks();
}

/* 系统调用：打印字符串 (便利函数) */
static uint32_t sys_print(const char *str) {
    uint32_t len = 0;
    while (str[len]) len++;  /* 计算字符串长度 */
    return sys_write(1, str, len);
}

/* 系统调用表 */
typedef uint32_t (*syscall_func_t)(uint32_t, uint32_t, uint32_t, uint32_t);

static syscall_func_t syscall_table[] = {
    [SYS_INVALID] = NULL,
    [SYS_WRITE]   = (syscall_func_t)sys_write,
    [SYS_READ]    = (syscall_func_t)sys_read,
    [SYS_EXIT]    = (syscall_func_t)sys_exit,
    [SYS_GETTIME] = (syscall_func_t)sys_gettime,
    [SYS_PRINT]   = (syscall_func_t)sys_print,
    /* 可以继续添加更多系统调用 */
};

#define SYSCALL_COUNT (sizeof(syscall_table) / sizeof(syscall_table[0]))

/* 系统调用名称表 (用于调试) */
static const char* syscall_names[] = {
    [SYS_INVALID] = "invalid",
    [SYS_WRITE]   = "write",
    [SYS_READ]    = "read", 
    [SYS_EXIT]    = "exit",
    [SYS_GETTIME] = "gettime",
    [SYS_PRINT]   = "print",
};

/* SVC异常处理函数 */
void handle_swi(uint32_t syscall_num, struct syscall_regs *regs) {
    uint32_t result = (uint32_t)-1;  /* 默认返回错误 */
    
    /* 增加总的系统调用计数 */
    total_syscalls++;
    
    /* 增加特定系统调用计数 */
    if (syscall_num < 16) {
        syscall_counts[syscall_num]++;
    }
    
    /* 调试输出 */
    uart_puts("SWI #");
    uart_put_hex(syscall_num);
    if (syscall_num < SYSCALL_COUNT && syscall_names[syscall_num]) {
        uart_puts(" (");
        uart_puts(syscall_names[syscall_num]);
        uart_puts(")");
    }
    uart_puts(" called with args: ");
    uart_put_hex(regs->r0);
    uart_puts(", ");
    uart_put_hex(regs->r1);
    uart_puts(", ");
    uart_put_hex(regs->r2);
    uart_puts(", ");
    uart_put_hex(regs->r3);
    uart_puts("\r\n");
    
    /* 检查系统调用号是否有效 */
    if (syscall_num < SYSCALL_COUNT && syscall_table[syscall_num] != NULL) {
        /* 调用对应的系统调用函数 */
        result = syscall_table[syscall_num](regs->r0, regs->r1, regs->r2, regs->r3);
    } else {
        uart_puts("ERROR: Unknown system call number: ");
        uart_put_hex(syscall_num);
        uart_puts("\r\n");
    }
    
    /* 将返回值放入r0寄存器 */
    regs->r0 = result;
}

/* 用户程序系统调用包装函数 */
static inline uint32_t syscall(uint32_t num, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
    uint32_t result;
    
    asm volatile(
        "mov r0, %1\n"     /* 系统调用号 */
        "mov r1, %2\n"     /* 参数1 */
        "mov r2, %3\n"     /* 参数2 */
        "mov r3, %4\n"     /* 参数3 */
        "svc #0\n"         /* 触发系统调用 */
        "mov %0, r0\n"     /* 获取返回值 */
        : "=r"(result)
        : "r"(num), "r"(arg1), "r"(arg2), "r"(arg3)
        : "r0", "r1", "r2", "r3"
    );
    
    return result;
}

/* 测试系统调用 */
void test_syscalls(void) {
    uart_puts("\r\n=== Testing System Calls ===\r\n");
    
    /* 测试写系统调用 */
    const char *msg1 = "Hello from syscall write!\r\n";
    uint32_t result1 = syscall(SYS_WRITE, 1, (uint32_t)msg1, 28);
    uart_puts("Write syscall returned: ");
    uart_put_hex(result1);
    uart_puts("\r\n");
    
    /* 测试print系统调用 */
    const char *msg2 = "Hello from syscall print!\r\n";
    uint32_t result2 = syscall(SYS_PRINT, (uint32_t)msg2, 0, 0);
    uart_puts("Print syscall returned: ");
    uart_put_hex(result2);
    uart_puts("\r\n");
    
    /* 测试获取时间系统调用 */
    uint32_t time = syscall(SYS_GETTIME, 0, 0, 0);
    uart_puts("Current time from syscall: ");
    uart_put_hex(time);
    uart_puts(" ticks\r\n");
    
    /* 测试stderr写入 */
    const char *err_msg = "This is an error message!\r\n";
    uint32_t result3 = syscall(SYS_WRITE, 2, (uint32_t)err_msg, 28);
    uart_puts("Stderr write returned: ");
    uart_put_hex(result3);
    uart_puts("\r\n");
    
    /* 测试读系统调用 */
    char buffer[64];
    uint32_t result4 = syscall(SYS_READ, 0, (uint32_t)buffer, sizeof(buffer));
    uart_puts("Read syscall returned: ");
    uart_put_hex(result4);
    uart_puts(" bytes: \"");
    uart_puts(buffer);
    uart_puts("\"\r\n");
    
    /* 测试无效系统调用 */
    uint32_t result5 = syscall(99, 0, 0, 0);
    uart_puts("Invalid syscall returned: ");
    uart_put_hex(result5);
    uart_puts("\r\n");
    
    uart_puts("=============================\r\n");
}

/* 获取系统调用统计信息 */
void print_syscall_stats(void) {
    uart_puts("\r\n=== System Call Statistics ===\r\n");
    uart_puts("Total system calls: ");
    uart_put_hex(total_syscalls);
    uart_puts("\r\n");
    
    for (uint32_t i = 1; i < SYSCALL_COUNT; i++) {
        if (syscall_counts[i] > 0) {
            uart_puts("  ");
            if (i < sizeof(syscall_names)/sizeof(syscall_names[0]) && syscall_names[i]) {
                uart_puts(syscall_names[i]);
            } else {
                uart_puts("syscall_");
                uart_put_hex(i);
            }
            uart_puts(": ");
            uart_put_hex(syscall_counts[i]);
            uart_puts(" calls\r\n");
        }
    }
    uart_puts("==============================\r\n");
} 