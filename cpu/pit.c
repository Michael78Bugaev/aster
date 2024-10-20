#include <cpu/pit.h>
#include <vga.h>
#include <io/iotools.h>
#include <io/idt.h>
#include <stdbool.h>
#include <stdint.h>

uint64_t ticks;
const uint32_t freq = 1000;
int target = 0;
int old_ticks;
bool end = false;

void inwait(struct InterruptRegisters *regs);

void init_pit()
{
    ticks = 0;
    target = 0;
    irq_install_handler(0, &pit_handler);

    uint32_t div = 1193180 / freq;

    port_byte_out(0x43, 0x36);
    port_byte_out(0x40, (uint8_t)div & 0xFF);
    port_byte_out(0x40, (uint8_t)((div >> 8) & 0xFF));
}
void pit_handler(struct InterruptRegisters *regs)
{
    ticks += 1;
}

void inwait(struct InterruptRegisters *regs)
{
    if (end = false)
    {
        if (ticks < target)
        {
            ticks += 1;
        }
        else
        {
            kprint("timer ended!");
            end = true;
            irq_install_handler(0, &pit_handler);
        }
    }
}

void wait(int ms)
{
    old_ticks = ticks;
    target = ms + ticks;

    while (ticks < target)
    {
        
    }
    
}

uint64_t get_ticks()
{
    return ticks;
}