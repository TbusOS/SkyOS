# 阶段一：ARM32基础与引导

## 🎯 学习目标

通过本阶段的学习，学生将掌握：

1. **ARM32架构基础**
   - ARM Cortex-A15处理器特性
   - ARM寄存器组织 (R0-R15, CPSR, SPSR)
   - ARM处理器模式与异常级别
   - ARM指令集基础

2. **ARM汇编编程**
   - ARM汇编语法和指令
   - 异常向量表设置
   - 堆栈管理和模式切换
   - 内联汇编与C语言混合编程

3. **硬件抽象理解**
   - QEMU virt machine硬件布局
   - 设备树和硬件规格查询
   - UART串口编程
   - 内存映射IO访问

4. **系统启动流程**
   - ARM处理器复位后的执行流程
   - 链接脚本和内存布局
   - BSS段初始化
   - 从汇编到C语言的过渡

## 📚 理论知识

### ARM32架构核心概念

#### 1. 处理器模式
ARM32处理器有7种工作模式：

| 模式 | 编码 | 特权级别 | 用途 |
|------|------|----------|------|
| User (usr) | 10000 | 非特权 | 正常用户程序执行 |
| FIQ | 10001 | 特权 | 快速中断请求处理 |
| IRQ | 10010 | 特权 | 普通中断请求处理 |
| Supervisor (svc) | 10011 | 特权 | 软件中断和复位 |
| Abort (abt) | 10111 | 特权 | 内存访问异常 |
| Undefined (und) | 11011 | 特权 | 未定义指令异常 |
| System (sys) | 11111 | 特权 | 特权用户模式 |

#### 2. 寄存器组织
- **通用寄存器**: R0-R12
- **堆栈指针**: R13 (SP)
- **链接寄存器**: R14 (LR) 
- **程序计数器**: R15 (PC)
- **程序状态寄存器**: CPSR, SPSR

#### 3. 异常向量表
ARM处理器的异常向量表定义了各种异常的处理入口：

```
地址     异常类型              说明
0x00     Reset                复位异常
0x04     Undefined            未定义指令异常  
0x08     SWI                  软件中断
0x0C     Prefetch Abort       预取指令异常
0x10     Data Abort           数据访问异常
0x14     Reserved             保留
0x18     IRQ                  普通中断
0x1C     FIQ                  快速中断
```

## 🗂️ 文件组织

```
stage01/
├── docs/
│   ├── README.md                    # 本文件
│   ├── ARM32硬件规格说明.md         # 硬件规格详解
│   └── theory/
│       ├── arm-assembly.md          # ARM汇编语言
│       ├── exception-handling.md    # 异常处理机制
│       └── memory-layout.md         # 内存布局设计
├── labs/
│   └── 实验01-ARM32启动.md          # 实验指导书
├── code/
│   ├── boot/
│   │   ├── start.S                  # ARM启动汇编代码
│   │   └── boot.lds                 # 链接脚本
│   ├── kernel/
│   │   └── main.c                   # C语言主函数
│   └── Makefile                     # 构建脚本
└── resources/
    ├── qemu-virt-devicetree.dts     # QEMU设备树示例
    └── analyze_hardware.py          # 硬件分析工具
```

## 🚀 快速开始

### 1. 环境准备

```bash
# 安装必要工具
sudo apt-get install gcc-arm-none-eabi qemu-system-arm make device-tree-compiler

# 验证安装
arm-none-eabi-gcc --version
qemu-system-arm --version
```

### 2. 编译运行

```bash
# 进入代码目录
cd stages/stage01/code

# 编译项目
make all

# 在QEMU中运行
make run
```

### 3. 预期输出

```
======================================
    SkyOS - ARM32 教学操作系统
======================================
版本: 0.1.0 (教学演示版)
架构: ARM Cortex-A15
编译时间: Dec 20 2024 15:30:45
--------------------------------------
处理器ID: 0x412FC0F1
当前模式: Supervisor (CPSR: 0x600001D3)
中断状态: IRQ禁用 FIQ禁用
--------------------------------------
内核初始化完成！
======================================

开始心跳显示 (按Ctrl+A X退出QEMU):
心跳 #0x00000000 - SkyOS 正在运行!
心跳 #0x00000001 - SkyOS 正在运行!
...
```

## 🔬 核心技术点

### 1. ARM启动汇编 (start.S)

