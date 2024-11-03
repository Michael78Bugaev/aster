#ifndef CHIPSET_H
#define CHIPSET_H

#include <stdint.h>
#include <stdbool.h>
#include <cpu/pci.h>

// Стандартные PCI ID для чипсетов
#define INTEL_VENDOR_ID    0x8086
#define AMD_VENDOR_ID      0x1022

// Базовые адреса регистров конфигурации
#define CHIPSET_CONFIG_ADDRESS  0xCF8
#define CHIPSET_CONFIG_DATA     0xCFC

typedef struct {
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t revision;
    uint8_t prog_if;
    uint8_t subclass;
    uint8_t class_code;
    uint32_t capabilities;
} chipset_info_t;

// Функции драйвера
bool chipset_init(void);
void chipset_get_info(chipset_info_t* info);
uint32_t chipset_read_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void chipset_write_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value);
bool chipset_enable_feature(uint32_t feature);
bool chipset_disable_feature(uint32_t feature);

#endif