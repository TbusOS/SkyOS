# ARM64硬件规格说明文档

## 🎯 文档目的

本文档详细解释SkyOS ARM64版本中使用的硬件规格、内存布局、寄存器地址等技术细节的来源，帮助学生理解ARM64架构的"神奇数字"是从哪里来的，并提供具体的技术文档下载链接。

## 📋 QEMU virt machine ARM64硬件规格

### 为什么选择QEMU virt machine？

1. **标准化**: virt machine是QEMU提供的标准化ARM64虚拟机
2. **文档完善**: 有详细的硬件规格文档和源码可查
3. **教学友好**: 简化了真实硬件的复杂性，便于学习
4. **调试支持**: 支持GDB调试和设备树分析

### QEMU virt machine官方文档

**主要参考文档**:
- **QEMU ARM系统仿真**: https://www.qemu.org/docs/master/system/target-arm.html
- **QEMU virt machine专页**: https://qemu-project.gitlab.io/qemu/system/arm/virt.html
- **QEMU源码仓库**: https://gitlab.com/qemu-project/qemu
- **QEMU源码文件**: `hw/arm/virt.c` (定义内存映射)
- **ARM Architecture Reference Manual**: https://developer.arm.com/documentation/ddi0487/

## 🗺️ 内存布局详解

### 1. RAM基址：0x40000000

**来源说明**:
- **QEMU源码位置**: `hw/arm/virt.c` 文件中的 `VIRT_MEM` 定义
- **源码链接**: https://gitlab.com/qemu-project/qemu/-/blob/master/hw/arm/virt.c
- **具体代码**:
  ```c
  static const MemMapEntry base_memmap[] = {
      [VIRT_MEM] = { 0x40000000, LEGACY_RAMLIMIT_BYTES },
      // ...
  };
  ```

**技术背景**:
- ARM64虚拟地址空间可以从任意地址开始
- 0x40000000 (1GB) 为QEMU virt machine的标准RAM起始地址
- 与ARM32保持兼容，便于教学对比

**验证方法**:
```bash
# 查看QEMU内存布局
qemu-system-aarch64 -machine virt -cpu cortex-a57 -monitor stdio
(qemu) info mtree
```

### 2. UART基址：0x09000000

**来源说明**:
- **QEMU源码位置**: `hw/arm/virt.c` 文件中的 `VIRT_UART` 定义
- **源码代码**:
  ```c
  static const MemMapEntry base_memmap[] = {
      [VIRT_UART] = { 0x09000000, 0x00001000 },
      // ...
  };
  ```

**设备详情**:
- **设备类型**: ARM PL011 UART
- **地址空间**: 4KB (0x09000000 - 0x09000FFF)
- **中断号**: IRQ 33 (SPI 1)

## 📖 ARM64寄存器规格

### 1. 异常向量表基址寄存器 (VBAR_EL1)

**来源文档**:
- **ARM DDI 0487**: ARM Architecture Reference Manual ARMv8-A
- **下载链接**: https://developer.arm.com/documentation/ddi0487/
- **章节**: D13.2.146 VBAR_EL1, Vector Base Address Register (EL1)

**寄存器详情**:
- **位宽**: 64位
- **复位值**: 0x0000000000000000
- **用途**: 存储异常向量表的基地址
- **对齐要求**: 必须2KB对齐 (bit[10:0] = 0)

### 2. 处理器标识寄存器 (MIDR_EL1)

**来源文档**:
- **ARM DDI 0487**: ARM Architecture Reference Manual ARMv8-A
- **章节**: D13.2.64 MIDR_EL1, Main ID Register
- **Cortex-A57 TRM**: https://developer.arm.com/documentation/ddi0488/

**寄存器字段** (Cortex-A57):
```
Bits [31:24] - Implementer: 0x41 (ARM Limited)
Bits [23:20] - Variant: 0x1
Bits [19:16] - Architecture: 0xF (ARMv8)
Bits [15:4]  - PartNum: 0xD07 (Cortex-A57)
Bits [3:0]   - Revision: 0x0-0x2
```

### 3. 当前异常级别寄存器 (CurrentEL)

**来源文档**:
- **ARM DDI 0487**: ARM Architecture Reference Manual ARMv8-A
- **章节**: D13.2.26 CurrentEL, Current Exception Level

