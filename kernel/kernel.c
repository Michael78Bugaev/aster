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
    ata_init();
    start_global_config();
    kprint("Example usage of ATA driver \n");
    uint32_t t = chs_to_lba(0, 0, 0);
    uint16_t data[4] = {1, 2, 3, 3};
    if(ata_write(t, 1, data, 4)) {
        kprint("Write error\n");
    }
    uint16_t *tmp = ata_read(t, 0);
    for (int i = 0; i < strlen(tmp); i++)
    {
        printf("%4x\n", tmp[i]);
    }
    sash_shell();
    
}
