#ifndef CHIPSET_H
#define CHIPSET_H

#include <stdint.h>
#include <stdbool.h>
#include <io/idt.h>

#define CHIPSET_BASE_PORT 0x200 // Базовый адрес порта чипсета
#define CHIPSET_MAX_REGISTERS 16 // Максимальное количество регистров

typedef struct {
    uint16_t base_port;
    uint8_t registers[CHIPSET_MAX_REGISTERS];
    bool initialized;
} ChipsetDevice;

// Инициализация драйвера чипсета
void chipset_initialize();

// Чтение данных из регистра чипсета
uint8_t chipset_read_register(uint8_t reg);

// Запись данных в регистр чипсета
void chipset_write_register(uint8_t reg, uint8_t data);

// Включение определенной функции чипсета
void chipset_enable_feature(uint8_t feature);

// Отключение определенной функции чипсета
void chipset_disable_feature(uint8_t feature);

// Получение статуса чипсета
uint16_t chipset_get_status();

// Обработчик прерываний чипсета
void chipset_interrupt_handler(struct InterruptRegisters *regs);

// Настройка DMA для чипсета
void chipset_configure_dma(uint8_t channel, uint32_t address, uint32_t count);

// Очистка и завершение работы драйвера
void chipset_cleanup();

// Диагностика чипсета
void chipset_run_diagnostics();

#endif // CHIPSET_DRIVER_H