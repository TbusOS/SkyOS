# ARM32硬件规格说明文档

## 🎯 文档目的

本文档详细解释SkyOS中使用的ARM32硬件规格、内存布局、寄存器地址等技术细节的来源，帮助学生理解"这些神奇数字"是从哪里来的，并提供具体的文档下载链接。

## 📋 QEMU virt machine硬件规格

### 为什么选择QEMU virt machine？

1. **标准化**: virt machine是QEMU提供的标准化ARM虚拟机
2. **文档完善**: 有详细的硬件规格文档
3. **教学友好**: 简化了真实硬件的复杂性
4. **易于调试**: 支持GDB调试和JTAG接口

### QEMU virt machine官方文档

**主要参考文档**:
- **QEMU ARM系统仿真**: https://www.qemu.org/docs/master/system/target-arm.html
- **QEMU virt machine专页**: https://qemu-project.gitlab.io/qemu/system/arm/virt.html
- **QEMU源码仓库**: https://gitlab.com/qemu-project/qemu
- **QEMU源码文件**: `hw/arm/virt.c` (定义内存映射)
- **ARM Cortex-A15 TRM**: ARM Technical Reference Manual

## 🗺️ 内存布局详解

### 1. 为什么异常向量表在0x40000000？

#### ARM架构的异常向量表机制

**文档来源**: ARM Architecture Reference Manual ARMv7-A and ARMv7-R edition
- **文档编号**: ARM DDI 0406C.d
- **下载链接**: https://developer.arm.com/documentation/ddi0406/cd/
- **相关章节**: Section B1.8.1 "Exception vectors"

ARM处理器的异常向量表地址由VBAR（Vector Base Address Register）决定：

```c
// ARM Cortex-A15的默认异常向量表基地址
// 参考: ARM DDI 0406C.d, Section B4.1.156
uint32_t vbar = 0x40000000;  // QEMU virt machine的默认值
```

#### QEMU virt machine的内存映射

**文档来源**: QEMU源码
- **源码文件**: `hw/arm/virt.c`
- **Git仓库**: https://gitlab.com/qemu-project/qemu.git
- **在线查看**: https://gitlab.com/qemu-project/qemu/-/blob/master/hw/arm/virt.c

查看QEMU源码 `hw/arm/virt.c`：

```c
// QEMU virt machine内存布局 (来源: hw/arm/virt.c, line ~140)
static const MemMapEntry base_memmap[] = {
    [VIRT_FLASH] =              {          0, 0x08000000 },
    [VIRT_CPUPERIPHS] =         { 0x08000000, 0x00020000 },
    [VIRT_UART] =               { 0x09000000, 0x00001000 },
    [VIRT_RTC] =                { 0x09010000, 0x00001000 },
    [VIRT_FW_CFG] =             { 0x09020000, 0x00000018 },
    [VIRT_GPIO] =               { 0x09030000, 0x00001000 },
    [VIRT_SECURE_UART] =        { 0x09040000, 0x00001000 },
    [VIRT_SMMU] =               { 0x09050000, 0x00020000 },
    [VIRT_PCDIMM_ACPI] =        { 0x09070000, ACPI_PCIHP_SIZE},
    [VIRT_ACPI_GED] =           { 0x09080000, ACPI_GED_EVT_SEL_LEN},
    [VIRT_NVDIMM_ACPI] =        { 0x09090000, NVDIMM_ACPI_IO_LEN},
    [VIRT_PVTIME] =             { 0x090a0000, 0x00010000 },
    [VIRT_SECURE_GPIO] =        { 0x090b0000, 0x00001000 },
    [VIRT_MMIO] =               { 0x0a000000, 0x00000200 },
    // ... 更多设备映射
    [VIRT_MEM] =                { 0x40000000, LEGACY_RAMLIMIT_BYTES },
    // 主内存从0x40000000开始
};
```

#### 具体查询方法

**方法1: 查看QEMU启动信息**
```bash
qemu-system-arm -machine virt -cpu cortex-a15 -m 256M -nographic -kernel /dev/null -S -monitor stdio
(qemu) info mtree
```

**方法2: 查看设备树信息**
```bash
qemu-system-arm -machine virt -cpu cortex-a15 -m 256M -machine dumpdtb=virt.dtb
dtc -I dtb -O dts virt.dtb
```

**方法3: 查看QEMU源码**
```bash
# 下载QEMU源码
git clone https://gitlab.com/qemu-project/qemu.git
# 查看ARM virt machine定义
cat qemu/hw/arm/virt.c | grep -A 20 "base_memmap"
```

### 2. 链接脚本中的内存配置

我们的 `boot/boot.lds`：

