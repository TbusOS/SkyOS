#!/usr/bin/env python3
"""
SkyOS硬件规格分析工具
用于解析QEMU virt machine的硬件配置并生成易于理解的报告

使用方法:
1. 生成设备树: qemu-system-arm -machine virt -machine dumpdtb=virt.dtb
2. 转换为文本: dtc -I dtb -O dts virt.dtb > virt.dts
3. 运行分析: python3 analyze_hardware.py virt.dts
"""

import sys
import re
import argparse
from dataclasses import dataclass
from typing import List, Dict, Optional

@dataclass
class MemoryRegion:
    name: str
    base_addr: int
    size: int
    description: str
    
    @property
    def end_addr(self) -> int:
        return self.base_addr + self.size - 1
    
    def __str__(self) -> str:
        return f"{self.name}: 0x{self.base_addr:08X} - 0x{self.end_addr:08X} ({self.size // 1024}KB) - {self.description}"

@dataclass 
class InterruptInfo:
    irq_num: int
    device_name: str
    irq_type: str
    description: str
    
    def __str__(self) -> str:
        return f"IRQ {self.irq_num}: {self.device_name} ({self.irq_type}) - {self.description}"

class DeviceTreeParser:
    """设备树解析器"""
    
    def __init__(self, dts_content: str):
        self.content = dts_content
        self.memory_regions: List[MemoryRegion] = []
        self.interrupts: List[InterruptInfo] = []
        
    def parse(self):
        """解析设备树内容"""
        self._parse_memory_regions()
        self._parse_interrupts()
        
    def _parse_memory_regions(self):
        """解析内存区域"""
        # 匹配设备节点和reg属性
        device_pattern = r'(\w+)@([0-9a-f]+)\s*\{'
        reg_pattern = r'reg\s*=\s*<(.*?)>;'
        compatible_pattern = r'compatible\s*=\s*["]([^"]+)["]'
        
        lines = self.content.split('\n')
        current_device = None
        current_base = None
        current_compatible = None
        
        for i, line in enumerate(lines):
            line = line.strip()
            
            # 匹配设备节点
            device_match = re.search(device_pattern, line)
            if device_match:
                current_device = device_match.group(1)
                current_base = int(device_match.group(2), 16)
                current_compatible = None
                continue
                
            # 匹配compatible属性
            compatible_match = re.search(compatible_pattern, line)
            if compatible_match and current_device:
                current_compatible = compatible_match.group(1)
                continue
                
            # 匹配reg属性
            reg_match = re.search(reg_pattern, line)
            if reg_match and current_device and current_base is not None:
                reg_values = reg_match.group(1).split()
                if len(reg_values) >= 4:
                    # 处理64位地址格式: <addr_high addr_low size_high size_low>
                    base_addr = (int(reg_values[0], 16) << 32) + int(reg_values[1], 16)
                    size = (int(reg_values[2], 16) << 32) + int(reg_values[3], 16)
                elif len(reg_values) >= 2:
                    # 处理32位地址格式: <addr size>
                    base_addr = int(reg_values[0], 16)
                    size = int(reg_values[1], 16)
                else:
                    continue
                    
                description = self._get_device_description(current_device, current_compatible)
                
                self.memory_regions.append(MemoryRegion(
                    name=current_device,
                    base_addr=base_addr,
                    size=size,
                    description=description
                ))
                
                current_device = None
                current_base = None
                current_compatible = None
    
    def _parse_interrupts(self):
        """解析中断信息"""
        # 匹配设备和中断属性
        device_pattern = r'(\w+)@([0-9a-f]+)\s*\{'
        interrupts_pattern = r'interrupts\s*=\s*<(.*?)>;'
        compatible_pattern = r'compatible\s*=\s*["]([^"]+)["]'
        
        lines = self.content.split('\n')
        current_device = None
        current_compatible = None
        
        for line in lines:
            line = line.strip()
            
            # 匹配设备节点
            device_match = re.search(device_pattern, line)
            if device_match:
                current_device = device_match.group(1)
                current_compatible = None
                continue
                
            # 匹配compatible属性
            compatible_match = re.search(compatible_pattern, line)
            if compatible_match and current_device:
                current_compatible = compatible_match.group(1)
                continue
                
            # 匹配interrupts属性
            interrupts_match = re.search(interrupts_pattern, line)
            if interrupts_match and current_device:
                interrupt_values = interrupts_match.group(1).split()
                if len(interrupt_values) >= 3:
                    # ARM GIC格式: <type irq_num flags>
                    irq_type_num = int(interrupt_values[0], 16)
                    irq_num = int(interrupt_values[1], 16)
                    
                    # SPI中断需要加32偏移
                    if irq_type_num == 0:
                        irq_num += 32
                    
                    irq_type = "Edge" if len(interrupt_values) > 2 and int(interrupt_values[2], 16) & 0x1 else "Level"
                    description = self._get_device_description(current_device, current_compatible)
                    
                    self.interrupts.append(InterruptInfo(
                        irq_num=irq_num,
                        device_name=current_device,
                        irq_type=irq_type,
                        description=description
                    ))
                
                current_device = None
                current_compatible = None
    
    def _get_device_description(self, device_name: str, compatible: str = None) -> str:
        """获取设备描述"""
        descriptions = {
            'pl011': 'ARM PL011 UART串口控制器',
            'pl031': 'ARM PL031 RTC实时时钟',
            'gpio': 'ARM PL061 GPIO控制器',
            'pl061': 'ARM PL061 GPIO控制器',
            'virtio_mmio': 'VirtIO MMIO设备',
            'pcie': 'PCIe主机控制器',
            'memory': '主内存 (RAM)',
            'flash': 'Flash存储器',
            'intc': 'ARM GIC中断控制器',
            'v2m': '未知设备',
        }
        
        # 优先使用compatible字符串匹配
        if compatible:
            if 'pl011' in compatible:
                return 'ARM PL011 UART串口控制器'
            elif 'pl031' in compatible:
                return 'ARM PL031 RTC实时时钟'
            elif 'pl061' in compatible:
                return 'ARM PL061 GPIO控制器'
            elif 'virtio,mmio' in compatible:
                return 'VirtIO MMIO设备'
            elif 'arm,gic' in compatible:
                return 'ARM GIC中断控制器'
            elif 'gic-v2m-frame' in compatible:
                return f'未知设备 ({compatible})'
        
        return descriptions.get(device_name, f'未知设备类型 ({device_name})')

