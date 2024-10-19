#include <vga.h>
#include <cpu/gdt.h>
#include <string.h>
#include <io/kb.h>
#include <cpu/mem.h>
#include <stdbool.h>
#include <fs/fat16.h>
#include <fs/disk.h>
#include <drv/ide.h>
#include <io/idt.h>

void kentr(void)
{
    clear_screen();
    init_gdt();
    init_idt();
    init_pit();
    init_dmem();
    fs_load();

    kprint(">");
    irq_install_handler(1, &sash);
    
    return;
}