```ld
MEMORY
{
    RAM (rwx) : ORIGIN = 0x40000000, LENGTH = 256M
}
```

**配置依据**: 
- **QEMU源码**: `hw/arm/virt.c` 中的 `VIRT_MEM` 定义
- **验证方法**: 设备树中的 `memory@40000000` 节点

## 🔌 UART串口寄存器详解

### 1. UART基地址0x09000000的来源

#### 查询方法1: QEMU设备树

**文档来源**: QEMU生成的设备树
- **生成命令**: `qemu-system-arm -machine virt -cpu cortex-a15 -m 256M -machine dumpdtb=virt.dtb`
- **转换命令**: `dtc -I dtb -O dts virt.dtb > virt.dts`

```bash
# 生成设备树文件
qemu-system-arm -machine virt -cpu cortex-a15 -m 256M -machine dumpdtb=virt.dtb
dtc -I dtb -O dts virt.dtb > virt.dts

# 查看UART配置
grep -A 10 "uart@" virt.dts
```

输出示例：
```dts
pl011@9000000 {
    clock-names = "uartclk", "apb_pclk";
    clocks = <0x8000 0x8000>;
    interrupts = <0x00 0x01 0x04>;
    reg = <0x00 0x9000000 0x00 0x1000>;
    compatible = "arm,pl011", "arm,primecell";
};
```

#### 查询方法2: QEMU信息命令

```bash
qemu-system-arm -machine virt -cpu cortex-a15 -nographic -S -monitor stdio
(qemu) info qtree
```

#### 查询方法3: 查看QEMU源码

**源码位置**: `hw/arm/virt.c` 中的定义
- **Git链接**: https://gitlab.com/qemu-project/qemu/-/blob/master/hw/arm/virt.c
- **具体行号**: 约第140行

```c
static const MemMapEntry base_memmap[] = {
    // ...
    [VIRT_UART] = { 0x09000000, 0x00001000 },
    // ...
};
```

### 2. PL011 UART寄存器规格

QEMU virt machine使用ARM PL011 UART控制器。

#### 官方文档来源
- **文档标题**: ARM PrimeCell UART (PL011) Technical Reference Manual
- **文档编号**: ARM DDI 0183G
- **发布日期**: 2005-2017
- **下载链接**: https://developer.arm.com/documentation/ddi0183/latest/
- **备用链接**: https://documentation-service.arm.com/static/5e8e1323fd22d71e3a45cb1b

#### PL011寄存器映射表

**文档来源**: ARM DDI 0183G, Table 3-2 "Summary of PrimeCell UART registers"

| 偏移地址 | 寄存器名称 | 描述 | 我们的定义 | 文档章节 |
|----------|------------|------|------------|----------|
| 0x000 | UARTDR | 数据寄存器 | `UART_DR` | Section 3.3.1 |
| 0x004 | UARTRSR/UARTECR | 接收状态/错误清除寄存器 | - | Section 3.3.2 |
| 0x018 | UARTFR | 标志寄存器 | `UART_FR` | Section 3.3.6 |
| 0x020 | UARTILPR | IrDA低功耗计数器寄存器 | - | Section 3.3.7 |
| 0x024 | UARTIBRD | 整数波特率寄存器 | - | Section 3.3.8 |
| 0x028 | UARTFBRD | 小数波特率寄存器 | - | Section 3.3.9 |
| 0x02C | UARTLCR_H | 线控制寄存器 | - | Section 3.3.10 |
| 0x030 | UARTCR | 控制寄存器 | - | Section 3.3.11 |

#### UARTFR寄存器位域定义

**文档来源**: ARM DDI 0183G, Section 3.3.6 "Flag Register, UARTFR"

```c
// 来源: ARM PL011 TRM DDI 0183G, Section 3.3.6, Table 3-10
#define UART_FR_TXFF    (1 << 5)   // 发送FIFO满 (Bit 5)
#define UART_FR_RXFE    (1 << 4)   // 接收FIFO空 (Bit 4)
#define UART_FR_BUSY    (1 << 3)   // UART忙碌 (Bit 3)
#define UART_FR_DCD     (1 << 2)   // 数据载波检测 (Bit 2)
#define UART_FR_DSR     (1 << 1)   // 数据集就绪 (Bit 1)
#define UART_FR_CTS     (1 << 0)   // 清除发送 (Bit 0)
```

**验证方法**: 下载完整的PL011技术参考手册进行对照

### 3. 寄存器访问代码解释

