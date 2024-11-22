// kernel.c
#include <vga.h>
#include <cpu/gdt.h>
#include <io/idt.h>
#include <cpu/mem.h>
#include <cpu/pit.h>
#include <fs/initrd.h>
#include <stdio.h>
#include <drv/pci.h>
#include <config.h>
#include <chset/chipset.h>
#include <io/kb.h>
#include <sash.h>
#include <progress.h>

void kentr(void) {

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
    init_pci();

    init_vfs();
    printf("\n");
    uint8_t welcome_data[] = "Welcome!";
    File *welcome = new_file("/README", welcome_data, sizeof(welcome_data));
    list_directory(current_directory);

    execute_sash("verinfo");

    printf("\nmasteruser: %s &", "/");
    irq_install_handler(1, &sash);
}