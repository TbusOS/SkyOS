# ARM异常处理技术手册

## 🎯 文档目的

本文档深入介绍ARM32异常处理机制的技术细节，为学习SkyOS阶段2提供完整的理论基础和实践指导。

## 📚 ARM异常处理基础

### ARM异常类型详解

ARM32架构定义了7种类型的异常，每种异常都有特定的用途和处理方式：

| 异常类型 | 向量地址 | 优先级 | 进入模式 | 触发条件 |
|----------|----------|--------|----------|----------|
| Reset | 0x00 | 1 (最高) | Supervisor | 系统复位或上电 |
| Undefined Instruction | 0x04 | 7 (最低) | Undefined | 执行未定义指令 |
| Software Interrupt (SVC) | 0x08 | 6 | Supervisor | 执行SVC/SWI指令 |
| Prefetch Abort | 0x0C | 5 | Abort | 指令预取失败 |
| Data Abort | 0x10 | 2 | Abort | 数据访问失败 |
| IRQ | 0x18 | 4 | IRQ | 外部中断请求 |
| FIQ | 0x1C | 3 | FIQ | 快速中断请求 |

### ARM处理器模式

ARM32有7种处理器模式，每种模式有不同的权限和寄存器组织：

```
用户模式 (User, 0x10)
  - 非特权模式
  - 用户应用程序运行模式
  - 访问受限的系统资源

系统模式 (System, 0x1F)  
  - 特权模式
  - 与用户模式共享寄存器
  - 用于特权级的用户程序

监管模式 (Supervisor, 0x13)
  - 特权模式
  - 操作系统内核模式
  - Reset和SWI异常进入此模式

中断模式 (IRQ, 0x12)
  - 特权模式
  - 处理IRQ中断
  - 有独立的SP和LR

快速中断模式 (FIQ, 0x11)
  - 特权模式
  - 处理FIQ中断
  - 有独立的R8-R14寄存器

中止模式 (Abort, 0x17)
  - 特权模式
  - 处理内存访问异常
  - 有独立的SP和LR

未定义模式 (Undefined, 0x1B)
  - 特权模式
  - 处理未定义指令异常
  - 有独立的SP和LR
```

## 🔧 异常处理机制

### 异常发生时的硬件行为

当异常发生时，ARM处理器自动执行以下步骤：

1. **保存返回地址**：将PC保存到目标模式的LR寄存器
2. **保存处理器状态**：将CPSR保存到目标模式的SPSR寄存器
3. **更新CPSR**：设置新的处理器模式，禁用中断
4. **跳转执行**：PC设置为对应的异常向量地址

### 异常返回机制

异常处理完成后，需要正确返回到被中断的程序：

```assembly
@ 从异常返回的标准方法
ldmfd sp!, {r0-r12, pc}^    @ 恢复寄存器并返回，^表示同时恢复CPSR

@ 或者使用明确的返回指令
movs pc, lr                 @ s后缀表示同时恢复CPSR
```

### 异常向量表布局

```assembly
.section .vectors, "ax"
_vectors:
    ldr pc, =reset_handler      @ 0x00: Reset
    ldr pc, =undef_handler      @ 0x04: Undefined Instruction
    ldr pc, =swi_handler        @ 0x08: Software Interrupt
    ldr pc, =prefetch_handler   @ 0x0C: Prefetch Abort
    ldr pc, =data_handler       @ 0x10: Data Abort
    nop                         @ 0x14: Reserved
    ldr pc, =irq_handler        @ 0x18: IRQ
    ldr pc, =fiq_handler        @ 0x1C: FIQ
```

## 💻 系统调用机制(SVC)

### SVC指令详解

软件中断指令(SVC，以前称为SWI)用于实现系统调用：

```assembly
svc #immed_24    @ 触发软件中断，immed_24是24位立即数
```

### 系统调用参数传递

标准的ARM调用约定(AAPCS)：
- R0-R3：传递参数和返回值
- R4-R11：被调用者保存的寄存器
- R12：临时寄存器
- R13(SP)：堆栈指针
- R14(LR)：链接寄存器
- R15(PC)：程序计数器

### SVC异常处理流程

```assembly
swi_handler:
    @ 保存上下文
    stmfd sp!, {r0-r12, lr}
    
    @ 获取SVC指令中的立即数
    ldr r0, [lr, #-4]           @ 读取触发异常的指令
    bic r0, r0, #0xFF000000     @ 提取24位立即数
    
    @ 调用C语言处理函数
    mov r1, sp                  @ 传递寄存器帧指针
    bl handle_swi
    
    @ 恢复上下文并返回
    ldmfd sp!, {r0-r12, pc}^
```

