/*
 * SkyOS ARM Generic Interrupt Controller (GIC) 实现
 * 文件: kernel/gic.c
 * 
 * 实现ARM GIC v2中断控制器的配置和管理
 */

#include <stdint.h>

/* 外部函数声明 */
extern void uart_puts(const char *str);
extern void uart_put_hex(uint32_t value);
extern void timer_handle_interrupt(void);

/* QEMU virt machine GIC地址定义 */
#define GIC_DIST_BASE   0x08000000  /* 分发器基址 */
#define GIC_CPU_BASE    0x08010000  /* CPU接口基址 */

/* GIC分发器寄存器偏移 */
#define GICD_CTLR       0x000  /* 分发器控制寄存器 */
#define GICD_TYPER      0x004  /* 分发器类型寄存器 */
#define GICD_IIDR       0x008  /* 分发器实现标识寄存器 */
#define GICD_IGROUPR    0x080  /* 中断组寄存器 */
#define GICD_ISENABLER  0x100  /* 中断使能设置寄存器 */
#define GICD_ICENABLER  0x180  /* 中断使能清除寄存器 */
#define GICD_ISPENDR    0x200  /* 中断挂起设置寄存器 */
#define GICD_ICPENDR    0x280  /* 中断挂起清除寄存器 */
#define GICD_ISACTIVER  0x300  /* 中断活跃设置寄存器 */
#define GICD_ICACTIVER  0x380  /* 中断活跃清除寄存器 */
#define GICD_IPRIORITYR 0x400  /* 中断优先级寄存器 */
#define GICD_ITARGETSR  0x800  /* 中断目标CPU寄存器 */
#define GICD_ICFGR      0xC00  /* 中断配置寄存器 */
#define GICD_SGIR       0xF00  /* 软件生成中断寄存器 */

/* GIC CPU接口寄存器偏移 */
#define GICC_CTLR       0x000  /* CPU接口控制寄存器 */
#define GICC_PMR        0x004  /* 优先级屏蔽寄存器 */
#define GICC_BPR        0x008  /* 二进制点寄存器 */
#define GICC_IAR        0x00C  /* 中断确认寄存器 */
#define GICC_EOIR       0x010  /* 中断结束寄存器 */
#define GICC_RPR        0x014  /* 运行优先级寄存器 */
#define GICC_HPPIR      0x018  /* 最高优先级挂起中断寄存器 */
#define GICC_ABPR       0x01C  /* 别名二进制点寄存器 */
#define GICC_IIDR       0x0FC  /* CPU接口标识寄存器 */

/* 寄存器访问宏 */
#define GIC_DIST_REG(offset) (*(volatile uint32_t*)(GIC_DIST_BASE + (offset)))
#define GIC_CPU_REG(offset)  (*(volatile uint32_t*)(GIC_CPU_BASE + (offset)))

/* 中断ID定义 */
#define SGI_BASE        0   /* 软件生成中断 0-15 */
#define PPI_BASE        16  /* 私有外设中断 16-31 */
#define SPI_BASE        32  /* 共享外设中断 32+ */

/* ARM Generic Timer Physical Timer中断 */
#define TIMER_IRQ_ID    30  /* ARM Generic Timer Physical Timer */

/* GIC控制位定义 */
#define GICD_CTLR_ENABLE    (1 << 0)   /* 分发器使能 */
#define GICC_CTLR_ENABLE    (1 << 0)   /* CPU接口使能 */

/* 中断优先级 */
#define IRQ_PRIORITY_HIGH   0x40
#define IRQ_PRIORITY_NORMAL 0x80
#define IRQ_PRIORITY_LOW    0xC0

/* 全局变量 */
static uint32_t gic_num_irqs = 0;
static uint32_t gic_cpu_count = 0;
static volatile uint32_t irq_counts[1024] = {0}; /* 中断计数统计 */
static volatile uint32_t total_irqs = 0;

/* 读取GIC分发器类型信息 */
static void gic_read_distributor_info(void) {
    uint32_t typer = GIC_DIST_REG(GICD_TYPER);
    
    /* 计算支持的中断数量 */
    gic_num_irqs = ((typer & 0x1F) + 1) * 32;
    
    /* 计算CPU数量 */
    gic_cpu_count = ((typer >> 5) & 0x7) + 1;
    
    uart_puts("GIC信息:\r\n");
    uart_puts("  支持中断数: ");
    uart_put_hex(gic_num_irqs);
    uart_puts("\r\n");
    uart_puts("  CPU数量: ");
    uart_put_hex(gic_cpu_count);
    uart_puts("\r\n");
    uart_puts("  类型寄存器: ");
    uart_put_hex(typer);
    uart_puts("\r\n");
}

