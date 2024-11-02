// kernel.c
#include <vga.h>
#include <cpu/gdt.h>
#include <io/idt.h>
#include <cpu/mem.h>
#include <cpu/pit.h>
#include <stdio.h>
#include <storage.h>
#include <drv/ata.h>
#include <fs/fat32.h>
#include <drv/sata.h>
#include <io/kb.h>
#include <sash.h>
#include <progress.h>

void kentr(void) {
    clear_screen();
    // Инициализация GDT
    init_gdt();
    // Инициализация IDT
    init_idt();
    // Инициализация PIT
    init_pit();
    // Инициализация памяти
    init_dmem();
    
    printf("Scanning PCI bus...\n");
    pci_scan_bus();
    printf("PCI bus scanned.\n");
    if (sata_initialize(&device, 0x1F0, 0x3F6, 0x170, true)) {
        printf("SATA device initialized.\n");
        // malloc(sizeof(uint8_t[512]));
        // uint8_t read_buf[512];
        // if (sata_read_sectors(&device, 0, 1, read_buf)) {
        //     clear_screen();
        //     for (int i = 0; i < 128; i++) {
        //         for (int u = 0; u < 4; u++)
        //         {
        //             wait(10);
        //             kprint_hex_w(read_buf[i]);
        //             kprint(" ");

        //         }
        //         printf("\n");
        //     }
        //     mfree(&read_buf);
        // } else {
        //     printf("Read failed.\n");
        // }

        // malloc(sizeof(uint8_t[512]));
        // uint8_t write_buf[512];
        // if (sata_write_sectors(&device, 0, 1, write_buf)) {
        //     printf("Write successful.\n");
        //     mfree(&write_buf);
        // } else {
        //     printf("Write failed.\n");
        // }
    } else {
        printf("Failed to initialize SATA device.\n");
    }

    printf("\n<(0a)>Aster<(07)> 32-bit kernel 1.00\n");
    printf("2024-2025 Created by Michael Bugaev\n");

    printf("<(02)>masteruser<(0f)>: <(0d)>%s<(0e)> &<(07)>", get_curdir());
    irq_install_handler(1, &sash);
}