class HardwareAnalyzer:
    """硬件分析器"""
    
    def __init__(self, parser: DeviceTreeParser):
        self.parser = parser
        
    def generate_report(self) -> str:
        """生成硬件分析报告"""
        report = []
        
        # 标题
        report.append("╔════════════════════════════════════════════════════════════════════════════════╗")
        report.append("║                           SkyOS硬件规格分析报告                                ║")
        report.append("╚════════════════════════════════════════════════════════════════════════════════╝")
        report.append("")
        
        # 内存布局
        report.append("📍 内存布局")
        report.append("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
        
        # 按地址排序内存区域
        sorted_regions = sorted(self.parser.memory_regions, key=lambda x: x.base_addr)
        for region in sorted_regions:
            size_kb = max(region.size // 1024, 0)
            report.append(f"{region.name}: 0x{region.base_addr:08X} - 0x{region.end_addr:08X} ({size_kb}KB) - {region.description}")
        
        report.append("")
        
        # 中断配置
        report.append("⚡ 中断配置")  
        report.append("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
        
        # 按中断号排序
        sorted_interrupts = sorted(self.parser.interrupts, key=lambda x: x.irq_num)
        for interrupt in sorted_interrupts:
            report.append(f"IRQ {interrupt.irq_num}: {interrupt.device_name} ({interrupt.irq_type}) - {interrupt.description}")
        
        report.append("")
        
        # 关键代码对应关系
        report.append("💡 关键代码对应关系")
        report.append("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
        report.append("")
        
        # 查找UART基地址
        uart_region = next((r for r in sorted_regions if 'pl011' in r.name.lower()), None)
        if uart_region:
            report.append("UART基地址定义 (kernel/main.c):")
            report.append(f"    #define UART0_BASE      0x{uart_region.base_addr:08X}    // 来源: 设备树 {uart_region.name}@{uart_region.base_addr:x}")
        
        # 查找内存基地址
        memory_region = next((r for r in sorted_regions if r.name == 'memory'), None)
        if memory_region:
            report.append("    ")
            report.append("内存起始地址 (boot/boot.lds):")
            report.append(f"    MEMORY {{ RAM (rwx) : ORIGIN = 0x{memory_region.base_addr:08X}, LENGTH = 256M }}")
        
        report.append("    ")
        
        # 验证命令
        report.append("🔍 验证命令")
        report.append("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
        report.append("1. 生成设备树:")
        report.append("   qemu-system-arm -machine virt -cpu cortex-a15 -machine dumpdtb=virt.dtb")
        report.append("   ")
        report.append("2. 转换为文本:")
        report.append("   dtc -I dtb -O dts virt.dtb > virt.dts")
        report.append("   ")
        report.append("3. 查看特定设备:")
        report.append("   grep -A 5 \"pl011@\" virt.dts      # UART配置")
        report.append("   grep -A 5 \"memory@\" virt.dts     # 内存配置")
        report.append("   grep -A 5 \"intc@\" virt.dts       # 中断控制器")
        report.append("   ")
        report.append("4. 运行时查询:")
        report.append("   (qemu) info mtree               # QEMU monitor中查看内存树")
        report.append("   (qemu) info qtree               # 查看设备树")
        report.append("")
        
        # 参考文档
        report.append("📚 参考文档")
        report.append("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
        report.append("- QEMU ARM System Emulation: https://www.qemu.org/docs/master/system/target-arm.html")
        report.append("- QEMU virt machine: https://qemu-project.gitlab.io/qemu/system/arm/virt.html")
        report.append("- ARM PL011 UART TRM: ARM DDI 0183")  
        report.append("- ARM GIC-400 TRM: ARM DDI 0471")
        report.append("- Device Tree Specification: https://www.devicetree.org/")
        
        return "\n".join(report)
    
    def generate_c_header(self) -> str:
        """生成C头文件"""
        header = []
        header.append("/*")
        header.append(" * SkyOS硬件地址定义")
        header.append(" * 自动生成于设备树分析")
        header.append(" * 请勿手动修改此文件")
        header.append(" */")
        header.append("")
        header.append("#ifndef _SKYOS_HARDWARE_H_")
        header.append("#define _SKYOS_HARDWARE_H_")
        header.append("")
        header.append("#include <stdint.h>")
        header.append("")
        
        # 内存布局定义
        header.append("/* 内存布局 */")
        sorted_regions = sorted(self.parser.memory_regions, key=lambda x: x.base_addr)
        for region in sorted_regions:
            name_upper = region.name.upper()
            header.append(f"#define {name_upper}_BASE        0x{region.base_addr:08X}    /* {region.description} */")
            if region.name != 'memory':  # 内存区域不需要SIZE定义
                header.append(f"#define {name_upper}_SIZE        0x{region.size:08X}    /* {region.size} bytes */")
            header.append("")
        
        # 中断号定义
        if self.parser.interrupts:
            header.append("/* 中断号定义 */")
            sorted_interrupts = sorted(self.parser.interrupts, key=lambda x: x.irq_num)
            for interrupt in sorted_interrupts:
                name_upper = interrupt.device_name.upper()
                header.append(f"#define {name_upper}_IRQ          {interrupt.irq_num}                /* {interrupt.description} */")
        
        header.append("")
        header.append("#endif /* _SKYOS_HARDWARE_H_ */")
        
        return "\n".join(header)

def main():
    parser = argparse.ArgumentParser(description='分析QEMU ARM virt machine硬件配置')
    parser.add_argument('devicetree', nargs='?', default='docs/labs/qemu-virt-devicetree.dts',
                       help='设备树文件路径 (默认使用示例文件)')
    parser.add_argument('--output-header', '-H', help='输出C头文件路径')
    parser.add_argument('--output-report', '-R', help='输出报告文件路径')
    
    args = parser.parse_args()
    
    # 读取设备树文件
    try:
        with open(args.devicetree, 'r', encoding='utf-8') as f:
            dts_content = f.read()
    except FileNotFoundError:
        print(f"❌ 错误: 无法找到设备树文件 {args.devicetree}")
        print("💡 提示: 请使用以下命令生成设备树文件:")
        print("   qemu-system-arm -machine virt -cpu cortex-a15 -machine dumpdtb=virt.dtb")
        print("   dtc -I dtb -O dts virt.dtb > virt.dts")
        return 1
    
    # 解析设备树
    dt_parser = DeviceTreeParser(dts_content)
    dt_parser.parse()
    
    # 生成分析报告
    analyzer = HardwareAnalyzer(dt_parser)
    
    # 生成并显示报告
    report = analyzer.generate_report()
    
    if args.output_report:
        # 保存报告到文件
        with open(args.output_report, 'w', encoding='utf-8') as f:
            f.write(report)
        print(f"✅ 硬件分析报告已保存到: {args.output_report}")
    else:
        # 显示报告
        print(report)
    
    # 生成C头文件
    if args.output_header:
        header_content = analyzer.generate_c_header()
        with open(args.output_header, 'w', encoding='utf-8') as f:
            f.write(header_content)
        print(f"✅ C头文件已保存到: {args.output_header}")
    
    return 0

if __name__ == '__main__':
    sys.exit(main()) 