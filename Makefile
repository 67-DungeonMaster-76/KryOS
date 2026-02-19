# Makefile for KryOS
# ------------------

# Directories
SRC_DIR = src
BUILD_DIR = build_stuff
OUTPUT_DIR = output
ISO_DIR = isodir

# Assembler and compiler
AS = nasm
CC = gcc
LD = ld

# Flags
ASFLAGS = -f elf32
CFLAGS = -m32 -c -ffreestanding -nostdlib -nostdinc -fno-stack-protector -I$(SRC_DIR)/kernel
LDFLAGS = -m elf_i386 -T link.ld

# Source files
ASM_SRC = $(SRC_DIR)/bootloader/boot.asm
CPU_ASM_SRC = $(SRC_DIR)/kernel/cpu.asm
C_SRC = $(SRC_DIR)/kernel/kernel.c
UTILS_SRC = $(SRC_DIR)/kernel/utils.c
GDT_SRC = $(SRC_DIR)/kernel/gdt.c
IDT_SRC = $(SRC_DIR)/kernel/idt.c
KEYBOARD_SRC = $(SRC_DIR)/kernel/keyboard.c
CLI_SRC = $(SRC_DIR)/kernel/cli.c
STRING_SRC = $(SRC_DIR)/kernel/string.c
GRAPHICS_SRC = $(SRC_DIR)/kernel/graphics.c
DEMO_SRC = $(SRC_DIR)/kernel/demo.c
FB_CONSOLE_SRC = $(SRC_DIR)/kernel/fb_console.c

# Object files
ASM_OBJ = $(BUILD_DIR)/boot.o
CPU_ASM_OBJ = $(BUILD_DIR)/cpu.o
C_OBJ = $(BUILD_DIR)/kernel.o
UTILS_OBJ = $(BUILD_DIR)/utils.o
GDT_OBJ = $(BUILD_DIR)/gdt.o
IDT_OBJ = $(BUILD_DIR)/idt.o
KEYBOARD_OBJ = $(BUILD_DIR)/keyboard.o
CLI_OBJ = $(BUILD_DIR)/cli.o
STRING_OBJ = $(BUILD_DIR)/string.o
GRAPHICS_OBJ = $(BUILD_DIR)/graphics.o
DEMO_OBJ = $(BUILD_DIR)/demo.o
FB_CONSOLE_OBJ = $(BUILD_DIR)/fb_console.o

# All objects for linking
ALL_OBJS = $(ASM_OBJ) $(CPU_ASM_OBJ) $(C_OBJ) $(UTILS_OBJ) $(GDT_OBJ) $(IDT_OBJ) $(KEYBOARD_OBJ) $(CLI_OBJ) $(STRING_OBJ) $(GRAPHICS_OBJ) $(DEMO_OBJ) $(FB_CONSOLE_OBJ)

# Output
KERNEL = $(OUTPUT_DIR)/kernel
ISO = $(OUTPUT_DIR)/kryos.iso

# QEMU for running
QEMU = qemu-system-i386

# Default target
all: $(ISO)

# Create output directory
$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Link object files to create kernel
$(KERNEL): $(ALL_OBJS) $(OUTPUT_DIR)
	$(LD) $(LDFLAGS) -o $@ $(ALL_OBJS)

# Assemble bootloader
$(ASM_OBJ): $(ASM_SRC) $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

# Assemble CPU functions
$(CPU_ASM_OBJ): $(CPU_ASM_SRC) $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

# Compile kernel
$(C_OBJ): $(C_SRC) $(SRC_DIR)/kernel/utils.h $(SRC_DIR)/kernel/gdt.h $(SRC_DIR)/kernel/idt.h $(SRC_DIR)/kernel/keyboard.h $(SRC_DIR)/kernel/graphics.h $(SRC_DIR)/kernel/fb_console.h $(SRC_DIR)/kernel/cli.h $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Compile utils
$(UTILS_OBJ): $(UTILS_SRC) $(SRC_DIR)/kernel/utils.h $(SRC_DIR)/kernel/string.h $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Compile GDT
$(GDT_OBJ): $(GDT_SRC) $(SRC_DIR)/kernel/gdt.h $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Compile IDT
$(IDT_OBJ): $(IDT_SRC) $(SRC_DIR)/kernel/idt.h $(SRC_DIR)/kernel/utils.h $(SRC_DIR)/kernel/keyboard.h $(SRC_DIR)/kernel/string.h $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Compile keyboard
$(KEYBOARD_OBJ): $(KEYBOARD_SRC) $(SRC_DIR)/kernel/keyboard.h $(SRC_DIR)/kernel/utils.h $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Compile CLI
$(CLI_OBJ): $(CLI_SRC) $(SRC_DIR)/kernel/cli.h $(SRC_DIR)/kernel/fb_console.h $(SRC_DIR)/kernel/keyboard.h $(SRC_DIR)/kernel/graphics.h $(SRC_DIR)/kernel/demo.h $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Compile string
$(STRING_OBJ): $(STRING_SRC) $(SRC_DIR)/kernel/string.h $(SRC_DIR)/kernel/stdint.h $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Compile graphics
$(GRAPHICS_OBJ): $(GRAPHICS_SRC) $(SRC_DIR)/kernel/graphics.h $(SRC_DIR)/kernel/utils.h $(SRC_DIR)/kernel/string.h $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Compile demo
$(DEMO_OBJ): $(DEMO_SRC) $(SRC_DIR)/kernel/demo.h $(SRC_DIR)/kernel/graphics.h $(SRC_DIR)/kernel/utils.h $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Compile framebuffer console
$(FB_CONSOLE_OBJ): $(FB_CONSOLE_SRC) $(SRC_DIR)/kernel/fb_console.h $(SRC_DIR)/kernel/graphics.h $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Create ISO directory structure
$(ISO_DIR)/boot/kernel: $(KERNEL)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL) $(ISO_DIR)/boot/kernel
	cp $(SRC_DIR)/bootloader/grub.cfg $(ISO_DIR)/boot/grub/grub.cfg

# Create ISO image
$(ISO): $(ISO_DIR)/boot/kernel
	grub-mkrescue -o $(ISO) $(ISO_DIR)

# Run kernel in QEMU (direct kernel boot)
run: $(KERNEL)
	$(QEMU) -kernel $(KERNEL)

# Run ISO in QEMU
run-iso: $(ISO)
	$(QEMU) -cdrom $(ISO)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(OUTPUT_DIR) $(ISO_DIR)

# Rebuild from scratch
rebuild: clean all

.PHONY: all clean run run-iso rebuild
