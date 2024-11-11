// kernel.c
#include <vga.h>
#include <cpu/gdt.h>
#include <io/idt.h>
#include <cpu/mem.h>
#include <cpu/pit.h>
#include <stdio.h>
#include <drv/pci.h>
#include <chset/chipset.h>
#include <io/kb.h>
#include <drv/disk.h>
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

    init_chipset();
    init_pci();

    printf("\nAster 32-bit kernel 1.00\n");
    printf("2024-2025 Created by Michael Bugaev\n");

    printf("masteruser: %s &", "/");
    irq_install_handler(1, &sash);
}