**寄存器字段**:
```
Bits [63:4] - RES0 (Reserved, reads as zero)
Bits [3:2]  - EL (Exception Level)
            00 = EL0, 01 = EL1, 10 = EL2, 11 = EL3
Bits [1:0]  - RES0
```

### 4. 系统控制寄存器 (SCTLR_EL1)

**来源文档**:
- **ARM DDI 0487**: ARM Architecture Reference Manual ARMv8-A
- **章节**: D13.2.118 SCTLR_EL1, System Control Register (EL1)

**关键控制位**:
```
Bit [0]  - M (MMU enable)
Bit [1]  - A (Alignment check enable)
Bit [2]  - C (Data cache enable)
Bit [3]  - SA (Stack Alignment Check Enable)
Bit [12] - I (Instruction cache enable)
Bit [19] - WXN (Write permission implies XN)
```

## 🔧 ARM64异常向量表规格

### 异常向量表结构

**来源文档**:
- **ARM DDI 0487**: ARM Architecture Reference Manual ARMv8-A
- **章节**: D1.10.2 Vector tables

**向量表布局**:
```
基址 + 0x000: Current EL with SP_EL0, Synchronous
基址 + 0x080: Current EL with SP_EL0, IRQ/vIRQ
基址 + 0x100: Current EL with SP_EL0, FIQ/vFIQ
基址 + 0x180: Current EL with SP_EL0, SError/vSError

基址 + 0x200: Current EL with SP_ELx, Synchronous
基址 + 0x280: Current EL with SP_ELx, IRQ/vIRQ
基址 + 0x300: Current EL with SP_ELx, FIQ/vFIQ
基址 + 0x380: Current EL with SP_ELx, SError/vSError

基址 + 0x400: Lower EL using AArch64, Synchronous
基址 + 0x480: Lower EL using AArch64, IRQ/vIRQ
基址 + 0x500: Lower EL using AArch64, FIQ/vFIQ
基址 + 0x580: Lower EL using AArch64, SError/vSError

基址 + 0x600: Lower EL using AArch32, Synchronous
基址 + 0x680: Lower EL using AArch32, IRQ/vIRQ
基址 + 0x700: Lower EL using AArch32, FIQ/vFIQ
基址 + 0x780: Lower EL using AArch32, SError/vSError
```

**关键特性**:
- 每个向量条目占128字节
- 总共16个向量条目，占用2KB空间
- 支持同一异常级别和低异常级别的异常处理
- 支持AArch64和AArch32状态的异常

## 🌐 串口设备规格 (PL011 UART)

### ARM PL011 UART技术手册

**官方文档**:
- **文档编号**: ARM DDI 0183G
- **标题**: ARM PrimeCell UART (PL011) Technical Reference Manual
- **下载链接**: https://developer.arm.com/documentation/ddi0183/latest/
- **版本**: Revision r1p5

### 寄存器映射

基址: 0x09000000

| 偏移 | 寄存器名 | 全称 | 描述 |
|------|----------|------|------|
| 0x00 | UARTDR | Data Register | 数据寄存器 |
| 0x04 | UARTRSR/UARTECR | Receive Status/Error Clear | 接收状态/错误清除 |
| 0x18 | UARTFR | Flag Register | 标志寄存器 |
| 0x20 | UARTILPR | IrDA Low-Power Counter | IrDA低功耗计数器 |
| 0x24 | UARTIBRD | Integer Baud Rate Divisor | 整数波特率除数 |
| 0x28 | UARTFBRD | Fractional Baud Rate Divisor | 分数波特率除数 |
| 0x2C | UARTLCR_H | Line Control Register | 线路控制寄存器 |
| 0x30 | UARTCR | Control Register | 控制寄存器 |

### 关键寄存器详解

**UARTFR (标志寄存器) - 偏移0x18**:
```
Bit [7] - TXFE (Transmit FIFO Empty)
Bit [6] - RXFF (Receive FIFO Full)  
Bit [5] - TXFF (Transmit FIFO Full)  ← 我们使用的位
Bit [4] - RXFE (Receive FIFO Empty)
Bit [3] - BUSY (UART Busy)
```

**代码中的使用**:
```c
#define UART_FR_TXFF    (1 << 5)  // 发送FIFO满标志

void uart_putc(char c) {
    while (REG(UART_FR) & UART_FR_TXFF) {
        /* 等待发送FIFO不满 */
    }
    REG(UART_DR) = c;
}
```