```c
// kernel/main.c 中的代码及其来源
#define UART0_BASE      0x09000000          // 来源: QEMU hw/arm/virt.c, line ~145
#define UART_DR         (UART0_BASE + 0x00) // 来源: ARM DDI 0183G, Table 3-2
#define UART_FR         (UART0_BASE + 0x18) // 来源: ARM DDI 0183G, Table 3-2
#define UART_FR_TXFF    (1 << 5)            // 来源: ARM DDI 0183G, Section 3.3.6

void uart_putc(char c) {
    // 等待发送FIFO不满 - 编程模型来源: ARM DDI 0183G, Section 2.3.2
    while (REG(UART_FR) & UART_FR_TXFF) {
        /* 空等待 */
    }
    
    // 发送字符 - 数据寄存器使用方法: ARM DDI 0183G, Section 3.3.1
    REG(UART_DR) = c;
}
```

## 🔧 ARM系统寄存器详解

### 协处理器寄存器访问

```c
// 获取处理器ID的指令来源
uint32_t get_processor_id(void) {
    uint32_t id;
    asm volatile("mrc p15, 0, %0, c0, c0, 0" : "=r"(id));
    return id;
}
```

**指令编码来源**: 
- **文档**: ARM Architecture Reference Manual ARMv7-A and ARMv7-R edition
- **文档编号**: ARM DDI 0406C.d  
- **下载链接**: https://developer.arm.com/documentation/ddi0406/cd/
- **具体章节**: Section B4.1.106 "MIDR, Main ID Register"

**指令格式说明**:
- `p15`: 系统控制协处理器 (参考: Section B4.1.4)
- `c0, c0, 0`: Main ID Register (MIDR) 编码 (参考: Table B4-2)
- **功能**: 读取处理器标识信息

### CPSR寄存器详解

```c
// 获取当前程序状态寄存器(CPSR)
uint32_t get_cpsr(void) {
    uint32_t cpsr;
    asm volatile("mrs %0, cpsr" : "=r"(cpsr));
    return cpsr;
}
```

**寄存器定义来源**:
- **文档**: ARM DDI 0406C.d
- **章节**: Section A2.5 "Program status registers"
- **位域定义**: Figure A2-1 "CPSR bit assignments"

| 位域 | 名称 | 描述 | 文档章节 |
|------|------|------|----------|
| [31] | N | 负数标志 | A2.5.1 |
| [30] | Z | 零标志 | A2.5.1 |
| [29] | C | 进位标志 | A2.5.1 |
| [28] | V | 溢出标志 | A2.5.1 |
| [7] | I | IRQ中断屏蔽 | A2.5.2 |
| [6] | F | FIQ中断屏蔽 | A2.5.2 |
| [4:0] | M[4:0] | 处理器模式 | A2.5.3 |

## 🔍 如何查找硬件规格信息

### 1. 对于QEMU虚拟机

#### 方法一：查看QEMU文档
```bash
# 在线文档
https://qemu.readthedocs.io/en/latest/system/arm/virt.html

# 本地帮助
qemu-system-arm -machine help
qemu-system-arm -machine virt,help
```

#### 方法二：设备树分析
```bash
# 生成并分析设备树
qemu-system-arm -machine virt -cpu cortex-a15 -machine dumpdtb=virt.dtb
dtc -I dtb -O dts virt.dtb > virt.dts
less virt.dts
```

#### 方法三：运行时查询
```bash
# QEMU monitor命令
qemu-system-arm -machine virt -monitor stdio
(qemu) info mtree      # 内存布局
(qemu) info qtree      # 设备树
(qemu) info registers  # 寄存器状态
```

#### 方法四：源码分析
**QEMU源码仓库**: https://gitlab.com/qemu-project/qemu
```bash
git clone https://gitlab.com/qemu-project/qemu.git
# 查看关键文件
less qemu/hw/arm/virt.c           # ARM virt machine定义
less qemu/include/hw/arm/virt.h   # 内存映射常量
```

### 2. 对于真实ARM硬件

#### 树莓派示例
**官方文档**: 
- **BCM2835 ARM Peripherals**: https://datasheets.raspberrypi.org/bcm2835/bcm2835-peripherals.pdf
- **BCM2711 ARM Peripherals**: https://datasheets.raspberrypi.org/bcm2711/bcm2711-peripherals.pdf

```bash
# 查看设备树
cat /proc/device-tree/...
# 或
dtc -I fs /proc/device-tree

# 查看内存映射
cat /proc/iomem

# 查看中断
cat /proc/interrupts
```

#### BeagleBone Black示例
**官方文档**:
- **AM335x Technical Reference Manual**: https://www.ti.com/lit/ug/spruh73q/spruh73q.pdf
- **AM335x Datasheet**: https://www.ti.com/lit/ds/symlink/am3359.pdf

```bash
# 查看引脚映射
cat /sys/kernel/debug/pinctrl/44e10800.pinmux/pins

# 查看设备树覆盖
ls /proc/device-tree/
```

