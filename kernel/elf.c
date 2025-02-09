#include <cpu/elf.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

void executeElfFile(uint8_t *file, uint32_t size) {
    Elf32_Ehdr *elfHeader = (Elf32_Ehdr *)file;

    // Проверяем, является ли файл ELF
    if (elfHeader->e_ident[0] != 0x7F || elfHeader->e_ident[1] != 'E' || elfHeader->e_ident[2] != 'L' || elfHeader->e_ident[3] != 'F') {
        printf("Error: Not an ELF file\n");
        return;
    }

    // Проверяем, является ли файл исполняемым
    if (elfHeader->e_type != 2) {
        printf("Error: Not an executable ELF file\n");
        return;
    }

    // Проверяем, является ли файл 32-битным
    if (elfHeader->e_ident[4] != 1) {
        printf("Error: Not a 32-bit ELF file\n");
        return;
    }

    // Проверяем, является ли файл для x86 архитектуры
    if (elfHeader->e_machine != 3) {
        printf("Error: Not an x86 ELF file\n");
        return;
    }

    // Загружаем программные заголовки
    Elf32_Phdr *programHeaders = (Elf32_Phdr *)(file + elfHeader->e_phoff);

    // Загружаем секции
    Elf32_Shdr *sections = (Elf32_Shdr *)(file + elfHeader->e_shoff);

    // Загружаем строку таблицы секций
    char *sectionStringTable = (char *)(file + sections[elfHeader->e_shstrndx].sh_offset);

    // Загружаем символы
    Elf32_Sym *symbols = (Elf32_Sym *)(file + sections[1].sh_offset);

    // Выполняем программу
    uint32_t entryPoint = elfHeader->e_entry;
    if (entryPoint != 0) {

        void (*entryPointFunction)() = (void (*)())entryPoint;
        entryPointFunction();
    } else {
        printf("Error: Entry point is not set\n");
    }
}