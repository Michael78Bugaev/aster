#ifndef CHIPSET_H
#define CHIPSET_H

#include <stdint.h>
#include <io/idt.h>

// Определение адресов регистров чипсета
#define CHIPS_BASE_ADDRESS 0x80000000
#define CHIPS_CONFIG_REGISTER 0x00
#define CHIPS_STATUS_REGISTER 0x04

// Функции для работы с чипсетом
void init_chipset();
void interrupt_handler(struct InterruptRegisters *regs);

#endif