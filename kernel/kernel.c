#include <vga.h>
#include <cpu/gdt.h>
#include <io/idt.h>

void kentr(void)
{
    clear_screen();
    init_gdt();
    init_idt();
    kprint("Origin Aster (Aster kernel)\n\nTrying to divide by zero... he-he-he!!\nkprint(1/0);");
    kprint(1/0);
    return;
}