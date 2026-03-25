KERNEL      := kernel.bin
ISO         := kfs.iso

ASM         := nasm
CC          := gcc
LD          := ld
GRUB_MK     := $(or $(shell command -v grub2-mkrescue 2>/dev/null),$(shell command -v grub-mkrescue 2>/dev/null),grub-mkrescue)
GRUB_FLAGS  := --compress=xz --core-compress=xz --themes= --locales= --fonts=

BOOT_DIR    := boot
BUILD_DIR   := build
GRUB_DIR    := grub
ISO_DIR     := iso
SRCS_DIR    := srcs
CFLAGS      := -m32 -ffreestanding -fno-builtin -fno-stack-protector \
               -nostdlib -nodefaultlibs -fno-pie -O2 -Wall -Wextra \
               -I$(SRCS_DIR)/console -I$(SRCS_DIR)/kprintf -I$(SRCS_DIR)/vga -I$(SRCS_DIR)/keyboard

KERNEL_OBJS := $(BUILD_DIR)/kernel.o \
               $(BUILD_DIR)/console.o \
               $(BUILD_DIR)/kprintf.o \
               $(BUILD_DIR)/vga.o \
               $(BUILD_DIR)/keyboard.o \
               $(BUILD_DIR)/vga_cursor.o

BOOT_OBJ    := $(BUILD_DIR)/boot.o

.PHONY: all clean run

all: $(ISO)

$(BOOT_OBJ): $(BOOT_DIR)/boot.asm
	mkdir -p $(BUILD_DIR)
	$(ASM) -f elf32 $< -o $@

$(BUILD_DIR)/kernel.o: $(SRCS_DIR)/kernel.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/console.o: $(SRCS_DIR)/console/console.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kprintf.o: $(SRCS_DIR)/kprintf/kprintf.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/vga.o: $(SRCS_DIR)/vga/vga.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/keyboard.o: $(SRCS_DIR)/keyboard/keyboard.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/vga_cursor.o: $(SRCS_DIR)/vga/vga_cursor.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL): $(BOOT_OBJ) $(KERNEL_OBJS) linker.ld
	$(LD) -m elf_i386 -T linker.ld -o $@ $(BOOT_OBJ) $(KERNEL_OBJS)

$(ISO): $(KERNEL) $(GRUB_DIR)/grub.cfg
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL) $(ISO_DIR)/boot/kernel.bin
	cp $(GRUB_DIR)/grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	$(GRUB_MK) -o $@ $(GRUB_FLAGS) $(ISO_DIR)

re: clean all run

clean:
	rm -rf $(BUILD_DIR) $(ISO_DIR)
	rm -f $(KERNEL) $(ISO)

run: $(ISO)
	qemu-system-i386 -cdrom $(ISO) --enable-kvm --monitor stdio -s
