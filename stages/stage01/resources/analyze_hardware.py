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
    device: str
    irq_num: int
    trigger_type: str
    description: str
    
    def __str__(self) -> str:
        return f"IRQ {self.irq_num}: {self.device} ({self.trigger_type}) - {self.description}"

class HardwareAnalyzer:
    def __init__(self):
        self.memory_regions: List[MemoryRegion] = []
        self.interrupts: List[InterruptInfo] = []
        self.devices: Dict[str, Dict] = {}
        
    def parse_devicetree(self, dt_content: str):
        """解析设备树内容"""
        lines = dt_content.split('\n')
        current_device = None
        device_data = {}
        
        for line in lines:
            line = line.strip()
            
            # 检测设备节点开始
            device_match = re.match(r'(\w+)@([0-9a-fA-F]+)\s*{', line)
            if device_match:
                current_device = device_match.group(1)
                base_addr = int(device_match.group(2), 16)
                device_data = {'base_addr': base_addr, 'properties': {}}
                continue
            
            # 内存节点特殊处理
            memory_match = re.match(r'memory@([0-9a-fA-F]+)\s*{', line)
            if memory_match:
                current_device = 'memory'
                base_addr = int(memory_match.group(1), 16)
                device_data = {'base_addr': base_addr, 'properties': {}}
                continue
            
            # 解析属性
            if current_device and '=' in line:
                self._parse_property(line, device_data['properties'])
            
            # 设备节点结束
            if line == '};' and current_device:
                self.devices[current_device] = device_data
                self._process_device(current_device, device_data)
                current_device = None
                device_data = {}
    
    def _parse_property(self, line: str, properties: Dict):
        """解析设备树属性"""
        # reg属性 (地址和大小)
        reg_match = re.search(r'reg\s*=\s*<([^>]+)>', line)
        if reg_match:
            values = [int(x.strip(), 16) for x in reg_match.group(1).split() if x.strip().startswith('0x')]
            if len(values) >= 4:
                # 64位地址格式: <addr_hi addr_lo size_hi size_lo>
                addr = (values[0] << 32) | values[1]
                size = (values[2] << 32) | values[3]
                properties['reg'] = {'addr': addr, 'size': size}
            elif len(values) >= 2:
                # 32位地址格式: <addr size>
                properties['reg'] = {'addr': values[0], 'size': values[1]}
        
        # interrupts属性
        int_match = re.search(r'interrupts\s*=\s*<([^>]+)>', line)
        if int_match:
            values = [int(x.strip(), 16) for x in int_match.group(1).split() if x.strip().startswith('0x')]
            if len(values) >= 3:
                # GIC格式: <type irq_num flags>
                properties['interrupts'] = {
                    'type': values[0],
                    'irq': values[1], 
                    'flags': values[2]
                }
        
        # compatible属性
        compat_match = re.search(r'compatible\s*=\s*"([^"]+)"', line)
        if compat_match:
            properties['compatible'] = compat_match.group(1)
    
    def _process_device(self, device_name: str, device_data: Dict):
        """处理单个设备的信息"""
        props = device_data.get('properties', {})
        
        # 添加内存区域
        if 'reg' in props:
            addr = props['reg']['addr']
            size = props['reg']['size']
            desc = self._get_device_description(device_name, props.get('compatible', ''))
            
            self.memory_regions.append(MemoryRegion(
                name=device_name,
                base_addr=addr,
                size=size,
                description=desc
            ))
        
        # 添加中断信息
        if 'interrupts' in props:
            irq_info = props['interrupts']
            trigger = "Level" if irq_info['flags'] & 0x4 else "Edge"
            
            self.interrupts.append(InterruptInfo(
                device=device_name,
                irq_num=irq_info['irq'],
                trigger_type=trigger,
                description=self._get_device_description(device_name, props.get('compatible', ''))
            ))
    
    def _get_device_description(self, device_name: str, compatible: str) -> str:
        """获取设备描述"""
        descriptions = {
            'memory': '主内存 (RAM)',
            'pl011': 'ARM PL011 UART串口控制器',
            'pl061': 'ARM PL061 GPIO控制器', 
            'pl031': 'ARM PL031 RTC实时时钟',
            'intc': 'ARM GIC-400通用中断控制器',
            'timer': 'ARM通用定时器',
            'pcie': 'PCIe主机控制器',
            'virtio_mmio': 'VirtIO MMIO设备',
            'fw-cfg': 'QEMU固件配置接口'
        }
        
        # 根据compatible字符串匹配
        for key, desc in descriptions.items():
            if key in device_name.lower() or key in compatible.lower():
                return desc
        
        return f'未知设备 ({compatible})'
    
    def generate_header_file(self) -> str:
        """生成C语言头文件"""
        header = """/*
 * SkyOS硬件地址定义
 * 自动生成于设备树分析
 * 请勿手动修改此文件
 */

#ifndef _SKYOS_HARDWARE_H_
#define _SKYOS_HARDWARE_H_

#include <stdint.h>

/* 内存布局 */
"""
        
        # 添加内存区域定义
        for region in sorted(self.memory_regions, key=lambda x: x.base_addr):
            name = region.name.upper().replace('-', '_')
            header += f"#define {name}_BASE        0x{region.base_addr:08X}    /* {region.description} */\n"
            if region.name != 'memory':  # 不为主内存添加大小定义
                header += f"#define {name}_SIZE        0x{region.size:08X}    /* {region.size} bytes */\n"
            header += "\n"
        
        # 添加中断定义
        header += "/* 中断号定义 */\n"
        for irq in sorted(self.interrupts, key=lambda x: x.irq_num):
            name = irq.device.upper().replace('-', '_')
            header += f"#define {name}_IRQ         {irq.irq_num:2d}                /* {irq.description} */\n"
        
        header += "\n#endif /* _SKYOS_HARDWARE_H_ */\n"
        return header
    
    def generate_report(self) -> str:
        """生成硬件分析报告"""
        report = """
╔════════════════════════════════════════════════════════════════════════════════╗
║                           SkyOS硬件规格分析报告                                ║
╚════════════════════════════════════════════════════════════════════════════════╝

📍 内存布局
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
"""
        
        # 内存区域表格
        for region in sorted(self.memory_regions, key=lambda x: x.base_addr):
            report += f"{region}\n"
        
        report += f"""
⚡ 中断配置
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
"""
        
        # 中断信息表格
        for irq in sorted(self.interrupts, key=lambda x: x.irq_num):
            report += f"{irq}\n"
        
        report += f"""
💡 关键代码对应关系
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
"""
        
        # 查找UART配置
        uart_region = next((r for r in self.memory_regions if 'pl011' in r.name.lower()), None)
        if uart_region:
            report += f"""
UART基地址定义 (kernel/main.c):
    #define UART0_BASE      0x{uart_region.base_addr:08X}    // 来源: 设备树 pl011@{uart_region.base_addr:x}
    
内存起始地址 (boot/boot.lds):"""
        
        memory_region = next((r for r in self.memory_regions if r.name == 'memory'), None)
        if memory_region:
            report += f"""
    MEMORY {{ RAM (rwx) : ORIGIN = 0x{memory_region.base_addr:08X}, LENGTH = {memory_region.size//1024//1024}M }}
    """
        
        report += f"""
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
- QEMU ARM System Emulation: https://qemu.readthedocs.io/en/latest/system/arm/virt.html
- ARM PL011 UART TRM: ARM DDI 0183  
- ARM GIC-400 TRM: ARM DDI 0471
- Device Tree Specification: https://www.devicetree.org/
"""
        
        return report

