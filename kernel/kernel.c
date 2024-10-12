#include <vga.h>
#include <cpu/gdt.h>
#include <io/kb.h>
#include <io/idt.h>

void kentr(void)
{
    clear_screen();
    init_gdt();
    init_idt();
    kprint("Origin Aster (Aster kernel)\n\n");
    kprint(get_string());
    //kprint(1/0);
    return;
}