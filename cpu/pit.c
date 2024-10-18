#include <cpu/pit.h>
#include <vga.h>
#include <io/iotools.h>
#include <io/idt.h>
#include <stdint.h>

uint64_t ticks;
const uint32_t freq = 1000;

void init_pit()
{
    ticks = 0;
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