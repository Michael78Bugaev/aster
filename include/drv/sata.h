#ifndef SATA_H
#define SATA_H

#include <stdint.h>
#include <drv/pci.h>

// Структура для хранения информации о SATA контроллере
typedef struct {
    pci_dev_t pci_dev; // Информация о PCI устройстве
    uint32_t base_address; // Базовый адрес контроллера
    uint32_t irq; // IRQ контроллера
} ahci_controller_t;

// Функция инициализации драйвера
void ahci_sata_init();

// Функция чтения данных с диска
void ahci_sata_read(uint32_t sector, uint32_t count, uint8_t *buffer);

// Функция записи данных на диск
void ahci_sata_write(uint32_t sector, uint32_t count, uint8_t *buffer);

#endif // SATA_H