/* 禁用所有中断 */
static void gic_disable_all_interrupts(void) {
    uint32_t i;
    
    /* 禁用所有SPI中断 */
    for (i = SPI_BASE; i < gic_num_irqs; i += 32) {
        GIC_DIST_REG(GICD_ICENABLER + (i / 32) * 4) = 0xFFFFFFFF;
    }
    
    /* 禁用所有PPI中断 */
    GIC_DIST_REG(GICD_ICENABLER + 0) = 0xFFFF0000;
    
    /* 禁用所有SGI中断 */
    GIC_DIST_REG(GICD_ICENABLER + 0) |= 0x0000FFFF;
}

/* 清除所有挂起中断 */
static void gic_clear_all_pending(void) {
    uint32_t i;
    
    for (i = 0; i < gic_num_irqs; i += 32) {
        GIC_DIST_REG(GICD_ICPENDR + (i / 32) * 4) = 0xFFFFFFFF;
    }
}

/* 设置中断优先级 */
static void gic_set_priority(uint32_t irq_id, uint8_t priority) {
    uint32_t reg_offset = GICD_IPRIORITYR + (irq_id / 4) * 4;
    uint32_t bit_offset = (irq_id % 4) * 8;
    uint32_t reg_val = GIC_DIST_REG(reg_offset);
    
    /* 清除旧优先级并设置新优先级 */
    reg_val &= ~(0xFF << bit_offset);
    reg_val |= (priority << bit_offset);
    GIC_DIST_REG(reg_offset) = reg_val;
}

/* 设置中断目标CPU */
static void gic_set_target(uint32_t irq_id, uint8_t cpu_mask) {
    uint32_t reg_offset = GICD_ITARGETSR + (irq_id / 4) * 4;
    uint32_t bit_offset = (irq_id % 4) * 8;
    uint32_t reg_val = GIC_DIST_REG(reg_offset);
    
    /* 清除旧目标并设置新目标 */
    reg_val &= ~(0xFF << bit_offset);
    reg_val |= (cpu_mask << bit_offset);
    GIC_DIST_REG(reg_offset) = reg_val;
}

/* 使能指定中断 */
void gic_enable_interrupt(uint32_t irq_id) {
    uint32_t reg_offset = GICD_ISENABLER + (irq_id / 32) * 4;
    uint32_t bit_offset = irq_id % 32;
    
    GIC_DIST_REG(reg_offset) = (1 << bit_offset);
}

/* 禁用指定中断 */
void gic_disable_interrupt(uint32_t irq_id) {
    uint32_t reg_offset = GICD_ICENABLER + (irq_id / 32) * 4;
    uint32_t bit_offset = irq_id % 32;
    
    GIC_DIST_REG(reg_offset) = (1 << bit_offset);
}

/* 检查中断是否使能 */
uint32_t gic_is_interrupt_enabled(uint32_t irq_id) {
    uint32_t reg_offset = GICD_ISENABLER + (irq_id / 32) * 4;
    uint32_t bit_offset = irq_id % 32;
    
    return (GIC_DIST_REG(reg_offset) >> bit_offset) & 1;
}

/* 触发软件生成中断 */
void gic_send_sgi(uint32_t sgi_id, uint32_t target_cpu_mask) {
    uint32_t sgir_val = (target_cpu_mask << 16) | sgi_id;
    GIC_DIST_REG(GICD_SGIR) = sgir_val;
}

/* 初始化GIC */
void gic_init(void) {
    uart_puts("初始化ARM GIC v2中断控制器...\r\n");
    
    /* 禁用分发器和CPU接口 */
    GIC_DIST_REG(GICD_CTLR) = 0;
    GIC_CPU_REG(GICC_CTLR) = 0;
    
    /* 读取GIC信息 */
    gic_read_distributor_info();
    
    /* 禁用所有中断 */
    gic_disable_all_interrupts();
    
    /* 清除所有挂起中断 */
    gic_clear_all_pending();
    
    /* 配置定时器中断 */
    uart_puts("配置定时器中断 (IRQ ");
    uart_put_hex(TIMER_IRQ_ID);
    uart_puts(")...\r\n");
    
    /* 设置定时器中断优先级 */
    gic_set_priority(TIMER_IRQ_ID, IRQ_PRIORITY_NORMAL);
    
    /* 设置定时器中断目标CPU (CPU 0) */
    gic_set_target(TIMER_IRQ_ID, 0x01);
    
    /* 启用定时器中断 */
    gic_enable_interrupt(TIMER_IRQ_ID);
    
    /* 设置CPU接口优先级屏蔽 (允许所有优先级) */
    GIC_CPU_REG(GICC_PMR) = 0xFF;
    
    /* 设置二进制点 (所有位用于优先级) */
    GIC_CPU_REG(GICC_BPR) = 0;
    
    /* 启用CPU接口 */
    GIC_CPU_REG(GICC_CTLR) = GICC_CTLR_ENABLE;
    
    /* 启用分发器 */
    GIC_DIST_REG(GICD_CTLR) = GICD_CTLR_ENABLE;
    
    uart_puts("GIC初始化完成\r\n");
}

