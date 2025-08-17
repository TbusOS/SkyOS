# 阶段一：ARM64基础与引导

## 🎯 学习目标

通过本阶段的学习，学生将掌握：

1. **ARM64架构基础**
   - ARM Cortex-A57处理器特性
   - ARM64寄存器组织 (X0-X30, SP, PC)
   - ARM64异常级别 (EL0-EL3)
   - AArch64指令集基础

2. **ARM64汇编编程**
   - ARM64汇编语法和指令
   - 异常向量表设置 (每个条目128字节)
   - 堆栈管理和异常级别切换
   - 内联汇编与C语言混合编程

3. **硬件抽象理解**
   - QEMU virt machine ARM64硬件布局
   - 设备树和硬件规格查询
   - UART串口编程 (PL011)
   - 内存映射IO访问

4. **系统启动流程**
   - ARM64处理器复位后的执行流程
   - 链接脚本和内存布局
   - BSS段初始化
   - 从汇编到C语言的过渡

## 📚 理论知识

### ARM64架构核心概念

#### 1. 异常级别 (Exception Levels)
ARM64处理器有4个异常级别：

| 级别 | 名称 | 特权级别 | 用途 |
|------|------|----------|------|
| EL0 | Application | 非特权 | 用户应用程序 |
| EL1 | Kernel | 特权 | 操作系统内核 |
| EL2 | Hypervisor | 特权 | 虚拟化管理器 |
| EL3 | Secure Monitor | 特权 | 安全监控器 |

#### 2. 寄存器组织
- **通用寄存器**: X0-X30 (64位) / W0-W30 (32位)
- **堆栈指针**: SP (每个EL独立)
- **程序计数器**: PC
- **状态寄存器**: CPSR (通过特殊指令访问)
- **系统寄存器**: 通过MRS/MSR指令访问

#### 3. 异常向量表
ARM64异常向量表包含16个条目，每个128字节：
- Current EL with SP_EL0: 4个条目
- Current EL with SP_ELx: 4个条目  
- Lower EL using AArch64: 4个条目
- Lower EL using AArch32: 4个条目

每组4个条目对应：Synchronous, IRQ, FIQ, SError

## 🧪 实验内容

### 实验一：ARM64基础启动

#### 目标
1. 理解ARM64启动流程
2. 编写基础的ARM64汇编代码
3. 实现UART串口输出
4. 在QEMU中运行ARM64内核

#### 步骤
1. **环境准备**
   ```bash
   # 安装ARM64工具链
   brew install aarch64-elf-gcc
   
   # 安装QEMU
   brew install qemu
   ```

2. **代码分析**
   - 分析 `boot/start.S` 中的异常向量表设置
   - 理解 `boot/boot.lds` 中的内存布局
   - 研究 `kernel/main.c` 中的寄存器访问

3. **编译运行**
   ```bash
   cd stages/stage01-arm64/code
   make all
   make run
   ```

#### 预期输出
```
======================================
    SkyOS - ARM64 教学操作系统
======================================
版本: 0.1.0 (ARM64演示版)
架构: ARM Cortex-A57 (ARMv8-A)
平台: QEMU virt machine
--------------------------------------
处理器ID (MIDR_EL1): 0x411FD070
当前异常级别: EL1 (Kernel) (0x0000000000000001)
多处理器ID (MPIDR_EL1): 0x0000000080000000
系统控制寄存器 (SCTLR_EL1): 0x0000000030100180
--------------------------------------
ARM64特性检查:
  MMU状态: 禁用
  缓存状态: 禁用
  地址对齐检查: 禁用
--------------------------------------
内核初始化完成！
======================================

开始心跳显示 (按Ctrl+A X退出QEMU):
💓 心跳 #0000000000000000 - SkyOS ARM64 正在运行!
💓 心跳 #0000000000000001 - SkyOS ARM64 正在运行!
...
```

## 🔍 硬件规格背景

### QEMU virt machine ARM64

**内存布局**:
- RAM起始地址: 0x40000000
- RAM大小: 256MB
- UART0基址: 0x09000000 (ARM PL011)

**关键寄存器**:
- MIDR_EL1: 处理器标识寄存器
- CurrentEL: 当前异常级别
- MPIDR_EL1: 多处理器亲和性寄存器
- SCTLR_EL1: 系统控制寄存器

### 参考文档
- [ARM Architecture Reference Manual ARMv8](https://developer.arm.com/documentation/ddi0487/)
- [QEMU ARM System Emulation](https://www.qemu.org/docs/master/system/target-arm.html)
- [ARM Cortex-A57 TRM](https://developer.arm.com/documentation/ddi0488/)

## 🤔 思考题

1. ARM64异常向量表为什么每个条目要占128字节？
2. ARM64的异常级别与ARM32的处理器模式有什么区别？
3. QEMU virt machine的内存布局是如何定义的？
4. ARM64启动时如何从异常向量表跳转到C代码？
5. 为什么要在启动时清零BSS段？

## 📈 扩展练习

1. 修改代码，添加更多ARM64系统寄存器的读取
2. 实现简单的异常处理程序
3. 添加对ARM64特性寄存器的解析
4. 尝试启用MMU和缓存 