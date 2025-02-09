// kernel.c
#include <vga.h>
#include <cpu/gdt.h>
#include <io/idt.h>
#include <cpu/mem.h>
#include <drv/vbe.h>
#include <cpu/pit.h>
#include <multiboot.h>
#include <fs/ext2.h>
#include <drv/ide.h>
#include <drv/sata.h>
#include <stdio.h>
#include <cpu/elf.h>
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
    //identify();
    start_global_config();
    printf("Initiliazing devices...\n");
    device_init();
    // printf("Initializing virtual filesystem...\n");
    // vfs_init();
    // ASTERFS_init();
    // devfs_init();
    // fat12_init();
    // ext2_init();
    ide_init();

    sash_shell();
    
}
