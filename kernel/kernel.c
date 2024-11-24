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
    uint8_t init[] = "clear\nverinfo\n";
    File *init_src = new_file("/init", init, sizeof(init));
    
    execute_init("/init");

    printf("\nmasteruser: %s &", current_directory->name);
    irq_install_handler(1, &sash);
}