def main():
    parser = argparse.ArgumentParser(description='分析QEMU ARM virt machine硬件配置')
    parser.add_argument('devicetree', nargs='?', default='docs/labs/qemu-virt-devicetree.dts',
                        help='设备树文件路径 (默认使用示例文件)')
    parser.add_argument('--output-header', '-H', help='输出C头文件路径')
    parser.add_argument('--output-report', '-R', help='输出报告文件路径')
    
    args = parser.parse_args()
    
    try:
        with open(args.devicetree, 'r') as f:
            dt_content = f.read()
    except FileNotFoundError:
        print(f"❌ 错误: 无法找到设备树文件 {args.devicetree}")
        print("💡 提示: 请使用以下命令生成设备树文件:")
        print("   qemu-system-arm -machine virt -cpu cortex-a15 -machine dumpdtb=virt.dtb")
        print("   dtc -I dtb -O dts virt.dtb > virt.dts")
        return 1
    
    # 分析硬件配置
    analyzer = HardwareAnalyzer()
    analyzer.parse_devicetree(dt_content)
    
    # 生成报告
    report = analyzer.generate_report()
    if args.output_report:
        with open(args.output_report, 'w') as f:
            f.write(report)
        print(f"✅ 硬件分析报告已保存到: {args.output_report}")
    else:
        print(report)
    
    # 生成头文件
    if args.output_header:
        header = analyzer.generate_header_file()
        with open(args.output_header, 'w') as f:
            f.write(header)
        print(f"✅ C头文件已保存到: {args.output_header}")
    
    return 0

if __name__ == '__main__':
    sys.exit(main()) 