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
    //_GLOBAL_MBOOT = boot_info;
    init_idt();
    init_pit();
    init_dmem();
    init_chipset(); 
    pci_init();
    ata_init();
    init_vfs();

    start_global_config();

    printf("\nLocal host name: ");
    char *cusr = scanf();
    strcpy(COMPUTER_NAME, cusr);
    printf("Login: ");
    char *usr = scanf();
    strcpy(current_username, usr);
    printf("Welcome to Aster!\n", current_username);
    printf("<(0f)>This kernel may be content some errors or issues<(07)>, because i'm only\none, who develops this project.\n\nIf you want, you can join to the Aster Operating System\ndevelopment and help me.\n\n");
    //vbe_screen_clear(boot_info, 0x0f);
    // _globl_cursor.x = 0;
    // _globl_cursor.y = 0;
    // printf("Test\n");

    //irq_install_handler(1, &sash);
    sash_shell();
    
}
