# SkymOS ARM32 Makefile
# 教学类操作系统构建脚本

# 架构配置
ARCH ?= arm32
TARGET = arm-none-eabi

# 交叉编译工具链
CC = $(TARGET)-gcc
AS = $(TARGET)-as  
LD = $(TARGET)-ld
OBJCOPY = $(TARGET)-objcopy
OBJDUMP = $(TARGET)-objdump

# 编译标志
CFLAGS = -mcpu=cortex-a15 -marm -ffreestanding -nostdlib -nostartfiles \
         -Wall -Wextra -g -O2 -fno-stack-protector
ASFLAGS = -mcpu=cortex-a15 -g
LDFLAGS = -T boot/boot.lds -nostdlib

# 目录结构
BOOT_DIR = boot
KERNEL_DIR = kernel
INCLUDE_DIR = include
BUILD_DIR = build

# 源文件
BOOT_SOURCES = $(wildcard $(BOOT_DIR)/*.S)
KERNEL_SOURCES = $(wildcard $(KERNEL_DIR)/*.c)
ASM_SOURCES = $(wildcard $(KERNEL_DIR)/*.S)

# 目标文件
BOOT_OBJECTS = $(BOOT_SOURCES:$(BOOT_DIR)/%.S=$(BUILD_DIR)/%.o)
KERNEL_OBJECTS = $(KERNEL_SOURCES:$(KERNEL_DIR)/%.c=$(BUILD_DIR)/%.o)
ASM_OBJECTS = $(ASM_SOURCES:$(KERNEL_DIR)/%.S=$(BUILD_DIR)/%.o)

# 最终目标
KERNEL_ELF = $(BUILD_DIR)/skymos.elf
KERNEL_BIN = $(BUILD_DIR)/skymos.bin
KERNEL_IMG = $(BUILD_DIR)/skymos.img

# QEMU配置
QEMU = qemu-system-arm
QEMU_FLAGS = -machine virt -cpu cortex-a15 -m 256M -nographic \
             -kernel $(KERNEL_BIN)
QEMU_DEBUG_FLAGS = $(QEMU_FLAGS) -s -S

# 默认目标
.PHONY: all clean run debug help

all: $(KERNEL_IMG)

# 创建构建目录
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# 编译汇编文件
$(BUILD_DIR)/%.o: $(BOOT_DIR)/%.S | $(BUILD_DIR)
	@echo "AS $<"
	@$(AS) $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.S | $(BUILD_DIR)
	@echo "AS $<"
	@$(AS) $(ASFLAGS) -o $@ $<

# 编译C文件
$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.c | $(BUILD_DIR)
	@echo "CC $<"
	@$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c -o $@ $<

# 链接生成ELF文件
$(KERNEL_ELF): $(BOOT_OBJECTS) $(KERNEL_OBJECTS) $(ASM_OBJECTS)
	@echo "LD $@"
	@$(LD) $(LDFLAGS) -o $@ $^

# 生成二进制文件
$(KERNEL_BIN): $(KERNEL_ELF)
	@echo "OBJCOPY $@"
	@$(OBJCOPY) -O binary $< $@

# 生成镜像文件
$(KERNEL_IMG): $(KERNEL_BIN)
	@echo "Creating kernel image..."
	@cp $< $@

# 在QEMU中运行
run: $(KERNEL_BIN)
	@echo "Running SkymOS in QEMU..."
	@$(QEMU) $(QEMU_FLAGS)

# 调试模式
debug: $(KERNEL_BIN)
	@echo "Starting QEMU in debug mode..."
	@echo "Connect with: arm-none-eabi-gdb -ex 'target remote localhost:1234' $(KERNEL_ELF)"
	@$(QEMU) $(QEMU_DEBUG_FLAGS)

# 反汇编
disasm: $(KERNEL_ELF)
	@$(OBJDUMP) -d $< > $(BUILD_DIR)/skymos.disasm
	@echo "Disassembly saved to $(BUILD_DIR)/skymos.disasm"

# 显示符号表
symbols: $(KERNEL_ELF)
	@$(OBJDUMP) -t $< > $(BUILD_DIR)/skymos.symbols
	@echo "Symbols saved to $(BUILD_DIR)/skymos.symbols"

# 生成SD卡镜像 (用于真实硬件)
sdcard: $(KERNEL_IMG)
	@echo "Creating SD card image..."
	@dd if=/dev/zero of=$(BUILD_DIR)/skymos.sdcard bs=1M count=64
	@echo "Copy $(KERNEL_IMG) to SD card"

# 检查工具链
check-toolchain:
	@echo "Checking ARM toolchain..."
	@which $(CC) || (echo "ARM toolchain not found. Install with: sudo apt-get install gcc-arm-none-eabi"; exit 1)
	@which $(QEMU) || (echo "QEMU not found. Install with: sudo apt-get install qemu-system-arm"; exit 1)
	@echo "Toolchain check passed!"

# 清理
clean:
	@echo "Cleaning build files..."
	@rm -rf $(BUILD_DIR)

# 帮助信息
help:
	@echo "SkymOS ARM32 Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all          - Build kernel image"
	@echo "  run          - Run in QEMU"
	@echo "  debug        - Run in QEMU debug mode"
	@echo "  disasm       - Generate disassembly"
	@echo "  symbols      - Generate symbol table"
	@echo "  sdcard       - Create SD card image"
	@echo "  check-toolchain - Check if tools are installed"
	@echo "  clean        - Remove build files"
	@echo "  help         - Show this help"
	@echo ""
	@echo "Environment variables:"
	@echo "  ARCH=arm32   - Target architecture (default)"
	@echo ""
	@echo "Examples:"
	@echo "  make all              # Build everything"
	@echo "  make run              # Run in QEMU"
	@echo "  make debug            # Debug with GDB"

# 依赖关系
-include $(OBJECTS:.o=.d) 