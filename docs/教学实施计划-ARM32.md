# SkymOS ARM32教学实施计划

## 📋 总体实施策略

### 教学模式
- **理论+实践结合**: 每个概念都配有实际编程练习
- **渐进式学习**: 从简单的ARM汇编开始，逐步构建完整系统
- **硬件导向**: 重点理解ARM硬件特性和约束
- **项目驱动**: 每个阶段都有明确的可运行成果

### 考核方式
- **实验报告** (40%): 每个阶段的实验总结和代码分析
- **代码质量** (30%): 代码规范、注释质量、可读性
- **功能演示** (20%): 在真实硬件或QEMU上的演示
- **创新扩展** (10%): 自主实现的额外功能

## 🎯 分阶段详细实施计划

### 阶段一：ARM32基础与引导 (Week 1-2)

#### 理论学习目标
- ARM Cortex-A架构概览
- ARM寄存器组织(R0-R15, CPSR, SPSR)
- ARM处理器模式与异常级别
- ARM指令集基础(数据处理、访存、分支)

#### 实践任务
1. **环境搭建**
   ```bash
   # 安装交叉编译工具链
   sudo apt-get install gcc-arm-none-eabi qemu-system-arm
   
   # 验证工具链
   arm-none-eabi-gcc --version
   qemu-system-arm --version
   ```

2. **第一个ARM汇编程序**
   ```assembly
   .section .text
   .global _start
   
   _start:
       @ 设置堆栈指针
       ldr sp, =stack_top
       
       @ 调用C main函数
       bl main
       
   hang:
       b hang
   
   .section .bss
   .space 4096
   stack_top:
   ```

3. **UART输出实现**
   - 配置QEMU virt machine的UART
   - 实现字符串输出函数
   - 显示"Hello SkymOS on ARM32!"

#### 实验指导
- **实验1**: 编写最小ARM启动代码
- **实验2**: 实现简单的串口输出
- **实验3**: 理解ARM链接脚本

#### 评估标准
- [ ] 能够编译ARM汇编代码
- [ ] 理解ARM寄存器用途
- [ ] 成功在QEMU中运行代码
- [ ] 实现基本的UART输出

### 阶段二：ARM异常处理与中断 (Week 3-4)

#### 理论学习目标
- ARM异常模型(Reset, Undefined, SWI, Prefetch Abort, Data Abort, IRQ, FIQ)
- 异常向量表结构
- 上下文保存与恢复机制
- ARM Generic Interrupt Controller (GIC)

#### 实践任务
1. **异常向量表设置**
   ```assembly
   .section .vectors, "ax"
   .global _vectors
   _vectors:
       ldr pc, =reset_handler      @ 0x00: Reset
       ldr pc, =undef_handler      @ 0x04: Undefined
       ldr pc, =swi_handler        @ 0x08: Software Interrupt
       ldr pc, =prefetch_handler   @ 0x0C: Prefetch Abort
       ldr pc, =data_handler       @ 0x10: Data Abort
       nop                         @ 0x14: Reserved
       ldr pc, =irq_handler        @ 0x18: IRQ
       ldr pc, =fiq_handler        @ 0x1C: FIQ
   ```

2. **定时器中断实现**
   - 配置ARM Generic Timer
   - 实现IRQ处理程序
   - 上下文保存恢复

3. **系统调用机制**
   - 实现SWI指令处理
   - 参数传递约定
   - 简单的系统调用表

#### 实验指导
- **实验4**: 设置异常向量表
- **实验5**: 实现定时器中断
- **实验6**: 编写第一个系统调用

#### 评估标准
- [ ] 正确处理ARM异常
- [ ] 实现定时器中断
- [ ] 理解上下文切换
- [ ] 实现基本系统调用

### 阶段三：ARM内存管理 (Week 5-7)