### 3. ARM处理器规格查询

#### ARM官方文档下载中心
**主站点**: https://developer.arm.com/documentation

#### 核心文档列表

| 文档标题 | 编号 | 版本 | 下载链接 | 说明 |
|----------|------|------|----------|------|
| ARM Architecture Reference Manual ARMv7-A | DDI 0406 | C.d | https://developer.arm.com/documentation/ddi0406/cd/ | ARM架构规范 |
| ARM Cortex-A15 Technical Reference Manual | DDI 0438 | r4p0 | https://developer.arm.com/documentation/ddi0438/i/ | Cortex-A15处理器 |
| ARM Generic Interrupt Controller Architecture | IHI 0048 | B.b | https://developer.arm.com/documentation/ihi0048/bb/ | GIC中断控制器 |
| ARM PrimeCell UART (PL011) TRM | DDI 0183 | G | https://developer.arm.com/documentation/ddi0183/latest/ | UART控制器 |
| ARM PrimeCell GPIO (PL061) TRM | DDI 0190 | B | https://developer.arm.com/documentation/ddi0190/b/ | GPIO控制器 |
| ARM PrimeCell RTC (PL031) TRM | DDI 0224 | C | https://developer.arm.com/documentation/ddi0224/c/ | RTC实时时钟 |

#### 协处理器寄存器快速查询

**常用系统寄存器编码** (来源: ARM DDI 0406C.d, Appendix B4):

```c
// 处理器标识寄存器
#define CP15_MIDR       "p15, 0, %0, c0, c0, 0"   // Main ID Register
#define CP15_CTR        "p15, 0, %0, c0, c0, 1"   // Cache Type Register
#define CP15_TCMTR      "p15, 0, %0, c0, c0, 2"   // TCM Type Register
#define CP15_TLBTR      "p15, 0, %0, c0, c0, 3"   // TLB Type Register

// 系统控制寄存器
#define CP15_SCTLR      "p15, 0, %0, c1, c0, 0"   // System Control Register
#define CP15_ACTLR      "p15, 0, %0, c1, c0, 1"   // Auxiliary Control Register

// 内存管理寄存器
#define CP15_TTBR0      "p15, 0, %0, c2, c0, 0"   // Translation Table Base Register 0
#define CP15_TTBR1      "p15, 0, %0, c2, c0, 1"   // Translation Table Base Register 1
#define CP15_TTBCR      "p15, 0, %0, c2, c0, 2"   // Translation Table Base Control Register

// 使用示例
static inline uint32_t read_midr(void) {
    uint32_t value;
    asm volatile("mrc " CP15_MIDR : "=r"(value));
    return value;
}
```

## 📚 推荐学习路径

### 第一步：获取核心文档
1. **下载ARM DDI 0406C.d** - ARM架构参考手册
2. **下载ARM DDI 0438** - Cortex-A15技术参考手册  
3. **下载ARM DDI 0183G** - PL011 UART技术参考手册
4. **克隆QEMU源码** - 了解虚拟硬件实现

### 第二步：实践验证
1. **生成设备树文件** - 查看实际硬件布局
2. **对比文档和代码** - 验证地址和寄存器定义
3. **使用调试工具** - GDB查看寄存器状态
4. **阅读内核源码** - 参考Linux内核驱动实现

### 第三步：扩展学习
1. **研究其他ARM SoC** - 如树莓派、BeagleBone等
2. **学习设备树语法** - 深入理解硬件描述
3. **实践硬件移植** - 将代码移植到真实硬件
4. **参与开源项目** - 贡献ARM相关代码

## 🎯 学习建议

### 对于初学者
1. **从官方文档开始**: 不要依赖二手资料
2. **建立文档库**: 下载并整理所有相关文档
3. **动手验证**: 每个地址都要亲自验证
4. **做好笔记**: 记录文档章节和页码

### 对于进阶学习
1. **深入源码**: 阅读QEMU、Linux内核源码
2. **比较实现**: 不同厂商的ARM SoC差异
3. **性能优化**: 理解ARM架构的性能特性
4. **安全特性**: 学习ARM TrustZone等安全机制

## ⚠️ 重要提醒

1. **版本控制**: ARM文档会更新，注意版本号
2. **许可协议**: 遵守ARM和各厂商的文档使用协议
3. **准确性**: 以官方文档为准，网络资料仅供参考
4. **实践验证**: 所有理论都要通过实际代码验证

---

**记住**: 所有的"神奇数字"都有其明确来源，关键是知道在哪里查找官方文档和如何验证！养成查阅原始文档的习惯，这是成为优秀系统程序员的基本素养。 