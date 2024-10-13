#include <vga.h>
#include <cpu/gdt.h>
#include <cpu/pit.h>
#include <io/kb.h>
#include <io/idt.h>

void kentr(void)
{
    clear_screen();
    init_gdt();
    init_idt();
    init_pit();
    //kprint(get_string());
    //kprint(1/0);
    irq_install_handler(1, &handler);
    return;
}