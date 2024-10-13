#include <io/idt.h>
#include <stdint.h>
#include <cpu/pit.h>
#include <io/iotools.h>
#include <vga.h>

uint64_t ticks;
const uint32_t freq = 1000;

void init_pit()
{
    ticks = 0;
    irq_install_handler(0, &pit_handler);

    uint32_t div = 1193180 / freq;

    port_byte_out(0x43, 0x36);
    port_byte_out(0x40, (uint8_t)(div & 0xFF));
    port_byte_out(0x40, (uint8_t)((div >> 8) & 0xFF));
}
void pit_handler(struct InterruptRegisters *regs)
{
    ticks += 1;
    int old = get_cursor();
    set_cursor(0);
    kprintc("TICK: ", 0x70);
    kprintc(ticks, 0x70);
    set_cursor(old);
}