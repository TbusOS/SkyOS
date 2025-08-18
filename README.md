# SkymOS - ARM32教学类操作系统

一个专为ARM32/ARM64架构设计的教学类操作系统项目
SkymOS是一个专为教学设计的ARM32架构操作系统，旨在帮助学生循序渐进地理解ARM处理器和操作系统的核心概念和实现原理。

## 项目目标

通过分阶段的学习，让学生能够：
1. 理解ARM处理器的基本架构和指令集
2. 掌握嵌入式系统的启动流程  
3. 学习操作系统内核的核心概念
4. 实践底层系统编程技术

## 教学设计

SkymOS采用分阶段的教学设计，每个阶段都有明确的学习目标和可运行的成果：

### Stage 01: 系统启动与输出 (第1-2周)
- **学习目标**: 了解ARM启动流程，实现基本输出
- **技术要点**: 汇编语言、链接脚本、UART通信
- **输出**: 在ARM开发板或QEMU上显示"Hello SkymOS"
- **重点**: ARM处理器模式、CPSR寄存器、异常向量表

### 🔗 [阶段二：ARM异常处理与中断](stages/stage02/) (Week 3-4)  
- **目标**: 理解ARM异常模型、中断控制器(GIC)
- **实践**: 实现异常向量表、中断服务程序
- **输出**: 支持定时器中断的基础内核
- **重点**: SVC模式、IRQ/FIQ处理、上下文保存恢复

### 🔗 [阶段三：ARM内存管理](stages/stage03/) (Week 5-7)
- **目标**: 理解ARM MMU、页表结构、虚拟内存
- **实践**: 配置MMU、实现二级页表、内存分配器
- **输出**: 支持虚拟内存的内核
- **重点**: Translation Table Base Register、页表遍历、缓存一致性

### 🔗 [阶段四：进程管理与上下文切换](stages/stage04/) (Week 8-10)
- **目标**: ARM上下文切换、进程控制块、调度器
- **实践**: 实现进程创建、ARM上下文切换汇编代码
- **输出**: 支持多进程的操作系统
- **重点**: 寄存器保存恢复、用户态/内核态切换、系统调用

### 🔗 [阶段五：设备树与驱动框架](stages/stage05/) (Week 11-13)
- **目标**: 理解设备树、平台设备驱动模型
- **实践**: 解析设备树、实现UART/GPIO驱动
- **输出**: 支持设备树的驱动框架
- **重点**: DTS/DTB、设备匹配、寄存器映射

### 🔗 [阶段六：文件系统与存储](stages/stage06/) (Week 14-15)
- **目标**: 简单文件系统、SD卡/eMMC访问
- **实践**: 实现FAT32文件系统、块设备驱动
- **输出**: 支持文件读写的完整系统
- **重点**: 块设备抽象、缓存机制、文件操作

### 🔗 [阶段七：系统优化与调试](stages/stage07/) (Week 16)
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
SkymOS/
├── README.md                        # 项目总体介绍
├── 快速开始.md                      # 快速上手指南
├── stages/                          # 分阶段学习内容
│   ├── README.md                    # 分阶段说明
│   ├── stage01/                     # 阶段一：ARM32基础与引导
│   │   ├── docs/                    # 理论文档
│   │   ├── labs/                    # 实验指导
│   │   ├── code/                    # 源代码
│   │   └── resources/               # 辅助资源
│   ├── stage02/                     # 阶段二：异常处理与中断
│   ├── stage03/                     # 阶段三：内存管理
│   ├── stage04/                     # 阶段四：进程管理
│   ├── stage05/                     # 阶段五：设备树与驱动
│   ├── stage06/                     # 阶段六：文件系统
│   └── stage07/                     # 阶段七：系统优化
├── docs/                            # 全局文档
│   └── 教学实施计划-ARM32.md         # 完整教学计划
└── tools/                           # 全局工具
```

## 🚀 快速开始

### 环境安装
```bash
# 安装ARM交叉编译工具链
sudo apt-get install gcc-arm-none-eabi qemu-system-arm make git

# 克隆项目
git clone https://github.com/TbusOS/SkymOS.git
cd SkymOS
```

### 第一次运行
```bash
# 进入第一阶段
cd stages/stage01/code

# 编译运行
make all
make run
```

### 预期输出
```
======================================
    SkymOS - ARM32 教学操作系统
======================================
版本: 0.1.0 (教学演示版)
架构: ARM Cortex-A15
处理器ID: 0x412FC0F1
当前模式: Supervisor (CPSR: 0x600001D3)
--------------------------------------
内核初始化完成！
======================================
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

### 核心文档
- [ARM Architecture Reference Manual](stages/stage01/docs/ARM32硬件规格说明.md)
- [完整教学实施计划](docs/教学实施计划-ARM32.md)
- [分阶段学习指南](stages/README.md)

### 实验指导
- [阶段一实验：ARM32启动](stages/stage01/labs/实验01-ARM32启动.md)
- 更多实验指导正在开发中...

### 开发工具
- [硬件分析工具](stages/stage01/resources/analyze_hardware.py)
- [QEMU设备树示例](stages/stage01/resources/qemu-virt-devicetree.dts)

## 🎓 教学使用建议

### 对于教师
1. **按阶段教学**: 每个阶段2-3周，确保学生充分理解
2. **理论结合实践**: 每个概念都有对应的编程练习
3. **渐进式复杂度**: 从简单的ARM汇编到完整的操作系统
4. **硬件导向**: 重点理解ARM硬件特性和约束

### 对于学生
1. **循序渐进**: 不要跳跃阶段，确保每个阶段都掌握
2. **动手实践**: 每个阶段都要亲自编译运行
3. **查阅文档**: 养成查阅ARM官方文档的习惯
4. **对比学习**: 与Linux内核实现对比学习

## 🤝 贡献指南
欢迎提交bug报告、功能建议或代码贡献！特别欢迎针对不同ARM开发板的移植贡献。

### 贡献方式
1. Fork本项目
2. 创建功能分支 (`git checkout -b feature/new-stage`)
3. 提交更改 (`git commit -am 'Add new learning stage'`)
4. 推送到分支 (`git push origin feature/new-stage`)
5. 创建Pull Request

## 📞 支持与帮助
- **GitHub Issues**: 技术问题和bug报告
- **GitHub Discussions**: 学习讨论和经验分享
- **文档**: 查看各阶段的详细文档

## 📄 许可证
MIT License - 详见 [LICENSE](LICENSE) 文件

---

**开始您的ARM32操作系统学习之旅！** 🚀

从 [阶段一](stages/stage01/) 开始，循序渐进地构建您自己的教学操作系统。 