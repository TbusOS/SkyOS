# SkyOS - ARM32教学类操作系统

## 🌟 项目简介
SkyOS是一个专为教学设计的ARM32架构简化版操作系统，旨在帮助学生循序渐进地理解ARM处理器和操作系统的核心概念和实现原理。

## 🎯 学习目标
- 理解ARM32处理器架构特性
- 掌握ARM汇编语言编程
- 学习ARM内存管理单元(MMU)
- 理解ARM异常和中断处理
- 掌握嵌入式系统启动流程
- 学习设备树(Device Tree)机制

## 📖 学习路径

### 阶段一：ARM32基础与引导 (Week 1-2)
- **目标**: 理解ARM32架构、寄存器、指令集
- **实践**: 编写ARM汇编引导程序，设置堆栈
- **输出**: 在ARM开发板或QEMU上显示"Hello SkyOS"
- **重点**: ARM处理器模式、CPSR寄存器、异常向量表

### 阶段二：ARM异常处理与中断 (Week 3-4)  
- **目标**: 理解ARM异常模型、中断控制器(GIC)
- **实践**: 实现异常向量表、中断服务程序
- **输出**: 支持定时器中断的基础内核
- **重点**: SVC模式、IRQ/FIQ处理、上下文保存恢复

### 阶段三：ARM内存管理 (Week 5-7)
- **目标**: 理解ARM MMU、页表结构、虚拟内存
- **实践**: 配置MMU、实现二级页表、内存分配器
- **输出**: 支持虚拟内存的内核
- **重点**: Translation Table Base Register、页表遍历、缓存一致性

### 阶段四：进程管理与上下文切换 (Week 8-10)
- **目标**: ARM上下文切换、进程控制块、调度器
- **实践**: 实现进程创建、ARM上下文切换汇编代码
- **输出**: 支持多进程的操作系统
- **重点**: 寄存器保存恢复、用户态/内核态切换、系统调用

### 阶段五：设备树与驱动框架 (Week 11-13)
- **目标**: 理解设备树、平台设备驱动模型
- **实践**: 解析设备树、实现UART/GPIO驱动
- **输出**: 支持设备树的驱动框架
- **重点**: DTS/DTB、设备匹配、寄存器映射

### 阶段六：文件系统与存储 (Week 14-15)
- **目标**: 简单文件系统、SD卡/eMMC访问
- **实践**: 实现FAT32文件系统、块设备驱动
- **输出**: 支持文件读写的完整系统
- **重点**: 块设备抽象、缓存机制、文件操作

### 阶段七：系统优化与调试 (Week 16)
- **目标**: 性能优化、调试工具、实际硬件移植
- **实践**: 添加性能计数器、JTAG调试支持
- **输出**: 可在真实ARM硬件上运行的教学OS
- **重点**: ARM性能监控、硬件调试、移植适配

## 🛠️ 开发环境
- **目标架构**: ARM Cortex-A系列 (ARMv7-A)
- **编程语言**: C/ARM Assembly
- **交叉编译器**: arm-none-eabi-gcc
- **调试工具**: QEMU (virt machine), OpenOCD, GDB
- **开发板**: 树莓派3/4, BeagleBone Black 或 QEMU虚拟机
- **构建系统**: Make/CMake

## 📁 项目结构
```
SkyOS/
├── docs/              # 教学文档
│   ├── arm32/         # ARM32架构文档
│   ├── theory/        # 理论基础
│   └── labs/          # 实验指导
├── boot/              # ARM引导程序
│   ├── start.S        # 启动汇编代码
│   └── boot.lds       # 链接脚本
├── kernel/            # 内核源码
│   ├── arch/arm/      # ARM架构相关代码
│   ├── mm/            # 内存管理
│   ├── sched/         # 进程调度
│   └── drivers/       # 设备驱动
├── include/           # 头文件
│   └── asm/           # ARM汇编接口
├── devicetree/        # 设备树文件
├── tools/             # 开发工具
│   ├── mkimage/       # 镜像制作工具
│   └── debugger/      # 调试脚本
└── tests/             # 测试代码
```

## 🚀 快速开始
```bash
# 安装ARM交叉编译工具链
sudo apt-get install gcc-arm-none-eabi

# 构建系统
make ARCH=arm32 all

# 在QEMU ARM虚拟机中运行
make ARCH=arm32 qemu

# 烧录到SD卡 (真实硬件)
make ARCH=arm32 sdcard

# 通过OpenOCD调试
make ARCH=arm32 debug
```

## 🎯 ARM32特殊学习重点

### 1. ARM处理器模式
- User/System/Supervisor/IRQ/FIQ/Abort/Undefined模式
- 模式切换与特权级别管理

### 2. ARM汇编编程
- ARM指令集与Thumb指令集
- 条件执行、桶形移位器
- 协处理器指令(如MMU配置)

### 3. ARM内存架构
- 强序/弱序内存模型
- 缓存一致性协议
- DMA一致性问题

### 4. ARM中断系统
- 向量中断控制器(VIC/GIC)
- 中断优先级与嵌套
- 快速中断(FIQ)处理

### 5. ARM调试技术
- JTAG/SWD调试接口
- ARM CoreSight调试架构
- 硬件断点与观察点

## 📚 学习资源
- [ARM Architecture Reference Manual](docs/arm32/arm_manual.md)
- [ARM汇编语言教程](docs/arm32/assembly.md)
- [设备树详解](docs/arm32/devicetree.md)
- [实验指导手册](docs/labs/)
- [常见问题解答](docs/faq.md)

## 🤝 贡献指南
欢迎提交bug报告、功能建议或代码贡献！特别欢迎针对不同ARM开发板的移植贡献。

## 📄 许可证
MIT License - 详见 [LICENSE](LICENSE) 文件 