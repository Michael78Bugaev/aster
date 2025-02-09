#ifndef ELF_H
#define ELF_H

#include <stdint.h>

// ELF заголовок
typedef struct {
    uint8_t e_ident[16]; // Идентификатор ELF
    uint16_t e_type; // Тип файла (1 - relocatable, 2 - executable, 3 - shared object)
    uint16_t e_machine; // Архитектура (3 - x86)
    uint32_t e_version; // Версия ELF
    uint32_t e_entry; // Адрес точки входа
    uint32_t e_phoff; // Смещение таблицы программных заголовков
    uint32_t e_shoff; // Смещение таблицы секций
    uint32_t e_flags; // Флаги
    uint16_t e_ehsize; // Размер ELF-заголовка
    uint16_t e_phentsize; // Размер записи таблицы программных заголовков
    uint16_t e_phnum; // Количество записей таблицы программных заголовков
    uint16_t e_shentsize; // Размер записи таблицы секций
    uint16_t e_shnum; // Количество записей таблицы секций
    uint16_t e_shstrndx; // Индекс строки таблицы секций
} Elf32_Ehdr;

// Программный заголовок
typedef struct {
    uint32_t p_type; // Тип сегмента (1 - PT_LOAD)
    uint32_t p_offset; // Смещение сегмента в файле
    uint32_t p_vaddr; // Виртуальный адрес сегмента
    uint32_t p_paddr; // Физический адрес сегмента
    uint32_t p_filesz; // Размер сегмента в файле
    uint32_t p_memsz; // Размер сегмента в памяти
    uint32_t p_flags; // Флаги сегмента
    uint32_t p_align; // Выравнивание сегмента
} Elf32_Phdr;

// Секция
typedef struct {
    uint32_t sh_name; // Индекс имени секции
    uint32_t sh_type; // Тип секции (1 - SHT_PROGBITS)
    uint32_t sh_flags; // Флаги секции
    uint32_t sh_addr; // Виртуальный адрес секции
    uint32_t sh_offset; // Смещение секции в файле
    uint32_t sh_size; // Размер секции
    uint32_t sh_link; // Индекс связанной секции
    uint32_t sh_info; // Дополнительная информация
    uint32_t sh_addralign; // Выравнивание секции
    uint32_t sh_entsize; // Размер записи секции
} Elf32_Shdr;

// Символ
typedef struct {
    uint32_t st_name; // Индекс имени символа
    uint32_t st_value; // Значение символа
    uint32_t st_size; // Размер символа
    uint8_t st_info; // Информация о символе
    uint8_t st_other; // Дополнительная информация
    uint16_t st_shndx; // Индекс секции символа
} Elf32_Sym;

void executeElfFile(uint8_t *file, uint32_t size);

#endif