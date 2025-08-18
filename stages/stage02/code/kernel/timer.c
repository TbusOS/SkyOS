/*
 * SkyOS ARM Generic Timer实现
 * 文件: kernel/timer.c
 * 
 * 实现ARM Generic Timer的配置和中断处理
 */

#include <stdint.h>

/* 外部函数声明 */
extern void uart_puts(const char *str);
extern void uart_put_hex(uint32_t value);

/* ARM Generic Timer寄存器访问函数 */
static inline uint32_t read_cntfrq(void) {
    uint32_t freq;
    asm volatile("mrc p15, 0, %0, c14, c0, 0" : "=r"(freq));
    return freq;
}

static inline uint64_t read_cntpct(void) {
    uint64_t val;
    asm volatile("mrrc p15, 0, %Q0, %R0, c14" : "=r"(val));
    return val;
}

static inline uint32_t read_cntp_tval(void) {
    uint32_t tval;
    asm volatile("mrc p15, 0, %0, c14, c2, 0" : "=r"(tval));
    return tval;
}

static inline void write_cntp_tval(uint32_t tval) {
    asm volatile("mcr p15, 0, %0, c14, c2, 0" : : "r"(tval));
}

static inline uint32_t read_cntp_ctl(void) {
    uint32_t ctl;
    asm volatile("mrc p15, 0, %0, c14, c2, 1" : "=r"(ctl));
    return ctl;
}

static inline void write_cntp_ctl(uint32_t ctl) {
    asm volatile("mcr p15, 0, %0, c14, c2, 1" : : "r"(ctl));
}

/* Timer Control register bits */
#define CNTP_CTL_ENABLE     (1 << 0)   /* Timer enable */
#define CNTP_CTL_IMASK      (1 << 1)   /* Timer interrupt mask */
#define CNTP_CTL_ISTATUS    (1 << 2)   /* Timer interrupt status */

/* 全局变量 */
static uint32_t timer_frequency = 0;
static volatile uint32_t timer_ticks = 0;
static volatile uint32_t timer_interrupts = 0;
static uint32_t timer_interval = 0;

/* 获取定时器频率 */
uint32_t timer_get_frequency(void) {
    if (timer_frequency == 0) {
        timer_frequency = read_cntfrq();
    }
    return timer_frequency;
}

/* 读取物理计数器值 */
uint64_t timer_get_counter(void) {
    return read_cntpct();
}

/* 读取定时器值寄存器 */
uint32_t timer_get_tval(void) {
    return read_cntp_tval();
}

/* 设置定时器值寄存器 */
void timer_set_tval(uint32_t tval) {
    write_cntp_tval(tval);
}

/* 读取定时器控制寄存器 */
uint32_t timer_get_control(void) {
    return read_cntp_ctl();
}

/* 设置定时器控制寄存器 */
void timer_set_control(uint32_t ctl) {
    write_cntp_ctl(ctl);
}

/* 初始化ARM Generic Timer */
void timer_init(void) {
    uart_puts("初始化ARM Generic Timer...\r\n");
    
    /* 获取定时器频率 */
    timer_frequency = timer_get_frequency();
    uart_puts("定时器频率: ");
    uart_put_hex(timer_frequency);
    uart_puts(" Hz\r\n");
    
    /* 计算定时器间隔 (100Hz = 10ms) */
    timer_interval = timer_frequency / 100;
    uart_puts("定时器间隔: ");
    uart_put_hex(timer_interval);
    uart_puts(" 计数 (10ms)\r\n");
    
    /* 禁用定时器中断并清除状态 */
    timer_set_control(0);
    
    /* 设置定时器值 */
    timer_set_tval(timer_interval);
    
    /* 启用定时器，不屏蔽中断 */
    timer_set_control(CNTP_CTL_ENABLE);
    
    uart_puts("ARM Generic Timer 初始化完成\r\n");
}

/* 定时器中断处理函数 */
void timer_handle_interrupt(void) {
    /* 增加中断计数 */
    timer_interrupts++;
    timer_ticks++;
    
    /* 重新设置下次中断 */
    timer_set_tval(timer_interval);
    
    /* 每秒输出一次统计信息 (100次中断 = 1秒) */
    if (timer_ticks % 100 == 0) {
        uart_puts("⏰ 定时器: ");
        uart_put_hex(timer_ticks / 100);
        uart_puts("秒 (");
        uart_put_hex(timer_ticks);
        uart_puts(" 滴答, ");
        uart_put_hex(timer_interrupts);
        uart_puts(" 中断)\r\n");
    }
}

