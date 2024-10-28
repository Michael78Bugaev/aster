#include <vga.h>
#include <cpu/gdt.h>
#include <string.h>
#include <io/kb.h>
#include <cpu/mem.h>
#include <fs/fat32.h>
#include <sash.h>
#include <drv/ata.h>
#include <stdbool.h>
#include <sfat32.h>
#include <cpu/pit.h>
#include <io/idt.h>

#define NULL (void*)0

void kentr(void)
{
    init_gdt();
    init_idt();
    init_pit();
    init_dmem();

    kprintc("Warning: disks have not initiliazed yet.\n         You need to initiliaze them by yourself.\n\n", 0x0F);
    
    execute_sash("rainbow");
    execute_sash("verinfo");

    kprint(get_current_directory());
    kprint(">");
    irq_install_handler(1, &sash);
    
    return;
}