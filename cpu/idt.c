#include <io/idt.h>
#include <io/iotools.h>
#include <vga.h>
#include <stdint.h>

struct idt_entry_struct idt_entries[256];
struct idt_ptr_struct idt_ptr;

extern void idt_flush(uint32_t);

void init_idt()
{
    kprint("Setting up Interrupt Descriptor Table...");
    idt_ptr.limit = sizeof(struct idt_entry_struct) * 256 - 1;
    idt_ptr.base = (uint32_t) &idt_entries;
    memset(&idt_entries, 0, sizeof(struct idt_entry_struct) * 256);

    port_byte_out(0x20, 0x11);
    port_byte_out(0xA0, 0x11);

    port_byte_out(0x21, 0x20);
    port_byte_out(0xA1, 0x28);

    port_byte_out(0x21, 0x04);
    port_byte_out(0xA1, 0x02);

    port_byte_out(0x21, 0x01);
    port_byte_out(0xA1, 0x01);

    port_byte_out(0x21, 0x00);
    port_byte_out(0xA1, 0x00);

    set_idt_gate(0, (uint32_t)isr0, 0x08, 0x8E);
    set_idt_gate(1, (uint32_t)isr1, 0x08, 0x8E);
    set_idt_gate(2, (uint32_t)isr2, 0x08, 0x8E);
    set_idt_gate(3, (uint32_t)isr3, 0x08, 0x8E);
    set_idt_gate(4, (uint32_t)isr4, 0x08, 0x8E);
    set_idt_gate(5, (uint32_t)isr5, 0x08, 0x8E);
    set_idt_gate(6, (uint32_t)isr6, 0x08, 0x8E);
    set_idt_gate(7, (uint32_t)isr7, 0x08, 0x8E);
    set_idt_gate(8, (uint32_t)isr8, 0x08, 0x8E);
    set_idt_gate(9, (uint32_t)isr9, 0x08, 0x8E);
    set_idt_gate(10, (uint32_t)isr10, 0x08, 0x8E);
    set_idt_gate(11, (uint32_t)isr11, 0x08, 0x8E);
    set_idt_gate(12, (uint32_t)isr12, 0x08, 0x8E);
    set_idt_gate(13, (uint32_t)isr13, 0x08, 0x8E);
    set_idt_gate(14, (uint32_t)isr14, 0x08, 0x8E);
    set_idt_gate(15, (uint32_t)isr15, 0x08, 0x8E);
    set_idt_gate(16, (uint32_t)isr16, 0x08, 0x8E);
    set_idt_gate(17, (uint32_t)isr17, 0x08, 0x8E);
    set_idt_gate(18, (uint32_t)isr18, 0x08, 0x8E);
    set_idt_gate(19, (uint32_t)isr19, 0x08, 0x8E);
    set_idt_gate(20, (uint32_t)isr20, 0x08, 0x8E);
    set_idt_gate(21, (uint32_t)isr21, 0x08, 0x8E);
    set_idt_gate(22, (uint32_t)isr22, 0x08, 0x8E);
    set_idt_gate(23, (uint32_t)isr23, 0x08, 0x8E);
    set_idt_gate(24, (uint32_t)isr24, 0x08, 0x8E);
    set_idt_gate(25, (uint32_t)isr25, 0x08, 0x8E);
    set_idt_gate(26, (uint32_t)isr26, 0x08, 0x8E);
    set_idt_gate(27, (uint32_t)isr27, 0x08, 0x8E);
    set_idt_gate(28, (uint32_t)isr28, 0x08, 0x8E);
    set_idt_gate(29, (uint32_t)isr29, 0x08, 0x8E);
    set_idt_gate(30, (uint32_t)isr30, 0x08, 0x8E);
    set_idt_gate(31, (uint32_t)isr31, 0x08, 0x8E);

    set_idt_gate(32, (uint32_t)irq0, 0x08, 0x8E);
    set_idt_gate(33, (uint32_t)irq1, 0x08, 0x8E);
    set_idt_gate(34, (uint32_t)irq2, 0x08, 0x8E);
    set_idt_gate(35, (uint32_t)irq3, 0x08, 0x8E);
    set_idt_gate(36, (uint32_t)irq4, 0x08, 0x8E);
    set_idt_gate(37, (uint32_t)irq5, 0x08, 0x8E);
    set_idt_gate(38, (uint32_t)irq6, 0x08, 0x8E);
    set_idt_gate(39, (uint32_t)irq7, 0x08, 0x8E);
    set_idt_gate(40, (uint32_t)irq8, 0x08, 0x8E);
    set_idt_gate(41, (uint32_t)irq9, 0x08, 0x8E);
    set_idt_gate(42, (uint32_t)irq10, 0x08, 0x8E);
    set_idt_gate(43, (uint32_t)irq11, 0x08, 0x8E);
    set_idt_gate(44, (uint32_t)irq12, 0x08, 0x8E);
    set_idt_gate(45, (uint32_t)irq13, 0x08, 0x8E);
    set_idt_gate(46, (uint32_t)irq14, 0x08, 0x8E);
    set_idt_gate(47, (uint32_t)irq15, 0x08, 0x8E);

    set_idt_gate(128, (uint32_t)isr128, 0x08, 0x8E);
    set_idt_gate(177, (uint32_t)isr177, 0x08, 0x8E);

    idt_flush((uint32_t)&idt_ptr);
    kprint("done.\n");
}

