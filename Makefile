TARGET      := i386-unknown-none.json
TARGET_NAME := i386-unknown-none
KERNEL      := kernel.bin
ISO         := kfs.iso

ASM         := nasm
LD          := ld
CARGO       := cargo +nightly
GRUB_MK     := grub-mkrescue

BOOT_DIR    := boot
GRUB_DIR    := grub
ISO_DIR     := iso
TARGET_DIR  := target/$(TARGET_NAME)/release

.PHONY: all clean run

all: $(ISO)

$(BOOT_DIR)/boot.o: $(BOOT_DIR)/boot.asm
	$(ASM) -f elf32 $< -o $@

$(TARGET_DIR)/libkfs.a: Cargo.toml src/lib.rs src/kernel.rs $(TARGET)
	$(CARGO) build -Z build-std=core --release --target $(TARGET)

$(KERNEL): $(BOOT_DIR)/boot.o $(TARGET_DIR)/libkfs.a linker.ld
	$(LD) -m elf_i386 -T linker.ld -o $@ $(BOOT_DIR)/boot.o $(TARGET_DIR)/libkfs.a

$(ISO): $(KERNEL) $(GRUB_DIR)/grub.cfg
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL) $(ISO_DIR)/boot/kernel.bin
	cp $(GRUB_DIR)/grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	$(GRUB_MK) -o $@ $(ISO_DIR)

clean:
	$(CARGO) clean
	rm -f $(BOOT_DIR)/boot.o $(KERNEL) $(ISO)
	rm -rf $(ISO_DIR)

run: $(ISO)
	qemu-system-i386 -cdrom $(ISO)