#### 理论学习目标
- ARM Memory Management Unit (MMU)
- 两级页表结构
- Translation Table Base Register (TTBR)
- 缓存和TLB管理
- 内存属性与访问权限

#### 实践任务
1. **MMU初始化**
   ```c
   void mmu_init(void) {
       // 创建页目录
       uint32_t *pgd = create_page_directory();
       
       // 映射内核空间 (1:1映射)
       map_kernel_space(pgd);
       
       // 映射设备寄存器
       map_device_memory(pgd);
       
       // 启用MMU
       enable_mmu(pgd);
   }
   ```

2. **页面分配器**
   - 物理页面管理
   - 空闲页面链表
   - 页面分配/释放接口

3. **虚拟内存管理**
   - 进程地址空间布局
   - 用户态/内核态地址映射
   - 缺页异常处理

#### 实验指导
- **实验7**: MMU配置与启用
- **实验8**: 实现页面分配器
- **实验9**: 虚拟内存映射

#### 评估标准
- [ ] 成功启用MMU
- [ ] 实现页面分配
- [ ] 理解虚拟内存概念
- [ ] 处理内存访问异常

### 阶段四：进程管理与上下文切换 (Week 8-10)

#### 理论学习目标
- ARM上下文切换机制
- 进程控制块(PCB)设计
- 调度算法实现
- 用户态/内核态切换

#### 实践任务
1. **进程结构定义**
   ```c
   struct task_struct {
       uint32_t pid;
       uint32_t state;
       uint32_t *stack_ptr;
       uint32_t *page_dir;
       struct registers regs;
       struct task_struct *next;
   };
   ```

2. **ARM上下文切换**
   ```assembly
   context_switch:
       @ 保存当前进程寄存器
       stmfd sp!, {r0-r12, lr}
       str sp, [r0]        @ 保存栈指针到PCB
       
       @ 恢复目标进程
       ldr sp, [r1]        @ 从PCB恢复栈指针
       ldmfd sp!, {r0-r12, pc}  @ 恢复寄存器并跳转
   ```

3. **简单调度器**
   - 轮询调度算法
   - 进程创建与销毁
   - 时间片管理

#### 实验指导
- **实验10**: 实现进程结构
- **实验11**: ARM上下文切换
- **实验12**: 多进程调度

#### 评估标准
- [ ] 正确实现上下文切换
- [ ] 支持多进程运行
- [ ] 理解调度算法
- [ ] 实现进程同步原语

### 阶段五：设备树与驱动框架 (Week 11-13)

#### 理论学习目标
- Device Tree Source (DTS) 语法
- 设备树编译与加载
- 平台设备驱动模型
- 设备匹配机制

#### 实践任务
1. **设备树解析**
   ```c
   struct device_node {
       char *name;
       char *type;
       struct property *properties;
       struct device_node *child;
       struct device_node *sibling;
   };
   ```

2. **UART驱动实现**
   ```c
   struct uart_driver {
       void (*init)(struct uart_port *port);
       void (*putc)(struct uart_port *port, char c);
       char (*getc)(struct uart_port *port);
   };
   ```

3. **GPIO驱动框架**
   - GPIO引脚配置
   - 中断GPIO处理
   - 用户空间接口

#### 实验指导
- **实验13**: 解析设备树
- **实验14**: 实现UART驱动
- **实验15**: GPIO控制

#### 评估标准
- [ ] 正确解析设备树
- [ ] 实现平台驱动
- [ ] 理解设备模型
- [ ] 支持基本IO设备

### 阶段六：文件系统与存储 (Week 14-15)

#### 理论学习目标
- 块设备抽象层
- FAT32文件系统结构
- 虚拟文件系统(VFS)
- 缓存与同步机制

#### 实践任务
1. **块设备层**
   ```c
   struct block_device {
       uint32_t block_size;
       uint32_t total_blocks;
       int (*read)(struct block_device *dev, uint32_t block, void *buf);
       int (*write)(struct block_device *dev, uint32_t block, void *buf);
   };
   ```