void set_idt_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags)
{
    idt_entries[num].base_low = base & 0xFFFF;
    idt_entries[num].sel = sel;
    idt_entries[num].flags = flags | 0x60;
    idt_entries[num].base_high = (base >> 16) & 0xFFFF;
    idt_entries[num].always0 = 0;
}

char *get_ex(int num);
char *get_ex(int num)
{
    switch (num)
    {
        case 0:
            return "Division by zero";
        case 1:
            return "Debug";
        case 2:
            return "Non-maskable interrupt";
        case 3:
            return "Breakpoint";
        case 4:
            return "Overflow";
        case 5:
            return "Bound range exceeded";
        case 6:
            return "Invalid opcode";
        case 7:
            return "Device not available";
        case 8:
            return "Double fault";
        case 9:
            return "Coprocessor segment overrun";
        case 10:
            return "Invalid TSS";
        case 11:
            return "Segment not present";
        case 12:
            return "Stack fault";
        case 13:
            return "General protection fault";
        case 14:
            return "Page fault";
        case 15:
            return "Unknown interrupt";
        case 16:
            return "Coprocessor error";
        case 17:
            return "Alignment fault";
        case 18:
            return "Machine check";
        case 19:
            return "Reserved 1";
        case 20:
            return "Reserved 2";
        case 21:
            return "Reserved 3";
        case 22:
            return "Reserved 4";
        case 23:
            return "Reserved 5";
        case 24:
            return "Reserved 6";
        case 25:
            return "Reserved 7";
        case 26:
            return "Reserved 8";
        case 27:
            return "Reserved 9";
        case 28:
            return "Reserved 10";
        case 29:
            return "Reserved 11";
        case 30:
            return "Reserved 12";
        default:
            return "Unknown exception";
    }
}

void isr_handler(struct InterruptRegisters* resgs)
{
    if (resgs->int_no < 32)
    {
        kprintc("\nSystem exception! Reason: ", 0x0C);
        kprintc(get_ex(resgs->int_no), 0x0C);
        for(;;);
    }
}

void *irq_routines[16] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

void irq_install_handler(int irq, void(*handler)(struct InterruptRegisters *r)) 
{ 
    irq_routines[irq] = handler; 
}

void irq_uninstall_handler(int irq) { irq_routines[irq] = 0; }

void irq_handler(struct InterruptRegisters* regs)
{
    void (*handler)(struct InterruptRegisters *regs);
    handler = irq_routines[regs->int_no - 32];

    if (handler)
    {
        handler(regs);
    }

    if (regs->int_no >= 40)
    {
        port_byte_out(0xA0, 0x20);
    }

    port_byte_out(0x20, 0x20);
}