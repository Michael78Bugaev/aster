#ifndef SATA_CHIPSET_H
#define SATA_CHIPSET_H

#include <stdint.h>
#include <stdbool.h>
#include <drv/sata.h>
#include <cpu/pci_chipset.h>

#define SATA_PCI_CLASS    0x01
#define SATA_PCI_SUBCLASS 0x06

// Функции для работы с SATA-дисками на стандартном чипсете
bool sata_chipset_init(void);
bool sata_chipset_read(uint32_t lba, uint8_t sectors, void *buffer);
bool sata_chipset_write(uint32_t lba, uint8_t sectors, const void *buffer);
void sata_chipset_identify(void);
pci_device_t* sata_chipset_find_controller(void);

#endif // SATA_CHIPSET_H