**关键代码段**:
```assembly
.section .vectors, "ax"
_vectors:
    ldr pc, =reset_handler      @ 复位向量
    ldr pc, =undef_handler      @ 未定义指令
    ldr pc, =swi_handler        @ 软件中断
    ...

reset_handler:
    cpsid if                    @ 禁用中断
    msr cpsr, #0x13            @ 切换到SVC模式
    ldr sp, =svc_stack_top     @ 设置堆栈
    bl main                     @ 跳转到C函数
```

**技术要点**:
- 异常向量表的设置和重定位
- ARM处理器模式切换
- 堆栈指针初始化
- BSS段清零操作

### 2. 内存布局设计 (boot.lds)

**链接脚本结构**:
```ld
MEMORY {
    RAM (rwx) : ORIGIN = 0x40000000, LENGTH = 256M
}

SECTIONS {
    .vectors : { KEEP(*(.vectors)) } > RAM
    .text    : { *(.text*) } > RAM  
    .data    : { *(.data*) } > RAM
    .bss     : { *(.bss*) } > RAM
}
```

**设计原理**:
- QEMU virt machine内存布局分析
- 段的组织和地址分配
- 符号定义和引用

### 3. UART串口编程 (main.c)

**硬件抽象**:
```c
#define UART0_BASE      0x09000000    // 来源: QEMU virt machine
#define UART_DR         (UART0_BASE + 0x00)  // 数据寄存器
#define UART_FR         (UART0_BASE + 0x18)  // 标志寄存器
#define UART_FR_TXFF    (1 << 5)             // 发送FIFO满

void uart_putc(char c) {
    while (REG(UART_FR) & UART_FR_TXFF);    // 等待FIFO不满
    REG(UART_DR) = c;                       // 发送字符
}
```

**技术要点**:
- ARM PL011 UART控制器操作
- 内存映射IO访问
- 硬件寄存器查询和操作

## 📋 实验任务

### 必做任务

1. **环境搭建与编译**
   - 安装ARM交叉编译工具链
   - 成功编译SkyOS stage01代码
   - 在QEMU中运行并观察输出

2. **硬件规格分析**
   - 使用硬件分析工具分析QEMU virt machine
   - 查阅ARM PL011 UART技术手册
   - 验证代码中的硬件地址定义

3. **代码理解与调试**
   - 使用GDB调试启动过程
   - 分析ARM寄存器状态
   - 理解异常向量表的作用

### 扩展任务

1. **功能增强**
   - 添加更多系统信息输出
   - 实现格式化输出函数
   - 增强错误处理机制

2. **性能分析**
   - 测量启动时间
   - 分析指令执行效率
   - 优化关键代码路径

## 🎯 评估标准

### 知识理解 (40%)
- ARM32架构基本概念
- 异常向量表机制
- 内存布局设计原理
- UART编程原理

### 实践能力 (40%)
- 环境搭建和编译
- 代码调试和分析
- 问题定位和解决
- 硬件规格查询

### 创新扩展 (20%)
- 功能改进和优化
- 代码质量和规范
- 文档编写能力
- 学习主动性

## 🔗 参考资料

### 核心文档
- [ARM Architecture Reference Manual DDI 0406C.d](https://developer.arm.com/documentation/ddi0406/cd/)
- [ARM Cortex-A15 Technical Reference Manual DDI 0438](https://developer.arm.com/documentation/ddi0438/i/)
- [ARM PL011 UART TRM DDI 0183G](https://developer.arm.com/documentation/ddi0183/latest/)

### 工具文档
- [QEMU ARM System Emulation](https://qemu.readthedocs.io/en/latest/system/arm/virt.html)
- [GCC ARM Embedded Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm)

### 学习资源
- [ARM汇编语言程序设计](https://www.arm.com/resources/education/education-kits)
- [嵌入式系统设计与实现](https://www.embedded.com/)

## 💡 学习建议

### 理论学习
1. **系统性学习**: 按照文档顺序，先理论后实践
2. **对比学习**: 与x86架构对比，理解RISC特点
3. **深入研究**: 查阅ARM官方文档，不满足于表面理解

### 实践练习
1. **动手操作**: 每个代码片段都要亲自运行验证
2. **调试分析**: 使用GDB深入理解执行过程
3. **修改实验**: 尝试修改代码，观察不同效果

### 问题解决
1. **查阅文档**: 遇到问题首先查阅官方技术手册
2. **系统分析**: 从硬件到软件，系统性分析问题
3. **实验验证**: 通过实验验证理论推测

---

完成本阶段学习后，您将具备ARM32系统编程的基础能力，为后续的中断处理、内存管理等高级主题做好准备。 