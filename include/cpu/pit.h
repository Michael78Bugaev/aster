#ifndef PIT_H
#define PIT_H

#include <io/idt.h>
#include <stdint.h>

#define TICKS_PER_SECOND 500

void init_pit();
void pit_handler(struct InterruptRegisters *regs);
void wait(int ms);
uint64_t get_ticks();

#endif