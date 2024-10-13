#ifndef PIT_H
#define PIT_H

#include <io/idt.h>

void init_pit();
void pit_handler(struct InterruptRegisters *regs);

#endif