/* IRQ中断处理程序 */
void handle_irq(void) {
    /* 读取中断确认寄存器，获取中断ID */
    uint32_t iar = GIC_CPU_REG(GICC_IAR);
    uint32_t irq_id = iar & 0x3FF;
    
    /* 增加总中断计数 */
    total_irqs++;
    
    /* 增加特定中断计数 */
    if (irq_id < 1024) {
        irq_counts[irq_id]++;
    }
    
    /* 根据中断ID分发处理 */
    switch (irq_id) {
        case TIMER_IRQ_ID:
            /* 处理定时器中断 */
            timer_handle_interrupt();
            break;
            
        case 1022:
            /* 无效中断 */
            uart_puts("无效IRQ中断\r\n");
            break;
            
        case 1023:
            /* 伪中断 */
            uart_puts("伪IRQ中断\r\n");
            break;
            
        default:
            /* 未知中断 */
            uart_puts("未知IRQ: ");
            uart_put_hex(irq_id);
            uart_puts("\r\n");
            break;
    }
    
    /* 发送中断结束信号 */
    GIC_CPU_REG(GICC_EOIR) = iar;
}

/* 获取GIC状态信息 */
void gic_print_status(void) {
    uint32_t dist_ctlr = GIC_DIST_REG(GICD_CTLR);
    uint32_t cpu_ctlr = GIC_CPU_REG(GICC_CTLR);
    uint32_t pmr = GIC_CPU_REG(GICC_PMR);
    uint32_t rpr = GIC_CPU_REG(GICC_RPR);
    uint32_t hppir = GIC_CPU_REG(GICC_HPPIR);
    
    uart_puts("\r\n=== GIC状态信息 ===\r\n");
    uart_puts("分发器控制: ");
    uart_put_hex(dist_ctlr);
    uart_puts(" (");
    uart_puts((dist_ctlr & GICD_CTLR_ENABLE) ? "启用" : "禁用");
    uart_puts(")\r\n");
    
    uart_puts("CPU接口控制: ");
    uart_put_hex(cpu_ctlr);
    uart_puts(" (");
    uart_puts((cpu_ctlr & GICC_CTLR_ENABLE) ? "启用" : "禁用");
    uart_puts(")\r\n");
    
    uart_puts("优先级屏蔽: ");
    uart_put_hex(pmr);
    uart_puts("\r\n");
    
    uart_puts("运行优先级: ");
    uart_put_hex(rpr);
    uart_puts("\r\n");
    
    uart_puts("最高优先级挂起中断: ");
    uart_put_hex(hppir);
    uart_puts("\r\n");
    
    uart_puts("定时器中断状态: ");
    uart_puts(gic_is_interrupt_enabled(TIMER_IRQ_ID) ? "启用" : "禁用");
    uart_puts("\r\n");
    
    uart_puts("总中断数: ");
    uart_put_hex(total_irqs);
    uart_puts("\r\n");
    
    uart_puts("定时器中断数: ");
    uart_put_hex(irq_counts[TIMER_IRQ_ID]);
    uart_puts("\r\n");
    
    uart_puts("==================\r\n");
}

/* 获取中断统计信息 */
void gic_print_interrupt_stats(void) {
    uart_puts("\r\n=== 中断统计信息 ===\r\n");
    uart_puts("总中断数: ");
    uart_put_hex(total_irqs);
    uart_puts("\r\n");
    
    /* 显示活跃的中断 */
    for (uint32_t i = 0; i < 64; i++) {  /* 只显示前64个中断 */
        if (irq_counts[i] > 0) {
            uart_puts("  IRQ ");
            uart_put_hex(i);
            uart_puts(": ");
            uart_put_hex(irq_counts[i]);
            uart_puts(" 次");
            if (i == TIMER_IRQ_ID) {
                uart_puts(" (定时器)");
            }
            uart_puts("\r\n");
        }
    }
    uart_puts("==================\r\n");
}

/* 测试软件生成中断 */
void gic_test_sgi(void) {
    uart_puts("测试软件生成中断...\r\n");
    
    /* 配置SGI 0 */
    gic_set_priority(0, IRQ_PRIORITY_HIGH);
    gic_enable_interrupt(0);
    
    /* 发送SGI 0到当前CPU */
    gic_send_sgi(0, 0x01);
    
    uart_puts("SGI测试完成\r\n");
}

/* 获取GIC版本信息 */
void gic_print_version_info(void) {
    uint32_t dist_iidr = GIC_DIST_REG(GICD_IIDR);
    uint32_t cpu_iidr = GIC_CPU_REG(GICC_IIDR);
    
    uart_puts("\r\n=== GIC版本信息 ===\r\n");
    uart_puts("分发器ID: ");
    uart_put_hex(dist_iidr);
    uart_puts("\r\n");
    uart_puts("CPU接口ID: ");
    uart_put_hex(cpu_iidr);
    uart_puts("\r\n");
    uart_puts("支持中断数: ");
    uart_put_hex(gic_num_irqs);
    uart_puts("\r\n");
    uart_puts("CPU数量: ");
    uart_put_hex(gic_cpu_count);
    uart_puts("\r\n");
    uart_puts("==================\r\n");
} 