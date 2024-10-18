#include <vga.h>
#include <cpu/gdt.h>
#include <string.h>
#include <io/kb.h>
#include <io/idt.h>

void kentr(void)
{
    clear_screen();
    init_gdt();
    init_idt();
    init_pit();
    
    char *input;

    while (1)
    {
        get_string(input);
        kprint("You entered: ");
        kprintci(input, 0x70);
            kprint("\n");
        for (int i = 0; i < strlen(input); i++)
        {
            input[i] = 0;
        }
    }
    
    return;
}