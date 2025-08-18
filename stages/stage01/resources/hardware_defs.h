/*
 * SkymOS硬件地址定义
 * 自动生成于设备树分析
 * 请勿手动修改此文件
 */

#ifndef _SKYMOS_HARDWARE_H_
#define _SKYMOS_HARDWARE_H_

#include <stdint.h>

/* 内存布局 */
#define INTC_BASE        0x08000000    /* ARM GIC中断控制器 */
#define INTC_SIZE        0x00010000    /* 65536 bytes */

#define V2M_BASE        0x08020000    /* 未知设备 */
#define V2M_SIZE        0x00001000    /* 4096 bytes */

#define PL011_BASE        0x09000000    /* ARM PL011 UART串口控制器 */
#define PL011_SIZE        0x00001000    /* 4096 bytes */

#define PL031_BASE        0x09010000    /* ARM PL031 RTC实时时钟 */
#define PL031_SIZE        0x00001000    /* 4096 bytes */

#define CFG_BASE        0x09020000    /* 未知设备类型 (cfg) */
#define CFG_SIZE        0x00000018    /* 24 bytes */

#define GPIO_BASE        0x09030000    /* ARM PL061 GPIO控制器 */
#define GPIO_SIZE        0x00001000    /* 4096 bytes */

#define VIRTIO_MMIO_BASE        0x0A000000    /* VirtIO MMIO设备 */
#define VIRTIO_MMIO_SIZE        0x00000200    /* 512 bytes */

#define PCIE_BASE        0x3F000000    /* PCIe主机控制器 */
#define PCIE_SIZE        0x01000000    /* 16777216 bytes */

#define MEMORY_BASE        0x40000000    /* 主内存 (RAM) */

/* 中断号定义 */
#define CPU_IRQ          13                /* 未知设备类型 (cpu) */
#define PL011_IRQ          33                /* ARM PL011 UART串口控制器 */
#define PL031_IRQ          34                /* ARM PL031 RTC实时时钟 */
#define GPIO_IRQ          39                /* ARM PL061 GPIO控制器 */
#define VIRTIO_MMIO_IRQ          48                /* VirtIO MMIO设备 */

#endif /* _SKYMOS_HARDWARE_H_ */