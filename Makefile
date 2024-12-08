include make.cfg

$(BUILD_DIR)/AstrKernel: $(BUILD_DIR)/boot.asmo $(BUILD_DIR)/idt.asmo $(BUILD_DIR)/gdt.asmo kernel.o
	@echo "Linking..."
	@$(LD) -m elf_i386 $(LDFLAGS) -o $@ $^ progress.o videocard.o ata.o multiboot.o nfat.o fat32.o dir.o usb.o sedit.o file.o initrd.o pci.o chipset.o cbreak.o config.o iotools.o stdio.o mem.o display.o gdt.o idt.o kb.o string.o pit.o sash.o

	@cp $(BUILD_DIR)/AstrKernel $(BUILD_DIR)/iso/boot/AstrKernel
	@grub-mkrescue -o aster_32-bit.iso $(BUILD_DIR)/iso
	@make clean
	@qemu-system-i386 -m 1024M -cdrom aster_32-bit.iso

$(BUILD_DIR)/idt.asmo: $(BOOT_DIR)/idt.asm
	@echo "Compiling interrupts main file..."
	@$(AS) $(ASMFLAGS) -o $@ $^

$(BUILD_DIR)/boot.asmo: $(BOOT_DIR)/boot.asm
	@echo "Compiling multiboot..."
	@$(AS) $(ASMFLAGS) -o $@ $^

$(BUILD_DIR)/gdt.asmo: $(BOOT_DIR)/gdt.asm
	@echo "Compiling GDT..."
	@$(AS) $(ASMFLAGS) -o $@ $(BOOT_DIR)/gdt.asm

$(BUILD_DIR)/disk.asmo: $(BOOT_DIR)/disk.asm
	@echo "Compiling disk driver..."
	@$(AS) $(ASMFLAGS) -o $@ $(BOOT_DIR)/disk.asm

kernel.o:
	@echo "Compiling kernel..."
	@$(CC) $(CFLAGS) $(C_FILES)

clean:
	@rm -rf *.o $(BUILD_DIR)/*.asmo