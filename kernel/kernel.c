// kernel.c
#include <vga.h>
#include <cpu/gdt.h>
#include <io/idt.h>
#include <cpu/pit.h>
#include <stdio.h>
#include <storage.h>
#include <drv/ata.h>
#include <drv/sata.h>
#include <io/kb.h>
#include <sash.h>
#include <progress.h>

void kentr(void) {
    // Инициализация GDT
    init_gdt();
    // Инициализация IDT
    init_idt();
    // Инициализация PIT
    init_pit();
    // Инициализация памяти
    init_dmem();
    
    storage_init();

    storage_device_t* disk = storage_get_device(0);
    if (disk) {
        // Читаем сектор
        uint8_t buffer[512];
        if (disk->read(0, 1, buffer) == 0) {
            printf("Successfully read first sector\n");
        }
    }

    // Продолжение работы ядра...
    printf("\nAster 32-bit kernel 1.00\n");
    printf("2024-2025 Created by Michael Bugaev\n");

    printf("%s>", get_current_directory());
    irq_install_handler(1, &sash);
}