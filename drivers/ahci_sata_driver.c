#include <drv/pci.h>
#include <drv/sata.h>
#include <stdio.h>
#include <string.h>
#include <io/idt.h>
#include <io/iotools.h>
#include <stdint.h>

ahci_controller_t controller;
void ahci_irq_handler(struct InterruptRegisters *regs);

// Функция инициализации драйвера
void ahci_sata_init() {
    // Ищем SATA контроллер на PCI шине
    pci_dev_t dev = pci_get_device(0x8086, 0x2829, -1); // Intel SATA AHCI контроллер
    if (dev.bits == 0) {
        printf("AHCI SATA controller not found\n");
        return;
    }

    // Получаем базовый адрес контроллера
    uint32_t base_address = pci_read(dev, PCI_BASE_ADDRESS_0);
    printf("AHCI SATA controller base address: 0x%x\n", base_address);

    // Получаем IRQ контроллера
    uint32_t irq = pci_read(dev, PCI_INTERRUPT_LINE);
    printf("AHCI SATA controller IRQ: %d\n", irq);

    // Инициализируем контроллер
    ahci_controller_t controller;
    controller.pci_dev = dev;
    controller.base_address = base_address;
    controller.irq = irq;

    // Регистрируем IRQ обработчик
    irq_install_handler(irq, &ahci_irq_handler);

    // Включаем контроллер
    uint32_t ahci_enable = 1;
    port_dword_out(base_address + 0x40, ahci_enable);

    printf("AHCI SATA controller initialized\n");
}

// IRQ обработчик для SATA контроллера
void ahci_irq_handler(struct InterruptRegisters *regs) {
    // Обработка прерывания от SATA контроллера
    uint32_t ahci_status = port_dword_in(controller.base_address + 0x40);
    if (ahci_status & 0x1) {
        printf("AHCI SATA controller interrupt\n");
        // Обработка прерывания
    }
}

// Функция чтения данных с диска
void ahci_sata_read(uint32_t sector, uint32_t count, uint8_t *buffer) {
    // Подготовка команды чтения
    uint32_t ahci_cmd = 0x1; // Чтение
    uint32_t ahci_sector = sector;
    uint32_t ahci_count = count;

    // Запись команды в контроллер
    port_dword_out(controller.base_address + 0x48, ahci_cmd);
    port_dword_out(controller.base_address + 0x50, ahci_sector);
    port_dword_out(controller.base_address + 0x58, ahci_count);

    // Ожидание завершения операции
    while (1) {
        uint32_t ahci_status = port_dword_in(controller.base_address + 0x40);
        if (ahci_status & 0x2) {
            break;
        }
    }

    // Чтение данных из контроллера
    uint32_t ahci_data = port_dword_in(controller.base_address + 0x60);
    memcpy(buffer, (uint8_t *)&ahci_data, count * 512);
}

// Функция записи данных на диск
void ahci_sata_write(uint32_t sector, uint32_t count, uint8_t *buffer) {
    // Подготовка команды записи
    uint32_t ahci_cmd = 0x2; // Запись
    uint32_t ahci_sector = sector;
    uint32_t ahci_count = count;

    // Запись команды в контроллер
    port_dword_out(controller.base_address + 0x48, ahci_cmd);
    port_dword_out(controller.base_address + 0x50, ahci_sector);
    port_dword_out(controller.base_address + 0x58, ahci_count);

    // Ожидание завершения операции
    while (1) {
        uint32_t ahci_status = port_dword_in(controller.base_address + 0x40);
        if (ahci_status & 0x2) {
            break;
        }
    }

    // Запись данных в контроллер
    uint32_t ahci_data = port_dword_in(controller.base_address + 0x60);
    memcpy((uint8_t *)&ahci_data, buffer, count * 512);
}