2. **简单文件系统**
   - 超级块管理
   - 目录项操作
   - 文件读写接口

#### 实验指导
- **实验16**: 实现块设备
- **实验17**: FAT32文件系统

#### 评估标准
- [ ] 支持块设备访问
- [ ] 实现文件系统
- [ ] 文件读写功能
- [ ] 目录操作

### 阶段七：系统优化与调试 (Week 16)

#### 理论学习目标
- ARM性能监控单元(PMU)
- 调试接口与工具
- 系统性能分析
- 硬件移植技巧

#### 实践任务
1. **性能监控**
   - 指令/周期计数
   - 缓存命中率统计
   - 内存访问分析

2. **调试支持**
   - JTAG调试接口
   - 内核符号表
   - 堆栈回溯

3. **硬件移植**
   - 树莓派适配
   - BeagleBone支持
   - 自定义板卡移植

#### 实验指导
- **实验18**: 性能分析工具
- **实验19**: 硬件调试
- **实验20**: 板卡移植

#### 评估标准
- [ ] 性能监控功能
- [ ] 支持硬件调试
- [ ] 成功移植到真实硬件
- [ ] 系统稳定性测试

## 📊 教学资源配置

### 硬件资源
- **开发板**: 树莓派4B × 20台
- **调试器**: J-Link EDU × 5台  
- **示波器**: 用于信号分析
- **逻辑分析仪**: 调试总线通信

### 软件工具
- **QEMU**: ARM虚拟化平台
- **OpenOCD**: 开源调试器
- **GDB**: 交叉调试工具
- **Device Tree Compiler**: DTC工具

### 实验环境
- **Linux主机**: Ubuntu 20.04 LTS
- **交叉编译**: arm-none-eabi-gcc
- **版本控制**: Git + GitLab
- **CI/CD**: 自动化测试流水线

## 🎯 学习成果评估

### 知识掌握度评估
1. **ARM架构理解** (25%)
   - 寄存器组织
   - 指令集特性
   - 处理器模式

2. **系统编程能力** (25%)
   - 汇编语言编程
   - 中断处理
   - 内存管理

3. **驱动开发技能** (25%)
   - 设备树使用
   - 驱动框架理解
   - 硬件接口编程

4. **系统集成能力** (25%)
   - 模块间协调
   - 性能优化
   - 问题调试

### 项目展示要求
- **现场演示**: 在真实硬件上运行
- **代码讲解**: 核心模块实现原理
- **功能测试**: 各子系统功能验证
- **扩展功能**: 创新性功能实现

## 📅 时间安排建议

### 每周时间分配
- **理论课程**: 4学时/周
- **实验实践**: 6学时/周  
- **自主学习**: 4学时/周
- **讨论答疑**: 2学时/周

### 关键里程碑
- **Week 4**: 基础中断系统演示
- **Week 8**: MMU与虚拟内存演示
- **Week 12**: 多进程系统演示
- **Week 16**: 完整系统最终展示

## 🔧 常见问题与解决方案

### 技术难点
1. **ARM汇编调试困难**
   - 使用GDB remote调试
   - 添加详细的代码注释
   - 单步执行验证

2. **MMU配置复杂**
   - 提供配置模板
   - 分步骤验证
   - 参考ARM手册

3. **设备树理解困难**
   - 从简单例子开始
   - 对比Linux DTS文件
   - 实际硬件验证

### 教学建议
- **循序渐进**: 不要急于求成，确保每个概念都理解透彻
- **实践为主**: 理论必须结合实际编程练习
- **问题导向**: 通过解决实际问题来学习
- **团队协作**: 鼓励学生间的讨论与合作

这个实施计划确保学生能够系统性地学习ARM32架构的操作系统开发，从底层硬件到上层应用都有涉及，为后续的嵌入式系统开发打下坚实基础。 