/* 获取当前滴答数 */
uint32_t get_timer_ticks(void) {
    return timer_ticks;
}

/* 获取定时器中断计数 */
uint32_t timer_get_interrupt_count(void) {
    return timer_interrupts;
}

/* 获取定时器状态信息 */
void timer_print_status(void) {
    uint32_t ctl = timer_get_control();
    uint32_t tval = timer_get_tval();
    uint64_t counter = timer_get_counter();
    
    uart_puts("\r\n=== ARM Generic Timer 状态 ===\r\n");
    uart_puts("频率: ");
    uart_put_hex(timer_frequency);
    uart_puts(" Hz\r\n");
    
    uart_puts("控制寄存器: ");
    uart_put_hex(ctl);
    uart_puts(" (");
    if (ctl & CNTP_CTL_ENABLE) {
        uart_puts("启用");
    } else {
        uart_puts("禁用");
    }
    if (ctl & CNTP_CTL_IMASK) {
        uart_puts(", 中断屏蔽");
    } else {
        uart_puts(", 中断使能");
    }
    if (ctl & CNTP_CTL_ISTATUS) {
        uart_puts(", 中断挂起");
    }
    uart_puts(")\r\n");
    
    uart_puts("定时器值: ");
    uart_put_hex(tval);
    uart_puts("\r\n");
    
    uart_puts("物理计数器: ");
    uart_put_hex((uint32_t)(counter >> 32));
    uart_put_hex((uint32_t)(counter & 0xFFFFFFFF));
    uart_puts("\r\n");
    
    uart_puts("总滴答数: ");
    uart_put_hex(timer_ticks);
    uart_puts("\r\n");
    
    uart_puts("中断次数: ");
    uart_put_hex(timer_interrupts);
    uart_puts("\r\n");
    
    uart_puts("运行时间: ");
    uart_put_hex(timer_ticks / 100);
    uart_puts(".");
    uart_put_hex((timer_ticks % 100) / 10);
    uart_puts(" 秒\r\n");
    
    uart_puts("=============================\r\n");
}

/* 延时函数 (基于定时器) */
void timer_delay_ms(uint32_t milliseconds) {
    uint32_t start_ticks = timer_ticks;
    uint32_t target_ticks = start_ticks + (milliseconds / 10); /* 10ms per tick */
    
    while (timer_ticks < target_ticks) {
        /* 等待定时器中断 */
        asm volatile("wfi");
    }
}

/* 微秒级延时 (基于计数器) */
void timer_delay_us(uint32_t microseconds) {
    uint64_t start_counter = timer_get_counter();
    /* 简化计算，避免64位除法 */
    uint32_t delay_cycles = (microseconds * (timer_frequency >> 10)) >> 10; /* 约等于 / 1000000 */
    uint64_t target_counter = start_counter + delay_cycles;
    
    while (timer_get_counter() < target_counter) {
        /* 忙等待 */
    }
}

/* 获取当前时间戳 (微秒) */
uint64_t timer_get_timestamp_us(void) {
    uint64_t counter = timer_get_counter();
    /* 使用32位计算避免64位除法 */
    uint32_t counter_low = (uint32_t)counter;
    uint32_t freq_mhz = timer_frequency / 1000000; /* 频率转为MHz */
    if (freq_mhz == 0) freq_mhz = 1; /* 避免除零 */
    return counter_low / freq_mhz;
}

/* 性能测量辅助函数 */
typedef struct {
    uint64_t start_counter;
    const char *name;
} timer_benchmark_t;

/* 开始性能测量 */
timer_benchmark_t timer_benchmark_start(const char *name) {
    timer_benchmark_t bench;
    bench.name = name;
    bench.start_counter = timer_get_counter();
    return bench;
}

/* 结束性能测量并输出结果 */
void timer_benchmark_end(timer_benchmark_t bench) {
    uint64_t end_counter = timer_get_counter();
    uint32_t elapsed_cycles = (uint32_t)(end_counter - bench.start_counter);
    /* 使用32位计算避免64位除法 */
    uint32_t freq_mhz = timer_frequency / 1000000; /* 频率转为MHz */
    if (freq_mhz == 0) freq_mhz = 1; /* 避免除零 */
    uint32_t elapsed_us = elapsed_cycles / freq_mhz;
    
    uart_puts("⏱️  ");
    uart_puts(bench.name);
    uart_puts(": ");
    uart_put_hex(elapsed_cycles);
    uart_puts(" 周期, ");
    uart_put_hex(elapsed_us);
    uart_puts(" 微秒\r\n");
} 