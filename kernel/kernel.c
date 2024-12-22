// kernel.c
#include <vga.h>
#include <cpu/gdt.h>
#include <io/idt.h>
#include <cpu/mem.h>
#include <drv/vbe.h>
#include <cpu/pit.h>
#include <multiboot.h>
#include <fs/ext2.h>
#include <fs/initrd.h>
#include <drv/ata.h>
#include <stdio.h>
#include <string.h>
#include <drv/pci.h>
#include <config.h>
#include <chset/chipset.h>
#include <io/kb.h>
#include <sash.h>
#include <progress.h>

void kentr(uint32_t magic, struct multiboot_info* boot_info) {
    init_gdt();
    _GLOBAL_MBOOT = boot_info;
    init_idt();
    init_pit();

    init_dmem();
    init_chipset();
    pci_init();
    ata_init();
    init_vfs();
    start_global_config();
    vbe_screen_clear(boot_info, 0x00);
    // vbe_printf(boot_info, "Aster Operating System 0.7", 0, 0, 0x07);
    // vbe_printf(boot_info, "Copyright (C) 2024-2025 Michael Bugaev", 0, 1, 0x07);
    // vbe_printf(boot_info, "/ &", 0, 3, 0x07);

    _print(_globl_cursor, "Testing!");
    _print(_globl_cursor, "Aster!");

    sash_shell();
    
}
