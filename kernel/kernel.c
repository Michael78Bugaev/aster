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
#include <drv/sata.h>
#include <stdio.h>
#include <fs/fat32.h>
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
    init_vfs();

    start_global_config();
    ahci_sata_init();
    sash_shell();
    
}