## 🚨 故障异常处理

### 数据访问异常(Data Abort)

数据访问异常发生时，ARM提供了详细的故障信息：

```c
/* 读取故障状态寄存器 */
uint32_t dfsr, far;
asm volatile("mrc p15, 0, %0, c5, c0, 0" : "=r"(dfsr));  // Data Fault Status
asm volatile("mrc p15, 0, %0, c6, c0, 0" : "=r"(far));   // Fault Address

/* 解析故障类型 */
switch (dfsr & 0xF) {
    case 0x1: /* Alignment fault */
    case 0x3: /* Access flag fault */
    case 0x5: /* Translation fault (section) */
    case 0x7: /* Translation fault (page) */
    case 0x9: /* Domain fault (section) */
    case 0xB: /* Domain fault (page) */
    case 0xD: /* Permission fault (section) */
    case 0xF: /* Permission fault (page) */
}
```

### 指令预取异常(Prefetch Abort)

```c
/* 读取指令故障状态寄存器 */
uint32_t ifsr, ifar;
asm volatile("mrc p15, 0, %0, c5, c0, 1" : "=r"(ifsr));  // Instruction Fault Status
asm volatile("mrc p15, 0, %0, c6, c0, 2" : "=r"(ifar));  // Instruction Fault Address
```

## 📊 中断控制

### CPSR中断控制位

CPSR(Current Program Status Register)的中断控制位：

```
Bit 7 (I): IRQ disable bit
  0 = IRQ interrupts enabled
  1 = IRQ interrupts disabled

Bit 6 (F): FIQ disable bit  
  0 = FIQ interrupts enabled
  1 = FIQ interrupts disabled

Bits 4-0: Processor mode
  0x10 = User
  0x11 = FIQ
  0x12 = IRQ
  0x13 = Supervisor
  0x17 = Abort
  0x1B = Undefined
  0x1F = System
```

### 中断使能/禁用

```assembly
@ 禁用IRQ中断
mrs r0, cpsr
orr r0, r0, #0x80
msr cpsr_c, r0

@ 启用IRQ中断
mrs r0, cpsr
bic r0, r0, #0x80
msr cpsr_c, r0

@ 使用CPS指令(ARMv6+)
cpsid i        @ 禁用IRQ
cpsie i        @ 启用IRQ
cpsid if       @ 禁用IRQ和FIQ
cpsie if       @ 启用IRQ和FIQ
```

## 📖 官方技术文档

### ARM官方文档

1. **ARM Architecture Reference Manual ARMv7-A**
   - 下载链接：https://developer.arm.com/documentation/ddi0406/
   - 描述：完整的ARMv7-A架构参考手册
   - 重点章节：B1.8 Exception handling, B1.9 Exception types

2. **ARM Cortex-A15 Technical Reference Manual**
   - 下载链接：https://developer.arm.com/documentation/ddi0438/
   - 描述：Cortex-A15处理器技术参考手册
   - 重点章节：第3章 Programmers Model, 第9章 Exception Handling

3. **ARM Generic Interrupt Controller v2.0**
   - 下载链接：https://developer.arm.com/documentation/ihi0048/
   - 描述：GIC架构规范和编程接口
   - 重点章节：第3章 GIC Architecture, 第4章 Distributor

### 处理器实现指南

4. **ARM Cortex-A Series Programmer's Guide**
   - 下载链接：https://developer.arm.com/documentation/den0013/
   - 描述：Cortex-A系列编程指南
   - 重点章节：第10章 Exception handling, 第18章 Interrupt handling

5. **ARM System Developer's Guide**
   - 下载链接：https://developer.arm.com/documentation/den0024/
   - 描述：ARM系统开发者指南(AArch64为主，但原理通用)
   - 重点章节：第9章 Exception levels, 第10章 Interrupt handling

### QEMU虚拟化文档

6. **QEMU ARM System Emulation**
   - 链接：https://www.qemu.org/docs/master/system/target-arm.html
   - 描述：QEMU ARM系统仿真文档
   - 重点：virt machine的中断控制器配置

