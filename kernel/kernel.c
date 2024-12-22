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
    init_idt();
    init_pit();
    int j = 0;
    for (int x = 0; x < 480; x++)
    {
        for (int y = 0; y < 111; y++)
        {
            pixel(boot_info, x, y, aster_vbe[j]);
            j++;
        }
    }
    init_dmem();
    init_chipset();
    pci_init();
    ata_init();
    init_vfs();
    start_global_config();
    clear_screen();
    printf("Flags: 0x%x\n", boot_info->flags);
    printf("Mem Lower: 0x%x\n", boot_info->mem_lower);
    printf("Mem Upper: 0x%x\n", boot_info->mem_upper);
    printf("Boot Device: 0x%x\n", boot_info->boot_device);
    printf("Cmdline: 0x%x\n", boot_info->cmdline);
    printf("Modcount: 0x%x\n", boot_info->mods_count);
    printf("Modaddr: 0x%x\n", boot_info->mods_addr);
    printf("Mmapaddr: 0x%x\n", boot_info->mmap_addr);
    printf("VBE Mode: 0x%x\n", boot_info->vbe_mode);
    printf("Framebuffer Address: 0x%x\n", boot_info->framebuffer_addr);
    printf("Framebuffer Width: %d\n", boot_info->framebuffer_width);
    printf("Framebuffer Height: %d\n", boot_info->framebuffer_height);
    printf("Framebuffer Depth: %d\n", boot_info->framebuffer_bpp);
    printf("Framebuffer Pitch: %d\n", boot_info->framebuffer_pitch);
    printf("Magic: 0x%x\n", magic);
    //init_multiboot(boot_info);
    uint64_t *VESA = (uint64_t*)boot_info->framebuffer_addr;
    uint64_t* pixel_addr = 0;
    uint64_t color;
    for (int x = 0; x < boot_info->framebuffer_width; x++)
    {
        for (int y = 0; y < boot_info->framebuffer_height; y++)
        {
            pixel(boot_info, x, y, 0x00);
        } 
    }
    vbe_printf(boot_info, "Aster Operating System 0.7\nlala", 0, 0, 0x07);
    vbe_printf(boot_info, "Copyright (c) 2022-2023 Michael Bugaev", 0, 2, 0x07);
    vbe_printf(boot_info, "Framebuffer address: ", 0, 4, 0x07);
    char *addr;
    itoa(boot_info->framebuffer_addr, addr, 16);
    vbe_printf(boot_info, addr, 21, 4, 0x07);

    // char init[] = "verinfo";
    // File *init_src = new_file("/init", init, sizeof(init));
    // execute_init(init_src->name);
    // irq_install_handler(1, &sash);
    sash_shell();
    
}
