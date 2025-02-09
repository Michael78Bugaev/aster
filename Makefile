# Определение переменных
CC = gcc
AS = nasm
LD = ld
CFLAGS=-c -g -fcommon -Werror -Wimplicit -w -I include/ -ffreestanding -m32 -fno-inline-functions -O2 -fno-omit-frame-pointer
CPPFLAGS=-c -g -fcommon -Werror -w -I ./include/ -ffreestanding -m32 -fno-inline-functions -O2 -fno-omit-frame-pointer
ASMFLAGS=-f elf32
LDFLAGS= -T link.ld --allow-multiple-definition -m elf_i386

# Определение директорий
SRC_DIR = .
BUILD_DIR = build

# Определение файлов
C_SOURCES = $(wildcard $(SRC_DIR)/*.c $(SRC_DIR)/*/*.c $(SRC_DIR)/*/*/*.c)
ASM_SOURCES = $(wildcard $(SRC_DIR)/*.asm $(SRC_DIR)/*/*.asm $(SRC_DIR)/*/*/*.asm)
C_OBJECTS = $(addprefix $(BUILD_DIR)/, $(notdir $(C_SOURCES:.c=.o)))
ASM_OBJECTS = $(addprefix $(BUILD_DIR)/, $(notdir $(ASM_SOURCES:.asm=.asmo)))

# Цель по умолчанию
all: $(BUILD_DIR) kernel

# Создание директории build
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Компилирование C-файлов
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/*/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/*/*/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Компилирование ASM-файлов
$(BUILD_DIR)/%.asmo: $(SRC_DIR)/%.asm
	$(AS) $(ASMFLAGS) $< -o $@

$(BUILD_DIR)/%.asmo: $(SRC_DIR)/*/%.asm
	$(AS) $(ASMFLAGS) $< -o $@

$(BUILD_DIR)/%.asmo: $(SRC_DIR)/*/*/%.asm
	$(AS) $(ASMFLAGS) $< -o $@

# Линковка объектных файлов
kernel: $(C_OBJECTS) $(ASM_OBJECTS)
	$(LD) $(LDFLAGS) -o $(BUILD_DIR)/kernel $^
	cp $(BUILD_DIR)/kernel ./iso/boot/aster
	grub-mkrescue -o aster_32-bit.iso ./iso
	qemu-system-i386 -cdrom aster_32-bit.iso -m 4096M

# Цель для очистки
clean:
	rm -rf $(BUILD_DIR)