7. **QEMU virt machine specification**
   - 链接：https://qemu-project.gitlab.io/qemu/system/arm/virt.html
   - 描述：QEMU virt machine硬件规格
   - 重点：内存布局、设备地址、中断号分配

## 🔬 调试技术

### GDB异常调试

```bash
# 启动QEMU调试模式
qemu-system-arm -machine virt -cpu cortex-a15 -m 256M -nographic \
                -kernel skyos.elf -s -S

# 连接GDB
arm-none-eabi-gdb skyos.elf
(gdb) target remote localhost:1234
(gdb) break *0x40000008    # 在SWI向量设断点
(gdb) break handle_swi     # 在处理函数设断点
(gdb) continue
```

### 异常信息收集

```c
struct exception_context {
    uint32_t registers[13];    // R0-R12
    uint32_t lr;               // 返回地址
    uint32_t spsr;             // 保存的程序状态
    uint32_t cpsr;             // 当前程序状态
    uint32_t fault_addr;       // 故障地址(如果适用)
    uint32_t fault_status;     // 故障状态(如果适用)
    uint32_t exception_type;   // 异常类型
    uint64_t timestamp;        // 时间戳
};
```

## 🎯 实践练习

### 练习1：触发不同异常

```c
// 未定义指令异常
asm volatile(".word 0xFFFFFFFF");

// 数据访问异常
volatile uint32_t *invalid = (uint32_t*)0xFFFFFFFF;
*invalid = 0x12345678;

// 地址对齐异常
volatile uint32_t *misaligned = (uint32_t*)0x40000001;
*misaligned = 0x12345678;
```

### 练习2：系统调用实现

```c
// 定义系统调用号
#define SYS_WRITE    1
#define SYS_READ     2  
#define SYS_EXIT     3

// 系统调用包装函数
static inline int syscall(int num, int arg1, int arg2, int arg3) {
    int result;
    asm volatile(
        "mov r0, %1\n"
        "mov r1, %2\n"
        "mov r2, %3\n"
        "mov r3, %4\n"
        "svc #0\n"
        "mov %0, r0\n"
        : "=r"(result)
        : "r"(num), "r"(arg1), "r"(arg2), "r"(arg3)
        : "r0", "r1", "r2", "r3"
    );
    return result;
}
```

### 练习3：异常性能分析

```c
// 测量异常处理延迟
void measure_exception_overhead(void) {
    uint64_t start, end;
    
    // 获取开始时间
    asm volatile("mrrc p15, 0, %Q0, %R0, c14" : "=r"(start));
    
    // 触发系统调用
    syscall(SYS_WRITE, 1, (int)"test", 4);
    
    // 获取结束时间  
    asm volatile("mrrc p15, 0, %Q0, %R0, c14" : "=r"(end));
    
    uint32_t cycles = (uint32_t)(end - start);
    uart_puts("Exception overhead: ");
    uart_put_hex(cycles);
    uart_puts(" cycles\n");
}
```

## 📝 最佳实践

### 异常处理设计原则

1. **保持简洁**：异常处理程序应该尽可能简单和快速
2. **完整保存上下文**：确保所有必要的寄存器都被保存和恢复
3. **错误处理**：提供详细的错误信息，便于调试
4. **可重入性**：考虑异常处理的可重入性问题
5. **性能优化**：关键路径要优化，减少延迟

### 系统调用接口设计

1. **统一接口**：使用统一的调用约定和参数传递方式
2. **错误处理**：定义清楚的错误码和返回值约定
3. **参数验证**：在内核中验证用户传递的参数
4. **安全检查**：防止权限提升和越界访问
5. **向后兼容**：保持系统调用接口的稳定性

## 🔗 相关资源

### 开源项目参考

- **Linux Kernel**：https://github.com/torvalds/linux
  - `arch/arm/kernel/entry-armv.S` - ARM异常入口
  - `arch/arm/kernel/traps.c` - 异常处理
- **U-Boot**：https://github.com/u-boot/u-boot
  - `arch/arm/cpu/armv7/start.S` - ARM启动代码
- **Xinu OS**：https://github.com/xinu-os/xinu
  - 简单的教学操作系统实现

### 在线资源

- **ARM Community**：https://community.arm.com/
- **ARM Developer**：https://developer.arm.com/
- **QEMU Documentation**：https://www.qemu.org/docs/

通过学习本手册并结合实际编程练习，您将深入理解ARM异常处理机制，为开发高质量的嵌入式系统和操作系统奠定坚实基础。 