## 📚 技术文档下载汇总

### ARM官方文档

1. **ARM Architecture Reference Manual ARMv8-A**
   - 文档编号: ARM DDI 0487
   - 下载链接: https://developer.arm.com/documentation/ddi0487/
   - 描述: ARM64架构的权威参考手册

2. **ARM Cortex-A57 Technical Reference Manual**
   - 文档编号: ARM DDI 0488
   - 下载链接: https://developer.arm.com/documentation/ddi0488/
   - 描述: Cortex-A57处理器技术参考手册

3. **ARM PL011 UART Technical Reference Manual**
   - 文档编号: ARM DDI 0183G
   - 下载链接: https://developer.arm.com/documentation/ddi0183/
   - 描述: PL011 UART控制器技术手册

4. **ARM Generic Interrupt Controller Architecture Specification**
   - 文档编号: ARM IHI 0069
   - 下载链接: https://developer.arm.com/documentation/ihi0069/
   - 描述: GIC中断控制器架构规范

### QEMU相关文档

1. **QEMU System Emulation User's Guide**
   - 下载链接: https://www.qemu.org/docs/master/system/
   - 格式: 在线HTML / PDF下载
   - 描述: QEMU系统仿真用户指南

2. **QEMU源码仓库**
   - Git地址: https://gitlab.com/qemu-project/qemu.git
   - 浏览器: https://gitlab.com/qemu-project/qemu
   - 关键文件: `hw/arm/virt.c`, `hw/char/pl011.c`

### Linux内核相关

1. **Device Tree Specification**
   - 版本: v0.3
   - 下载链接: https://github.com/devicetree-org/devicetree-specification/releases
   - 描述: 设备树规范文档

2. **Linux ARM64启动文档**
   - 位置: Linux内核源码 `Documentation/arm64/booting.rst`
   - 在线: https://www.kernel.org/doc/html/latest/arm64/booting.html

## 🔍 如何查找硬件规格

### 1. QEMU运行时查询

```bash
# 启动QEMU monitor
qemu-system-aarch64 -machine virt -cpu cortex-a57 -monitor stdio

# 查看内存布局
(qemu) info mtree

# 查看设备信息
(qemu) info qtree

# 生成设备树
qemu-system-aarch64 -machine virt -cpu cortex-a57 -machine dumpdtb=virt.dtb
dtc -I dtb -O dts virt.dtb > virt.dts
```

### 2. 设备树分析

```bash
# 查看UART配置
grep -A 10 "pl011@" virt.dts

# 查看内存配置  
grep -A 5 "memory@" virt.dts

# 查看中断控制器
grep -A 10 "intc@" virt.dts
```

### 3. QEMU源码分析

**关键源码文件**:
- `hw/arm/virt.c` - virt machine定义
- `hw/char/pl011.c` - PL011 UART实现
- `hw/intc/arm_gicv2m.c` - GIC中断控制器
- `target/arm/cpu64.c` - ARM64 CPU实现

### 4. 真实硬件规格查询

对于真实ARM64硬件开发：

1. **查看处理器手册**: 根据MIDR_EL1的值确定具体型号
2. **查看板级手册**: 获取内存布局和设备地址
3. **查看设备树**: Linux系统中的 `/proc/device-tree`
4. **查看寄存器**: 通过 `/proc/cpuinfo` 等接口

## 💡 学习建议

### 循序渐进的学习路径

1. **第一步**: 理解基本概念
   - 阅读ARM Architecture Reference Manual的基础章节
   - 理解异常级别和寄存器组织

2. **第二步**: 分析QEMU实现
   - 查看QEMU源码中的硬件定义
   - 理解虚拟硬件与真实硬件的对应关系

3. **第三步**: 实践验证
   - 修改代码中的地址和配置
   - 观察运行结果的变化

4. **第四步**: 扩展应用
   - 尝试添加新的设备支持
   - 移植到真实ARM64硬件

### 推荐学习顺序

1. ARM64基础架构 (本阶段)
2. 异常和中断处理
3. 内存管理和MMU
4. 设备驱动开发
5. 多核处理器支持
6. 虚拟化技术

通过系统的学习这些参考资料，学生将能够深入理解ARM64架构的精髓，为后续的操作系统开发打下坚实基础。 