// kernel.c
#include <vga.h>
#include <cpu/gdt.h>
#include <io/idt.h>
#include <cpu/mem.h>
#include <cpu/pit.h>
#include <drv/chipset.h>
#include <drv/sata_chipset.h>
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

    printf("\n<(0a)>Aster<(07)> 32-bit kernel 1.00\n");
    printf("2024-2025 Created by Michael Bugaev\n");

    printf("<(02)>masteruser<(0f)>: <(0d)>%s<(0e)> &<(07)>", get_curdir());
    irq_install_handler(1, &sash);
}