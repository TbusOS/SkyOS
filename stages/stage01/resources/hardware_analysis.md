╔════════════════════════════════════════════════════════════════════════════════╗
║                           SkyOS硬件规格分析报告                                ║
╚════════════════════════════════════════════════════════════════════════════════╝

📍 内存布局
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
intc: 0x08000000 - 0x0800FFFF (64KB) - ARM GIC中断控制器
v2m: 0x08020000 - 0x08020FFF (4KB) - 未知设备
pl011: 0x09000000 - 0x09000FFF (4KB) - ARM PL011 UART串口控制器
pl031: 0x09010000 - 0x09010FFF (4KB) - ARM PL031 RTC实时时钟
cfg: 0x09020000 - 0x09020017 (0KB) - 未知设备类型 (cfg)
gpio: 0x09030000 - 0x09030FFF (4KB) - ARM PL061 GPIO控制器
virtio_mmio: 0x0A000000 - 0x0A0001FF (0KB) - VirtIO MMIO设备
pcie: 0x3F000000 - 0x3FFFFFFF (16384KB) - PCIe主机控制器
memory: 0x40000000 - 0x4FFFFFFF (262144KB) - 主内存 (RAM)

⚡ 中断配置
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
IRQ 13: cpu (Level) - 未知设备类型 (cpu)
IRQ 33: pl011 (Level) - ARM PL011 UART串口控制器
IRQ 34: pl031 (Level) - ARM PL031 RTC实时时钟
IRQ 39: gpio (Level) - ARM PL061 GPIO控制器
IRQ 48: virtio_mmio (Edge) - VirtIO MMIO设备

💡 关键代码对应关系
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

UART基地址定义 (kernel/main.c):
    #define UART0_BASE      0x09000000    // 来源: 设备树 pl011@9000000
    
内存起始地址 (boot/boot.lds):
    MEMORY { RAM (rwx) : ORIGIN = 0x40000000, LENGTH = 256M }
    
🔍 验证命令
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
1. 生成设备树:
   qemu-system-arm -machine virt -cpu cortex-a15 -machine dumpdtb=virt.dtb
   
2. 转换为文本:
   dtc -I dtb -O dts virt.dtb > virt.dts
   
3. 查看特定设备:
   grep -A 5 "pl011@" virt.dts      # UART配置
   grep -A 5 "memory@" virt.dts     # 内存配置
   grep -A 5 "intc@" virt.dts       # 中断控制器
   
4. 运行时查询:
   (qemu) info mtree               # QEMU monitor中查看内存树
   (qemu) info qtree               # 查看设备树

📚 参考文档
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
- QEMU ARM System Emulation: https://www.qemu.org/docs/master/system/target-arm.html
- QEMU virt machine: https://qemu-project.gitlab.io/qemu/system/arm/virt.html
- ARM PL011 UART TRM: ARM DDI 0183
- ARM GIC-400 TRM: ARM DDI 0471
- Device Tree Specification: https://www.devicetree.org/