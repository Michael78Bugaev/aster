// pci.h
#ifndef PCI_H
#define PCI_H

#include <stdint.h>
#include <drv/sata.h>

// Адреса конфигурационного пространства PCI
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

// Смещения в конфигурационном пространстве PCI
#define PCI_VENDOR_ID      0x00
#define PCI_DEVICE_ID      0x02
#define PCI_COMMAND        0x04
#define PCI_STATUS         0x06
#define PCI_REVISION_ID    0x08
#define PCI_PROG_IF        0x09
#define PCI_SUBCLASS       0x0A
#define PCI_CLASS          0x0B
#define PCI_CACHE_LINE_SIZE 0x0C
#define PCI_LATENCY_TIMER  0x0D
#define PCI_HEADER_TYPE    0x0E
#define PCI_BIST           0x0F
#define PCI_BAR0           0x10
#define PCI_BAR1           0x14
#define PCI_BAR2           0x18
#define PCI_BAR3           0x1C
#define PCI_BAR4           0x20
#define PCI_BAR5           0x24

// AHCI Class Code
#define AHCI_CLASS    0x01
#define AHCI_SUBCLASS 0x06
#define AHCI_PROG_IF  0x01

// Функции для работы с PCI
uint32_t pci_config_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void pci_config_write(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value);
int pci_find_device(uint16_t vendor_id, uint16_t device_id, uint16_t *bus, uint16_t *device, uint16_t *function);
uint32_t pci_read_bar(uint8_t bus, uint8_t slot, uint8_t func, uint8_t bar);

// Структура для хранения информации об устройстве PCI
typedef struct {
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t class_code;
    uint8_t subclass;
    uint8_t prog_if;
    uint8_t revision;
    uint8_t bus;
    uint8_t device;
    uint8_t function;
} pci_device_t;

// Функция для получения информации об устройстве
void pci_get_device_info(uint8_t bus, uint8_t device, uint8_t function, pci_device_t *dev_info);

// Функция для сканирования шины PCI
void pci_scan_bus(void);

uint32_t pci_read(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset);
HBA_MEM* find_ahci_controller();

#endif