#ifndef PIT_H
#define PIT_H

#include <io/idt.h>
#include <stdint.h>

void init_pit();
void pit_handler(struct InterruptRegisters *regs);
void wait(int ms);
uint64_t get_ticks();

#endif