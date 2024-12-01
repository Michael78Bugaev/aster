// kernel.c
#include <vga.h>
#include <cpu/gdt.h>
#include <io/idt.h>
#include <cpu/mem.h>
#include <cpu/pit.h>
#include <multiboot.h>
#include <fs/initrd.h>
#include <drv/ata.h>
#include <stdio.h>
#include <drv/pci.h>
#include <config.h>
#include <chset/chipset.h>
#include <io/kb.h>
#include <sash.h>
#include <progress.h>

void kentr(uint32_t magic, struct multiboot_info* boot_info) {

    start_global_config();
    // Инициализация GDT
    init_gdt();
    // Инициализация IDT
    init_idt();
    // Инициализация PIT
    init_pit();
    // Инициализация памяти
    init_dmem();

    init_chipset();
    pci_init();
    ata_init();

    // Пример буфера для чтения и записи
    uint8_t read_buffer[512];
    uint8_t write_buffer[512];

    // Заполнение буфера данными для записи
    for (int i = 0; i < 512; i++) {
        write_buffer[i] = (uint8_t)i; // Пример данных: последовательные числа
    }

    // Запись данных на первый диск (drive 0) в сектор 0
    ata_write(0, 0, 1, write_buffer);
    printf("Data written to drive 0, sector 0.\n");

    // Чтение данных с первого диска (drive 0) из сектора 0
    ata_read(0, 0, 1, read_buffer);
    printf("Data read from drive 0, sector 0:\n");

    // Вывод прочитанных данных
    for (int i = 0; i < 512; i++) {
        if (i % 16 == 0) {
            printf("\n");
        }
        printf("%02X ", read_buffer[i]);
    }
    printf("\n");

    // Проверка соответствия записанных и прочитанных данных
    if (memcmp(read_buffer, write_buffer, 512) == 0) {
        printf("Data verification successful: written data matches read data.\n");
    } else {
        printf("Data verification failed: written data does not match read data.\n");
    }

    init_vfs();
    printf("\n");
    uint8_t init[] = "clear\nverinfo\n";
    File *init_src = new_file("/init", init, sizeof(init));

    execute_init("/init");

    init_multiboot(boot_info);

    printf("\n%s &", current_directory->name);
    irq_install_handler(1, &sash);
}