include make.cfg

$(BUILD_DIR)/AstrKernel: $(BUILD_DIR)/boot.asmo $(BUILD_DIR)/idt.asmo kernel.o
	@echo "Linking..."
	@echo $(OBJ)
	$(LD) -m elf_i386 $(LDFLAGS) -o $@ $^ iotools.o vga.o

	cp $(BUILD_DIR)/AstrKernel $(BUILD_DIR)/iso/boot/AstrKernel
	grub-mkrescue -o aster_32-bit.iso $(BUILD_DIR)/iso
	qemu-system-i386 -cdrom aster_32-bit.iso

$(BUILD_DIR)/idt.asmo: $(BOOT_DIR)/idt.asm
	@echo "Compiling interrupts main file..."
	$(AS) $(ASMFLAGS) -o $@ $^

$(BUILD_DIR)/boot.asmo: $(BOOT_DIR)/boot.asm
	@echo "Compiling multiboot..."
	$(AS) $(ASMFLAGS) -o $@ $^

kernel.o:
	@echo "Compiling kernel..."
	$(CC) $(CFLAGS) $(C_FILES)

clean:
	rm -rf *.o $(BUILD_DIR)/*.o